#!/usr/bin/env perl
use strict;
use warnings;

use Carp;
use Params::Validate;
use Readonly;
use SQL::Abstract;

BEGIN { unshift @INC, $ENV{BOITHOHOME} . "/Modules" }
use SD::Sql::ConnSimple qw(sql_exec get_dbh sql_setup);

Readonly::Scalar my $LOT_COUNT      => 4096;
Readonly::Scalar my $LOG_PATH_TPL   => "$ENV{BOITHOHOME}/lot/%d/%d/indexlog.txt";
Readonly::Scalar my $INDEXERLOT_LOG_REGEX 
	=> qr{^(\d+) new=(\d+),recrawled=(\d+),untouched=(\d+)$};

my %db_setup = sql_setup();
$db_setup{database} = "sd_stats";
my $dbh = get_dbh(%db_setup);

for my $lot (1..$LOT_COUNT) {
	my $path = gen_log_path($lot);
	print "Reading log $path\n";
	my $num_entries = 0;
	my $succs = read_lot_log($path, sub { 
		$num_entries++; 
		run_to_db($dbh, $lot, @_) 
	});
	unless ($succs) {
		print "Error during parse, skipping\n";
		next;
	}
	print "Done parsing $num_entries entires.\n";
	
	print "Truncating\n";
	open my $fh, ">", $path
		or croak "truncate '$path' ", $!;
}

sub read_lot_log {
	validate_pos(@_, 1, 1);
	my ($path, $data_func) = @_;

	my $fh;
	unless (open $fh, '<', $path) {
		warn "Unable to open '$path'.";
		return;
	}

	for my $index_run (<$fh>) {
		my @data = $index_run =~ /$INDEXERLOT_LOG_REGEX/;
		&$data_func(@data);
	}
	1;
}

sub run_to_db {
	my ($dbh, $lot, $unixtime, $new, $recrawled, $untouched) = @_;
	sql_exec($dbh, "INSERT INTO lot_indexing 
				(time, lot, new_crawls, recrawls, untouched)
			VALUES  (FROM_UNIXTIME(?), ?, ?, ?, ?)",
		$unixtime, $lot, $new,
		$recrawled, $untouched
	);
}

sub gen_log_path {
	validate_pos(@_, { regex => qr(^\d+$) });
	my $lot = shift;
	my $folder = $lot % 64;
	return sprintf $LOG_PATH_TPL, $folder, $lot;
}


1;
