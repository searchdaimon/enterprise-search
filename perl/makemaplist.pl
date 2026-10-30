###################################################################
#
# This program creates a makemaplist.conf file from the folders
# it is given. It will try to spread the data evenly.
#
#
###################################################################

if ($#ARGV < 0) {
	die "usage: perl perl/makemaplist.pl directory [ directory ]";
}

for my $i (0 .. $#ARGV) {
	#print "$ARGV[$i]\n";
	#remove any trailing /
	$ARGV[$i] =~ s/\/$//;
}

my $count = 0;
for my $i (0 .. 64) {

	my $nr = $i % ($#ARGV +1);

	#print "nr: $nr (i: $i, a: $#ARGV)\n";
	print "$ARGV[$nr]/$count\n";

	++$count;
}
