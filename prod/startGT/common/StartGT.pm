package StartGT;

# ----------------------------------------------------------------------
# StartGT
# =======
#
# MU3E checkout + one mcideal mu3eSim run → single alignment/geometry ROOT.
# All GT flavours (mcideal, mcrealistic, data) read that file; installed-
# component selection is done in cdbRunPayloadWriter (-m 2025/2026/all).
#
# Config + host overlay: RelvalConfig; git build: Setup (../relval/common).
#
# History
#         2026/08/11 first shot
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Cwd qw(getcwd abs_path);
use File::Path qw(make_path);

our @EXPORT_OK = qw(
    startgt_context
    startgt_gt_flavour
    startgt_alignment_root
    startgt_setup
    startgt_run_mu3esim
    startgt_ensure_test_cdb
    startgt_run_alignment_payloads
    startgt_status
);

my @TEST_CDB_SUBDIRS = qw(payloads globaltags tags runrecords configs);

# ----------------------------------------------------------------------
sub _strip {
    my ($s) = @_;
    return "" unless defined $s;
    $s =~ s/^\s+|\s+$//g;
    return $s;
}

# ----------------------------------------------------------------------
sub _prefix {
    my $now = localtime;
    return "$now/startGT        / ";
}

# ----------------------------------------------------------------------
sub _log {
    my ($ctx, @msg) = @_;
    print(_prefix(), @msg, "\n");
}

# ----------------------------------------------------------------------
sub _run {
    my ($ctx, @cmd) = @_;
    _log($ctx, "cmd: @cmd");
    return if $ctx->{dry_run};
    system(@cmd);
    die "Command failed: @cmd\n" if $? != 0;
}

# ----------------------------------------------------------------------
sub _cfg_key {
    my ($cfg, @keys) = @_;
    for my $k (@keys) {
        my $v = _strip($cfg->{$k} // "");
        return $v if $v ne "";
    }
    return "";
}

# ----------------------------------------------------------------------
sub startgt_workdir {
    my ($cfg) = @_;
    my $wd = _strip($cfg->{mu3e_workdir} // "");
    if ($wd eq "") {
        my $tag = _strip($cfg->{mu3e_tag} // "mu3e");
        $wd = "mu3e-$tag";
    }
    return $wd;
}

# ----------------------------------------------------------------------
sub startgt_conditions_year {
    my ($cfg) = @_;
    return _strip($cfg->{conditions_year} // "");
}

# ----------------------------------------------------------------------
sub startgt_gt_flavour {
    my ($gt) = @_;
    $gt = lc(_strip($gt));
    return "mcideal"     if $gt =~ /^mcideal/;
    return "mcrealistic" if $gt =~ /^mcrealistic/;
    return "data"        if $gt =~ /^data/;
    die "startGT: cannot infer flavour from GT '$gt' (expect mcideal*, mcrealistic*, or data*)\n";
}

# ----------------------------------------------------------------------
sub startgt_year_from_gt {
    my ($gt, $cfg) = @_;
    if ($gt =~ /=((20\d{2}))/ ) {
        return $1;
    }
    return startgt_conditions_year($cfg);
}

# ----------------------------------------------------------------------
sub startgt_test_cdb_root {
    my ($ctx, $cfg) = @_;
    my $rel = _strip($cfg->{test_cdb_dir} // "output/test-CDB");
    return ($rel =~ m{^/}) ? $rel : "$ctx->{run_dir}/$rel";
}

# ----------------------------------------------------------------------
sub startgt_cdb_code_basedir {
    my ($cfg) = @_;
    my $base = _strip($cfg->{CDB_code_basedir} // "");
    die "startGT: CDB_code_basedir required in host overlay\n" if $base eq "";
    return $base;
}

# ----------------------------------------------------------------------
sub startgt_cdb_writer_exe {
    my ($cfg) = @_;
    return startgt_cdb_code_basedir($cfg) . "/bin/cdbRunPayloadWriter";
}

# ----------------------------------------------------------------------
# Single alignment ROOT for all GTs (from one mcideal mu3eSim run).
sub startgt_alignment_rel {
    my ($cfg) = @_;
    my $out = _cfg_key($cfg, "alignment_output", "alignment_output_mcideal");
    if ($out eq "") {
        my $tag = _strip($cfg->{mu3e_tag} // "unknown");
        $out = "output/mu3e_alignment_$tag.root";
    }
    return $out;
}

# ----------------------------------------------------------------------
sub startgt_alignment_root {
    my ($cfg) = @_;
    my $ctx = startgt_context($cfg);
    my $rel = startgt_alignment_rel($cfg);
    return ($rel =~ m{^/}) ? $rel : "$ctx->{run_dir}/$rel";
}

# ----------------------------------------------------------------------
sub startgt_payload_filter_mode {
    my ($cfg, $gt) = @_;
    my $flavour = startgt_gt_flavour($gt);
    return "all" if $flavour eq "mcideal";
    my $year = startgt_year_from_gt($gt, $cfg);
    die "startGT: need conditions year for '$gt' (=YYYY in name or conditions_year in config)\n"
        if $year eq "";
    return $year;
}

# ----------------------------------------------------------------------
sub startgt_context {
    my ($cfg, %opts) = @_;
    require Setup;
    my $basedir = Setup::setup_basedir($cfg);
    my $workdir = startgt_workdir($cfg);
    my $mu3e_dir = "$basedir/$workdir";

    return {
        cfg              => $cfg,
        basedir          => $basedir,
        workdir          => $workdir,
        mu3e_dir         => $mu3e_dir,
        build_dir        => "$mu3e_dir/_build",
        run_dir          => "$mu3e_dir/run",
        mu3e_tag         => _strip($cfg->{mu3e_tag} // ""),
        conditions_year  => startgt_conditions_year($cfg),
        sim_run_id       => 0 + (_strip($cfg->{sim_run_id} // $cfg->{run_id} // "1")),
        sim_n_events     => 0 + (_strip($cfg->{sim_n_events} // $cfg->{n_events} // "1")),
        dry_run          => $opts{dry_run} // 0,
    };
}

# ----------------------------------------------------------------------
sub _ensure_run_dir {
    my ($ctx) = @_;
    die "startGT: MU3E workdir missing: $ctx->{mu3e_dir} (run ./startGT setup first)\n"
        unless $ctx->{dry_run} || -d $ctx->{mu3e_dir};
    die "startGT: MU3E build missing: $ctx->{build_dir} (run ./startGT setup first)\n"
        unless $ctx->{dry_run} || -d $ctx->{build_dir};
    make_path("$ctx->{run_dir}/output") unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
sub startgt_setup {
    my ($cfg, %opts) = @_;
    require Setup;
    Setup::setup_run_config($cfg, %opts);
}

# ----------------------------------------------------------------------
sub startgt_run_mu3esim {
    my ($cfg, %opts) = @_;
    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $conf = _cfg_key($cfg, "sim_conf_mcideal", "sim_conf");
    die "startGT: sim_conf_mcideal (or sim_conf) required\n" if $conf eq "";
    die "startGT: sim conf missing: $ctx->{run_dir}/$conf\n"
        unless $ctx->{dry_run} || -f "$ctx->{run_dir}/$conf";

    my $out     = startgt_alignment_rel($cfg);
    my $out_abs = ($out =~ m{^/}) ? $out : "$ctx->{run_dir}/$out";
    my $out_dir = $out_abs;
    $out_dir =~ s{/[^/]+$}{};
    make_path($out_dir) unless $ctx->{dry_run};

    my $exe = "$ctx->{build_dir}/mu3eSim/mu3eSim";
    die "startGT: mu3eSim missing: $exe (run ./startGT setup first)\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    _log($ctx, "mu3eSim mcideal baseline geometry");
    _log($ctx, "  conf:     $conf");
    _log($ctx, "  output:   $out_abs");

    my $cwd = getcwd();
    chdir($ctx->{run_dir}) or die "Cannot chdir $ctx->{run_dir}: $!\n" unless $ctx->{dry_run};
    _run(
        $ctx, $exe,
        "--run", $ctx->{sim_run_id},
        "-n",    $ctx->{sim_n_events},
        "--conf", $conf,
        "--output", $out,
    );
    chdir($cwd) unless $ctx->{dry_run};

    unless ($ctx->{dry_run}) {
        die "startGT: alignment output missing: $out_abs\n" unless -f $out_abs;
        _log($ctx, "alignment root ready: $out_abs");
    }
}

# ----------------------------------------------------------------------
sub startgt_ensure_test_cdb {
    my ($ctx, $cfg) = @_;
    my $root = startgt_test_cdb_root($ctx, $cfg);
    _log($ctx, "test-CDB root: $root");
    for my $sub (@TEST_CDB_SUBDIRS) {
        make_path("$root/$sub") unless $ctx->{dry_run};
    }
    return $root;
}

# ----------------------------------------------------------------------
sub startgt_run_alignment_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g GT required (e.g. -g mcidealv7.1 or -g 'datav7.1=2026V0')\n" if $gt eq "";

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $align = startgt_alignment_root($cfg);
    die "startGT: alignment ROOT missing: $align (run ./startGT sim first)\n"
        unless $ctx->{dry_run} || -f $align;

    my $cdb_root    = startgt_ensure_test_cdb($ctx, $cfg);
    my $payload_dir = "$cdb_root/payloads";
    my $exe         = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $filter = startgt_payload_filter_mode($cfg, $gt);
    my $ann    = _strip($opts{annotation} // $cfg->{alignment_annotation} // "ideal geometry");

    my $cdb_dir = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter -c alignment");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT (-t):   $gt");
    _log($ctx, "  filter:    $filter");
    _log($ctx, "  input:     $align");
    _log($ctx, "  payloads:  $payload_dir");

    my @cmd = ($exe, "-c", "alignment", "-f", $align, "-a", $ann, "-p", $payload_dir, "-t", $gt);
    push @cmd, "-m", $filter unless $filter eq "all";

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};
    _run($ctx, @cmd);
    chdir($cwd) unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
sub startgt_status {
    my ($cfg, %opts) = @_;
    my $ctx = startgt_context($cfg, %opts);
    my $exe = "$ctx->{build_dir}/mu3eSim/mu3eSim";
    my $cdb_root = startgt_test_cdb_root($ctx, $cfg);
    my $align = startgt_alignment_root($cfg);
    my $conf  = _cfg_key($cfg, "sim_conf_mcideal", "sim_conf");

    print(_prefix(), "status");
    print(" GT=", ($opts{gt} // "")) if defined $opts{gt} && $opts{gt} ne "";
    print("\n");
    print("  setup_basedir:     $ctx->{basedir}\n");
    print("  mu3e_workdir:      $ctx->{workdir}\n");
    print("  mu3e_tag:          $ctx->{mu3e_tag}\n");
    print("  conditions_year:   ", ($ctx->{conditions_year} ne "" ? $ctx->{conditions_year} : "(unset)"), "\n");
    print("  test-CDB:          $cdb_root\n");
    print("  mu3e checkout:     ", (-d $ctx->{mu3e_dir} ? "yes" : "no"), "\n");
    print("  build dir:         ", (-d $ctx->{build_dir} ? "yes" : "no"), "\n");
    print("  mu3eSim:           ", (-x $exe || -f $exe ? "yes" : "no"), "\n");
    print("  run dir:           $ctx->{run_dir}\n");
    print("  sim_conf:          $conf\n");
    print("  alignment ROOT:    $align (", (-f $align ? "present" : "missing"), ")\n");

    if (defined $opts{gt} && $opts{gt} ne "") {
        my $filter = startgt_payload_filter_mode($cfg, $opts{gt});
        print("  GT flavour:        ", startgt_gt_flavour($opts{gt}), "\n");
        print("  payload filter -m: $filter\n");
        print("  payloads dir:      $cdb_root/payloads (",
            (-d "$cdb_root/payloads" ? "yes" : "no"), ")\n");
    }

    require Setup;
    Setup::setup_status_config($cfg, %opts);
}

1;
