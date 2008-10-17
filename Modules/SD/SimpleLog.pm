package SD::SimpleLog;
use strict;
use warnings;

sub new { 
    my ($class, $logfile, $enabl) = @_;
    $enabl = 1 unless defined $enabl;

    my $succs = open my $logh, ">>", $logfile;
    if (!$succs) {
        warn "Unable to write to logfile: $!. Logging disabled.";
        $enabl = 0;
    }

    bless { 
        to_stdout => 0,
        enabled => $enabl,
        logh => $logh,
    }, $class;
}

sub write {
    my ($s, @data) = @_;

    return unless $s->{enabled};

    my $time = localtime(time);    
    my $msg = "$time - " . join (q{}, @data) . "\n";
    print $msg if $s->{to_stdout};
    print {$s->{logh}} $msg;
    1;
}

sub show_in_stdout { 
    my ($s, $on) = @_;
    $s->{to_stdout} = $on;
    $s;
}

1;
