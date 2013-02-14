# Class: Boitho::NetConfig
# Configure network interface on Fedora 3
package Boitho::NetConfig;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Net::IP qw(ip_is_ipv4 ip_is_ipv6);
use FileHandle;
#use IPC::Open2;

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
	croak "Must provide 4 parameters." unless @_ == 4;
	my ($ifcfg, $netscript_dir, $configwrite_path, $resolv_path) = @_;

	croak "Config file $netscript_dir/$ifcfg does not exist."
		unless -e "$netscript_dir/$ifcfg";

	croak "Resolv config file $resolv_path does not exist."
		unless -e "$resolv_path";
		
	croak "Configwrite, $configwrite_path, is not executable"
		unless -x $configwrite_path;

	$self->{'netscript_dir'} = $netscript_dir;
	$self->{'ifcfg'} = $ifcfg;
	$self->{'configwrite_path'} = $configwrite_path;
	$self->{'resolv_path'} = $resolv_path;
	1;
}

sub parse {
    croak "parse() is deprecated. Use parse_netconf() instead."
}

##
# Parse Fedora network config into a hash
#
# Returns:
# 	config_ref - Hashref with config values.
sub parse_netconf {
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
# Parse resolv.conf into a hash
#
# Returns:
#   config_ref - Hashref with resolv values.
#		 Values are in ARRAYs, because resolv.conf supports 
#		 more than one value for a single keyword.
sub parse_resolv {
    my $self = shift;
    open my $resolv_h, "<", $self->{'resolv_path'}
	or croak "Unable to open resolv file: $!";

    my %config;

    foreach my $line (<$resolv_h>) {
	chomp $line;
	next unless $line; # blank
	my ($key, $value) = split " ", $line;
	push @{$config{$key}}, $value;
    }

    return \%config;
}

sub generate {
    croak "generate() is deprecated. Use generate_netconf()";
}


##
# Generate config file. Stores config in instance, also returns it.
#
# Attributes:
# 	options_ref - Hash with config options.
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
sub generate_netconf {
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


## 
# Generate resolv.conf file. Store config instane, also returns it.
# Does not support all keywords that resolv.conf supports. See section "Keywords" for full list.
#
# Attributes:
#   keywords_ref - Hash with resolv keywords and their values.
#
# Keywords:
#   nameserver - IP to nameserver. Can contain a listref with more than one ip.
#
# Returns:
#   resolv_content - Content for generated file.
sub generate_resolv {
    my ($self, $keyword_ref) = @_;

    # validate
    $self->_validate_resolv_keywords($keyword_ref);

    # generate file content
    my %keywords = %{$keyword_ref};
    my $resolv_content;
    while (my ($key, $value_ref) = each %keywords) {
	foreach my $value (@{$value_ref}) {
	    $resolv_content .= "$key $value\n";
	}
    }

    $self->{'resolv_content'} = $resolv_content;
    return $resolv_content;

}

sub save {
    croak "save() is deprecated. Use save_netconf() instead.";
}


##
# Save netconf config file.
# Overwrites old config file.
sub save_netconf {
	my $self = shift;
	my $content = $self->{'config_content'};

	croak "No content to write. You must run generate_netconf() first."
	    unless defined $content;

	$self->_save_file($content, "netconfig");
	1;
}

sub save_resolv {
    my $self = shift;
    my $content = $self->{'resolv_content'};
	
    croak "No content to write. You must run generate_resolv() first."
	    unless defined $content;

    $self->_save_file($content, "resolv");
    1;
}


##
# Generic save-file method for passing on data to the confwrite wrapper.
# Croaks on write error.
#
# Attributes:
#   content - Config content to write.
#   keyword - What file to write to. See Keywords header for available keywords.
# 
# Keywords:
#   netconf - For writing to netconfig file.
#   resolv  - For writing to resolv config file.
sub _save_file {
    my ($self, $content, $keyword) = @_;

    my @valid_keywords = qw(netconfig resolv);
    croak "Unknown keyword $keyword provided."
	unless grep { /^$keyword$/ } @valid_keywords;
    
    croak "content argument not provided"
	unless defined $content;

    croak "keyword argument not provided"
	unless defined $keyword;

    my $path = $self->{'configwrite_path'};
    
    my $success = 1;

    #write
    open my $configwriterh, "|$path $keyword"
	or die "Unable to execute configwrite: $!\n";
    print { $configwriterh } $content;
    #print { $configwriterh } eof;
    close $configwriterh 
	or $success = 0;
	
    unless ($success) {
	croak "Configwriter was unable to write to configfile. (See apache error log for details)";
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
	
	croak "Unable to restart network: $input\n"
		unless $success;
		
	return $input;
}


# Group: Private Methods

##
# Validate keywords. Croaks on error.
# Note: Remember to croak on newline (\n) in a keys value, so user can't inject unchecked lines into config file.
#
# Attributes:
#   keywords_ref - Hash with resolv keywords and their values. 
sub _validate_resolv_keywords {
    my ($self, $keywords_ref) = @_;
    my %keywords = %{$keywords_ref};
    
    # Supported keywords
    my @valid_keywords = qw(nameserver);
    
    # Valdate
    foreach my $key (keys %keywords) {
	croak "Not supported resolv keyword $key used\n"
	    unless grep { /^$key$/  } @valid_keywords;
    }

    # Validate namesever
    if (defined $keywords{'nameserver'}) {
	my $value = $keywords{'nameserver'};
	
	if (ref($value) eq "SCALAR") {
	    croak "$value is not a valid nameserver IP"
		unless ip_is_ipv4($value) or ip_is_ipv6($value);
	}

	elsif (ref($value) eq "ARRAY") {
	    foreach my $ip (@{$value}) {
		croak "$ip is not a valid nameserver IP"
		    unless ip_is_ipv4($ip) or ip_is_ipv6($value);
	    }
	}

    }
}

##
# Validate configfile options. Croaks on error.
# NOTE: Does not check the contents of IP, Nemask or Gateway if using dhcp.
#
# Attributes:
#	options_ref - Hash with config options
sub _validate_ifcfg_options {
	my ($self, $options_ref) = @_;
	my %options = %$options_ref;


	# Known option check
	my @valid_options = qw(GATEWAY NAME BOOTPROTO DEVICE MTU
				NETMASK BROADCAST IPADDR NETWORK ONBOOT);

	foreach my $option (keys %options) {
		croak "Unknown option $option"
			unless grep { /^$option$/ } @valid_options;
	}

	# Valid BOOTPROTO
	if (defined $options{'BOOTPROTO'}) {
		my @valid_values = qw(static none dhcp bootp);
		croak "$options{BOOTPROTO} is not a valid connection (BOOTPROTO) method"
			unless grep { /^$options{'BOOTPROTO'}$/ } @valid_values;
	}

	# Gateway, netmask etc can be empty if we're using dhcp.
	my $using_dhcp;
	$using_dhcp = 1 if $options{'BOOTPROTO'} eq "dhcp";
	# Valid GATEWAY NETMASK BROADCAST IPADDR
        if (not $using_dhcp) {
            foreach my $option (qw(GATEWAY NETMASK BROADCAST IPADDR NETWORK)) {
                my $ip = $options{$option};
                next unless defined $ip;

		croak "$option: $ip is not a valid IP."
			unless ip_is_ipv4($ip) or ip_is_ipv6($ip);
	    }
        }


	# Valid ONBOOT
	if (defined $options{'ONBOOT'}) {
		my $value = $options{'ONBOOT'};
		croak "Not valid ONBOOT value. Must be yes or no"
			unless (($value eq "yes") or ($value eq "no"));
	}

	# Valid network device
	#if (defined $option{'DEVICE'}) {
	#runarb: ser ut til og ha glemt en s her. Skal være optionS, ikke option
	if (defined $options{'DEVICE'}) {
	    my $value = $options{'DEVICE'};
	    my @valid_devices = qw(eth0 eth1);
	    croak "$value is not a valid network device"
		unless grep { /^$value$/ } @valid_devices;
	}
	1;
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
          'DEVICE' => 'eth0',
          'MTU' => '""',
          'NETMASK' => '255.255.255.0',
          'BROADCAST' => '192.168.1.255',
          'IPADDR' => '192.168.1.51',
          'NETWORK' => '192.168.1.0',
          'ONBOOT' => 'yes',
	 });
}

1;
