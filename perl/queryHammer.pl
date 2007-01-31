use LWP::Simple;


if ($#ARGV == -1) {
	print "No query file givven\n";
}

open(INF,$ARGV[0]) or die("$ARGV[0] $!");

@querys = <INF>;

close(INF);

foreach my $s (@querys) {
	chomp $s;
	print "$s\n";

	$content = get("http://bbh-001.boitho.com/webklient/?start=1&query=$s&sprok=NBO");
}


