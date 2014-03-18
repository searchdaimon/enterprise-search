##
# Parse old XML result set.
package ResultParse::SDOld;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;
use List::Util qw(min);

use ResultParse::SDResult;
use config qw(%CFG);
our @ISA = qw(ResultParse::SDResult);



sub res_info {
	my ($s, $page) = @_;
	my $res_info = $s->{xml}{RESULT_INFO};
	
	my $res_from = min(
		($CFG{num_results} * ($page - 1)) + 1,
		$res_info->[0]{TOTAL});
	my $res_to   = min($res_info->[0]{TOTAL},
			   $CFG{num_results} * $page);

	#carp "From $res_from To $res_to ";

	return {
		total_res     => $res_info->[0]{TOTAL},
		search_time   => $res_info->[0]{TIME},
		filtered      => $res_info->[0]{FILTERED},
		filtered      => $res_info->[0]{QUERY},
		res_from      => $res_from,
		res_to        => $res_to,
		spelling      => {
			text   => $res_info->[0]{SPELLCHECKEDQUERY},
			query  => $res_info->[0]{SPELLCHECKEDQUERY},
		}
	}
}


sub collections {
	my $s = shift;
	my $coll_xml = $s->{xml}{COLLECTION};
	
	my %coll_info = (
		selected => undef,
		coll => [ ],
		all_query => $s->filteradd($s->{query}, 'collection'),
	);

	if (ref $coll_xml ne "ARRAY") {
		carp "No collections in XML";
		return %coll_info;
	}

	for my $c_ref (@{$coll_xml}) {
		
		# "All" is the default tab (no coll selected)
		next if $c_ref->{NAME}[0] eq "All"; 

		$coll_info{selected}  = $c_ref->{NAME}[0]
			if $c_ref->{SELECTED};

		push @{$coll_info{coll}}, {
			name    => $c_ref->{NAME}[0],
			results => $c_ref->{TOTALRESULTSCOUNT}[0], 
			query   => $s->filteradd(
				$s->{query}, 
				'collection', 
				'collection', 
				$c_ref->{NAME}[0]
			),
		};
	}
	return \%coll_info;
}

sub errors {
	my $s = shift;
	my $err_xml = $s->{xml}{ERROR} || $s->{xml}{error};
	return unless $err_xml;
	return map { $_->{ERRORMESSAGE} 
		? @{$_->{ERRORMESSAGE}} 
		: @{$_->{errormessage}} } @{$err_xml};

}

sub results {
	my $res_xml = shift->{xml}{RESULT};
	return [ ] unless $res_xml;

	my @res;
	foreach my $r (@{$res_xml}) {
		my %attr;
		if ($r->{attributes}[0]{attribute}) {
			%attr = map {
				$_->{key} => {
					value => $_->{value},
					query => $_->{query},
					attr_query => $_->{attribute_query},
				}
			} @{$r->{attributes}[0]{attribute}};
		}

		push @res, {
			title   => $r->{TITLE}[0],
			url     => $r->{URL}[0],
			uri     => $r->{URI}[0],
			fulluri => $r->{FULLURI}[0],
			snippet => $r->{DESCRIPTION}[0],
			age     => $r->{TIME_ISO}[0],
			cache   => $r->{CACHE}[0],
			thumb   => $r->{THUMBNAIL}[0],
			filetype => $r->{filetype}[0],
			icon => $r->{icon}[0],
			attributes => \%attr,
			dupes => [ map { +{ 
				url => $_->{URL}[0], 
				uri => $_->{URI}[0],
				fulluri => $_->{FULLURI}[0],
			} } @{$r->{DUPLICATESURLS}} ],
		};
	}
	return \@res;
}





1;

