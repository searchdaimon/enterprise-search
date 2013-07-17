#!/usr/bin/perl

use Boitho::DocumentIndex;
use common qw(fin_domene gyldig_url);
use revindex qw(OpenForApending FileClose ApendAray);
use IR qw(lexer stoppord ResulveUrl adultFilter);

use strict;

use constant minimum_term_lengde => 4;
use constant maksimum_term_lengde => 20;


if ($#ARGV == -1) {
	print "usage: IndexerLotUrl.pl LotNR\n\n";
	exit;
}


my $LotNr = $ARGV[0];

our $terminated = 0;

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;

my $revindexha = revindex->OpenForApending('Anchor',$LotNr,'>');

while ((not $terminated) && (my ($DocID,$url,$sprok,$Offensive_code,$Dokumenttype,$CrawleDato,$AntallFeiledeCrawl,$AdultWeight,$RepositoryPointer,$RepositorySize,$ResourcePointer,$ResourceSize,$IPAddress,$response) = Boitho::DocumentIndex::DIGetNext($LotNr))) {

	#print "$url\n";
	#print "$DocID\n";
	
	my $domene = fin_domene($url);
	#er ganske kritisk på hvilkene urler vi vil ha med her, så får vi heller opne opp i hovedindeksen
        my $domeneForTesting = $domene;
	my $lineCount = 0;
        while ($domeneForTesting =~ /-/g) { $lineCount++ }

	if ($domene eq '') {
		#ToDo: hvorfor er så mange blanke ???
		print "no domain given DocID: $DocID, Url: $url\n";
	}
	elsif (length($domene) > 30) {
		#print "to long $domene: " . length($domene) . "\n";
	}
	elsif($lineCount > 2) {
		#print "To many lines $domene: $lineCount\n";
	}
	else {
		#print "$domene:\n";

		#reverser domene, slik at "www.boitho.com" blir til "com.boitho.www"
		my @temp = split(/\./,$domene);
		@temp = reverse(@temp);
		$domene = join('.',@temp);

		#print "$DocID  $domene\n";

		my %termer = {};	#inaliserer
		%termer = lexer($domene,'Url',%termer);

	
		%termer = stoppord($sprok,minimum_term_lengde,maksimum_term_lengde,%termer);

		#debug: vise orden
		#foreach my $s (keys %termer) {
		#	print "$s\n";
		#}		

		$revindexha->ApendAray($DocID,'aa',%termer);		


	}
}

$revindexha->FileClose();


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


