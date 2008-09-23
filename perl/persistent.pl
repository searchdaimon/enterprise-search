 package Embed::Persistent;
 #persistent.pl

 use strict;
 our %Cache;
 use Symbol qw(delete_package);
 use Devel::Symdump;
 use Carp;
 BEGIN { unshift @INC, $ENV{BOITHOHOME} . "/Modules" }

 sub valid_package_name {
     my($string) = @_;
     $string =~ s/([^A-Za-z0-9\/])/sprintf("_%2x",unpack("C",$1))/eg;
     # second pass only for words starting with a digit
     $string =~ s|/(\d)|sprintf("/_%2x",unpack("C",$1))|eg;

     # Dress it up as a real package name
     $string =~ s|/|::|g;
     return "Embed" . $string;
 }

 #søker gjenom en array på jakt etter key
 sub inArray {
     my($key, @arr) = @_;

     foreach my $i (@arr) {
	if ($i eq $key) {
		return 1;
	}
     }
     return 0;
 }
 sub eval_file {
     my($filename, $folder, $delete, $execute, $pointer, $opt) = @_;
     my $package = valid_package_name($filename);
     my $mtime = -M $filename;
     my $ret = 0;

#     print "eval_file( filename=$filename, delete=$delete, execute=$execute, pointer=$pointer )\n";
#
#     print "options:\n";
#     foreach my $k (keys %{ $opt }) {
# 	     print "$k: $opt->{$k}\n";
#     }
    
     if (!inArray($folder,@INC)) {
		push @INC, $folder;
     }

     if(defined $Cache{$package}{mtime}
        &&
        $Cache{$package}{mtime} <= $mtime)
     {
        # we have compiled this subroutine already,
        # it has not been updated on disk, nothing left to do
        print STDERR "already compiled $package->handler\n";
     }
     else {

        local $/ = undef;

        open my $srch, $filename
            or die "open '$filename'", $!;
        my $sub = <$srch>;
        close $srch;

        #wrap the code into a subroutine inside our unique package
        my $eval = qq|package $package; sub handler {  $sub; } |;
        {
            # hide our variables within this block
            my($filename,$mtime,$package,$sub);
            eval $eval;
        }
        die $@ if $@;

        #cache it unless we're cleaning out each time
        $Cache{$package}{mtime} = $mtime unless $delete;
     }
	#debug: printer ut inc1
	#print "inc1:\n";
	#print join("\n", @INC);
	#print "\n";
		
     eval { 
		$package->handler();
                $ret = $package->$execute(bless({ ptr => $pointer }, 'Perlcrawl'), $opt);
	};

     die $@ if $@;

     delete_package($package) if $delete;

     #take a look if you want
     #print Devel::Symdump->rnew($package)->as_string, $/;
 
     #print "eval_file: rutine return value $ret\n";
     return $ret;
 }

 1;

