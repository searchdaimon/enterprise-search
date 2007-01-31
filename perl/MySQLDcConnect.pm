package MySQLDcConnect;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw();
@EXPORT_OK = qw();



####################################################################
#settup
#####
#MySQL settup
$user = "boitho";
$Password = "G7J7v5L5Y7";
$server = "localhost";
$database = "boithodc";

use DBI; #bruker DBI databse interfase

#kobler opp og returnerer database hantereren
sub GetHandler {
	# dette kjøres hver gang mymod.pm kalles
	my $dbh = DBI->connect("DBI:mysql:database=$database;host=$server;port=3306",						#Kobler til databasen
                             $user, $Password) or warn("Can`t connect: $DBI::errstr");	#
	
return $dbh;						 
}
							 
		
