package SlurmBatch;

# ----------------------------------------------------------------------
# SlurmBatch — shared slurm/run submit + wait for prompt0 tasks.
#
# Same tools as processRuns (slurm/run, slurm-*.csh, squeue, hadd).
# Per-task differences are the spec: stage, wrapper, env, NJOBS, merge.
#
#   my $rc = SlurmBatch::run($ctx, {
#     name    => "minalyzer",
#     stage   => "mlzr",
#     wrapper => $ctx->{slurm_analyzer_csh},
#     njobs   => 1,              # force one job (sort); else int(total/chunk)+1
#     chunk   => 100000,
#     total   => $nevents,
#     skip_if => $merged_file,
#     env     => [ [STORAGE1 => $s], [DATADIR => $d], ... ],
#     job_env => sub { my ($j) = @_; return [ [ ANLZR => "-s$j->{skip} -e$j->{chunk}" ] ]; },
#     merge   => sub { ... },
#   });
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Cwd qw(getcwd abs_path);
use File::Basename qw(dirname basename);
use File::Copy qw(move);
use File::Path qw(make_path);

use TaskLib qw(task_log);

our @EXPORT_OK = qw(run njobs prepare_dirs encode_replace);

# ----------------------------------------------------------------------
sub njobs {
    my (%a) = @_;
    return 1 if defined $a{njobs} && $a{njobs} > 0;
    my $total = 0 + ($a{total} // 0);
    my $chunk = 0 + ($a{chunk} // 0);
    return 1 if $chunk < 1;
    return int($total / $chunk) + 1;
}

# ----------------------------------------------------------------------
# Move existing rundir/storage aside (processRuns: mv $dir $stage/old).
sub prepare_dirs {
    my ($ctx, $stage, $run) = @_;
    my $root = $ctx->{root} // "";
    die "SlurmBatch: root empty in ctx\n" if $root eq "";

    my $jobs_parent = "$root/slurm/jobs/$stage";
    my $stor_parent = "$root/slurm/storage1/$stage";
    my $rundir  = "$jobs_parent/$run";
    my $storage = "$stor_parent/$run";

    _rotate($rundir,  "$jobs_parent/old");
    _rotate($storage, "$stor_parent/old");
    make_path($rundir)  unless $ctx->{dry_run};
    make_path($storage) unless $ctx->{dry_run};
    return { rundir => $rundir, storage => $storage, storage_parent => $stor_parent };
}

sub _rotate {
    my ($dir, $old_parent) = @_;
    return unless -d $dir;
    make_path($old_parent);
    my $name = basename($dir);
    my $dest = "$old_parent/$name";
    if (-e $dest) {
        system("rm", "-rf", $dest);
    }
    move($dir, $old_parent) or die "SlurmBatch: cannot mv $dir -> $old_parent: $!\n";
}

# ----------------------------------------------------------------------
# Normalize job_env / env extras to a list of [key, value] arrayrefs.
# Accepts [ [K => v], ... ] or a flat [ K => v, ... ].
sub _pairs {
    my ($extra) = @_;
    return () unless defined $extra && ref($extra) eq "ARRAY" && @$extra;
    if (ref($extra->[0]) eq "ARRAY") {
        return @$extra;
    }
    my @out;
    for (my $i = 0; $i + 1 < @$extra; $i += 2) {
        push @out, [ $extra->[$i], $extra->[$i + 1] ];
    }
    return @out;
}

# ----------------------------------------------------------------------
sub encode_replace {
    my ($pairs) = @_;
    my @parts;
    for my $p (@$pairs) {
        my ($k, $v) = @$p;
        next unless defined $k && $k ne "";
        $v = "" unless defined $v;
        if ($v =~ /\s/ || $v =~ /^-/) {
            $v =~ s/"/\\"/g;
            push @parts, "$k \"$v\"";
        } else {
            push @parts, "$k $v";
        }
    }
    return join("%", @parts);
}

# ----------------------------------------------------------------------
# Relative wrappers live next to this module (prod/prompt0/slurm/...).
# Absolute paths are left unchanged. Falls back to mu3eanca if the
# prompt0 file is not there (old run2025/scripts/... values).
sub _abs_wrapper {
    my ($ctx, $rel) = @_;
    $rel = "" unless defined $rel;
    return $rel if $rel =~ m{^/};
    my $prompt0 = abs_path(dirname(__FILE__));
    my $here = "$prompt0/$rel";
    return $here if -f $here;
    my $anca = $ctx->{mu3eanca} // "";
    my $legacy = ($anca ne "") ? "$anca/$rel" : "";
    return $legacy if $legacy ne "" && -f $legacy;
    return $here;
}

sub _queue_args {
    my ($q) = @_;
    $q = "" unless defined $q;
    $q =~ s/^["']|["']$//g;
    return grep { $_ ne "" } split /\s+/, $q;
}

sub _tar {
    my ($ctx) = @_;
    return $ctx->{tar_file} if defined $ctx->{tar_file} && $ctx->{tar_file} ne "";
    my $root = $ctx->{root} // "";
    my $wd   = $ctx->{workdir} // "";
    return ($root ne "" && $wd ne "") ? "$root/slurm/$wd.tar.gz" : "";
}

# ----------------------------------------------------------------------
sub _submit_one {
    my ($ctx, $name, $rundir, $cmd) = @_;
    task_log($ctx, $name, "SLURM: @$cmd");
    if ($ctx->{dry_run}) {
        task_log($ctx, $name, "DBX: @$cmd");
        return (0, "");
    }

    my $cwd = getcwd();
    chdir($rundir) or die "SlurmBatch: cannot chdir $rundir: $!\n";
    my $pid = open(my $rh, "-|", @$cmd);
    if (!defined $pid) {
        chdir($cwd);
        die "SlurmBatch: cannot run @$cmd: $!\n";
    }
    my $out = "";
    while (my $line = <$rh>) {
        $out .= $line;
        print $line;
    }
    close $rh;
    my $rc = $?;
    chdir($cwd) or die "SlurmBatch: cannot chdir back to $cwd: $!\n";
    if ($rc != 0) {
        task_log($ctx, $name, "slurm/run exited " . ($rc >> 8));
        return (0, "");
    }

    my $slurmid = 0;
    my $slurmout = "";
    for my $ln (split /\n/, $out) {
        if ($ln =~ /Submitted ->([0-9]*)<- with name/) {
            $slurmid = $1;
        }
        if ($ln =~ /batch submission: (\S+)/) {
            $slurmout = $1;
        }
    }
    if ($slurmid > 0) {
        task_log($ctx, $name, "SLURM job $slurmid -> $slurmout");
    }
    return ($slurmid, $slurmout);
}

# ----------------------------------------------------------------------
sub wait_jobs {
    my ($ctx, $name, $rundir, $ids) = @_;
    my %m = %$ids;
    my $njobs = keys %m;
    return 1 if $njobs < 1;
    return 1 if $ctx->{dry_run};

    my $slurmids = "(" . join(", ", sort keys %m) . ")";
    task_log($ctx, $name, "slurmids = $slurmids, sleep 30 second before looking at logfiles");
    sleep(30);

    while ($njobs > 0) {
        if ($njobs > 20) {
            sleep(60);
        } elsif ($njobs > 10) {
            sleep(30);
        } else {
            sleep(10);
        }
        for my $sjob (keys %m) {
            my $result = `squeue -j $sjob 2>&1`;
            if ($result =~ /Invalid job id specified/) {
                task_log($ctx, $name, "SLURM job $sjob not found, probably hit time limit");
                delete $m{$sjob};
                $njobs = keys %m;
                next;
            }
            my $rel  = $m{$sjob};
            my $logf = "$rundir/$rel";
            my $tail = "no file yet\n";
            if (-e $logf) {
                $tail = `tail -1 "$logf"`;
            }
            print("    [$sjob:$rel] $tail");
            if ($tail =~ /This is the end, my friend/) {
                delete $m{$sjob};
            }
            $njobs = keys %m;
        }
    }
    task_log($ctx, $name, "all slurm jobs finished");
    return 1;
}

# ----------------------------------------------------------------------
# Returns 0 on success / skip-existing, 2 on failure.
sub run {
    my ($ctx, $spec) = @_;
    my $name  = $spec->{name}  // "slurm";
    my $stage = $spec->{stage} // die "SlurmBatch: spec.stage required\n";
    my $run   = $ctx->{run}    // die "SlurmBatch: ctx.run required\n";
    my $slurm = $ctx->{slurm_run} // "";
    die "SlurmBatch: slurm_run empty in ctx\n" if $slurm eq "";

    if (defined $spec->{skip_if} && $spec->{skip_if} ne "" && -e $spec->{skip_if}) {
        task_log($ctx, $name, "merged file $spec->{skip_if} already exists, skipping");
        return 0;
    }

    my $tar = _tar($ctx);
    my $wrap = _abs_wrapper($ctx, $spec->{wrapper});
    if (!$ctx->{dry_run}) {
        die "SlurmBatch: tar missing: $tar (run ./prompt init)\n" unless -f $tar;
        die "SlurmBatch: wrapper missing: $wrap\n" unless -f $wrap;
        die "SlurmBatch: slurm/run missing: $slurm\n" unless -f $slurm || -x $slurm;
    }

    my $dirs = prepare_dirs($ctx, $stage, $run);
    task_log($ctx, $name, "rundir = $dirs->{rundir}, storage = $dirs->{storage}");

    my $njobs = njobs(
        njobs => $spec->{njobs},
        total => $spec->{total},
        chunk => $spec->{chunk},
    );
    my $chunk = 0 + ($spec->{chunk} // 0);
    my $total = 0 + ($spec->{total} // 0);
    task_log($ctx, $name, "NJOBS: $njobs  total=$total chunk=$chunk");

    my @q = _queue_args($ctx->{slurm_queue} // "-p hourly");
    my @cmd0 = ((-x $slurm) ? ($slurm) : ($^X, $slurm), @q, "-t", $tar, "-c", $wrap);

    my %ids;
    my @jobs;
    for (my $i = 0; $i < $njobs; $i++) {
        my $job = "${run}_$i";
        my $skip = ($chunk > 0) ? $i * $chunk : 0;
        my @pairs = @{ $spec->{env} // [] };
        if ($spec->{job_env}) {
            my $extra = $spec->{job_env}->({
                i     => $i,
                njobs => $njobs,
                chunk => $chunk,
                total => $total,
                skip  => $skip,
                job   => $job,
                last  => ($i == $njobs - 1) ? 1 : 0,
            });
            push @pairs, _pairs($extra);
        }
        my $r = encode_replace(\@pairs);
        my @cmd = (@cmd0, "-r", $r, $job);
        my ($sid, $sout) = _submit_one($ctx, $name, $dirs->{rundir}, \@cmd);
        push @jobs, $job;
        if ($sid > 0) {
            my $logrel = "tmp-$job/" . ($sout ne "" ? $sout : $job) . ".slurm.log";
            $ids{$sid} = $logrel;
        }
    }

    if ($ctx->{dry_run}) {
        if ($spec->{merge}) {
            $spec->{merge}->($ctx, {
                name    => $name,
                run     => $run,
                srun    => sprintf("%05d", $run),
                jobs    => \@jobs,
                rundir  => $dirs->{rundir},
                storage => $dirs->{storage},
                storage_parent => $dirs->{storage_parent},
            });
        }
        return 0;
    }
    if (!keys %ids) {
        task_log($ctx, $name, "ERROR no slurm job submitted, something went wrong?!");
        return 2;
    }

    wait_jobs($ctx, $name, $dirs->{rundir}, \%ids);

    if ($spec->{merge}) {
        $spec->{merge}->($ctx, {
            name    => $name,
            run     => $run,
            srun    => sprintf("%05d", $run),
            jobs    => \@jobs,
            rundir  => $dirs->{rundir},
            storage => $dirs->{storage},
            storage_parent => $dirs->{storage_parent},
        });
    }
    return 0;
}

# ----------------------------------------------------------------------
sub hadd {
    my ($ctx, $name, $out, @in) = @_;
    die "SlurmBatch: hadd output empty\n" if !defined $out || $out eq "";
    die "SlurmBatch: hadd has no inputs\n" unless @in;
    make_path(dirname($out));
    unlink($out) if -e $out;
    my @cmd = ("hadd", $out, @in);
    task_log($ctx, $name, "@cmd");
    if ($ctx->{dry_run}) {
        return 1;
    }
    my $rc = system(@cmd);
    if ($rc != 0) {
        task_log($ctx, $name, "hadd failed (" . ($rc >> 8) . "): $out");
        return 0;
    }
    task_log($ctx, $name, "merged root file: $out");
    return 1;
}

1;
