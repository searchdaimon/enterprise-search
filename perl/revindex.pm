
package revindex;
require Exporter;
@revindex::ISA = qw(Exporter);
@revindex::EXPORT = qw();
@revindex::EXPORT_OK = qw(OpenForApending FileClose ApendAray StoreRevIndexNET OpenForApendingNET);

use Boitho::Lot;
use IO::File;

use constant AntallBarrals => 64; # må være lik som i iindex.pm

#new funksjon
# 
# innvariabler:
# $updateMode, hvilken type oppdateing vi skal gjøre, enten >> for appending, eller > for overskriving, samme som for filer
sub OpenForApending {
	my($class,$type,$LotNr,$updateMode) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for objekt
	
	if ($LotNr eq '') {
		die("No lot nr given");
	}
	if ($updateMode eq '') {
		die("No update mode given");
	}
	#laster crc32 modulen
	use String::CRC32;
	use POSIX;
	
	#oppner (og eventuelt lager filer)
	my $i = 0;

	#henter lot path, slik at vi vet hvor vi skal lagre
	my $Path = Boitho::Lot::GetFilPathForLot($LotNr);
	$Path = $Path . '/revindex/' . $type;
	
	#sjekker om den pathen fins, hvis ikke lagrer vi den
	if (not -e $Path) {
	
		use common qw(MakePath);
		MakePath($Path);
	}
	
	for (0 .. AntallBarrals) {
	
		#ToDo: tar ikke med "type" her
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
	
	#skriver DocID
	for my $i (0 .. AntallBarrals) {
		#print "DocID: $DocID\n";
		#print { $self->{filhonterere}->{$i} } pack('L A3',$DocID,$sprok);
		$self->{inalisert}->{$i} = 0;
	} 
	
	#skeriver arrayen
	foreach my $i (keys %termer) {
		#print "i: $i\n";
		
		#jobber på ord i $i
		my $crc32 = crc32($i);
		
		my $bukket = fmod($crc32,AntallBarrals);
	
		if (!$self->{inalisert}->{$bukket}) {
			print { $self->{filhonterere}->{$bukket} } pack('L A3',$DocID,$sprok);
			$self->{inalisert}->{$bukket} = 1;
		}
	
		print { $self->{filhonterere}->{$bukket} } pack('L L',$crc32,scalar(@{$termer{$i}})); #bruke noe mindre en L her, kansje 16 bit ?
		
		#skriver alle forekomstene etter hverandre
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
			elsif ($y->[1] eq 'Athor') {
					#$y->[0] = $y->[0] + 0;
			}
			else {
				die("Ukjent posisjon $y->[1]");
			}
			#print "$y->[1]:$y->[0]\n";
			
			#print { $self->{filhonterere}->{$bukket} } "$y->[1]:$y->[0],";
			print { $self->{filhonterere}->{$bukket} } pack('S',$y->[0]);	#Skriver treffet
			#print { $self->{filhonterere}->{$bukket} } HitListCode($y->[1],$y->[0]);
		}
		#exit;
		
	}
	
	#skriver **\n får å markere slut på record

	for my $i (0 .. AntallBarrals) {
		#hvis vi skrev noe data
		if ($self->{inalisert}->{$i}) {
			print { $self->{filhonterere}->{$i} } "**\cJ";
		}
	} 
}

sub FileClose {
	my $self = shift;
	#lokker filhonetrerene
	my $i = 0;
	for (0 .. AntallBarrals) {
		close($self->{filhonterere}->{$i})or die("Cant close: $!");
	
		$i++;
	} 
}

#new funksjon
#
# innvariabler:
# $updateMode, hvilken type oppdateing vi skal gjøre, enten >> for appending, eller > for overskriving, samme som for filer
sub OpenForApendingNET {
        my($class,$type,$LotNr,$HostName) = @_;
        #my $class = '';
        my $self = {}; #allocate new hash for objekt

        if ($LotNr eq '') {
                die("No lot nr given");
        }

        $self->{type} = $type;

        $self->{LotNr} = $LotNr;
        $self->{HostName} = $HostName;

        #oppner (og eventuelt lager filer)
        for my $i (0 .. AntallBarrals) {

                #open($self->{filhonterere}->{$i},">>data/revindex/$type/$i.txt") or die("Can't open $i.txt for appending: $!");
                #ToDo: tar ikke med "type" her
                #open($self->{filhonterere}->{$i},$updateMode . $Path . "/$i.txt") or die("Can't open $Path/$i.txt for $updateMode: $!");

                #oppretter en midlertidig fil
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
        #lokker filhonetrerene

        for my $i (0 .. AntallBarrals) {
                close($self->{filhonterere}->{$i}) or die("Cant close: $!");

                #sender filen

                #sletter filen
                my $dest = revindex . '/' . $self->{type} . '/' . $i . '.txt';
                #print "sending: $dest to $self->{HostName}\n";

                Boitho::Reposetory::rSendFile($self->{name}->{$i}, $dest, $self->{LotNr}, $self->{HostName});

                unlink($self->{name}->{$i}) or warn ("Can't  unlink $self->{name}->{$i}: $!");

        }
}

