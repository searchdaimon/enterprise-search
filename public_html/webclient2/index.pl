#!/usr/bin/env perl
use strict;
use warnings;

use config qw(%CFG @SEARCH_ENV_LOGGING %VALID_TPL %TPL_FILE %VALID_LANG %DEF_TPL_OPT);
use ResultParse::SDOld;

use Carp;
use Readonly;
use URI::Escape qw(uri_escape);
use CGI::State;
use LWP::Simple qw(get);
use Data::Dumper;
use I18N::SearchRes;
use Template;

my $lang ||= $CFG{lang};
$lang = lc $lang;
$lang = "en_us" 
	if $lang eq "ENG"; # legacy;

my $cgi = new CGI();
my %state = %{CGI::State->state($cgi)};
my $tpl = init_tpl($state{tpl}, $lang);

fatal("Invalid language " . $lang)
	unless $VALID_LANG{$lang};

my $tpl_file;
my %tpl_vars;


if (defined(my $query = $state{query})) {
	my $page = $state{page};
	if (!$page || $state{page} !~ /^\d+$/) {
		$page = 1;
	}
	($tpl_file, %tpl_vars) = show_search($query, $state{debug}, $page, $lang);
}
else {
	($tpl_file, %tpl_vars) = show_main();
}

print_html($tpl_file, %tpl_vars);

sub print_html {
	my ($tpl_file, %tpl_vars) = @_;

	print $cgi->header(
	        -type => "text/html", 
	        -charset => "UTF-8",
	);
	$tpl->process($tpl_file, \%tpl_vars)
		or croak "Template '$tpl_file' error: ", $tpl->error(), "\n";
}

sub fatal {
	my @err = @_;
	carp @err;
	print_html($TPL_FILE{error}, ( errors => \@err, query => $state{query} ));
	exit 1;
}
	

sub show_main {
	return $TPL_FILE{main};
}

sub show_search {
	my ($query, $show_debug, $page, $lang) = @_;


	my $search_uri = gen_search_uri(query => $query, page => $page, lang => $lang);
	my $xml_str = get($search_uri)
		or fatal("No result from dispatcher.");

	if ($show_debug) {
		print CGI::header(-type => "text/xml", -charset => "UTF-8");
		print $xml_str;
		exit;
	}

	my $res = ResultParse::SDOld->new($xml_str, \&fatal, $query);

	
	fatal($res->errors()) 
		if $res->errors();

	return ($TPL_FILE{results}, (
		res_info     => $res->res_info($page),
		coll_info    => $res->collections(),
		results      => $res->results(),
		navigation   => $res->navigation(),
		sort_info    => $res->sort_info(),
		query        => $query,
		#filters      => $res->filters(),
		icon_url     => sub {
			my $icon = shift;
			my $path = "$CFG{icon}{dir}/\Q$icon\E.$CFG{icon}{ext}";
			return $path if -e $path;
			warn "Icon '$path' does not exist.";
			return q{};
		},
		page_nav => {
			show_pages  => $CFG{page_nav}->{show_pages},
			num_results => $CFG{num_results},
			page        => $page,
		},
	));
}

sub gen_search_uri {
	my %attr = @_;

	croak "Query missing"
		unless defined $attr{query};
	croak "Result page missing"
		unless defined $attr{page};

	# Add defaults
	$attr{lang}     ||= $CFG{lang};
	$attr{username} ||= $CFG{username};
	$attr{maxhits}  ||= $CFG{num_results};
	$attr{subname}  ||= $CFG{subname};
	$attr{secret}   ||= $CFG{secret};
	$attr{tkey}     ||= $CFG{tkey};
	#$attr{version} ||= $CFG{version};

	# Add env variables
	for my $e (@SEARCH_ENV_LOGGING) {
		$attr{$e} = $ENV{$e};
	}

	# Legacy ...
	$attr{search_bruker} = $attr{username};
	delete $attr{username};
	$attr{userip} = $attr{REMOTE_ADDR};
	delete $attr{REMOTE_ADDR};
	$attr{AmazonAssociateTag} = "";
	$attr{AmazonSubscriptionId} = "";
	$attr{start} = $attr{page};
	delete $attr{page};

	#warn "searchd params: ", Dumper(\%attr);

	# Generate URI
	my $param_str = join "&", map { 
		my $p = "$_=" . uri_escape($attr{$_} || "");
	} keys %attr;

	return $CFG{search_uri} . "?" . $param_str;
}

sub init_tpl {
	my ($tpl_name, $lang) = @_;
	$tpl_name ||= $CFG{tpl};
	$lang ||= $CFG{lang};

	# init translation

	my @langs = $lang eq $CFG{lang} 
		? ($lang) 
		: ($lang, $CFG{lang});
	my $i18n = I18N::SearchRes->get_handle(@langs)
		or fatal("Couldn't make a language handle");
	$i18n->fail_with(sub { 
		shift; 
		warn "Maketext lookup failure:", Dumper(\@_) 
	});

        my $i18n_filter = sub {
		#warn "Params: ", Dumper(\@_);
		$i18n->maketext(@_);
	}; 
	# tpl instance, with filter and vmethod
        $Template::Stash::SCALAR_OPS->{i18n} = $i18n_filter;
	my (undef) = $Template::Stash::SCALAR_OPS->{i18n}; # rm warning

	$VALID_TPL{$tpl_name} or fatal("Invalid template '$tpl_name'");

	my %opt = %DEF_TPL_OPT;
	$opt{INCLUDE_PATH} .= $tpl_name;
	$opt{FILTERS}->{i18n}   = $i18n_filter;
	$opt{FILTERS}->{warn}   = sub { warn "Template: ", @_; return q{}; };
	$opt{FILTERS}->{strong} = sub { "<strong>" . $_[0] . "</strong>" };

	my $tpl = $tpl = Template->new(%opt);
	return $tpl;
}

1;
