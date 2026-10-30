package Page::Abstract::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract;
use Common::TplCheckList;
use Sql::SessionData;
use Net::IP qw(ip_is_ipv4 ip_is_ipv6);
use XML::Simple qw(XMLout XMLin);
our @ISA = qw(Page::Abstract);
use config qw($CONFIG);
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::NetConfig;

# Method: _init (protected)
# Initialize variables
sub _init {
    my $s = shift;

    # Boitho::NetConfig refuses to construct unless the appliance network files
    # (ifcfg-ethN, resolv.conf, the setuid configwrite helper) all exist. In a
    # container or on a non-appliance host they do not, and dying here used to
    # turn the whole settings page into a 500. Remember the reason instead and
    # let the page explain that network configuration is unavailable.
    $s->{netConfig} = eval {
        Boitho::NetConfig->new($CONFIG->{'net_ifcfg'},
                $CONFIG->{'netscript_dir'},
                $CONFIG->{'configwrite_path'},
                $CONFIG->{'resolv_path'});
    };
    if (!$s->{netConfig}) {
        ($s->{netConfigError} = $@ || "unknown error") =~ s/ at \S+ line \d+\.?\s*$//;
    }

    $s->{sessData} = Sql::SessionData->new($s->{dbh});
}

##
# True when the network configuration files are available on this system.
sub network_available { defined $_[0]->{netConfig} }

# The Boitho::NetConfig instance, or croak with the reason it is unavailable.
sub _netconfig {
    my $s = shift;
    croak "Network configuration is not available on this system: $s->{netConfigError}"
        unless $s->{netConfig};
    return $s->{netConfig};
}

##
# Add variables into $vars that are 
# needed to show network config dialog.
#
# netconf_user_ref and resolv_user_ref are optional 
# params that overwrite the deaults.
sub show_network_config {
	my ($s, $vars, $netconf_usr_ref, $resolv_usr_ref) = @_;

	# get defaults (empty, with an explanation, when this system has no
	# appliance network configuration to read)
	my ($netconf_ref, $resolv_ref) = ({}, {});
	if ($s->network_available) {
		$netconf_ref = $s->{netConfig}->parse_netconf();
		$resolv_ref  = $s->{netConfig}->parse_resolv();
	}
	else {
		$vars->{network_unavailable} = 1;
		$vars->{network_error} = "Network configuration is not available on this system: "
			. $s->{netConfigError};
	}
	
	
	# overwrite defaults with user values
	while (my ($key, $value) = each %{$netconf_usr_ref}) {
		$netconf_ref->{$key} = $value;
	}
	while (my ($key, $value) = each %{$resolv_usr_ref}) {
		$resolv_ref->{$key} = $value;
	}

	# show values.
	$vars->{'netconf'} = $netconf_ref;
	$vars->{'resolv'}  = $resolv_ref;

	# Add checkList instance for error/success messages.
	$vars->{'checkList'} = Common::TplCheckList->new();
        1;
}



sub run_updates {
    my ($s, $restart_id, $netconf, $resolv) = @_;
    my @res;

    eval {
        $s->save_resolv($resolv);
        push @res, ["DNS settings saved", 1];

        $s->save_netconf($netconf);
        push @res, ["IP settings saved.", 1];
    };
    if ($@) {
        push @res, ["Error updating settings: " . $@, 0];
        return;
    }

    my $output;
    eval {
        $output = $s->_netconfig->restart();
        push @res, ["Network restarted", 1];
    };
    if ($@) {   
        push @res, ["Error during network restart: " . $@, 0];
        return;
    }
    $s->upd_net_results($restart_id, $output, @res);
}


sub get_restart_data {
    my ($s, $restart_id) = @_;
    croak "restart_id missing"
        unless defined $restart_id and $restart_id =~ /^\d+$/;
   
    my %nfo =  $s->{sessData}->get($restart_id);
    my %result = $nfo{data} ? %{XMLin($nfo{data})} : ();

    my $succs = 1;
    my $list = Common::TplCheckList->new();
    for my $r (@{$result{res}}) {
        $list->add($r->{msg}, $r->{succs});
        $succs = 0 if not $r->{succs};
    }
    return (
            update_succs => $succs,
            update_list => $list,
            restart_output => $result{restart_output},
    )

}
#




##
# Attributes:
#	settings_ref    - Users netconf values.
# 
# Croaks on error.
sub save_netconf {
    my ($s, $settings_ref) = @_;

    # Set BOOTPROTO to static, if user selected method: static.
    my $method = $settings_ref->{'method'};
    $settings_ref->{'BOOTPROTO'} = "static"
        if defined $method and ($method eq "static");

    $settings_ref->{'ONBOOT'} = "yes"; # start on boot - always.
    $settings_ref->{'DEVICE'} = $CONFIG->{'net_device'};

    # 'method' is not used in netconf,
    # only in the UI.
    delete $settings_ref->{'method'};

    $s->_netconfig->generate_netconf($settings_ref);
    $s->_netconfig->save_netconf();
    1;
}

##
# Attributes:
#	keywords_ref - Users resolv values.
#
# Croaks on error.
sub save_resolv {
    my ($s, $keywords_ref) = @_;
    $s->_clean_resolv($keywords_ref);
    $s->_netconfig->generate_resolv($keywords_ref);
    $s->_netconfig->save_resolv();
}

sub upd_net_results {
    my ($s, $restart_id, $restart_output, @res) = @_;
    @res = map { { msg => $_->[0],  succs => $_->[1] } } @res;
    $s->{sessData}->update($restart_id,
        data => XMLout({ 
            res => \@res, 
            restart_output => $restart_output,
        }, RootName => "results", NoAttr => 1)
    );
}
sub new_net_results { $_[0]->{sessData}->insert((type => "network")) }

##
# Remove blank values from resolv keys.
sub _clean_resolv {
	my ($s, $keywords_ref) = @_;

	foreach my $value_ref (values %{$keywords_ref}) {

		my $size = scalar @{$value_ref};
		for my $i (0..$size) {
			delete $value_ref->[$i] unless $value_ref->[$i];
		}
	}
}
1;
