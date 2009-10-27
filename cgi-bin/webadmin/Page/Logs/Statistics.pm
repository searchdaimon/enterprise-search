package Page::Logs::Statistics;
use strict;
use warnings;

use Carp;
use Page::Abstract;
use DateTime;
use File::Temp qw(tempfile);
use Data::Dumper;
use POSIX;
use Readonly;

use Sql::SessionData;
use Sql::Shares;
use config qw(%CONFIG);

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Boitho::Infoquery;

our @ISA = qw(Page::Abstract);

use constant DEFAULT_LAST_VALUE         => 30;
use constant DEFAULT_USERS_VALUE        => 15;
Readonly::Scalar my $TPL_MAIN => "logs_statistics.html";

sub show {
	my ($s, $tpl_vars, $last, $user) = @_;

	$last = DEFAULT_LAST_VALUE
		unless defined $last;
	croak "'last' must be an integer"
		unless int $last;
	$tpl_vars->{last} = $last;
	
	$tpl_vars->{users} = [ sort { $a->{user} cmp $b->{user} }
		@{$s->get_users($last)} ];
	$tpl_vars->{user} = $user;
	
	return $TPL_MAIN;
}

sub get_data_queries {
	my ($self, $days, $limit, $user) = @_;

	my $dbh = $self->get_dbh();

	my $where = '';
	$where = 'AND search_bruker = ? ' if (defined($user));
	my $sth = $dbh->prepare(
	'SELECT query, COUNT(*) AS num FROM search_logg '.
	'WHERE tid > DATE_SUB(NOW(), INTERVAL ? DAY) ' . $where .
	'GROUP BY trim(query) ORDER BY num DESC LIMIT '. $limit);
	my $data = '';

	if (defined $user) {
		$sth->execute($days, $user);
	} else {
		$sth->execute($days);
	}

	my @a;
	while ((my $r = $sth->fetchrow_arrayref)) {
		push @a, $r->[0] . ";" . $r->[1];
	}

	return join("\n", reverse @a);

}

sub get_users_stat {
	my ($self, $days, $limit) = @_;

	my $dbh = $self->get_dbh();

	my $sth = $dbh->prepare(
	'SELECT search_bruker AS user, COUNT(*) AS num FROM search_logg '.
	'WHERE tid > DATE_SUB(NOW(), INTERVAL ? DAY) ' .
	'GROUP BY user ORDER BY num ASC LIMIT '. $limit);
	my $data = '';

	$sth->execute($days);

	while ((my $r = $sth->fetchrow_arrayref)) {
		$data .= $r->[0] . ";" . $r->[1] . "\n";
	}

	return $data;

}

sub get_users {
	my ($self, $days) = @_;

	my $dbh = $self->get_dbh();
	my $sth = $dbh->prepare(
	'SELECT search_bruker AS user, COUNT(*) AS num ' .
	'FROM search_logg ' .
	'WHERE tid > DATE_SUB(NOW(), INTERVAL ? DAY) ' .
	'GROUP BY user ORDER BY num DESC'
	);
	$sth->execute($days);
	my @users;
	while ((my $r = $sth->fetchrow_hashref)) {
		push @users, {	user => $r->{user},
				num => $r->{num}, 
		};
	}

	return \@users;
}

sub get_searches_day {
	my ($self, $days, $user) = @_;

	my $dbh = $self->get_dbh();

	my $where = '';
	$where = 'AND search_bruker = ? ' if (defined($user));
	my $sth = $dbh->prepare(
		'SELECT DATE(tid) AS date, COUNT(*) AS num '.
		'FROM search_logg '.
		'WHERE tid > DATE_SUB(NOW(), INTERVAL ? DAY) '. $where .
		'GROUP by date'
	);
	if (defined $user) {
		$sth->execute($days, $user);
	} else {
		$sth->execute($days);
	}

	my $data = '';
	my $dt = DateTime->now()->subtract(days => $days);

	sub md {
		my $dt = shift;
		sprintf("%s. %d.", $dt->month_abbr, $dt->day);
	}

	while ((my $r = $sth->fetchrow_arrayref)) {
		$data .= md($dt).";0\n" while $dt->add(days => 1)->ymd lt $r->[0];
		$data .= md($dt) . ";" . $r->[1] . "\n";
	}

	return $data;
}

sub get_crawled_docs {
	my ($s, $id, $sid) = @_;

	my $sqlShares = Sql::Shares->new($s->{dbh});
	my $collection = $sqlShares->get_collection_name($id);
	my $iq = Boitho::Infoquery->new($CONFIG{'infoquery'});
	my $sd = Sql::SessionData->new($s->{dbh});

	my %r = $sd->get($sid);
	my $collection_name = $sqlShares->get_collection_name($id);

	my $docs = $r{data};
	$docs = '' unless defined $docs;
	my %a = map { my @v = split(/=/, $_, 2); $v[0] => $v[1]; } split(/#/, $docs);

	my $newdocs = $iq->documentsInCollection($collection_name);

	my @points;
	my $cur = time;
	$a{$cur} = $newdocs;
	my @out;
	my $dt = DateTime->now()->subtract(seconds => 30);
	my $dt2 = DateTime->now()->subtract(seconds => 30);
	my $itr = 0; 
	my $last = undef;
	my $first = undef;
	
	for my $k (sort keys %a) {
		next if ($k < $dt2->epoch());
		push @out, $dt->hms() . ";0" while not defined $first and $dt->add(seconds=>1)->epoch() < $k;
		$first = 1;

		my $v = $a{$k};
		my $v2 = $v;
		push @points, "$k=$v";

		if (not defined $last) {
			#warn "Found first last: $v\n";
			$last = $v;
			$v = 0;
			next;
		} else {
			$v -= $last;
		}
		#warn "Last: $last v: $v2 diff: $v\n";

		#warn "f $k => ".$v."\n";
		push @out, POSIX::strftime('%H:%M:%S', localtime($k)) . ";" . $v ;
		$last = $v2 if ($v2 > $last);
	}
	$sd->update($sid, data => join('#', @points));


	return join("\n", @out);
}

1;
