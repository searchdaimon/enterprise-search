
#$RindexRecord = 'ggggggggffff';
#print 'L' . ('L' x ((length($RindexRecord) -1) / 4));
#
#exit;
use strict;
use iindexOLD;

use common qw(GetAllLotPaths);

#Indekser(42,'Main');
#exit;

my @LotPaths = GetAllLotPaths('revindex/Anchor');

my @revindexpaths = ();

print "lots med anchors: \n";
foreach my $lotPath (@LotPaths) {
        #print "lotPath: $lotPath\n";
        my $revindexPath = "$lotPath/revindex/Anchor";

        if (-e $revindexPath) {
                print "skal indekseren $revindexPath\n";
                push(@revindexpaths,$revindexPath);
        }
}

#temp:
#Indekser(42,'Anchor',@revindexpaths);
#Indekser(42,'Anchor',('/mnt/node1/hda4/lot/12/12/revindex/Anchor'));
#exit;
#if we got an input argument we only take that bucket. This way you can for example take only 42
if ($#ARGV != -1) {
	print "Indexing bucket $ARGV[0]\n";

	Indekser($ARGV[0],'Anchor',@revindexpaths);
}
else {

	for my $i (0 .. 64) {
        	print "$i\n";

        	eval {  #eval so that we just continue if an error occurs
                	Indekser($i,'Anchor',@revindexpaths);
        	}; warn $@ if $@;
	}
}
#exit;

my @LotPaths = GetAllLotPaths('revindex/Main');

my @revindexpaths = ();

print "lots med revindex: \n";
foreach my $lotPath (@LotPaths) {
	#print "lotPath: $lotPath\n";
	my $revindexPath = "$lotPath/revindex/Main";
	
	if (-e $revindexPath) {
		print "$revindexPath\n";
		push(@revindexpaths,$revindexPath);
	}
}
#exit;

for my $i (0 .. 64) {
	print "$i\n";

	eval {	#eval so that we just continue if an error occurs
		Indekser($i,'Main',@revindexpaths);
	}; warn $@ if $@;

}




sub Indekser {
	#my $revindexFilNr = shift;
	my($revindexFilNr,$type,@revindexpaths) = @_;
	
	#my $fil = "data/revindex/$type/$revindexFilNr.txt";

	my $iindex_ha = iindex->new;


	#goes through all the revindexes 
	my $count = 0;
	foreach my $revindexPath (@revindexpaths) {

		print "\t$count\n";

		#my $iindex_ha = iindex->new;
	
		my $fil = "$revindexPath/$revindexFilNr.txt";
		#print "File: $fil\n";
	
		$/ = "**\cJ";
		open(INF,"$fil") or die("Can't open $fil: $!");
		binmode(INF);
	
		my $buf = '';
		my $char = '';
		#reads the file char by char while waiting for eof
		#looks for **\n
		#while (not eof(INF)) {
		while (<INF>) {
			chomp;
		
				
						my $RindexRecord = $_;       #uses $buf
						#print "Ny record: $RindexRecord\n";
			
						my $lestTilOgMed = 0; #shows how far we have come reading the record
						#if we have more than a DocID and language stored about this DocID
						if (length($RindexRecord) > 7) {

							#print "$RindexRecord\n";
							#print " -" . substr($RindexRecord,0,4) . "-\n";
							my ($DocID,$sprok) = unpack('L A3',substr($RindexRecord,0,7));
							$lestTilOgMed = $lestTilOgMed + 7;
		
							#print "DocID: $DocID\n";
							#print "$sprok - $DocID\n";
		
							while ((length($RindexRecord) -1) > $lestTilOgMed) {
								#print " dd: lengthRindexRecord: " . ( length($RindexRecord) -1 ) . " lestTilOgMed $lestTilOgMed\n";
								my ($WordID,$antall) = unpack('L L',substr($RindexRecord,$lestTilOgMed,8));
								$lestTilOgMed = $lestTilOgMed + 8;
							
							
								######
								# debug - shows data for a specific word
								#if ($WordID == 4020398583) {
									#print "WordID: $WordID antall: $antall\n";
								#}
								#/debug
								######
							
								my $HitListSize = $antall * 2;
								my $HitList = substr($RindexRecord,$lestTilOgMed,$HitListSize);
								$lestTilOgMed = $lestTilOgMed + $HitListSize;
			
								#print "HitList: -$HitList- som er " . unpack('L',$HitList) . "\n";
								#temp: builds for only one word, "chat"
								
								$iindex_ha->add($sprok,$DocID,$WordID,$antall,$HitList);
								
							}
						}
		
		}
		#print "file end\n";
		#temp: should this be here instead ???
		close(INF) or warn ("Cant close file: $!");
		#$iindex_ha->WriteIIndexToFile($type,$revindexFilNr);

		#temp: does not index the last lots
		if ($count > 66) {
			last;
		}

		$count++;
	}
	#close(INF) or die ("Cant close file: $!");
	print "writing index\n";
	$iindex_ha->WriteIIndexToFile($type,$revindexFilNr);
}
