

if ($#ARGV == -1) {
	print "Usage: ./readMainIndex.pl MainIdex\n";
	exit;
}

open(INF,$ARGV[0]) or die($!);
binmode(INF);

while (!eof(INF)) {

	read(INF,$post,24);

	my ($sha1,$DocID) = unpack('A20 I',$post);

	print "$DocID\n";

}


close(INF);
