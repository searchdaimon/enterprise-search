package commonDemon;
require Exporter;
@commonDemon::ISA = qw(Exporter);
@commonDemon::EXPORT = qw();
@commonDemon::EXPORT_OK = qw(daemonis signal_handler_inaliser signal_handler opt_handler);

###############################################################################################################
# become a daemon :-)																  #
###############################################################################################################
sub daemonis {
	my($logOUT,$logError,$programnavn) = @_;

	my($output);
	my($pid);
#set the working directory correctly

	print "Prover å bli en demon\n";
	$output = `pwd 2>&1`;
	$output =~ s/\/perl$//g;
	chomp($output);

	$path_to_main_dir = $output;
	
	#our($data_dir) = 	$path_to_main_dir . '/data';
	#our($tmpfs_dir) = 	$path_to_main_dir . '/tmpfs';
	#our($var_dir) = 		$path_to_main_dir . '/var';
	
	print "Setter hoved direktory til $path_to_main_dir\n";
	
	print "Forker\n";
	$pid = fork;
	exit if $pid;
	die "Couldn't fork: $!" unless defined($pid);

	use POSIX;
	POSIX::setsid() or die "Can't start a new session: $!";
	
	#redirect the file handles.
	#should perhaps redirect the file handle to /dev/null instead, since we risk missing error messages now

	if ($logOUT) {
		print "Logger til OUT til /tmp/" .$programnavn. "OUT.log\n";
		open STDOUT, ">>/tmp/" .$programnavn. "OUT.log" or die "Can't write to OUT log: $!";
		
	}
	else {
		open STDOUT, '>/dev/null' or die "Can't write to /dev/null: $!";
	}
	
	if ($logError) {
		print "Logger til Error til /tmp/" .$programnavn. "Err.log\n";
		open STDERR, ">>/tmp/" .$programnavn. "ERR.log" or die "Can't write to error log: $!";
	}
	else {
		open STDERR, '>/dev/null' or die "Can't write to /dev/null: $!";
	}


	open STDIN, '/dev/null'   or die "Can't read /dev/null: $!";
	
	
	
	signal_handler_inaliser();
	
}
###############################################################################################################
###############################################################################################################
sub signal_handler_inaliser {
	#install a new signal handler, so that if we die the data is saved
	#$SIG{INT} = \&signal_handler;
	$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
}
###############################################################################################################
###############################################################################################################
sub signal_handler {	
	if (not $killet) {
		$time_to_die = 1;
		$killet = 1;
		print "\n\aOk, begynner og avslutte\n\n";
	}
	else {
		die("Motok killsignal nr to, dør");
	}
}
###############################################################################################################
sub hendelselog {
	my($hvem,$hva) = @_;
	
	#print out if we are in verbose mode
	if ($GLOBAL_verbos) {
		print "$hvem: $hva\n";
	}
}
###############################################################################################################
# parse and handle arguments that come in via the command line
###############################################################################################################
sub opt_handler {
	my($egene_argumeneter,$help) = @_;
	my(%opts) = '';
	my($argumenter);
	
	#valid arguments
	# t = use tmpfs space
	$argumenter = 'vh';
	
	$argumenter .= $egene_argumeneter; 
	
	use Getopt::Std;
	
	getopts($argumenter, \%opts);
	

	#verbose mode? if so we print out lots of data from the event log
	if ($opts{v}) {
		$GLOBAL_verbos = 1;
	}
	if ($opts{h}) {
		print $help;
		
		print qq{
		
Generell bruk:
			
	-v verbos mode, viser debugingsinfor
		};
		print "\n"; # tidy termination so that we do not get the terminal prompt in the wrong place
		exit;
	}

	return %opts;
}
