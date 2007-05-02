package Page::Settings::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;

our @ISA = qw(Page::Abstract::Network);

use constant NETWORK_TPL => "settings_network.html";

##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $user_settings_ref, $user_resolv_ref) = @_;
	return ($self->SUPER::show_network_config($vars, $user_settings_ref, $user_resolv_ref), NETWORK_TPL);
}

1;