package Page::Connector::Form;
use strict;
use warnings;

use Carp;
use Params::Validate qw(validate_pos OBJECT);

use Page::API;
use Page::Abstract;
our @ISA = qw(Page::Abstract);

sub _init {
	validate_pos(@_, 1, 0);
	my ($s, $utils) = @_;

	for (qw(sql_conn)) {
		croak "subclass did not define '$_'"
			unless $s->{$_};
	}

	$s->{utils} = $utils;
	$s;
}

sub show_list {
	my ($s, $tpl_vars) = @_;
	
	$tpl_vars->{sd_connectors}
		= [ $s->{sql_conn}->get({extension => 1, read_only => 1}, '*', 'name') ];

	$tpl_vars->{local_connectors}
		= [ $s->{sql_conn}->get({extension => 1, read_only => 0}, '*', 'name') ];

	return;
}

sub upload_source {
	validate_pos(@_, 1, { regex => qr(^\d+$)}, { type => OBJECT }, 1);
	my ($s, $conn_id, $file, $path) = @_;

	$s->{sql_conn}->is_extension($conn_id)
		or croak "'$conn_id' is not a connector extension";

	not $s->{sql_conn}->is_readonly($conn_id)
		or croak "'$conn_id' is read only";


	my ($source, $buff);
	while (read $file, $buff, 1024) {
		$source .= $buff;
	}

	open my $fh, ">", $path
		or croak "Unable to open source file '$path' for writing", $!;

	print {$fh} $source;

	1;
}



1;

