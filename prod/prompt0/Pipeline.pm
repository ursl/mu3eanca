package Pipeline;

# ----------------------------------------------------------------------
# Pipeline
# ========
#
# Per-run task list for prompt0. Tasks are executable scripts in tasks/.
#
#   pipeline: beam
#   pipeline_alias: beam=midasmeta,skipSmallRuns,minalyzer,mu3esort,mu3etrirec
#
# CLI:  ./prompt [-P beam | -P t1,t2,...] run RUN [RUN ...]
#
# A task that exits non-zero stops the remaining tasks for that run
# (next run still starts). Exit 1 is a soft skip (e.g. skipSmallRuns);
# exit >= 2 is a hard failure (prompt exits non-zero at the end).
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use File::Basename qw(dirname);
use File::Path qw(make_path);
use Cwd qw(abs_path);
use POSIX qw(strftime);
use IO::Handle;

use Prompt qw(prompt_context prompt_repo_dir);
use TaskLib qw(task_prefix year_for_run expand_year raw_file);

our @EXPORT_OK = qw(
    pipeline_aliases
    pipeline_tasks
    pipeline_print
    pipeline_run
    pipeline_task_dir
);

# ----------------------------------------------------------------------
sub _strip {
    my ($s) = @_;
    return "" unless defined $s;
    $s =~ s/^\s+|\s+$//g;
    return $s;
}

# ----------------------------------------------------------------------
sub _stamp {
    return strftime("%y%m%d-%H%M%S", localtime);
}

# ----------------------------------------------------------------------
sub _log {
    print(task_prefix("pipeline"), @_, "\n");
}

# ----------------------------------------------------------------------
# Run a task script; stdout+stderr go to the parent's STDOUT (already teed).
# Returns the child exit code.
sub _run_task {
    my ($script, $run, $ctx) = @_;
    STDOUT->autoflush(1);

    my $pid = open(my $rh, "-|");
    die "pipeline: fork failed: $!\n" unless defined $pid;
    if ($pid == 0) {
        open STDERR, ">&STDOUT" or die "pipeline: dup STDERR: $!\n";
        exec($script, $run, $ctx) or die "pipeline: exec $script: $!\n";
    }
    my $buf;
    while (read($rh, $buf, 8192)) {
        print STDOUT $buf;
    }
    close $rh;
    return ($? == -1) ? 255 : ($? >> 8);
}

# ----------------------------------------------------------------------
sub pipeline_task_dir {
    my $here = abs_path(dirname(__FILE__));
    return "$here/tasks";
}

# ----------------------------------------------------------------------
# name => "t1,t2,..." from repeated pipeline_alias: name=t1,t2
# and optional pipeline_<name>: t1,t2 keys.
sub pipeline_aliases {
    my ($cfg) = @_;
    my %a;
    for my $k (sort keys %$cfg) {
        next unless $k =~ /^pipeline_(.+)$/;
        my $name = $1;
        next if $name eq "alias";
        next if ref($cfg->{$k});
        my $v = _strip($cfg->{$k});
        $a{$name} = $v if $v ne "";
    }
    my $raw = $cfg->{pipeline_alias};
    my @lines = ref($raw) eq "ARRAY" ? @$raw : (defined $raw ? ($raw) : ());
    for my $line (@lines) {
        $line = _strip($line);
        next if $line eq "";
        if ($line =~ /^([A-Za-z0-9_.-]+)\s*=\s*(.+)$/) {
            $a{$1} = _strip($2);
        } else {
            die "pipeline_alias must be name=task,task,... (got: $line)\n";
        }
    }
    return %a;
}

# ----------------------------------------------------------------------
sub pipeline_tasks {
    my ($cfg, $spec) = @_;
    $spec = _strip($spec // "");
    $spec = _strip($cfg->{pipeline} // "") if $spec eq "";
    die "pipeline: no pipeline (set pipeline: in the cfg or pass -P)\n"
        if $spec eq "";

    my %alias = pipeline_aliases($cfg);
    my $guard = 0;
    while ($spec !~ /,/ && exists $alias{$spec}) {
        $spec = $alias{$spec};
        die "pipeline: alias loop involving '$spec'\n" if ++$guard > 10;
    }

    my @tasks = grep { $_ ne "" } map { _strip($_) } split(/,/, $spec);
    die "pipeline: empty task list (from '$spec')\n" unless @tasks;
    return @tasks;
}

# ----------------------------------------------------------------------
# Filename token: alias (`beam`, `test`) or a comma-list turned into dashes.
sub pipeline_name {
    my ($cfg, $spec) = @_;
    $spec = _strip($spec // "");
    $spec = _strip($cfg->{pipeline} // "") if $spec eq "";
    $spec =~ s/,+/-/g;
    $spec =~ s/[^\w.-]+/_/g;
    return $spec ne "" ? $spec : "pipeline";
}

# ----------------------------------------------------------------------
sub pipeline_known_tasks {
    my $dir = pipeline_task_dir();
    my @ids;
    if (opendir my $dh, $dir) {
        @ids = sort grep {
            $_ !~ /^\./ && $_ ne "TaskLib.pm" && -f "$dir/$_" && -x "$dir/$_"
        } readdir $dh;
        closedir $dh;
    }
    return @ids;
}

# ----------------------------------------------------------------------
sub pipeline_print {
    my ($cfg) = @_;
    my %alias = pipeline_aliases($cfg);
    my $def = _strip($cfg->{pipeline} // "");
    print(task_prefix("prompt"), "pipelines\n");
    print("  default:     ", ($def ne "" ? $def : "(unset)"), "\n");
    if (%alias) {
        for my $name (sort keys %alias) {
            print("  alias $name: $alias{$name}\n");
        }
    } else {
        print("  aliases:     (none)\n");
    }
    my @known = pipeline_known_tasks();
    print("  task scripts:", (@known ? " " . join(" ", @known) : " (none in tasks/)"), "\n");
}

# ----------------------------------------------------------------------
sub _quote {
    my ($v) = @_;
    return "" unless defined $v;
    return $v if $v =~ /^[A-Za-z0-9_.:\/@+=,-]*$/;
    $v =~ s/"/\\"/g;
    return "\"$v\"";
}

# ----------------------------------------------------------------------
sub _write_ctx {
    my ($path, $kv) = @_;
    open my $fh, ">", $path or die "pipeline: cannot write $path: $!\n";
    for my $k (sort keys %$kv) {
        next unless defined $kv->{$k};
        next if ref($kv->{$k});
        print $fh "$k: " . _quote($kv->{$k}) . "\n";
    }
    close $fh;
}

# ----------------------------------------------------------------------
sub _ctx_kv {
    my ($cfg, $pctx, $run) = @_;
    my $srun = sprintf("%05d", $run);
    my $year = year_for_run($cfg, $run);
    my $data = expand_year(_strip($cfg->{data_dir} // ""), $year);
    my $raw  = _strip($cfg->{raw_input_base} // "");
    $raw = "$data/raw" if $raw eq "" && $data ne "";
    $raw = expand_year($raw, $year);
    my $workdir = $pctx->{workdir} // "";
    my %kv = (
        run              => $run,
        srun             => $srun,
        year             => $year,
        dry_run          => $pctx->{dry_run} ? 1 : 0,
        root             => $pctx->{root} // "",
        parent           => $pctx->{parent} // "",
        workdir          => $workdir,
        data_dir         => $data,
        raw_dir          => $raw,
        raw_input_layout => $pctx->{raw_input_layout} // "runblock3",
        mlzr_dir         => ($data ne "" && $workdir ne "") ? "$data/mlzr/$workdir" : "",
        trirec_dir       => ($data ne "" && $workdir ne "") ? "$data/trirec/$workdir" : "",
        mu3eanca         => $pctx->{mu3eanca} // "",
        prompt_dir       => abs_path(dirname(__FILE__)),
        slurm_run        => $pctx->{slurm_run} // "",
        slurm_queue      => $pctx->{slurm_queue} // "",
        cdb_dbconn       => $pctx->{cdb_dbconn} // "",
        cdb_GT           => $pctx->{cdb_GT} // "",
        rdb_url          => $pctx->{rdb_url} // "",
        n_events_chunk   => $pctx->{n_events} // 0,
        min_events       => $pctx->{min_events} // 0,
        run_block_max    => $pctx->{run_block_max} // 0,
        cmake_libdir     => _strip($cfg->{cmake_libdir} // "lib"),
        slurm_analyzer_csh => _strip($cfg->{slurm_analyzer_csh} // ""),
        slurm_sort_csh     => _strip($cfg->{slurm_sort_csh} // ""),
        slurm_trirec_csh   => _strip($cfg->{slurm_trirec_csh} // ""),
    );
    $kv{tar_file} = ($pctx->{root} ne "" && $workdir ne "")
        ? "$pctx->{root}/slurm/$workdir.tar.gz" : "";
    $kv{raw_file} = raw_file(\%kv, $run) if $raw ne "";
    for my $r (@{ $cfg->{setup_repos} // [] }) {
        next unless defined $r->{id} && $r->{id} ne "";
        $kv{ $r->{id} } = prompt_repo_dir($cfg, $r->{id});
    }
    return \%kv;
}

# ----------------------------------------------------------------------
# Returns number of hard failures (task exit >= 2).
sub pipeline_run {
    my ($cfg, %opts) = @_;
    my @runs = @{ $opts{runs} // [] };
    die "pipeline: no run numbers (./prompt -P beam run 12345)\n" unless @runs;

    my @tasks = pipeline_tasks($cfg, $opts{pipeline});
    my $pname = pipeline_name($cfg, $opts{pipeline});
    my $tdir  = pipeline_task_dir();
    my @known = pipeline_known_tasks();
    my %known = map { $_ => 1 } @known;
    for my $t (@tasks) {
        die "pipeline: unknown task '$t' (have: "
            . (@known ? join(", ", @known) : "none") . ")\n"
            unless $known{$t};
    }

    my $pctx = prompt_context($cfg, %opts);
    my $dry  = $pctx->{dry_run} ? 1 : 0;
    my $hard = 0;

    _log("pipeline: " . join(",", @tasks)
        . "  runs=" . join(",", @runs)
        . ($dry ? " (dry-run)" : ""));

    for my $run (@runs) {
        die "pipeline: bad run number '$run'\n" unless $run =~ /^\d+$/;
        my $kv   = _ctx_kv($cfg, $pctx, $run);
        my $rdir = "$pctx->{root}/runs/" . sprintf("%05d", $run);
        my $ctx  = "$rdir/ctx.cfg";
        my $stamp = _stamp();
        my $logfile = "$rdir/$stamp-$pname.log";
        $kv->{log_stamp} = $stamp;
        $kv->{log_dir}   = $rdir;
        $kv->{log_file}  = $logfile;
        $kv->{pipeline_name} = $pname;
        if (!$dry) {
            make_path($rdir);
            _write_ctx($ctx, $kv);
        }

        my $tied = 0;
        if ($dry) {
            _log("would log to $logfile");
        } else {
            open my $tty, ">&", \*STDOUT or die "pipeline: dup STDOUT: $!\n";
            $tty->autoflush(1);
            open my $runlog, ">", $logfile or die "pipeline: cannot write $logfile: $!\n";
            $runlog->autoflush(1);
            print $runlog $opts{banner}, "\n" if defined $opts{banner} && $opts{banner} ne "";
            print $runlog task_prefix("pipeline"), "log $logfile\n";
            tie *STDOUT, "Pipeline::Tee", $tty, $runlog
                or die "pipeline: tie STDOUT: $!\n";
            $tied = 1;
        }

        my $err = "";
        eval {
            _log("run $run year=$kv->{year}  " . join(" -> ", @tasks)
                . ($kv->{raw_file} ? "  raw=$kv->{raw_file}" : ""));
            my $stopped = 0;
            for my $t (@tasks) {
                my $script = "$tdir/$t";
                if ($dry) {
                    _log("[$t] would: $script $run $ctx");
                    next;
                }
                _log("[$t] start");
                my $exit = _run_task($script, $run, $ctx);
                if ($exit == 0) {
                    _log("[$t] ok");
                    next;
                }
                $stopped = 1;
                if ($exit == 1) {
                    _log("[$t] skip (exit 1); remaining tasks not run for run $run");
                } else {
                    _log("[$t] FAIL (exit $exit); remaining tasks not run for run $run");
                    $hard++;
                }
                last;
            }
            _log("run $run " . ($stopped ? "stopped" : "done"));
            1;
        } or do { $err = $@ || "pipeline: run $run failed\n"; };
        untie *STDOUT if $tied;
        die $err if $err;
    }
    return $hard;
}

1;

# ----------------------------------------------------------------------
# Tee parent-process prints (and task output) to the terminal and one run log.
package Pipeline::Tee;
use strict;
use warnings;

sub TIEHANDLE {
    my ($class, $real, $log) = @_;
    bless { real => $real, log => $log }, $class;
}

sub PRINT {
    my $self = shift;
    my $ok1 = print { $self->{real} } @_;
    my $ok2 = print { $self->{log} } @_;
    return $ok1 && $ok2;
}

sub PRINTF {
    my $self = shift;
    my $fmt  = shift;
    return $self->PRINT(sprintf($fmt, @_));
}

1;
