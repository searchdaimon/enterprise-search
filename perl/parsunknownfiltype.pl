if ($#ARGV == -1) {
        print "parsunknownfiltype.pl file.log\n";
        exit;
}


open(INF,"$ARGV[0]") or die("$ARGV[0]: $!");

my %filtypes;
while(<INF>) {
	chomp;

	#print "l $_\n";

	my @elements = split(/: /,$_);
	my $type = $elements[$#elements];

	#print "type: $type\n";
	$filtypes{$type} = $filtypes{$type} +1;
}


close(INF);


foreach my $i (sort { $filtypes{$b} <=> $filtypes{$a} } keys %filtypes) {
	printf("$i: $filtypes{$i}\n");
}
