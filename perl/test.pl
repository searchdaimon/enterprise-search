use URI::URL;

my $url = "http://www.Exsempel.com:80/å.html";

print ResulveUrl('http://www.dmoz.org/',$url), "\n";

sub ResulveUrl {
        my($BaseUrl,$NyUrl) = @_;

        $link = new URI::URL $NyUrl;
        $FerdigUrl = $link->abs($BaseUrl);

        return $FerdigUrl;
}

