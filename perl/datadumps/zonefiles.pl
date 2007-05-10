use strict;

if ($#ARGV != 1) {
	print "usage: ./zonefiles.pl ttl zonefile\n";

}


my $ttl = $ARGV[0];
my $zonefile = $ARGV[1];


print "ttl: $ttl, zonefile: $zonefile\n";


open(INF,"$zonefile") or die("$zonefile: $!"); 

my $lastdomain = '';
while (<INF>) {
	chomp;

	my $line = $_;

	if ($line =~ /NS/) {
		#print "NS: $line\n";

		my @ell = split(/ /,$line);

		
		if (($#ell == 2) && ($ell[1] eq 'NS')) {
			#print "good: $line\n";
			my $domain = 'http://www.' . lc($ell[0]) . '.' . $ttl . '/';

			if ($lastdomain ne $domain) {
				print "$domain\n";
			}
			$lastdomain = $domain;
		}
		else {
			#print "bad: $line\n";
		}
	}
}

close(INF);
