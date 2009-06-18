package NavMenu;
use strict;
use warnings;
use Carp;
use Readonly;
use Exporter qw(import);
our @EXPORT_OK = qw(read_navmenu_cfg);

Readonly::Scalar my $NAVMENU_CFG_PATH => q{config/%s/navmenu.cfg};

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

sub _strip_whitespace { 
	map { $_ =~ s/(^\s+|\s+$)//g; $_ } @_ 
}

sub _remove_comments {
	map { $_ =~ s/#.*$//g; $_ } @_;
}


1;
