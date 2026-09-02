package Prompt;

# ----------------------------------------------------------------------
# Prompt
# ======
#
# prompt0 production area: dated workdir under setup_basedir, with one
# git checkout per [repo] block (mu3e, minalyzer, extra packages).
#
# Config + host overlay: RelvalConfig (host-<platform>.cfg).
# Git clone/build/relink: Setup (../common).
#
# History
#         2026/09/02 first shot (setup / init / status)
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Cwd qw(getcwd abs_path);
use File::Path qw(make_path);

our @EXPORT_OK = qw(
    prompt_platform
    prompt_apply_area
    prompt_context
    prompt_setup
    prompt_init
    prompt_bootstrap
    prompt_status
    prompt_list
);

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
    return "$now/prompt         / ";
}

# ----------------------------------------------------------------------
sub _log {
    print(_prefix(), @_, "\n");
}

# ----------------------------------------------------------------------
sub _run {
    my ($ctx, @cmd) = @_;
    _log("cmd: @cmd");
    return if $ctx->{dry_run};
    system(@cmd);
    die "Command failed: @cmd\n" if $? != 0;
}

# ----------------------------------------------------------------------
sub _cfg_list {
    my ($cfg, $key, @default) = @_;
    my $raw = $cfg->{$key};
    return @default unless defined $raw;
    if (ref($raw) eq "ARRAY") {
        my @items = grep { $_ ne "" } map { _strip($_) } @$raw;
        return @items ? @items : @default;
    }
    my @items = grep { $_ ne "" } split(/[\s,]+/, _strip($raw));
    return @items ? @items : @default;
}

# ----------------------------------------------------------------------
# merlin-l-00x is merlin6; login00x / merlin7 is merlin7.
# Explicit -H / PROMPT_HOST wins.
sub prompt_platform {
    my ($cli_host) = @_;
    return $cli_host if defined $cli_host && $cli_host ne "";
    return $ENV{PROMPT_HOST} if defined $ENV{PROMPT_HOST} && $ENV{PROMPT_HOST} ne "";
    require Sys::Hostname;
    my $h = Sys::Hostname::hostname();
    $h =~ s/\..*//;
    return "merlin6" if $h =~ /^merlin-l-/i || $h =~ /^merlin6$/i;
    return "merlin7" if $h =~ /^login00/i   || $h =~ /^merlin7$/i;
    return $h;
}

# ----------------------------------------------------------------------
# Dated production area: {host setup_basedir}/{prompt_workdir}/...
sub prompt_apply_area {
    my ($cfg, %opts) = @_;
    return $cfg if $cfg->{_prompt_area_applied};

    my $parent = _strip($cfg->{setup_basedir} // $cfg->{mu3e_relval_basedir} // "");
    $cfg->{_host_setup_basedir} = $parent;

    my $wd = _strip($opts{setup_name} // $cfg->{prompt_workdir} // $cfg->{version} // "");
    if ($wd ne "") {
        $wd =~ s/[\/\s]/_/g;
        $cfg->{prompt_workdir} = $wd;
        if ($parent ne "") {
            $cfg->{setup_basedir} = "$parent/$wd";
        }
    }
    $cfg->{_prompt_area_applied} = 1;
    return $cfg;
}

# ----------------------------------------------------------------------
sub prompt_repo_dir {
    my ($cfg, $id) = @_;
    my $root = _strip($cfg->{setup_basedir} // "");
    for my $r (@{ $cfg->{setup_repos} // [] }) {
        next unless $r->{id} eq $id;
        return "$root/$r->{workdir}";
    }
    return "$root/$id";
}

# ----------------------------------------------------------------------
sub prompt_context {
    my ($cfg, %opts) = @_;
    prompt_apply_area($cfg, %opts);
    require Setup;
    my $root    = _strip($cfg->{setup_basedir} // "");
    my $workdir = _strip($cfg->{prompt_workdir} // "");
    my $data    = _strip($cfg->{data_dir} // "");
    my $raw     = _strip($cfg->{raw_input_base} // "");
    $raw = "$data/raw" if $raw eq "" && $data ne "";

    my $anca = _strip($cfg->{mu3eanca} // "");
    my $slurm_run = _strip($cfg->{slurm_run} // "");
    $slurm_run = "$anca/slurm/run" if $slurm_run eq "" && $anca ne "";

    return {
        cfg            => $cfg,
        root           => $root,
        parent         => _strip($cfg->{_host_setup_basedir} // ""),
        workdir        => $workdir,
        data_dir       => $data,
        raw_dir        => $raw,
        mlzr_dir       => ($data ne "" && $workdir ne "") ? "$data/mlzr/$workdir" : "",
        trirec_dir     => ($data ne "" && $workdir ne "") ? "$data/trirec/$workdir" : "",
        mu3eanca       => $anca,
        slurm_run      => $slurm_run,
        slurm_queue    => _strip($cfg->{slurm_queue} // "-p hourly"),
        cdb_dbconn     => _strip($cfg->{cdb_dbconn} // ""),
        cdb_GT         => _strip($cfg->{cdb_GT} // ""),
        rdb_url        => _strip($cfg->{rdb_url} // ""),
        n_events       => 0 + (_strip($cfg->{n_events} // "100000")),
        min_events     => 0 + (_strip($cfg->{min_events} // "10000")),
        run_block_max  => 0 + (_strip($cfg->{run_block_max} // "10")),
        slurm_stages   => [ _cfg_list($cfg, "slurm_stages", qw(mlzr trirec sort)) ],
        tar_packages   => [ _cfg_list($cfg, "tar_packages", qw(mu3e minalyzer)) ],
        data_products  => [qw(mlzr trirec)],
        dry_run        => $opts{dry_run} // 0,
        jobs           => $opts{jobs},
    };
}

# ----------------------------------------------------------------------
sub prompt_setup {
    my ($cfg, %opts) = @_;
    prompt_apply_area($cfg, %opts);
    my $ctx = prompt_context($cfg, %opts);
    die "prompt: setup_basedir required in host overlay (host-<platform>.cfg)\n"
        if $ctx->{parent} eq "";
    die "prompt: prompt_workdir required in version config\n"
        if $ctx->{workdir} eq "";
    _log("setup production area $ctx->{root}");
    require Setup;
    Setup::setup_run_config($cfg, %opts);
}

# ----------------------------------------------------------------------
sub _write_githashes {
    my ($ctx, $cfg) = @_;
    my $path = "$ctx->{root}/githashes";
    _log("githashes: $path");
    return if $ctx->{dry_run};

    open my $fh, ">", $path or die "prompt: cannot write $path: $!\n";
    my @dirs;
    for my $r (@{ $cfg->{setup_repos} // [] }) {
        push @dirs, [ $r->{id}, "$ctx->{root}/$r->{workdir}" ];
    }
    for my $extra (_cfg_list($cfg, "githash_extra")) {
        push @dirs, [ $extra, "$ctx->{root}/$extra" ];
    }
    for my $pair (@dirs) {
        my ($label, $dir) = @$pair;
        print $fh "===== $label ($dir) =====\n";
        if (-d "$dir/.git" || -f "$dir/.git") {
            my $log = `git -C "$dir" --no-pager log -n 1 2>/dev/null`;
            print $fh ($log ne "" ? $log : "(no git log)\n");
        } else {
            print $fh "(not a git checkout)\n";
        }
        print $fh "\n";
    }
    close $fh;
    _log("githashes written");
}

# ----------------------------------------------------------------------
sub _run_alignment {
    my ($ctx, $cfg) = @_;
    my $mu3e = prompt_repo_dir($cfg, "mu3e");
    my $exe  = "$mu3e/_build/mu3eSim/mu3eSim";
    my $run  = "$mu3e/run";
    my $conf = _strip($cfg->{sim_conf} // "sim_beam.json");
    my $out  = _strip($cfg->{alignment_output} // "mu3e_alignment.root");
    my $nrun = 0 + (_strip($cfg->{sim_run_id} // "1"));
    my $nev  = 0 + (_strip($cfg->{sim_n_events} // "1"));

    _log("alignment mu3eSim conf=$conf output=$run/$out");
    if (!$ctx->{dry_run}) {
        die "prompt: mu3e missing: $mu3e (run ./prompt setup first)\n" unless -d $mu3e;
        die "prompt: mu3eSim missing: $exe (run ./prompt setup first)\n"
            unless -x $exe || -f $exe;
        make_path($run);
        die "prompt: sim conf missing: $run/$conf\n" unless -f "$run/$conf";
    }
    my $cwd = getcwd();
    chdir($run) or die "Cannot chdir $run: $!\n" unless $ctx->{dry_run};
    _run($ctx, $exe, "--run", $nrun, "-n", $nev, "--conf", $conf, "--output", $out);
    chdir($cwd) unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
sub _make_tar {
    my ($ctx, $cfg) = @_;
    my $root = $ctx->{root};
    my $tar  = "slurm/$ctx->{workdir}.tar.gz";
    my @pkgs = @{ $ctx->{tar_packages} };
    die "prompt: tar_packages empty\n" unless @pkgs;

    _log("tar $root/$tar  packages=@pkgs");
    if (!$ctx->{dry_run}) {
        make_path("$root/slurm");
        for my $p (@pkgs) {
            die "prompt: tar package missing: $root/$p (run ./prompt setup first)\n"
                unless -d "$root/$p";
        }
    }
    my $cwd = getcwd();
    chdir($root) or die "Cannot chdir $root: $!\n" unless $ctx->{dry_run};
    unlink("$root/$tar") if !$ctx->{dry_run} && -e "$root/$tar";
    _run(
        $ctx, "tar", "zcvf", $tar,
        "--exclude", ".git",
        "--exclude", "mu3e/install",
        "--exclude", "minalyzer/json_output",
        "--exclude", "minalyzer/root_output_files",
        @pkgs,
    );
    chdir($cwd) unless $ctx->{dry_run};
}

# ----------------------------------------------------------------------
# Production-area init (processRuns -i): slurm dirs, data blocks, hashes, alignment, tar.
sub prompt_init {
    my ($cfg, %opts) = @_;
    my $ctx = prompt_context($cfg, %opts);
    die "prompt: setup_basedir required in host overlay\n" if $ctx->{parent} eq "";
    die "prompt: prompt_workdir required in version config\n" if $ctx->{workdir} eq "";
    die "prompt: production root missing: $ctx->{root} (run ./prompt setup first)\n"
        unless $ctx->{dry_run} || -d $ctx->{root};

    _log("init production area $ctx->{root}");

    for my $stage (@{ $ctx->{slurm_stages} }) {
        make_path("$ctx->{root}/slurm/jobs/$stage/old")     unless $ctx->{dry_run};
        make_path("$ctx->{root}/slurm/storage1/$stage/old") unless $ctx->{dry_run};
        _log("slurm dirs: jobs/$stage storage1/$stage");
    }

    if ($ctx->{data_dir} eq "") {
        _log("data_dir unset; skip mlzr/trirec block dirs");
    } else {
        my $nmax = $ctx->{run_block_max};
        $nmax = 10 if $nmax < 1;
        for my $prod (@{ $ctx->{data_products} }) {
            for (my $i = 0; $i < $nmax; $i++) {
                my $block = sprintf("%03d", $i);
                my $dir   = "$ctx->{data_dir}/$prod/$ctx->{workdir}/$block";
                _log("mkdir $dir") if $i == 0;
                make_path($dir) unless $ctx->{dry_run};
            }
            _log("data blocks: $ctx->{data_dir}/$prod/$ctx->{workdir}/  000.."
                . sprintf("%03d", $nmax - 1));
        }
    }

    _write_githashes($ctx, $cfg);
    _run_alignment($ctx, $cfg);
    _make_tar($ctx, $cfg);
    _log("init done");
}

# ----------------------------------------------------------------------
sub prompt_bootstrap {
    my ($cfg, %opts) = @_;
    prompt_setup($cfg, %opts);
    prompt_init($cfg, %opts);
}

# ----------------------------------------------------------------------
sub prompt_status {
    my ($cfg, %opts) = @_;
    my $ctx = prompt_context($cfg, %opts);
    my $mu3e = prompt_repo_dir($cfg, "mu3e");
    my $mlzr = prompt_repo_dir($cfg, "minalyzer");
    my $exe  = "$mu3e/_build/mu3eSim/mu3eSim";
    my $man  = "$mlzr/_build/analyzer/minalyzer";
    my $align = "$mu3e/run/" . _strip($cfg->{alignment_output} // "mu3e_alignment.root");
    my $tar  = "$ctx->{root}/slurm/$ctx->{workdir}.tar.gz";

    print(_prefix(), "status\n");
    print("  platform overlay:  ", ($cfg->{_host_overlay} // ""), "\n");
    print("  setup_basedir:     ", ($ctx->{parent} ne "" ? $ctx->{parent} : "(unset)"), "\n");
    print("  prompt_workdir:    ", ($ctx->{workdir} ne "" ? $ctx->{workdir} : "(unset)"), "\n");
    print("  production root:   ", ($ctx->{root} ne "" ? $ctx->{root} : "(unset)"),
        " (", (-d $ctx->{root} ? "present" : "missing"), ")\n");
    print("  data_dir:          ", ($ctx->{data_dir} ne "" ? $ctx->{data_dir} : "(unset)"), "\n");
    print("  raw_input_base:    ", ($ctx->{raw_dir} ne "" ? $ctx->{raw_dir} : "(unset)"), "\n");
    print("  mlzr_dir:          $ctx->{mlzr_dir}\n") if $ctx->{mlzr_dir} ne "";
    print("  trirec_dir:        $ctx->{trirec_dir}\n") if $ctx->{trirec_dir} ne "";
    print("  cdb_dbconn:        $ctx->{cdb_dbconn}\n");
    print("  cdb_GT:            $ctx->{cdb_GT}\n");
    print("  slurm_run:         $ctx->{slurm_run}\n");
    print("  slurm_queue:       $ctx->{slurm_queue}\n");
    print("  tar_packages:      ", join(" ", @{ $ctx->{tar_packages} }), "\n");
    print("  tar file:          $tar (", (-f $tar ? "present" : "missing"), ")\n");
    print("  mu3eSim:           ", ((-x $exe || -f $exe) ? "yes" : "no"), "  $exe\n");
    print("  minalyzer:         ", ((-x $man || -f $man) ? "yes" : "no"), "  $man\n");
    print("  alignment ROOT:    $align (", (-f $align ? "present" : "missing"), ")\n");
    print("  githashes:         ",
        (-f "$ctx->{root}/githashes" ? "present" : "missing"), "\n");

    require Setup;
    Setup::setup_status_config($cfg, %opts);
}

# ----------------------------------------------------------------------
sub prompt_list {
    my ($cfg) = @_;
    print(_prefix(), "config ", ($cfg->{_config_path} // ""), "\n");
    print("  host overlay: ", ($cfg->{_host_overlay} // ""),
        ((defined $cfg->{_host_overlay} && -f $cfg->{_host_overlay}) ? " (loaded)" : " (missing)"),
        "\n");
    print("  host setup_basedir: ", ($cfg->{_host_setup_basedir} // "(unset)"), "\n");
    print("  production root:    ", ($cfg->{setup_basedir} // "(unset)"), "\n");
    for my $key (sort keys %$cfg) {
        next if $key eq "setup_repos" || $key eq "relval_tasks"
            || $key eq "includes" || $key =~ /^_/;
        my $val = $cfg->{$key};
        next unless defined $val;
        if (ref($val) eq "ARRAY") {
            print("  $key: ", join(", ", @$val), "\n");
            next;
        }
        next if ref($val);
        print("  $key: $val\n");
    }
    my $repos = $cfg->{setup_repos} // [];
    print(_prefix(), "packages (", scalar(@$repos), ")\n");
    for my $r (@$repos) {
        my $co = $r->{branch} ne "" ? "branch=$r->{branch}" : "tag=$r->{tag}";
        print("  - id=$r->{id} $co workdir=$r->{workdir}"
            . " build=" . ($r->{build} ? "yes" : "no")
            . " relink=" . ($r->{relink} ? "yes" : "no")
            . "\n");
    }
}

1;
