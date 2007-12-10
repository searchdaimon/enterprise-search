# Class: Page::System::Packages
# Install / update software
package Page::System::Packages;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Path;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::YumWrapper;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

use constant PACKAGES_TPL => "system_upload.html";

my $yum;

sub _init {
	my ($self) = @_;
	$yum = Boitho::YumWrapper->new(
            $CONFIG->{'rpm_upload_folder'}, $CONFIG->{'yum_wrapper_path'});
	return $self;
}


sub show {
    my ($self, $vars, $show_available) = @_;

    my @uploaded = $yum->list_rpm_dir();
    $vars->{'uploaded_packages'} = \@uploaded;

    $self->_show_installed($vars);
    

    if ($show_available) {
        $yum->clean();
        my ($success, @output) = $yum->check_update();


        unless ($success) {
            # something went wrong. Show the user.
            my $errmsg;
            if (scalar @output) {
                $errmsg = "Error during update: " . join("<br />\n", @output);
            }
            else {
                $errmsg = "Unknown error during update. The error log should contain the details.";
            }
            $vars->{'available_packages_error'} = $errmsg;
        }

        else {
            # all good, show the output.
            $vars->{'available_packages'} = \@output;
        }
    }

    return ($vars, PACKAGES_TPL);
}

sub _show_installed {
    my ($self, $vars) = @_;
    my ($succs, @input) = $yum->list();
    unless ($succs) {
        $vars->{pkg_listing_error} 
            = "Unable to list installed packages:\n" . join "\n", @input;
        return;
    }
    my @pkgs = grep { $_->{release} eq 'boitho-released' } @input;
    $vars->{installed_pkgs} = \@pkgs;
    1;
}




sub upload_package {
	my ($self, $vars, $file) = @_;
	my $upload_folder = $CONFIG->{'rpm_upload_folder'};
	my $buffer;
	

	# Remove chars the wrapper won't like.
	my $filename = $self->strip_filename($file);	

	if ((not $filename) or (not $filename =~ m{\.rpm$}xs)) {
		# No rpm suffix
		$vars->{'error_no_rpm_suff'} = 1;
		if ($filename) {
			$vars->{'error_no_rpm_filename'} = $filename;
		}
		return $self->show($vars);
	}
	
	
	# Add number suffix if file already exists.
	my $filenum = q{};
	while (-e "$upload_folder/$filenum$filename") {
		if (not $filenum) {
			$filenum = 1;
		}
		else {
			$filenum++;
		}
	}
	my $filepath = "$upload_folder/$filenum$filename";

	# Start writing to file.
	unless (-e $upload_folder) {
		carp "Upload folder ($upload_folder) does not exist, attempting to create it.";
		eval { mkpath($upload_folder) };
		croak "Unable to create folder $upload_folder, $@\n" if $@;
		
	}

	open my $rpm_file, ">", $filepath
		or croak "RPM Upload: Unable to open $filepath for writing";
	while (read($file, $buffer, 1024)) {
		print {$rpm_file} $buffer;
	}

	$vars->{'succ_upload_file'} = $filename;

	#Show default page.
	return $self->show($vars);
}

##
# Strip mean (and not mean) characters from a given filename.
# Helper function for upload_package();
#
# Parameters:
#   filename - String with filename.
#
# Returns
#   filename - Stripped filename
sub strip_filename {
	my ($self, $filename) = @_;
	# Allowed chars: a-z, 0-9, -, .
	my $newname = q{};
	foreach my $char (split q{}, $filename) {
		my $allowed = 0;
		$allowed = 1 if ($char =~ /\w/); #isalnum
		$allowed = 1 if ($char eq '.' or $char eq '-');
		next unless $allowed;
		
		$newname .= $char;
	}
	
	$newname = "unknown.rpm" if $newname eq ".rpm";
	return $newname;
}

##
# Runs yum update and shows results from execution.
sub update_packages {
    my ($self, $vars) = @_;
    
    my ($status, @output) = $yum->update();
    
    $vars->{'update_result'} = \@output;

    return $self->show($vars);
}

##
# Install uploaded packages. Show results of install.
sub install_uploaded {
	my ($self, $vars) = @_;

	my @new_packages = $yum->list_rpm_dir();
	my @output;

	foreach my $package (@new_packages) {
		my ($success, @result) = $yum->install($package);
		@output = (@output, @result);

		if ($success) {
			my $rpm_dir = $CONFIG->{'rpm_upload_folder'};	
			#unlink "$rpm_dir/$package";
			
		}
		
	}
	$vars->{'install_result'} = \@output;
	
	return $self->show($vars);
}

##
# Delete uploaded file
sub remove_uploaded_package {
	my ($self, $vars, $filename) = @_;

	unless ($filename) {
		carp "remove_upload_package called without a filename";
		return;
	}
	# validate
	unless ($filename =~ /rpm$/) {
		$vars->{'remove_uploaded_succ'}    = 0;
		$vars->{'remove_uploaded_message'} = "File is not a rpm file. Can't delete'.";
		return $self->show($vars);
	}

	# if the file was uploaded through the webadmin,
	# it should pass the strip_filename function without a problem
	$filename = $self->strip_filename($filename);

	# Delete file.
	my $rpm_dir = $CONFIG->{'rpm_upload_folder'};
	my $path = "$rpm_dir/$filename";

	my $deleted_count;
	if (-e $path) {
		$deleted_count = unlink $path;
	}

	# Check for error.
	if ($deleted_count) {
		$vars->{'remove_uploaded_succ'}    = 1;
		$vars->{'remove_uploaded_message'} = "$filename has been deleted.";
	}	
	else  {
		$vars->{'remove_uploaded_succ'} = 0; 
		$vars->{'remove_uploaded_message'} = "Unable to delete file $filename";
	}

	return $self->show($vars);
}

1;
