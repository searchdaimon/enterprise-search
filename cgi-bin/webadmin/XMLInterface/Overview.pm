# Class: XMLInterface::Overview
# Provides XML data for webadmin overview.
package XMLInterface::Overview;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Sql::Shares;
use XMLInterface::Abstract;
use Common::Data::Overview;
use XML::Writer;
use IO::String;

our @ISA = qw(XMLInterface::Abstract);

my $dbh;
my $sqlShares;
my $dataOverview;

sub _init {
	my $self = shift;
	$dbh = $self->{'dbh'};
	$dataOverview = Common::Data::Overview->new($dbh);
}

##
# Returns XML with all data needed to generate overview.
# See bottom of document for output example.
#
# Returns:
#	xmlstring - XML with overview data.
sub get_full_overview() {
	my $self = shift;
	my @connectors = $dataOverview->get_connectors_with_collections();
	my $xml = q{};
	my $output = IO::String->new($xml);
	my $wr = XML::Writer->new(OUTPUT => $output, DATA_MODE => 1,  DATA_INDENT => 1);

	$wr->startTag('connectors');

	
	foreach my $connector_ref (@connectors) {
		$wr->startTag('connector');	
		$wr->dataElement('name', $connector_ref->{'name'});
		
		
		$wr->startTag('collections');
		foreach my $collection_ref (@{$connector_ref->{'shares'}}) {
			#carp "Ny collection: ", Dumper($collection_ref);
			$wr->startTag('collection');
			$self->_add_key_value_nodes(\$wr, $collection_ref);
			$wr->endTag('collection');
		}
		$wr->endTag('collections');
		$wr->endTag('connector');
		
	}
	
	$wr->endTag('connectors');

	return $xml;
}

##
# Adds node for all keys in a hashref.
# Example: 'user' => 'John Doe' becomes a <user>John Doe</user> node.
#
# Attributes:
#	xml_writer_ref - Pointer to the XML Writer instance to use.
#	hashref    - Hashref with values to add.
sub _add_key_value_nodes {
	my ($self, $xml_writer_ref, $hashref) = @_;
	my $wr = ${$xml_writer_ref};
	while (my ($key, $value) = each %{$hashref}) {
		#return unless $key and $value;
		${$xml_writer_ref}->dataElement($key, $value);
	}

	#return $xml_writer;
	1;
}

1;