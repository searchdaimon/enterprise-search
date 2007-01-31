($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($ARGV[0]);
        $year = $year +1900;

	print "$sec$min:$hour $mday-$mon-$year\n";

