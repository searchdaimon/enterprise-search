#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use CGI::State;
use XMLInterface::Overview;
use Sql::Sql;
use Common::FormFlow;
use Carp;

##
# Provides XML-data for javascript functions.
print CGI::header("text/xml");

my $cgi = CGI->new;
my $state_ptr = CGI::State->state($cgi);
my %state = %{$state_ptr};

my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();


my $function_list = Common::FormFlow->new();
$function_list->add('get_overview', \&get_overview);

my $function = $state{'f'};
$function_list->process($function);


# Group : Rest functions

sub get_overview {
	my $xml_overview = XMLInterface::Overview->new($dbh);
	print $xml_overview->get_full_overview();
}

