
########################################################################################################
#honterer alt arbeid mot URL indekser
########################################################################################################

package urlindex;   
require Exporter;
@urlindex::ISA = qw(Exporter);
@urlindex::EXPORT = qw();
@urlindex::EXPORT_OK = qw(New);

#struct update_block
#{
#    unsigned char       sha1[20];
#    unsigned char       url[200];
#    unsigned char       linktext[50];
#    unsigned int        DocID_from;
#};

use constant NyeUrlerFilRecord => 'A20 A200 A50 L'; #sha1(nyUrl),nyUrl,linketekst,KildeDocID
use constant MasterIndexFilRecord => 'A40 L A200';
use constant MasterIndexFilRecordSize => 244;

use Digest::SHA1  qw(sha1_hex);

use Boitho::Lot;

#:4use common qw(count_handler);

#new funksjon
sub New {
	my($class,$LotNr) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for objekt
	
	if ($LotNr eq '') {
		die("No lot nr given");
	}
	my $Path = Boitho::Lot::GetFilPathForLot($LotNr);
	
	#$self->{'NyeUrlerFil'} = 'data/urlindex/nye.txt';
	$self->{'NyeUrlerFil'} = $Path . 'nyeurler';

	print "$self->{'NyeUrlerFil'}\n";	

	open ($self->{'NyeUrlerFH'},'>' . $self->{'NyeUrlerFil'}) or warn("Cant open $self->{'NyeUrlerFil'}: $!");
        binmode($self->{'NyeUrlerFH'});
	
	bless($self, $class);

	return $self;
}




sub addNewUrl {
	my($self,$sha1Vedi,$kildeDocID,$KildeUrl,$nyUrl,$urltext) = @_;
	

	print "$kildeDocID,$urltext\n";
	#if (length($url) <= 200) {
		#'A20 A200 A50 L'; sha1(nyUrl),nyUrl,linketekst,KildeDocID
		print { $self->{'NyeUrlerFH'} } pack(NyeUrlerFilRecord,$sha1Vedi,$nyUrl,$urltext,$kildeDocID);
		
	#}
}

sub FileClose {
	my $self = shift;
	
	if (exists $self->{'NyeUrlerFH'}) {
		close($self->{'NyeUrlerFH'});
	}
	
}

