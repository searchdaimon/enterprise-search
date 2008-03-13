
package Page::Abstract::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract;
use Common::TplCheckList;
use Net::IP qw(ip_is_ipv4 ip_is_ipv6);
our @ISA = qw(Page::Abstract);
use config qw($CONFIG);
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::NetConfig;

my $netConfig;

# Method: _init (protected)
# Initialize variables
sub _init {
	my $self = shift;

	$netConfig = Boitho::NetConfig
		->new($CONFIG->{'net_ifcfg'}, $CONFIG->{'netscript_dir'}, 
			  $CONFIG->{'configwrite_path'}, $CONFIG->{'resolv_path'});
}

##
# Add variables into $vars that are needed to show network config dialog.
#
# Attributes:
#	vars             - Template vars
#	netconf_user_ref - (Optional) user network settings to overwrite defaults with.
#   resolv_user_ref  - (Optional) resolv settings to overwrite defaults with.
#
# Returns:
#	vars - Template vars
sub show_network_config {
	my ($self, $vars, $netconf_user_ref, $resolv_user_ref) = @_;

	# get defaults
	my $netconf_defaults_ref = $netConfig->parse_netconf();
	my $resolv_defaults_ref  = $netConfig->parse_resolv();
	
	
	# overwrite defaults with user values
		#netconf
	while (my ($key, $value) = each %{$netconf_user_ref}) {
		$netconf_defaults_ref->{$key} = $value;
	}
		#resolv
	while (my ($key, $value) = each %{$resolv_user_ref}) {
		$resolv_user_ref->{$key} = $value;
	}

	# show values.
	$vars->{'netconf'} = $netconf_defaults_ref;
	$vars->{'resolv'}  = $resolv_defaults_ref;

	# Add checkList instance for error/success messages.
	$vars->{'checkList'} = Common::TplCheckList->new();
        1;
}

##
# Generate config files and attempt to save them.
#
# Attributes:
#	vars		  - Template vars
#	netconf_user_ref  - Users netconf values.
#	resolv_user_ref	  - Users resolv values.
sub update_network_settings {
	my ($self, $vars, $netconf_user_ref, $resolv_user_ref) = @_;
	my $restart_result;
        
	$netconf_user_ref->{'ONBOOT'} = "yes"; # device shall start on boot.
	$netconf_user_ref->{'DEVICE'} = $CONFIG->{'net_device'};
	
	# Attempt to generate and save config files.
        
        $self->_clean_resolv($resolv_user_ref); # remove empty settings.
	my ($resolv_succ, $resolv_errmsg) 
			= $self->_save_resolv($resolv_user_ref);
	my ($netconf_succ, $netconf_errmsg) 
			= $self->_save_netconf($netconf_user_ref);

                        # NESTE STEG: Lagre oppd resultat i db.

	
	# Restart the network if there was no error earlier.
	my ($restart_succ, $restart_message);
	if ($netconf_succ and $resolv_succ) {
		($restart_succ, $restart_message) = $self->_restart_network();
	}
	else {
		$restart_succ = 0;
		$restart_message = "Network not restarted due to earlier errors.";
	}

	# Store in template vars.
	$vars->{'netconf_succ'}  = $netconf_succ; 
	$vars->{'netconf_error'} = $netconf_errmsg;
	
	$vars->{'resolv_succ'}   = $resolv_succ;
	$vars->{'resolv_error'}  = $resolv_errmsg;

	$vars->{'restart_succ'}  = $restart_succ;
	$vars->{'restart_message'} = $restart_message;

	
	return 1 if $netconf_succ 
            and $resolv_succ and $restart_succ;
        return;
}

# Group: Private methods
##
# Method to restart network.
# Helper method for process_network_config.
#
# Returns:
#	(success, message) - Where success: True/false, Message: Restart output.
sub _restart_network {
	eval {
            return (1, $netConfig->restart());
	};
        return (0, $@) if $@
}


##
# Attempt to generate and save netconf config file.
# Helper method for process_network_config
#
# Attributes:
#	settings_ref    - Users netconf values.
#
# Returns:
#	(success, error) - List with true/false on success, and error message (if any).
sub _save_netconf {
	my ($self, $settings_ref) = @_;

	# Set BOOTPROTO to static, if user selected method: static.
	my $method = $settings_ref->{'method'};
	$settings_ref->{'BOOTPROTO'} = "static"
		if defined $method and ($method eq "static");

	# Remove key method, it's a not a valid netconf, but rather
	# a helper key for the user form.
	delete $settings_ref->{'method'};
	
	# Generate config file, and attempt to save it.
	eval {
		$netConfig->generate_netconf($settings_ref);
	};

	return (0, $@) if $@; # Don't save if generate failed.

	eval {
		$netConfig->save_netconf();
	};

	return ($@) ? (0, $@) : 1;
}

##
# Attempt to generate and save resolv config file.
# Helper method for process_network_config
#
# Attributes:
#	keywords_ref - Users resolv values.
#
# Returns:
#	(success, error) - List with true/false on success, and error message (if any).
sub _save_resolv {
	my ($self, $keywords_ref) = @_;
	
	eval {
		$netConfig->generate_resolv($keywords_ref);
	};
	return (0, $@) if $@; # Don't resplace resolv.conf if generate failed..
	
	eval {
		$netConfig->save_resolv();
	};
	return ($@) ? (0, $@) : 1;
	
}

##
# Remove blank values from keys.
# Helper function for process_network_config
#
# Attributes:
#	keywords_ref - Users resolv values.
sub _clean_resolv {
	my ($self, $keywords_ref) = @_;

	foreach my $value_ref (values %{$keywords_ref}) {

		my $size = scalar @{$value_ref};
		for my $i (0..$size) {
			delete $value_ref->[$i] unless $value_ref->[$i];
		}
	}
}
1;
