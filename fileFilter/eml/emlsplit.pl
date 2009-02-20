#!/usr/bin/perl

use warnings;
use strict;

#use Email::Simple;
use Email::MIME;
use Data::Dumper;

# A bit modified list stolen from mime.types in ubuntu
my %types = 
(	"application/andrew-inset" => "ez",
	"application/atom" => "atom",
	"application/atomcat+xml" => "atomcat",
	"application/atomserv+xml" => "atomsrv",
	"application/cap" => "cap",
	"application/cu-seeme" => "cu",
	"application/dsptype" => "tsp",
	"application/futuresplash" => "spl",
	"application/hta" => "hta",
	"application/java-archive" => "jar",
	"application/java-serialized-object" => "ser",
	"application/java-vm" => "class",
	"application/mac-binhex40" => "hqx",
	"application/mac-compactpro" => "cpt",
	"application/mathematica" => "nb",
	"application/msaccess" => "mdb",
	"application/msword" => "doc",
	"application/octet-stream" => "bin",
	"application/oda" => "oda",
	"application/ogg" => "ogg",
	"application/pdf" => "pdf",
	"application/pgp-keys" => "key",
	"application/pgp-signature" => "pgp",
	"application/pics-rules" => "prf",
	"application/postscript" => "ps",
	"application/rar" => "rar",
	"application/rdf+xml" => "rdf",
	"application/rss+xml" => "rss",
	"application/rtf" => "rtf",
	"application/smil" => "smi",
	"application/wordperfect" => "wpd",
	"application/wordperfect5.1" => "wp5",
	"application/xhtml+xml" => "xhtml",
	"application/xml" => "xml",
	"application/zip" => "zip",
	"application/vnd.cinderella" => "cdy",
	"application/vnd.google-earth.kml+xml" => "kml",
	"application/vnd.google-earth.kmz" => "kmz",
	"application/vnd.mozilla.xul+xml" => "xul",
	"application/vnd.ms-excel" => "xls",
	"application/vnd.ms-pki.seccat" => "cat",
	"application/vnd.ms-pki.stl" => "stl",
	"application/vnd.ms-powerpoint" => "ppt",
	"application/vnd.oasis.opendocument.chart" => "odc",
	"application/vnd.oasis.opendocument.database" => "odb",
	"application/vnd.oasis.opendocument.formula" => "odf",
	"application/vnd.oasis.opendocument.graphics" => "odg",
	"application/vnd.oasis.opendocument.graphics-template" => "otg",
	"application/vnd.oasis.opendocument.image" => "odi",
	"application/vnd.oasis.opendocument.presentation" => "odp",
	"application/vnd.oasis.opendocument.presentation-template" => "otp",
	"application/vnd.oasis.opendocument.spreadsheet" => "ods",
	"application/vnd.oasis.opendocument.spreadsheet-template" => "ots",
	"application/vnd.oasis.opendocument.text" => "odt",
	"application/vnd.oasis.opendocument.text-master" => "odm",
	"application/vnd.oasis.opendocument.text-template" => "ott",
	"application/vnd.oasis.opendocument.text-web" => "oth",
	"application/vnd.rim.cod" => "cod",
	"application/vnd.smaf" => "mmf",
	"application/vnd.stardivision.calc" => "sdc",
	"application/vnd.stardivision.chart" => "sds",
	"application/vnd.stardivision.draw" => "sda",
	"application/vnd.stardivision.impress" => "sdd",
	"application/vnd.stardivision.math" => "sdf",
	"application/vnd.stardivision.writer" => "sdw",
	"application/vnd.stardivision.writer-global" => "sgl",
	"application/vnd.sun.xml.calc" => "sxc",
	"application/vnd.sun.xml.calc.template" => "stc",
	"application/vnd.sun.xml.draw" => "sxd",
	"application/vnd.sun.xml.draw.template" => "std",
	"application/vnd.sun.xml.impress" => "sxi",
	"application/vnd.sun.xml.impress.template" => "sti",
	"application/vnd.sun.xml.math" => "sxm",
	"application/vnd.sun.xml.writer" => "sxw",
	"application/vnd.sun.xml.writer.global" => "sxg",
	"application/vnd.sun.xml.writer.template" => "stw",
	"application/vnd.symbian.install" => "sis",
	"application/vnd.visio" => "vsd",
	"application/vnd.wap.wbxml" => "wbxml",
	"application/vnd.wap.wmlc" => "wmlc",
	"application/vnd.wap.wmlscriptc" => "wmlsc",
	"application/x-123" => "wk",
	"application/x-7z-compressed" => "7z",
	"application/x-abiword" => "abw",
	"application/x-apple-diskimage" => "dmg",
	"application/x-bcpio" => "bcpio",
	"application/x-bittorrent" => "torrent",
	"application/x-cab" => "cab",
	"application/x-cbr" => "cbr",
	"application/x-cbz" => "cbz",
	"application/x-cdf" => "cdf",
	"application/x-cdlink" => "vcd",
	"application/x-chess-pgn" => "pgn",
	"application/x-cpio" => "cpio",
	"application/x-csh" => "csh",
	"application/x-debian-package" => "deb",
	"application/x-director" => "dcr",
	"application/x-dms" => "dms",
	"application/x-doom" => "wad",
	"application/x-dvi" => "dvi",
	"application/x-httpd-eruby" => "rhtml",
	"application/x-flac" => "flac",
	"application/x-font" => "pfa",
	"application/x-freemind" => "mm",
	"application/x-futuresplash" => "spl",
	"application/x-gnumeric" => "gnumeric",
	"application/x-go-sgf" => "sgf",
	"application/x-graphing-calculator" => "gcf",
	"application/x-gtar" => "gtar",
	"application/x-hdf" => "hdf",
	"application/x-httpd-php" => "phtml",
	"application/x-httpd-php-source" => "phps",
	"application/x-httpd-php3" => "php3",
	"application/x-httpd-php3-preprocessed" => "php3p",
	"application/x-httpd-php4" => "php4",
	"application/x-ica" => "ica",
	"application/x-internet-signup" => "ins",
	"application/x-iphone" => "iii",
	"application/x-iso9660-image" => "iso",
	"application/x-java-jnlp-file" => "jnlp",
	"application/x-javascript" => "js",
	"application/x-jmol" => "jmz",
	"application/x-kchart" => "chrt",
	"application/x-killustrator" => "kil",
	"application/x-koan" => "skp",
	"application/x-kpresenter" => "kpr",
	"application/x-kspread" => "ksp",
	"application/x-kword" => "kwd",
	"application/x-latex" => "latex",
	"application/x-lha" => "lha",
	"application/x-lyx" => "lyx",
	"application/x-lzh" => "lzh",
	"application/x-lzx" => "lzx",
	"application/x-maker" => "frm",
	"application/x-mif" => "mif",
	"application/x-ms-wmd" => "wmd",
	"application/x-ms-wmz" => "wmz",
	"application/x-msdos-program" => "com",
	"application/x-msi" => "msi",
	"application/x-netcdf" => "nc",
	"application/x-ns-proxy-autoconfig" => "pac",
	"application/x-nwc" => "nwc",
	"application/x-object" => "o",
	"application/x-oz-application" => "oza",
	"application/x-pkcs7-certreqresp" => "p7r",
	"application/x-pkcs7-crl" => "crl",
	"application/x-python-code" => "pyc",
	"application/x-quicktimeplayer" => "qtl",
	"application/x-redhat-package-manager" => "rpm",
	"application/x-sh" => "sh",
	"application/x-shar" => "shar",
	"application/x-shockwave-flash" => "swf",
	"application/x-stuffit" => "sit",
	"application/x-sv4cpio" => "sv4cpio",
	"application/x-sv4crc" => "sv4crc",
	"application/x-tar" => "tar",
	"application/x-tcl" => "tcl",
	"application/x-tex-gf" => "gf",
	"application/x-tex-pk" => "pk",
	"application/x-texinfo" => "texinfo",
	"application/x-trash" => "~",
	"application/x-troff" => "t",
	"application/x-troff-man" => "man",
	"application/x-troff-me" => "me",
	"application/x-troff-ms" => "ms",
	"application/x-ustar" => "ustar",
	"application/x-wais-source" => "src",
	"application/x-wingz" => "wz",
	"application/x-x509-ca-cert" => "crt",
	"application/x-xcf" => "xcf",
	"application/x-xfig" => "fig",
	"application/x-xpinstall" => "xpi",
	"audio/basic" => "au",
	"audio/midi" => "mid",
	"audio/mpeg" => "mpga",
	"audio/mpegurl" => "m3u",
	"audio/prs.sid" => "sid",
	"audio/x-aiff" => "aif",
	"audio/x-gsm" => "gsm",
	"audio/x-mpegurl" => "m3u",
	"audio/x-ms-wma" => "wma",
	"audio/x-ms-wax" => "wax",
	"audio/x-pn-realaudio" => "ra",
	"audio/x-realaudio" => "ra",
	"audio/x-scpls" => "pls",
	"audio/x-sd2" => "sd2",
	"audio/x-wav" => "wav",
	"chemical/x-alchemy" => "alc",
	"chemical/x-cache" => "cac",
	"chemical/x-cache-csf" => "csf",
	"chemical/x-cactvs-binary" => "cbin",
	"chemical/x-cdx" => "cdx",
	"chemical/x-cerius" => "cer",
	"chemical/x-chem3d" => "c3d",
	"chemical/x-chemdraw" => "chm",
	"chemical/x-cif" => "cif",
	"chemical/x-cmdf" => "cmdf",
	"chemical/x-cml" => "cml",
	"chemical/x-compass" => "cpa",
	"chemical/x-crossfire" => "bsd",
	"chemical/x-csml" => "csml",
	"chemical/x-ctx" => "ctx",
	"chemical/x-cxf" => "cxf",
	"chemical/x-embl-dl-nucleotide" => "emb",
	"chemical/x-galactic-spc" => "spc",
	"chemical/x-gamess-input" => "inp",
	"chemical/x-gaussian-checkpoint" => "fch",
	"chemical/x-gaussian-cube" => "cub",
	"chemical/x-gaussian-input" => "gau",
	"chemical/x-gaussian-log" => "gal",
	"chemical/x-gcg8-sequence" => "gcg",
	"chemical/x-genbank" => "gen",
	"chemical/x-hin" => "hin",
	"chemical/x-isostar" => "istr",
	"chemical/x-jcamp-dx" => "jdx",
	"chemical/x-kinemage" => "kin",
	"chemical/x-macmolecule" => "mcm",
	"chemical/x-macromodel-input" => "mmd",
	"chemical/x-mdl-molfile" => "mol",
	"chemical/x-mdl-rdfile" => "rd",
	"chemical/x-mdl-rxnfile" => "rxn",
	"chemical/x-mdl-sdfile" => "sd",
	"chemical/x-mdl-tgf" => "tgf",
	"chemical/x-mmcif" => "mcif",
	"chemical/x-mol2" => "mol2",
	"chemical/x-molconn-Z" => "b",
	"chemical/x-mopac-graph" => "gpt",
	"chemical/x-mopac-input" => "mop",
	"chemical/x-mopac-out" => "moo",
	"chemical/x-mopac-vib" => "mvb",
	"chemical/x-ncbi-asn1" => "asn",
	"chemical/x-ncbi-asn1-ascii" => "prt",
	"chemical/x-ncbi-asn1-binary" => "val",
	"chemical/x-ncbi-asn1-spec" => "asn",
	"chemical/x-pdb" => "pdb",
	"chemical/x-rosdal" => "ros",
	"chemical/x-swissprot" => "sw",
	"chemical/x-vamas-iso14976" => "vms",
	"chemical/x-vmd" => "vmd",
	"chemical/x-xtel" => "xtel",
	"chemical/x-xyz" => "xyz",
	"image/gif" => "gif",
	"image/ief" => "ief",
	"image/jpeg" => "jpeg",
	"image/pcx" => "pcx",
	"image/png" => "png",
	"image/svg+xml" => "svg",
	"image/tiff" => "tiff",
	"image/vnd.djvu" => "djvu",
	"image/vnd.wap.wbmp" => "wbmp",
	"image/x-cmu-raster" => "ras",
	"image/x-coreldraw" => "cdr",
	"image/x-coreldrawpattern" => "pat",
	"image/x-coreldrawtemplate" => "cdt",
	"image/x-corelphotopaint" => "cpt",
	"image/x-icon" => "ico",
	"image/x-jg" => "art",
	"image/x-jng" => "jng",
	"image/x-ms-bmp" => "bmp",
	"image/x-photoshop" => "psd",
	"image/x-portable-anymap" => "pnm",
	"image/x-portable-bitmap" => "pbm",
	"image/x-portable-graymap" => "pgm",
	"image/x-portable-pixmap" => "ppm",
	"image/x-rgb" => "rgb",
	"image/x-xbitmap" => "xbm",
	"image/x-xpixmap" => "xpm",
	"image/x-xwindowdump" => "xwd",
	"message/rfc822" => "eml",
	"model/iges" => "igs",
	"model/mesh" => "msh",
	"model/vrml" => "wrl",
	"text/calendar" => "ics",
	"text/css" => "css",
	"text/csv" => "csv",
	"text/h323" => "323",
	"text/html" => "html",
	"text/iuls" => "uls",
	"text/mathml" => "mml",
	"text/plain" => "txt",
	"text/txt" => "txt",
	"text/richtext" => "rtx",
	"text/scriptlet" => "sct",
	"text/texmacs" => "tm",
	"text/tab-separated-values" => "tsv",
	"text/vnd.sun.j2me.app-descriptor" => "jad",
	"text/vnd.wap.wml" => "wml",
	"text/vnd.wap.wmlscript" => "wmls",
	"text/x-bibtex" => "bib",
	"text/x-boo" => "boo",
	"text/x-c++hdr" => "h++",
	"text/x-c++src" => "c++",
	"text/x-chdr" => "h",
	"text/x-component" => "htc",
	"text/x-csh" => "csh",
	"text/x-csrc" => "c",
	"text/x-dsrc" => "d",
	"text/x-diff" => "diff",
	"text/x-haskell" => "hs",
	"text/x-java" => "java",
	"text/x-literate-haskell" => "lhs",
	"text/x-moc" => "moc",
	"text/x-pascal" => "p",
	"text/x-pcs-gcd" => "gcd",
	"text/x-perl" => "pl",
	"text/x-python" => "py",
	"text/x-setext" => "etx",
	"text/x-sh" => "sh",
	"text/x-tcl" => "tcl",
	"text/x-tex" => "tex",
	"text/x-vcalendar" => "vcs",
	"text/x-vcard" => "vcf",
	"video/3gpp" => "3gp",
	"video/dl" => "dl",
	"video/dv" => "dif",
	"video/fli" => "fli",
	"video/gl" => "gl",
	"video/mpeg" => "mpeg",
	"video/mp4" => "mp4",
	"video/quicktime" => "qt",
	"video/vnd.mpegurl" => "mxu",
	"video/x-la-asf" => "lsf",
	"video/x-mng" => "mng",
	"video/x-ms-asf" => "asf",
	"video/x-ms-wm" => "wm",
	"video/x-ms-wmv" => "wmv",
	"video/x-ms-wmx" => "wmx",
	"video/x-ms-wvx" => "wvx",
	"video/x-msvideo" => "avi",
	"video/x-sgi-movie" => "movie",
	"x-conference/x-cooltalk" => "ice",
	"x-epoc/x-sisx-app" => "sisx",
	"x-world/x-vrml" => "vrm"
);

my $fileemail;

my $dirfiltername = "/tmp/dirfilter-" . $< . "/";

$fileemail = shift @ARGV or die "Usage: ./emlsplit.pl emlfile" ;

#debug: lagrer filen slik at vi har .eml filene.
#system("cp $fileemail /tmp/dirfilter-rb-tmp/");

my $message;
{
	local(*FH);
	open(FH, $fileemail) or die "Unable to open file\n";
	$message = do { local($/) ; <FH> } ;
}

my $parsed = Email::MIME->new($message);

#runarb: 01.11.07 må lage dirfilter mappen hvis vi ikke har
if (!(-e "$dirfiltername")) {
	mkdir("$dirfiltername") or die("mkdir $dirfiltername");
}

my $headername = "$dirfiltername".$parsed->invent_filename.".header.";
open(my $mh, "> $headername") or die "$!: $headername";
foreach my $hn (qw(Subject From To Cc Bcc Reply-to Date)) {
	my @values = $parsed->header($hn);
	next if (length(@values) == 0);
	print $mh "$hn:";
	print $mh join(", ", @values);
	print $mh "\n";
}
close($mh);

sub writemail {
	my ($p) = @_;

	my @parts = $p->parts;

	foreach (@parts) {
		my $fn;

		if (defined $_->content_type and $_->content_type =~ /^multipart\//) {
			my @sub = $_->subparts;
			foreach my $subp (@sub) {
				writemail($subp);
			}
			next;	
		}
		
		my ($ct) = defined $_->content_type ? split(";", $_->content_type) : "text/txt";
		$fn = "$dirfiltername".$_->invent_filename($ct);
		my $suffix;
		$suffix = defined($types{$ct}) ? $types{$ct} : "dat";
		if ($ct eq 'application/octet-stream') {
			if (defined($_->{ct}) && defined($_->{ct}->{attributes}) &&
			    defined($_->{ct}->{attributes}{name}) && $_->{ct}->{attributes}{name} =~ /\.(\w+)$/) {
				$suffix = $1;
				$fn .= ".$suffix";
			}
		}
		print ($suffix ." ".$fn."\n");
		open(my $wf, "> $fn") || die "$fn: $!";
		print $wf $_->body;
		close $wf;
	}
}

writemail($parsed);

print ("txt" ." ".$headername."\n");

use Date::Parse;

if (defined($ENV{SDMETAFILE}))  {
	# Write some data to the metaspec file
	my $metafile = $ENV{SDMETAFILE};
	open(FH, "> $metafile");
	my @values = $parsed->header("Date");
	if (defined($values[0])) {
		print FH "lastmodified = " . str2time($values[0]) . "\n";
	}
	close(FH);
}
