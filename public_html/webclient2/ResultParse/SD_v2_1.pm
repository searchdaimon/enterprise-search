package ResultParse::SD_v2_1;
use strict;
use warnings;

use List::Util qw(min);
use Carp;
use Data::Dumper;

use ResultParse::SDResult;
use config qw(%CFG);

our @ISA = qw(ResultParse::SDResult);


sub errors {
	my $s = shift;
	my $err_xml = $s->{xml}{error} || $s->{xml}{error};
	return unless $err_xml;
	return @{$err_xml->[0]{errormessage}};
}

sub res_info {
	my ($s, $page) = @_;
	my $res_info = $s->{xml}{result_info};
	
	my $res_from = min(
		($CFG{num_results} * ($page - 1)) + 1,
		$res_info->[0]{total});
	my $res_to = min($res_info->[0]{total},
			   $CFG{num_results} * $page);

	return {
		total_res     => $res_info->[0]{total},
		search_time   => $res_info->[0]{time},
		filtered      => $res_info->[0]{filtered},
		res_from      => $res_from,
		res_to        => $res_to,
		spelling      => {
			text   => $res_info->[0]{spellcheckedquery},
			query  => $res_info->[0]{spellcheckedquery},
		}
	}
}


sub collections {
	my $s = shift;
	my $coll_xml = $s->{xml}{collection};
	
	my %coll_info = (
		selected => undef,
		coll => [ ],
		all_query => $s->filteradd($s->{query}, 'collection'),
	);

	if (ref $coll_xml ne "ARRAY") {
		#Runarb 2010, tar bort denne da vi ikke sender collections hvis vi ikke har noen å søke i. For esk under /public
		#uten noen offentlige collectioner.
		#carp "No collections in XML";
		return \%coll_info;
	}

	for my $c_ref (@{$coll_xml}) {
		
		# "All" is the default tab (no coll selected)
		next if $c_ref->{name}[0] eq "All"; 

		$coll_info{selected}  = $c_ref->{name}[0]
			if $c_ref->{selected};

		push @{$coll_info{coll}}, {
			name    => $c_ref->{name}[0],
			results => $c_ref->{totalresultscount}[0], 
			query   => $s->filteradd(
				$s->{query}, 
				'collection', 
				'collection', 
				$c_ref->{name}[0]
			),
		};
	}
	return \%coll_info;
}


sub results {
	my $res_xml = shift->{xml}{result};
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
			title   => $r->{title}[0],
			url     => $r->{url}[0],
			uri     => $r->{uri}[0],
			fulluri => $r->{fulluri}[0],
			snippet => $r->{description}[0],
			age     => $r->{time_iso}[0],
			cache   => $r->{cache}[0],
			thumb   => $r->{thumbnail} ? $r->{thumbnail}[0]{content} : undef,
			filetype => $r->{filetype}[0],
			icon => $r->{icon}[0],
			attributes => \%attr,
			dupes => [ map { +{ 
				url => $_->{url}[0], 
				uri => $_->{uri}[0],
				fulluri => $_->{fulluri}[0],
			} } @{$r->{duplicateurl}} ],
		};
	}
	return \@res;
}





1;
