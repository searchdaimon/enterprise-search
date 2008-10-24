#!/usr/bin/env perl
use strict;
use warnings;
use Page::Connector;
use Page::Connector::API;
use Page::Connector::Form;
use Data::Dumper;
use JSON::XS;
use Carp;
use CGI;
use Readonly;
my $page = Page::Connector->new();
my %state = $page->get_state();
my %tpl_vars;
my $tpl_file;

my $is_api;
my %api_vars;

if (my $form = $state{show}) {
    my $id = $state{connid};
    my $pageForm = Page::Connector::Form->new();

    if ($form eq 'edit') { 
        $tpl_file = $pageForm->show_edit(\%tpl_vars, $id);
    }
    elsif ($form eq 'del') {
        $tpl_file = $pageForm->show_delete(\%tpl_vars, $state{connid});
    }

    else { croak "Unknown form '$form'" }
}

elsif (my $act = $state{act}) {
    my $pageForm = Page::Connector::Form->new();
    if ($act eq 'upload') {
        $pageForm->upload_source(
            $state{connid},
            $state{upload}->{file}
        );
        $tpl_vars{selected_tab} = 1; #switch to edit after upl
        $tpl_file = $pageForm->show_edit(\%tpl_vars, $state{connid});
    }
    
    elsif ($act eq 'new') {
        my $conn_id = $pageForm->create_new();
        $tpl_file = $pageForm->show_edit(\%tpl_vars, $conn_id);
    }
    elsif ($act eq 'clone') {
	my $conn_id = $pageForm->clone($state{connid});
	$tpl_file = $pageForm->show_edit(\%tpl_vars, $conn_id);
    }
    else { croak "Unknown action '$act'" }

}
elsif (my $api = $state{api}) {
    my $pageAPI = Page::Connector::API->new($state{connid});
    $is_api = 1;

   if ($api eq 'edit') {
        $pageAPI->save_source(\%api_vars, $state{edit}->{source});
    }
    elsif ($api eq 'save_attr') {
        $pageAPI->save_cfg_attr(\%api_vars, %{$state{save}}
        );
    }
    elsif ($api eq 'save_name') {
        $pageAPI->save_cfg_name(\%api_vars, $state{save}->{name});
    }
    elsif ($api eq 'param_add') {
        $pageAPI->add_param(\%api_vars, 
            $state{param}->{param}, $state{param}->{example});
    }
    elsif ($api eq 'param_del') {
        $pageAPI->del_param(\%api_vars, $state{param}->{id});
    }
    elsif ($api eq 'test_apply') {
        $pageAPI->apply_test_cfg(\%api_vars, %{$state{share}});
    }
    elsif ($api eq 'test_run') {
        $pageAPI->test_run(\%api_vars);
    }
    elsif ($api eq 'test_output') {
        $pageAPI->test_output(\%api_vars, $state{test_id});
    }
    elsif ($api eq 'test_kill') {
        $pageAPI->test_kill(\%api_vars, $state{test_id});
    }
    elsif ($api eq 'test_cfg_html') { 
        # fetch HTML for test coll config options.
        $pageAPI->cfg_html(\%api_vars, sub {
            my ($file, $vars, $output_ref) = @_;
            $page->process_tpl($file, $vars, ( 
                tpl_folders => 'connector', 
                html_output => $output_ref));
        });
    }
    else { croak "Unknown api call '$api'" }
}
elsif (defined $state{confirm_delete}) {
    croak "not a post request"
        unless $ENV{REQUEST_METHOD} eq 'POST';
    $tpl_file = Page::Connector::Form->new->delete(\%tpl_vars, $state{id});
}

if (!(defined $tpl_file) and !$is_api) {
    $tpl_file = Page::Connector::Form->new->show_list(\%tpl_vars);
}

if ($is_api) {
    my $json = JSON::XS->new;
    $json->pretty(1);
    print $page->get_cgi->header("text/plain");
    print $json->encode(\%api_vars);
}
else {
    $page->process_tpl($tpl_file, \%tpl_vars, ( tpl_folders => 'connector' ))
}

1;
