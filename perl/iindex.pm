package iindex;   
require Exporter;
@iindex::ISA = qw(Exporter);
@iindex::EXPORT = qw();
@iindex::EXPORT_OK = qw(WriteIIndexToFile add new);

use constant DictionaryAdressSizeForTerm => 'L L ';						#the template used for $adress,$SizeForTerm in DictionaryHash and in DictionaryTemplate
use constant DictionaryTemplate => 'L' . DictionaryAdressSizeForTerm;
use constant DictionaryRecordSize => 12; #the size of each dictionary record

use constant AntallBarrals => 64; # must be equal to the value in revindex.pm

#points for positions in documents
use constant poengBody => 4;		#4
use constant poengHeadline  => 10;	#10
use constant poengTittel  => 15;	#15
use constant poengUrl  => 60;		#60

use POSIX;
use common qw(MakePath);
use StringPop;

use Boitho::Lot;

#new function
sub new {
	my($class) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for object
	
	$self->{'AntallIIndexElementer'} = 0;	#tracks how many elements we have in memory
		
	bless($self, $class);

	return $self;
}

sub add {
	my($self,$sprok,$DocID,$WordID,$antall,$HitList) = @_;
	
	#push(@{ $self->{'iindex'}->{$WordID} },pack("L L L$antall",$DocID,$antall,$HitList));



	
	${ $self->{'iindex'}->{$sprok}->{$WordID} }[$#{ $self->{'iindex'}->{$sprok}->{$WordID} } +1][0] = $antall;
	${ $self->{'iindex'}->{$sprok}->{$WordID} }[$#{ $self->{'iindex'}->{$sprok}->{$WordID} }][1] = $DocID;
	${ $self->{'iindex'}->{$sprok}->{$WordID} }[$#{ $self->{'iindex'}->{$sprok}->{$WordID} }][2] = $HitList;
	
#	print "$WordID ($self->{'AntallIIndexElementer'}): $#{ $self->{'iindex'}->{$sprok}->{$WordID} }\n";	
	
	#print "nn: $DocID,$count\n";
	
	$self->{'AntallIIndexElementer'}++;
}

sub WriteIIndexToFile {
	my($self,$type,$revindexFilNr,$lotNr,$subname) = @_;

	
	foreach my $sprok (keys %{ $self->{'iindex'} }) {
	
		#stores in the same place as the lot with the same number
		#my $LotPath = Boitho::Lot::GetFilePathForIindex($revindexFilNr);
		
		my $LotPath = Boitho::Lot::GetFilPathForLot($lotNr,$subname);
		#removes the trailing /
		chop($LotPath);

		#print "LotPath for $revindexFilNr: $LotPath\n";

		#5: my $indexPath = "data/iindex/$type/index/$sprok";
		my $indexPath = "$LotPath/iindex/$type/index/$sprok";
		if (not -e $indexPath ) {	#checks and creates the path if needed.
			#print "creating path $indexPath\n";
			MakePath($indexPath);
		}
		$fil = $indexPath  . '/' . $revindexFilNr . '.txt';
		open(IINDEX,">$fil") or die("Can't open $fil: $!");
		binmode(IINDEX);
	
		
		#5: $DictionaryPath  = "data/iindex/$type/dictionary/$sprok";
		$DictionaryPath  = "$LotPath/iindex/$type/dictionary/$sprok";
		if (not -e $path_komet_til) {	#checks and creates the path if needed.
			#print "creating path $DictionaryPath\n";
			MakePath($DictionaryPath);
		}
		$fil = $DictionaryPath   . '/' . $revindexFilNr . '.txt';
		open(IDICTIONARY ,">$fil") or die("Can't open $fil: $!");
		binmode(IDICTIONARY);
	
		#print "indexPath $indexPath, DictionaryPath $DictionaryPath\n";
		foreach my $term (sort {$a <=> $b} keys %{ $self->{'iindex'}->{$sprok} }) { # {$a <=> $b} to sort numerically, not alphabetically by ASCII which is the perl default.
		
			my $AntallSiderMedTerm = scalar(@{ $self->{'iindex'}->{$sprok}->{$term} });
		
			%TermHash = ();
			
			foreach my $i (@{ $self->{'iindex'}->{$sprok}->{$term} }) {
			
			
				#puts the data into a hash
				if (exists $TermHash{$i->[1]}) {
					#print "$i->[1] already exists\n";
					#exit;
					
					#if we already have info about this docID, add to the hit list and increment the hit count
					${ $TermHash{$i->[1]} }[0] += $i->[0]; #adds to the count
                                        ${ $TermHash{$i->[1]} }[1] .= $i->[2];    #adds to the hit list
				}	
				else {
					${ $TermHash{$i->[1]} }[0] = $i->[0]; #adds to the count
					${ $TermHash{$i->[1]} }[1] = $i->[2];	#adds to the hit list
				}		
			}

			my $countAntall = keys %TermHash;

			print IINDEX pack('L L',$term,$countAntall);

			#debug: shows what hits we have
			#print "term $term, count $countAntall\n";
			#sorts by DocID and builds the index
			foreach my $DocID (sort {$a <=> $b} keys %TermHash) {
					

				#temp: misc number tests			
				if (${ $TermHash{$DocID} }[0] =~ /\D/) {
        				die "not a number";
				}
				if (${ $TermHash{$DocID} }[0] > 10000) {
					print "lager then n: ${ $TermHash{$DocID} }[0]\n";
				}
				if (${ $TermHash{$DocID} }[0] < 1) {
                                        print "smaler then 1: ${ $TermHash{$DocID} }[0]\n";
                                }

				#print IINDEX pack('L L',$DocID,${ $TermHash{$DocID} }[0]);
				#14 may. language support
				print IINDEX pack('L A L',$DocID,0,${ $TermHash{$DocID} }[0]);

				print IINDEX ${ $TermHash{$DocID} }[1]; #adds the hit list
					
				#debug: shows things
				#print "\tDocID: $DocID count: ${ $TermHash{$DocID} }[0]\n";
			
			}
			
			#adds info about wordid and number of pages
			
			
		
			#print " SizeForTerm: $SizeForTerm ";
			#print "\n\n";

			#temp, this does not work. Use tell before and after to find the length		
			my $adress = tell(IINDEX) - $SizeForTerm;
			#print IDICTIONARY "$term $adress:$SizeForTerm\n";
			print IDICTIONARY pack(DictionaryTemplate,$term, $adress, $SizeForTerm);
		}
	
		close(IINDEX);
		close(IDICTIONARY);
	
		print "AntallIIndexElementer $self->{'AntallIIndexElementer'}\n";
	}
}

##########################################################################################
# Loads the dictionary into a hash
#
#	$self->{'DictionaryHash'}->{WordID} holds the entire dictionary
# 	maps WordID to address and size
#	Address and size are stored according to the constant DictionaryAdressSizeForTerm 
##########################################################################################
sub HashLodeDictionary {
	my($self) = @_;
	
	use File::Find;
	
	#$self->%DictionaryHash = {};
	$self->{'DictionaryHash'} = {};
	
	my %options = (wanted => \&list_filer, no_chdir=>1);

	find(\%options,'data/iindex');

	
	sub list_filer {
	
		if (!(-d $File::Find::name)) {
			#push(@languages,$File::Find::name);
			
			#print "$File::Find::name\n";
			#if we have a dictionary file, which then should have "dictionary_" in it
			if ($File::Find::name =~ /dictionary_/) {
					print "aa: $File::Find::name " . keys(%{ $self->{'DictionaryHash'} }) . "\n";
				
					#$dictionary_fil  = 'data/iindex/dictionary_' . $revindexFilNr . '.txt';
					open(IDICTIONARY ,"$File::Find::name") or die("Can't open dictionary $fil: $!");
					binmode(IDICTIONARY);
					
					local $/ = "**\cJ";
					
					my @inn = <IDICTIONARY>;
				
	
					foreach my $i (@inn) {
						chomp($i);
						my ($WordID, $adress, $SizeForTerm) = unpack(DictionaryTemplate, $i);
					
						#print "$WordID, $adress, $SizeForTerm\n";
						
						$self->{'DictionaryHash'}->{$WordID} = pack(DictionaryAdressSizeForTerm,$adress,$SizeForTerm);
					}
	
					close(IDICTIONARY);
			}
		}
	}
exit;
}
##########################################################################################
# Reads from the index
#
#	uses $self->{'DictionaryHash'}->{WordID} to find address and size
##########################################################################################
sub ReadIIndexRecordOLD {
	my($self,$WordID) = @_;

	#print "WordID $WordID" $self->{'DictionaryHash'}->{$WordID} . "\n";
	
	if (exists $self->{'DictionaryHash'}->{$WordID}) {
		my ($adress,$SizeForTerm) = unpack(DictionaryAdressSizeForTerm,$self->{'DictionaryHash'}->{$WordID});
	
		#print "WordID: $WordID, adress: $adress, SizeForTerm: $SizeForTerm\n";
	
		$revindexFilNr = fmod($WordID,AntallBarrals);
		my $fil = 'data/iindex/' . $revindexFilNr . '.txt';
	
		open(IINDEX,"$fil") or die("Can't open $fil: $!");
		binmode(IINDEX);
		
		seek(IINDEX,$adress,0);
	
		read(IINDEX,$inn,$SizeForTerm);
	
		close(IINDEX);
	
		#print "IINDEX: -$inn-\n";
	
		#my ($k,$j,$i) = unpack('L L S', $inn);

		#print "$k,$j,$i\n";
		return $inn;
	}
	else {
		return undef;
	}
}
##########################################################################################
# Binary searches to read a record from an index
#
##########################################################################################
sub ReadIIndexRecord {
	my($self,$type,$sprok,$Query_WordID) = @_;
	
	
	#print "L:" . unpack('L',pack('L',$Query_WordID)) . "\n";
	#print "Searching for $Query_WordID\n";
	#finds the correct file
	my $revindexFilNr = fmod($Query_WordID,AntallBarrals);
	
	my $dictionary_fil  = "data/iindex/$type/dictionary/$sprok/$revindexFilNr.txt";
	
	#tries to open files in tmpfs 
	open(IDICTIONARY ,"tmpfs/$dictionary_fil") or open(IDICTIONARY ,"$dictionary_fil") or die("Can't open dictionary $dictionary_fil: $!");
	binmode(IDICTIONARY);
		
	#print "Antall_poster: $halvert i fil $dictionary_fil\n";
	
	my $max = (stat(IDICTIONARY))[7] / DictionaryRecordSize;;
	my $min = 0;
	
	#print "\tmin: $min, max: $max, halvert: $halvert\n";
	
	
	#binary search
	my $seeks = 0;
	my $fant = 0;
	#if (0) {
	while ((($max - $min) > 1) && (not $fant)) {
	
		$halvert = floor(((($max - $min) / 2) + $min));
		$posisjon = $halvert * DictionaryRecordSize;
	
		#print "\tmin: $min, max: $max, halvert: $halvert\n";
	
		seek(IDICTIONARY,$posisjon,0);
	
		read(IDICTIONARY,$post,DictionaryRecordSize);
	
		($WordID, $adress, $SizeForTerm) = unpack(DictionaryTemplate,$post);
	
	
		if ($Query_WordID == $WordID) {
			#print "fant $WordID\n";
			#last;
			$fant = 1;
			#exit;
		}
		elsif ($Query_WordID > $WordID) {
			#print "$Query_WordID > $WordID\n";
			$min = $halvert;
		}
		elsif ($Query_WordID < $WordID) {
			#print "$Query_WordID < $WordID\n";
			$max = $halvert;
		}
		
		
		#print "$WordID, $adress, $SizeForTerm\n";
		#exit;
		$seeks++;
		
		if ($seeks == 50) {
			die("brukte mere en 50 seeks på å finne record i dictonary");
		}
	
	}
	
	
	
	#print "used $seeks attempts\n";
	
	close(IDICTIONARY);
	#}
	#print "WordID: $WordID, adress: $adress, SizeForTerm: $SizeForTerm\n";
	#$WordID = 365508689;
	#$adress = 1189918;
	#$SizeForTerm = 1040162;
	#$fant = 1;
	
	#if we found something we look it up in the index and read in the index
	if ($fant) {
		#$revindexFilNr = fmod($WordID,AntallBarrals);
		my $fil = "data/iindex/$type/index/$sprok/$revindexFilNr.txt";
	
		#print "adress: $adress, SizeForTerm: $SizeForTerm\n";
		
		open(IINDEX,"$fil") or die("Can't open $fil: $!");
		binmode(IINDEX);
		seek(IINDEX,$adress,0);
	
		read(IINDEX,$inn,$SizeForTerm);
	
		close(IINDEX);
	
		#print "IINDEX: -$inn-\n";
	
		#my ($k,$j,$i) = unpack('L L S', $inn);

		#print "$k,$j,$i\n";
		return $inn;
	}
	else {
		return undef;
	}
	
	#exit;
}

##########################################################################################
# Returns an array with terms, occurrences and positions for a single word. 
#
#	Inn:
#	WordID
#
#	return
#	string	-Number of pages that have this hit
#	array
#		DocID	- DocID of a page that has this word in it
#		Antall	- Number of hits on that page
#		Hitt	- Array of positions
#	uses $self->{'DictionaryHash'}->{WordID} to find address and size
##########################################################################################
sub GetIndexAsArray {
	my($self,$type,$sprok,$WordID) = @_;

	my $RawIndex = $self->ReadIIndexRecord($type,$sprok,$WordID);
	## temp ##
	#print "\nSequential search: \n";
	#$self->ReadIIndexRecordMedSekuensiseltSeach($WordID);
	#exit;
	# /temp ##
	
	if ($RawIndex ne undef) {
		#print "RawIndex: -$RawIndex-\n";
		my @treffArray = ();
		
		my $indexLengde = length($RawIndex);
	
	
		$StringPopHa = StringPop->new($RawIndex);

		my ($term,$AntallTreff) = unpack('L L',$StringPopHa->Pop(8));
	
		#print "\nterm: $term, AntallTreff: $AntallTreff\n";
		#loops through the entire index
		my $lest = 8;
		while ($indexLengde > $lest) {
			my ($DocID,$Antall) = unpack('L L',$StringPopHa->Pop(8));

			my @hits = unpack('S' x $Antall,$StringPopHa->Pop($Antall * 2));
	
			#print "$DocID,$Antall\n";
	
			$lest = $lest + 8 + ($Antall * 2);
	
			#for each hit do:
			#foreach my $i (@hits) {
			#	print "\thit: $i\n";
			#}
		
			#for each hit add points:
			my $poeng = 0;
	
			my $urlPoeng = 0;
			#my %points = ();
			foreach my $i (@hits) {
				
				if ($i <= 100) {
					$poeng = $poeng + poengUrl;
				}
				elsif ($i <= 500) {
					$poeng = $poeng + poengTittel;
				}
				elsif ($i <= 1000) {
					$poeng = $poeng + poengHeadline;
				}
				else {
					$poeng = $poeng + poengBody;
				}
				#if ($i >= 1000) {	#Body
				#	$poeng = $poeng + poengBody;
				#	#$points{'poengBody'} = $points{'poengBody'} + poengBody;
				#	#$points{'AntallBody'}++;
				#}
				#elsif ($i >= 500) {	#Headline
				#	$poeng = $poeng + poengHeadline;
				#	#$points{'poengHeadline'} = $points{'poengHeadline'} + poengHeadline;
				#	#$points{'AntallBody'}++;
				#}
				#elsif ($i >= 100) {	#Tittel
				#	$poeng = $poeng + poengTittel;
				#	#$points{'poengTittel'} = $points{'poengTittel'} + poengTittel;
				#	#$points{'AntallHeadline'}++;
				#}
				#elsif ($i >= 0) { #Url
				#	$poeng = $poeng + poengUrl;
				#	#$points{'poengUrl'} = $points{'poengUrl'} + poengUrl;
				#	#$points{'AntallUrl'}++;
				#	#$urlPoeng = $urlPoeng + poengUrl;
				#	print "aaaa\n";
				#}
				#else {
				#	warn("Unknown position $i");	
				#}
			}
			
			#my $poeng = 0;
			
			
			#if ($urlPoeng != 0) {
			#	$poeng += ($urlPoeng / (((log ($urlPoeng / poengUrl)) * 10) +1));
			#	#print "$poeng += ($points{'poengUrl'} / (((log $points{'AntallUrl'}) * 10) +1))\n"
			#}
			
			$treffArray[$#treffArray +1][0] = $DocID;
			$treffArray[$#treffArray][1] = $Antall;
			$treffArray[$#treffArray][2] = \@hits;
			$treffArray[$#treffArray][3] = $poeng;
			$treffArray[$#treffArray][4] = 0; #link weight
		}
		
		return $AntallTreff,@treffArray;
	}
	else {
		return undef;
	}
	
}

##########################################################################################
# Sorts an index array by points
#
##########################################################################################
sub SortIndexArrayByPoints {
	my($self,@Array) = @_;
	
	@Array = sort {$a->[3] <=> $b->[3] } @Array;
	@Array = reverse(@Array);
	
	return @Array;
}
