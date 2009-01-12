#!/usr/bin/env perl

use strict;
use warnings;

use IO::String;
use MIME::Base64;
use XML::Writer;
use LWP::UserAgent;
use HTTP::Request;

my $xmlversion = "1";

sub xmlpush_start {
	my ($key, $output) = @_;
	my ($writer);

	${ $output } = "";
	$writer = new XML::Writer(DATA_INDENT => 4, DATA_MODE => 1, OUTPUT => IO::String->new(${ $output }));
	$writer->xmlDecl('utf-8');

	$writer->startTag('sddocument');

	$writer->startTag('version');
	$writer->characters($xmlversion);
	$writer->endTag();

	$writer->startTag('key');
	$writer->characters($key);
	$writer->endTag();

	return $writer;
}

sub xmlpush_end {
	my ($writer) = @_;

	$writer->endTag(); # Should be sddocument
	$writer->end(); # Close up the shop
}

sub xmlpush_add($$$$$$$$$$$) {
	my ($writer, $dformat, $dtype, $title, $uri, $lastmodified, $collection, $aclallow, $acldeny, $body, $attributes) = @_;


	$writer->startTag('add');

	$writer->startTag('documents');

	$writer->startTag('document');
	$writer->startTag('documentformat');
	$writer->characters($dformat);
	$writer->endTag();
	$writer->startTag('documenttype');
	$writer->characters($dtype);
	$writer->endTag();
	$writer->startTag('title');
	$writer->characters($title);
	$writer->endTag();
	$writer->startTag('lastmodified');
	$writer->characters($lastmodified);
	$writer->endTag();
	$writer->startTag('collection');
	$writer->characters($collection);
	$writer->endTag();
	$writer->startTag('uri');
	$writer->characters($uri);
	$writer->endTag();
	$writer->startTag('aclallow');
	$writer->characters($aclallow);
	$writer->endTag();
	$writer->startTag('acldeny');
	$writer->characters($acldeny);
	$writer->endTag();
	$writer->startTag('attributes');
	$writer->characters($attributes);
	$writer->endTag();
	$writer->startTag('body', encoding => 'base64' );
	$writer->characters(encode_base64($body));
	$writer->endTag();
	$writer->endTag();

	$writer->endTag(); # documents

	$writer->endTag(); # sdadd
}

sub xmlpush_close($$) {
	my ($writer, $collection) = @_;

	$writer->startTag('close');
	$writer->startTag('collection');
	$writer->characters($collection);
	$writer->endTag();
	$writer->endTag();
}

sub xmlpush_post($$) {
	my ($url, $xml) = @_;

	my $ua = new LWP::UserAgent;
	my $req = HTTP::Request->new(POST => $url);
	$req->content($xml);
	$req->header('Content-type' => 'text/xml');

	my $response = $ua->request( $req );

	if ($response->is_success) {
		return $response->content;
	} else {
		warn $response->status_line;
		return undef;
	}
}

use Data::Dumper;

sub xmlpush_users($$@) {
	my ($writer, $usersystem, @users) = @_;

	$writer->startTag('users', 'usersystem' => $usersystem);
	$writer->startTag('dropusers');
	$writer->endTag();
	foreach my $user (@users) {
		$writer->startTag('user', 'username' => $user->{'username'} );
		foreach my $group (@{ $user->{'groups'} }) {
			$writer->startTag('group');
			$writer->characters($group);
			$writer->endTag();
		}
		$writer->endTag();
	}
	$writer->endTag();
}

my $output;
my $writer = xmlpush_start('akey', \$output);


xmlpush_users($writer, 1, {'username' => 'rb', 'groups' => ['a', 'b', 'c'] } );

xmlpush_add($writer, 'text', 'txt', 'atitle', 'http://www.pvv.ntnu.no', '2008-05-04', 'test', 'ean', 'rb', 'Eirik!', 'a=b,c=d');
xmlpush_close($writer, 'test');

xmlpush_end($writer);

print "XML::\n" . $output. "\n\n";

print "Unable to push document\n" if not defined xmlpush_post("http://eirik.boitho.com/cgi-bin/bbdocumentWebAdd", $output)
