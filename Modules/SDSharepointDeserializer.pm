package SDSharepointDeserializer;

use Data::Dumper;

use SOAP::Lite;
use XML::Simple qw(:strict);

use base SOAP::Deserializer;

sub new {
	my $self = shift;
	my $class = ref($self) || $self;
	return $self if ref $self;

# We are still a SOAP::Deserializer...
	my $base_object = $class->SUPER::new();

	return bless ($base_object, $class);
}

sub deserialize {
	my ($self, $data) = @_;

	#print 'XML: '.$data."\n";

	my $ref = XMLin($data, ForceArray => ['z:row'], KeyAttr => []);

	#print Dumper($ref);

	if (exists $ref->{'soap:Body'}) {
		$ref = $ref->{'soap:Body'};
	}

	# Remove result and reponse keys
	foreach $x (qw(Response Result Collection)) {
		my @keys = grep(!/^xmlns$/, keys %{$ref});
		if (scalar @keys == 1) {
			my $key = $keys[0];
			if ($key =~ /$x/) {
				$ref = $ref->{$key};
			}
		}
	}

	# Fix plural keys
	# $ref->{keys}->{key}[] -> $ref->{keys}[]
	{
		foreach my $key (keys %{$ref}) {
			my @keysinner = keys %{ $ref->{$key} };
			if (scalar @keysinner == 1) {
				my $keyinner = $keysinner[0];

				if ($key eq $keyinner . 's') {
					$ref = {
						$key => $ref->{$key}->{$keyinner},
					};
				}
			}
		}
	}
	

	return $ref;
}

1;
