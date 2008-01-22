##
# Page for altering configuration for crawl_watch.
package Page::Settings::CollectionManager;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;
use Sql::Config;

our @ISA = qw(Page::Abstract);

use constant DEFAULT_TPL => "settings_collectionmanager.html";
use constant CM_CONFIGKEYS => qw(
    gc_default_rate gc_last_run
    recrawl_schedule_start recrawl_schedule_end
    cm_crawl_recheck_rate suggdict_run_hour suggdict_last_run
);

my $sqlConf;
sub _init {
    my $self = shift;
    $sqlConf = Sql::Config->new($self->{dbh});
}

sub show {
    my ($self, $vars) = @_,

    my %config;
    foreach my $key (CM_CONFIGKEYS) {
        $config{$key} = $sqlConf->get_setting($key);
    }
    $vars->{config} = \%config;
    
    return DEFAULT_TPL;
}

sub update_gc {
    my ($self, $vars, $gc_rate) = @_;

    if ($gc_rate =~ /^\d+$/) {
        $sqlConf->insert_setting('gc_default_rate', $gc_rate);
        $vars->{gc_updated} = 1;
    }
    else { croak "Wrong gc_rate format, $gc_rate" }
    
    return $self->show($vars);
}

sub update_schedule {
    my ($self, $vars, $use_schedule, $start, $end) = @_;

    unless ($use_schedule) {
        $sqlConf->insert_setting('recrawl_schedule_start', q{});
        $sqlConf->insert_setting('recrawl_schedule_end', q{});
        $vars->{schedule_updated} = 1;
        return $self->show($vars);
    }

    eval {
        croak "Invalid schedule format"
            unless $start =~ /^\d+$/ and $end =~/^\d+$/;
        croak "Invalid schedule hours"
            unless ($start <= 24 and $start > 0) and
            ($end <= 24 and $end > 0);
    };
    if ($@) {
        $vars->{schedule_update_err} = $@;
        $vars->{schedule_update_err} =~ s/at .*? line \d+//;
        return $self->show($vars);
    }
        

    $sqlConf->insert_setting('recrawl_schedule_start', $start);
    $sqlConf->insert_setting('recrawl_schedule_end', $end);
    $vars->{schedule_updated} = 1;
    return $self->show($vars);
    
}

sub update_suggdict {
    my ($self, $vars, $run_hour) = @_;
    croak "Invalid suggdict rebuild hour"
        unless ($run_hour =~ /^\d+$/
            and $run_hour >= 1
            and $run_hour <= 24);

    $sqlConf->insert_setting('suggdict_run_hour', $run_hour);
    $vars->{suggdict_updated} = 1;
    return $self->show($vars);
}

1;
