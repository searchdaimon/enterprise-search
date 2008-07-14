package Page::Logs;
use strict;
use warnings;
use Carp;
use File::stat;
use Data::Dumper;
use Sql::Search_logg;
use File::Temp qw(tempfile);
use config qw($CONFIG);
use Page::Abstract;

my %CONFIG = %$CONFIG;
our @ISA = qw(Page::Abstract);

use constant TPL_LOGFILE => 'logs_main.html';
use constant DEFAULT_LOG_LINES => 250;
use constant MAX_LOG_LINES => 500;



sub _init {
    my $self = shift;
    my $dbh = $self->{dbh};
    $self->{sqlSearch} = Sql::Search_logg->new($dbh);

    my $lines = $self->{state}{lines};
    $lines = DEFAULT_LOG_LINES unless $lines;
    $lines = MAX_LOG_LINES if $lines > MAX_LOG_LINES;
    $self->{lines} = $lines;
}


sub show_logfiles {
    my ($self, $vars) = @_;
    $vars->{loglist} = [$self->_get_logs()];
    $vars->{lines} = $self->{lines};
    return TPL_LOGFILE;
}

sub download {
	my $self = shift if ref $_[0];
	my $filename = shift;

	return unless $CONFIG{logfiles}->{$filename}; # valid filename

        my $path = $CONFIG{log_path} . "/" . $filename;
	open my $fh, '<', $path
		or croak "couldn't open $path: ", $!;

	print while <$fh>;
#	Avoid 'print <$file>;', it reads the entire file into memory.

	1;
}
sub downl_all_zip {
    my $s = shift if ref $_[0];
   
    chdir $CONFIG{log_path};

    my $logs_str;
    for my $file (keys %{$CONFIG{logfiles}}) {
        next unless -e $file;
        $logs_str .= "\Q$file\E ";
    }

    # create zip for all logs.
    my (undef, $filename) = tempfile(SUFFIX => ".zip", OPEN => 0);
    my $exec_str = "zip -1 \Q$filename\E $logs_str 2>&1 |";
    open my $ziph, $exec_str
        or croak "Unable to run zip", $!;

    my $output = join q{}, <$ziph>;
    warn $output if $output;
    close $ziph or croak "error running zip: $output";

    # read the zip.
    open my $fh, "<", $filename
        or croak "Unable to open '$filename'" , $!;
    
    print while <$fh>;
    close $fh;
    unlink $filename;
    1;
}




sub show_search_log {
	my ($self, $vars) = (@_);
	my $sqlSearch = $self->{'sqlSearch'};
	
	$vars->{'search'} = $sqlSearch->get_log;

	return ($vars, 'logs_search.html');
}

sub show_logfile_content {
	my ($self, $vars, $filename, $lines) = @_;
        croak "Unknown filename $filename"
            unless grep { $filename =~ /^$_$/ } keys %{$CONFIG->{logfiles}};
	my @content = $self->_read_log($filename, $self->{lines});
		
	my %logfile = (
		'filename' => $filename,
		'content' => \@content,
	);
	$vars->{'logfile'} = \%logfile;
        return $self->show_logfiles($vars);
}

sub _get_size {
	my $self = shift;
	my $filename = shift;
	return 0 unless(-e "$CONFIG{'log_path'}/$filename");

	my $size = stat("$CONFIG{'log_path'}/$filename")->size;
	
	if ($size < 1024) {
		return (int($size), '');
	}
	elsif (($size / 1024) < 1024) {
		return (int($size / 1024), "K");
	}
	else  {
		return (int($size / 1024 / 1024), "M");
	}
}

sub _read_log {
	my $self = shift;
	my $filename = shift;
	my $lines = shift;
	
	unless ($lines =~ /^\d+$/) {
		carp "lines variable did not contain a number.";	
		return;
	}
	
	$lines = 500 if ($lines > 500);

	return ( ) 
		unless ($self->_get_size($filename))[0];

	my $command = "tail";
	if ($filename =~ /\.\d+$/) {
		# Ends with a number. Asuming it's reversed.
		$command = "head";
	}
	open my $tail, "$command -n $lines $CONFIG{'log_path'}/\Q$filename\E |"
		or croak "coulnt execute tail: $!";
	my @content = map { chomp; $_ } <$tail>;

	@content = reverse @content if ($command eq "head");

	return @content;
}

sub _get_logs {
	my $self = shift;
	my @result = ( );
	my %logfiles = %{$CONFIG{'logfiles'}};
	while (my ($filename, $description) = each %logfiles) {
		my @size = $self->_get_size($filename);
		push @result, {
			'filename' => $filename,
			'description' => $description,
			'size' => \@size
		};

	}	

        @result = sort { $a->{description} cmp $b->{description} } @result;

	return @result;
}

1;
