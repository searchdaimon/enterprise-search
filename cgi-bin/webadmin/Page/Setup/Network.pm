package Page::Setup::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;

our @ISA = qw(Page::Abstract::Network);

##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $user_settings_ref) = @_;
	my $template_file = "setup_network_config.html";
	return $self->SUPER::show_network_config($vars, $user_settings_ref, $template_file);
}

1;