package Page::ClientTPL::Form;
use strict;
use warnings;

use Carp;
use Readonly;
use Data::Dumper;
use File::Copy qw(copy);
use File::Path qw(rmtree);
use File::Find qw(find);
use Params::Validate qw(validate_pos OBJECT SCALAR ARRAYREF);

use Page::ClientTPL;
our @ISA = qw(Page::ClientTPL);

use config qw(%CONFIG);

Readonly::Scalar my $TPL_LIST  => "clienttpl_list.html";
Readonly::Scalar my $TPL_EDIT  => "clienttpl_edit.html";
Readonly::Scalar my $TPL_DEL   => "clienttpl_del.html";

Readonly::Scalar my $SKELETON_DIR   => "skeletons/clienttpl";
Readonly::Array  my @SKELETON_TPL_FILES => @Page::ClientTPL::REQ_TPL_FILES;
Readonly::Array  my @SKELETON_LANG => qw(no en_us);

Readonly::Scalar my $LANG_FILE_NAME => "search.po";

sub new_template {
	my ($s, $tpl_vars) = @_;

	# create name & dir
	my $name = $s->gen_new_tpl_name("new_template");
	my $tpl_path = $s->tpl_path($name);
	mkdir $tpl_path
		or croak "Unable to create path $tpl_path";

	# copy over tpl skeleton files
	for my $file (@SKELETON_TPL_FILES) {
		my $file_path = "$SKELETON_DIR/$file";
		copy($file_path, $tpl_path)
			or croak "unable to copy $file_path to $tpl_path: ", $!;
	}
	# lang skeleton files
	for my $lang (@SKELETON_LANG) {
		$s->create_lang_dir($name, $lang);
		my $file_path = "$SKELETON_DIR/${lang}_search.po";
		my $lang_path = $s->lang_file_path($name, $lang);
		copy($file_path, $lang_path)
			or croak "unable to copy $file_path to $lang_path: ", $!;
	}

	return $s->show_edit($tpl_vars, $name);
}




sub gen_new_tpl_name {
	my ($s, $prefix) = @_;
	my $name = $prefix . "_";
	my $i = 1;
	while ($s->tpl_exists($name . $i)) {
		$i++;
	}
	$name .= $i;
}

##
# Lists all templates
sub show_list {
	my ($s, $tpl_vars) = @_;
	opendir my $dh, $CONFIG{client_tpl_path}
		or croak "open $CONFIG{client_tpl_path}: ", $!;
	
	my (@ro_tpl, @custom_tpl);
	for my $tpl (readdir $dh) {
		next if $s->in_ign_list($tpl);
		next unless $s->is_valid_name($tpl);
		$s->is_readonly_tpl($tpl) 
			? push @ro_tpl, $tpl
			: push @custom_tpl, $tpl;
	}
	
	$tpl_vars->{ro_tpl} = \@ro_tpl;
	$tpl_vars->{custom_tpl} = \@custom_tpl;
	return $TPL_LIST;
}

##
# 
sub show_edit {
	my ($s, $tpl_vars, $tpl) = @_;
	croak "Invalid template"
		unless $s->is_valid_name($tpl);
	croak "Template does not exist"
		unless $s->tpl_exists($tpl);

	$tpl_vars->{is_readonly} = $s->is_readonly_tpl($tpl);
	$tpl_vars->{tpl_files} = [ $s->_list_tpl_files($tpl) ];
	$tpl_vars->{lang_files} = [ $s->_list_lang_files($tpl) ];
	$tpl_vars->{tpl} = $tpl;

	return $TPL_EDIT;
}

sub _del_validate {
	my ($s, $tpl) = @_;
	croak "Invalid template"
		unless $s->is_valid_name($tpl);
	croak "Template does not exist"
		unless $s->tpl_exists($tpl);
	croak "Template is read only"
		if $s->is_readonly_tpl($tpl);
}

sub show_del {
	my ($s, $tpl_vars, $tpl) = @_;
	$s->_del_validate($tpl);
	$tpl_vars->{tpl_name} = $tpl;

	return $TPL_DEL;
}

sub del_template {
	my ($s, $tpl_vars, $tpl) = @_;
	$s->_del_validate($tpl);
	my $path = $s->tpl_path($tpl);

	rmtree($path); # croaks on fatal error, API says.
	croak "template '$path' was not deleted, possible partial delete"
		if -d $path;
	return $s->show_list($tpl_vars);
}

sub clone_template {
	my ($s, $tpl_vars, $old_tpl) = @_;

	croak "Invalid template"
		unless $s->is_valid_name($old_tpl);
	croak "Template does not exist"
		unless $s->tpl_exists($old_tpl);
	my $new_tpl = $s->gen_new_tpl_name("copy_of_" . $old_tpl);
	my $new_tpl_path = $s->tpl_path($new_tpl);
	mkdir $new_tpl_path
		or croak "Unable to create path $new_tpl_path";

	# copy template files
	find(
		sub { 
			my $file = $_;
			unless ($file =~ /\.tpl$/) {
				warn "Ignoring file '$file'";
				return;
			}
			my $o_path = $s->tpl_file_path($old_tpl, $file);
			my $n_path = $s->tpl_file_path($new_tpl, $file); 
			copy($o_path, $n_path)
				or croak "Unable to copy '$file' ", $!;
			
		},
		$s->tpl_path($old_tpl)
	);

	# copy language files
	my $old_locale_dir = $s->lang_path($old_tpl);
	opendir my $dh, $old_locale_dir
		or croak "unable to open dir $old_locale_dir", $!;

	$s->create_lang_dir($new_tpl);
	for my $dir (readdir $dh) {
		next if $s->in_ign_list($dir);
		unless ($s->is_lang_dir($old_tpl, $dir)) {
			warn "ignoring non-lang-dir $dir";
			next;
		}
		my $o_path = $s->lang_file_path($old_tpl, $dir);
		my $n_path = $s->lang_file_path($new_tpl, $dir);
		$s->create_lang_dir($new_tpl, $dir);
		copy($o_path, $n_path)
			or croak "unable top copy '$dir' ", $!;
	}

	return $s->show_edit($tpl_vars, $new_tpl);

}

sub _list_tpl_files {
	my ($s, $tpl) = @_;
	my $path = $CONFIG{client_tpl_path} . "/" . $tpl;
	opendir my $dh, $path
		or croak "open tpldir $path:", $!;
	my @tpl_files;
	for my $file (readdir $dh) {
		next if $s->in_ign_list($file);
		if ($file !~ /\.tpl$/) {
			warn "Ignoring non-template file '$file'";
			next;
		}
		push @tpl_files, $file;
	}
	return @tpl_files;
}



sub _list_lang_files {
	my ($s, $tpl) = @_;
	
	my $path = $s->lang_path($tpl);
	my @lang_dirs;
	opendir my $dh, $path
		or croak "open langdir $path: ", $!;

	for my $dir (readdir $dh) {
		next if $s->in_ign_list($dir);
		if (!$s->is_lang_dir($tpl, $dir)) {
			warn "ignoring non-lang dir: $dir";
			next;
		}
		push @lang_dirs, $dir;
	}
	return @lang_dirs;
}


1;
