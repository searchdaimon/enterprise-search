#!/home/boitho/boithoTools/bin/perl


if ($ARGV[0] && $ARGV[0] eq "groupsforuser" && $ARGV[1]) {
	# The username is in $ARGV[1]
	my $username = $ARGV[1];

        # Decides witch user is in which group
        # Here we have only two groups:
        #       * Musketeers with athos, aramis and porthos
        #       * Servants with planchet

	# Deeside who is in which group
	if ($username eq 'athos' || $username eq 'aramis' || $username eq 'porthos') {
		print "musketeers\n";
	}
	elsif ($username eq 'planchet') {
		print "servants\n";
	}
	else {
		warn("Unknow user!");
	} 

}
elsif ($ARGV[0] && $ARGV[0] eq "listusers") {
	# List all users, in this case The Three Musketeers and there servent Planchet
	print "athos\n";
	print "aramis\n";
	print "porthos\n";
	print "planchet\n";
}
else {
        print "Usage:\n";
        print "\tusers.pl groupsforuser \"user\"\n";
        print "\tList the groups user is member of.\n\n";
        print "\tusers.pl listusers\n";
        print "\tList all users.\n";

        exit;

}

