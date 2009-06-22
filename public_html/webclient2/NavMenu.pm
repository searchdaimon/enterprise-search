package NavMenu;
use strict;
use warnings;
use Carp;
use Readonly;
use Exporter qw(import);
use Data::Dumper;
our @EXPORT_OK = qw(read_navmenu_cfg);

Readonly::Scalar my $NAVMENU_CFG_PATH => q{config/%s/navmenu.cfg};
Readonly::Scalar my $SORT_ALPHA => "alpha";
Readonly::Scalar my $SORT_ALPHA_REVERSED => "alpha_reversed";

sub read_navmenu_cfg {
	my $tpl_name = shift;
	
	my $path = sprintf $NAVMENU_CFG_PATH, $tpl_name;
	open my $fh, "<", $path
		or croak "Unable to open navigation menu config ($path): ", $!;
	
	return join "\n", 
		grep { $_ ne q{} } # remove empty lines
		_strip_whitespace(
		_remove_comments(<$fh>));
}

sub _transl_group {
	my ($group_ref, $i18n) = @_;

	_transl_group($_, $i18n)
		for @{$group_ref->{groups}};
	
	for my $i (@{$group_ref->{items}}) {
		$i->{name} = &$i18n($i->{name});
	}

	$group_ref->{name} = &$i18n($group_ref->{name});
}

##
# Navigation needs to be translated before
# used in template for sorting. 
sub translate_nav {
	my ($nav_ref, $i18n) = @_;

	_transl_group($_, $i18n)
		for @{$nav_ref->{groups}};
}

sub _sort_group {
	my $group_ref = shift;

	_sort_group($_) for @{$group_ref->{groups}};

	my $sort_method = $group_ref->{sort};
	return unless $sort_method;

	my $sort;
	if ($sort_method eq $SORT_ALPHA) {
		$sort = sub { $a->{name} cmp $b->{name} };
	}
	elsif ($sort_method eq $SORT_ALPHA_REVERSED) {
		$sort = sub { $b->{name} cmp $a->{name} };
	}
	else { croak "Unknown sorting method $sort_method" }

	$group_ref->{groups} = [ sort $sort @{$group_ref->{groups}} ];
}

sub sort_nav {
	my $nav_ref = shift;
	
	_sort_group($_) for @{$nav_ref->{groups}};
}


sub _strip_whitespace { 
	map { $_ =~ s/(^\s+|\s+$)//g; $_ } @_ 
}

sub _remove_comments {
	map { $_ =~ s/#.*$//g; $_ } @_;
}


1;
