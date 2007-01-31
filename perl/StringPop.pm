##########################################################################################
# Biblotek for å kunne poppe av elementer fra en string
#
#	$self->{'LastCharacterAdress'} holder hvor vi er, slik at vi kan bruke substr til 
#	å kopiere inn det vi vil ha.
#
#	$self->{'InnString'} Stringen vi jobber på.
#
# 	For eksempel henet 10 tegn, så de 5 som kommer etter, og så 10 som kommer etter der:
#
#	$ha = StringPop->new($RawIndex);
#
#	$resultat = $ha->Pop(10);
#	$resultat = $ha->Pop(5);
#	$resultat = $ha->Pop(10);
##########################################################################################
package StringPop;   
require Exporter;
@StringPop::ISA = qw(Exporter);
@StringPop::EXPORT = qw();
@StringPop::EXPORT_OK = qw(new);

sub new {
	my($class,$InnString) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for objekt
	
	$self->{'LastCharacterAdress'} = 0;	#oversikt over hvor mange elemeneter vi har i minne
		
	$self->{'InnString'} = $InnString;	
	bless($self, $class);

	return $self;
}

sub Pop {
	my($self,$lengyh) = @_;

	my $data = substr($self->{'InnString'},$self->{'LastCharacterAdress'},$lengyh); # bør kansje returnere med en gang her? Men blir problemer med å oppdatere $self->{'LastCharacterAdress'} da
	$self->{'LastCharacterAdress'} = $self->{'LastCharacterAdress'} + $lengyh;
	
	return $data;
}