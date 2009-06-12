package Page::ClientTPL::API;
use strict;
use warnings;
use Readonly;
use Carp;
use File::Copy qw(move);
use File::Path qw(rmtree);
use Params::Validate qw(validate_pos);

use Page::ClientTPL;
use Page::API;
use config qw(%CONFIG);
our @ISA = qw(Page::ClientTPL Page::API);

Readonly::Scalar my $FILE_TPL => "tpl";
Readonly::Scalar my $FILE_LANG => "lang";


sub fetch_file {
	my ($s, $api_vars, $tpl, $file, $file_type) = @_;

	eval {
		if ($file_type eq $FILE_TPL) {
			$api_vars->{contents} = $s->_fetch_tpl_file($tpl, $file);
		}
		elsif ($file_type eq $FILE_LANG) {
			$api_vars->{contents} = $s->_fetch_lang_file($tpl, $file);
		}
		else {
			croak "Unknown file type '$file_type'";
		}
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = 1;
	}
	1;
}

sub _fetch_lang_file {
	my ($s, $tpl, $lang) = @_;
	$s->_validate_lang($tpl, $lang);
	my $path = $s->lang_file_path($tpl, $lang);
	return $s->_fetch_file_contents($path);
}

sub _fetch_tpl_file {
	my ($s, $tpl, $tpl_file) = @_;
	$s->_validate_tpl($tpl, $tpl_file);

	my $path = $s->tpl_file_path($tpl, $tpl_file);
	return $s->_fetch_file_contents($path);
}

sub save_file {
	validate_pos(@_, 1, 1, 1, 1, 1, 1);
	my ($s, $api_vars, $tpl, $file, $source, $file_type) = @_;
	eval {
		croak "Template is readonly"
			if $s->is_readonly_tpl($tpl);

		$s->_validate_tpl($tpl, undef);

		my $path;
		if ($file_type eq $FILE_TPL) {
			croak "Invalid template file"
				unless $s->is_valid_tplfile($file);

			$path = $s->tpl_file_path($tpl, $file);
		}
		elsif ($file_type eq $FILE_LANG) {
			$s->_validate_lang($tpl, $file);
			$path = $s->lang_file_path($tpl, $file);
		}
		else { croak "unsupported filetype" }
		
		$s->_save_to_file($path, $source);
	};
	$api_vars->{ok} = 1
		unless $s->api_error($api_vars, $@);
}

sub del_file {
	validate_pos(@_, 1, 1, 1, 1, 1);
	my ($s, $api_vars, $tpl, $file, $file_type) = @_;
	eval {
		croak "Template is readonly"
			if $s->is_readonly_tpl($tpl);

		$s->_validate_tpl($tpl, undef);

		if ($file_type eq $FILE_TPL) {
			croak "Invalid template file"
				unless $s->is_valid_tplfile($file);

			croak "'$file' is a required file and can not be deleted"
				if grep { $file eq $_ } @Page::ClientTPL::REQ_TPL_FILES;

			my $path = $s->tpl_file_path($tpl, $file);
			unlink $path 
				or croak "Unable to delete template file: ", $!;
		}
		elsif ($file_type eq $FILE_LANG) {
			$s->_validate_lang($tpl, $file);
			my $lang_dir = $s->lang_path($tpl, $file) . "/$file";

			#croak $lang_dir  ;
			rmtree($lang_dir); # croaks on fatal
		}
		else { croak "Unsupported filetype" }
		
	};
	$api_vars->{ok} = 1
		unless $s->api_error($api_vars, $@);
}

sub rename_template {
	my ($s, $api_vars, $tpl, $new_tpl_name) = @_;
	eval {
		# old name validation
		$s->_validate_tpl($tpl);


		# new name validation
		croak "Invalid template name"
			unless $s->is_valid_name($new_tpl_name) 
			   && !$s->in_ign_list($new_tpl_name);
		croak "Template already exists"
			if $s->tpl_exists($new_tpl_name);

		{ # move templates
			my $old_path = $s->tpl_path($tpl);
			my $new_path = $s->tpl_path($new_tpl_name);
			move($old_path, $new_path)
				or croak "Could not move '$old_path' to '$new_path'", $!;
		}

		{ # move language
			my $old_path = $s->lang_path($tpl);
			my $new_path = $s->lang_path($new_tpl_name);
			move($old_path, $new_path)
				or croak "Could not move '$old_path' to '$new_path'", $!;
		}
	};
	$api_vars->{ok} = 1
		unless $s->api_error($api_vars, $@);
}

sub create_file {
	my ($s, $api_vars, $tpl, $file, $file_type) = @_;

	my $new_file_name;
	eval {

		# Validate 
		croak "Template is readonly"
			if $s->is_readonly_tpl($tpl);

		$s->_validate_tpl($tpl, undef);

		# Create file
		if ($file_type eq $FILE_TPL) {
			$new_file_name = $s->_create_template($tpl, $file);
		}
		elsif ($file_type eq $FILE_LANG) {
			$new_file_name = $s->_create_lang($tpl, $file);
		}
		else { croak "unknown filetype '$file_type'" }
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = 1;
		$api_vars->{new_file_name} = $new_file_name;
	}
}

sub _create_lang {
	my ($s, $tpl, $new_lang) = @_;

	croak "Invalid language"
		unless $s->is_valid_lang($new_lang);
	
	croak "Language already exists"
		if $s->lang_exists($tpl, $new_lang);
	
	$s->create_lang_dir($tpl, $new_lang);
	my $lang_file_path = $s->lang_file_path($tpl, $new_lang);
	open my $fh, ">", $lang_file_path
		or croak "Unable to create language file ($lang_file_path) ", $!;
	print {$fh} qq{msgid = ""\nmsgstr = ""\n\n};
	close $fh;
	
	return $new_lang;
}

sub _create_template {
	my ($s, $tpl, $new_tpl_file) = @_;
	$new_tpl_file .= ".tpl"
		unless $new_tpl_file =~ /\.tpl$/;

	croak "Invalid file name"
		unless $s->is_valid_tplfile($new_tpl_file);

	my $path = $s->tpl_file_path($tpl, $new_tpl_file);

	croak "File already exists"
		if -e $path;

	# Create file
	open my $fh, ">", $path
		or croak "Unable to create file: ", $!;
	close $fh;

	return $new_tpl_file;
}
	
sub _validate_lang {
	my ($s, $tpl, $lang) = @_;
	croak "Invalid language"
		unless $s->is_valid_lang($lang) && !$s->in_ign_list($tpl);
	croak "Language file does not exist"
		unless $s->tpl_exists($tpl) && $s->lang_exists($tpl, $lang);
}

sub _validate_tpl {
	my ($s, $tpl, $tpl_file) = @_;
	croak "Invalid template"
		unless $s->is_valid_name($tpl) && !$s->in_ign_list($tpl);
	croak "Template does not exist"
		unless $s->tpl_exists($tpl);
	if (defined $tpl_file) {
		croak "Invalid template file"
			unless $s->is_valid_tplfile($tpl_file);
	}
	1;
}
##
# Read file contents.
sub _fetch_file_contents {
	my ($s, $path) = @_;

	open my $fh, "<", $path
		or croak "Opening file: " . $!;
	
	return join q{}, <$fh>;
}

sub _save_to_file {
	my ($s, $path, $data) = @_;
	
	open my $fh, ">", $path
		or croak "Saving to file: " . $!;
	print {$fh} $data;
	1;
}


1;
