#!/usr/bin/perl

#leser

if ($ARGV[0] eq '') {
	die("Spesifiser host\n\nekse: perl IndexerLot.pl localhost 2\n");
}
if ($ARGV[1] eq '') {
	die("Spesifiser lot nr\n\nekse: perl IndexerLot.pl localhost 2\n");
}

$LotNr = $ARGV[1];
$HostName = $ARGV[0];

##################
# Tester for om det er ledig disk kapasitet
#my $out = `df -m /`;
#
#@outlines = split(/\n/,$out);
#@lineelements = split(/ +/,$outlines[1]);
#
#print "Available space: $lineelements[3]\n";
#
#if ($lineelements[3] < 10000) {
#	die("to litle space to store data");
#}

##################
#print "dont indexing $LotNr\n";
#exit;

#21 okt 2005: bytter fra 20 til 30 da enkelte ord faktisk er så lange :(
use constant minimum_term_lengde => 2;
use constant maksimum_term_lengde => 20;

use Time::HiRes;
use Compress::Zlib;

my $begynneTdi = Time::HiRes::time;

#use constant HostName => localhost;
#use constant HostName => 'bbs-001.boitho.com';
#use Lot;

use htmlParser qw(strip_html);
use IR qw(lexer stoppord ResulveUrl adultFilter);
use revindex qw(OpenForApendingNET StoreRevIndexNET ApendAray);
use common qw(fin_domene gyldig_url);

#inaliserer språkfinns systemet
our $LinguaIdent_var = inaliser_languages();
use Digest::SHA1  qw(sha1);

#4:$lotha = Lot->New($LotNr);

#4:$lotha->create();

#$lotha->RepositoryOpen('');


#documnet indeks med mere
#4:$lotha->LodeTables();

#exit;
my $revindexha = revindex->OpenForApendingNET('Main',$LotNr,$HostName);

use urlindex;

#my $urlindexHA = urlindex->New($LotNr);


use Boitho::DocumentIndex;

use Boitho::Reposetory;
#Boitho::Reposetory::ropen();


$AntallUrler = 0;

$gyldigSprok{'AFR'} = 1;
$gyldigSprok{'BUL'} = 1;
$gyldigSprok{'EST'} = 1;
$gyldigSprok{'GLE'} = 1;
$gyldigSprok{'IND'} = 1;
$gyldigSprok{'MAR'} = 1;
$gyldigSprok{'QUE'} = 1;
$gyldigSprok{'SPA'} = 1;
$gyldigSprok{'UKR'} = 1;
$gyldigSprok{'ALB'} = 1;
$gyldigSprok{'CAT'} = 1;
$gyldigSprok{'FIN'} = 1;
$gyldigSprok{'GLV'} = 1;
$gyldigSprok{'ITA'} = 1;
$gyldigSprok{'MAY'} = 1;
$gyldigSprok{'RUM'} = 1;
$gyldigSprok{'SWA'} = 1;
$gyldigSprok{'VIE'} = 1;
$gyldigSprok{'AMH'} = 1;
$gyldigSprok{'CHI'} = 1;
$gyldigSprok{'FRA'} = 1;
$gyldigSprok{'GRE'} = 1;
$gyldigSprok{'JPN'} = 1;
$gyldigSprok{'NBO'} = 1;
$gyldigSprok{'RUS'} = 1;
$gyldigSprok{'SWE'} = 1;
$gyldigSprok{'WEL'} = 1;
$gyldigSprok{'ARA'} = 1;
$gyldigSprok{'CZE'} = 1;
$gyldigSprok{'FRY'} = 1;
$gyldigSprok{'HEB'} = 1;
$gyldigSprok{'KOR'} = 1;
$gyldigSprok{'NEP'} = 1;
$gyldigSprok{'SAN'} = 1;
$gyldigSprok{'TAM'} = 1;
$gyldigSprok{'YID'} = 1;
$gyldigSprok{'ARM'} = 1;
$gyldigSprok{'DAN'} = 1;
$gyldigSprok{'GEO'} = 1;
$gyldigSprok{'HIN'} = 1;
$gyldigSprok{'LAT'} = 1;
$gyldigSprok{'PER'} = 1;
$gyldigSprok{'SCC'} = 1;
$gyldigSprok{'TGL'} = 1;
$gyldigSprok{'BAQ'} = 1;
$gyldigSprok{'ENG'} = 1;
$gyldigSprok{'GER'} = 1;
$gyldigSprok{'HUN'} = 1;
$gyldigSprok{'LAV'} = 1;
$gyldigSprok{'POL'} = 1;
$gyldigSprok{'SCR'} = 1;
$gyldigSprok{'THA'} = 1;
$gyldigSprok{'BRE'} = 1;
$gyldigSprok{'EPO'} = 1;
$gyldigSprok{'GLA'} = 1;
$gyldigSprok{'ICE'} = 1;
$gyldigSprok{'LIT'} = 1;
$gyldigSprok{'POR'} = 1;
$gyldigSprok{'SLV'} = 1;
$gyldigSprok{'TUR'} = 1;
$gyldigSprok{'aa'} = 1;

our $terminated = 0;

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;

#finner når denne sist ble indeksert
my $lastIndexTime = Boitho::Reposetory::GetLastIndexTimeForLotNET($HostName,$LotNr);


print "lastIndexTime $lastIndexTime\n";
if ($lastIndexTime != 0) {
	print "Indextime is not 0, exiting\n";
	exit;
}


#while($post = $lotha->RepositoryPostGetNext())
#my $AntallForBehandling = 10200000;
#for my $count (0 .. $AntallForBehandling) {
my $PageCount = 0;
while (( my($DocID, $url, $content_type, $IPAddressInNumber, $IPAddress ,$response, $userID, $clientVersion, $rhtmlSize, $rimageSize, $html, undef, $radress, $DownlodeTime) = Boitho::Reposetory::rGetNextNET($HostName,$LotNr, $lastIndexTime, 0)) && (not $terminated)) {

#fører eval til at vi lekker minne, og går tregere (da den rekompilerer blokken hver gang ???)
#eval {	

	#print "$DocID\n";
	#print "$DocID $url at $radress\n";
	#print "$IPAddress,$response, $clientVersion\n";
if (not defined $html) {
	warn("html is undef");
	
}
elsif ($DocID < 0) {
	print "Got bad DocID $DocID. Is the file broken?\n";
	print "Url: $url\nresponse: $response\nclientVersion: $clientVersion\n";
}	
elsif (($response >= 199) && ($response <= 299)) {


	#print "html length " . length($html) . "\n" . substr($html,0,10 . "\n\n");

	#print "$count\n";
	#viser hvor lang vi har komet:
	#my $prosent = (100 / $AntallForBehandling) * $count;
	#if ($prosent =~ /^[+-]?\d+$/) {
	#	print "$prosent %\n";
	#}
	
	
	
	my (%ParsetSide) = strip_html($html);
	

	#debug: viser all ordene	
	#foreach my $i (keys %ParsetSide) {
	#	print "$i\n";
	#}

	###
	#if (length($body_fra_html) > 1000) {
	#	print "Tester bare på 1000 tegn ikke " . length($body_fra_html);
	#	$sprok = $LinguaIdent_var->identify(substr($body_fra_html,0,1000));
	#	#$LinguaIdent_var->identify($text)
	#}
	#else {
	#	$sprok = $LinguaIdent_var->identify($body_fra_html);
	#}
	#print "Sprok: $sprok\n";
	###
	
	# temp. Forhindrer at rarae språk kommer inn
	#if (not $gyldigSprok{$sprok}) {
	#	#warn("ikke gyldig sptåk: $sprok");
	#	$sprok = '';
	#}
	#/temp

	#temp: skrur av språkdetektering
	#if ($sprok eq '') {
	#	
	#	$sprok = fin_sprok($ParsetSide{'innhold'} . $ParsetSide{'description'});
	#}
	#else {
	#	print "$sprok\n";
	#}
	
	# temp. Forhindrer at rarae språk kommer inn
	#if (not $gyldigSprok{$sprok}) {
	#	#warn("ikke gyldig sptåk: $sprok");
	#	$sprok = 'aa';
	#}
	#/temp
	$sprok = 'aa';
	
	

	my %termer = {};	#inaliserer
	%termer = lexer($ParsetSide{'innhold'},'Body',%termer);


	#lar athor indeksen hondtere urler	
	#my $domene = fin_domene($url);
	#if (length($domene) < 30) {
	#	%termer = lexer($domene,'Url',%termer);
	#	
	#}


	#tar bare med title hvis den er kortere en 80 tegn, dette da det ellers typsik er spam.
	if (length($ParsetSide{'title'}) < 80) {
		%termer = lexer($ParsetSide{'title'},'Tittel',%termer);
	}
	%termer = lexer($ParsetSide{'headlines'},'Headline',%termer);
	#adultnes kalkulasjon
		my $AdultWeight = adultFilter(%termer);
		#leger til ordene i spamen hvis vi har smapm
		# + keywords og description fra htmlene da adult odr ofte forekomer der på sider som er vanskeilg å bestemme
		my %TempSpam = {}; #lexer må ha en array inn, så vi makker en
		%TempSpam = lexer($ParsetSide{'spam'} . ' ' . $ParsetSide{'keywords'} . ' ' . $ParsetSide{'description'} . ' ' . $url,'spam',%TempSpam);
		$AdultWeight += adultFilter(%TempSpam);
		undef %TempSpam;
	#/adultnes kalkulasjon
				
	#skriver siden til Documnet index
	
	Boitho::Reposetory::DIWriteNET($HostName, $url, $sprok, 0, $content_type, $DownlodeTime, 0, $AdultWeight, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);

	
	#print "  adultnes: $adultnes\n";
	
	%termer = stoppord($sprok,minimum_term_lengde,maksimum_term_lengde,%termer);
	
	#Debug: lister opp alle ordene og posisjonen
	#foreach my $i (keys %termer) {
	#	print "$i: ";
	#	foreach my $y (@{$termer{$i}}) {
	#		print "$y->[1]: $y->[0], ";
	#	}
	#	print "\n";
	#}
	
	
	 
	$revindexha->ApendAray($DocID,$sprok,%termer);
	

	
	#######
	#fin fulle URLer
	#sjekker om vi er på en dynamisk url, i såfal skal ikke dynamsike URLer være med
	my $DynamiskSide = 0;
	if ($url =~ /\?/) {	#leter etter ? tegn
		$DynamiskSide = 1;
	}

	my %SettUrler = {};	#ingen vits i å teste en url bare forde den er flere plasser på en side
	foreach my $i (@{ $ParsetSide{urler} }) {
		
		if (not exists $SettUrler{$i->[0]}) {
			$SettUrler{$i->[0]} = 1; #markerer at vi har testet denne
			my $nyUlr = ResulveUrl($url,$i->[0]); #finner full url
		
			#legger til urler
			if (
													#sjekker at vi enten ikke er på en dynamsik side
					((not $DynamiskSide) || (($DynamiskSide) && ($nyUlr !~ /\?/)))	#eller hvis vi er så tar vi bare nyeurler som ikke er dynamiske
				&&	($nyUlr ne $url)						#ikke den samme som den vi indserer nå
				&&	(gyldig_url($nyUlr))						#er en gyldig url
			) {
			
					Boitho::Reposetory::addNewUrl($nyUlr,$i->[1],$DocID,"");				

			}
			#Debug: viser hvilkene URler vi forkaster
			#else {
			#	print "forkastet $url -> $nyUlr\n";
			#}
		}
	}
	#######

	$PageCount++;

	#if ($PageCount > 100) {
	#	last;
	#}
}
else {
	#ikke 200 side
	#Boitho::DocumentIndex::DIWrite($url, '', 0, $content_type, 0, $DownlodeTime, 0, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);	
	Boitho::Reposetory::DIWriteNET($HostName,$url, '', 0, $content_type, 0, $DownlodeTime, 0, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);	

}
#}; warn $@ if $@; #eval 
}

#skriver når denne sist ble indeksert, men bare hvis vi ikke terminerte med ^c, ellers har vi ikke vert gjenom helle filen
if (not $terminated) {
	print "skriver indekstid til lot: $LotNr\n";
	Boitho::Reposetory::setLastIndexTimeForLotNET($HostName,$LotNr);
} else {
	print "terminert, var $terminated\n";
}

print "sending data\n";
$revindexha->StoreRevIndexNET();
print "done\n";

Boitho::Reposetory::closeNET();


print "DIClose\n";
Boitho::DocumentIndex::DIClose();
#print "rclose\n";
#Boitho::Reposetory::rclose();
#print "end\n";
#$urlindexHA->FileClose();
#4:$lotha->FileClose();




my $tid =  Time::HiRes::time - $begynneTdi;
print "Tokk tid: $tid\nIndekserte $PageCount sider\n\n";

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

############################################################################################################################
# finner språk
############################################################################################################################
sub fin_sprok {
	my($text) = @_;
	my($sprok) = '';
	
	#sender bare tilbake no for nå, skal lage denne siden
	#$sprok = 'no';
	if ($text eq '') {
		warn("kalte fin_sprok uten en tekst for identifisering");
		$sprok = 'aa';
	}
	else {
		$sprok = $LinguaIdent_var->identify($text), "\n";
	}
	
	#hvis vi ikke fant et ståk, setter vi det til aa
	if ($sprok eq '') {
		$sprok = 'aa';
	}
	return $sprok;
}

############################################################################################################################
# laster språkfilene
############################################################################################################################
sub inaliser_languages {
	my(@languages) = ();
 
 	

	use File::Find;

	my %options = (wanted => \&list_filer, no_chdir=>1);

	find(\%options,"data/languages");


	sub list_filer {
	
		if (!(-d $File::Find::name)) {
			push(@languages,$File::Find::name);
			
			#print "$File::Find::name\n";
		}
	}

	use LinguaIdent;
 	my $LinguaIdent_var  = new LinguaIdent(@languages); # blir den global for hele programet her ?
	
	return $LinguaIdent_var;
}
