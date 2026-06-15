##########################################################################################
# Library for popping elements off a string
#
#	$self->{'LastCharacterAdress'} keeps track of where we are, so that we can use substr to
#	copy in what we want.
#
#	$self->{'InnString'} The string we are working on.
#
# 	For example fetch 10 characters, then the 5 that come after, and then 10 that come after that:
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
	my $self = {}; #allocate new hash for object
	
	$self->{'LastCharacterAdress'} = 0;	#overview of how many elements we have in memory
		
	$self->{'InnString'} = $InnString;	
	bless($self, $class);

	return $self;
}

sub Pop {
	my($self,$lengyh) = @_;

	my $data = substr($self->{'InnString'},$self->{'LastCharacterAdress'},$lengyh); # should maybe return right away here? But that causes problems with updating $self->{'LastCharacterAdress'}
	$self->{'LastCharacterAdress'} = $self->{'LastCharacterAdress'} + $lengyh;
	
	return $data;
}