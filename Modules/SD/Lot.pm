##
# Fetch lot document data.
# Wrapper class for SD::Lot::XS
package SD::Lot;
use SD::Lot::XS;
use Params::Validate qw(validate_pos);
use strict;
use warnings;

sub new {
	validate_pos(@_, 1, { regex => qr(^\d+$) }, 0);
	my ($class, $lot_nr, $subname) = @_;
	$subname ||= "";
	SD::Lot::XS::lot_init($lot_nr, $subname);
	return bless {
		lot_size => SD::Lot::XS::lot_doc_count(),
	}, shift;
}

sub doc_rank {
	my ($s, $doc) = @_;
	return (
		rank => SD::Lot::XS::get_doc_rank($doc)
	);
}

sub doc_info {
	my ($s, $doc) = @_;
	my @d = SD::Lot::XS::get_doc_info($doc);
	return (
		url				=> $d[0],
		lang			=> $d[1],
		# offensive_code deprecated,
		doctype			=> $d[3],
		crawl_time		=> $d[4],
		failed_crawls	=> $d[5],
		adult_weight    => $d[6],
		repository_ptr  => $d[7],
		html_size		=> $d[8],
		image_size		=> $d[9],
		resource_ptr	=> $d[10],
		ip_addr			=> $d[11],
		http_response	=> $d[12],
		user_id			=> $d[13],
		client_version	=> $d[14],
		num_out_link	=> $d[15],
		summary_ptr		=> $d[16],
		summary_size	=> $d[17],
		crc32			=> $d[18],
		last_seen		=> $d[19],
	);
}

sub size { return $_[0]->{lot_size} }

sub DESTROY {
	SD::Lot::XS::lot_close();
}


#use Data::Dumper;
#my $s = SD::Lot->new(252);
#for my $i (1 .. $s->size) {
#	print Dumper({$s->doc_rank($i)}, {$s->doc_info($i)});
#}
1;
