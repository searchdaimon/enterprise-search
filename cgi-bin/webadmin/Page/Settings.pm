package Page::Settings;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use config (qw($CONFIG));
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::SettingsExport;
use Sql::Config;
use Sql::Shares;
use Sql::ShareGroups;
my %CONFIG = %$CONFIG;

# Helper class with functions used with add.cgi
# Function used to add shares.
#

sub new {
	my $class = shift;
	my $dbh = shift;
	my $self = {};
	bless $self, $class;
	
	$self->_init($dbh);

	return $self;
}

sub _init {
	my ($self, $dbh, $state) =  (@_);
	$self->{'dbh'} = $dbh;
	$self->{'sqlConfig'} = Sql::Config->new($dbh);
	$self->{'settings'} = Boitho::SettingsExport->new($dbh);
}

## Delete all settings.
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


sub update_settings($$) {
	my ($self, $vars, $setting) = @_;
	my $sqlConfig = $self->{'sqlConfig'};
	while (my ($key, $value) = each(%$setting)) {
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
	my $sqlConfig = $self->{'sqlConfig'};
	my $template_file = "settings_advanced.html";
	$vars->{'settings'} = $sqlConfig->get_all(); 
	
	# Get default settings for setting spesified in config file.
	my @default_settings;
	foreach my $setting (@{$CONFIG{'default_settings'}}) {
		$setting->{'table_value'} = 
			$sqlConfig->get_setting($setting->{'table_key'});
		push @default_settings, $setting;
	}
	$vars->{'default_settings'} = \@default_settings;

	return ($vars, $template_file);
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

## Update preferred dist version.
sub select_dist_version($$) {
	my ($self, $vars, $selected) = @_;
	my $sqlConfig = $self->{'sqlConfig'};	

	croak "Unknown version \"$selected\" selected"
		unless grep /^$selected$/, ('production', 'testing', 'development');

	$sqlConfig->update_setting('dist_preference', $selected);
	$vars->{'selected_new'} = $selected;

	return $self->show_main_settings($vars);
}

sub export_settings($) {
	my $self = shift;
	my $settings = $self->{'settings'};
	return $settings->export_settings('OBFUSCATE');	
}


1;
