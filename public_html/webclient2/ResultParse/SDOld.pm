##
# Parse old XML result set.
package ResultParse::SDOld;
use strict;
use warnings;

use XML::Simple qw(XMLin :strict);
use Carp;
use Data::Dumper;
use Readonly;
use List::Util qw(min);
	
use config qw(%CFG);

Readonly::Hash my %VALID_SORT => map { $_ => 1 } qw(relv oldest newest);

sub new {
	my ($class, $xml_str, $fatal_func, $query) = @_;
	utf8::encode($xml_str);
	my $xml;
	eval { $xml = XMLin($xml_str, ForceArray => 1, KeyAttr => [], SuppressEmpty => 1) };
	if ($@) {
		&$fatal_func("XML parse error: " . $@);
		return;
	}
	
	my $s = bless { 
		xml => $xml,
		fatal => $fatal_func,
		query => $query,
	}, $class;
	$s->{filters} = { $s->_query_filters($query) };
	return $s;
}

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
		all_query => $s->_filteradd($s->{query}, 'collection'),
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
			query   => $s->_filteradd(
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

sub navigation {
	my $s = shift;
	my $nav_xml = $s->{xml}{navigation}[0];
	my %nav_info = (
		return_query => undef,
		groups => [ ],
	);
	warn "No navigation in XML"
		unless $nav_xml;
	return \%nav_info unless $nav_xml;

	$nav_info{return_query} = $nav_xml->{query};

	return \%nav_info
		unless $nav_xml->{group};
	
	for my $g (@{$nav_xml->{group}}) {
		push @{$nav_info{groups}}, $s->_build_group($g);
	}
	return \%nav_info;
}

sub _build_group {
	my ($s, $g) = @_;

	my %group = (
		name     => $g->{name},
		expanded => $g->{expanded} eq "true" ? 1 : 0,
		hits     => $g->{hits},
		items    => [ ],
		query    => $g->{query},
		selected => $g->{selected} ? 1 : 0,
		groups   => [ ],
		icon     => $g->{icon},
		sort     => $g->{sort},
	);

	if ($g->{group}) {
		$group{groups} = [ map { 
				$s->_build_group($_) 
		} @{$g->{group}} ];
	}

	if ($g->{item}) {
		$group{items}  = [ map { +{
			name  => $_->{name},
			hits  => $_->{hits},
			query => $_->{query},
			selected => $_->{selected} ? 1 : 0,
			icon  => $_->{icon},
		} } ref $g->{item} eq "ARRAY" ? @{$g->{item}} : ($g->{item}) ];
	}

	return \%group;
}

sub sort_info {
	my ($s) = @_;
	
	my $relv = $s->_filteradd($s->{query}, 'sort');
	my $new  = $s->_filteradd($s->{query}, 'sort', 'sort', 'newest');
	my $old  = $s->_filteradd($s->{query}, 'sort', 'sort', 'oldest');

	my $current_sort = lc $s->{filters}{'sort'}[0];
	$current_sort = "relv"
		unless defined $current_sort && $VALID_SORT{$current_sort};

	return {
		current => $current_sort,
		query   => {
			relv    => $relv,
			newest  => $new,
			oldest  => $old,
		}
	};
}

sub filters { shift->{filters} }


##
# Remove and/or add filter from/to query.
# TODO: deprecate. We want this in the XML result.
sub _filteradd {
	my ($s, $query, $remove_filter, @add) = @_;

	if ($remove_filter) {
		#$query =~ s/\b\Q$remove_filter\E:"?((?:\w| )+)"?(?:\Z|\s)//g;
		$query =~ s/\b\Q$remove_filter\E\s*:\s*(\"[^\"]*\"|[^\s]*)//g;
		$query =~ s/^\s+|\s+$//g;
	}

	if (@add) {
		carp "Added filter '$add[0]' is missing a value"
			unless defined $add[1];
		$query .= " $add[0]:$add[1]";
	}

	return $query;
}


##
# Filters in query.
# TODO: deprecate. We want this in the XML result.
sub _query_filters {
	my ($s, $query) = @_;
	my %filters;
	while ($query =~ /\b(\w+):(\w+)\b/g) {
		push @{$filters{lc $1}}, $2;
	}
	while ($query =~ /\b(\w+):"((?:\w| )+)"(?:\Z|\s)/g) {
		push @{$filters{lc $1}}, $2;
	}
	return %filters;
}


1;

