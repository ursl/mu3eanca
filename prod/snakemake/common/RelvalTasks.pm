package RelvalTasks;

use strict;
use warnings;
use Exporter qw(import);

our @EXPORT_OK = qw(load_relval_tasks);

# ----------------------------------------------------------------------
sub _normalize_mode {
    my ($item) = @_;
    my $mode = lc($item->{mode} // '');
    $mode =~ s/^\s+|\s+$//g;
    if ($mode eq 'sim' || $mode eq 'data') {
        return $mode;
    }
    if (_task_raw_input($item) ne '') {
        return 'data';
    }
    return 'sim';
}

# ----------------------------------------------------------------------
sub _task_raw_input {
    my ($item) = @_;
    my $raw = $item->{raw_input} // '';
    $raw =~ s/^\s+|\s+$//g;
    return $raw if $raw ne '';
    my $legacy = $item->{sort_input} // '';
    $legacy =~ s/^\s+|\s+$//g;
    return $legacy;
}

# ----------------------------------------------------------------------
sub _task_cdb_gt {
    my ($item, $default_gt) = @_;
    my $gt = $item->{cdb_GT} // $item->{cdb_gt} // '';
    $gt =~ s/^\s+|\s+$//g;
    return $gt ne '' ? $gt : ($default_gt // '');
}

# ----------------------------------------------------------------------
sub _migrate_sim_scenarios {
    my ($config) = @_;
    my $scenarios = $config->{sim_scenarios} // [];
    return () unless ref $scenarios eq 'ARRAY' && @$scenarios;
    my $default_gt = $config->{cdb_GT} // $config->{cdb_gt} // '';
    my @tasks;
    for my $item (@$scenarios) {
        my %task = %$item;
        $task{id} = $task{id} // die "sim_scenarios entry missing id\n";
        $task{mode} = 'sim';
        $task{cdb_GT} = _task_cdb_gt(\%task, $default_gt);
        push @tasks, \%task;
    }
    return @tasks;
}

# ----------------------------------------------------------------------
sub load_relval_tasks {
    my ($config) = @_;
    my $default_gt = $config->{cdb_GT} // $config->{cdb_gt} // '';
    my $default_run_id = 0 + ($config->{run_id} // 0);
    my $default_n_events = 0 + ($config->{n_events} // 0);

    my $raw = $config->{relval_tasks} // [];
    $raw = [] unless ref $raw eq 'ARRAY';

    my @tasks;
    if (@$raw) {
        for my $item (@$raw) {
            my %task = %$item;
            die "relval_tasks entry missing id\n" unless defined $task{id} && $task{id} ne '';
            $task{mode} = _normalize_mode(\%task);
            $task{cdb_GT} = _task_cdb_gt(\%task, $default_gt);
            die "relval_tasks/$task{id}: cdb_GT required (set per task or cdb_GT in config)\n"
                if $task{cdb_GT} eq '';
            if ($task{mode} eq 'sim') {
                die "relval_tasks/$task{id}: sim_conf required for mode sim\n"
                    unless defined $task{sim_conf} && $task{sim_conf} ne '';
            } else {
                die "relval_tasks/$task{id}: raw_input required for mode data\n"
                    if _task_raw_input(\%task) eq '';
            }
            die "relval_tasks/$task{id}: trirec_conf required\n"
                unless defined $task{trirec_conf} && $task{trirec_conf} ne '';
            $task{run_id} = 0 + ($task{run_id} // $default_run_id);
            $task{n_events} = 0 + ($task{n_events} // $default_n_events);
            die "relval_tasks/$task{id}: run_id must be > 0\n" if $task{run_id} <= 0;
            if ($task{mode} eq 'sim') {
                die "relval_tasks/$task{id}: n_events must be > 0 for mode sim\n"
                    if $task{n_events} <= 0;
            }
            push @tasks, \%task;
        }
    } else {
        @tasks = _migrate_sim_scenarios($config);
        die "no relval tasks: set relval_tasks or sim_scenarios in config\n" unless @tasks;
    }

    my %seen;
    for my $task (@tasks) {
        die "duplicate relval task id '$task->{id}'\n" if $seen{$task->{id}}++;
    }

    return \@tasks;
}

1;
