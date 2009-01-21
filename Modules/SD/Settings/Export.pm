package SD::Settings::Export;
BEGIN { unshift @INC, $ENV{BOITHOHOME} . "/Modules" }
use strict;
use warnings;
use Carp;
use MIME::Base64;
use File::Temp qw(tempdir tempfile);
use File::Basename qw(basename);
use File::Path qw(rmtree);

use Readonly;
use Archive::Tar;
use Data::Dumper;

use SD::Settings::Abstract;
our @ISA = qw(SD::Settings::Abstract);

use SD::Sql::ConnSimple qw(sql_fetch_results);

Readonly::Scalar my $TBL_CONN => "connectors";
Readonly::Array my @EXPORT_TABLES => qw(
	activeUsers collectionAuth config connectors
	connectorsFor foreignUserSystem param scanResults
	shareGroups shareParam shareResults shareUsers
	shares system systemMapping systemParam
	systemParamValue);

Readonly::Hash my %CONN_IGNORE_FILES => map { $_ => 1 } qw(CVS .svn);

sub _init {
	my $s = shift;
	$s->{sql_files} = [];
	$s->{conn_files} = [];
	$s->{tmpdir} = tempdir();
}

##
# Returns path to gzip archive containing
# exported data.
sub export {
	my $s = shift;
	# Connectors
	eval {
		my @conn = $s->_sql_custom_conn();
		$s->_save_connectors(@conn);
	};
	if ($@) {
		die "Unable to export custom connectors: ", $@;
	}

	# SQL tables
	eval {
		$s->_save_table($_) for @EXPORT_TABLES;
	};
	if ($@) {
		die "Unable to export SQL tables: ", $@;
	}

	$s->_save_info();
	my $export_file = $s->_tar_export();
	
	eval { rmtree($s->{tmpdir}) };
	if ($@) { warn "del temp dir: ", $@ } #ignore
	
	return $export_file;
}



sub _save_info {
	my $s = shift;
	my %sql_files = map { $_ => $s->file_md5("$s->{tmpdir}/$_") } 
		@{$s->{sql_files}};
	my %conn_files = map { $_ => $s->file_md5("$s->{tmpdir}/$_") } 
		@{$s->{conn_files}};

	my $nfo = $SD::Settings::Abstract::INFO_FILE;
	return $s->_data_to_file($nfo, $s->{json}->encode({
		version    => $s->{version},
		sql_files  => \%sql_files,
		conn_files => \%conn_files,
	}));
}

sub _save_connectors {
	my ($s, @conn) = @_;

	for my $c (@conn) {
		my $conn_file = "conn_$c->{id}";
		$s->_data_to_file($conn_file, $s->{json}->encode({
			info => {
				id => $c->{id},
				name => $c->{name},
			},
			content => encode_base64($s->_conn_to_tar($c->{name})),
		}));
		push @{$s->{conn_files}}, $conn_file;
	}
	$s;
}

sub _save_table {
	my ($s, $tbl) = @_;

	my $cmd = "mysqldump ".
                "--complete-insert " .
		"--no-create-info ".
		"--compact " .
		"--password=\Q$s->{db_setup}{Password}\E ".
		"--user=\Q$s->{db_setup}{user}\E ".
		"\Q$s->{db_setup}{database}\E " . $tbl;
	
	open my $dumph, "$cmd |"
		or die "Could not open execute mysqldump $!\n";
	my $output = encode_base64(join "\n", <$dumph>);
	close $dumph || die "mysqldump exited with error";
	my $file = "sql_$tbl";
	$s->_data_to_file($file, $output);
	push @{$s->{sql_files}}, $file;

	$s;
}

sub _conn_to_tar {
	my ($s, $name) = @_;
	my $tar = Archive::Tar->new;
	
	my $path = "$SD::Settings::Abstract::CONN_DIR$name";
	croak "Path '$path' for connector '$name' is not a directory"
		unless -d $path;
	my @files = grep { !$CONN_IGNORE_FILES{basename($_)} }
		    glob "$SD::Settings::Abstract::CONN_DIR/$name/*";
	
	$_->prefix($name) for $tar->add_files(@files);

	return $tar->write();
}

sub _tar_export {
	my $s = shift;
	my $tar = Archive::Tar->new;
	$tar->add_files(glob("$s->{tmpdir}/*"));
	my (undef, $gz_file) = tempfile(SUFFIX => ".tar.gz");
	$tar->write($gz_file, 1);
	return $gz_file;
}

sub _sql_custom_conn {
	my $s = shift;
	my $q = "SELECT id, name 
		 FROM $TBL_CONN
		 WHERE id > $SD::Settings::Abstract::SD_RESERVED_CONN
		 AND extension != 0";
	return sql_fetch_results($s->{dbh}, $q);
}

sub _data_to_file {
	my ($s, $file, $str) = @_;
	open my $fh, ">", "$s->{tmpdir}/$file"
		or die "create file: ", $!;
	print {$fh} $str;
	close $fh;
	$s;
}

#my $export = SD::Settings::Export->new;
#print $export->export();

1;
