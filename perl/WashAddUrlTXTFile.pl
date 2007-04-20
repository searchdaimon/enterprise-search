
use IR qw(ResulveUrl);


if ($#ARGV != 0) {
	print "Usage:\n\tWashAddUrlFile.pl urls.txt\n";
	exit;
}

open(INF,$ARGV[0]) or die ("$ARGV[0]: $!");


while (<INF>) {

	chomp;
	
	$url = $_;

	$url = ResulveUrl('http://www.boitho.com/legg_til_link.htm.no',$url);
	
	print "$url\n";
	
}

close(INF);

