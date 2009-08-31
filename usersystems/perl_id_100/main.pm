package PerlUserSystem;
use strict;
use warnings;

use UserSystem;

use Carp;
use File::Temp qw(tempfile);

sub list_users {
	my $params = shift;

	my @users;
	_scp_fetch_file($params->{ip}, $params->{username}, $params->{password}, $Const::PASSWD_FILE, sub {
		my $line = shift;
		
		#uucp:x:10:14:uucp:/var/spool/uucp:/sbin/nologin
		my ($user, undef, undef, undef, undef, undef, $shell) = split q{:}, $line, 7;
		
		return unless defined $shell;
		return if $shell =~ /nologin$/;
		
		chomp $user;
		return if $user eq q{};

		push @users, $user;
	});
	return @users;
}

sub list_groups {
	my ($params, $group_user) = @_;

	my @groups;
	_scp_fetch_file($params->{ip}, $params->{username}, $params->{password}, $Const::GROUP_FILE, sub {
		return if @groups; # found groups already.

		my $line = shift;
		my ($user, undef, undef, $user_groups) = split q{:}, $line, 4;

		return unless $user eq $group_user;
		chomp $user_groups;
		return if $user_groups eq q{};

		@groups = split q{,}, $user_groups;
	});

	return @groups;
}

sub _scp_fetch_file {
	my ($host, $user, $passwd, $remote_file, $readline_callb) = @_;

	# The SCP/SFTP/SSH modules in yum were 64bit, we need 32bit.
	# So was the Expect module. Thus using a duct tape solution:
	# Generate expect script, run it with external utility.
	
	my (undef, $local_file) = tempfile();
	$remote_file = "$user\@$host:$remote_file";

	my $expect_script = sprintf $Const::EXPECT_TPL, 
		$Const::EXPECT_TIMEOUT, "\Q$remote_file\E", "\Q$local_file\E", "\Q$passwd\E";

	my ($exp_fh, $exp_file) = tempfile();
	print {$exp_fh} $expect_script;
	close $exp_fh;

	print "Connecting to '$host' with username '$user'\n";
	
	open my $exph, "$Const::EXPECT_CMD $exp_file |"
		or die "unable to run expect: ", $!;
	print $_ while <$exph>;
	unlink $exp_file;
	close $exph or die "$Const::EXPECT_CMD exited with error.";

	open my $fh, "<", $local_file
		or croak "Unable to open '$local_file': ", $!;
	while (my $line = <$fh>) {
		&$readline_callb($line);
	}
	1;
}

sub authenticate {
	my ($params, $username, $password) = (@_);

	my $expect_script = sprintf $Const::EXPECT_SSH, 
		$Const::EXPECT_TIMEOUT, "\Q$$username\@$params->{ip}\E", "\Qid\E", "\Q$$password\E";

	my ($exp_fh, $exp_file) = tempfile();
	print {$exp_fh} $expect_script;
	close $exp_fh;

	print "Connecting to '$params->{ip}' with username '$$username'\n";

	my $exph;

	if (! open $exph, "$Const::EXPECT_CMD $exp_file |") {
		print "unable to run expect: $!\n";
		return 0;
	    }

	print $_ while <$exph>;
	unlink $exp_file;
	if (! close $exph) {
		print "$Const::EXPECT_CMD exited with error.\n";
		return 0;
	    }

	return 1;
}


1;

package Const;
use Readonly;
Readonly::Scalar our $SCP_CMD => "scp";
Readonly::Scalar our $PASSWD_FILE => "/etc/passwd";
Readonly::Scalar our $GROUP_FILE => "/etc/group";

Readonly::Scalar our $EXPECT_CMD => "expect";
Readonly::Scalar our $EXPECT_TIMEOUT => 20;
Readonly::Scalar our $EXPECT_TPL => q|
set timeout %s
spawn scp %s %s
expect {
	"password:" {
		send "%s\r\n"
		exp_continue
	} "yes/no)?" {
		send "yes\r\n"
		set timeout -1
		exp_continue
	} "Permission denied, please try again." {
		puts "Permission denied, please try again."
		exit 1
	} eof {
		exit
	} timeout {
		puts "timeout"
		exit 1
	} -re . { #match everyting else
		exp_continue
	}
}
|;
Readonly::Scalar our $EXPECT_SSH => q|
set timeout %s
spawn ssh %s %s
expect {
	"password:" {
		send "%s\r\n"
		exp_continue
	} "yes/no)?" {
		send "yes\r\n"
		set timeout -1
		exp_continue
	} "Permission denied, please try again." {
		puts "Permission denied, please try again."
		exit 1
	} eof {
		exit
	} timeout {
		puts "timeout"
		exit 1
	} -re . { #match everyting else
		exp_continue
	}
}
|;

1;
