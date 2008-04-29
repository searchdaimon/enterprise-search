package Page::Settings::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract::Network;

our @ISA = qw(Page::Abstract::Network);

use constant NETWORK_TPL => "settings_network.html";
use constant RESTART_TPL => "settings_network_restart.html";

##
#
# See <Page::Abstract::Network>
sub show_network_config {
	my ($self, $vars, $user_settings_ref, $user_resolv_ref) = @_;
	return ($self->SUPER::show_network_config($vars, $user_settings_ref, $user_resolv_ref), NETWORK_TPL);
}

sub show_restart {
    my ($s, $vars, $netcfg) = @_;
    my $restart_id = $s->new_net_results();

    my $url = qw{};
    if ($netcfg->{method} eq 'static') {
        $url .= "http://$netcfg->{IPADDR}/cgi-bin/webadmin/";
    }
    $url .= "settings.cgi?view=network_restart&amp;restart=$restart_id";
    $vars->{return_url} = $url;

    return (RESTART_TPL, $restart_id);
}

sub show_post_restart {
    my ($s, $vars, $restart_id) = @_;
    my %data = $s->get_restart_data($restart_id);
    $vars->{$_} = $data{$_} for keys %data;
    $vars->{post_restart} = 1;
    return RESTART_TPL;
}


1;
