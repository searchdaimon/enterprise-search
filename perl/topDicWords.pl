my %wordhash;

while (<STDIN>) {

	chomp;

	my ($word, $nr, undef) = split(/ /,$_);

	$wordhash{$word} += $nr;

}

@sorted = sort { $wordhash{$b} <=> $wordhash{$a} } keys %wordhash; 

my $count = 0;
foreach my $i (@sorted) {
	print "$i: $wordhash{$i}\n";

	if($count > 1000) {
		last;
	}

	++$count;
}
