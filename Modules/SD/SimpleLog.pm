package SD::SimpleLog;
use strict;
use warnings;

my $logfile_path;
my $enabled;

sub new { 
    my ($class, $logfile, $enabl) = @_;

    ($logfile_path, $enabled) = ($logfile, $enabl);
    bless {}, $class;
}

sub write {
    my ($self, @data) = @_;

    return unless $enabled;

    my $success = open my $logh, ">>", $logfile_path;

    if ($success) {
        my $time = gmtime(time());
        print {$logh} "$time - ";
        print {$logh} join(q{}, @data);
        print {$logh} "\n";
    }
    else {
        warn "Unable to write to logfile: $!. Disabling logging.";
        $enabled = 0;
    }

    close $logh;
    1;
}

1;
