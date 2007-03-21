package Page::Abstract::Network;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Abstract;
our @ISA = qw(Page::Abstract);
use config qw($CONFIG);
BEGIN {
	push @INC, "Modules";
}
use Boitho::NetConfig;

# Method: _init (protected)
# Initialize variables
sub _init {
	my $self = shift;
	$self->{'netConfig'} 
		= Boitho::NetConfig->new(	$CONFIG->{'net_ifcfg'},
									$CONFIG->{'netscript_dir'},
									$CONFIG->{'configwrite_path'});

}

##
# Add variables into $vars that are needed to show network config dialog.
#
# Attributes:
#	vars - Template vars
#	user_settings_ref - (Optional) user settings to overwrite defaults with.
#	template_file - Template file to return
#
# Returns:
#	vars - Template vars
#	templafe_file - Template file
sub show_network_config {
	my ($self, $vars, $user_settings_ref, $template_file) = @_;
	
	# get defaults
	my $netConfig = $self->{'netConfig'};
	my $defaults_ref = $netConfig->parse();
	
	# overwrite with user
	while (my ($key, $value) = each %$user_settings_ref) {
		$defaults_ref->{$key} = $value;
	}
	$vars->{'netconf'} = $defaults_ref;
	return ($vars, $template_file);
}


sub process_network_config {
	my ($self, $vars, $user_settings_ref) = @_;
	my $netConfig = $self->{'netConfig'};
	my $restart_result;
	
	# Override BOOTPROTO if static method is selected. Remove method parameter, it does not belong.
	my $method = $user_settings_ref->{'method'};
	$user_settings_ref->{'BOOTPROTO'} = "static"
		if defined $method and ($method eq "static");
	delete $user_settings_ref->{'method'};
	
	# attempt to store.
	eval {
		$netConfig->generate($user_settings_ref);
		$netConfig->save();
		$restart_result = $netConfig->restart();
	};
	my $error = ($@) ? $@ : undef;
	carp $error if $error;
	
	$vars->{'net_restart'} = $restart_result
		if $restart_result;
	
	if ($error) {
		$vars->{'error_msg_net'} = $error;
		return ($vars, 0);
	}
	else {
		# Show next step.
		$vars->{'succ_msg_net'} = 1;
		return ($vars, 1);
	}
}