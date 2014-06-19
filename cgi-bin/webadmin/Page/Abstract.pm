# Class: Page::Abstract
# Function for Page classes. Should be inhereted, not used directly.
package Page::Abstract;
use strict;
use warnings;

BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Carp;
use Data::Dumper;
use Sql::Sql;
use CGI qw(header);
use CGI::State;
use Template;
use Template::Stash;
use config qw(%CONFIG);
use Sql::ActiveUsers;
use Sql::Config;

my %DEF_TPL_OPT = (
    INCLUDE_PATH => './templates:./templates/common',
    #ANYCASE => 1,
    COMPILE_EXT => 'ctt',
    COMPILE_DIR => $CONFIG{tpl_tmp},
);
my %TPL_SCALAR_FUNC = (
    escape => sub { "\Q$_[0]\E" },
    cgi_escape => sub { CGI::escape($_[0]) },
);

my $dbh;
my $sqlActive;
my $sqlCfg;

# Constructor: new
#
# Attributes:
#	@ - Anything, sent to the childclass' _init function.
sub new {
    my $class = shift;
    my $d = $dbh || Sql::Sql->new->get_connection();

    my $cgi = CGI->new;
    my $self =  bless {
        dbh => $d,
        cgi => $cgi,
        state => CGI::State->state($cgi),
    }, $class;

    if ($self->can("_init")) { 
        $self->_init(@_);
    }

	$sqlActive ||= Sql::ActiveUsers->new($d);
	$sqlCfg ||= Sql::Config->new($d);

    return $self;
}

sub get_dbh { $_[0]->{dbh} }
sub get_state {
	my $s = shift;
	my $state = $s ? $s->{state} : CGI::State->state(CGI->new);
	return wantarray ? %{$state} : $state;
}
sub get_cgi { $_[0] ? $_[0]->{cgi} : CGI->new }

sub process_tpl {
    my ($s, $tpl_file, $vars_ref, %opt) = @_;
    croak "No tpl_file provided"
        unless defined $tpl_file;

    #warn("tpl_file: $tpl_file\nvars_ref:\n" . Dumper(  $vars_ref  ) . "opt:\n" . Dumper(\%opt));

    # Init tpl
    while (my ($key, $val) = each %DEF_TPL_OPT) {
        $opt{$key} = $val 
            unless defined $opt{$key};
    }
    while (my ($name, $sub)  = each %TPL_SCALAR_FUNC) {
        $Template::Stash::SCALAR_OPS->{$name} = $sub;
    }

    my @extra_folders;
    if (ref $opt{tpl_folders} eq 'ARRAY') {
        @extra_folders = @{$opt{tpl_folders}};
    }
    else {
        @extra_folders = ($opt{tpl_folders})
            if defined $opt{tpl_folders};
    }
    delete $opt{tpl_folders};

    $opt{INCLUDE_PATH} .= join ":./templates/", ('', @extra_folders)
        if scalar @extra_folders;

	# Add values always needed
	$vars_ref->{header_active_users} = $sqlActive->num_active();
	$vars_ref->{header_license_system_set} = $sqlCfg->get_setting("licensesystem") ? 1 :0;

    # Print html.
    my $no_header = $opt{no_header} and delete $opt{no_header};

    my $output = $opt{html_output};
    delete $opt{html_output};

    my @tpl_params = ($tpl_file, $vars_ref);
    my $tpl = Template->new(%opt);
    if (defined $output) {
        push @tpl_params, $output;
    }
    else {
        print header(-type => "text/html", -charset => "UTF-8")
            unless $no_header;
    }
    $tpl->process(@tpl_params)
        or croak "Template '$tpl_file' error: ", $tpl->error();
    
    1;
}

1;
