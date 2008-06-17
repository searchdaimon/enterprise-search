## Boitho Administration configuration file
# Configuration file for bbadmin.
package config;
use strict;
use warnings;
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw($CONFIG %CONFIG);
our %CONFIG;
our $CONFIG = \%CONFIG; #backwards compatible..

# Group: Extra connectors
$CONFIG{conn_base_dir} = "/tmp/conn";

# Group: Template
# Tmp path for compiled templates, etc.
$CONFIG{tpl_tmp} = $ENV{BOITHOHOME} . "/var/webadmin_tmp";

# Group: MySQL configuration

# Path to the file containing info for establishing a MySQL connection.
$CONFIG{'config_path'} = $ENV{'BOITHOHOME'} . '/config/setup.txt';

# Group: Logs

# Path to where to look for logfiles.
$CONFIG{'log_path'} = $ENV{'BOITHOHOME'} . '/logs';

# Logfiles visible to the user.
# Syntax to add new one: 'filename' => "description",
$CONFIG{'logfiles'} = {
	'access_log' => "Web server access",
	'error_log' => "Web server error",
	'crawlManager_access' => "Crawling management access",
	'crawlManager_error' => "Crawling management error log",
	'indexing' => "Document indexing log",
	'crawl_watch.log' => "Collection management log",
	'boithoad_access' => "Active directory access log",
	'boithoad_error' => "Active directory error log",
        'bb_auto_update.log' => 'Blackbox auto update log',
        'searchdbb_stderr' => 'Search Error Log',
        'searchdbb_stdout' => 'Search Access Log',
	'gc' => 'Garbage Collection',

};


# Group: External utilites

# Path to the infoquery executable
$CONFIG{'infoquery'} = $ENV{'BOITHOHOME'} . '/bin/infoquery';

# Path to bb phone hone executable.
$CONFIG{'phone_home_path'} = $ENV{'BOITHOHOME'} . "/bin/setuidcaller";

# Scan, to generate ip list from ranges (not a SD developed utility)
$CONFIG{'genip_path'} = $ENV{BOITHOHOME} . "/Modules/Boitho/Scan/genip/genip";

# Group: Settings

# Values that are shown as "default values" under advanced settings.
# - name: name of the setting, as the user sees it.
# - table_key: what key to associate it with in the config database table.
# - original is the value reasonable default value.
$CONFIG{'default_settings'} = [
	{
		'name' => 'Crawling rate (in minutes)',
		'table_key' => 'default_crawl_rate',
		'original' => '1440'
	},
];

# Valid authentication methods for integration
$CONFIG{'valid_auth_methods'} = ['msad', 'ldap', 'shadow'];
$CONFIG{repomod_path} = $ENV{BOITHOHOME} . "/setuid/repomodwrap";
$CONFIG{dist_versions} = { production => 1, testing => 2, devel => 3 };


# Group: System

# Path to upload new rpm packages for installation
# - This should only be writeable to the http-process.
# - This also needs to be configured in the rpm-wrapper.
$CONFIG{'rpm_upload_folder'} = "/tmp/rpm";

# Init-services to include in webadmin
$CONFIG{'init_services'} = { 
    'crawlManager' => "Crawling Management", 
    'crawl_watch'  => "Collection Management",
    'searchdbb'    => "Search Service",
    'boithoad'     => "Authentication Daemon",
    'bbAutoUpdate' => "Blackbox Auto Update",
    'boitho-bbdn'  => "Document Management",
    'suggest'  => "Suggest",
};

# Path to init dir with bb services..
$CONFIG->{'init_dir'} = "/etc/init.d/";

# Path to init-services wrapper.
$CONFIG{'init_wrapper_path'} = $ENV{'BOITHOHOME'} . "/setuid/initwrapper";

# Path to yum wrapper
$CONFIG{'yum_wrapper_path'}  = $ENV{'BOITHOHOME'} . "/setuid/yumwrapper";

# Path to maplist.conf file.
$CONFIG{'maplist_path'} = $ENV{'BOITHOHOME'} . "/config/maplist.conf";

# Core dump directory
$CONFIG{'cores_path'}   = "/coredumps/saved";

# Commands to execute in gdb when generating a crash report
$CONFIG{'gdb_report_cmd'} = qq{bt};

# URL to submit crash reports to
$CONFIG->{'core_report_url'} = "http://dagurval.boitho.com/cgi-bin/report/submit.pl";

$CONFIG{login_htpasswd_path} = $ENV{BOITHOHOME} . "/cgi-bin/webadmin/.htpasswd"; 
    #"/home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd";
$CONFIG{login_admin_user} = "admin";

$CONFIG{adv_starred_fields} = ['msad_password', 'ldap_password', 'sudo'];


# Group: Network

# The network device that is configurable
$CONFIG{'net_device'} = "eth1";

# Name of network device configuration file
# Is hardcoded into configwrite.c and *needs also be changed there*
$CONFIG{'net_ifcfg'} = "ifcfg-eth1"; ## Must also be changed in configwrite.c

# Path to where the config file is
# Is hardcoded into configwrite.c and *needs also be changed there*
$CONFIG{'netscript_dir'} = "/etc/sysconfig/network-scripts"; ## Must also be changed in configwrite.c


# Path to configwrite executable
$CONFIG{'configwrite_path'} = $ENV{'BOITHOHOME'} . "/setuid/configwrite";

# Path to resolv.conf (used to alter dns server settings.)
$CONFIG{'resolv_path'} = "/etc/resolv.conf";

# Network interface to configure
$CONFIG{'network_interface'} = "eth1";



# Group: License authentication

$CONFIG{'bb_sha_test_path'} = $ENV{'BOITHOHOME'} . "/bin/bb_sha_test";
$CONFIG{'bb_verify_msg'}    = $ENV{'BOITHOHOME'} . "/bin/bb_verify_msg";

# URL to the search daimon interface
$CONFIG{'interface_url'} = "http://dagurval.boitho.com/cgi-bin/license_auth/if.pl";

# URL to the search daimon manual activation
$CONFIG{'activate_url'} = "http://dagurval.boitho.com/cgi-bin/license_auth/custom.pl";

# File to store license in
$CONFIG{'license_file'}   = $ENV{'BOITHOHOME'} . "/config/bb_license";
$CONFIG{'signature_file'} = $ENV{'BOITHOHOME'} . "/config/bb_signature";




1;
