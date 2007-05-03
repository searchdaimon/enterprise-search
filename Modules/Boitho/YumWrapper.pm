# Class: Boitho::YumWrapper
# Wrapper to execute some yum actions
package Boitho::YumWrapper;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Glob qw(bsd_glob);
use File::Basename qw(basename);

use constant USE_DEBUG_DATA => 0;

my $rpm_dir;
my $wrapper_path;

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    # init
    ($rpm_dir, $wrapper_path) = @_;
    croak "Path \"$rpm_dir\" to RPM directory you provided is not a directory."
	unless -d $rpm_dir;

    croak "Wrapper path \"$wrapper_path\" provided is not executable"
	unless -x $wrapper_path;

    return $self;
}

##
# Lists available updates.
#
# Returns:
#   List with hashrefs, format {'name' => 'Name', 'version' => 'Version', 'release' => 'Release'}
sub check_update {
    my $self = shift;

    my ($succ, @input);
    
    if (USE_DEBUG_DATA) {
	$succ = 1;
	@input = qq(Loading "installonlyn" plugin
Setting up repositories
Reading repository metadata in from local files

audit-libs.x86_64                        1.4.2-5.fc6            updates
audit-libs.i386                          1.4.2-5.fc6            updates
audit-libs-python.x86_64                 1.4.2-5.fc6            updates
boitho-crawlManager.i386                 1.3.3-1                boitho-released
boitho-searchdbb.i386                    5.4.11-1               boitho-released
	);
	@input = split "\n", $input[0];
	carp Dumper(\@input);
    }
    else {
	($succ, @input) = $self->_exec_action("check-update");
    }

    unless ($succ) { 
	return ($succ, @input);
    }

    my @packages;
    foreach my $in (@input) {
	chomp $in;
	if ($in =~ m/^(\S+) \s\s+ (\S+) \s\s+ (\S+)/xs) {
	    push @packages, {
		'name' => $1,
		'version' => $2,
		'release' => $3,
	    };
	}
    }

    return ($succ, @packages);
}

##
# Updates every currently installed package on local machine.
sub update {
    my $self = shift;
    return $self->_exec_action("update");
}

##
# Cleans updates-list cached on local machine.
sub clean {
    my $self = shift;
    return $self->_exec_action("clean");
}

##
# Installs given rpm package.
#
# Parameter:
#   package - Software package.
sub install {
    my ($self, $package) = @_;
    return $self->_exec_action("install", "\Q$package\E");

}

##
# Lists rpm files in $rpm_dir.
sub list_rpm_dir {
    my @files = bsd_glob("$rpm_dir/*rpm");
    for my $f (@files) { 
	$f = basename($f);
    }

    return @files;
}


# Group: Private methods

##
 # Execute with wrapper.
 #
 # Attributes:
 #	service   - Service name
 #	parameter - Parameter to service. WARN: It won't be escaped.
 #
 # Returns:
 #      status - True if wrapper exited with success, false if not.
 #	@output - List with output.
sub _exec_action {
    my ($self, $service, $parameter) = @_;
    $parameter = "" unless defined $parameter;
    my $exec = $wrapper_path . " $service $parameter|";

    open my $wraph, $exec
	or croak "Unable to execute yum-wrapper, $?";

    my @output = <$wraph>;
    my $status = 1;

    close $wraph or $status = 0;
    return ($status, @output);
}

1;
