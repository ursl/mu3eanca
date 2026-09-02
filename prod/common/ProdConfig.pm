package ProdConfig;

# ----------------------------------------------------------------------
# ProdConfig
# ==========
#
# Shared config loader for prod workflows (relval, startGT, prompt0).
# Lives in prod/common together with Setup.pm.
# Named ProdConfig (not Config) because Perl already ships Config.pm.
#
# Format:
#   # comment
#   key: value
#   key: "quoted value"
#
#   # --- repositories (two equivalent styles) ---
#
#   # 1) Prefixed keys (any name):
#   mu3e_repo: "git@bitbucket.org:mu3e/mu3e"
#   mu3e_tag: "v6.5"
#   # mu3e_checkout_branch: "dev"
#   # merge one or more branches/tags/commits after checkout:
#   mu3e_merges: "CDB-v6.9pre"
#   # mu3e_merges: "branch1, branch2"          # comma-separated also ok
#   # mu3e_checkout_merge: "CDB-v6.9pre"     # longer alias, same meaning
#   # mu3e_workdir: "mu3e-v6.5"
#   # mu3e_build: true
#   # mu3e_relink: true
#   # mu3e_submodules: true
#   # mu3e_install: true
#   # mu3e_cmake_args: "-DCMAKE_INSTALL_PREFIX={path}/install"
#
#   mu3eValidation_repo: "git@bitbucket.org:mu3e/mu3eValidation"
#   mu3eValidation_tag: "main"
#   mu3eValidation_build: true
#   # mu3eValidation_merges: "some-feature-branch"
#
#   # 2) Explicit [repo] blocks:
#   [repo]
#   id: mu3eUtil
#   repo: "git@bitbucket.org:mu3e/mu3eUtil"
#   tag: v1.0
#   build: true
#
#   # --- meta setup: run several setup configs ---
#   include: config-setup-v6.5.cfg
#   include: config-setup-mu3eValidation.cfg
#   # or:
#   [include]
#   file: config-setup-v6.5.cfg
#
#   # --- meta relval: run several relval configs (config-relval-all.cfg) ---
#   include: config-relval-v6.5.cfg
#   include: config-relval-v6.9pre0.cfg
#
#   # --- relval tasks ---
#   [task]
#   id: signal
#   ...
#
# Host overlay (CWD): host-<platform>.cfg preferred, else config-<host>.cfg.
#
#   base: prompt-base.cfg     # merge defaults; this file overrides
#
# History
#         2026/07/13 first shot
#         2026/07/13 multi-repo setup + include
#         2026/07/13 {id}_merges / {id}_merge aliases
#         2026/09/02 rename RelvalConfig -> ProdConfig
#         2026/09/02 base: merge for versioned prompt cfgs
# ----------------------------------------------------------------------

use strict;
use warnings;
use Exporter qw(import);
use Sys::Hostname qw(hostname);
use File::Basename qw(dirname);
use File::Spec;

our @EXPORT_OK = qw(
    load_config
    load_setup_units
    load_relval_units
    host_name
    host_overlay_file
    read_cfg_file
    discover_repos
);

# Keys that append when repeated (not overwrite).
my %LIST_KEYS = map { $_ => 1 } qw(
    include
    alltags
    tar_packages
    slurm_stages
    githash_extra
    pipeline_alias
    run_year
);

# ----------------------------------------------------------------------
sub host_name {
    my ($cli_host) = @_;
    return $cli_host if defined $cli_host && $cli_host ne "";
    return $ENV{RELVAL_HOST} if defined $ENV{RELVAL_HOST} && $ENV{RELVAL_HOST} ne "";
    my $h = hostname();
    $h =~ s/\..*//;
    return $h;
}

# ----------------------------------------------------------------------
# prompt0 uses host-<platform>.cfg (e.g. host-merlin6.cfg).
# startGT / relval keep config-<hostname>.cfg.
sub host_overlay_file {
    my ($host) = @_;
    return "" unless defined $host && $host ne "";
    return "host-$host.cfg" if -f "host-$host.cfg";
    return "config-$host.cfg";
}

# ----------------------------------------------------------------------
sub _strip {
    my ($s) = @_;
    return "" unless defined $s;
    $s =~ s/^\s+|\s+$//g;
    return $s;
}

# ----------------------------------------------------------------------
sub _parse_value {
    my ($raw) = @_;
    $raw = _strip($raw);
    if ($raw =~ /^"(.*)"$/) {
        return $1;
    }
    if ($raw =~ /^'(.*)'$/) {
        return $1;
    }
    return $raw;
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
sub _split_list {
    my ($val) = @_;
    return () unless defined $val && _strip($val) ne "";
    if (ref($val) eq "ARRAY") {
        return map { _strip($_) } grep { defined $_ && _strip($_) ne "" } @$val;
    }
    return map { _strip($_) } grep { $_ ne "" } split(/[\s,]+/, $val);
}

# ----------------------------------------------------------------------
# Returns ($scalars_href, $tasks_aref, $repos_aref, $includes_aref)
sub read_cfg_file {
    my ($path) = @_;
    my %cfg = ();
    my @tasks = ();
    my @repos = ();
    my @includes = ();
    return (\%cfg, \@tasks, \@repos, \@includes) unless defined $path && -f $path;

    open(my $fh, "<", $path) or die "Cannot open $path: $!\n";
    my $mode = "";          # "", task, repo, include
    my %block = ();

    my $flush = sub {
        return if $mode eq "";
        if ($mode eq "task") {
            die "$path: [task] block missing id\n"
                unless defined $block{id} && $block{id} ne "";
            my %copy = %block;
            push @tasks, \%copy;
        } elsif ($mode eq "repo") {
            die "$path: [repo] block missing id\n"
                unless defined $block{id} && $block{id} ne "";
            die "$path: [repo] $block{id} missing repo url\n"
                unless defined $block{repo} && $block{repo} ne "";
            my %copy = %block;
            push @repos, \%copy;
        } elsif ($mode eq "include") {
            my $file = $block{file} // $block{path} // "";
            die "$path: [include] block missing file:\n" if $file eq "";
            push @includes, $file;
        }
        %block = ();
        $mode = "";
    };

    while (my $line = <$fh>) {
        chomp($line);
        $line =~ s/\r$//;
        next if $line =~ /^\s*#/;
        next if $line =~ /^\s*$/;

        if ($line =~ /^\s*\[(task|repo|include)\]\s*$/i) {
            $flush->();
            $mode = lc($1);
            next;
        }

        if ($line =~ /^\s*([A-Za-z0-9_]+)\s*:\s*(.*)$/) {
            my ($key, $val) = ($1, _parse_value($2));
            if ($mode ne "") {
                $block{$key} = $val;
            } elsif ($LIST_KEYS{$key}) {
                $cfg{$key} = [] unless ref($cfg{$key}) eq "ARRAY";
                push @{$cfg{$key}}, $val;
                push @includes, $val if $key eq "include";
            } else {
                $cfg{$key} = $val;
            }
            next;
        }

        die "$path: cannot parse line $. : $line\n";
    }
    $flush->();
    close($fh);

    # Also pick up include: list stored in scalars
    if (ref($cfg{include}) eq "ARRAY") {
        for my $f (@{$cfg{include}}) {
            push @includes, $f unless grep { $_ eq $f } @includes;
        }
    }

    return (\%cfg, \@tasks, \@repos, \@includes);
}

# ----------------------------------------------------------------------
sub _merge_scalars {
    my ($base, $over) = @_;
    for my $key (keys %$over) {
        if ($LIST_KEYS{$key} && ref($over->{$key}) eq "ARRAY") {
            $base->{$key} = [] unless ref($base->{$key}) eq "ARRAY";
            push @{$base->{$key}}, @{$over->{$key}};
        } else {
            $base->{$key} = $over->{$key};
        }
    }
}

# ----------------------------------------------------------------------
# Known suffixes after the repo id (longest first for matching).
my @REPO_SUFFIXES = qw(
    checkout_merge_branch
    checkout_merges
    checkout_merge
    checkout_branch
    checkout_tag
    cmake_args
    submodules
    workdir
    make_jobs
    relink
    merges
    merge
    install
    build
    tag
    repo
);

# ----------------------------------------------------------------------
sub _repo_defaults {
    my ($id) = @_;
    my $is_mu3e = ($id eq "mu3e");
    return (
        build      => $is_mu3e ? 1 : 0,
        relink     => $is_mu3e ? 1 : 0,
        submodules => $is_mu3e ? 1 : 0,
        install    => $is_mu3e ? 1 : 0,
    );
}

# ----------------------------------------------------------------------
# Turn a raw repo hash (from [repo] or prefixed keys) into a normalized one.
sub _normalize_repo {
    my ($raw, $cfg) = @_;
    my $id = _strip($raw->{id} // "");
    die "repo missing id\n" if $id eq "";

    my $url = _strip($raw->{repo} // $raw->{url} // "");
    die "repo $id: missing repo/url\n" if $url eq "";

    my $branch = _strip($raw->{checkout_branch} // $raw->{branch} // "");
    my $tag    = _strip($raw->{checkout_tag} // $raw->{tag} // "");
    die "repo $id: need tag or checkout_branch\n" if $tag eq "" && $branch eq "";
    die "repo $id: use only one of tag or checkout_branch\n"
        if $tag ne "" && $branch ne "";

    my @merges = _split_list($raw->{checkout_merge});
    push @merges, _split_list($raw->{checkout_merge_branch});
    push @merges, _split_list($raw->{checkout_merges});
    push @merges, _split_list($raw->{merge});
    push @merges, _split_list($raw->{merges});

    my %def = _repo_defaults($id);
    my $build = exists $raw->{build}
        ? _truthy($raw->{build}) : $def{build};
    my $relink = exists $raw->{relink}
        ? _truthy($raw->{relink}) : $def{relink};
    my $submodules = exists $raw->{submodules}
        ? _truthy($raw->{submodules}) : $def{submodules};
    my $install = exists $raw->{install}
        ? _truthy($raw->{install}) : $def{install};

    my $workdir = _strip($raw->{workdir} // "");
    if ($workdir eq "") {
        my $label = $branch ne "" ? $branch : $tag;
        $label =~ s/[\/\s]/_/g;
        $workdir = "$id-$label";
    }
    $workdir =~ s/[\/\s]/_/g;

    my $jobs = $raw->{make_jobs} // $cfg->{make_jobs} // 4;
    my $cmake_args = _strip($raw->{cmake_args} // "");

    return {
        id         => $id,
        repo       => $url,
        tag        => $tag,
        branch     => $branch,
        merges     => \@merges,
        workdir    => $workdir,
        build      => $build,
        relink     => $relink,
        submodules => $submodules,
        install    => $install,
        make_jobs  => 0 + $jobs,
        cmake_args => $cmake_args,
    };
}

# ----------------------------------------------------------------------
# Discover repos from prefixed keys: <id>_repo, <id>_tag, ...
sub discover_repos {
    my ($cfg, $explicit_repos) = @_;
    $explicit_repos //= [];

    my %by_id;
    my @order;

    # Prefixed keys
    for my $key (sort keys %$cfg) {
        next if ref($cfg->{$key});
        my $matched;
        for my $suf (@REPO_SUFFIXES) {
            if ($key =~ /^(.+)_(\Q$suf\E)$/) {
                my ($id, $field) = ($1, $2);
                # Avoid treating plain "make_jobs" etc. — need a non-empty id
                # and the key must be id_repo companion eventually.
                next if $id eq "";
                # Skip global keys that look like suffixes of empty patterns
                next if $key eq "make_jobs" || $key eq "relink_script";
                $matched = [$id, $field];
                last;
            }
        }
        next unless $matched;
        my ($id, $field) = @$matched;
        if (!exists $by_id{$id}) {
            $by_id{$id} = { id => $id };
            push @order, $id;
        }
        $by_id{$id}{$field} = $cfg->{$key};
    }

    # Only keep ids that have a _repo URL
    my @from_keys;
    for my $id (@order) {
        next unless defined $by_id{$id}{repo} && _strip($by_id{$id}{repo}) ne "";
        push @from_keys, _normalize_repo($by_id{$id}, $cfg);
    }

    my @from_blocks;
    for my $raw (@$explicit_repos) {
        push @from_blocks, _normalize_repo($raw, $cfg);
    }

    # Blocks override same id from keys
    my %seen;
    my @out;
    for my $r (@from_blocks, @from_keys) {
        next if $seen{$r->{id}}++;
        push @out, $r;
    }
    # Prefer block order first, then key order — actually we want: if both,
    # block wins. Current loop puts blocks first then skips key dupes. Good.

    # Re-do: blocks first (canonical), then keys not in blocks
    %seen = ();
    @out = ();
    for my $r (@from_blocks) {
        $seen{$r->{id}} = 1;
        push @out, $r;
    }
    for my $r (@from_keys) {
        next if $seen{$r->{id}}++;
        push @out, $r;
    }
    return \@out;
}

# ----------------------------------------------------------------------
sub _normalize_tasks {
    my ($cfg, $tasks) = @_;
    my $default_gt       = $cfg->{cdb_GT} // $cfg->{cdb_gt} // "";
    my $default_run_id   = 0 + ($cfg->{run_id} // 0);
    my $default_n_events = 0 + ($cfg->{n_events} // 0);

    my @out;
    my %seen;
    for my $item (@$tasks) {
        my %task = %$item;
        die "relval task missing id\n" unless defined $task{id} && $task{id} ne "";
        die "duplicate relval task id '$task{id}'\n" if $seen{$task{id}}++;

        my $mode = lc(_strip($task{mode} // ""));
        if ($mode ne "sim" && $mode ne "data") {
            my $raw = _strip($task{raw_input} // "");
            $mode = ($raw ne "") ? "data" : "sim";
        }
        $task{mode} = $mode;

        my $gt = _strip($task{cdb_GT} // $task{cdb_gt} // "");
        $gt = $default_gt if $gt eq "";
        die "relval_tasks/$task{id}: cdb_GT required\n" if $gt eq "";
        $task{cdb_GT} = $gt;

        if ($mode eq "sim") {
            die "relval_tasks/$task{id}: sim_conf required for mode sim\n"
                unless defined $task{sim_conf} && $task{sim_conf} ne "";
        } else {
            die "relval_tasks/$task{id}: raw_input required for mode data\n"
                unless defined $task{raw_input} && _strip($task{raw_input}) ne "";
        }
        die "relval_tasks/$task{id}: trirec_conf required\n"
            unless defined $task{trirec_conf} && $task{trirec_conf} ne "";

        $task{run_id}   = 0 + ($task{run_id} // $default_run_id);
        $task{n_events} = 0 + ($task{n_events} // $default_n_events);
        die "relval_tasks/$task{id}: run_id must be > 0\n" if $task{run_id} <= 0;
        if ($mode eq "sim") {
            die "relval_tasks/$task{id}: n_events must be > 0 for mode sim\n"
                if $task{n_events} <= 0;
        }

        if (!defined $task{treedump} || $task{treedump} eq "") {
            $task{treedump} = ($mode eq "sim") ? "true" : "false";
        }

        push @out, \%task;
    }
    return \@out;
}

# ----------------------------------------------------------------------
sub _apply_host_overlay {
    my ($cfg, $tasks, $repos, $host) = @_;
    my $host_path = host_overlay_file($host);
    $cfg->{_host} = $host // "";
    $cfg->{_host_overlay} = $host_path;
    return ($cfg, $tasks, $repos) unless $host_path ne "" && -f $host_path;

    my ($hcfg, $htasks, $hrepos, $hincludes) = read_cfg_file($host_path);
    _merge_scalars($cfg, $hcfg);
    $tasks = $htasks if @$htasks;
    # host [repo] blocks replace/add by id
    if (@$hrepos) {
        my %by_id = map { $_->{id} => $_ } @$repos;
        for my $r (@$hrepos) {
            $by_id{$r->{id}} = $r;
        }
        $repos = [ values %by_id ];
    }
    return ($cfg, $tasks, $repos);
}

# ----------------------------------------------------------------------
# Merge [repo]/[task] lists: overlay replaces/adds by id, base order kept.
sub _merge_by_id {
    my ($base, $over) = @_;
    $base = [] unless $base && @$base;
    $over = [] unless $over && @$over;
    return $over unless @$base;
    return $base unless @$over;
    my %by_id = map { $_->{id} => $_ } @$base;
    my @order = map { $_->{id} } @$base;
    for my $r (@$over) {
        my $id = $r->{id} // "";
        next if $id eq "";
        push @order, $id unless exists $by_id{$id};
        $by_id{$id} = $r;
    }
    return [ map { $by_id{$_} } @order ];
}

# ----------------------------------------------------------------------
# base: FILE — load defaults first; this file overrides scalars and
# appends LIST_KEYS. Not the same as include: (meta units for relval).
sub _apply_base {
    my ($cfg, $tasks, $repos, $cfg_path) = @_;
    my $rel = _strip($cfg->{base} // $cfg->{defaults} // "");
    return ($cfg, $tasks, $repos) if $rel eq "";

    my $bpath = _resolve_include($cfg_path, $rel);
    die "base not found: $rel (resolved $bpath) from $cfg_path\n"
        unless -f $bpath;

    my ($bcfg, $btasks, $brepos) = read_cfg_file($bpath);
    ($bcfg, $btasks, $brepos) = _apply_base($bcfg, $btasks, $brepos, $bpath);

    _merge_scalars($bcfg, $cfg);
    $bcfg->{_base} = $bpath;
    my $merged_tasks = (@$btasks && @$tasks) ? _merge_by_id($btasks, $tasks)
        : (@$tasks ? $tasks : $btasks);
    my $merged_repos = (@$brepos && @$repos) ? _merge_by_id($brepos, $repos)
        : (@$repos ? $repos : $brepos);
    return ($bcfg, $merged_tasks, $merged_repos);
}

# ----------------------------------------------------------------------
# Resolve path relative to the including config's directory.
sub _resolve_include {
    my ($base_cfg_path, $inc) = @_;
    return $inc if $inc =~ m{^/};
    my $dir = dirname($base_cfg_path);
    return File::Spec->catfile($dir, $inc);
}

# ----------------------------------------------------------------------
# Load one config file (base: merge, then host overlay).
# include: lines are not merged here; see load_setup_units / load_relval_units.
sub load_config {
    my ($base_path, $host) = @_;
    die "config not found: $base_path\n" unless -f $base_path;

    my ($cfg, $tasks, $repos, $includes) = read_cfg_file($base_path);
    ($cfg, $tasks, $repos) = _apply_base($cfg, $tasks, $repos, $base_path);
    ($cfg, $tasks, $repos) = _apply_host_overlay($cfg, $tasks, $repos, $host);

    $cfg->{setup_repos}  = discover_repos($cfg, $repos);
    $cfg->{relval_tasks} = _normalize_tasks($cfg, $tasks);
    $cfg->{includes}     = $includes;
    $cfg->{_config_path} = $base_path;
    return $cfg;
}

# For meta configs with include: lines, expand each included file as a unit.
# Used by setup (load_setup_units) and relval (load_relval_units).
# Returns list of config hrefs (each already host-overlaid).
sub _load_included_units {
    my ($base_path, $host) = @_;
    my $cfg = load_config($base_path, $host);
    my $includes = $cfg->{includes} // [];

    if (!@$includes) {
        return ($cfg);
    }

    my @units;
    for my $inc (@$includes) {
        my $path = _resolve_include($base_path, $inc);
        die "include not found: $inc (resolved $path) from $base_path\n"
            unless -f $path;
        push @units, load_config($path, $host);
    }
    return @units;
}

# ----------------------------------------------------------------------
sub load_setup_units {
    return _load_included_units(@_);
}

# ----------------------------------------------------------------------
sub load_relval_units {
    return _load_included_units(@_);
}

1;
