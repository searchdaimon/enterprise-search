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
	$yum = Boitho::YumWrapper->new($CONFIG->{'rpm_upload_folder'}, $CONFIG->{'yum_wrapper_path'});
	return $self;
}


sub show {
	my ($self, $vars, $show_available) = @_;

	my @uploaded = $yum->list_rpm_dir();
	$vars->{'uploaded_packages'} = \@uploaded;
	
	if ($show_available) {
		$yum->clean();
		my @updates = $yum->check_update();
		$vars->{'available_packages'} = \@updates;
	}
	
	return ($vars, PACKAGES_TPL);
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
# Runs yum update and shows package page with results from execution.
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
		my ($success, @result) = $yum->localinstall($package);
		@output = (@output, @result);

		if ($success) {
			#TODO: Delete rpm file.
		}
		
	}
	$vars->{'install_result'} = \@output;
	
	return $self->show($vars);
}

1;
