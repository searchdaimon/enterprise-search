#!/bin/sh

$BOITHOHOME/bin/dictionarywordsLot all
sh $BOITHOHOME/script/reloadDictionary.sh
/etc/init.d/suggest restart
$BOITHOHOME/bin/suggest_client_redict localhost
