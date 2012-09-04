package Page::Apiinfo;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use File::Path;
use Page::Abstract;
use Common::TplCheckList;
use Params::Validate qw(validate_pos OBJECT);
use Sql::Config;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_SEARCH => "apiinfo_search.html";
use constant TPL_SEARCH_STAGE_2 => "apiinfo_search_stage_2.html";
use constant TPL_SEARCH_STAGE_3 => "apiinfo_search_stage_3.html";
use constant TPL_PUSH => "apiinfo_push.html";

my $sqlConf;
sub _init {
	my ($self) = @_;

	$sqlConf = Sql::Config->new($self->{dbh});
	return $self;
}

sub show_search_select {
	my ($s, $vars) = @_;

	return TPL_SEARCH;
}

sub show_select_mode {
    my ($s, $vars, $searchmode) = @_;

    $vars->{searchmode} = $searchmode;

    return TPL_SEARCH_STAGE_2;
}

sub show_push {
	my ($s, $vars) = @_;

	my $key = $sqlConf->get_setting('key');
	if (!$key) { carp "Can't lookup key"}
	$vars->{key} = $key;

    return TPL_PUSH;

}
sub show_generate_url {
	my ($s, $vars, $searchmode, $outformat, $query, $username) = @_;

	my $key = $sqlConf->get_setting('key');
	if (!$key) { carp "Can't lookup key"}
	$vars->{key} = $key;

	$vars->{query} = $query;

	if ($searchmode eq "forward_auth" && $outformat eq 'opensearch') {
		$vars->{path} = "/webclient2/api/opensearch/1.1/search?query=$query";
	}
	elsif ($searchmode eq "forward_auth" && $outformat eq 'nativexml') {
		$vars->{path} = "/webclient2/api/sd/2.1/search?query=$query";
	}
	elsif ($searchmode eq "anonymous" && $outformat eq 'opensearch') {
		$vars->{path} = "/webclient2/api/anonymous/opensearch/1.1/search?query=$query";
	}
	elsif ($searchmode eq "anonymous" && $outformat eq 'nativexml') {
		$vars->{path} = "/webclient2/api/anonymous/sd/2.1/search?query=$query";
	}
	elsif ($searchmode eq "pre_auth" && $outformat eq 'opensearch') {
		$vars->{path} = "/cgi-bin/dispatcher_allbb?query=$query&search_bruker=$username&bbkey=$key&outformat=opensearch";
	}
	elsif ($searchmode eq "pre_auth" && $outformat eq 'nativexml') {
		$vars->{path} = "/cgi-bin/dispatcher_allbb?query=$query&search_bruker=$username&bbkey=$key";
	}
	else {
		carp "Unknown type $searchmode -> $outformat";
	}

	return TPL_SEARCH_STAGE_3;

}



1;
