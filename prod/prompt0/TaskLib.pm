package TaskLib;

# ----------------------------------------------------------------------
# TaskLib — tiny helpers for prompt0/tasks/* scripts.
#
#   my ($run, $ctxfile) = TaskLib::args(@ARGV);
#   my $ctx = TaskLib::load($ctxfile);
#   TaskLib::task_log($ctx, "skipSmallRuns", "events=$n");
#   TaskLib::ctx_set($ctxfile, n_events => $n);
#
# Exit convention (honoured by Pipeline.pm):
#   0  continue with the next task
#   1  soft skip: stop remaining tasks for this run (e.g. too few events)
#   2+ hard failure: stop remaining tasks; prompt exits non-zero
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);

our @EXPORT_OK = qw(
    args load task_log ctx_set
    task_prefix
    block_dir raw_file
    rdb_events
    parse_run_years year_for_run expand_year
    expand_ctx resolve_path
);

# ----------------------------------------------------------------------
sub _strip {
    my ($s) = @_;
    return "" unless defined $s;
    $s =~ s/^\s+|\s+$//g;
    $s =~ s/^"(.*)"$/$1/;
    $s =~ s/^'(.*)'$/$1/;
    return $s;
}

# ----------------------------------------------------------------------
sub task_prefix {
    my ($name) = @_;
    $name = "task" unless defined $name && $name ne "";
    $name .= "               ";
    $name = substr($name, 0, 15);
    my $now = localtime;
    return "$now/$name / ";
}

# ----------------------------------------------------------------------
sub args {
    my @a = @_;
    my $run = $a[0];
    my $ctx = $a[1];
    die "Usage: $0 RUN CTX.cfg\n" unless defined $run && $run =~ /^\d+$/;
    die "Usage: $0 RUN CTX.cfg  (missing ctx file)\n"
        unless defined $ctx && $ctx ne "";
    return ($run, $ctx);
}

# ----------------------------------------------------------------------
sub load {
    my ($path) = @_;
    die "TaskLib: ctx not found: $path\n" unless -f $path;
    my %kv;
    open my $fh, "<", $path or die "TaskLib: cannot read $path: $!\n";
    while (my $line = <$fh>) {
        chomp $line;
        $line =~ s/\r$//;
        next if $line =~ /^\s*#/;
        next if $line =~ /^\s*$/;
        if ($line =~ /^\s*([A-Za-z0-9_.-]+)\s*:\s*(.*)$/) {
            $kv{$1} = _strip($2);
        }
    }
    close $fh;
    return \%kv;
}

# ----------------------------------------------------------------------
sub ctx_set {
    my ($path, %pairs) = @_;
    open my $fh, ">>", $path or die "TaskLib: cannot append $path: $!\n";
    for my $k (sort keys %pairs) {
        my $v = defined $pairs{$k} ? $pairs{$k} : "";
        $v =~ s/"/\\"/g;
        print $fh "$k: $v\n";
    }
    close $fh;
}

# ----------------------------------------------------------------------
sub task_log {
    my ($ctx, $name, @msg) = @_;
    $name = $ctx->{id} if (!defined $name || $name eq "") && ref($ctx) eq "HASH";
    print(task_prefix($name), @msg, "\n");
}

# ----------------------------------------------------------------------
# processRuns getBlock: run/1000 as 3-digit directory.
sub block_dir {
    my ($run) = @_;
    return sprintf("%03d", int($run / 1000));
}

# ----------------------------------------------------------------------
sub raw_file {
    my ($ctx, $run) = @_;
    $run = $ctx->{run} unless defined $run;
    my $base = $ctx->{raw_dir} // "";
    die "TaskLib: raw_dir empty in ctx\n" if $base eq "";
    my $srun = sprintf("run%05d", $run);
    my $layout = $ctx->{raw_input_layout} // "runblock3";
    if ($layout eq "flat") {
        return "$base/$srun.mid.lz4";
    }
    return "$base/" . block_dir($run) . "/$srun.mid.lz4";
}

# ----------------------------------------------------------------------
# Run → calendar year.
#
#   run_year: 9410=2025    # inclusive last run of 2025
#   run_year: 2026         # open (current) year: 9411 and up, so far
#
# When 2027 starts, add `run_year: NNNN=2026` and change the open year.
sub parse_run_years {
    my ($cfg) = @_;
    my $raw = (ref($cfg) eq "HASH") ? $cfg->{run_year} : undef;
    my @lines = ref($raw) eq "ARRAY" ? @$raw : (defined $raw ? ($raw) : ());
    my @bounds;
    my $open = "";
    for my $line (@lines) {
        $line = _strip($line);
        next if $line eq "";
        if ($line =~ /^(\d+)\s*=\s*(\d{4})$/) {
            push @bounds, [ 0 + $1, $2 ];
        } elsif ($line =~ /^(\d{4})$/) {
            $open = $1;
        } else {
            die "run_year must be LAST=YYYY or YYYY (got: $line)\n";
        }
    }
    @bounds = sort { $a->[0] <=> $b->[0] } @bounds;
    return { bounds => \@bounds, open => $open };
}

# ----------------------------------------------------------------------
# Year for a run, or the open/current year if $run is undef.
sub year_for_run {
    my ($cfg, $run) = @_;
    my $p = parse_run_years($cfg);
    if (!defined $run || $run eq "") {
        return $p->{open} if $p->{open} ne "";
        return @{ $p->{bounds} } ? $p->{bounds}[-1][1] : "";
    }
    die "TaskLib: bad run number '$run'\n" unless $run =~ /^\d+$/;
    for my $b (@{ $p->{bounds} }) {
        return $b->[1] if $run <= $b->[0];
    }
    return $p->{open} if $p->{open} ne "";
    die "TaskLib: no year for run $run (set run_year: in the version cfg)\n";
}

# ----------------------------------------------------------------------
sub expand_year {
    my ($text, $year) = @_;
    return $text unless defined $text && $text =~ /\{year\}/;
    die "TaskLib: {year} in path but year is unset\n"
        if !defined $year || $year eq "";
    $text =~ s/\{year\}/$year/g;
    return $text;
}

# ----------------------------------------------------------------------
# {key} from ctx (e.g. {mu3eana} {root} {year}). Unknown keys stay as {key}.
sub expand_ctx {
    my ($ctx, $text) = @_;
    return "" unless defined $text;
    $text =~ s/\{([A-Za-z0-9_]+)\}/
        (ref($ctx) eq "HASH" && defined $ctx->{$1} && $ctx->{$1} ne "")
            ? $ctx->{$1} : "{$1}"
    /ge;
    return $text;
}

# ----------------------------------------------------------------------
# Cwd / binary paths: never dirname(exe). Absolute after expand; empty ->
# $base; relative -> $base/$spec.
sub resolve_path {
    my ($ctx, $spec, $base) = @_;
    $spec = expand_ctx($ctx, defined $spec ? $spec : "");
    $base = expand_ctx($ctx, defined $base ? $base : "");
    return $base if $spec eq "" || $spec eq ".";
    return $spec if $spec =~ m{^/};
    return ($base ne "") ? "$base/$spec" : $spec;
}

# ----------------------------------------------------------------------
# RDB REST (processRuns countEvents). Returns 0 if the JSON has no Events.
sub rdb_events {
    my ($ctx, $run) = @_;
    my $base = $ctx->{rdb_url} // "";
    die "TaskLib: rdb_url empty in ctx\n" if $base eq "";
    $base =~ s{/+$}{};
    my $url = "$base/run/$run";
    task_log($ctx, "countEvents", "url = $url");
    my $json = `curl -s --max-time 30 "$url"`;
    my $rc = $? >> 8;
    die "TaskLib: curl failed ($rc) for $url\n" if $rc != 0 && $json eq "";
    if ($json =~ /"Events"\s*:\s*(\d+)/) {
        return 0 + $1;
    }
    return 0;
}

1;
