package Sql::ShareUsers;
use strict;
use warnings;
use Carp;
use DBI;
use Data::Dumper;
use Sql::Webadmin;

our @ISA = qw(Sql::Webadmin);
my $table = "shareUsers";

sub set_users {
	my ($self, $share_id, $users_ref) = @_;
	
	$self->_delete_users($share_id);
	foreach my $user (@{$users_ref}) {
	    $self->_insert($share_id, $user);
	}
	
	1;
}

sub _insert {
	my ($self, $share_id, $user) = @_;

	
	my $query = "INSERT INTO $table (share, name) 
					VALUES (?, ?)";
        $self->sql_insert($query, $share_id, $user);
}


sub _delete_users {
	my ($self, $share_id) = @_;
	croak "illigal shareid" unless $share_id =~ /^\d+$/;
	my $query = "DELETE FROM $table WHERE share = ?";
	$self->sql_delete($query, $share_id);
}

sub get_users {
    my ($self, $share_id) = @_;
    croak "illigal shareid" unless $share_id =~ /^\d+$/;
    my $query = "SELECT name FROM $table WHERE share = ?";
    return $self->sql_array($query, $share_id);
}

sub get { shift->SUPER::get($table, @_) }
sub insert { shift->SUPER::insert($table, @_) }
sub update { shift->SUPER::update($table, @_) }
sub delete { shift->SUPER::delete($table, @_) }

1;
