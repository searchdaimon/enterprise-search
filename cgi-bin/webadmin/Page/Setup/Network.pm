package Page::Setup::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;
use Sql::SessionData;
use Common::TplCheckList;

our @ISA = qw(Page::Abstract::Network);

use constant TPL_SETUP_NETCONF => "setup_network_config.html";
use constant TPL_RESTART => "setup_network_restart.html";

##
# Shows 'network restarting' page.
sub show_restart {
    my ($s, $vars, $netcfg) = @_;

    my $restart_id = $s->new_net_results();

    my $url = qw{};
    if ($netcfg->{method} eq 'static') {
        $url .= "http://$netcfg->{IPADDR}/cgi-bin/webadmin/";
    }
    $url .= "setup.cgi?view=network_restart&id=$restart_id";
    
    $vars->{return_url} = $url;

    return (TPL_RESTART, $restart_id);
}

sub show_post_restart {
    my ($s, $vars, $restart_id) = @_;
    croak "restart_id missing"
        unless defined $restart_id and $restart_id =~ /^\d+$/;
    
    my %data = $s->get_restart_data($restart_id);
    for (keys %data) {
        $vars->{$_} = $data{$_};
    }
    $vars->{post_restart} = 1;
    return TPL_RESTART;
}    

##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $usr_settings, $resolv) = @_;
	
	return (
            $self->SUPER::show_network_config(
                $vars, $usr_settings, $resolv),
            TPL_SETUP_NETCONF);
}

1;
