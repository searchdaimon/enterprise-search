use strict;
use warnings;

use CGI::Carp qw(fatalsToBrowser warningsToBrowser);
use Readonly;
use URI::Escape qw(uri_escape);
use CGI::State;
use LWP::Simple qw(get);
use Data::Dumper;
use I18N::SearchRes;
use Template;
use CGI qw(escapeHTML);
use LWP::Simple qw(get);
use MIME::Base64 qw(encode_base64 decode_base64);
use config qw(%TPL_FILE %CFG @SEARCH_ENV_LOGGING %DEF_TPL_OPT);
use ResultParse::SDOld;
use NavMenu qw(read_navmenu_cfg);

BEGIN {
	CGI::Carp::set_message(" ");
	{
		# We want warnings to be shown in browser.
		# TODO: Redefining _warn is not a good way to do this,
		# 	neither is calling fatalsToBrowser
		no warnings; # So it doesn't warn about being redefined.
		*CGI::Carp::_warn = sub { fatalsToBrowser("Warning: " . shift) };
	}
};

my $cgi = new CGI();
my %state = %{CGI::State->state($cgi)};

my %query_params; # holds GET params that should
	# be passed along as user navigates
my ($tpl, $tpl_name) = init_tpl();

my $tpl_file;
my %tpl_vars;

our $isanonymous;

if (defined(my $query = $state{query})) {
	my $page = $state{page};
	if (!$page || $state{page} !~ /^\d+$/) {
		$page = 1;
	}
	my $show_xml = $state{debug} && $state{debug} eq "showxml";
	($tpl_file, %tpl_vars) = show_search($query, $show_xml, $page, $isanonymous, $tpl_name);
}
elsif (exists $state{cache}) {
	($tpl_file, %tpl_vars) = show_cache(map { $_ => $state{$_} } qw(signature time document collection host));
}
else {
	($tpl_file, %tpl_vars) = show_main();
}

print_html($tpl_file, %tpl_vars);

sub print_html {
	my ($tpl_file, %tpl_vars) = @_;
	
	$tpl_vars{query_params} = \%query_params;
	$tpl_vars{gen_cache_url} = \&gen_cache_url;

	print $cgi->header(
	        -type => "text/html", 
	        -charset => "UTF-8",
	);
	warningsToBrowser();
	$tpl->process($tpl_file, \%tpl_vars)
		or croak "Template '$tpl_file' error: ", $tpl->error(), "\n";
}

sub fatal {
	my @err = @_;
	#carp @err;
	if ($tpl) {
		print_html($TPL_FILE{error}, ( errors => \@err, query => $state{query} ));
	}
	else {
		croak Dumper(\@err);
	}
	exit 1;
}
	

sub show_main {
	return $TPL_FILE{main};
}

sub show_search {
	my ($query, $show_debug, $page, $anonymous, $tpl_name) = @_;

	my $navmenu_cfg = encode_base64(read_navmenu_cfg($tpl_name));

	my $search_uri = gen_search_uri(query => $query, page => $page, anonymous => $anonymous, navmenucfg => $navmenu_cfg);
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
			return $path; #if -e $path;
			#warn "Icon '$path' does not exist.";
			return q{};
		},
		page_nav => {
			show_pages  => $CFG{page_nav}->{show_pages},
			num_results => $CFG{num_results},
			page        => $page,
		},
	));
}

sub show_cache {
	my %cache_params = @_;
	for (qw(signature time document collection host)) {
		fatal("Parameter '$_' missing")
			unless defined $cache_params{$_};
	}
	for (qw(signature time document)) {
		fatal("Invalid value for '$_'")
			unless $cache_params{$_} =~ /^\d+$/;
	}

	fatal("Invalid value for 'host'")
		unless $cache_params{host} =~ /^([a-z0-9]|\.)+$/i;
	
	my $uri = sprintf $CFG{cache_uri_tpl}, map({ uri_escape($_) }
		$cache_params{host},
		$cache_params{signature},
		$cache_params{'time'},
		$cache_params{document},
		$cache_params{collection});

	my $cache_data;
	eval {
		local $SIG{ALRM} = sub { 
			die "cache download exceeded timeout ", $CFG{cache_timeout} 
		};
		alarm $CFG{cache_timeout};
		$cache_data = get($uri);
		alarm 0;
	};
	if ($@) {
		#carp $@;
		$@ =~ /timeout/ 
			? fatal("Unable to load cache, cache server timed out.")
			: fatal("Unable to load cache, unknown error.");
	}
	fatal("Unable to load cache, cache server provided no data.")
		unless defined $cache_data;

	return ($TPL_FILE{cache}, cache_data => $cache_data);
}

sub gen_search_uri {
	my %attr = @_;

	croak "Query missing"
		unless defined $attr{query};
	croak "Result page missing"
		unless defined $attr{page};

	# Add defaults
	$attr{lang}     ||= $query_params{lang} || $CFG{lang};
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
	$attr{userip} = $ENV{REMOTE_ADDR};
	delete $attr{REMOTE_ADDR};
	$attr{AmazonAssociateTag} = "";
	$attr{AmazonSubscriptionId} = "";
	$attr{start} = $attr{page};
	delete $attr{page};

	
	# Anonymous search
	if ($attr{anonymous}) {
		delete $attr{search_bruker};
		$attr{anonymous} = '1';
	}


	#warn "searchd params: ", Dumper(\%attr);

	# Generate URI
	my $param_str = join "&", map { 
		my $p = "$_=" . uri_escape($attr{$_} || "");
	} keys %attr;

	return $CFG{search_uri} . "?" . $param_str;
}

sub init_tpl {
	
	# select template
	my $tpl_name = $state{tpl};
	if ($tpl_name) {
		croak "Invalid template '$tpl_name'"
			unless $tpl_name =~ /^[a-zA-Z_\-0-9]+$/ && (-d "tpl/$tpl_name");
		$query_params{tpl} = $tpl_name;
	}
	else { $tpl_name = $CFG{tpl} }

	# select language
	my $lang;
	if ($lang = $state{lang}) {
		if (!valid_lang($tpl_name, $lang)) {
			croak "Invalid language " . $state{lang};
		}
		$query_params{lang} = $lang;
	}
	elsif ($ENV{HTTP_ACCEPT_LANGUAGE}) {
		my @browser_lang = split q{,}, $ENV{HTTP_ACCEPT_LANGUAGE};
		my $valid_lang;
		for my $l (@browser_lang) {
			$l  =~ s/-/_/g;
			$l =~ s/;.*$//;
			if (valid_lang($tpl_name, $l)) {
				$valid_lang = $l;
				last;
			}
		}
		if ($valid_lang) {
			$lang = $valid_lang;
			$query_params{lang} = $lang;
		}
		else { 
			$lang = valid_lang($tpl_name, $CFG{lang}) 
				? $CFG{lang} : undef;
		}
	}
	else { 
		$lang = valid_lang($tpl_name. $CFG{lang}) 
			? $CFG{lang} : undef;
	}

	# init translation
	my $i18n_filter;
	if (defined $lang) {
		my @langs = $lang eq $CFG{lang} 
			? ($lang) 
			: ($lang, $CFG{lang});
		I18N::SearchRes::load_lang($tpl_name);
		my $i18n = I18N::SearchRes->get_handle(@langs)
			or fatal("Couldn't make a language handle");
		$i18n->fail_with(sub { 
			shift; 
			warn "No locale for: ", join(", ", @_);
			return 'I18N_error';
		});

        	$i18n_filter = sub {
			#warn "Params: ", Dumper(\@_);
			$i18n->maketext(@_);
		}; 
	}
	else {
		# Support templates with no translations
		$i18n_filter = sub { shift } 
	}

	# tpl instance, with filter and vmethod
        $Template::Stash::SCALAR_OPS->{i18n} = $i18n_filter;
	$Template::Stash::SCALAR_OPS->{query_url} = \&gen_query_url;
	my (undef) = $Template::Stash::SCALAR_OPS->{i18n}; # rm warning
	my (undef) = $Template::Stash::SCALAR_OPS->{query_url};


	my %opt = %DEF_TPL_OPT;
	$opt{INCLUDE_PATH} .= $tpl_name;
	$opt{FILTERS}->{i18n}   = $i18n_filter;
	$opt{FILTERS}->{warn}   = sub { warn "Template: ", @_; return q{}; };
	$opt{FILTERS}->{strong} = sub { "<strong>" . $_[0] . "</strong>" };
	$opt{FILTERS}->{query_url} = \&gen_query_url;
	#$opt{FILTERS}->{cache_url} = \&gen_cache_url;

	my $tpl = Template->new(%opt)
		|| croak $tpl->error();
	return ($tpl, $tpl_name);
}

sub _gen_url {
	my $base_url = shift;
	while (my ($k, $v) = each %query_params) {
		$base_url .= "&$k=$v"
	}
	return escapeHTML($base_url);
}

sub gen_query_url {
	my $query = shift;
	return _gen_url("?query=$query");
}

sub gen_cache_url {
	my $cache_params = shift;
	my $base_url = "?cache";
	while (my ($k, $v) = each %{$cache_params}) {
		$base_url .= "&$k=$v"
	}
	return _gen_url($base_url);
}

sub valid_lang { 
	croak "missing param" unless @_ >= 2;
	my ($tpl_name, $lang) = @_;
	return unless defined $lang;
	return $lang =~ /^[a-z_]+$/ && (-d "./locale/$tpl_name/$lang");
}

1;
