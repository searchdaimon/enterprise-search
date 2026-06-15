#!/usr/bin/perl
# runs every 5 min to generate statistics for downloaded pages


use MySQLDcConnect;

my $dbh = MySQLDcConnect::GetHandler();








#-- update user statistics
$rv = $dbh->do(qq{
        insert into userhistory5min select 'NULL',downloaded.user,downloaded.crawl - userhistory5minlast.crawl,downloaded.robotstxt - userhistory5minlast.robotstxt,NOW() from userhistory5minlast,downloaded where userhistory5minlast.user = downloaded.user
}) or warn("can´t do statment: ",$dbh->errstr);

#-- update total statistics
#$rv = $dbh->do(qq{
#        insert into history5min select 'NULL',sum(downloaded.crawl - userhistory5minlast.crawl),sum(downloaded.robotstxt - userhistory5minlast.robotstxt),NOW() from userhistory5minlast,downloaded  where userhistory5minlast.user = downloaded.user
#}) or warn("can´t do statment: ",$dbh->errstr);

$rv = $dbh->do(qq{
        insert into history5min select 'NULL',sum(crawl),sum(robotstxt),NOW() from okdownloaded where date > (NOW() - INTERVAL 5 MINUTE)
}) or warn("can´t do statment: ",$dbh->errstr);

#-- update latest measurements overview
$rv = $dbh->do(qq{
        REPLACE into userhistory5minlast select user,crawl,robotstxt from downloaded
}) or warn("can´t do statment: ",$dbh->errstr);


#-- delete 5 min stats older than 30 days
