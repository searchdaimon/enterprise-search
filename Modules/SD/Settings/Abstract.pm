package SD::Settings::Abstract;
use strict;
use warnings;
use Readonly;
use SD::Sql::ConnSimple qw(sql_setup get_dbh);
use Digest::MD5 qw(md5_hex);
use JSON::XS;
use Carp;

Readonly::Scalar our $INFO_FILE => "export.info";
Readonly::Scalar our $CONN_DIR  => "$ENV{BOITHOHOME}/crawlers/";
Readonly::Scalar our $SD_RESERVED_CONN => 20_000;
#Readonly::Scalar our $CONN_DIR  => "/tmp/crawlers/";
Readonly::Scalar my $VERSION    => 1;

sub new {
	my ($class, $param) = (shift, shift);
	croak "Connector dir '$CONN_DIR' is not a directory"
		unless -d $CONN_DIR;

	my $s = bless { 
		db_setup => $param->{db_info} || { sql_setup() },
		json => JSON::XS->new()->pretty(1),
		version => $VERSION,
		skip_validation => $param->{skip_validation},
	}, $class;
	$s->_conn_db();
	if ($s->can('_init')) {
		$s->_init(@_);
	}
	$s;
}

sub _conn_db {
	my $s = shift;
	$s->{dbh} = get_dbh(%{$s->{db_setup}});
	$s;
}

sub file_md5 {
	md5_hex(shift->slurp(shift));
}

sub slurp {
	my ($s, $file) = @_;
	local $/ = undef;
	open my $fh, "<", $file
		or croak "file open: $!";
	return <$fh>;
}


1;
