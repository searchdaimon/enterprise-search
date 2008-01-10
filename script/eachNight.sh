#!/bin/sh

$BOITHOHOME/bin/dictionarywordsLot all
sh $BOITHOHOME/script/reloadDictionary.sh
$BOITHOHOME/bin/suggest_client_redict localhost
