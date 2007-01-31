#!/usr/bin/perl


if ($#ARGV > 1) {
	die("spesifiser lot nr");
}

$LotNr = $ARGV[0];

use constant minimum_term_lengde => 2;
use constant maksimum_term_lengde => 20;

use Boitho::Reposetory;

use revindex qw(OpenForApending FileClose ApendAray);
use IR qw(lexer stoppord ResulveUrl adultFilter);

my $revindexha = revindex->OpenForApending('Athor',$LotNr,'>>');

our $terminated = 0;

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;

print "start anchorGetNext for lot $LotNr\n";
while ((!$terminated) && (($DocID,$text,$radress,$rsize) = Boitho::Reposetory::anchorGetNext($LotNr))) {
	#print "$DocID,$text\n"; 
	#print "$DocID\n";


	my %termer = {};
	%termer = lexer($text,'Body',%termer);

	$sprok = 'aa';

	%termer = stoppord($sprok,minimum_term_lengde,maksimum_term_lengde,%termer);

	#foreach my $s (keys %termer) {
	#	print "$s\n";
	#}

	$revindexha->ApendAray($DocID,$sprok,%termer);

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
