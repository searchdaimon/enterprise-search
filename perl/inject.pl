
if ($#ARGV != 1) {
 print qq{
merger en stor og en listen fil slik at den lill filen blir injersert med gjevne melomrom. Finner selv ut hvor ofte.

	./inject.pl smalfile largefile
};
 exit;
}


my $smalfile = $ARGV[0];
my $largefile = $ARGV[1];

open(SMAL,"$smalfile") or die("Cant open smal file \"$smalfile\": $!");
flock(SMAL,2);

open(LARGE,"$largefile") or die("Cant open large fil \"$largefile\": $!");
flock(LARGE,2);

my $scala = (stat(LARGE))[7] / (stat(SMAL))[7];

print "scala: $scala\n";
my $count = 0;
while (<LARGE>) {

	print $_;

	if (($count % $scala) == 1) {
		my $smaline = <SMAL>;
		print $smaline;
	}	

	$count++;
}

#skriver eventuel rest
while (<SMAL>) {
	print $_;
}

close(SMAL);
close(LARGE);

