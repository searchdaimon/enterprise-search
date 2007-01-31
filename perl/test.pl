
use Boitho::Lot;
use Socket;

Boitho::Lot::lotlistLoad();

my %lotStorageServerCashe = {};

sub calkualtStorageServer {
        my $DocID = shift;
        my $lot = Boitho::Lot::rLotForDOCid($DocID);

	my $ipaddr = '';

	if (not exists $lotStorageServerCashe{$lot}) {

        	#husk vi bruker IP her, ikke hostname !!!!!
        	my $hostname =  Boitho::Lot::lotlistGetServer($lot);

		#henter ip
		$ipaddr = inet_ntoa(inet_aton($hostname));

		$lotStorageServerCashe{$lot} = $ipaddr;

	}
	else {
		print "got ip froa cashe\n";
		$ipaddr = $lotStorageServerCashe{$lot};
	}

	return $ipaddr;
	
}

print calkualtStorageServer(1) . "\n";
print calkualtStorageServer(1) . "\n";
