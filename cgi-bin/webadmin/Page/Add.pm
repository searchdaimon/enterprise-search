package Page::Add;
use strict;
use warnings;
use Page::Abstract;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::ShareUsers;
use Sql::ScanResults;
use Sql::Param;
use Boitho::Infoquery;
BEGIN { push @INC, $ENV{BOITHOHOME} . "/Modules" }

use Data::Dumper;
use Common::Collection;
use Data::Collection;
use Carp;

use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

sub _init {
	my $self = shift;
        my $dbh = $self->{dbh};
	$self->{sqlShares} = Sql::Shares->new($dbh);
	$self->{sqlConnectors} = Sql::Connectors->new($dbh);
	$self->{sqlAuth} = Sql::CollectionAuth->new($dbh);
	$self->{collection} = Common::Collection->new($dbh);
        $self->{sqlResults} = Sql::ScanResults->new($dbh);
	
}

##
# Adds a given share to the database.
sub add_share {
    my ($s, %share) = @_;

    my %attr;
    for my $key (keys %COLLECTION_ATTR) {
        if (defined $share{$key}) {
            $attr{$key} = $share{$key};
        }
    }

    $attr{active} = defined $share{active}; # HTML form sets it to "on"

    my $coll = Data::Collection->new($s->{dbh}, \%attr);
    unless ($attr{auth_id}) { 
        $coll->set_auth($share{username}, $share{password});
    }
    $coll->create();

	# Start crawling the collection
	{
		my $iq = new Boitho::Infoquery($CONFIG->{infoquery});
		$iq->crawlCollection($attr{collection_name})
			or carp $iq->error;
	}
    
}

# Print the html for the first form used when adding a share.
sub show_first_form {
	my ($self, $vars) = (@_);
	my $sql = $self->{'sqlConnectors'};

	$vars->{'connectors'} = [ $sql->get({ active => 1}) ];
	return ($vars, 'add.html');

}

# Print the html for the second form used when adding a share.
sub show_second_form {
	my ($self, $vars, $connector) = (@_);
        
        my $coll = Data::Collection->new($self->{dbh}, { 
                connector => $connector, 
        });
            

	my $sqlConnectors = $self->{sqlConnectors};
	#my $sqlAuth = $self->{sqlAuth};
        #my $sqlParam = Sql::Param->new($self->{dbh});
	#my $iq = new Boitho::Infoquery($CONFIG->{infoquery});

	my $share = $self->{state}->{'share'};
	if (!$share->{connector_name}) {
		$share->{connector_name} = $sqlConnectors->get_name($connector)
	}
        $vars->{share} = $share;
        #$vars->{parameters} = [ $sqlParam->get({connector => $connector}) ];

	#my @auth_data = $sqlAuth->get_all_auth();

        #my $input_fields_ref = $sqlConnectors->get_input_fields($connector);
        #my @fields = $input_fields_ref ? @{$input_fields_ref} : ();
        my %form_data = $coll->form_data();#self->{collection}->coll_form_data(@fields);
        #die Dumper(\%form_data);

        while (my ($k, $v) = each %form_data) {
            $vars->{$k} = $v;
        }

	my $state = $self->{state};
	$vars->{'from_scan'} = $state->{'from_scan'}
		if $state->{'from_scan'}; # Coming from scan result. Contain's id.
	
	return ($vars, 'add_details.html');

}

## Handles what to do when the user submits the first form.
sub submit_first_form {
	my ($self, $vars, $share, %misc_opts) = (@_);
	my $dbh        = $self->{'dbh'};
	my $state      = $self->{'state'};
	my $collection = $self->{'collection'};
	my ($valid, $msg) = $collection->validate($share, 
			qw(collection_name connector));
			
	my $template_file;
	
	unless($valid) {
		# User didn't provide valid input.
		# Show the first form again.
		$vars->{$msg} = 1;
		$vars->{'share'} = $share;

		if (%misc_opts) {
			$vars->{from_scan} = $misc_opts{from_scan};
		}
		($vars, $template_file) =
			$self->show_first_form($vars);
	}
	else {
		# All OK. Show the second form.
		$vars->{'state'} = $state;
		($vars, $template_file) = 
			$self->show_second_form($vars, $share->{'connector'});
	}
	
	return ($vars, $template_file);
}

sub submit_second_form {
	my ($self, $vars, $share, %misc_opts) = (@_);

	
	my $state = $self->{'state'};
	my $dbh = $self->{'dbh'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $collection = $self->{'collection'};
	
	if ($share->{collection_name} eq "") {
		$share->{collection_name} = $self->_gen_coll_name();
	}
	
	# Check for any errors
	my ($valid, $msg) = $collection->validate(
		$share,	qw(collection_name connector));
				
	unless($valid) {
		# Input from the form was not valid. Show it again.
		$vars->{$msg} = 1; # Add the error to vars
		$vars->{'share'} = $share;
		$vars->{'state'} = $state;
		
		if (%misc_opts) {
			$vars->{from_scan} = $misc_opts{from_scan};
		}

		return $self->show_second_form($vars, $share->{'connector'});
	}

        # Input was all OK.
        # Add share to database.
        $self->add_share(%{$share});
        
        $vars->{'success'} = 1;

        if (defined($state->{'from_scan'})) {
                # User was adding a collection from scan results.
                # We'll just send him back to it.
                print CGI::redirect("scan.cgi?action=process&scan_added=1&id=" . $state->{'from_scan'});
                exit;
        }

        return $self->show_first_form($vars);
}

##
# Add template vars based on result from scan.
sub vars_from_scan {
    my ($self, $share, $scan_id) = @_;

    croak "Missing arguments path or host from scan"
        unless $share->{host} and $share->{path};

    $share->{auth_id} 
        = $self->{sqlResults}->get_authid($scan_id);


    my $sqlConnectors = $self->{'sqlConnectors'};
    if ($sqlConnectors->get_name($share->{connector}) eq 'SMB') {
        $share->{'resource'} = q{\\\\} . $share->{host} . q{\\} . $share->{path};
    }
}


sub _gen_coll_name {
	my $s = shift;

    my @chars = ('a'..'z', 'A'..'Z', 0..9);
    my $name;
    do {
        $name = "collection_";
        $name .= $chars[rand @chars] for 1..5;
    } while ($s->{sqlShares}->collection_name_exists($name));
    return $name;
}

1;
