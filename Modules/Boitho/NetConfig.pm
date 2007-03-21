# Class: Boitho::NetConfig
# Configure network interface on Fedora 3
package Boitho::NetConfig;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Net::IP qw(ip_is_ipv4 ip_is_ipv6);
use FileHandle;
use IPC::Open2;

# Constructor: new
#
# Attributes:
#	ifcfg - Name of IF config file
#	netscript_dir - Path to config directory
sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->_init(@_);
	return $self;

}

# Method: init (private)
# Initialize.
#
# Attributes:
#	ifcfg - Name of IF config file
#	netscript-dir - Path to config directory
sub _init {
	my $self = shift;
	croak "Must provide 3 parameters." unless @_ == 3;
	my ($ifcfg, $netscript_dir, $configwrite_path) = @_;

	croak "Config file $netscript_dir/$ifcfg does not exist."
		unless -e "$netscript_dir/$ifcfg";
		
	croak "Configwrite, $configwrite_path, is not executable"
		unless -x $configwrite_path;

	$self->{'netscript_dir'} = $netscript_dir;
	$self->{'ifcfg'} = $ifcfg;
	$self->{'configwrite_path'} = $configwrite_path;
	1;
}

##
# Parse config into a hash
#
# Returns:
# 	config_ref - Hashref with config values.
sub parse {
	my $self = shift;
	my $full_path = $self->{'netscript_dir'} . "/" . $self->{'ifcfg'};
	open my $cfgfile_h, "<", $full_path
		or croak "Unable to open config file: $!";

	my %config; 
	
	CONFIG_LINE:
	for my $line (<$cfgfile_h>) {
		chomp $line;
		next unless $line; #blank
		unless ($line =~ /^([a-z]|[A-Z])+=\S+$/) { # matches <characters>=<not space>
			warn "Parser doesn't understand $line, ignoring.";
			next CONFIG_LINE;
		}
		my ($key, $value) = split "=", $line;
		$config{$key} = $value;
	}
	
	return \%config;
}

##
# Generate config file. Stores config in instance, also returns it.
#
# Attributes:
# 	options_ref = Hash with config options.
#
# Options:
#	GATEWAY - Gateway (IP)
#	NAME - Device name
#	BOOTPROTO - Protocol (possible: none static dhcp bootp)
#	DEVICE - Network device
#	MTU - Maximum Transfer Unit (see man ifconfig)
#	NETMASK - Netmask (IP)
#	BROADCAST - Broadcast (IP)
#	IPADDR - IP for network device
#	NETWORK - Network (IP)
#	ONBOOT - Start on boot
# Returns:
#	config_content = String with config file.
sub generate {
	my ($self, $options_ref) = @_;
	
	$self->_validate_ifcfg_options($options_ref);

	my %options = %$options_ref;
	my $config_content;
	while (my ($key, $value) = each %options) {
		$config_content .= $key . "=" . $value . "\n";
	}

	$self->{'config_content'} = $config_content;
	return $config_content;
}

# Group: Private Methods

##
# Validate configfile options. Croaks on error.
#
# Attributes:
#	options_ref - Hash with config options
sub _validate_ifcfg_options {
	my ($self, $options_ref) = @_;
	my %options = %$options_ref;


	# Known option check
	my @valid_options = qw(GATEWAY NAME BOOTPROTO DEVICE MTU
				NETMASK BROADCAST IPADDR NETWORK ONBOOT);

	foreach my $option (@valid_options) {
		croak "Unknown option $option"
			unless grep /^$option$/, @valid_options;
	}

	# Valid BOOTPROTO
	if (defined $options{'BOOTPROTO'}) {
		my @valid_values = qw(static none dhcp bootp);
		croak unless grep /^$options{'BOOTPROTO'}$/, @valid_values;
	}

	# Valid GATEWAY NETMASK BROADCAST IPADDR
	foreach my $option (qw(GATEWAY NETMASK BROADCAST IPADDR NETWORK)) {
		next unless defined $options{$option};
		last if grep /^$options{'BOOTPROTO'}$/, "dhcp", "bootp"; # Ignore if it doesn't matter.
		my $ip = $options{$option};
		croak "$option: $ip is not a valid IP."
			unless ip_is_ipv4($ip) or ip_is_ipv6($ip);
	}


	# Valid ONBOOT
	if (defined $options{'ONBOOT'}) {
		my $value = $options{'ONBOOT'};
		croak "Not valid ONBOOT value. Must be yes or no"
			unless (($value eq "yes") or ($value eq "no"));
	}
	
	1;
}

##
# Save config file.
# Overwrites config file.
sub save {
	my $self = shift;
	my $content = $self->{'config_content'};
	my $path = $self->{'configwrite_path'};
	
	my $success = 1;
	
	open2 (*CWREAD, *CWWRITE, "$path")
		or die "Unable to execute configwrite: $!\n";
	print CWWRITE $content;
	print CWWRITE eof;
	close CWWRITE or $success = 0; 
	my $input = <CWREAD>;
	close CWREAD  or $success = 0;
	
	
	unless ($success) {
		die "Error writing config: $input";
	}

	
	1;
}

##
# Restart network connection
#
# Returns:
#	input - Text printed by restart procedure.
sub restart {
	my $self = shift;
	my $path = $self->{'configwrite_path'};
	my $success = 1;
	
	open my $cw_h, "$path restart|"
		or croak "Unable to execute configwrite: $!\n";
	
	my @input_raw = <$cw_h>;
	close $cw_h or $success = 0;
	
	my $input = join "\n", @input_raw;
	
	die "Unable to restart network: $input\n"
		unless $success;
		
	return $input;
}


use constant TEST => 0;
if (TEST) {
	my $test = Boitho::NetConf->new();
	print "Parse config:\n";
	print Dumper($test->parse_ifcfg());
	print "New config:\n", $test->generate_ifcfg(
	 {	
	  'GATEWAY' => '192.168.1.99',
          'BOOTPROTO' => 'none',
          'DEVICE' => 'eth1',
          'MTU' => '""',
          'NETMASK' => '255.255.255.0',
          'BROADCAST' => '192.168.1.255',
          'IPADDR' => '192.168.1.51',
          'NETWORK' => '192.168.1.0',
          'ONBOOT' => 'yes',
	 });
}

1;