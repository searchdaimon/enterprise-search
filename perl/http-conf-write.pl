#!/usr/bin/env perl

use warnings;
use strict;

my $authtype = $ARGV[0] || "msad";


sub write_authdata($) {

}

# Blackbox host header
print <<EOF;
<VirtualHost *>
	SetEnv BOITHOHOME /home/boitho/boithoTools

	DocumentRoot /home/boitho/boithoTools/public_html
	ServerName bbdemo.boitho.com
	ServerAdmin webmaster\@boitho.com
	ScriptAlias /cgi-bin/ /home/boitho/boithoTools/cgi-bin/
	ErrorLog /home/boitho/boithoTools/logs/error_log
	CustomLog /home/boitho/boithoTools/logs/access_log "%h %l %u %t \"%r\" %s %b \"%{Referer}i\" \"%{User-agent}i\""
	LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined
	LogFormat "%h %l %u %t \"%r\" %>s %b" common
	LogFormat "%{Referer}i -> %U" referer
	LogFormat "%{User-agent}i" agent

EOF

my $serveralias = $ARGV[1];

if (defined($serveralias)) {
	print "\tServerAlias $serveralias\n\n";
}

# CGI for webadmin and so on
print <<EOF;
	<Directory "/home/boitho/boithoTools/cgi-bin">
		AllowOverride all
	</Directory> 

EOF


if ($authtype eq 'msad') {
	print <<EOF;
	<Directory "/home/boitho/boithoTools/public_html">
		AllowOverride all 
	</Directory>
EOF
} elsif ($authtype eq 'msadsso') {
	my $realm = $ARGV[2];
	if (!defined($serveralias)) {
		$serveralias = 'blackbox.local';
	}
	if (!defined($realm)) {
		$realm = 'BLACKBOX.LOCAL';
	}
	print <<EOF;
	<Directory "/home/boitho/boithoTools/public_html">
		AuthType kerberos
		AuthName "Blackbox login"
		KrbServiceName HTTP/$serveralias\@$realm
		KrbMethodNegotiate On
		KrbSaveCredentials On
		KrbMethodK5Passwd Off
		KrbAuthRealms $realm
		Krb5KeyTab /home/boitho/boithoTools/var/http.keytab

		require valid-user
	</Directory>
EOF
}

# Blackbox host footer
print <<EOF;
</VirtualHost>
EOF
