#!/usr/bin/env perl

use warnings;
use strict;

my $authtype = $ARGV[0] || "msad";


sub write_authdata {
	my ($auth, $serveralias, $realm) = @_;

	if ($auth eq 'msad') {
		print <<EOF;
		AuthName modBoitho
		AuthBoitho on
		AuthType Basic
		require valid-user
EOF
	} elsif ($auth eq 'msadsso') {
		print <<EOF;
		AuthType kerberos
		AuthName "Blackbox Login"
		KrbServiceName HTTP/$serveralias\@$realm
		KrbMethodNegotiate On
		KrbSaveCredentials On
		KrbMethodK5Passwd Off
		KrbAuthRealms $realm
		Krb5KeyTab /home/boitho/boithoTools/var/http.keytab
		require valid-user
EOF
	}
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
my $realm = $ARGV[2];
if (!defined($serveralias) && $authtype eq 'msadsso') {
	print STDERR "No server alias specified, defaulting to 'blackbox.local'\n";
	$serveralias = 'www.blackbox.local';
}
if (!defined($realm) && $authtype eq 'msadsso') {
	print STDERR "No realm specified, defaulting to 'BLACKBOX.LOCAL'\n";
	$realm = 'BLACKBOX.LOCAL';
}
if (defined($serveralias)) {
	print "\tServerAlias $serveralias\n\n";
}

# CGI for webadmin and so on
print <<EOF;
	<Directory "/home/boitho/boithoTools/cgi-bin">
		AllowOverride all
	</Directory> 

EOF

print <<EOF;
	<Directory "/home/boitho/boithoTools/public_html">
		AllowOverride all 
	</Directory>
EOF




print <<EOF;
	<Directory "/home/boitho/boithoTools/public_html/webclient">
EOF
write_authdata($authtype, $serveralias, $realm);
print <<EOF;
	</Directory>
EOF

# Blackbox host footer
print <<EOF;
</VirtualHost>
EOF
