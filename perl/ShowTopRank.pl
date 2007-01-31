use Boitho::DocumentIndex;

our $terminated = 0;

my @toppRankArray = ();


open(INF,'../data/popindex') or die($!);

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
my $DocID = 0;

while ((!eof(INF)) && not ($terminated)) {

	read(INF,$post,4);
	$rank = unpack('I',$post);

	if ($rank > 1) {
		$toppRankArray[++$#toppRankArray][0] = $rank;
		$toppRankArray[$#toppRankArray][1] = $DocID
	}
	#print "$DocID: $rank\n";

	$DocID++;
}

@toppRankArray = sort {$b->[0] <=> $a->[0]} @toppRankArray;

my $count = 0;
while (($count < 100) && ($count < $#toppRankArray)) {

	
	my ($url,$Sprok,$Offensive_code,$Dokumenttype,$CrawleDato,$AntallFeiledeCrawl,$Sha1,$AdultWeight,$RepositoryPointer,$RepositorySize,$ResourcePointer,$ResourceSize) = Boitho::DocumentIndex::DIRead($toppRankArray[$count][1]);

	print "$toppRankArray[$count][1]: $toppRankArray[$count][0] , $url\n";

	$count++;
}



close(INF);

############################################################################################################################
# hånterer signaler
############################################################################################################################
sub signal_handler {	
	if (not $terminated) {
		$terminated = 1;

		print "\n\aOk, begynner og avslutte\n\n";
	}
	else {
		die("Motok killsignal nr to, dør");
	}
}
