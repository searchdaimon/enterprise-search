package CrawlWatch::Config;
BEGIN { push @INC, "$ENV{BOITHOHOME}/Modules" }
use SD::Sql::ConnSimple qw(sql_fetch_single sql_exec);
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(bb_config_get bb_config_update);

# Read configvalue from DB.
sub bb_config_get {
    my ($dbh, $confkey) = @_;
    my $query = "SELECT configvalue FROM config
                    WHERE configkey = ?";
    my $val = sql_fetch_single($dbh, $query, $confkey);
    
    return $val;
}

##
# Update configvalue in db.
sub bb_config_update {
    my ($dbh, $confkey, $value) = @_;
    my $query = "UPDATE config
        SET configvalue = ? 
        WHERE configkey = ?";
    
    sql_exec($dbh, $query, $value, $confkey);
    1;
}

1;
