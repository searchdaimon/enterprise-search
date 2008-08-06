my $file = shift(@ARGV) or die "must specify a file";

my %users;

open(INF,"$file");



while (<INF>) {
	chomp;
	my $line = $_;

	if ($line !~ /\/exchange\//) {
		next;
	}

	#print "$line\n";
	my @el = split(/ /,$line);

	my $user = $el[4];

	#print "url1: $user\n";

	$user =~ s/exchange\/([^\/]+)\//XXXX/; 
	$user = $1;

	$users{$user} = $users{$user} +1;

	#print "url2: $el[4]\n";

}

close(INF);


foreach my $i (keys %users) {
	printf("%-10s = %u\n", $i, $users{$i});
}
