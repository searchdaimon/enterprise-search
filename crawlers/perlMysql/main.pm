use strict;

package Perlcrawl;
use SD::Crawl;

use DBI; #bruker DBI databse interfase


sub crawlupdate {
	
	my ($pointer ) = @_;	

	####################################################################
	# Settup
	####################################################################
	my $user = "boitho";
	my $Password = "G7J7v5L5Y7";
	my $server = "localhost";
	my $database = "sirdf_com";
	my $forumurl = "http://www.sirdf.com/forum";
	####################################################################

   	#Kobler til databasen
	my $dbh = DBI->connect("DBI:mysql:database=$database;host=$server;port=3306", 
                       $user, $Password) or die("Can`t connect: $DBI::errstr");


	#gjør klar sql spøring
	my $sth = $dbh->prepare("select topic_id,pid,post,title,author_name from forum_posts,forum_topics where forum_posts.topic_id = forum_topics.tid") or dienice("Can`t prepare statment: ", $dbh->errstr);
	my $rv = $sth->execute;

	#laster ned sql resultater
	while ((SD::Crawl::pdocumentContinue($pointer)) && (my ($topic_id, $pid, $post, $title, $author_name ) = $sth->fetchrow_array)) {
		print "topic_id: $topic_id, $title\n";

		#lager de elementene vi vil sende til søkesystemet.
		my $sd_url 		= "$forumurl/viewtopic.php?t=$topic_id#$pid";
		my $sd_size 		= length($post);
		my $sd_acl_allow 	= "Everyone";
		my $sd_acl_denied 	= "";
		my $sd_type 		= "hnxt";
		my $sd_title		= "$title ( by $author_name )";
		my $sd_doc		= $post;

		if (not SD::Crawl::pdocumentExist($pointer, $sd_url, 0, $sd_size )) {

			#dokumentet finnes ikke, så vi legger det til
			# pdocumentAdd() har format:
			# pdocumentAdd( x, url, lastmodified, dokument_size, document, title, acl_allow, acl_denied )
			SD::Crawl::pdocumentAdd($pointer, $sd_url, 0 ,$sd_size, $sd_doc, $sd_title, $sd_type, $sd_acl_allow, $sd_acl_denied);		

		}

	}
	
	print "Thank you for using SD Perl Crawler. Going away now.\n";

	SD::Crawl::pdocumentError($pointer,"aaa test");
}
