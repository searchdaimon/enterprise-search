if ($#ARGV != 0) {
        print "Usage:\n\tWashAddUrlFile.pl urls.txt\n";
        exit;
}

open(INF,$ARGV[0]) or die ("$ARGV[0]: $!");


while (<INF>) {

	my ($url,$mail) = split(/ | /,$_);

	print "$url\n";
}


close(INF);
