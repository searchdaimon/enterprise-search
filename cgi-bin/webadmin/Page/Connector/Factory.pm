package Page::Connector::Factory;
use strict;
use warnings;

use Readonly;
use Carp;
use Exporter qw(import);
use Params::Validate qw(validate_pos);

use Page::Connector::CrawlerUtils;
use Page::Connector::UserSysUtils;

# should probably be loaded on demand, but compile time errors are nice.
use Page::Connector::Crawler::Form;
use Page::Connector::Crawler::API;
use Page::Connector::UserSys::Form;
use Page::Connector::UserSys::API;

Readonly::Scalar our $CONN_CRAWLER => 1;
Readonly::Scalar our $CONN_USERSYS => 2;
our @EXPORT = qw($CONN_CRAWLER $CONN_USERSYS);

sub form {
	validate_pos(@_, 1, { regex => qr/^\d+$/ });
	my (undef, $type) = @_;
	
	return Page::Connector::Crawler::Form->new(Page::Connector::CrawlerUtils->new)
		if $type == $CONN_CRAWLER;

	return Page::Connector::UserSys::Form->new(Page::Connector::UserSysUtils->new)
		if $type == $CONN_USERSYS;

	croak "Unknown connector type '$type'";
}

sub api {
	validate_pos(@_, 1, { regex => qr/^\d+$/ }, { regex => qr/^\d+$/ });
	my (undef, $type, $conn_id) = @_;
	
	return Page::Connector::Crawler::API->new($conn_id, Page::Connector::CrawlerUtils->new)
		if $type == $CONN_CRAWLER;

	return Page::Connector::UserSys::API->new($conn_id, Page::Connector::UserSysUtils->new)
		if $type == $CONN_USERSYS;
	
	croak "Unknown connector type '$type'";
}

1;
