#!/usr/bin/perl
# kjøres hvert 5 min for å generere statestikk for nedlastede sider


use MySQLDcConnect;

my $dbh = MySQLDcConnect::GetHandler();








#-- oppdaterer bruker statestikk
$rv = $dbh->do(qq{
        insert into userhistory5min select 'NULL',downloaded.user,downloaded.crawl - userhistory5minlast.crawl,downloaded.robotstxt - userhistory5minlast.robotstxt,NOW() from userhistory5minlast,downloaded where userhistory5minlast.user = downloaded.user
}) or warn("can´t do statment: ",$dbh->errstr);

#-- oppdaterer total statestikk
#$rv = $dbh->do(qq{
#        insert into history5min select 'NULL',sum(downloaded.crawl - userhistory5minlast.crawl),sum(downloaded.robotstxt - userhistory5minlast.robotstxt),NOW() from userhistory5minlast,downloaded  where userhistory5minlast.user = downloaded.user
#}) or warn("can´t do statment: ",$dbh->errstr);

$rv = $dbh->do(qq{
        insert into history5min select 'NULL',sum(crawl),sum(robotstxt),NOW() from okdownloaded where date > (NOW() - INTERVAL 5 MINUTE)
}) or warn("can´t do statment: ",$dbh->errstr);

#-- oppdaterer siste molinger oversikt
$rv = $dbh->do(qq{
        REPLACE into userhistory5minlast select user,crawl,robotstxt from downloaded
}) or warn("can´t do statment: ",$dbh->errstr);


#-- sletter eldre en 30 dager 5 min stat
