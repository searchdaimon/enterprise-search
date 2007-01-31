sub loadeBaqnnList {
	my %DomainBlockList = {};

	#leser inn alle sidene som er blokert
	open(INF,'../data/bann_list.txt') or die($!);
	while (<INF>) {

		chomp;
		s/#.*//;	#no comments
		s/^\s+//;	#no leading white
		s/\s+$//;	#no traling white	
		#print "-$_- \n";
	
		$DomainBlockList{$_} = 1;
	}
	close(INF);

	return %DomainBlockList;
}
%DomainBlockList = loadeBaqnnList();

if (IsBlocketByDomain('alaska.uscity.net')) {
	print "Domain is bloked\n";
}

sub IsBlocketByDomain() {
	my $domain = shift;
	my $ReturnStatus = 0;

	my @elements = split(/\./,$domain);	#splitter opp domene i enkeltdele
	@elements = reverse(@elements);

	my $forTest = '';
	foreach my $s (@elements) {
		#print "$s\n";

		if ($forTest ne '') {
			$forTest = $s . '.' . $forTest;
		}
		else {
			$forTest = $s;
		}

		if ($DomainBlockList{$forTest}) {
			$ReturnStatus = 1;
			print "$forTest er bannet\n";
		}		
		#print "forTest $forTest\n";
	}	

	return $ReturnStatus;
}
