# Class: Boitho::YumWrapper
# Wrapper to execute some yum actions
package Boitho::YumWrapper;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Glob qw(bsd_glob);
use File::Basename qw(basename);
use File::Path qw(mkpath);

use constant USE_DEBUG_DATA => 1;

my $rpm_dir;
my $wrapper_path;

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;

    # init
    ($rpm_dir, $wrapper_path) = @_;

    if (defined $rpm_dir) {
        if (-e $rpm_dir and !(-d $rpm_dir)) {
            croak "rpm dir $rpm_dir exists, but is not a directory.
                It must be a directory.";
        }

        unless (-d $rpm_dir) {
            carp "rpm folder $rpm_dir does not exist. Making it.";

            eval    { mkpath($rpm_dir) };
            if ($@) { croak "Unable to create rpm dir $rpm_dir: $@"	}
        }
    }

    croak "Wrapper path \"$wrapper_path\" provided is not executable"
	unless -x $wrapper_path;

    return $self;
}

##
# List installed packages.
sub list {
    my $self = shift;
    my ($succs, @input) = $self->_exec_action("list");
    return ($succs, @input)
        unless $succs;
    return ($succs, $self->_parse_pkg_list(@input));
}

##
# Lists available updates.
#
# Returns:
#   List with hashrefs, format {'name' => 'Name', 'version' => 'Version', 'release' => 'Release'}
sub check_update {
    my $self = shift;
    my ($succ, @input) = $self->_exec_action("check-update");
    return ($succ, @input)
        unless $succ;
    return ($succ, $self->_parse_pkg_list(@input));
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
    croak "rpm dir not defined" unless $rpm_dir;
    my @files = bsd_glob("$rpm_dir/*rpm");
    for my $f (@files) { 
	$f = basename($f);
    }

    return @files;
}


# Group: Private methods

sub _parse_pkg_list {
    my $self = shift;
    my @input = @_;

    my @pkgs;
    foreach my $in (@input) {
        chomp $in;
        if ($in =~ m/^(\S+) \s\s+ (\S+) \s\s+ (\S+)/xs) {
            push @pkgs, { 
                'name' => $1, 
                'version' => $2, 
                'release' => $3, 
            }
        }
    }
    return @pkgs;
}

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
