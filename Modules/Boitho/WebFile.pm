# Namespace: Boitho::WebFile
# Fetch images/css files in cgi-bin dir.
#
# Asumes:
#   - css files are in dir 'styles'
#   - images are in dir 'icon/size'
package Boitho::WebFile;
use strict;
use warnings;
use Carp;
use CGI;
use CGI::State;
use Exporter;
our @EXPORT_OK = qw(get);

my $cgi = CGI->new;
my $state_ptr = CGI::State->state($cgi);
my %state = %$state_ptr;
    

my @default_valid_size = qw(16x16 22x22 32x32 48x48 64x64 128x128 other);
my @default_valid_ext  = qw(png jpg jpeg);

##
# Get file
#
# Attributes:
#   use_ptr - What we can fetch. Can contain 'css' and 'image'.
sub get {
    my ($use_ptr, $valid_size_ptr, $valid_ext_ptr) = @_;
    
    # Initialize
    my ($use_images, $use_css);
    foreach my $use (@$use_ptr) {
	$use_images = 1 if $use eq 'image';
	$use_css    = 1 if $use eq 'css';
    }

    my @valid_size = (defined $valid_size_ptr) ? @$valid_size_ptr : @default_valid_size;
    my @valid_ext  = (defined $valid_ext_ptr ) ? @$valid_ext_ptr  : @default_valid_ext;

	# default values
    my $size = '32x32' if grep /^32x32$/, @valid_size;
    my $ext  = 'png'   if grep /^png$/, @valid_ext;
    my $icon = undef;
    my $css  = undef;

    # Start

    ($size, $ext, $icon, $css) = fetch_input(\@valid_size, \@valid_ext, $size, $ext, $icon, $css);

    if (defined $icon) {
	croak "Missing size, ext or icon" 
	    if grep { !defined $_ } $size, $ext;
	&print_image($size, $ext, $icon) if $use_images;
    }
    elsif (defined $css) {
	&print_css($css) if $use_css;
    }
    else {
	croak "Wrong parameters provided";
    }
}

##
# Prints content of given image
#
# Attributes:
#   size - image size
#   icon - image name
#   ext  - image extension
sub print_image() {
	my ($size, $ext, $icon) = @_;
	my $path = "icon/$size/$icon.$ext";
	die "Icon $path does not exist."
		unless -e $path;

	print $cgi->header("image/$ext");
	open my $image, '<', $path
		or die "Could not open image $path", $?;

	while (my $content = <$image>) {
		print $content;
	}
	close $image;
}

##
# Prints content of given css file.
#
# Attributes:
#   css - css name
sub print_css() {
	my $css = shift;
	my $path = "styles/$css.css";
	die "Css $path does not exist."
		unless -e $path;
	print $cgi->header("text/css");
	open my $css_file, '<', $path
		or die "Could not open css file $path", $?;
	while (my $line = <$css_file>) {
		print $line
	}
	close $css_file;
}

##
# Fetches input parameters.
# Takes default values as attributes.
#
# Attributes:
#   valid_size_ptr - Array ref with valid sizes
#   valid_ext_ptr  - Array ref with valid file extensions
#   size - Image size
#   ext  - File extension
#   icon - Image icon
#   css  - Css file
# 
# Returns:
#   @params = ($size, $ext, $icon, $css)
sub fetch_input() {
	my ($valid_size_ptr, $valid_ext_ptr, $size, $ext, $icon, $css) = @_;

	if (defined $state{'size'}) {
		my $s = $state{'size'};
		die "Invalid size parameter"
			unless grep /^$s$/, @$valid_size_ptr;
		$size = $s;
	}

	if (defined $state{'ext'}) {
		my $e = $state{'ext'};
		die "Invalid extension parameter"
			unless grep /^$e$/, @$valid_ext_ptr;

		$ext = $e;
	}

	if (defined $state{'icon'} or defined $state{'i'}) {
		my $i = $state{'icon'} ? $state{'icon'} :  $state{'i'};
		$icon = $i if &valid_filename($i);
	}

	if (defined $state{'css'}) {
		my $c = $state{'css'};
		$css = $c if &valid_filename($c);
	}

	return ($size, $ext, $icon, $css);
}

##
# Validates filename
#
# Attributes:
#   name - filename
sub valid_filename($) {
	my $name = shift;
	my @valid = ('a'..'z', 0..9, '_');

	die "Illigal character, . in icon name."
		if $name =~ /\./;

	foreach my $char (split '', $name) {
		die "Illigal character, $char in icon name."
			unless grep /^$char$/, @valid;
	}
	1;
}

1;
