package Page::Settings;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use config qw(%CONFIG);
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::SettingsExport;
use Sql::Config;
use Sql::Shares;
use Sql::ShareGroups;
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
	$self->{settings} = Boitho::SettingsExport->new($dbh);

        croak "Repomod is not executable"
            unless -x $CONFIG{repomod_path};
}

## 
# Delete all settings.
sub delete_all_settings($) {
	my $self = shift;
	my $keep_configkeys = shift;
	my $dbh = $self->{'dbh'};

	for ('Sql::Config', 'Sql::Shares', 'Sql::ShareGroups', 'Sql::CollectionAuth') {
		my $sql = $_->new($dbh);
		if ($keep_configkeys and $_ eq 'Sql::Config') {
			$_->delete_all($keep_configkeys);
			next;
		}
		$sql->delete_all();
	}
	1;
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


sub confirmed_delete_settings($$) {
	my ($self, $vars) = @_;
	$self->delete_all_settings('KEEP CONFIGKEYS');
	my $template_file = 'settings_delete_done.html';
	return ($vars, $template_file);
}

sub show_confirm_dialog($$) {
	my ($self, $vars) = @_;
	return ($vars, "settings_delete_all.html");
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

	return ($vars, TPL_ADVANCED);
}

sub show_advanced_settings_updated($$) {
	my ($self, $vars) = @_;
	return $self->show_advanced_settings($vars);
}

sub show_import_export($$) {
	my ($self ,$vars) = @_;
	return ($vars, "settings_import_export.html");
}

sub import_settings($$$) {
	my ($self, $vars, $file) = @_;
	my $settings = $self->{'settings'};
	$self->delete_all_settings();
	eval {
		$settings->import_settings($file, 'OBFUSCATE');
	};
	$vars->{'import_success'} = {	
			'success' =>1,
			'message' => "OK",
	};
	
	$vars->{'import_success'} = {
			'success' => 0,
			'message' => $@, }
			if $@; # import failed.
	
	return $self->show_import_export($vars);
}

sub show_main_settings($$) {
	my ($self, $vars) = @_;
	my $template_file = "settings_main.html";
	my $sqlConfig = $self->{'sqlConfig'};
	$vars->{'dist_preference'}
		= $sqlConfig->get_setting("dist_preference");


	return ($vars, $template_file);
}

sub select_dist_version {
    my ($s, $vars, $dist) = @_;

    my $versions = $CONFIG{dist_versions};
    croak "Invalid dist version '$dist'"
        unless $CONFIG{dist_versions}->{$dist};

    my $prev_dist = $s->{sqlConfig}->get_setting(CFG_DIST_VERSION);
    if ($versions->{$dist} < $versions->{$prev_dist}) {
        # Can't downgrade versions
        $vars->{dist_downgrade_err} = { prev => $prev_dist, 'new' => $dist };
    }
    else {
        my ($succs, $output) = $s->_run_repomod($dist);
        if ($succs) {
            $s->{sqlConfig}->update_setting(CFG_DIST_VERSION, $dist);
            $vars->{dist_succs} = $dist;
        }
        else {
            $vars->{dist_err} = $output;
        }
    }

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


sub export_settings($) {
	my $self = shift;
	my $settings = $self->{'settings'};
	return $settings->export_settings('OBFUSCATE');	
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
