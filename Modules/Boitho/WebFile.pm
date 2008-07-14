# Namespace: Boitho::WebFile
# Fetch images/css/js files in cgi-bin dir.
#
# Asumes:
#   - css files are in dir styles
#   - images are in dir icon/size where size is image size (for example 32x32)
#   - javascripts are in the dir javascript
# These can be changed with set_css_dir(), set_img_dir() and set_js_dir()
package Boitho::WebFile;
use strict;
use warnings;
use Carp;
use CGI;
use CGI::State;
use Exporter;
our @EXPORT_OK = qw(get set_css_dir set_img_dir set_js_dir);

my $cgi = CGI->new;
my $state_ptr = CGI::State->state($cgi);
my %state = %$state_ptr;

my $css_dir = "styles";
my $img_dir = "icon";
my $js_dir  = "javascript";
my $swf_dir = "swf";

my @default_valid_size = qw(16x16 22x22 32x32 48x48 64x64 128x128 other);
my @default_valid_ext  = qw(png jpg jpeg swf);


##
# Get file
#
# Attributes:
#   use_ptr - What we can fetch. Can contain 'css', 'javascript' and 'image'.
sub get {
    my ($use_ptr, $valid_size_ptr, $valid_ext_ptr) = @_;
    
    # Initialize
    my ($use_images, $use_css, $use_js, $use_swf);
    foreach my $use (@$use_ptr) {
	$use_images = 1 if $use eq 'image';
	$use_css    = 1 if $use eq 'css';
	$use_js     = 1 if $use eq 'javascript';
        $use_swf    = 1 if $use eq 'swf';
    }

    my @valid_size = (defined $valid_size_ptr) ? @$valid_size_ptr : @default_valid_size;
    my @valid_ext  = (defined $valid_ext_ptr ) ? @$valid_ext_ptr  : @default_valid_ext;

	# default values
    my $size = '32x32' if grep /^32x32$/, @valid_size;
    my $ext  = 'png'   if grep /^png$/, @valid_ext;
    my ($icon, $css, $js, $swf);

    # Start

    ($size, $ext, $icon, $css, $js, $swf) = fetch_input(\@valid_size, \@valid_ext, $size, $ext, $icon, $css, $js, $swf);

    if (defined $icon) {
	croak "Missing size, ext or icon" 
	    if grep { !defined $_ } $size, $ext;
	print_image($size, $ext, $icon) if $use_images;
    }
    elsif (defined $css) {
	print_css($css) if $use_css;
    }
    elsif (defined $js) {
	print_js($js) if $use_js;
    }
    elsif (defined $swf) {
        print_swf($swf) if $use_swf;
    }
    else {
	croak "Wrong parameters provided";
    }
}

# Group: Setters

sub set_css_dir { $css_dir = shift }
sub set_js_dir  { $js_dir  = shift }
sub set_img_dir { $img_dir = shift }



# Group: Private functions

##
# Prints content of given image
#
# Attributes:
#   size - image size
#   icon - image name
#   ext  - image extension
sub print_image() {
	my ($size, $ext, $icon) = @_;
	my $path = "$img_dir/$size/$icon.$ext";
	print $cgi->header(-type => "image/$ext", -expires => "+1h");
	print_file_contents($path, "image");
	1;
}

##
# Prints content of given css file.
#
# Attributes:
#   css - css name
sub print_css {
	my $css = shift;
	my $path = "$css_dir/$css.css";
	print $cgi->header("text/css");
	print_file_contents($path, "css");
	1;
}

##
# Prints content of given javascript file.
#
# Attributes:
#   js - name of javascript.
sub print_js {
    my $js = shift;
    my $path = "$js_dir/$js.js";
    print $cgi->header("application/x-javascript");
    print_file_contents($path, "js");
}

sub print_swf {
    my $s = shift;
    print $cgi->header("application/x-shockwave-flash");
    print_file_contents("$swf_dir/$s.swf");
}

##
# Print all contents in a file.
#
# Attributes:
#  file_path - Path to read contents from.
#  file_type - File type we're reading (for use in error messages)
sub print_file_contents {
    my $file_path = shift;
    my $file_type = shift;


    die "$file_type file $file_path does not exist."
		unless -e $file_path;

    open my $fh, "<", $file_path
	or die "Could not open $file_type file $file_path", $?;

    while (my $line = <$fh>) { # print <$fh>; is faster, but may consume too much memory on large files.
	print $line;
    } 
    1;
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
sub fetch_input {
	my ($valid_size_ptr, $valid_ext_ptr, $size, $ext, $icon, $css, $js, $swf) = @_;

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
		$icon = $i if valid_filename($i);
	}

	if (defined $state{'css'}) {
		my $c = $state{'css'};
		$css = $c if valid_filename($c);
	}

	if (defined $state{'js'}) {
	    my $j = $state{'js'};   
	    $js = $j if valid_filename($j);
	}
        if (defined $state{swf}) {
            $swf = $state{swf}
                if valid_filename($state{swf});
        }
            
	return ($size, $ext, $icon, $css, $js, $swf);
}

##
# Validates filename
#
# Attributes:
#   name - filename
my %valid = map { $_ => 1 } ('a'..'z', 0..9, '_', '.');
sub valid_filename($) {
	my $name = shift;

	die "Illigal character, . in icon name."
		if $name =~ /\.\./;

	foreach my $char (split '', $name) {
		die "Illigal character, $char in icon name."
			unless $valid{lc $char};
	}
	1;
}

1;
