package Page::Setup::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;

our @ISA = qw(Page::Abstract::Network);

use constant TPL_SETUP_NETCONF => "setup_network_config.html";
use constant TPL_RESTART => "setup_network_restart.html";

##
# Shows 'network restarting' page.
sub show_restart {
    my ($s, $vars, $netcfg) = @_;

    my $restart_id = $s->{sessDataSql}
        ->insert((type => 'network'));

    my $url = qw{};
    if ($netcfg->{method} eq 'static') {
        $url .= "http://$netcfg->{IPADDR}/";
    }
    $url .= "setup.cgi?view=network_restart&id=$restart_id";

    return TPL_RESTART;
}

sub restart_network {
    my ($s, $vars, $restart_id, $netconf, $resolv) = @_;
}


##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $user_settings_ref, $resolv_keywords_ref) = @_;
	
	return ($self->SUPER::show_network_config($vars, 
            $user_settings_ref, $resolv_keywords_ref), TPL_SETUP_NETCONF);
}

1;
