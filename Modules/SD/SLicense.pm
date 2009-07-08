##
# slicence wrapper
package SD::SLicense;
use strict;
use warnings;
use Readonly;
use Params::Validate qw(validate_pos);
use Exporter qw(import);
use Carp;
our @EXPORT = qw(license_info);
our @EXPORT_OK = qw($DB_LICENSE_FIELD);

Readonly::Scalar my $SLICENSE_DEFAULT_PATH => $ENV{BOITHOHOME} . "/bin/slicense_info";
Readonly::Scalar our $DB_LICENSE_FIELD => "licensekey";

sub license_info {
	validate_pos(@_, 1, 0);
	my ($license_key, $slicense_info_path) = @_;
	$slicense_info_path ||= $SLICENSE_DEFAULT_PATH;

	if (!$license_key) {
		warn "license key not provided";
		return (valid => 0);
	}

	# fetch info
	my %lic_info;
	open my $h, "$slicense_info_path \Q$license_key\E |"
		or croak "Unable to open $slicense_info_path";
	while (my $line = <$h>) {
		if ($line =~ /^([a-z]+): ([a-z0-9]+)$/) {
			my ($k, $v) = ($1, $2);
			$v = 1 if $v eq 'yes';
			$v = 0 if $v eq 'no';
			$lic_info{$k} = $v;
		}
		else {
			warn "Unable to parse '$line' from slicense_info";
		}
	}
	close $h or warn "slicense_info exited with error";

	return %lic_info;
}

1;
