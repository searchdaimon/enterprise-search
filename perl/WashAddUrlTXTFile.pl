use common qw(gyldig_url fin_domene);

use IR qw(ResulveUrl);


if ($#ARGV != 0) {
	print "Usage:\n\tWashAddUrlFile.pl urls.txt\n";
	exit;
}

open(INF,$ARGV[0]) or die ("$ARGV[0]: $!");


while (<INF>) {
	
	$url = $_;

	$url = ResulveUrl('http://www.boitho.com/legg_til_link.htm.no',$url);
	
	
	if (not gyldig_url($url)) {
		#er ikke gyldig
		#print "ugyldig $url-\n";
	}
	else {
		print "$url\n";
				
	}
}

close(INF);

