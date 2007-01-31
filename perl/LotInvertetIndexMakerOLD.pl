

use strict;
use iindex;

use common qw(GetAllLotPaths);
use Boitho::Lot;
use supportedLang qw(isSupportedLang);


#Indekser(42,'Main');
#exit;


#temp:
#Indekser(42,'Athor',@revindexpaths);
#Indekser(42,'Athor',('/mnt/hdd1/lot/43/43/revindex/Athor'));
#exit;
#hvis vi fikk inputt argument tar vi bare den bøtten. Slik kan man for eks bare ta 42
my $lotNr = $ARGV[0];

############### Athor #################################################################
if ($#ARGV == 0) {
        print "Indexing all buvkets\n";

        for my $i (0 .. 64) {
                print "$i\n";

                eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                        Indekser($i,'Main',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Main');
                }; warn $@ if $@;
        }
}
else {
        print "Indexing bucket $ARGV[1]\n";

        Indekser($ARGV[1],'Main',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Main');
}

exit;
############### Main  #################################################################
if ($#ARGV == 0) {
	print "Indexing all buvkets\n";

	for my $i (0 .. 64) {
        	print "$i\n";

        	eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                	Indekser($i,'Athor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Athor');
        	}; warn $@ if $@;
	}
}
else {
        print "Indexing bucket $ARGV[1]\n";

        Indekser($ARGV[1],'Athor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Athor');
}

#######################################################################################

exit;
if ($#ARGV == 1) {
        print "Indexing bucket $ARGV[1]\n";

        Indekser($ARGV[1],'Athor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Athor');
}
else {

        for my $i (0 .. 64) {
                print "$i\n";

                eval {  #eval slik at vi bare fortsetter hvis det skjer noe feil
                        Indekser($i,'Athor',Boitho::Lot::GetFilPathForLot($lotNr). 'revindex/Athor');
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

#for my $i (0 .. 64) {
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


	#går gjenom alle revindeksene 
	foreach my $revindexPath (@revindexpaths) {
	
		#my $iindex_ha = iindex->new;

		my $fil = "$revindexPath/$revindexFilNr.txt";
		print "File: $fil\n";
	
		$/ = "**\cJ";
		open(INF,"$fil") or die("Can't open $fil: $!");
		binmode(INF);
	
		my $buf = '';
		my $char = '';
		#lsere filen char for char i påvente eof
		#leter etter **\n
		while (not eof(INF)) {
		#while (<INF>) {
		#	chomp;
		
				
						#my $RindexRecord = $_;       #bruker $buf
		
						read(INF,$DocID,4);
						$DocID = unpack('L',$DocID);
						read(INF,$sprok,3);
						$sprok = unpack('A3',$sprok);

						print "DocID: $DocID $sprok\n";
						exit;
						#print "Ny record: $RindexRecord\n";
			
						my $lestTilOgMed = 0; #viser hvor langt vi har kommet med å lese posten
						
						#hvis vi har mere en DocID og sprok lagret om denne DocIDen
						if (length($RindexRecord) > 7) {

							#print "length " . length($RindexRecord) . "\n";
							#print "$RindexRecord\n";
							#print " -" . substr($RindexRecord,0,4) . "-\n";
							my ($DocID,$sprok) = unpack('L A3',substr($RindexRecord,0,7)) or warn("cant substr: $!");
							$lestTilOgMed = $lestTilOgMed + 7;
		
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
								my $HitList = substr($RindexRecord,$lestTilOgMed,$HitListSize) or warn("cant substr: $!");
								$lestTilOgMed = $lestTilOgMed + $HitListSize;

								if (length($RindexRecord) < $HitListSize) {
									print "error hitlis to long\n";
									last;
								}
								#print "HitListSize $HitListSize\n";
			
								#print "HitList: -$HitList- som er -" . unpack('L',$HitList) . "-\n";
								if (isSupportedLang($sprok)) {
									$iindex_ha->add($sprok,$DocID,$WordID,$antall,$HitList);
								}
								else {
									print "lang not supported WordID: $WordID, antall: $antall\n";
								}
							}
						}
		}
		#temp: skal denne være her insteden ???
		close(INF) or warn ("Cant close file: $!");
		#$iindex_ha->WriteIIndexToFile($type,$revindexFilNr,$lotNr);
	}
	#close(INF) or die ("Cant close file: $!");

	$iindex_ha->WriteIIndexToFile($type,$revindexFilNr,$lotNr);
}
