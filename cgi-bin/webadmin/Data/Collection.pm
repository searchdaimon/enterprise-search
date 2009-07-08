package Data::Collection;
use strict;
use warnings;
use Carp;
use Readonly;
use Params::Validate qw(validate_pos OBJECT HASHREF);
use Data::Dumper;
use Exporter qw(import);

BEGIN { push @INC, $ENV{BOITHOHOME} . "/Modules" }

use Sql::Shares;
use Sql::Connectors;
use Sql::ShareGroups;
use Sql::ShareUsers;
use Sql::Param;
use Sql::CollectionAuth;
use Boitho::Infoquery;
use Sql::Hash::Param;
use Sql::System;

use config qw(%CONFIG);

our @EXPORT = qw(%COLLECTION_ATTR);


Readonly::Hash our %COLLECTION_ATTR
    => map { $_ => 1 } qw(id collection_name host connector 
        users groups params active auth_id crawler_success rate 
        query1 query2 resource domain userprefix system
	without_aclcheck alias);

sub new {
    validate_pos(@_, 1, { type => OBJECT }, { type => HASHREF });
    my ($class, $dbh, $attr_ref) = @_;

    my %attr = %{$attr_ref};

    my $sqlConn = Sql::Connectors->new($dbh);
    my $sqlShares = Sql::Shares->new($dbh);
    my $sqlAuth = Sql::CollectionAuth->new($dbh);
 
    # Handle data_ref
    for my $key (keys %attr) {
        croak "Collection attribute '$key' is not valid"
            unless $COLLECTION_ATTR{$key};
    }
    if (defined $attr{id}) {
        croak "'id' should be a int" 
            unless $attr{id} =~ /^\d+$/;

        $attr{connector} = $sqlShares->get_connector($attr{id})
            unless defined $attr{connector};
    }
    croak "Missing attribute 'connector'."
        unless defined $attr{connector} && $attr{connector} =~ /^\d+$/;

    my $input_fields_ref = $sqlConn->get_input_fields($attr{connector});
    my %self = (
        dbh  => $dbh,
        attr => \%attr,
        input_fields   => $input_fields_ref, 
        input_fields_h => { map { $_ => 1 } @{$input_fields_ref} },

        sqlShares => $sqlShares,
        sqlConn   => $sqlConn,
        sqlAuth   => $sqlAuth,
    );

    bless \%self, $class;
}

##
# True if a collection exists with collection_name.
sub name_exists {
    my $s = shift;
    $s->{sqlShares}->exists({ 
        collection_name => $s->{attr}{collection_name}
    });
}

sub set_auth {
    my ($s, $user, $pass) = @_;
    my $auth_id = $s->{sqlAuth}->add($user, $pass);
    $s->{attr}{auth_id} = $auth_id;
    $s;
}

sub get_attr { %{shift->{attr}} }

##
# Create a new collection
sub create {
	my $s = shift;

	croak "Collection already exists with id '$s->{attr}{id}'"
		if defined $s->{attr}{id};
	croak "Collection name '$s->{attr}{collection_name}' is taken"
		if $s->{attr}{collection_name} ne "" && $s->name_exists();


	# Create share
	my %ins_attr = $s->_tbl_shares_attr();
	my $id = $s->{sqlShares}->insert(\%ins_attr, 1);
	$s->{attr}{id} = $id;

	$s->update();
}

sub _tbl_shares_attr {
	my $s = shift;
	my %tbl_attr;
	for my $field (@Sql::Shares::FIELDS) {
		next if $field eq 'id';
		next unless exists $s->{attr}{$field};
		$tbl_attr{$field} = $s->{attr}{$field};
	}
	return %tbl_attr;
}

##
# Update existing collection.
sub update {
	my $s = shift;
	my $id = $s->{attr}{id};

	my $sqlGroups = Sql::ShareGroups->new($s->{dbh});
	my $sqlUsers  = Sql::ShareUsers->new($s->{dbh});

	# Errors
	croak "Can not update without collection id"
		unless defined $id;

	# Updates

	my %upd_attr = $s->_tbl_shares_attr();

	$s->{sqlShares}->update(\%upd_attr, { id => $id });

	if ($s->{input_fields_h}{groups}) {
		$sqlGroups->set_groups($id, $s->{attr}{groups});
	}

	if ($s->{input_fields_h}{exchange_user_select}) {
		$sqlUsers->set_users($id, [grep { defined $_ } @{$s->{attr}{users}}]);
	}

	if ($s->{input_fields_h}{custom_parameters}) {
		tie my %collParams, 'Sql::Hash::Param', $id, $s->{dbh};
		my %parameters = $s->{attr}{params} ? %{$s->{attr}{params}} : ();
		$collParams{$_} = $parameters{$_} for keys %parameters;
	}

	$s;
}

sub delete {
    my $s = shift;
    my $id = $s->{attr}{id};

    croak "Can't delete collection without id"
        unless defined $id;
    
    my $sqlGroups = Sql::ShareGroups->new($s->{dbh});
    my $sqlUsers  = Sql::ShareUsers->new($s->{dbh});

    $sqlGroups->delete({ share => $id });
    $sqlUsers->delete({ share => $id });
    tie my %param, 'Sql::Hash::Param', $id, $s->{dbh};
    undef %param;
    $s->{sqlShares}->delete_share($id);

    1;
}


##
# Data needed in a form 
# for editing/managing a collection.
sub form_data {
	my $s = shift;

	my $iq = new Boitho::Infoquery($CONFIG{infoquery});
	my $sqlConnectors = Sql::Connectors->new($s->{dbh});
	my $sqlParam = Sql::Param->new($s->{dbh});
	my $sqlSystem = Sql::System->new($s->{dbh});

	my %form_data;
	$form_data{connectors} = $sqlConnectors->get_connectors()
		if $s->{input_fields_h}{connectors};

	$form_data{group_list} = $iq->listGroups()
		if $s->{input_fields_h}{groups};

	$form_data{user_list} = $iq->listMailUsers()
		if $s->{input_fields_h}{exchange_user_select};

	$form_data{authentication} = [ $s->{sqlAuth}->get_all_auth() ]
		if $s->{input_fields_h}{authentication};

	$form_data{input_fields} = $s->{input_fields};

	$form_data{parameters} = [ $sqlParam->get({ connector => $s->{attr}{connector}}) ]
		if $s->{input_fields_h}{custom_parameters};

	if ($s->{input_fields_h}{user_system}) {
		$form_data{user_systems} = [ 
			$sqlSystem->list()
		#	$sqlSystem->get({ }, 
		#	['name', 'connector', 'id', 'is_primary'], 
		#	['is_primary', 'name'])
		];
	}

	return %form_data;
}

##
# Data for a collection needed in a
# form for deiting/managing.
sub coll_data {
    my $s = shift;
    my $id = $s->{attr}{id};

    my $sqlGroups = Sql::ShareGroups->new($s->{dbh});
    my $sqlUsers  = Sql::ShareUsers->new($s->{dbh});

    my %data = %{ $s->{sqlShares}->get_share($id) };
    $data{connector_name} = $s->{sqlConn}->get_name($s->{attr}{connector});
   
    $data{groups} = $sqlGroups->get_groups($id)
        if $s->{input_fields_h}{groups};

    $data{users} = [$sqlUsers->get_users($id)]
        if $s->{input_fields_h}{exchange_user_select};

    if ($s->{input_fields_h}{custom_parameters}) {
        tie my %params, 'Sql::Hash::Param', $s->{attr}{id}, $s->{dbh};
        $data{params} = \%params;
    }

    return %data;
}

1;
