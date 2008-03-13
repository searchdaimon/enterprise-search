package Sql::SessionData;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Sql::Abstract;
our @ISA = qw(Sql::Abstract);

my $tbl = "sessionData";
my @valid = qw(id success type data);

sub _init {
	my $self = shift;
}

sub insert {
    my ($s, %data) = @_;

    my ($attrb, $values, @binds) = $s->construct_insert_query(
        \%data, @valid);
    my $q = "INSERT INTO $tbl ($attrb) VALUES ($values)"

    return $s->sql_insert_returning_id($q, @binds);
}

sub update {
    my ($s, $id, %data) = @_;
    my ($set_str, @binds) = $s->construct_update_query(
        \%data, @valid);
    my $q = "UPDATE $tbl $set_str WHERE id = ?",
    return $s->sql_update($q, @binds, $id);
}
    
        
sub delete_by_type {
    my ($s, $type) = @_;
    my $q = "DELETE FROM $tbl WHERE type = ?";
    return $s->sql_delete($q, $type);
}

1;
