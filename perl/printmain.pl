open(INF,"main") or die($!);
binmode(INF);
while (not (eof(INF))) {

	read(INF,$post,24);
	
	($sha1,$DocID)= unpack('A20 I',$post);

	print "$sha1 -> $DocID\n";

}

close(INF);
