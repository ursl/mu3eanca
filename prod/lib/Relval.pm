package Relval;

# ----------------------------------------------------------------------
# Relval
# ======
#
# Run release-validation tasks against an existing MU3E setup workdir.
#
# Per-task pipeline:
#   sim:  mu3eSim -> mu3eSort -> mu3eTrirec -> fillhist -> [treedump]
#   data: raw MID -> mu3eSort -> mu3eTrirec -> fillhist -> [treedump]
#
# Compare / histocompare: target compare (see relval_run_compare).
#
# History
#         2026/07/14 first shot
#         2026/07/14 add compare against reference setup
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Cwd qw(abs_path getcwd);
use File::Basename qw(dirname);
use File::Path qw(make_path);
use File::Spec;

our @EXPORT_OK = qw(
    relval_context
    relval_run_all
    relval_run_task
    relval_run_compare
    relval_fmt_tpl
    relval_ref_workdir
);

my @ALIGN_OBJECTS = qw(sensors fibres tiles mppcs);

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
    return "$now/Relval         / ";
}

# ----------------------------------------------------------------------
sub _log {
    print(_prefix(), @_, "\n");
}

# ----------------------------------------------------------------------
sub _truthy {
    my ($v) = @_;
    return 0 unless defined $v;
    $v = lc(_strip($v));
    return 0 if $v eq "" || $v eq "0" || $v eq "false" || $v eq "no" || $v eq "off";
    return 1;
}

# ----------------------------------------------------------------------
sub _run {
    my ($ctx, @cmd) = @_;
    _log("cmd: @cmd");
    return if $ctx->{dry_run};
    system(@cmd);
    if ($? != 0) {
        die "Command failed: @cmd\n";
    }
}

# ----------------------------------------------------------------------
sub relval_fmt_tpl {
    my ($tpl, $task_id) = @_;
    my $out = $tpl // "";
    $out =~ s/\{task\}/$task_id/g;
    $out =~ s/\{scenario\}/$task_id/g;
    return $out;
}

# ----------------------------------------------------------------------
sub _runblock_dir {
    my ($run_id, $layout) = @_;
    my $block = int($run_id / 1000);
    return sprintf("%03d", $block) if ($layout // "") eq "runblock3";
    return sprintf("%04d", $block) if ($layout // "") eq "runblock4";
    return "";
}

# ----------------------------------------------------------------------
sub _raw_input_path {
    my ($ctx, $task) = @_;
    my $path = _strip($task->{raw_input} // $task->{sort_input} // "");
    die "Relval: task $task->{id}: raw_input required for mode data\n" if $path eq "";
    return $path if $path =~ m{^/};

    my $base = _strip($ctx->{raw_input_base} // "");
    return $path if $base eq "";

    my $layout = _strip($task->{raw_input_layout} // $ctx->{raw_input_layout} // "flat");
    my $rb = _runblock_dir($task->{run_id}, $layout);
    my $prefix = "";
    if ($rb ne "" && $path !~ /^\Q$rb\E\//) {
        $prefix = "$rb/";
    }
    return "$base/$prefix$path";
}

# ----------------------------------------------------------------------
sub _treedump_enabled {
    my ($task) = @_;
    if (defined $task->{treedump} && _strip($task->{treedump}) ne "") {
        return _truthy($task->{treedump});
    }
    return ($task->{mode} // "") eq "sim" ? 1 : 0;
}

# ----------------------------------------------------------------------
# Resolve basedir + MU3E workdir used for running tasks.
sub relval_context {
    my ($cfg, %opts) = @_;

    my $basedir = _strip($cfg->{setup_basedir} // $cfg->{mu3e_relval_basedir} // "");
    die "Relval: setup_basedir (or mu3e_relval_basedir) required\n" if $basedir eq "";

    my $workdir = _strip($opts{setup_name} // $cfg->{mu3e_workdir} // $cfg->{mu3e_workdir_name} // "");
    if ($workdir eq "") {
        my $tag = _strip($cfg->{mu3e_tag} // "");
        die "Relval: set mu3e_workdir (e.g. mu3e-v6.9pre0) or mu3e_tag in config\n"
            if $tag eq "";
        $tag =~ s/[\/\s]/_/g;
        $workdir = "mu3e-$tag";
    }
    $workdir =~ s/[\/\s]+/_/g;

    my $mu3e_dir = "$basedir/$workdir";
    my $run_dir  = "$mu3e_dir/run";
    my $build    = "$mu3e_dir/_build";

    my $code_base = _strip($cfg->{relval_code_basedir} // "");
    if ($code_base eq "") {
        # prod/ when running ./relval from there
        $code_base = abs_path(dirname($0));
    }
    my $ana_base = _strip($cfg->{ana_basedir} // "");
    if ($ana_base eq "") {
        $ana_base = "$code_base/ana";
        die "Relval: ana_basedir missing: $ana_base\n" unless -d $ana_base;
    }

    my $mu3e_tag = _strip($cfg->{mu3e_tag} // "");
    if ($mu3e_tag eq "" && $workdir =~ /^mu3e-(.+)$/) {
        $mu3e_tag = $1;
    }

    my %ctx = (
        cfg              => $cfg,
        basedir          => $basedir,
        workdir          => $workdir,
        mu3e_dir         => $mu3e_dir,
        run_dir          => $run_dir,
        build_dir        => $build,
        mu3e_tag         => $mu3e_tag,
        cdb_dbconn       => _strip($cfg->{cdb_dbconn} // $cfg->{cdb_db_conn} // ""),
        raw_input_base   => _strip($cfg->{raw_input_base} // ""),
        raw_input_layout => _strip($cfg->{raw_input_layout} // "flat"),
        sim_tpl          => $cfg->{sim_output_tpl} // "output/sim-{task}.root",
        sort_tpl         => $cfg->{sort_output_tpl} // "output/sort-{task}.root",
        trirec_tpl       => $cfg->{trirec_output_tpl} // "output/trirec-{task}.root",
        fillhist_tpl     => $cfg->{fillhist_output_tpl} // "output/histograms-{task}.root",
        fillhist_exe     => "$ana_base/bin/runFillHistograms",
        compare_exe      => "$ana_base/bin/runCompareHistograms",
        compare_template => "$ana_base/template-relval.tex",
        util_dir         => _strip($cfg->{mu3eUtil_workdir} // "mu3eUtil-dev"),
        validation_dir   => _strip($cfg->{mu3eValidation_workdir} // "mu3eValidation-master"),
        dry_run          => $opts{dry_run} // 0,
    );
    $ctx{treedumper_exe} = "$basedir/$ctx{util_dir}/_build/tools/treedump/mu3eTreeDumper";
    $ctx{treedump_cfg}   = "$basedir/$ctx{validation_dir}/scripts/treedump_and_histocompare/config.json";

    return \%ctx;
}

# ----------------------------------------------------------------------
sub _ensure_run_dir {
    my ($ctx) = @_;
    my $mu3e = $ctx->{mu3e_dir};
    die "Relval: MU3E workdir missing: $mu3e (run setup first)\n"
        unless $ctx->{dry_run} || -d $mu3e;
    die "Relval: MU3E build missing: $ctx->{build_dir} (run setup/build first)\n"
        unless $ctx->{dry_run} || -d $ctx->{build_dir};
    make_path("$ctx->{run_dir}/output") unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
sub _run_sim {
    my ($ctx, $task) = @_;
    my $id  = $task->{id};
    my $out = relval_fmt_tpl($ctx->{sim_tpl}, $id);
    my $exe = "$ctx->{build_dir}/mu3eSim/mu3eSim";
    my $cwd = getcwd();
    chdir($ctx->{run_dir}) or die "Cannot chdir $ctx->{run_dir}: $!\n" unless $ctx->{dry_run};
    _run(
        $ctx, $exe,
        "--run", $task->{run_id},
        "-n", $task->{n_events},
        "--conf", $task->{sim_conf},
        "--output", $out,
    );
    chdir($cwd) unless $ctx->{dry_run};
    return $out;
}

# ----------------------------------------------------------------------
sub _run_sort {
    my ($ctx, $task, $input) = @_;
    my $id  = $task->{id};
    my $out = relval_fmt_tpl($ctx->{sort_tpl}, $id);
    my $exe = "$ctx->{build_dir}/mu3eSim/sort/mu3eSort";
    my $gt  = $task->{cdb_GT};
    die "Relval: task $id: cdb_dbconn required\n" if $ctx->{cdb_dbconn} eq "";

    if (!$ctx->{dry_run} && $input =~ m{^/} && !-f $input) {
        die "Relval: sort input missing: $input\n";
    }

    my $cwd = getcwd();
    chdir($ctx->{run_dir}) or die "Cannot chdir $ctx->{run_dir}: $!\n" unless $ctx->{dry_run};
    _run(
        $ctx, $exe,
        $input,
        "--output", $out,
        "--run", $task->{run_id},
        "--cdb.dbconn=$ctx->{cdb_dbconn}",
        "--cdb.globalTag=$gt",
    );
    chdir($cwd) unless $ctx->{dry_run};
    return $out;
}

# ----------------------------------------------------------------------
sub _run_trirec {
    my ($ctx, $task, $sort_rel) = @_;
    my $id  = $task->{id};
    my $out = relval_fmt_tpl($ctx->{trirec_tpl}, $id);
    my $exe = "$ctx->{build_dir}/mu3eTrirec/mu3eTrirec";
    my $conf = $task->{trirec_conf};
    my $fallback = _strip($task->{trirec_conf_fallback} // "");
    my $gt = $task->{cdb_GT};

    if (!$ctx->{dry_run}) {
        if (!-f "$ctx->{run_dir}/$conf") {
            if ($fallback ne "" && -f "$ctx->{run_dir}/$fallback") {
                _log("task $id: trirec conf '$conf' missing, using fallback '$fallback'");
                $conf = $fallback;
            } else {
                die "Relval: task $id: trirec conf '$conf' not found under $ctx->{run_dir}\n";
            }
        }
        die "Relval: task $id: sort input missing: $ctx->{run_dir}/$sort_rel\n"
            unless -f "$ctx->{run_dir}/$sort_rel";
    }

    my $cwd = getcwd();
    chdir($ctx->{run_dir}) or die "Cannot chdir $ctx->{run_dir}: $!\n" unless $ctx->{dry_run};
    _run(
        $ctx, $exe,
        $sort_rel,
        "--conf", $conf,
        "--output", $out,
        "--run", $task->{run_id},
        "--cdb.dbconn=$ctx->{cdb_dbconn}",
        "--cdb.globalTag=$gt",
    );
    chdir($cwd) unless $ctx->{dry_run};
    return $out;
}

# ----------------------------------------------------------------------
sub _run_fillhist {
    my ($ctx, $task, $trirec_rel) = @_;
    my $id  = $task->{id};
    my $out = relval_fmt_tpl($ctx->{fillhist_tpl}, $id);
    my $exe = $ctx->{fillhist_exe};
    die "Relval: fillhist exe missing: $exe (build snakemake/relval/ana)\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $ann = "mu3e-$ctx->{mu3e_tag}_$task->{cdb_GT}";
    my $in  = "$ctx->{mu3e_dir}/run/$trirec_rel";
    my $out_path = "$ctx->{mu3e_dir}/run/$out";

    # runFillHistograms --out is relative to cwd in snakemake as run/...;
    # pass absolute to be robust.
    _run(
        $ctx, $exe,
        "--in", $in,
        "--ann", $ann,
        "--out", $out_path,
    );
    return $out;
}

# ----------------------------------------------------------------------
sub _run_treedump {
    my ($ctx, $task, $trirec_rel) = @_;
    my $id = $task->{id};
    my $exe = $ctx->{treedumper_exe};
    my $cfg = $ctx->{treedump_cfg};
    die "Relval: treedumper missing: $exe\n" unless $ctx->{dry_run} || -f $exe;
    die "Relval: treedump config missing: $cfg\n" unless $ctx->{dry_run} || -f $cfg;

    my $in = "$ctx->{mu3e_dir}/run/$trirec_rel";
    make_path("$ctx->{run_dir}/output") unless $ctx->{dry_run};

    for my $obj (@ALIGN_OBJECTS) {
        my $prefix = "$ctx->{run_dir}/output/treedump-$id-$obj";
        _run(
            $ctx, $exe,
            $cfg,
            $in,
            "alignment/$obj",
            $prefix,
        );
    }
}

# ----------------------------------------------------------------------
sub relval_run_task {
    my ($ctx, $task) = @_;
    my $id   = $task->{id};
    my $mode = $task->{mode};
    _log("task $id mode=$mode start");

    my $sort_rel;
    if ($mode eq "sim") {
        my $sim_rel = _run_sim($ctx, $task);
        $sort_rel = _run_sort($ctx, $task, $sim_rel);
    } elsif ($mode eq "data") {
        my $raw = _raw_input_path($ctx, $task);
        $sort_rel = _run_sort($ctx, $task, $raw);
    } else {
        die "Relval: unknown mode '$mode' for task $id\n";
    }

    my $trirec_rel = _run_trirec($ctx, $task, $sort_rel);
    _run_fillhist($ctx, $task, $trirec_rel);

    if (_treedump_enabled($task)) {
        _run_treedump($ctx, $task, $trirec_rel);
    }

    _log("task $id done");
}

# ----------------------------------------------------------------------
# Resolve reference workdir under basedir.
# Accepts "v6.5", "mu3e-v6.5", or a bare directory name that exists.
sub relval_ref_workdir {
    my ($cfg, $basedir) = @_;
    my $ref = _strip($cfg->{compare_against_setup} // "");
    die "Relval: compare_against_setup required (e.g. mu3e-v6.5 or v6.5)\n"
        if $ref eq "";

    return $ref if $ref =~ m{^/};
    return $ref if -d "$basedir/$ref";
    return $ref if $ref =~ /^mu3e-/;
    my $prefixed = "mu3e-$ref";
    return $prefixed if -d "$basedir/$prefixed" || 1;
    return $prefixed;
}

# ----------------------------------------------------------------------
sub _compare_dir_name {
    my ($ctx, $task_id, $ref_workdir) = @_;
    return $task_id . "__" . $ctx->{workdir} . "__vs__" . $ref_workdir;
}

# ----------------------------------------------------------------------
sub _run_compare_task {
    my ($ctx, $task, $ref_workdir) = @_;
    my $id = $task->{id};
    my $hist_rel = relval_fmt_tpl($ctx->{fillhist_tpl}, $id);
    my $in1 = "$ctx->{mu3e_dir}/run/$hist_rel";
    my $in2 = "$ctx->{basedir}/$ref_workdir/run/$hist_rel";
    my $out_dir_rel = "run/output/compare/" . _compare_dir_name($ctx, $id, $ref_workdir);
    my $out_dir = "$ctx->{mu3e_dir}/$out_dir_rel";

    _log("compare task=$id");
    _log("  new: $in1");
    _log("  ref: $in2");
    _log("  out: $out_dir");

    if (!$ctx->{dry_run}) {
        die "Relval: missing new histogram: $in1 (run 'all' on $ctx->{workdir} first)\n"
            unless -f $in1;
        die "Relval: missing ref histogram: $in2 (run 'all' on $ref_workdir first)\n"
            unless -f $in2;
        die "Relval: compare exe missing: $ctx->{compare_exe}\n"
            unless -f $ctx->{compare_exe};
        die "Relval: compare template missing: $ctx->{compare_template}\n"
            unless -f $ctx->{compare_template};
        make_path($out_dir);
    }

    my $cwd = getcwd();
    if (!$ctx->{dry_run}) {
        chdir($ctx->{mu3e_dir}) or die "Cannot chdir $ctx->{mu3e_dir}: $!\n";
        _run($ctx, "cp", $ctx->{compare_template}, "./template-relval.tex");
    }
    _run(
        $ctx, $ctx->{compare_exe},
        "--in1", $in1,
        "--in2", $in2,
        "--dir", $out_dir_rel,
    );
    chdir($cwd) unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
sub relval_run_compare {
    my ($cfg, %opts) = @_;
    my $ctx = relval_context($cfg, %opts);
    my $ref = relval_ref_workdir($cfg, $ctx->{basedir});
    my $tasks = $cfg->{relval_tasks} // [];
    die "Relval: no tasks in config\n" unless @$tasks;

    _log("workdir=$ctx->{mu3e_dir}");
    _log("compare_against=$ref ($ctx->{basedir}/$ref)");
    die "Relval: reference workdir missing: $ctx->{basedir}/$ref\n"
        unless $ctx->{dry_run} || -d "$ctx->{basedir}/$ref";

    for my $task (@$tasks) {
        _run_compare_task($ctx, $task, $ref);
    }
    _log("compare done");
}

# ----------------------------------------------------------------------
sub relval_run_all {
    my ($cfg, %opts) = @_;
    my $ctx = relval_context($cfg, %opts);
    my $tasks = $cfg->{relval_tasks} // [];
    die "Relval: no tasks in config\n" unless @$tasks;

    _log("workdir=$ctx->{mu3e_dir}");
    _log("tasks=" . scalar(@$tasks));
    _ensure_run_dir($ctx);

    for my $task (@$tasks) {
        relval_run_task($ctx, $task);
    }
    _log("all tasks done");
}

1;
