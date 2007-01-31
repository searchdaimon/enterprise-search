use DB_File;

#låser, slik at bare vi har tilgang til databasen
open(LOCK,">../data/robottxt.lock") or die ("Cant open data/robottxt.lock: $!");
#print "locking robottxt.lock\n";
flock(LOCK,2) or die ("Can't lock lock file: $!");
print LOCK $$;


#opner databasen
tie %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");

foreach my $domene (keys %DBMrobottxt) {

	my $DomenesRobottxt = $DBMrobottxt{$domene};
	my ($RobotsTxtType,$RobotsTxtData) = unpack('A A*',$DomenesRobottxt);

	if ($RobotsTxtType ne '2') {
		print "domene: $domene\nRobotsTxtType: $RobotsTxtType\nRobotsTxtData: $RobotsTxtData\n\n";
	}
}

untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");

close(LOCK);
