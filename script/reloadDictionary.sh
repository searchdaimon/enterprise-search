#!/bin/sh

$BOITHOHOME/bin/dictionarywordsLot all
kill -USR1 `cat $BOITHOHOME/var/searchd.pid`
