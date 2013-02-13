package Page::Settings;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use File::Copy;
use config qw(%CONFIG);
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use SD::Settings::Export;
use SD::Settings::Import;
use SD::SLicense qw(license_info $DB_LICENSE_FIELD);
use Sql::Config;
use Sql::Shares;
use Sql::ShareGroups;
use File::Temp qw(tempfile);
BEGIN {
    eval {
        require Apache::Htpasswd;
    };
    if ($@) { 
        carp "Unable to load Apache::Htpasswd", 
            " user won't be able to change password.";
    }
}
use Page::Abstract;
our @ISA = qw(Page::Abstract);

use constant TPL_ADVANCED => "settings_advanced.html";
use constant PASSWD_STARS => "******";
use constant CFG_DIST_VERSION => 'dist_preference';

sub _init {
	my $self = shift;
	my $dbh = $self->{dbh};
	$self->{sqlConfig} = Sql::Config->new($dbh);

        croak "Repomod is not executable"
            unless -x $CONFIG{repomod_path};
}



sub update_settings {
	my ($self, $vars, $setting) = @_;
	my $sqlConfig = $self->{'sqlConfig'};
        
        my %starred = map { $_ => 1 } 
            @{$CONFIG{adv_starred_fields}};

	while (my ($key, $value) = each %{$setting}) {
            next if $starred{$key} 
                and $value eq PASSWD_STARS;

            $sqlConfig->update_setting($key, $value);
	}
	
	$vars->{'success_settings_update'} = 1;
	return $vars;
}

sub update_license {
	my ($s, $vars, $license) = @_;
	$s->{sqlConfig}->update_setting($DB_LICENSE_FIELD, $license);
	$vars->{upd_license_success} = 1;
	return $s->show_main_settings($vars);
}

sub confirmed_delete_settings($$) {
	my ($self, $vars) = @_;
	my $template_file = 'settings_delete_done.html';
	return $template_file;
}

sub show_confirm_dialog($$) {
	my ($self, $vars) = @_;
	return "settings_delete_all.html";
}

sub show_frontpage_settings {
	my ($self, $vars) = @_;
	my $template_file = "settings_frontpage.html";
	my $sqlConfig = $self->{'sqlConfig'};
	$vars->{'frontpage_preference'}
		= $sqlConfig->get_setting("frontpage_preference");


	return $template_file;
}

sub show_anostat_settings {
	my ($self, $vars) = @_;
	my $template_file = "settings_anostat.html";
	my $sqlConfig = $self->{'sqlConfig'};
	$vars->{'anostat_preference'}
		= $sqlConfig->get_setting("anostat_preference");


	return $template_file;
}

sub show_advanced_settings($$) {
	my ($self, $vars) = @_;

        my %settings = $self->{sqlConfig}->get_all();
        for my $key (@{$CONFIG{adv_starred_fields}}) {
            $settings{$key} = PASSWD_STARS;
        }

        my @default = @{$CONFIG{default_settings}};
        $_->{table_value} = $settings{$_->{table_key}}
            for @default;
        
        $vars->{all_settings} = \%settings;
        $vars->{default_settings} = \@default;

	return TPL_ADVANCED;
}

sub show_advanced_settings_updated($$) {
	my ($self, $vars) = @_;
	return $self->show_advanced_settings($vars);
}

sub show_import_export($$) {
	my ($self ,$vars) = @_;
	return "settings_import_export.html";
}

sub import_settings {
	my ($s, $vars, $fileh) = @_;
	my $tmpfile = $s->_get_import_file($fileh);
	my $import = SD::Settings::Import->new();
	my ($succs, $msg) = (1, "OK");
	eval {
		$import->do_import($tmpfile);
	};
	if (my $err = $@) {
		$err =~ s/at .* line \d+\.?$//g;
		$msg = $err;
		$succs = 0;
	}

	unlink $tmpfile;
	$vars->{'import_success'} = {
		'success' => $succs,
		'message' => $msg,
	};

	return $s->show_import_export($vars);
}

sub _get_import_file {
	my ($s, $fileh) = @_;
	my (undef, $tmpfile) = tempfile();
	open my $tmph, ">", $tmpfile
		or croak "open tmpfile: ", $!;

	#my $buffer;
	print {$tmph} <$fileh>;
#	while (read($fileh, $buffer, 1024)) {
#		print {$tmph} $buffer;
#	}
	close $tmph;
	
	return $tmpfile;
}

1;



sub show_main_settings {
	my ($self, $vars) = @_;
	my $template_file = "settings_main.html";
	my $sqlConfig = $self->{'sqlConfig'};
	$vars->{'dist_preference'}
		= $sqlConfig->get_setting("dist_preference");

	my $l = $sqlConfig->get_setting($DB_LICENSE_FIELD);
	my %license = license_info($l, $CONFIG{slicense_info_path});
	$license{key} = $l;
	$vars->{license} = \%license;



	return $template_file;
}

sub api_check_license {
	my ($s, $api_vars, $license) = @_;
	my %nfo = license_info($license, $CONFIG{slicense_info_path});
	$api_vars->{valid} = $nfo{valid};
	1;
}

sub select_frontpage_version {
    my ($s, $vars, $fpage) = @_;

    my %frontpage_versions = ('webclient2' => 1, 'public' => 1, 'infopage.html' => 1);

    croak "Invalid frontpage version '$fpage'"
        unless $frontpage_versions{$fpage};

    my $oldfile = $ENV{BOITHOHOME} . "/public_html/.htaccess";
    my $newfile = "/tmp/public_html-htaccess.new";


    eval {
	    # Open input file in read mode
	    open INPUTFILE, "<", $oldfile or die("Cant open $oldfile $!");
	    # Open output file in write mode
	    open OUTPUTFILE, ">", $newfile or die("Cant open $newfile $!");

	    # Read the input file line by line
	    while (my $l = <INPUTFILE>) {
	         $l =~ s/(RewriteRule \^\$ \/)(.*?)( \[R\])/$1$fpage$3/g;
	         print OUTPUTFILE $l; 
	    }


	    close INPUTFILE;
	    close OUTPUTFILE;

	    move($newfile,$oldfile) or die("Cant rename (move) $newfile -> oldfile: $!");
    };

    if ($@) {
	warn $@;
        $vars->{frontpage_err} = $@;
    }
    else {
    	$s->{sqlConfig}->update_setting("frontpage_preference", $fpage);
    	$vars->{frontpage_succs} = $fpage;
    }

    return $s->show_frontpage_settings($vars);
}

sub select_anostat_version {
    my ($s, $vars, $fanostat) = @_;

    my %anostat_versions = ('legal' => 1, 'illegal' => 1, );

    croak "Invalid frontpage version '$fanostat'"
        unless $anostat_versions{$fanostat};


    $s->{sqlConfig}->update_setting("anostat_preference", $fanostat);
    $vars->{anostat_succs} = $fanostat;

    return $s->show_anostat_settings($vars);
}


sub select_dist_version {
    my ($s, $vars, $dist) = @_;

    my $versions = $CONFIG{dist_versions};
    croak "Invalid dist version '$dist'"
        unless $CONFIG{dist_versions}->{$dist};

    #my $prev_dist = $s->{sqlConfig}->get_setting(CFG_DIST_VERSION);
    #if ($versions->{$dist} < $versions->{$prev_dist}) {
    #    # Can't downgrade versions
    #    $vars->{dist_downgrade_err} = { prev => $prev_dist, 'new' => $dist };
    #}
    #else {
        my ($succs, $output) = $s->_run_repomod($dist);
        if ($succs) {
            $s->{sqlConfig}->update_setting(CFG_DIST_VERSION, $dist);
            $vars->{dist_succs} = $dist;
        }
        else {
            $vars->{dist_err} = $output;
        }
    #}

    return $s->show_main_settings($vars);
}

sub _run_repomod {
    my ($s, $repo) = @_;
    my $exec_str = "$CONFIG{repomod_path} $repo 2>&1 |";
    open my $rh, $exec_str
        or croak $!;

    my $output = join q{}, <$rh>;
    my $succs = 1;
    close $rh or $succs = 0;

    return wantarray ? ($succs, $output) : $succs;
}


sub export_settings {
	my $s = shift;
	my $export = SD::Settings::Export->new();
	return $export->export();
}

sub update_admin_passwd {
    my ($self, $vars, $pass_ref) = @_;
    my %pass = %{ $pass_ref };
    my $htpasswd;
    eval {
        $htpasswd = Apache::Htpasswd->new($CONFIG{login_htpasswd_path});
    } or $vars->{pass_update_err} = $@;

    my $succs;
    eval {
        croak "New passwords don't match."
            unless $pass{'new'} eq $pass{'new_rep'};
        croak "Old password is incorrect."
            unless $htpasswd->htCheckPassword(
                    $CONFIG{login_admin_user}, $pass{'old'});
        croak "No new password entered"
            unless $pass{'new'};

        $succs = $htpasswd->htpasswd(
                $CONFIG{login_admin_user}, $pass{'new_rep'}, $pass{'old'});
    };
    if ($@) {
        $vars->{pass_update_err} = $@;
        $vars->{pass_update_err} =~ s/at .*? line \d+//g;
        $vars->{passwd} = $pass_ref;
    }
    elsif ($succs) {
        $vars->{pass_updated} = 1;
    }
    else {
        $vars->{pass_update_err} = $htpasswd->error();
        $vars->{passwd} = $pass_ref;
    }
    
    return $self->show_main_settings($vars);
}

1;
