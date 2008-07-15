package Perlcrawl;
use strict;
use warnings;

use Crawler;
our @ISA = qw(Crawler);

use Carp;
use JSON::XS qw(decode_json);
use Data::Dumper;
use LWP::Simple qw(get);


sub crawl_update {
    my (undef, $self) = @_;

    my $t = decode_json(
            get("http://twitter.com/statuses/public_timeline.json"));

    for my $usr (@{$t}) {
        my $content = $usr->{text};
        my $url = "http://twitter.com/" . "$usr->{user}{screen_name}/statuses/$usr->{id}";
        my $substr = substr($content, 0, 50);
        my $title = "$usr->{user}{name}: $substr ..";
     
        next if $self->document_exists($url, 0);

        print "Adding $title\n";
        $self->add_document((
            content   => $content,
            title     => $title,
            url       => $url,
            type      => "txt",
            acl_allow => "Everyone",
            last_modified => time(),
       ));
        sleep 1;
    }

    print "Twitter crawl done.\n"
}

sub path_access {
    my (undef, $self, $opt) = @_;

    
}

1;
