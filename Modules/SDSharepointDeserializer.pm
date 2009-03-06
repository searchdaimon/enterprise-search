package SDSharepointDeserializer;

use Data::Dumper;

use SOAP::Lite;
use XML::XPath;
use XML::XPath::XMLParser;

use base SOAP::Deserializer;

sub new {
	my $self = shift;
	my $class = ref($self) || $self;
	return $self if ref $self;

# We are still a SOAP::Deserializer...
	my $base_object = $class->SUPER::new();

	$self->{xpath} = undef;

	return bless ($base_object, $class);
}

my %methods = (
	'GetRoleCollectionFromWeb' => [
		'Roles',
	],
);

my %values = (
	Roles => {
		children => ['Role'],
	},
	Role => {
		allattrs => 1,
	},
);

sub parse_value {
	my ($self, $ctx, $name) = @_;

	my %data = ();

	my $value = $values{$name};
	die "Unknown value: $name" unless defined $value;

	#print Dumper($value);

	print "Node: $name\n";
	if (defined $value->{children}) {
		foreach my $child (@{ $value->{children} }) {
			$data{$child} = $self->_parse_values($self->{xpath}->find($child, $ctx), $child);
		}
	}
	if (defined $value->{allattrs}) {
		print "allattrs\n";
		my %attrs = map { $_->getName => $_->getData } $ctx->getAttributes;
		print Dumper(\%attrs);
		$data{attributes} = \%attrs;
	}

	print "Values: \n" .Dumper(\%data);

	return %data;
}

sub _parse_values {
	my ($self, $set, $name) = @_;

	return () if $set->size == 0;
	return map { $self->parse_value($_, $name); } $set->get_nodelist;
}

sub _parse_value {
	my ($self, $set, $name) = @_;

	#print Dumper($context);
	# XXX: check for multiple nodes
	die unless $set->size == 1;
	return $self->parse_value(@{ $set->get_nodelist() }[0], $name);
}

sub parse_method {
	my ($self, $context, $name) = @_;

	print "Parsing: '$name'\n";
	my %values = map { $_ => $self->_parse_value($self->{xpath}->find($_, $context), $_); } @{ $methods{$name} };

	return \%values;
}

# This method takes the raw SOAP response as
# it's only argument.. so we store it - so we
# can get at it later.
sub deserialize {
	my ($self, $data) = @_;

	my $xml = XML::XPath::XMLParser->new(xml => $data);
	my $root = $xml->parse;
	my $xpath = new XML::XPath->new(context => $root);
	$self->{xpath} = $xpath;

	foreach my $key (keys %methods) {
		my $set = $xpath->find("//".$key."Result/$key");
		if ($set->isa('XML::XPath::NodeSet')) {
			foreach my $ctx ($set->get_nodelist) {
				print Dumper($self->parse_method($ctx, $key));
			}
		}
	}
	
	return [];
}

1;
