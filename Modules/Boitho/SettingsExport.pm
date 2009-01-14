# Class: Boitho::SettingsExport
# Class for exporting and importing webadmin settings from/to mysql.
#
# DEPRECATED! Use SD::Settings::Import and SD::Setting::Export instead.
# 

package Boitho::SettingsExport;
use strict;
use warnings;
use Carp;
use MIME::Base64;
use Sql::Sql;

sub new($$) {
	my ($class, $dbh) = @_;
	my $self = {};
	$self->{'dbh'} = $dbh;
	bless $self, $class;
	return $self;
}

## Export settings-related tables from database.
## Returns a (dobule) base64 obfuscated sql dump.
sub export_settings($$) {
	my ($self, $obfuscate_it) = @_;
	my $settings_ptr = Sql::Sql->new->read_config();
	
	my $db = $settings_ptr->{'database'};
	my $pass = $settings_ptr->{'Password'};
	my $user = $settings_ptr->{'user'};
	
	my @tables_to_export = (
		'collectionAuth',
		'shares',
		'shareGroups',
		'config',
	);

	my $command = "mysqldump ".
                "--complete-insert " .
		"--no-create-info ".
		"--password=\Q$pass\E ".
		"--user=\Q$user\E ".
		"\Q$db\E ".
		join " ", @tables_to_export;

	open my $md, "$command |"
		or croak "Could not open execute mysqldump $!\n";
	my @output = <$md>;

	return encode_base64(encode_base64(join '', @output))
		if ($obfuscate_it);
	
	return join '', @output;
}

## Import settings exported earlier.
## NOTE: Does not verify content of what is being imported.
## The user is able to execute any sql query through this.
sub import_settings($$$) {
	my ($self, $file, $is_obfuscated) = @_;
	my $dbh = $self->{'dbh'};
	
	my $imported = $self->_fetch_import_contents($file);
	my $sql_string;

	if ($is_obfuscated) {		
		$sql_string
			= decode_base64(decode_base64($imported));
	}
	else {
		$sql_string = $imported;
	}

	croak "Imported file had no content."
		unless $sql_string;

	my @sql_queries = split "\n", $sql_string;

	
	
	foreach my $query (@sql_queries) {
		next unless $query; # Ignore empty lines.
		next if $query =~ /^--/; # Ignore comments
		Sql::Sql::simple_execute($dbh, $query);
	}
	1;
}

## Reads data from a filed post from a web-form
##
sub _fetch_import_contents($$) {
	my ($self, $file) = @_;
	my ($content, $buffer);
	while (read($file, $buffer, 1024)) {
		$content .= $buffer;
	}
	
	return $content;
}

1;
