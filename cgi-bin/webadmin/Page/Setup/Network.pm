package Page::Setup::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;

our @ISA = qw(Page::Abstract::Network);

use constant TPL_SETUP_NETCONF => "setup_network_config.html";

##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $user_settings_ref, $resolv_keywords_ref) = @_;
	
	return ($self->SUPER::show_network_config($vars, $user_settings_ref, $resolv_keywords_ref), TPL_SETUP_NETCONF);
}

1;