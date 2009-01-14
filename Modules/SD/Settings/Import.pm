package SD::Settings::Import;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Archive::Tar;
use File::Temp qw(tempdir);
use File::Basename qw(basename);
use MIME::Base64;
use IO::String;
use File::Path qw(rmtree);
use File::Copy::Recursive qw(dirmove);

BEGIN { unshift @INC, "$ENV{BOITHOHOME}/Modules" }
use SD::Settings::Abstract;
use SD::Sql::ConnSimple qw(sql_fetch_results);
our @ISA = qw(SD::Settings::Abstract);

sub _init {
	my $s = shift;
	$s->{tmpdir} = tempdir();
	$s->{conn_rollback_dir} = "/home/dagurval/tmp/lol";# tempdir();
}


sub do_import { # 'sub import' collides with package routine
	my ($s, $file_path) = @_;
	my $tar = Archive::Tar->new($file_path, 1)
		or croak "Unable to open file";
	
	$s->_extract($tar);
	$s->_read_info();
	$s->_validate()
	 	unless $s->{skip_validation};

	my $old_autocommit_val = $s->{dbh}->{AutoCommit};
	$s->{dbh}->{AutoCommit} = 0;
	croak "Unable to disable mysql AutoCommit"
		if $s->{dbh}->{AutoCommit};

	eval {
		$s->_import_sql();
		$s->_import_connectors();
	};
	if (my $err = $@) {
		$s->{dbh}->rollback();
		$s->{dbh}->{AutoCommit} = $old_autocommit_val;
		$s->_rollback_conn();
		$s->_rm_tmp();
		croak $err;
	}
	$s->{dbh}->commit();
	$s->_rm_tmp();
	1;
}

sub _rm_tmp {
	my $s = shift;
	eval { rmtree($s->{tmpdir}, $s->{conn_rollback_dir}) };
	if ($@) { carp "del temp dir: ", $@ } #ignore
}

sub _import_sql_tbl {
	my ($s, $sql_file) = @_;

	$sql_file =~ /^sql_(.*)/;
	my $tbl_name = $1;
	my $sql_import = decode_base64(
			$s->slurp("$s->{tmpdir}/$sql_file"));
	$s->{dbh}->do("DELETE FROM $tbl_name") # transaction did not work with TRUNCATE 
		or croak "SQL import error: ", $s->{dbh}->errstr;

	for (split "\n", $sql_import) {
		next unless $_;
		$s->{dbh}->do($_)
			or croak "SQL import error: ", $s->{dbh}->errstr;
	}
}

sub _import_sql {
	my $s = shift;
	for my $sql_file (keys %{$s->{info}{sql_files}}) {
		$s->_import_sql_tbl($sql_file);
	}
	$s;
}

sub _import_connectors {
	my $s = shift;
	my @files = keys %{$s->{info}->{conn_files}};
	
	$s->_move_existing_conn();

	for my $f (@files) {
		my $conn_data = $s->slurp("$s->{tmpdir}/$f");
		my $conn_ref = $s->{json}->decode($conn_data);
		$s->_import_conn(%{$conn_ref});
	}
}

sub _get_conn_extensions {
	my $s = shift;
	my $q = "SELECT name FROM connectors
		WHERE extension != 0
		AND id > $SD::Settings::Abstract::SD_RESERVED_CONN";

	return map { $_->{name} } (sql_fetch_results($s->{dbh}, $q));
}

sub _move_existing_conn {
	my $s = shift;
	my @moved;

	for my $conn ($s->_get_conn_extensions) {
		my $src_path = $SD::Settings::Abstract::CONN_DIR . "$conn";
		my $dst_path = $s->{conn_rollback_dir} . "/$conn";
		next unless -d $src_path;
		
		dirmove($src_path, $dst_path)
			? push @moved, $conn
			: warn "moving conn to rollback dir: ", $!;
	}

	$s->{conn_moved} = \@moved;
	$s;
}

sub _rollback_conn {
	my $s = shift;
	for my $conn (@{$s->{conn_moved}}) {
		my $src_path = $s->{conn_rollback_dir} . "/$conn";
		my $dst_path = $SD::Settings::Abstract::CONN_DIR . $conn;
		dirmove($src_path, $dst_path)
			or warn "conn rollback move: ", $!;
	}
	$s;
}

sub _import_conn  {
	my ($s, %conn)  = @_;
	

	my $tar_h = IO::String->new(decode_base64($conn{content}));
	my $tar = Archive::Tar->new($tar_h)
		|| croak "unable to read connector $conn{name} contents.";
	$tar->setcwd($SD::Settings::Abstract::CONN_DIR);
	$tar->extract();
}

sub _extract {
	my ($s, $tar) = @_;
	for my $file ($tar->list_files) {
		my $base_file = basename($file);
		$tar->extract_file($file, "$s->{tmpdir}/$base_file");
	}
	$s;
}

sub _read_info {
	my $s = shift;
	my $nfo_file = $SD::Settings::Abstract::INFO_FILE;

	$s->{info} = $s->{json}->decode(
		$s->slurp("$s->{tmpdir}/$nfo_file"));
	$s;

}

sub _validate_filelist {
	my ($s, %files) = @_;
	for my $file (keys %files) {
		my $path = $s->{tmpdir} . "/$file";
		my $md5 = $s->file_md5($path);
		if ($md5 ne $files{$file}) {
			croak "Invalid MD5sum for $file. " 
			    . "Was $md5, expected $files{$file}";
		}
	}
	$s;
}

sub _validate {
	my $s = shift;
	my %nfo = %{$s->{info}};
	croak "Export does not have version info. Corrupt file?"
		unless $nfo{version};

	for my $attr (keys %nfo) {
		if ($attr eq "sql_files"
		 || $attr eq "conn_files") {
			$s->_validate_filelist(%{$nfo{$attr}});
		}
		elsif ($attr eq "version") {
			next if $nfo{version} == $s->{version};

			my $ver_info = $nfo{version} < $s->{version}
				? "Export from older version."
				: "Export from newer version.";
			croak $ver_info, 
				" Export version: $nfo{version}",
				" Required version: $s->{version}";

		}
		else { 
			warn "Ignoring unknown export attribute: $attr";
		}
	}	
}

	

SD::Settings::Import->new->do_import(shift);
1;
