use strict;

if ($#ARGV != 0) {
	print "usage: ./zonefiles.pl ttl\n";

}

my $ttl = $ARGV[0];



my $lastdomain = '';
while (<STDIN>) {
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
