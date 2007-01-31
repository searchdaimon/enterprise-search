package supportedLang;
require Exporter;
@supportedLang::ISA = qw(Exporter);
@supportedLang::EXPORT = qw();
@supportedLang::EXPORT_OK = qw(isSupportedLang);

$gyldigSprok{'AFR'} = 1;
$gyldigSprok{'BUL'} = 1;
$gyldigSprok{'EST'} = 1;
$gyldigSprok{'GLE'} = 1;
$gyldigSprok{'IND'} = 1;
$gyldigSprok{'MAR'} = 1;
$gyldigSprok{'QUE'} = 1;
$gyldigSprok{'SPA'} = 1;
$gyldigSprok{'UKR'} = 1;
$gyldigSprok{'ALB'} = 1;
$gyldigSprok{'CAT'} = 1;
$gyldigSprok{'FIN'} = 1;
$gyldigSprok{'GLV'} = 1;
$gyldigSprok{'ITA'} = 1;
$gyldigSprok{'MAY'} = 1;
$gyldigSprok{'RUM'} = 1;
$gyldigSprok{'SWA'} = 1;
$gyldigSprok{'VIE'} = 1;
$gyldigSprok{'AMH'} = 1;
$gyldigSprok{'CHI'} = 1;
$gyldigSprok{'FRA'} = 1;
$gyldigSprok{'GRE'} = 1;
$gyldigSprok{'JPN'} = 1;
$gyldigSprok{'NBO'} = 1;
$gyldigSprok{'RUS'} = 1;
$gyldigSprok{'SWE'} = 1;
$gyldigSprok{'WEL'} = 1;
$gyldigSprok{'ARA'} = 1;
$gyldigSprok{'CZE'} = 1;
$gyldigSprok{'FRY'} = 1;
$gyldigSprok{'HEB'} = 1;
$gyldigSprok{'KOR'} = 1;
$gyldigSprok{'NEP'} = 1;
$gyldigSprok{'SAN'} = 1;
$gyldigSprok{'TAM'} = 1;
$gyldigSprok{'YID'} = 1;
$gyldigSprok{'ARM'} = 1;
$gyldigSprok{'DAN'} = 1;
$gyldigSprok{'GEO'} = 1;
$gyldigSprok{'HIN'} = 1;
$gyldigSprok{'LAT'} = 1;
$gyldigSprok{'PER'} = 1;
$gyldigSprok{'SCC'} = 1;
$gyldigSprok{'TGL'} = 1;
$gyldigSprok{'BAQ'} = 1;
$gyldigSprok{'ENG'} = 1;
$gyldigSprok{'GER'} = 1;
$gyldigSprok{'HUN'} = 1;
$gyldigSprok{'LAV'} = 1;
$gyldigSprok{'POL'} = 1;
$gyldigSprok{'SCR'} = 1;
$gyldigSprok{'THA'} = 1;
$gyldigSprok{'BRE'} = 1;
$gyldigSprok{'EPO'} = 1;
$gyldigSprok{'GLA'} = 1;
$gyldigSprok{'ICE'} = 1;
$gyldigSprok{'LIT'} = 1;
$gyldigSprok{'POR'} = 1;
$gyldigSprok{'SLV'} = 1;
$gyldigSprok{'TUR'} = 1;
$gyldigSprok{'aa'} = 1;


sub isSupportedLang {
	$query = shift;
	
	if ($gyldigSprok{$query}) {
		return 1;
	}
	else {
		return 0;
	}

}
