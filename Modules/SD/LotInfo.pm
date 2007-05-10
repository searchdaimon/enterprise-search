##
# Class SD::LotInfo
# Fetches info about searchdaimon indexed lots.
package SD::LotInfo;

use strict;
use warnings;

use Carp;
use Data::Dumper;
use Filesys::Df;
use File::Size;

use constant MOUNTS_INFO => "/proc/mounts";
use constant DEBUG => 0;

my $maplist_path;
my %opt = ();


##
# Default constructor.
#
# Parameters:
#   maplist_path - Path to maplist config file.
sub new {
	my $class = shift;
	$maplist_path = shift;

	croak "argument maplist_path was not provided."
		unless defined $maplist_path;

	croak "Provided maplist config ($maplist_path) is not readable."
		unless -r $maplist_path; #is readable

	my $self = {};
	bless $self, $class;

	return $self;
}


##
# Generates lots data.
#
# See <Additional Info> for output example.
# 
# Attributes:
# 	options_ref - Options
#
# Returns:
#   lotspace - List with hashref containing data regarding lots.
#
# Available options:
# 	show_lot_size - Fetch size for indiviudal lot.
sub lots_info {
	my ($self, $options_ref) = @_;
	%opt = %{$options_ref} if defined $options_ref;

	my @lot_paths   = $self->get_lot_paths();
	my @mount_paths = $self->get_mount_paths();

	if (DEBUG) { print Dumper(\@mount_paths) }

	my %merged;
	# add lots to their mount paths.
	foreach my $lot (@lot_paths) {
		
		my $mount = $self->_match_mount(\@mount_paths, $lot);

		croak "Could not find mount path for lot $lot"
			unless defined $mount;
			
		$self->_add_lot(\%merged, $mount, $lot);
	}

	# create data struct, add mount size details.
	my @lotspace;
	while (my ($mount_path, $lot_ref) = each %merged) {
		if (DEBUG) { print "building structure for mount: $mount_path\n" }
		my ($size, $used, $left, $per) 
			= $self->_get_mount_df($mount_path);
		push @lotspace, {
			'mount' => $mount_path,
			'size'  => $size,
			'used'  => $used,
			'left'  => $left,
			'per'   => $per,
			'lots'  => $lot_ref,
		};
	}
	
	%opt = ();

	return @lotspace;
}

##
# Parses lot paths in config file to a list.
#
# Returns:
#   lots - List with lots paths.
sub get_lot_paths {
	my $self = shift;

    open my $maplist_h, "<", $maplist_path
		or croak "Unable to open maplist config file $maplist_path: $?";

    my @lots;
    while (my $lot_path = <$maplist_h>) {
		chomp $lot_path;
		push @lots, $lot_path;
    }

    return @lots;
}

##
# Parses MOUNTS_INFO to get list of mount paths.
#
# Returns:
#   mounts - List of mount paths
sub get_mount_paths {
	open my $mountinfo_h, "<", MOUNTS_INFO
		or croak "Unable to read info for mountet partitions (file ", MOUNTS_INFO, "): $?";

	my @mounts;
	while (my $mount = <$mountinfo_h>) {
		chomp $mount;
		# /proc/mounts escapes spaces as \040, so it's safe to split this way.
		my (undef, $path) = split q{ }, $mount;
		$path =~ s/\\040/ /g;

		push @mounts, $path;
		
		if (DEBUG) { print "got mount $path\n" }
		
	}

	# Sorts by longest paths first.
	@mounts = sort { length $b <=> length $a } @mounts;

	return @mounts;
}

# Group: Private methods

##
# Get disk data for a mount path.
#
# Attributes:
#	mount_path - Mount path.
#
# Returns:
#	size - Total disk size
#	used - Used disk space
#	left - Space left on disk
sub _get_mount_df {
	my ($self, $mount_path) = @_;
	my ($size, $used, $left, $percent_full);	

	if (DEBUG) { print "getting df info for mount $mount_path\n" }
	my $df_ref = df($mount_path);

	print Dumper(\$df_ref);	

	if (defined $df_ref) {
		$size         = $df_ref->{'blocks'};
		$used         = $df_ref->{'used'};
		$left	      = $df_ref->{'bfree'};
		$percent_full = $df_ref->{'per'};
	}
	else {
		carp "Unable to get disk usage for mount $mount_path";
		($size, $used, $left, $percent_full) 
			= qw(Unknown Unknown Unknown Unknown);
	}

	return ($size, $used, $left, $percent_full);
}

##
# Pushes a lot to the mount_path it belongs to.
# 
# Attributes:
#	merged_ref - Hashref with mount path as key, and lots arrayref as value.
# 	mount_path - Path to assign to.
# 	lot_path   - Path to lot.
sub _add_lot {
	my ($self, $merged_ref, $mount_path, $lot_path) = @_;

	croak "Missing argument(s) in _add_lot()"
		if grep { !defined $_ } $merged_ref, $mount_path, $lot_path;
	
	my %lot_info;
	$lot_info{'path'} = $lot_path;
	
	if ($opt{'show_lot_size'}) {
		my $fsize = File::Size->new(
			'dir'	    => $lot_path,
			'blocksize' => 1024,
			'followsymlinks' => 1,
			'humanreadable' => 0,
		);

		$lot_info{'size'} = $fsize->getsize();
	}
	else {
		$lot_info{'size'} = undef;
	}

	push @{$merged_ref->{$mount_path}}, \%lot_info;
}

##
# Find the mount path that a lot belongs to.
#
# NOTE: We asume that mount paths are sorted, longest path first. See <get_mount_paths>
sub _match_mount {
	my ($self, $mount_paths_ref, $lot_path) = @_;

	foreach my $mount (@{$mount_paths_ref}) {
		if ($lot_path =~ m{^$mount}) {
			return $mount
		}
	}

	return;
}

# Group: Additional Info
#
# Example output from <lots_info>
# >(	{
# >	'left' => 82531292,
# >	'used' => 32253148,
# >	'mount' => '/',
# >	'per' => 28,
# >	'size' => 114784440
# >	'lots' => [
# >		{
# >			'size' => '0',
# >			'path' => '/home/dagurval/src/boitho/websearch/lot/0'
# >		},
# >		{
# >			'size' => '0',
# >			'path' => '/home/dagurval/src/boitho/websearch/lot/1'
# >		},
# >		],
# >	},
# >	{
# >	'left' => 8335876,
# >	'used' => 284582716,
# >	'mount' => '/mnt/test test',
# >	'per' => 97,
# >	'size' => 292918592
# >	'lots' => [
# >		{
# >			'size' => '219399893',
# >			'path' => '/mnt/test test/lot'
# >		}
# >		],
# >	}
# >)




if (DEBUG) {
	my $ls = SD::LotInfo->new("maplist.conf");
	my @r = $ls->lots_info({'show_lot_size' => 1});
	print Dumper(\@r);
}
