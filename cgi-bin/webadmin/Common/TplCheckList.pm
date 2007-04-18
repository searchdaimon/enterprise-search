##
# Module for creating checklist within a Template Toolkit template file.
package Common::TplCheckList;
use strict;
use warnings;

use Carp;
use CGI;

use constant SUCCESS => "success";
use constant FAILED  => "failed";

my @messages;
my $succ_icon = "file.cgi?i=agt_action_success";
my $fail_icon = "file.cgi?i=agt_action_fail";

##
# Default constructor
sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

##
# Change URL to default icons.
#
# Parameters:
#	icon - What icon to alter.
#	url  - URL to icon image.
sub set_icon {
	my ($self, $icon, $url) = @_;
	
	if ($icon eq SUCCESS) {
			$succ_icon = $url;
	}
	elsif ($icon eq FAILED) {
			$fail_icon = $url;
	}
	return;
}

##
# Add a message to the checklist.
#
# Parameters:
#	message - The message.
#	state   - Check state (success, failed etc).
#			  Supported states have been defined as constant.
sub add {
	my ($self, $message, $state) = @_;

	if (	($state eq SUCCESS)
		 or ($state eq FAILED)) {
		push @messages, {
			'msg'   => $message,
			'state' => $state,
		}
	}
	else {
		croak "TplCheckList unknown state: $state";
	}
	return;
}

##
# Print checklist.
#
# Parameters:
#	escape_html - Set to true if the messages should be HTML-escaped.
sub printList {
	my ($self, $escape_html) = @_;
	print $self->_generate_html($escape_html);
	return;
}

##
# Get checklist
#
# Paramters:
#	escape_html - Set to true if the messages should be HTML-escaped.
sub getList {
	my ($self, $escape_html) = @_;
	return $self->_generate_html($escape_html);

}


# Group: Private methods

##
# Generate HTML for checklist.
#
# Paramters:
#	escape_html - Set to true if the messages should be HTML-escaped.
#
# Returns:
#	ul_list - List in HTML syntax.
sub _generate_html {
	my ($self, $escape_html) = @_;
	my $html;

	# Return nothing if list is empty.
	return unless scalar @messages;

	$html .= "<ul class=\"checkList\">";
	foreach my $msg_ref (@messages) {
		my $msg   = $msg_ref->{'msg'};
		my $state = $msg_ref->{'state'};

		my $icon_url;

		if ($state eq SUCCESS) {
			$icon_url = $succ_icon;
		}	
		elsif ($state eq FAILED) {
			$icon_url = $fail_icon;
		}

		else {
			croak "Unknown state ", $state;
		}

		$msg = CGI::escapeHTML($msg)
			if $escape_html;

		$html .= "<li><img src=\"$icon_url\" alt=\"\" />$msg</li>\n";
	}
	$html .= "</ul>";
	return $html;
}


1;
