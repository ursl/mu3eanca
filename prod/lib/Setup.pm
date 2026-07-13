package Setup;

# ----------------------------------------------------------------------
# Setup
# =====
#
# Create and modify git repositories under setup_basedir
# (alias: mu3e_relval_basedir).
#
# Config drives one or more repos via prefixed keys or [repo] blocks
# (see RelvalConfig). Example:
#
#   ./relval -c config-setup-v6.5.cfg setup
#   ./relval -c meta-config-setup.cfg setup
#
# Per-repo steps:
#   clone/fetch → checkout tag|branch → merge → [submodules] → [build] → [relink]
#
# History
#         2026/07/13 first shot
#         2026/07/13 multi-repo rewrite
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Cwd qw(getcwd);
use File::Path qw(make_path);

our @EXPORT_OK = qw(
    setup_basedir
    setup_contexts_from_config
    setup_ensure_clone
    setup_checkout
    setup_merge
    setup_update_submodules
    setup_build
    setup_relink
    setup_run_repo
    setup_run_config
    setup_status_repo
    setup_status_config
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
    return "$now/Setup          / ";
}

# ----------------------------------------------------------------------
sub _log {
    my ($s, @msg) = @_;
    my $id = $s->{id} // "?";
    print(_prefix(), "[$id] ", @msg, "\n");
}

# ----------------------------------------------------------------------
sub _run {
    my ($s, @cmd) = @_;
    _log($s, "cmd: @cmd");
    return if $s->{dry_run};
    system(@cmd);
    if ($? != 0) {
        die "Command failed: @cmd\n";
    }
}

# ----------------------------------------------------------------------
sub _git {
    my ($s, @args) = @_;
    _run($s, "git", "-C", $s->{path}, @args);
}

# ----------------------------------------------------------------------
sub _git_ok {
    my ($s, @args) = @_;
    return 0 if $s->{dry_run};
    return system("git", "-C", $s->{path}, @args) == 0;
}

# ----------------------------------------------------------------------
sub _git_capture {
    my ($s, @args) = @_;
    return "(dry-run)" if $s->{dry_run};
    my $out = `git -C "$s->{path}" @args`;
    chomp($out);
    return $out;
}

# ----------------------------------------------------------------------
sub _dir_empty {
    my ($dir) = @_;
    return 1 unless -d $dir;
    opendir(my $dh, $dir) or return 1;
    my @entries = grep { $_ !~ /^\.\.?$/ } readdir($dh);
    closedir($dh);
    return @entries == 0;
}

# ----------------------------------------------------------------------
sub _dir_only_workflow_metadata {
    my ($dir) = @_;
    return 1 unless -d $dir;
    opendir(my $dh, $dir) or return 1;
    my @entries = grep { $_ !~ /^\.\.?$/ } readdir($dh);
    closedir($dh);
    my %ok = map { $_ => 1 } qw(.snakemake .markers markers logs status);
    for my $e (@entries) {
        return 0 unless $ok{$e};
    }
    return 1;
}

# ----------------------------------------------------------------------
sub _is_git_repo {
    my ($dir) = @_;
    return (-d "$dir/.git" || -f "$dir/.git");
}

# ----------------------------------------------------------------------
sub setup_basedir {
    my ($cfg) = @_;
    my $basedir = _strip($cfg->{setup_basedir} // $cfg->{mu3e_relval_basedir} // "");
    die "Setup: setup_basedir (or mu3e_relval_basedir) required in config / host overlay\n"
        if $basedir eq "";
    return $basedir;
}

# ----------------------------------------------------------------------
# Build one runtime context per repo in $cfg->{setup_repos}.
sub setup_contexts_from_config {
    my ($cfg, %opts) = @_;
    my $basedir = setup_basedir($cfg);
    my $repos   = $cfg->{setup_repos} // [];
    die "Setup: no repositories in " . ($cfg->{_config_path} // "config")
      . " (need e.g. mu3e_repo: ... or [repo] blocks)\n"
        unless @$repos;

    my @ctx;
    for my $repo (@$repos) {
        my $workdir = $repo->{workdir};
        # -s NAME overrides workdir when there is exactly one repo
        if (@$repos == 1 && defined $opts{setup_name} && _strip($opts{setup_name}) ne "") {
            $workdir = _strip($opts{setup_name});
            $workdir =~ s/[\/\s]/_/g;
        }

        my $jobs = $opts{jobs} // $repo->{make_jobs} // $cfg->{make_jobs} // 4;

        # Optional util merges still supported for mu3e via config keys
        my @util_merges;
        if ($repo->{id} eq "mu3e") {
            push @util_merges, _split_cfg_list($cfg->{mu3eUtil_checkout_merge});
            push @util_merges, _split_cfg_list($cfg->{mu3eUtil_checkout_merge_branch});
            push @util_merges, _split_cfg_list($cfg->{mu3eUtil_checkout_merges});
        }
        my $util_subdir = _strip($cfg->{mu3eUtil_subdir} // "modules/mu3eUtil");
        $util_subdir = "modules/mu3eUtil" if $util_subdir eq "";

        push @ctx, {
            id            => $repo->{id},
            repo          => $repo->{repo},
            tag           => $repo->{tag},
            branch        => $repo->{branch},
            merges        => [ @{$repo->{merges}} ],
            workdir       => $workdir,
            path          => "$basedir/$workdir",
            basedir       => $basedir,
            build         => $repo->{build},
            relink        => $repo->{relink},
            submodules    => $repo->{submodules},
            make_jobs     => 0 + $jobs,
            relink_script => _strip($cfg->{relink_script} // ""),
            util_merges   => \@util_merges,
            util_subdir   => $util_subdir,
            util_dir      => "$basedir/$workdir/$util_subdir",
            dry_run       => $opts{dry_run} // 0,
            cfg           => $cfg,
        };
    }
    return @ctx;
}

# ----------------------------------------------------------------------
sub _split_cfg_list {
    my ($val) = @_;
    return () unless defined $val && _strip($val) ne "";
    return map { _strip($_) } grep { $_ ne "" } split(/[\s,]+/, $val);
}

# ----------------------------------------------------------------------
sub setup_ensure_clone {
    my ($s) = @_;
    my $dir  = $s->{path};
    my $repo = $s->{repo};

    _log($s, "ensure clone: $dir");
    make_path($s->{basedir}) unless $s->{dry_run};
    make_path($dir) unless $s->{dry_run} || -d $dir;

    if (_is_git_repo($dir)) {
        _log($s, "already a git repo");
        _git($s, "fetch", "--force", "origin");
        _git($s, "fetch", "--tags", "--force", "origin");
        return;
    }

    if ($s->{dry_run}) {
        _log($s, "would clone $repo -> $dir");
        return;
    }

    if (_dir_empty($dir)) {
        _run($s, "git", "clone", $repo, $dir);
    } elsif (_dir_only_workflow_metadata($dir)
             || -f "$dir/CMakeLists.txt"
             || -f "$dir/.gitmodules")
    {
        _log($s, "init in place: $dir");
        _git($s, "init");
        unless (_git_ok($s, "remote", "get-url", "origin")) {
            _git($s, "remote", "add", "origin", $repo);
        }
        _git($s, "fetch", "--force", "origin");
        _git($s, "fetch", "--tags", "--force", "origin");
    } else {
        die "Setup: $dir exists but is not a git checkout of $s->{id}.\n"
          . "       Remove it and retry (e.g. rm -rf \"$dir\").\n";
    }
}

# ----------------------------------------------------------------------
sub _resolve_merge_ref {
    my ($s, $dir, $spec) = @_;
    $spec = _strip($spec);
    die "Setup: empty merge ref\n" if $spec eq "";

    my $git_ok = sub {
        return 0 if $s->{dry_run};
        return system("git", "-C", $dir, @_) == 0;
    };
    my $git_run = sub {
        _log($s, "cmd: git -C $dir @_");
        return if $s->{dry_run};
        system("git", "-C", $dir, @_) == 0 or die "git failed in $dir: @_\n";
    };

    $git_run->("fetch", "--force", "origin");
    $git_run->("fetch", "--tags", "--force", "origin");

    if ($git_ok->("rev-parse", "--verify", "--quiet", "${spec}^{commit}")) {
        return $spec;
    }
    $git_ok->("fetch", "--force", "origin", "${spec}:refs/remotes/origin/${spec}");
    if ($git_ok->("rev-parse", "--verify", "--quiet", "origin/${spec}^{commit}")) {
        return "origin/$spec";
    }
    $git_ok->("fetch", "--force", "origin", "refs/heads/${spec}:refs/remotes/origin/${spec}");
    if ($git_ok->("rev-parse", "--verify", "--quiet", "origin/${spec}^{commit}")) {
        return "origin/$spec";
    }
    die "Setup: merge ref '$spec' not available after fetch in $dir.\n";
}

# ----------------------------------------------------------------------
sub setup_checkout {
    my ($s) = @_;
    setup_ensure_clone($s);

    if ($s->{branch} ne "") {
        my $branch = $s->{branch};
        _log($s, "checkout branch origin/$branch");
        die "Setup: failed to fetch origin/$branch\n"
            unless $s->{dry_run} || _git_ok($s, "fetch", "--force", "origin", $branch);
        if (!$s->{dry_run} && _git_ok($s, "show-ref", "--verify", "--quiet", "refs/heads/$branch")) {
            _git($s, "checkout", $branch);
        } else {
            _git($s, "checkout", "-B", $branch, "origin/$branch");
        }
        _git($s, "reset", "--hard", "origin/$branch");
    } else {
        _log($s, "checkout ref $s->{tag}");
        _git($s, "checkout", $s->{tag});
    }

    _log($s, "HEAD: ", _git_capture($s, "rev-parse", "--short", "HEAD"));
}

# ----------------------------------------------------------------------
sub setup_merge {
    my ($s, @extra) = @_;
    my @specs = (@{$s->{merges}}, @extra);
    return unless @specs;

    for my $spec (@specs) {
        next if _strip($spec) eq "";
        my $merge_ref = $s->{dry_run}
            ? $spec
            : _resolve_merge_ref($s, $s->{path}, $spec);
        _log($s, "merge $merge_ref ($spec)");
        _git(
            $s, "merge", "--no-edit",
            "-m", "relval setup: merge $spec for testing",
            $merge_ref,
        );
        _log($s, "after merge: ", _git_capture($s, "rev-parse", "--short", "HEAD"));
    }
}

# ----------------------------------------------------------------------
sub setup_update_submodules {
    my ($s) = @_;
    return unless $s->{submodules};

    _log($s, "submodule update --init --recursive");
    _git($s, "submodule", "update", "--init", "--recursive");

    my @util_merges = @{$s->{util_merges} // []};
    return unless @util_merges;

    my $util = $s->{util_dir};
    if (!$s->{dry_run} && !_is_git_repo($util)) {
        die "Setup: mu3eUtil missing at $util (after submodule update)\n";
    }
    _log($s, "util merges in $util");
    for my $spec (@util_merges) {
        next if _strip($spec) eq "";
        my $merge_ref = $s->{dry_run}
            ? $spec
            : _resolve_merge_ref($s, $util, $spec);
        _log($s, "util merge $merge_ref ($spec)");
        _run(
            $s, "git", "-C", $util, "merge", "--no-edit",
            "-m", "relval setup: merge $spec in mu3eUtil",
            $merge_ref,
        );
    }
}

# ----------------------------------------------------------------------
sub setup_build {
    my ($s) = @_;
    return unless $s->{build};

    my $build = "$s->{path}/_build";
    my $jobs  = $s->{make_jobs};
    _log($s, "build in $build (-j$jobs)");
    if (!$s->{dry_run}) {
        make_path($build);
        my $cwd = getcwd();
        chdir($build) or die "Cannot chdir $build: $!\n";
        _run($s, "cmake", "..");
        _run($s, "make", "-j$jobs");
        chdir($cwd) or die "Cannot chdir back to $cwd: $!\n";
    } else {
        _log($s, "would: cmake .. && make -j$jobs");
    }
}

# ----------------------------------------------------------------------
sub setup_relink {
    my ($s) = @_;
    return unless $s->{relink};

    my $script = $s->{relink_script};
    my $run    = "$s->{path}/run";
    die "Setup: relink_script not set (needed for $s->{id} relink)\n" if $script eq "";
    die "Setup: relink_script not found: $script\n"
        unless $s->{dry_run} || -x $script || -f $script;

    _log($s, "relink in $run via $script");
    if (!$s->{dry_run}) {
        make_path($run);
        my $cwd = getcwd();
        chdir($run) or die "Cannot chdir $run: $!\n";
        _run($s, $script);

        if (!-d "bvr2026") {
            _log($s, "run/bvr2026 missing; fetching from bitbucket v6.5");
            my $tmpdir = "/tmp/relval-bvr2026-$$";
            _run($s, "git", "clone", "--depth", "1", "--branch", "v6.5",
                 "https://bitbucket.org/mu3e/mu3e", "$tmpdir/mu3e-v6.5");
            die "Setup: fetched repo missing run/bvr2026\n"
                unless -d "$tmpdir/mu3e-v6.5/run/bvr2026";
            _run($s, "cp", "-R", "$tmpdir/mu3e-v6.5/run/bvr2026", "./bvr2026");
            _run($s, "rm", "-rf", $tmpdir);
        }
        make_path("output");
        chdir($cwd) or die "Cannot chdir back to $cwd: $!\n";
    }
}

# ----------------------------------------------------------------------
sub setup_run_repo {
    my ($s) = @_;
    _log($s, "setup_run path=$s->{path}");
    setup_checkout($s);
    setup_merge($s);
    setup_update_submodules($s);
    setup_build($s);
    setup_relink($s);
    _log($s, "setup_run done");
}

# ----------------------------------------------------------------------
sub setup_run_config {
    my ($cfg, %opts) = @_;
    my @ctx = setup_contexts_from_config($cfg, %opts);
    for my $s (@ctx) {
        setup_status_repo($s);
        setup_run_repo($s);
    }
}

# ----------------------------------------------------------------------
sub setup_status_repo {
    my ($s) = @_;
    print(_prefix(), "status [$s->{id}]\n");
    print("  basedir:   $s->{basedir}\n");
    print("  workdir:   $s->{workdir}\n");
    print("  path:      $s->{path}\n");
    print("  repo:      $s->{repo}\n");
    if ($s->{branch} ne "") {
        print("  checkout:  branch $s->{branch}\n");
    } else {
        print("  checkout:  tag/ref $s->{tag}\n");
    }
    print("  merges:    ", (@{$s->{merges}} ? join(", ", @{$s->{merges}}) : "(none)"), "\n");
    print("  build:     ", ($s->{build} ? "yes" : "no"), "\n");
    print("  relink:    ", ($s->{relink} ? "yes" : "no"), "\n");
    print("  submodules:", ($s->{submodules} ? "yes" : "no"), "\n");
    print("  make_jobs: $s->{make_jobs}\n");
    print("  exists:    ", (-d $s->{path} ? "yes" : "no"), "\n");
    if (-d $s->{path} && _is_git_repo($s->{path}) && !$s->{dry_run}) {
        my $head = `git -C "$s->{path}" rev-parse --short HEAD 2>/dev/null`;
        chomp($head);
        my $desc = `git -C "$s->{path}" describe --always --dirty 2>/dev/null`;
        chomp($desc);
        print("  HEAD:      $head\n");
        print("  describe:  $desc\n");
    }
}

# ----------------------------------------------------------------------
sub setup_status_config {
    my ($cfg, %opts) = @_;
    my @ctx = setup_contexts_from_config($cfg, %opts);
    for my $s (@ctx) {
        setup_status_repo($s);
    }
}

1;
