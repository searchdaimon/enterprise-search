package Page::UserSys::API;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos);
use Readonly;

use Page::UserSys;
use Page::API;
our @ISA = qw(Page::UserSys Page::API);

Readonly::Scalar my $SYS_PRIMARY   => "prim";
Readonly::Scalar my $SYS_SECONDARY => "sec";

Readonly::Scalar my $DEF_LIST_LIMIT => 100;

Readonly::Scalar my $DEF_OFFSET => 0;

sub _init {
	my $s = shift;
	$s->SUPER::_init(@_);
	$s->{prim_id} = $s->{sql_sys}->primary_id();
}



sub add_mapping {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 1, 1);
	my ($s, $api_vars, $sec_sys, $prim_usr, $sec_usr) = @_;

	eval {
		$s->_common_errors({ sys_exists => $sec_sys });
		croak "Secondary system does not exist."
			unless $s->{sql_sys}->exists({ id => $sec_sys });
	};
	return if $s->api_error($api_vars, $@);
	
	$s->{sql_mapping}->insert({ 
			prim_usr => $prim_usr, 
			secnd_usr => $sec_usr,
			'system' => $sec_sys,
	});

	$api_vars->{ok} = "Mapping '$prim_usr' to '$sec_usr' added";
	$s;
}

sub del_mapping {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 1, 1);
	my ($s, $api_vars, $sec_sys, $prim_usr, $sec_usr) = @_;

	$s->{sql_mapping}->delete({ 
		'system' => $sec_sys,
		prim_usr => $prim_usr,
		secnd_usr => $sec_usr,
	});
	$api_vars->{ok} = "Mapping '$prim_usr' to '$sec_usr' deleted.";
	$s;
}

sub list_mapping {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 0, 0);
	my ($s, $api_vars, $sec_sys, $offset, $limit) = @_;
	$offset = $s->_adjust_offset($offset);
	$limit  = $s->_adjust_limit($limit);
	eval {
		$s->_common_errors({ sys_exists => $sec_sys });
	};
	return if $s->api_error($api_vars, $@);

	$api_vars->{mapped_total} = $s->{sql_mapping}->count({ 
		'system' => $sec_sys
	});
	
	$api_vars->{mapping} = [ map { 
		[ $_->{prim_usr}, $_->{secnd_usr} ] 
		} $s->{sql_mapping}->get_mapping($sec_sys, $offset, $limit) 
	];
	$s;
}

##
# List unmapped users from given system
sub list_unmapped {
	validate_pos(@_, 1, 1, 1, 1, 0, 0);
	my ($s, $api_vars, $sec_sys, $which_sys, $offset, $limit) = @_;
	$offset = $s->_adjust_offset($offset)
		if defined $offset;
	$limit  = $s->_adjust_limit($limit)
		if defined $limit;

	eval {
		croak "Invalid/missing value for 'system'"
			if (not $which_sys 
		    	    or ($which_sys ne $SYS_PRIMARY 
			    && $which_sys ne $SYS_SECONDARY));
		$s->_common_errors({ sys_exists => $sec_sys });
	};
	return if $s->api_error($api_vars, $@);

	# Get all mapped
	my $sql_field = ($which_sys eq $SYS_PRIMARY) 
		? 'prim_usr' : 'secnd_usr';
	my %mapped = map { $_->{$sql_field} => 1 }
			$s->{sql_mapping}->get_mapping($sec_sys);
	
	# List sys users, ignore mapped
	my $id = ($which_sys eq $SYS_PRIMARY) 
		? $s->{sql_sys}->primary_id() : $sec_sys;
	my $ignored = 0;
	my @unmapped;
	my @all_users = $s->list_users($id);
	for my $usr (@all_users) {
		next if $mapped{$usr};
		next if defined $offset && ($offset && $offset > $ignored++);
		push @unmapped, $usr;
		last if defined $limit && scalar @unmapped >= $limit;
	}

	$api_vars->{users} = \@unmapped;
	$api_vars->{users_total} = scalar @all_users;
	# NOTE: %mapped may contain users
	# that are no longer in the system.
	#croak Dumper(scalar @all_users, scalar keys %mapped);
	$api_vars->{unmapped_total} = @all_users - keys %mapped; 

	$s;
}

sub auto_map {
	my ($s, $api_vars, $sec_sys) = @_;
	eval {
		$s->_common_errors({
			is_POST => 1,
			sys_exists => $sec_sys,
			not_primary => $sec_sys,
		});
	};
	return if $s->api_error($api_vars, $@);

	my $automapped = 0;
	my %primary_users = map { $_ => 1 } $s->list_users($s->{prim_id});
	
	for my $u ($s->list_users($sec_sys)) {
		next unless $primary_users{$u};
		next if $s->{sql_mapping}->exists({
			prim_usr  => $u,
			secnd_usr => $u,
			'system'  => $sec_sys
		});
		$s->{sql_mapping}->insert({ 
			prim_usr  => $u,
			secnd_usr => $u,
			'system'  => $sec_sys,
		});
		$automapped++;
	}
	$api_vars->{automapped} = $automapped;
	$api_vars->{ok} = "$automapped users were mapped.";
	$s;
}

sub clear_mapping {
	my ($s, $api_vars, $sec_sys) = @_;
	eval {
		$s->_common_errors({
			is_POST => 1,
			sys_exists => $sec_sys,
			not_primary => $sec_sys
		});
	};
	return if $s->api_error($api_vars, $@);

	$s->{sql_mapping}->delete({ 'system' => $sec_sys });
	
	$api_vars->{ok} = "All mapping cleared.";
	$s;
}

sub _common_errors {
	my ($s, $checks) = @_;
	croak "Primary system is not set"
		unless $s->{prim_id};
	
	return unless $checks;
	
	if (exists $checks->{sys_exists}) {
		croak "System '$checks->{sys_exists}' does not exist"
			unless $s->{sql_sys}->exists({ 
				id => $checks->{sys_exists} 
			});
	}
	if ($checks->{not_POST}) {
		croak "Request must be a POST request."
			unless $ENV{REQUEST_METHOD} eq "POST";
	}
	if ($checks->{not_primary}) {
		croak "Secondary system can not be the same as primary."
			if $checks->{not_primary} == $s->{prim_id};
	}
}

sub _adjust_limit {
	my ($s, $limit) = @_;
	$limit ||= $DEF_LIST_LIMIT;
	return if $limit eq "null";
	croak "limit not a uint" 
		unless $limit =~ /^\d+$/;
	
	return $limit;
}

sub _adjust_offset {
	my ($s, $offset) = @_;
	$offset ||= $DEF_OFFSET;
	return if $offset eq "null";
	croak "offset not a uint" 
		unless $offset =~ /^\d+$/;
	return $offset;
}
1;
