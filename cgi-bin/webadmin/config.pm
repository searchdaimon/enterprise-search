## Boitho Administration configuration file
# Namespace: config
# Configuration file for bbadmin.
package config;
use strict;
use warnings;
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw($CONFIG);
our $CONFIG = {};

# Group: MySQL configuration

# Path to the file containing info for establishing a MySQL connection.
$CONFIG->{'config_path'} = $ENV{'BOITHOHOME'} . '/config/setup.txt';

# Group: Logs

# Path to where to look for logfiles.
$CONFIG->{'log_path'} = $ENV{'BOITHOHOME'} . '/logs';

# Logfiles visible to the user.
# Syntax to add new one: 'filename' => "description",
$CONFIG->{'logfiles'} = {
	'access_log' => "http access log",
	'error_log' => "http error log",
	'crawlManager_access' => "crawlManager access log",
	'crawlManager_error' => "crawlManager error log",
	'indexing' => "index log",
};


# Group: External utilites

# Path to the infoquery executable
$CONFIG->{'infoquery'} = $ENV{'BOITHOHOME'} . '/bin/infoquery';

# Path to bb phone hone executable.
$CONFIG->{'phone_home_path'} = $ENV{'BOITHOHOME'} . "/bin/setuidcaller";


# Group: Settings

# Values that are shown as "default values" under advanced settings.
# - name: name of the setting, as the user sees it.
# - table_key: what key to associate it with in the config database table.
# - original is the value reasonable default value.
$CONFIG->{'default_settings'} = [
	{
		'name' => 'Crawling rate (in minutes)',
		'table_key' => 'default_crawl_rate',
		'original' => '1440'
	},
];

# Valid authentication methods for integration
$CONFIG->{'valid_auth_methods'} = ['msad', 'ldap', 'shadow'];


# Group: System

# Path to upload new rpm packages for installation
# - This should only be writeable to the http-process.
# - This also needs to be configured in the rpm-wrapper.
$CONFIG->{'rpm_upload_folder'} = "/tmp/rpm";

# Init-services to include in webadmin
$CONFIG->{'init_services'} = ['crawlManager', 'boitho-bbdn', 'searchdbb'];

# Group: Network


# Name of network device configuration file
# Is hardcoded into configwrite.c and *needs also be changed there*
$CONFIG->{'net_ifcfg'} = "ifcfg-eth1"; ## Must also be changed in configwrite.c

# Path to where the config file is
# Is hardcoded into configwrite.c and *needs also be changed there*
$CONFIG->{'netscript_dir'} = "/etc/sysconfig/network-scripts"; ## Must also be changed in configwrite.c


# Path to configwrite executable
$CONFIG->{'configwrite_path'} = $ENV{'BOITHOHOME'} . "/Modules/Boitho/NetConfig/configwrite";


# Network interface to configure
$CONFIG->{'network_interface'} = "eth1";



# Group: License authentication

# Path to the software that generates hardware-hash.
$CONFIG->{'hwgen_path'} = "/bin/date";

# Path to the software that validates authentication code.
$CONFIG->{'authvalid_path'} = "/bin/true";

# URL to the search daimon interface
$CONFIG->{'interface_url'} = "http://dagurval.boitho.com/cgi-bin/license_auth/if.pl";

# URL to the search daimon manual activation
$CONFIG->{'activate_url'} = "http://dagurval.boitho.com/cgi-bin/license_auth/custom.pl";







## CONFIGURATION DONE. Please don't edit anything below. ##

# Group: Validation
# Code to check that everything is configured.
my @config_settings = qw(config_path log_path logfiles infoquery phone_home_path
				 default_settings valid_auth_methods rpm_upload_folder net_ifcfg 
				 netscript_dir network_interface configwrite_path hwgen_path authvalid_path
				interface_url activate_url);

conf_check(@config_settings);
##
# Croak if any config setting is missing.
#
# Attributes:
#	@settings - Settings to check
sub conf_check {
	my @settings = @_;
	
	foreach my $setting (@settings) {
		die "Setting $setting has not been configure. Please edit config.pm"
			unless defined $CONFIG->{$setting};
	}
}


1;
