use DB_File;
use strict;

tie my %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");

foreach my $key (keys %DBMrobottxt) {
	if ($key !~ / /) {
		print "$key $DBMrobottxt{$key}\n";
	}
}

untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");
