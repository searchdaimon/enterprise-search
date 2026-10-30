
package revindex;
require Exporter;
@revindex::ISA = qw(Exporter);
@revindex::EXPORT = qw();
@revindex::EXPORT_OK = qw(OpenForApending FileClose ApendAray StoreRevIndexNET OpenForApendingNET);

use Boitho::Lot;
use IO::File;

use constant AntallBarrals => 64; # must be equal to the value in iindex.pm

#new function
#
# input variables:
# $updateMode, which kind of update we want to do, either >> for appending, or > for overwriting, same as for files
sub OpenForApending {
	my($class,$type,$LotNr,$updateMode,$subname) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for object
	
	if ($LotNr eq '') {
		die("No lot nr given");
	}
	if ($updateMode eq '') {
		die("No update mode given");
	}
	#loads the crc32 module
	use String::CRC32;
	use POSIX;
	
	#opens (and creates files if necessary)
	my $i = 0;

	#gets the lot path, so we know where to store
	my $Path = Boitho::Lot::GetFilPathForLot($LotNr,$subname);
	$Path = $Path . '/revindex/' . $type;
	
	#checks whether that path exists, if not we create it
	if (not -e $Path) {
	
		use common qw(MakePath);
		MakePath($Path);
	}
	
	for (0 .. AntallBarrals) {
	
		#ToDo: does not include "type" here
		open($self->{filhonterere}->{$i},$updateMode . $Path . "/$i.txt") or die("Can't open $Path/$i.txt for $updateMode: $!");
		binmode($self->{filhonterere}->{$i});
		$i++;
	}
	bless($self, $class);
	return $self;
}

sub ApendAray {
	my($self,$DocID,$sprok,%termer) = @_;
	my $crc32;
	my $bukket;
	
	#writes DocID
	for my $i (0 .. AntallBarrals) {
		#print "DocID: $DocID\n";
		#print { $self->{filhonterere}->{$i} } pack('L A3',$DocID,$sprok);
		$self->{inalisert}->{$i} = 0;
	} 
	
	#writes the array
	foreach my $i (keys %termer) {
		#print "i: $i\n";
		
		#works on the word in $i
		my $crc32 = crc32($i);
		
		my $bukket = fmod($crc32,AntallBarrals);
	
		if (!$self->{inalisert}->{$bukket}) {
			
			#print { $self->{filhonterere}->{$bukket} } pack('L A3',$DocID,$sprok);
			#14 may 2007. Changing it to use a number as the language. Must add support for finding the right one,
			#not just writing 0
			print { $self->{filhonterere}->{$bukket} } pack('L A',$DocID,0);
			$self->{inalisert}->{$bukket} = 1;
		}
	
		print { $self->{filhonterere}->{$bukket} } pack('L L',$crc32,scalar(@{$termer{$i}})); #use something smaller than L here, maybe 16 bit ?
		
		#writes all the occurrences one after another
		foreach my $y (@{$termer{$i}}) {
		
			#print "ii: $i= $y->[1]: $y->[0] leges i $bukket\n";
		
			if ($y->[1] eq 'Body') {
				$y->[0] = $y->[0] + 1000;
			}
			elsif ($y->[1] eq 'Headline') {
				$y->[0] = $y->[0] + 500;
			}
			elsif ($y->[1] eq 'Tittel') {
				$y->[0] = $y->[0] + 100;
			}
			elsif ($y->[1] eq 'Url') {
					#$y->[0] = $y->[0] + 0;
					$y->[0] = $y->[0] + 1;
			}
			elsif ($y->[1] eq 'Anchor') {
					#$y->[0] = $y->[0] + 0;
			}
			else {
				die("Ukjent posisjon $y->[1]");
			}
			#print "$y->[1]:$y->[0]\n";
			
			#print { $self->{filhonterere}->{$bukket} } "$y->[1]:$y->[0],";
			print { $self->{filhonterere}->{$bukket} } pack('S',$y->[0]);	#Writes the hit
			#print { $self->{filhonterere}->{$bukket} } HitListCode($y->[1],$y->[0]);
		}
		#exit;
		
	}
	
	#writes **\n to mark the end of a record

	for my $i (0 .. AntallBarrals) {
		#if we wrote any data
		if ($self->{inalisert}->{$i}) {
			print { $self->{filhonterere}->{$i} } "**\cJ";
		}
	} 
}

sub FileClose {
	my $self = shift;
	#closes the file handles
	my $i = 0;
	for (0 .. AntallBarrals) {
		close($self->{filhonterere}->{$i})or die("Cant close: $!");
	
		$i++;
	} 
}

#new function
#
# input variables:
# $updateMode, which kind of update we want to do, either >> for appending, or > for overwriting, same as for files
sub OpenForApendingNET {
        my($class,$type,$LotNr,$HostName) = @_;
        #my $class = '';
        my $self = {}; #allocate new hash for object

        if ($LotNr eq '') {
                die("No lot nr given");
        }

        $self->{type} = $type;

        $self->{LotNr} = $LotNr;
        $self->{HostName} = $HostName;

        #opens (and creates files if necessary)
        for my $i (0 .. AntallBarrals) {

                #open($self->{filhonterere}->{$i},">>data/revindex/$type/$i.txt") or die("Can't open $i.txt for appending: $!");
                #ToDo: does not include "type" here
                #open($self->{filhonterere}->{$i},$updateMode . $Path . "/$i.txt") or die("Can't open $Path/$i.txt for $updateMode: $!");

                #creates a temporary file
                do { $self->{name}->{$i} = tmpnam() }
                        until $self->{filhonterere}->{$i} = IO::File->new($self->{name}->{$i}, O_RDWR|O_CREAT|O_EXCL);

                #END { unlink $self->{filhonterere}->{$i} or die("Could'n unlink $self->{name}->{$i}: $!") }

                binmode($self->{filhonterere}->{$i});

                #print "tmp file: $self->{name}->{$i}\n";

        }

        bless($self, $class);
        return $self;

}

sub StoreRevIndexNET {
        my $self = shift;
        #closes the file handles

        for my $i (0 .. AntallBarrals) {
                close($self->{filhonterere}->{$i}) or die("Cant close: $!");

                #sends the file

                #deletes the file
                my $dest = revindex . '/' . $self->{type} . '/' . $i . '.txt';
                #print "sending: $dest to $self->{HostName}\n";

                Boitho::Reposetory::rSendFile($self->{name}->{$i}, $dest, $self->{LotNr}, $self->{HostName});

                unlink($self->{name}->{$i}) or warn ("Can't  unlink $self->{name}->{$i}: $!");

        }
}

