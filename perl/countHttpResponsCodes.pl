use strict;

my @total = ();
for my $i (0 .. 999) {
	$total[$i] = 0;
}

foreach my $path (@ARGV) {

	print "$path\n";

	open(INF,$path) or die($!);

	my @contdata = <INF>;

	close(INF);

	foreach my $i (@contdata) {
		chomp $i;
		#print "$i\n";

		my ($respons,$nr) = split(/: /,$i);

		#print "$respons,$nr\n";

		$total[$respons] = $total[$respons] + $nr;
		#print "aa: respons: $respons, nr $nr, total $total[$respons]\n";
	}

}

print "For " . scalar(@ARGV) . " in files\n";
print "total:\n";
#foreach my $i (@total) {
for my $i (0 .. 999) {

	if ($total[$i] != 0) {
		print "$i: $total[$i]\n";
	}
}
