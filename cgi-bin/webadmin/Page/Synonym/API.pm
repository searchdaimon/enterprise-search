package Page::Synonym::API;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos);

use Page::API;
use Page::Synonym;
use Sql::Synonym;
use Sql::SynonymGroup;
use config qw(%CONFIG);
our @ISA = qw(Page::Synonym Page::API);




sub synonym_list {
	my ($s, $api_vars) = @_;
	my $group = $s->{sqlsyngroup};

	$api_vars->{list} = [ map {
		$_->{language} = $group->get_lang($_->{group});
		$_;
	} $s->{sqlsyn}->all_lists() ];

	$api_vars->{ok} = 1;
	1;
}

sub update {
	validate_pos(@_, 1, 1, 1, 1, 1);
	my ($s, $api_vars, $group, $list, $lang) = @_;
	eval {
		croak "Record does not exist."
			unless $s->{sqlsyngroup}->exists({ id => $group });

		$s->{sqlsyngroup}->update({ language => $lang }, { id => $group });

		# Delete all words in list, before reinserting
		$s->{sqlsyn}->delete({ group => $group });

		$s->_insert_synonym_words($group, $list);
		
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Updated";
	}
	1;
}

sub _insert_synonym_words {
	validate_pos(@_, 1, 1, 1);
	my ($s, $group, $list) = @_;;
	my %added;
	for my $word (split q{,}, $list) {
		$word =~ s/^\s+|\s+$//g;
		next if $word eq q{};
		next if $added{$word};

		$s->{sqlsyn}->insert({ group => $group, word => $word });
		$added{$word} = 1;
	}
	return scalar keys %added; #num rows added
}

sub add {
	validate_pos(@_, 1, 1, 1, 1);
	my ($s, $api_vars, $list, $lang) = @_;
	eval {
		my $group_id = $s->{sqlsyngroup}->insert({ language => $lang, }, 1);
		$s->_insert_synonym_words($group_id, $list);
		$api_vars->{group} = $group_id;
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Added";
	}
}
sub delete {
	validate_pos(@_, 1, 1, 1);
	my ($s, $api_vars, $group) = @_;
	eval {
		croak "Record does not exist."
			unless $s->{sqlsyngroup}->exists({ id => $group });
		$s->{sqlsyn}->delete({ group => $group });
		$s->{sqlsyngroup}->delete({ id => $group });
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Deleted";
	}
}


1;
