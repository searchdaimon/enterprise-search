package Crawler;
use strict;
use warnings;
use SD::Crawl;
use Data::Dumper;
use Carp;

sub add_document {
    my ($self, %params) = @_;

    # set defaults
    $params{acl_denied} ||= "";
    if (!$params{last_modified}) {
        warn "last_modified not provided, defaulting to 0";
        $params{last_modified} = 0;
    }
    $params{type} = q{}
        unless defined $params{type};

    # validate
    confess "parameter 'last_modified' needs to be unixtime."
        if $params{last_modified} !~ /^\d+$/ || $params{last_modified} < 0;

    for (qw(url content title type)) {
        confess "parameter '$_' is missing"
            unless defined $params{$_};
    }

    # run
    return SD::Crawl::pdocumentAdd($self->{ptr},
        $params{url},
        $params{last_modified},
        $params{content},
        $params{title},
        $params{type},
        $params{acl_allow},
        $params{acl_denied},
        $params{attributes} || ""
    );
}

sub document_exists {
    my ($self, $url, $last_modified, $size_bytes) = @_;


    $size_bytes ||= 0;
    croak "Second parameter (last modified) is missing."
        unless defined $last_modified;
    croak "Second parameter (last modified) needs to be unixtime."
        if $last_modified !~ /^\d+$/ || $last_modified < 0;

    return SD::Crawl::pdocumentExist($self->{ptr}, $url, $last_modified, $size_bytes);
}

1;

