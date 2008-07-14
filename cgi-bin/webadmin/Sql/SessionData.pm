package Sql::SessionData;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Sql::Webadmin;
our @ISA = qw(Sql::Webadmin);

my $tbl = "sessionData";
my @valid = qw(id success type data);


sub insert {
    my ($s, %data) = @_;

    my ($attrb, $values, @binds) = $s->construct_insert_query(
        \%data, @valid);
    my $q = "INSERT INTO $tbl ($attrb) VALUES ($values)";

    return $s->sql_insert_returning_id($q, @binds);
}

sub update {
    my ($s, $id, %data) = @_;
    croak "missing restart id"
        unless $id =~ /^\d+$/;

    my ($set_str, @binds) = $s->construct_update_query(
        \%data, @valid);
    my $q = "UPDATE $tbl SET $set_str WHERE id = ?";

    return $s->sql_update($q, @binds, $id);
}
    
sub delete_by_type {
    my ($s, $type) = @_;
    my $q = "DELETE FROM $tbl WHERE type = ?";
    return $s->sql_delete($q, $type);
}

sub get {
    my ($s, $id) = @_;
    croak "not valid id" 
        unless $id =~ /^\d+$/;
    my $q = "SELECT * FROM $tbl WHERE id = ?";
    my ($res) = $s->sql_hashref($q, $id);
    return %{$res} if $res;
    return;
}

sub exists { shift->SUPER::exists($tbl, 'id', @_) }
sub delete { shift->SUPER::delete($tbl, @_) }



1;
