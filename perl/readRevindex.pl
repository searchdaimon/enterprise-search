
use strict;
use iindex;

use common qw(GetAllLotPaths);
use Boitho::Lot;










	

		my $fil = $ARGV[0];
		print "File: $fil\n";

		$/ = "**\cJ";
		open(INF,"$fil") or die("Can't open $fil: $!");
		binmode(INF);
	
		my $buf = '';
		my $char = '';
		#lsere filen char for char i påvente eof
		#leter etter **\n
		#while (not eof(INF)) {
		while (<INF>) {
			chomp;
		
				
						my $RindexRecord = $_;       #bruker $buf
		
						#print "Ny record: $RindexRecord\n";
			
						my $lestTilOgMed = 0; #viser hvor langt vi har kommet med å lese posten
						
						#hvis vi har mere en DocID og sprok lagret om denne DocIDen
						if (length($RindexRecord) > 7) {

							#print "length " . length($RindexRecord) . "\n";
							#print "$RindexRecord\n";
							#print " -" . substr($RindexRecord,0,4) . "-\n";
							my ($DocID,$sprok) = unpack('L A3',substr($RindexRecord,0,7));
							$lestTilOgMed = $lestTilOgMed + 7;
		
							#print "DocID: $DocID\n";
							print "DocID: $DocID, sprok: $sprok\n";
		
							while ((length($RindexRecord) -1) > $lestTilOgMed) {
								#print " dd: lengthRindexRecord: " . ( length($RindexRecord) -1 ) . " lestTilOgMed $lestTilOgMed\n";
								my ($WordID,$antall) = unpack('L L',substr($RindexRecord,$lestTilOgMed,8));
								$lestTilOgMed = $lestTilOgMed + 8;
							
								print "\twordid $WordID - antall $antall\n";							

							
								my $HitListSize = $antall * 2;
								
								my $HitList = substr($RindexRecord,$lestTilOgMed,$HitListSize);
								$lestTilOgMed = $lestTilOgMed + $HitListSize;
			
								my @hits = unpack('S' x $antall,$HitList);

								foreach my $hit (@hits) {
									print "\thit: $hit\n";
								}	

							}
							print "\n";
						}
		}
		#temp: skal denne være her insteden ???
		close(INF) or warn ("Cant close file: $!");
		#$iindex_ha->WriteIIndexToFile($type,$revindexFilNr,$lotNr);
