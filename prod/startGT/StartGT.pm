package StartGT;

# ----------------------------------------------------------------------
# StartGT
# =======
#
# MU3E checkout + one mcideal mu3eSim run → single alignment/geometry ROOT.
# All GT flavours (mcideal, mcrealistic, data) read that file; installed-
# component selection is done in cdbRunPayloadWriter (-m 2025/2026/all).
#
# Config + host overlay: ProdConfig; git build: Setup (../common).
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
    startgt_gt_base
    startgt_resolve_gt
    startgt_alignment_root
    startgt_setup
    startgt_run_mu3esim
    startgt_ensure_test_cdb
    startgt_run_alignment_payloads
    startgt_run_quality_payloads
    startgt_run_stuff_payloads
    startgt_run_mask_payloads
    startgt_run_efficiency_payloads
    startgt_run_timecalib_payloads
    startgt_run_alltags
    startgt_alltags_config
    startgt_status
);

my @TEST_CDB_SUBDIRS = qw(payloads globaltags tags runrecords configs);
my @ALIGNMENT_CALIB_TYPES = qw(pixelalignment tilealignment fibrealignment mppcalignment);
my @QUALITY_CALIB_TYPES = qw(pixelqualitylm fibrequality tilequality);
my @STUFF_CALIB_TYPES = qw(eventstuffv1 eventstuffv2 detsetupv1);
my @MASK_CALIB_TYPES = qw(pixelmask);
my @EFFICIENCY_CALIB_TYPES = qw(pixelefficiency);
my @TIMECALIB_TYPES = qw(pixeltimecalibration tiletimecalibration);
my $DEFAULT_IOV = 1;

my %KNOWN_CALIB = map { $_ => 1 } (
    @ALIGNMENT_CALIB_TYPES,
    @QUALITY_CALIB_TYPES,
    @STUFF_CALIB_TYPES,
    @MASK_CALIB_TYPES,
    @EFFICIENCY_CALIB_TYPES,
    @TIMECALIB_TYPES,
);

my %QUALITY_ANNOTATIONS = (
    pixelqualitylm => "Perfect pixel detector with no deficiencies.",
    fibrequality   => "Perfect fibre detector with no deficiencies.",
    tilequality    => "Perfect tile detector with no deficiencies.",
);

# Ideal IOV-1 inputs for stuff calibrations (fixed JSON under CDB ascii/).
my %STUFF_CALIB = (
    eventstuffv1 => {
        ideal_file => "ascii/eventstuff-ideal.json",
        annotation => "No limitations to time stamps (ideal IOV 1).",
    },
    eventstuffv2 => {
        ideal_file => "ascii/eventstuffv2-ideal.json",
        annotation => "No limitations to time stamps (ideal IOV 1).",
    },
    detsetupv1 => {
        ideal_file => "ascii/detector-MagnetOn-v6.5.json",
        annotation => "Magnet turned on.",
    },
);

my %MASK_ANNOTATIONS = (
    pixelmask => "All pixels unmasked (ideal IOV 1).",
);

my %EFFICIENCY_ANNOTATIONS = (
    pixelefficiency => "Fully efficient for complete pixel detector.",
);

my %TIMECALIB_ANNOTATIONS = (
    pixeltimecalibration => "Zero shifts and uncertainties (ideal IOV 1).",
    tiletimecalibration  => "Zero offsets (ideal IOV 1).",
);

my %TIMECALIB_EXT = (
    pixeltimecalibration => "calib",
    tiletimecalibration  => "json",
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
sub _json_escape {
    my ($s) = @_;
    $s =~ s/\\/\\\\/g;
    $s =~ s/"/\\"/g;
    return $s;
}

# ----------------------------------------------------------------------
# CDB hash tag_<calib>_<gt>_iov_<n> → payloads/<calib>_<gt>/0000/...
sub startgt_alignment_tag_name {
    my ($calib, $gt) = @_;
    return "${calib}_${gt}";
}

# ----------------------------------------------------------------------
sub startgt_alignment_payload_path {
    my ($payload_dir, $calib, $gt, $iov) = @_;
    $iov //= $DEFAULT_IOV;
    my $tag   = startgt_alignment_tag_name($calib, $gt);
    my $hash  = "tag_${calib}_${gt}_iov_${iov}";
    my $block = sprintf("%04d", int($iov / 1000));
    return "$payload_dir/$tag/$block/$hash";
}

# ----------------------------------------------------------------------
sub startgt_gt_comment {
    my ($gt) = @_;
    return "Created (initially) for GT $gt with startGT";
}

# ----------------------------------------------------------------------
sub _json_format_string_array {
    my (@items) = @_;
    return "[ " . join(", ", map { '"' . _json_escape($_) . '"' } @items) . " ]";
}

# ----------------------------------------------------------------------
sub startgt_read_global_tag_tags {
    my ($path) = @_;
    return () unless -f $path;
    open my $fh, "<", $path or return ();
    local $/;
    my $content = <$fh>;
    close $fh;
    my @tags;
    if ($content =~ /"tags"\s*:\s*\[(.*?)\]/s) {
        my $inner = $1;
        while ($inner =~ /"((?:\\.|[^"\\])*)"/g) {
            my ($t) = ($1);
            $t =~ s/\\"/"/g;
            push @tags, $t;
        }
    }
    return @tags;
}

# All calibrations listed in GT config (alltags: …).
sub startgt_alltags_config {
    my ($cfg) = @_;
    my $raw = $cfg->{alltags};
    my @items;
    if (ref($raw) eq "ARRAY") {
        @items = map { _strip($_) } @$raw;
    } else {
        @items = map { _strip($_) } split(/[\s,]+/, _strip($raw // ""));
    }
    @items = grep { $_ ne "" } @items;
    die "startGT: alltags list empty in config (add alltags: lines to GT config)\n"
        unless @items;
    for my $calib (@items) {
        die "startGT: unknown alltags calibration '$calib'\n"
            unless $KNOWN_CALIB{$calib};
    }
    return @items;
}

# Full CDB tag name for a calibration prefix and GT.
sub startgt_tag_name_for_calib {
    my ($calib, $gt) = @_;
    return startgt_quality_tag_name($calib, $gt)
        if grep { $_ eq $calib } @QUALITY_CALIB_TYPES;
    return startgt_pixelefficiency_tag_name($gt)
        if $calib eq "pixelefficiency";
    return startgt_stuff_tag_name($calib, $gt)
        if grep { $_ eq $calib } @STUFF_CALIB_TYPES;
    return startgt_alignment_tag_name($calib, $gt);
}

sub startgt_pixelefficiency_tag_name {
    my ($gt) = @_;
    my $flavour = startgt_gt_flavour($gt);
    if ($flavour eq "mcideal" && $gt !~ /=/) {
        return "pixelefficiency_ideal";
    }
    return "pixelefficiency_${gt}";
}

sub startgt_calib_tag_suffix {
    my ($calib, $gt) = @_;
    my $tagname = startgt_tag_name_for_calib($calib, $gt);
    return startgt_quality_tag_suffix($tagname);
}

sub startgt_calib_payload_path {
    my ($payload_dir, $calib, $gt, $iov) = @_;
    $iov //= $DEFAULT_IOV;
    my $suffix = startgt_calib_tag_suffix($calib, $gt);
    my $hash   = "tag_${calib}_${suffix}_iov_${iov}";
    my $block  = sprintf("%04d", int($iov / 1000));
    return "$payload_dir/${calib}_${suffix}/$block/$hash";
}

# Chip/tile ID filter for ideal IOV-1 content (installed components).
sub startgt_installed_component_filter {
    my ($cfg, $gt) = @_;
    my $flavour = startgt_gt_flavour($gt);
    if ($flavour eq "mcideal" && $gt !~ /=/) {
        return $gt;
    }
    my $year = startgt_year_from_gt($gt, $cfg);
    die "startGT: need conditions year for '$gt' (=YYYY in name or conditions_year in config)\n"
        if $year eq "";
    return $year;
}

# pixelmask -m mode string (must contain "pixelmask" for cdbRunPayloadWriter).
sub startgt_pixelmask_mode {
    my ($cfg, $gt) = @_;
    my $flavour = startgt_gt_flavour($gt);
    if ($flavour eq "mcideal" && $gt !~ /=/) {
        return "pixelmask-ideal";
    }
    my $year = startgt_year_from_gt($gt, $cfg);
    die "startGT: need conditions year for pixelmask on '$gt'\n" if $year eq "";
    return "pixelmask-${year}";
}

sub startgt_stuff_tag_name {
    my ($calib, $gt) = @_;
    return "${calib}_${gt}";
}

sub startgt_ideal_input_filter {
    my ($cfg, $gt, $calib) = @_;
    if ($calib eq "pixelefficiency") {
        my $tagname = startgt_tag_name_for_calib($calib, $gt);
        return startgt_quality_ideal_filter($cfg, $gt, $tagname);
    }
    return startgt_calib_tag_suffix($calib, $gt);
}

sub startgt_write_calib_tags {
    my ($ctx, $cdb_root, $gt, @calibs) = @_;
    my $tags_dir    = "$cdb_root/tags";
    my $payload_dir = "$cdb_root/payloads";
    make_path($tags_dir) unless $ctx->{dry_run};

    my @written;
    for my $calib (@calibs) {
        my $tagname = startgt_tag_name_for_calib($calib, $gt);
        my $payload = startgt_calib_payload_path($payload_dir, $calib, $gt);
        if (!$ctx->{dry_run} && !-f $payload) {
            _log($ctx, "  skip $tagname (payload missing: $payload)");
            next;
        }
        startgt_write_tag_file($ctx, $tags_dir, $tagname, $gt);
        push @written, $tagname;
    }
    return @written;
}

# All tag names startGT may create for a given GT (from config alltags list).
sub startgt_expected_tag_names {
    my ($cfg, $gt) = @_;
    my @calibs = startgt_alltags_config($cfg);
    return map { startgt_tag_name_for_calib($_, $gt) } @calibs;
}

# Tag JSON files already on disk for this GT (from earlier startGT steps).
sub startgt_present_gt_tags {
    my ($ctx, $cfg, $cdb_root, $gt) = @_;
    my $tags_dir = "$cdb_root/tags";
    my @present;
    return @present unless -d $tags_dir;
    for my $tag (startgt_expected_tag_names($cfg, $gt)) {
        push @present, $tag if $ctx->{dry_run} || -f "$tags_dir/$tag";
    }
    return @present;
}

# ----------------------------------------------------------------------
# Merge tag names into globaltags/<gt>: existing file + disk + this step.
sub startgt_write_global_tag {
    my ($ctx, $cfg, $cdb_root, $gt, @add_tags) = @_;
    my $gt_dir = "$cdb_root/globaltags";
    make_path($gt_dir) unless $ctx->{dry_run};
    my $path = "$gt_dir/$gt";

    my %seen;
    my @tags;
    if (!$ctx->{dry_run} && -f $path) {
        for my $t (startgt_read_global_tag_tags($path)) {
            next if $seen{$t}++;
            push @tags, $t;
        }
    }
    for my $t (@add_tags, startgt_present_gt_tags($ctx, $cfg, $cdb_root, $gt)) {
        next if $t eq "" || $seen{$t}++;
        push @tags, $t;
    }
    @tags = sort @tags;

    my $comment = _json_escape(startgt_gt_comment($gt));
    my $json = "{ \"gt\" : \""
        . _json_escape($gt)
        . "\", \"tags\" : "
        . _json_format_string_array(@tags)
        . ", \"comment\" : \"$comment\" }\n";

    _log($ctx, "write global tag: $path (" . scalar(@tags) . " tags, sorted)");
    return if $ctx->{dry_run};
    open my $fh, ">", $path or die "startGT: cannot write $path: $!\n";
    print $fh $json;
    close $fh;
}

# ----------------------------------------------------------------------
sub startgt_alignment_tag_names {
    my ($gt) = @_;
    return map { startgt_alignment_tag_name($_, $gt) } @ALIGNMENT_CALIB_TYPES;
}

# ----------------------------------------------------------------------
sub startgt_write_tag_file {
    my ($ctx, $tags_dir, $tagname, $gt, $iovs) = @_;
    $iovs //= [$DEFAULT_IOV];
    my $iov_str = join(", ", @$iovs);
    my $comment = _json_escape(startgt_gt_comment($gt));
    my $json = "{ \"tag\" : \""
        . _json_escape($tagname)
        . "\", \"iovs\" : [$iov_str], \"comment\" : \"$comment\" }\n";
    my $path = "$tags_dir/$tagname";
    _log($ctx, "write tag: $path");
    return if $ctx->{dry_run};
    open my $fh, ">", $path or die "startGT: cannot write $path: $!\n";
    print $fh $json;
    close $fh;
}

# ----------------------------------------------------------------------
# Tag names follow cdbInitGT (mcideal: fibre/tile *_ideal; pixel always _<GT>).
sub startgt_quality_tag_name {
    my ($calib, $gt) = @_;
    my $flavour = startgt_gt_flavour($gt);
    if (($calib eq "fibrequality" || $calib eq "tilequality")
        && $flavour eq "mcideal" && $gt !~ /=/) {
        return "${calib}_ideal";
    }
    return "${calib}_${gt}";
}

# Suffix after calib_ — passed to cdbRunPayloadWriter -t for payload hash/dir.
sub startgt_quality_tag_suffix {
    my ($tagname) = @_;
    return $1 if $tagname =~ /^[^_]+_(.+)$/;
    return $tagname;
}

# Chip/tile/fibre ID filter for ideal *content* (perfect status, IOV 1).
# mcideal (no =year): full ideal detector via tag suffix (ideal / mcidealv6.9).
# mcrealistic / data / year-specific mcideal: installed components for that year.
sub startgt_quality_ideal_filter {
    my ($cfg, $gt, $tagname) = @_;
    my $flavour = startgt_gt_flavour($gt);
    if ($flavour eq "mcideal" && $gt !~ /=/) {
        return startgt_quality_tag_suffix($tagname);
    }
    my $year = startgt_year_from_gt($gt, $cfg);
    die "startGT: need conditions year for quality on '$gt' (=YYYY in name or conditions_year in config)\n"
        if $year eq "";
    return $year;
}

sub startgt_quality_payload_path {
    my ($payload_dir, $calib, $tagname, $iov) = @_;
    $iov //= $DEFAULT_IOV;
    my $suffix = startgt_quality_tag_suffix($tagname);
    my $hash   = "tag_${calib}_${suffix}_iov_${iov}";
    my $block  = sprintf("%04d", int($iov / 1000));
    return "$payload_dir/${calib}_${suffix}/$block/$hash";
}

sub startgt_quality_tag_names {
    my ($gt) = @_;
    return map { startgt_quality_tag_name($_, $gt) } @QUALITY_CALIB_TYPES;
}

sub startgt_write_quality_tags {
    my ($ctx, $cdb_root, $gt, @calibs) = @_;
    @calibs = @QUALITY_CALIB_TYPES unless @calibs;
    my $tags_dir    = "$cdb_root/tags";
    my $payload_dir = "$cdb_root/payloads";
    make_path($tags_dir) unless $ctx->{dry_run};

    my @written;
    _log($ctx, "write quality tag JSONs under $tags_dir");
    for my $calib (@calibs) {
        my $tagname = startgt_quality_tag_name($calib, $gt);
        my $payload = startgt_quality_payload_path($payload_dir, $calib, $tagname);
        if (!$ctx->{dry_run} && !-f $payload) {
            _log($ctx, "  skip $tagname (payload missing: $payload)");
            next;
        }
        startgt_write_tag_file($ctx, $tags_dir, $tagname, $gt);
        push @written, $tagname;
    }
    return @written;
}

sub startgt_stuff_payload_path {
    my ($payload_dir, $calib, $gt, $iov) = @_;
    $iov //= $DEFAULT_IOV;
    my $hash  = "tag_${calib}_${gt}_iov_${iov}";
    my $block = sprintf("%04d", int($iov / 1000));
    return "$payload_dir/${calib}_${gt}/$block/$hash";
}

sub startgt_write_stuff_tags {
    my ($ctx, $cdb_root, $gt, @calibs) = @_;
    @calibs = @STUFF_CALIB_TYPES unless @calibs;
    my $tags_dir    = "$cdb_root/tags";
    my $payload_dir = "$cdb_root/payloads";
    make_path($tags_dir) unless $ctx->{dry_run};

    my @written;
    _log($ctx, "write stuff tag JSONs under $tags_dir");
    for my $calib (@calibs) {
        my $tagname = startgt_stuff_tag_name($calib, $gt);
        my $payload = startgt_stuff_payload_path($payload_dir, $calib, $gt);
        if (!$ctx->{dry_run} && !-f $payload) {
            _log($ctx, "  skip $tagname (payload missing: $payload)");
            next;
        }
        startgt_write_tag_file($ctx, $tags_dir, $tagname, $gt);
        push @written, $tagname;
    }
    return @written;
}

sub startgt_write_alignment_tags {
    my ($ctx, $cdb_root, $gt, @calibs) = @_;
    @calibs = @ALIGNMENT_CALIB_TYPES unless @calibs;
    my $tags_dir    = "$cdb_root/tags";
    my $payload_dir = "$cdb_root/payloads";
    make_path($tags_dir) unless $ctx->{dry_run};

    my @written;
    _log($ctx, "write alignment tag JSONs under $tags_dir");
    for my $calib (@calibs) {
        my $tagname = startgt_alignment_tag_name($calib, $gt);
        my $payload = startgt_alignment_payload_path($payload_dir, $calib, $gt);
        if (!$ctx->{dry_run} && !-f $payload) {
            _log($ctx, "  skip $tagname (payload missing: $payload)");
            next;
        }
        startgt_write_tag_file($ctx, $tags_dir, $tagname, $gt);
        push @written, $tagname;
    }
    return @written;
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
sub startgt_gt_base {
    my ($cfg) = @_;
    my $base = _strip($cfg->{gt_base} // "");
    die "startGT: gt_base required in GT config (e.g. gt_base: \"v7.1\")\n" if $base eq "";
    return $base;
}

# IOV tag suffix for mcrealistic/data GTs, e.g. V0 → =2025V0 (conditions_year + conditions_tag).
sub startgt_conditions_tag {
    my ($cfg) = @_;
    return _strip($cfg->{conditions_tag} // $cfg->{conditions_version} // "V0");
}

# Build full global tag name from flavour + gt_base (+ conditions for data/mcrealistic).
sub startgt_build_gt_name {
    my ($cfg, $flavour) = @_;
    $flavour = lc(_strip($flavour));
    my $base = startgt_gt_base($cfg);

    if ($flavour eq "mcideal") {
        return "${flavour}${base}";
    }
    if ($flavour eq "mcrealistic" || $flavour eq "data") {
        my $year = startgt_conditions_year($cfg);
        die "startGT: conditions_year required in GT config for '$flavour' GT\n"
            if $year eq "";
        my $tag = startgt_conditions_tag($cfg);
        return "${flavour}${base}=${year}${tag}";
    }
    die "startGT: -g must be mcideal, mcrealistic, or data (got '$flavour')\n";
}

# Resolve -g argument (flavour shorthand) to full CDB global tag name.
sub startgt_resolve_gt {
    my ($cfg, $arg) = @_;
    $arg = lc(_strip($arg));
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $arg eq "";
    return startgt_build_gt_name($cfg, $arg);
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
    my $out = _strip($cfg->{alignment_output} // "");
    return "output/mu3e_alignment.root" if $out eq "";
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
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @ALIGNMENT_CALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

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
    _log($ctx, "cdbRunPayloadWriter alignment (@calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT (-t):   $gt");
    _log($ctx, "  filter:    $filter");
    _log($ctx, "  input:     $align");
    _log($ctx, "  payloads:  $payload_dir");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    if (@calibs == @ALIGNMENT_CALIB_TYPES) {
        my @cmd = ($exe, "-c", "alignment", "-f", $align, "-a", $ann, "-p", $payload_dir, "-t", $gt);
        push @cmd, "-m", $filter unless $filter eq "all";
        _run($ctx, @cmd);
    } else {
        for my $calib (@calibs) {
            my @cmd = ($exe, "-c", $calib, "-f", $align, "-a", $ann, "-p", $payload_dir, "-t", $gt);
            push @cmd, "-m", $filter unless $filter eq "all";
            _run($ctx, @cmd);
        }
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_alignment_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_quality_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @QUALITY_CALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $cdb_root    = startgt_ensure_test_cdb($ctx, $cfg);
    my $payload_dir = "$cdb_root/payloads";
    my $exe         = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $tmpdir = "$ctx->{run_dir}/output/startgt-quality";
    make_path($tmpdir) unless $ctx->{dry_run};

    my $cdb_dir = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter quality (perfect IOV 1, @calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT:        $gt");
    _log($ctx, "  payloads:  $payload_dir");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    for my $calib (@calibs) {
        my $tagname = startgt_quality_tag_name($calib, $gt);
        my $suffix  = startgt_quality_tag_suffix($tagname);
        my $filter  = startgt_quality_ideal_filter($cfg, $gt, $tagname);
        my $ann     = _strip($opts{annotation} // $cfg->{"${calib}_annotation"} // $QUALITY_ANNOTATIONS{$calib} // "");
        my $ext     = ($calib eq "tilequality") ? "json" : "csv";
        my $tmpfile = "$tmpdir/tmp-${calib}-${suffix}.${ext}";

        _log($ctx, "  $calib tag=$tagname filter=$filter");
        _run($ctx, $exe, "-m", "${calib}-ideal", "-f", $tmpfile, "-t", $filter);
        _run($ctx, $exe, "-c", $calib, "-f", $tmpfile, "-t", $suffix, "-p", $payload_dir, "-a", $ann);
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_quality_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_stuff_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @STUFF_CALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $cdb_root    = startgt_ensure_test_cdb($ctx, $cfg);
    my $payload_dir = "$cdb_root/payloads";
    my $exe         = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $cdb_dir = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter stuff (ideal IOV 1, @calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT (-t):   $gt");
    _log($ctx, "  payloads:  $payload_dir");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    for my $calib (@calibs) {
        my $meta = $STUFF_CALIB{$calib}
            or die "startGT: no stuff metadata for '$calib'\n";
        my $ideal = "$cdb_dir/$meta->{ideal_file}";
        die "startGT: $calib ideal input missing: $ideal\n"
            unless $ctx->{dry_run} || -f $ideal;
        my $ann = _strip($opts{annotation} // $cfg->{"${calib}_annotation"}
            // $meta->{annotation} // "");
        _log($ctx, "  $calib input: $ideal");
        _run($ctx, $exe, "-c", $calib, "-f", $ideal, "-t", $gt, "-p", $payload_dir, "-a", $ann);
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_stuff_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_mask_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @MASK_CALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $cdb_root = startgt_ensure_test_cdb($ctx, $cfg);
    my $exe      = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $mask_mode = startgt_pixelmask_mode($cfg, $gt);
    my $cdb_dir   = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter pixelmask (ideal IOV 1, @calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT (-t):   $gt");
    _log($ctx, "  CDB (-u):  $cdb_root");
    _log($ctx, "  mask -m:   $mask_mode");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    for my $calib (@calibs) {
        my $ann = _strip($opts{annotation} // $cfg->{"${calib}_annotation"}
            // $MASK_ANNOTATIONS{$calib} // "");
        _log($ctx, "  $calib");
        _run($ctx, $exe, "-m", $mask_mode, "-u", $cdb_root, "-t", $gt, "-a", $ann, "-r", "1");
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_calib_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_efficiency_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @EFFICIENCY_CALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $cdb_root    = startgt_ensure_test_cdb($ctx, $cfg);
    my $payload_dir = "$cdb_root/payloads";
    my $exe         = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $tmpdir = "$ctx->{run_dir}/output/startgt-efficiency";
    make_path($tmpdir) unless $ctx->{dry_run};

    my $cdb_dir = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter pixelefficiency (ideal IOV 1, @calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT:        $gt");
    _log($ctx, "  payloads:  $payload_dir");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    for my $calib (@calibs) {
        my $tagname = startgt_tag_name_for_calib($calib, $gt);
        my $suffix  = startgt_calib_tag_suffix($calib, $gt);
        my $filter  = startgt_ideal_input_filter($cfg, $gt, $calib);
        my $ann     = _strip($opts{annotation} // $cfg->{"${calib}_annotation"}
            // $EFFICIENCY_ANNOTATIONS{$calib} // "");
        my $tmpfile = "$tmpdir/tmp-${calib}-${suffix}.csv";

        _log($ctx, "  $calib tag=$tagname filter=$filter");
        _run($ctx, $exe, "-m", "${calib}-ideal", "-f", $tmpfile, "-t", $filter);
        _run($ctx, $exe, "-c", $calib, "-f", $tmpfile, "-t", $suffix, "-p", $payload_dir, "-a", $ann);
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_calib_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_timecalib_payloads {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @calibs = @{ $opts{calibs} // [] };
    @calibs = @TIMECALIB_TYPES unless @calibs;
    my $skip_global = $opts{skip_global_tag} ? 1 : 0;

    my $ctx = startgt_context($cfg, %opts);
    _ensure_run_dir($ctx);

    my $cdb_root    = startgt_ensure_test_cdb($ctx, $cfg);
    my $payload_dir = "$cdb_root/payloads";
    my $exe         = startgt_cdb_writer_exe($cfg);
    die "startGT: cdbRunPayloadWriter missing: $exe\n"
        unless $ctx->{dry_run} || -x $exe || -f $exe;

    my $tmpdir = "$ctx->{run_dir}/output/startgt-timecalib";
    make_path($tmpdir) unless $ctx->{dry_run};

    my $cdb_dir = startgt_cdb_code_basedir($cfg);
    _log($ctx, "cdbRunPayloadWriter time calibrations (ideal IOV 1, @calibs)");
    _log($ctx, "  cwd:       $cdb_dir");
    _log($ctx, "  GT:        $gt");
    _log($ctx, "  payloads:  $payload_dir");

    my $cwd = getcwd();
    chdir($cdb_dir) or die "Cannot chdir $cdb_dir: $!\n" unless $ctx->{dry_run};

    for my $calib (@calibs) {
        my $tagname = startgt_tag_name_for_calib($calib, $gt);
        my $suffix  = startgt_calib_tag_suffix($calib, $gt);
        my $filter  = startgt_ideal_input_filter($cfg, $gt, $calib);
        my $ext     = $TIMECALIB_EXT{$calib} // "calib";
        my $ann     = _strip($opts{annotation} // $cfg->{"${calib}_annotation"}
            // $TIMECALIB_ANNOTATIONS{$calib} // "");
        my $tmpfile = "$tmpdir/tmp-${calib}-${suffix}.${ext}";

        _log($ctx, "  $calib tag=$tagname filter=$filter");
        _run($ctx, $exe, "-m", "${calib}-ideal", "-f", $tmpfile, "-t", $filter);
        _run($ctx, $exe, "-c", $calib, "-f", $tmpfile, "-t", $suffix, "-p", $payload_dir, "-a", $ann);
    }

    chdir($cwd) unless $ctx->{dry_run};

    my @tag_names = startgt_write_calib_tags($ctx, $cdb_root, $gt, @calibs);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt, @tag_names) unless $skip_global;
    return @tag_names;
}

# ----------------------------------------------------------------------
sub startgt_run_alltags {
    my ($cfg, %opts) = @_;
    my $gt = _strip($opts{gt} // "");
    die "startGT: -g required (mcideal, mcrealistic, or data)\n" if $gt eq "";

    my @all = startgt_alltags_config($cfg);
    my %want = map { $_ => 1 } @all;

    my @alignment  = grep { $want{$_} } @ALIGNMENT_CALIB_TYPES;
    my @quality    = grep { $want{$_} } @QUALITY_CALIB_TYPES;
    my @stuff      = grep { $want{$_} } @STUFF_CALIB_TYPES;
    my @mask       = grep { $want{$_} } @MASK_CALIB_TYPES;
    my @efficiency = grep { $want{$_} } @EFFICIENCY_CALIB_TYPES;
    my @timecalib  = grep { $want{$_} } @TIMECALIB_TYPES;

    my $ctx = startgt_context($cfg, %opts);
    _log($ctx, "alltags for GT $gt (@all)");

    my %sub_opts = (%opts, skip_global_tag => 1);
    $sub_opts{calibs} = \@alignment if @alignment;
    startgt_run_alignment_payloads($cfg, %sub_opts) if @alignment;

    delete $sub_opts{calibs};
    $sub_opts{calibs} = \@quality if @quality;
    startgt_run_quality_payloads($cfg, %sub_opts) if @quality;

    delete $sub_opts{calibs};
    $sub_opts{calibs} = \@stuff if @stuff;
    startgt_run_stuff_payloads($cfg, %sub_opts) if @stuff;

    delete $sub_opts{calibs};
    $sub_opts{calibs} = \@mask if @mask;
    startgt_run_mask_payloads($cfg, %sub_opts) if @mask;

    delete $sub_opts{calibs};
    $sub_opts{calibs} = \@efficiency if @efficiency;
    startgt_run_efficiency_payloads($cfg, %sub_opts) if @efficiency;

    delete $sub_opts{calibs};
    $sub_opts{calibs} = \@timecalib if @timecalib;
    startgt_run_timecalib_payloads($cfg, %sub_opts) if @timecalib;

    my $cdb_root = startgt_ensure_test_cdb($ctx, $cfg);
    startgt_write_global_tag($ctx, $cfg, $cdb_root, $gt);
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
        my $gt = _strip($opts{gt});
        my $filter = startgt_payload_filter_mode($cfg, $gt);
        print("  GT flavour:        ", startgt_gt_flavour($gt), "\n");
        print("  GT (resolved):   $gt\n");
        print("  gt_base:           ", startgt_gt_base($cfg), "\n");
        print("  payload filter -m: $filter\n");
        print("  payloads dir:      $cdb_root/payloads (",
            (-d "$cdb_root/payloads" ? "yes" : "no"), ")\n");
        eval {
            my @calibs = startgt_alltags_config($cfg);
            print("  alltags config:    @calibs\n");
            for my $calib (@calibs) {
                my $tagname = startgt_tag_name_for_calib($calib, $gt);
                my $tagpath = "$cdb_root/tags/$tagname";
                print("  tag $tagname: ", (-f $tagpath ? "present" : "missing"), "\n");
            }
        };
        if ($@) {
            print("  alltags config:    (not set)\n");
        }
        my $gtpath = "$cdb_root/globaltags/$gt";
        print("  global tag:        $gtpath (", (-f $gtpath ? "present" : "missing"), ")\n");
    }

    require Setup;
    Setup::setup_status_config($cfg, %opts);
}

1;
