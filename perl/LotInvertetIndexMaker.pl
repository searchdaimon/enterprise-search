#!/usr/bin/perl

use strict;
use iindex;

use common qw(GetAllLotPaths);
use Boitho::Lot;
use Boitho::Reposetory;
use supportedLang qw(isSupportedLang);

use constant subname => 'www';

#for � skru av hvis mange er kalt fra et script
#print "dont index $ARGV[0]\n";
#exit;

#Indekser(42,'Main');
#exit;


#temp:
#Indekser(42,'Anchor',@revindexpaths);
#Indekser(42,'Anchor',('/mnt/hdd1/lot/43/43/revindex/Anchor'));
#exit;
if ($#ARGV < 1) {
	print "usage: ./LotInvertetIndexMaker.pl type lot\n\n\teks: ./LotInvertetIndexMaker.pl Anchor 95\n\n";
	exit;
}

#hvis vi fikk inputt argument tar vi bare den b�tten. Slik kan man for eks bare ta 42
my $type = $ARGV[0];
my $lotNr = $ARGV[1];

if (!Boitho::Lot::lotHasSufficientSpace($lotNr,4096,subname)) {
	print "Has insufficient space\n";
	exit;
}


if ($type eq 'Main') {
############### Main #################################################################
#finner n�r denne sist ble indeksert
my $lastIndexTime = Boitho::Reposetory::GetLastIndexTimeForLot($lotNr,subname);

print "lastIndexTime $lastIndexTime\n";
if ($lastIndexTime == 0) {
        print "Indextime is 0, exiting\n";
        exit;
}

if ($#ARGV == 1) {
        print "Indexing all buvkets\n";

        for my $i (0 .. 63) {
                #print "$i\n";

                eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                        Indekser($i,'Main',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Main');
                }; warn $@ if $@;
        }
	print "\n\n";
}
else {
        print "Indexing bucket $ARGV[2]\n";

        Indekser($ARGV[2],'Main',Boitho::Lot::GetFilPathForLot($lotNr,subname). 'revindex/Main');
}

}
elsif ($type eq 'Anchor') {
############### Anchor  #################################################################
if ($#ARGV == 1) {
	print "Indexing all buvkets\n";

	for my $i (0 .. 63) {
        	print "$i\n";

        	eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                	Indekser($i,'Anchor',Boitho::Lot::GetFilPathForLot($lotNr,subname). 'revindex/Anchor');
        	}; warn $@ if $@;
	}
}
else {
        print "Indexing bucket $ARGV[1]\n";

        Indekser($ARGV[2],'Anchor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Anchor');
}

#######################################################################################
}
else {
	print "ukjent type: $type\n";
}
exit;
if ($#ARGV == 1) {
        print "Indexing bucket $ARGV[1]\n";

        Indekser($ARGV[1],'Anchor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Anchor');
}
else {

        for my $i (0 .. 63) {
                print "$i\n";

                eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                        Indekser($i,'Anchor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Anchor');
                }; warn $@ if $@;
        }
}

exit;



#print "lots med revindex: \n";
#foreach my $lotPath (@LotPaths) {
#	#print "lotPath: $lotPath\n";
#	my $revindexPath = "$lotPath/revindex/Main";
#	
#	if (-e $revindexPath) {
#		print "$revindexPath\n";
#		push(@revindexpaths,$revindexPath);
#	}
#}
#exit;

#for my $i (0 .. 63) {
#	print "$i\n";
#
#	eval {	#eval slik at vi bare fortsetter hvis det skjer noe feil
#		Indekser($i,'Main',@revindexpaths);
#	}; warn $@ if $@;
#
#}




sub Indekser {
	#my $revindexFilNr = shift;
	my($revindexFilNr,$type,@revindexpaths) = @_;
	
	#my $fil = "data/revindex/$type/$revindexFilNr.txt";

	my $iindex_ha = iindex->new;


	#g�r gjenom alle revindeksene 
	foreach my $revindexPath (@revindexpaths) {
	
		#my $iindex_ha = iindex->new;

		my $fil = "$revindexPath/$revindexFilNr.txt";
		#print "File: $fil\n";
	
		$/ = "**\cJ";
		open(INF,"$fil") or die("Can't open $fil: $!");
		binmode(INF);
	
		my $buf = '';
		my $char = '';
		#lsere filen char for char i p�vente eof
		#leter etter **\n
		#while (not eof(INF)) {
		while (<INF>) {
			chomp;
		
				
						my $RindexRecord = $_;       #bruker $buf
		
						#print "Ny record: $RindexRecord\n";

			
						my $lestTilOgMed = 0; #viser hvor langt vi har kommet med � lese posten
						
						#hvis vi har mere en DocID og sprok lagret om denne DocIDen
						if (length($RindexRecord) > 7) {

							#print "length " . length($RindexRecord) . "\n";
							#print "$RindexRecord\n";
							#print " -" . substr($RindexRecord,0,4) . "-\n";
							#my ($DocID,$sprok) = unpack('L A3',substr($RindexRecord,0,7)) or warn("cant substr: $!");
							my ($DocID,$sprok) = unpack('L A',substr($RindexRecord,0,5)) or warn("cant substr: $!");
							$lestTilOgMed = $lestTilOgMed + 5;
		
							#print "DocID: $DocID\n";
							#print "DocID: $DocID, sprok: $sprok\n";
		
							#if (!isSupportedLang($sprok)) {
							#	print "unsuportet lang $sprok\n";
							#	#last;
							#}

							while ((length($RindexRecord) -1) > $lestTilOgMed) {
								#print " dd: lengthRindexRecord: " . ( length($RindexRecord) -1 ) . " lestTilOgMed $lestTilOgMed\n";
								my ($WordID,$antall) = unpack('L L',substr($RindexRecord,$lestTilOgMed,8)) or warn("cant substr: $!");
								$lestTilOgMed = $lestTilOgMed + 8;
							
								#print "\t$WordID : $antall\n";							

								######
								# debug - viser data for et bestemt ord
								#if ($WordID == 4020398583) {
								#	print "WordID: $WordID antall: $antall\n";
								#}
								#/debug
								######
							
								my $HitListSize = $antall * 2;
								if ((length($RindexRecord) - $lestTilOgMed) < $HitListSize) {
                                                                        print "error hitlis to long DocID: $DocID, lestTilOgMed $lestTilOgMed, HitListSize $HitListSize\n";
                                                                        last;
                                                                }


								my $HitList = substr($RindexRecord,$lestTilOgMed,$HitListSize) or warn("cant substr: $!");
								$lestTilOgMed = $lestTilOgMed + $HitListSize;

								#print "HitListSize $HitListSize\n";
			
								#print "HitList: -$HitList- som er -" . unpack('L',$HitList) . "-\n";
								if (isSupportedLang($sprok)) {
									$iindex_ha->add($sprok,$DocID,$WordID,$antall,$HitList);
								}
								else {
									#print "lang \"$sprok\" not supported WordID: $WordID, antall: $antall\n";
									#14 may 2007, legger til linjen under f�r � pr�ve � ha st�tte for stp�k tall
									$iindex_ha->add("aa",$DocID,$WordID,$antall,$HitList);
								}
							}
							if ($lestTilOgMed != length($RindexRecord)) {
								print "lestTilOgMed: $lestTilOgMed, length: " . length($RindexRecord) . " DocID: $DocID\n";
								#print "HitListSize $HitListSize\n";
								print "\n";
							}
						}
		}
		#temp: skal denne v�re her insteden ???
		close(INF) or warn ("Cant close file: $!");
		#$iindex_ha->WriteIIndexToFile($type,$revindexFilNr,$lotNr);
	}
	#close(INF) or die ("Cant close file: $!");

	$iindex_ha->WriteIIndexToFile($type,$revindexFilNr,$lotNr,subname);
}
