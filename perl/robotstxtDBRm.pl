#sletter ../data/robottxt


open(UDFILE,"../data/robottxt.lock") or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);
flock(UDFILE,2) or die($!);

unlink("../data/robottxt") or warn($!);


close(UDFILE);
