#!/usr/bin/env perl
use strict;
use warnings;
use XML::Simple;
use Carp;
use Data::Dumper;
use YAML;
use Log::Log4perl;
use Readonly;
use Date::Format;

BEGIN {
    push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec);

Readonly::Scalar my $STRIP_QUOTATIONS => 1;
Readonly::Scalar my $DB_NAME => "boithoweb";
Readonly::Array my @RAND_CHRS => ('a'..'z', 0..9, 'A'..'Z');
Readonly::Hash my %DB_LAYOUT => (
        sUser =>  'bruker_navn',
        sPassword => 'passord',
        sRealname => 'navn',
        sSecretQuestion => 'hemmelig_sporsmaal',
        sSecretAnswer => 'hemmelig_svar',
        nCreateDate => 'opprettet',
        nLastLoginDate => 'sist_login',
        nStatus => 'status',
        setProps => 'egenskaper'
    );
    
Readonly::Hash my %IGNORED_ELEMENTS => (
        nTrialExpireDate => 1,
        id => 1,
        iType => 1,
        UserAccount => 1,
        iMaxAlerts => 1,
        setApplications => 1,
        iAutoLogin => 1,
    );


# init log
Log::Log4perl::init('log4perl.conf');
my $log = Log::Log4perl->get_logger('jayde.import.user');

my $dbh;

main();

sub main {
    my @files = @ARGV;

    show_usage() if not @files;

    # Sanity check.
    foreach my $file (@files) {
        unless (-f $file) {
            print STDERR "$file is not a file\n";
            show_usage();
        }
    }

    connect_to_db();

    # parse contents to db.
    foreach my $file (@files) {
        my $record_open;
        
        open my $fh, "<", $file
            or die "unable to open file $file, ", $!;
        
        my @record;
        while (my $line = <$fh>) {
            if ($line =~ /^\s+<UserAccount/) {
                $record_open = 1;
                undef @record;
            }
            if ($line =~ /^\s+<\/UserAccount/) {
                $record_open = 0;
                push @record, $line;

                my %userdata;
                eval {
                    %userdata = %{ XMLin(join q{}, @record)  };
                };
                if ($@) {
                    $log->error("Unable to parse user record, error: ", $@, 
                        " Data: ", join(q{}, @record));
                    next;
                }
                add_user_record(%userdata);
                next;
            }
       
            push @record, $line;
        }
    }

    $log->info("Done parsing ", scalar @files, " documents.");
    1;
}

sub strip_quotations {
    my $str = shift;
    return $str unless $str;
    $str =~ s/^"//;
    $str =~ s/"$//;
    return $str;
    1;
}

sub show_usage {
    die "Usage: jayde_user_import.pl file1.xml [file2.xml ...]\n";
}

# Adds user record to DB.
sub add_user_record {
    my %user_data = @_;
    #$log->debug(Dumper(\%user_data));
    $log->info("Got user record for user ", $user_data{sUser});

    if (!$user_data{sUser}) {
        $log->fatal("Got a record without username.", "User data: ", Dumper(\%user_data));
    }


    # Transform data to suit our db-setup.
    my %transformed; 
    while (my ($key, $value) = each %user_data) {

        unless (defined $DB_LAYOUT{$key}) {
            next if $IGNORED_ELEMENTS{$key};
            $log->warn("Ignoring unknown element $key.");
            next;
        }

        my $new_key = $DB_LAYOUT{$key};
        if ($key eq "setProps") {
            next unless ref $value eq 'HASH';
            next unless $value->{'array-element'};

            $value = $value->{'array-element'};
            if ($STRIP_QUOTATIONS) {
                for my $i (0..scalar @{$value}) {
                    $value->[$i] = strip_quotations($value->[$i]);
                }
            }
            my %data = (
                    account_notes => $value->[0],
                    company_name  => $value->[2],
                    address1      => $value->[3],
                    address2      => $value->[4],
                    city          => $value->[5],
                    state         => $value->[6],
                    zip           => $value->[7],
                    country       => $value->[8],
                    phone         => $value->[9],
                    referrer      => $value->[16],
                    verified_date => $value->[27],
                    verified_ip   => $value->[28],
                    );
            $transformed{mail} = $value->[1];
            $transformed{$new_key} = Dump(\%data);
        }

        elsif ($key =~ /^nCreateDate$|^nLastLoginDate$/) {
            $transformed{$new_key} = time2str("%Y-%m-%d", $value);
        }

        else {

            if (ref $value) {
                $transformed{$new_key} = Dump($value);
            }
            else {
                $value = strip_quotations($value)
                    if $STRIP_QUOTATIONS;
                $transformed{$new_key} = $value;
            }
        }
    }

    # User also needs confirm and ban code.
    $transformed{ban_code} = rand_str(6);
    $transformed{confirm_code} = rand_str(6);

    #$log->debug(Dumper(\%transformed));
    add_to_db(%transformed);
}

sub rand_str {
    my $len = shift;
    my $rand_str;
    $rand_str .= @RAND_CHRS[rand @RAND_CHRS] for 1..$len;
    return $rand_str;
}



##
# Helper method to add a record to db.
sub add_to_db {
    my %data = @_;

    if (user_exists($data{bruker_navn})) {
        $log->warn("User $data{bruker_navn} conflicts with a user already in database. Ignoring.");
        return;
    }

    my @query = ("INSERT INTO brukere (", join(",", keys(%data)), ")",
                    " VALUES(", '?,' x (scalar(values %data) - 1), "?)");
    my $query = join q{}, @query;
    $log->debug("query: ", $query);
    sql_exec($dbh, $query, values %data);
    $log->debug("Done inserting ", $data{bruker_navn}, " to db.");
    1;
}

##
# Checks if given username exists in db
sub user_exists {
    my $user = shift;
    my $query = "SELECT bruker_navn FROM brukere
                    WHERE bruker_navn = ?";
    my $sth = sql_exec($dbh, $query, $user);
    
    return ($sth->fetchrow_array) ? 1 : 0;
}

##
# Connect to db.
sub connect_to_db {
    my %setup = sql_setup();
    $setup{database} = $DB_NAME;
    $dbh = get_dbh(%setup);
    1;
}

1;
