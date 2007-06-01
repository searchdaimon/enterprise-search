##
# Handle program crashes
package Page::System::Crashes;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Glob qw(bsd_glob GLOB_ERROR);
use File::stat qw(stat);
use File::Path qw(mkpath);
use File::Basename qw(basename);
use File::Temp qw(tempfile);

use HTTP::Request;
use LWP::UserAgent;

use Page::Abstract;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_CRASH_OVERVIEW => "system_crash_overview.html";
use constant TPL_CRASH_REPORT   => "system_crash_report.html";


sub _init {
	my $self = shift;
	# Make sure core dir exists.
	unless (-e $CONFIG->{'cores_path'}) {
		carp "Core directory ", $CONFIG->{'cores_path'}, 
			" does not exist. Attempting to create it.";
		eval { mkpath($CONFIG->{'cores_path'}) };
		if ($@) {
			carp "Failed creating core dir. ", $@;
		}
	}

	unless (-d $CONFIG->{'cores_path'}) {
		croak "Core directory", $CONFIG->{'cores_path'}, 
			" is not a directory, or does not exist.";
	}
}

sub show { return $_[0]->show_list(@_) }

sub show_list {
	my ($self, $vars) = @_;
	$vars->{'core_list'} = [$self->_get_core_list()];
	return ($vars, TPL_CRASH_OVERVIEW);
}

sub show_report {
	my ($self, $vars, $file) = @_;
	$vars->{'report'} = $self->_get_core_report($file);
	$vars->{'core'}   = $self->_get_core_info($CONFIG->{'cores_path'} . "/$file");
	return ($vars, TPL_CRASH_REPORT);
}

sub send_report {
	my ($self, $vars, $file, $time, $report) = @_;

	my $file_esc = CGI::escape($file);
	my $time_esc = CGI::escape($time);
	my $report_esc = CGI::escape($report);
	
	my $req = HTTP::Request->new(POST => $CONFIG->{'core_report_url'});
	$req->content_type('application/x-www-form-urlencoded');
	$req->content("file=$file_esc&time=$time_esc&report=$report");
	my $res = LWP::UserAgent->new->request($req);
	
	$vars->{'report_send_success'} = $res->is_success;
	unless ($res->is_success) {
		$vars->{'report_send_error'} = $res->status_line;
	}

	return $self->show_list($vars);
}

# Group: Private methods

sub send_report_with_POST {
	my ($self, $file, $time, $report) = @_;
	
}

##
# Crash-report for a given core file.
#
# Arguments:
#	file - Core filename
#
# Returns:
#	report - Generated report
sub _get_core_report {
	my ($self, $file) = @_;

	croak "Missing argument file"
		unless defined $file;
	
	my $file_path = $CONFIG->{'cores_path'} . "/$file";
	
	croak "Core file $file_path does not exist."
		unless -e $file_path;
	return $self->_get_gdb_backtrace($file_path);
	
}

##
# List of core files
#
# Returns:
#	core_list - List of core files (as hashref)
sub _get_core_list {
	my $self = shift;
	my @core_list;

	# get file list
	my @file_list = bsd_glob($CONFIG->{'cores_path'} . "/*");
	if (GLOB_ERROR) {
		croak "Glob error ", GLOB_ERROR;
	}
	
	# ..into a nice structure
	foreach my $file (@file_list) {
		
		# add time.
		my $info_ref = $self->_get_core_info($file);
		push @core_list, $info_ref;
	}
	
	return @core_list;
}

sub _get_core_info {
	my ($self, $file) = @_;

	my $stat = stat($file)
		or croak "No $file, $!";
		
	my $time = $stat->mtime;
	my $human_time = gmtime $time;
	
	return {
		'core_file' => basename($file),
		'time'      => $human_time, 
		};
}

##
# Backtrace for given core file.
# Helper method for <_get_core_report>
#
# Arguments:
#	file_path - Full path to core file.
#
# Returns:
#	backtrace - Core backtrace.
sub _get_gdb_backtrace {
	my ($self, $file_path) = @_;

	# Create a command file..
	my ($tmp_fh, $tmp_filename) = tempfile(CLEANUP => 1);
	print {$tmp_fh} $CONFIG->{'gdb_report_cmd'};
	close $tmp_fh;

	#croak `cat $tmp_filename`;
	
	my $exec = "gdb --core=\Q$file_path\E --batch --command=\Q$tmp_filename\E";
	open my $gdbh, "$exec |"
		or croak "Unable to execute gdb: ", $!;

		
	my @backtrace = <$gdbh>;
	close $gdbh;
	unlink $tmp_filename;
	return join '\n', @backtrace;
}

1;