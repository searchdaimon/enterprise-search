#!/usr/bin/perl

#reads

if ($ARGV[0] eq '') {
	die("Spesifiser host\n\nekse: perl IndexerLot.pl localhost 2\n");
}
if ($ARGV[1] eq '') {
	die("Spesifiser lot nr\n\nekse: perl IndexerLot.pl localhost 2\n");
}

$LotNr = $ARGV[1];
$HostName = $ARGV[0];

##################
# Tests whether there is free disk capacity
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

#21 oct 2005: switching from 20 to 30 since some words really are that long :(
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

#initializes the language detection system
our $LinguaIdent_var = inaliser_languages();
use Digest::SHA1  qw(sha1);

#4:$lotha = Lot->New($LotNr);

#4:$lotha->create();

#$lotha->RepositoryOpen('');


#document index and more
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

#finds when this was last indexed
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

#does eval cause us to leak memory and run slower (since it recompiles the block every time ???)
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
	#shows how far we have come:
	#my $prosent = (100 / $AntallForBehandling) * $count;
	#if ($prosent =~ /^[+-]?\d+$/) {
	#	print "$prosent %\n";
	#}
	
	
	
	my (%ParsetSide) = strip_html($html);
	

	#debug: shows all the words	
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
	
	# temp. Prevents rare languages from getting in
	#if (not $gyldigSprok{$sprok}) {
	#	#warn("not a valid language: $sprok");
	#	$sprok = '';
	#}
	#/temp

	#temp: turns off language detection
	#if ($sprok eq '') {
	#	
	#	$sprok = fin_sprok($ParsetSide{'innhold'} . $ParsetSide{'description'});
	#}
	#else {
	#	print "$sprok\n";
	#}
	
	# temp. Prevents rare languages from getting in
	#if (not $gyldigSprok{$sprok}) {
	#	#warn("not a valid language: $sprok");
	#	$sprok = 'aa';
	#}
	#/temp
	$sprok = 'aa';
	
	

	my %termer = {};	#initializes
	%termer = lexer($ParsetSide{'innhold'},'Body',%termer);


	#lets the anchor index handle URLs	
	#my $domene = fin_domene($url);
	#if (length($domene) < 30) {
	#	%termer = lexer($domene,'Url',%termer);
	#	
	#}


	#only includes the title if it is shorter than 80 chars, since otherwise it is typically spam.
	if (length($ParsetSide{'title'}) < 80) {
		%termer = lexer($ParsetSide{'title'},'Tittel',%termer);
	}
	%termer = lexer($ParsetSide{'headlines'},'Headline',%termer);
	#adultness calculation
		my $AdultWeight = adultFilter(%termer);
		#adds the words in the spam if we have spam
		# + keywords and description from the html since adult words often appear there on pages that are hard to classify
		my %TempSpam = {}; #lexer needs an array as input, so we make one
		%TempSpam = lexer($ParsetSide{'spam'} . ' ' . $ParsetSide{'keywords'} . ' ' . $ParsetSide{'description'} . ' ' . $url,'spam',%TempSpam);
		$AdultWeight += adultFilter(%TempSpam);
		undef %TempSpam;
	#/adultness calculation
				
	#writes the page to the Document index
	
	Boitho::Reposetory::DIWriteNET($HostName, $url, $sprok, 0, $content_type, $DownlodeTime, 0, $AdultWeight, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);

	
	#print "  adultnes: $adultnes\n";
	
	%termer = stoppord($sprok,minimum_term_lengde,maksimum_term_lengde,%termer);
	
	#Debug: lists all the words and their positions
	#foreach my $i (keys %termer) {
	#	print "$i: ";
	#	foreach my $y (@{$termer{$i}}) {
	#		print "$y->[1]: $y->[0], ";
	#	}
	#	print "\n";
	#}
	
	
	 
	$revindexha->ApendAray($DocID,$sprok,%termer);
	

	
	#######
	#find full URLs
	#checks whether we are on a dynamic url, in which case dynamic URLs should not be included
	my $DynamiskSide = 0;
	if ($url =~ /\?/) {	#looks for the ? char
		$DynamiskSide = 1;
	}

	my %SettUrler = {};	#no point in testing a url just because it appears in several places on a page
	foreach my $i (@{ $ParsetSide{urler} }) {
		
		if (not exists $SettUrler{$i->[0]}) {
			$SettUrler{$i->[0]} = 1; #marks that we have tested this one
			my $nyUlr = ResulveUrl($url,$i->[0]); #finds full url
		
			#adds URLs
			if (
													#checks that we are either not on a dynamic page
					((not $DynamiskSide) || (($DynamiskSide) && ($nyUlr !~ /\?/)))	#or if we are, we only take new URLs that are not dynamic
				&&	($nyUlr ne $url)						#not the same as the one we are indexing now
				&&	(gyldig_url($nyUlr))						#is a valid url
			) {
			
					Boitho::Reposetory::addNewUrl($nyUlr,$i->[1],$DocID,"");				

			}
			#Debug: shows which URLs we discard
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
	#not a 200 page
	#Boitho::DocumentIndex::DIWrite($url, '', 0, $content_type, 0, $DownlodeTime, 0, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);	
	Boitho::Reposetory::DIWriteNET($HostName,$url, '', 0, $content_type, 0, $DownlodeTime, 0, $radress, $rhtmlSize, $rimageSize, 0, 0, $IPAddressInNumber, $response, $DocID, $userID, $clientVersion);	

}
#}; warn $@ if $@; #eval 
}

#writes when this was last indexed, but only if we did not terminate with ^c, otherwise we have not been through the whole file
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
# handles signals
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
# finds language
############################################################################################################################
sub fin_sprok {
	my($text) = @_;
	my($sprok) = '';
	
	#just returns no for now, this part is still to be made
	#$sprok = 'no';
	if ($text eq '') {
		warn("kalte fin_sprok uten en tekst for identifisering");
		$sprok = 'aa';
	}
	else {
		$sprok = $LinguaIdent_var->identify($text), "\n";
	}
	
	#if we did not find a language, we set it to aa
	if ($sprok eq '') {
		$sprok = 'aa';
	}
	return $sprok;
}

############################################################################################################################
# loads the language files
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
 	my $LinguaIdent_var  = new LinguaIdent(@languages); # does it become global for the whole program here ?
	
	return $LinguaIdent_var;
}
