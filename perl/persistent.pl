 package Embed::Persistent;
 #persistent.pl

 use strict;
 our %Cache;
 use Symbol qw(delete_package);
 use Devel::Symdump;
 use Carp;
 BEGIN { 
	unshift @INC, $ENV{BOITHOHOME} . "/Modules";
 }

 sub valid_package_name {
     my($string) = @_;
     $string =~ s/([^A-Za-z0-9\/])/sprintf("_%2x",unpack("C",$1))/eg;
     # second pass only for words starting with a digit
     $string =~ s|/(\d)|sprintf("/_%2x",unpack("C",$1))|eg;

     # Dress it up as a real package name
     $string =~ s|/|::|g;
     return "Embed" . $string;
 }

sub eval_pkg {
	my ($pkg, $sub) = @_;
	#wrap the code into a subroutine inside our unique package
	eval qq|package $pkg; sub handler {  $sub; } |;
	die $@ if $@;
}

#void perl_embed_run(char *file_path, char *func_name, HV *func_params, char *obj_name, HV *obj_attr) {
sub eval_file2 {
	my ($file_path, $keep_cache, $func_name, $func_params_ref, $bless_name, $bless_ref) = @_;
	my $package = valid_package_name($file_path);
	my $mtime = -M $file_path;


	if(defined $Cache{$package}{mtime}
			&&
			$Cache{$package}{mtime} <= $mtime)
	{
		# we have compiled this subroutine already,
		# it has not been updated on disk, nothing left to do
		#print STDERR "Already compiled $package->handler\n";
	}
	else {

		local $/ = undef;

		open my $srch, $file_path
			or die "open '$file_path'", $!;
		my $sub = <$srch>;
		close $srch;

		my $eval = qq|package $package; sub handler {  $sub; } |;

		# eval out of the scope of this func
		# so our variables won't be accessable.
		eval_pkg($package, $sub);

		$Cache{$package}{mtime} = $mtime 
			if $keep_cache;
	}
	my $ret;
	eval { 
		$package->handler();
		if (defined $bless_name) {
			$ret = $package->$func_name(bless($bless_ref, $bless_name), $func_params_ref);
		}
		else {
			$ret = $package->$func_name($func_params_ref);
		}
	};
	die $@ if $@;

	delete_package($package) unless $keep_cache;

	#take a look if you want
	#print Devel::Symdump->rnew($package)->as_string, $/;

#print "eval_file: rutine return value $ret\n";
	return $ret;
}

	
 1;

