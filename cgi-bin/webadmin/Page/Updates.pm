package Page::Updates;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Path;
use Page::Abstract;
use Common::TplCheckList;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::YumWrapper;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_LIST => "updates_list.html";
use constant TPL_UPDATES => "updates_update.html";
use constant TPL_ADVANCED => "updates_advanced.html";

my $yum;

sub _init {
	my ($self) = @_;
	$yum = Boitho::YumWrapper->new(
            $CONFIG->{'rpm_upload_folder'}, 
            $CONFIG->{'yum_wrapper_path'});

	return $self;
}

sub show_list {
    my ($s, $vars, $show_list) = @_;
    $s->_show_installed($vars)
        if $show_list;
    return TPL_LIST;
}

sub show_updates {
    my ($s, $vars, $show_avail) = @_;

    $s->_show_available($vars)
        if $show_avail;
    return TPL_UPDATES;
}

sub show_advanced {
    my ($s, $vars) = @_;
    $vars->{new_pkgs} = [ $yum->list_rpm_dir() ];
    return TPL_ADVANCED;
}

sub upload_pkg {
    my ($s, $vars, $file) = @_;


    # Some browsers send full path. Use only the filename.
    $file =~ s/.*[\/\\](.*)/$1/;
    
    
    # Strip chrs that the wrapper doesn't like
    my $filename = $s->strip_filename($file);

    if (!$filename or $filename !~ /\.rpm$/) {
        $vars->{'error_not_rpm'} = $filename;
        return $s->show_advanced($vars);
    }

    # Add number suffix if file already exists.
    my $folder = $CONFIG->{'rpm_upload_folder'};
    my $filenum = q{};
    my $filepath;
    while (1) {
        $filepath = "$folder/$filenum$filename";
        $filenum = $filenum ? ++$filenum : 1;
        last unless -e $filepath;
    }

    # Write the file.
    unless (-e $folder) {
        carp "Upload folder ($folder) does not exist, attempting to create it.";
        eval { mkpath($folder) };
        croak "Unable to create folder $folder, $@\n" if $@;
    }

    open my $rpm_file, ">", $filepath
        or croak "RPM Upload: Unable to open $filepath for writing";
    
    my $buffer;
    while (read($file, $buffer, 1024)) {
        print {$rpm_file} $buffer;
    }

    $vars->{'succs_upload'} = $filename;

    return $s->show_advanced($vars);
}

##
# Runs yum update and shows results from execution.
sub update_packages {
    my ($s, $vars) = @_;
    my ($succs, @output) = $yum->update();
    
    $vars->{updt_succs} = $succs;
    $vars->{updt_result} = \@output;

    return $s->show_updates();
}

##
# Install uploaded packages. Show results of install.
sub install_uploaded {
    my ($s, $vars) = @_;

    my @new_pkgs = $yum->list_rpm_dir();

    my @output;
    my $inst_results = Common::TplCheckList->new();

    foreach my $pkg (@new_pkgs) {
        my ($succs, @inst_output) = $yum->install($pkg);
        @output = (@output, @inst_output);

        $inst_results->add(($succs ? "$pkg is installed"
                    : "$pkg could not be installed"),
                ($succs ? $inst_results->SUCCESS 
                 : $inst_results->FAILED));
        $s->del_pkg({}, $pkg) if $succs;
    }
    $vars->{'install_output'} = \@output;
    $vars->{'install_results'} = $inst_results;


    return $s->show_advanced($vars);
}

##
# Delete uploaded file
sub del_pkg {
    my ($self, $vars, $filename) = @_;

    croak "no filname given"
        unless $filename;

    # validate
    unless ($filename =~ /rpm$/i) {
        $vars->{remove_error} = $filename;
        return $self->show_advanced($vars);
    }

    $filename = $self->strip_filename($filename);

    # Delete file.
    my $rpm_dir = $CONFIG->{'rpm_upload_folder'};
    my $path = "$rpm_dir/$filename";
    warn "$path does not exist" unless -e $path;

    my $result = (-e $path and unlink $path)
        ? 'remove_succs' : 'remove_error';
    $vars->{$result} = $filename;

    return $self->show_advanced($vars);
}

##
# Strip characters that the yumwrapper doesn't like.
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
		$allowed = 1 if ($char =~ /([a-z]|[A-Z]|[0-9])+/);
		$allowed = 1 if $char eq '.' or $char eq '-';
		next unless $allowed;
		
		$newname .= $char;
	}
	
	$newname = "unknown.rpm" if $newname eq ".rpm";
	return $newname;
}

sub _show_installed {
    my ($self, $vars) = @_;
    my ($succs, @input) = $yum->list();
    unless ($succs) {
        $vars->{listing_error} 
            = join '<br />', @input;
        return;
    }
    my @pkgs = grep { $_->{name} =~ /^boitho/ } @input;
    $vars->{installed_pkgs} = \@pkgs;
    1;
}

sub _show_available {
    my ($s, $vars) = @_;
    $yum->clean();
    my ($succs, @output) = $yum->check_update();

    unless ($succs) {
        my $errmsg = join '<br />', @output
            if @output;
        $vars->{error_msg} = $errmsg || q{};
    }
    else {
        $vars->{'avail_pkgs'} = \@output;
    }
}



1;
