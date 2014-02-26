package Crawler;
use strict;
use warnings;
use SD::Crawl;
use Data::Dumper;
use Carp;
use threads;
use threads::shared;

my $t_lock :shared = 0;

sub add_document {
    my ($self, %params) = @_;

    # set defaults
    $params{acl_denied} ||= "";
    if (!$params{last_modified}) {
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
    if (defined $params{attributes}) {
	    confess "parameter 'attributes' should be a HASH, (got ", ref $params{attributes} || "SCALAR", ")"
		    unless ref $params{attributes} eq "HASH";

	    my $attrstr = q{};
	    while (my ($k, $v) = each %{$params{attributes}}) {
		    next unless defined $v;
		    next if $v eq '';

		    $v =~ s/[,=]/ /g; #remove all , and = because they are used as seperators.
		    $attrstr .= "$k=$v,";
	    }
	    chop($attrstr); #remove last ","
	    $params{attributes} = $attrstr;
    }

# run
    lock($t_lock); # Lock to prevent to threads to access the below at the same time
    return SD::Crawl::pdocumentAdd($self->{ptr},
        $params{url},
        $params{last_modified},
        $params{content},
        $params{title},
        $params{type},
        $params{acl_allow},
        $params{acl_denied},
        $params{attributes} || "",
        $params{image} || ""
    );
}

sub document_exists {
    my ($self, $url, $last_modified, $size_bytes) = @_;


    $size_bytes ||= 0;
    croak "document_exists(): Required second parameter (last modified) is missing."
        unless defined $last_modified;
    croak "document_exists(): Required second parameter (last modified) needs to be unixtime."
        if $last_modified !~ /^\d+$/ || $last_modified < 0;

    lock($t_lock); # Lock to prevent to threads to access the below at the same time
    return SD::Crawl::pdocumentExist($self->{ptr}, $url, $last_modified, $size_bytes);
}

sub normalize_http_url {
	my ($self, $url) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::htttp_url_normalization($url);
}

sub continue {
	my $self = shift;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::pdocumentContinue($self->{ptr});
}

sub change_collection {
	my ($self, $collection) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	SD::Crawl::pdocumentChangeCollection($self->{ptr}, $collection);
}

sub add_foreign_user {
	my ($self, $user, $group) = @_;

	croak "Need a user" unless defined $user;
	$group = $user unless (defined $group);

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::paddForeignUser($self->{ptr}, $user, $group);
}

sub remove_foreign_users {
	my ($self) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::premoveForeignUsers($self->{ptr});
}

sub closeCurrentCollection {
	my ($self) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	SD::Crawl::pcloseCurrentCollection($self->{ptr});
};

sub get_last_crawl_time {
	my ($self) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::pget_last_crawl($self->{ptr});
}

sub delete_uri {
	my ($self, $subname, $uri) = @_;

	lock($t_lock); # Lock to prevent to threads to access the below at the same time
	return SD::Crawl::pdeleteuri($self->{ptr}, $subname, $uri);
}

1;

