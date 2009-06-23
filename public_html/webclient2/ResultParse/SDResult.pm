package ResultParse::SDResult;
use strict;
use warnings;

use XML::Simple qw(XMLin :strict);
use Carp;
Readonly::Hash my %VALID_SORT => map { $_ => 1 } qw(relv oldest newest);

sub new {
	my ($class, $xml_str, $fatal_func, $query) = (shift, shift, shift, shift);
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

	if ($s->can('_init')) {
		$s->_init(@_);
	}

	return $s;
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
			version => $_->{version},
		} } ref $g->{item} eq "ARRAY" ? @{$g->{item}} : ($g->{item}) ];
	}

	return \%group;
}

sub sort_info {
	my ($s) = @_;
	
	my $relv = $s->filteradd($s->{query}, 'sort');
	my $new  = $s->filteradd($s->{query}, 'sort', 'sort', 'newest');
	my $old  = $s->filteradd($s->{query}, 'sort', 'sort', 'oldest');

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
# Filters in query.
# TODO: deprecate in newer versions. We want this in the XML result.
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


##
# Remove and/or add filter from/to query.
# TODO: deprecate in newer versions. We want this in the XML result.
sub filteradd {
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



1;
