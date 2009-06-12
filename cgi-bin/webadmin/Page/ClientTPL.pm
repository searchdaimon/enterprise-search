package Page::ClientTPL;
use strict;
use warnings;
use Readonly;
use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos);

use Page::Abstract;
our @ISA = qw(Page::Abstract);
use config qw(%CONFIG);

Readonly::Scalar my $IS_SD_TPL => qr{^(SD|default$)};
Readonly::Scalar my $VALID_TPL_NAME => qr(^[a-zA-Z_\-0-9]+$);
Readonly::Scalar my $VALID_TPLFILE_NAME => qr(^[a-zA-Z_\-0-9]+\.tpl$);
Readonly::Scalar my $VALID_LANGFILE_NAME => qr(^[a-z_]+$);
Readonly::Hash my %FILE_IGN_LIST => map { $_ => 1 } qw(CVS . ..);
Readonly::Array our @REQ_TPL_FILES 
	=> qw(cache.tpl error.tpl main.tpl results.tpl);

Readonly::Scalar our $LANG_FILE_NAME => "search.po";

sub tpl_exists { 
	my ($s, $tpl) = @_;
	my $path = "$CONFIG{client_tpl_path}/$tpl";
	return -e $path && -d $path;
}

sub is_valid_name { $_[1] =~ $VALID_TPL_NAME }
sub is_valid_tplfile { $_[1] =~ $VALID_TPLFILE_NAME }
sub in_ign_list { $FILE_IGN_LIST{$_[1]} }
sub is_readonly_tpl { shift; shift =~ $IS_SD_TPL }

sub tpl_path {
	my ($s, $tpl) = @_;
	return $CONFIG{client_tpl_path} . "/$tpl";
}

sub tpl_file_path {
	my ($s, $tpl, $tpl_file) = @_;
	return $s->tpl_path($tpl) . "/$tpl_file";
}

sub lang_path {
	my ($s, $tpl) = @_;
	return $CONFIG{client_path} . "/locale/" . $tpl;
}
sub lang_file_path {
	my ($s, $tpl, $lang) = @_;
	return $s->lang_path($tpl) . "/$lang/" . $LANG_FILE_NAME;
}

sub is_lang_dir {
	my ($s, $tpl, $dir) = @_;
	return unless $s->is_valid_lang($dir);
	return unless -e $s->lang_path($tpl) . "/$dir/$Page::ClientTPL::LANG_FILE_NAME";
}

sub is_valid_lang { $_[1] =~ /$VALID_LANGFILE_NAME/ }
sub lang_exists {
	validate_pos(@_, 1, 1, 1);
	my ($s, $tpl, $lang) = @_;
	return  (-d $s->lang_path($tpl))
		&& (-e $s->lang_file_path($tpl, $lang));
}

sub create_lang_dir {
	my ($s, $tpl, $lang) = @_;
	my $locale_dir = $s->lang_path($tpl);
	unless (-e $locale_dir) {
		mkdir $locale_dir
			or croak "unable to create $locale_dir", $!;
	}
	return 1 unless $lang;
	my $lang_dir = "$locale_dir/$lang";
	mkdir $lang_dir
		or croak "unable to create $lang_dir", $!;
	1;
}

1;
