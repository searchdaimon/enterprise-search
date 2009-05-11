
package Perlcrawl;
use Carp;
use Data::Dumper;
use strict;
use warnings;

use Crawler;
our @ISA = qw(Crawler);

use Parse::MediaWikiDump;

##
# Main loop for a crawl update.
# This is where a resource is crawled, and documents added.
sub crawl_update {
	my (undef, $self, $opt) = @_;

	my $file = $opt->{file};
	my $n_docs = int($opt->{numdocs});


	my $pages = Parse::MediaWikiDump::Pages->new($file);
	my $page;

	my $cur_docs = 0;
	while (defined ($page = $pages->page) and $cur_docs < $n_docs) {
		next unless $page->namespace eq '';        
		my $namespace = $page->namespace;
		my $id = $page->id;
		my $title = $page->title;
		my $text = $page->text;

		my $url = "wikipedia-en:$title";



#		my @lines = split(/\n/, $$text);
#		$text = undef;
#		foreach my $i

		my $size = length($$text);
		my $last_modified = 12346561;
		if (not $self->document_exists($url, $last_modified, $size)) {
			$self->add_document((
						type      => 'txt',
						content   => $$text,
						title     => $title,
						url       => $url,
						acl_allow => "Everyone", # permissions
						last_modified => $last_modified, # unixtime
						attributes => 'source=enwiki', # key1=value1,key2=value2
					    ));
			$cur_docs++;
		}
	}
};

sub path_access {
    my ($undef, $self, $opt) = @_;
    
    # During a user search, `path access' is called against the search results 
    # before they are shown to the user. This is to check if the user still has
    # access to the results.
    #
    # If this is irrelevant to you, just return 1.

    # You'll want to return 0 when:
    # * The document doesn't exist anymore
    # * The user has lost priviledges to read the document
    # * .. when you want the document to be filtered from a user search in general.

    return 1;
}

1;
