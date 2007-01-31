open(INF,$ARGV[0]) or die($!);
binmode(INF);
while (not (eof(INF))) {

 read(INF,$post,4);

 $DocID = unpack('I',$post);

 print "$DocID\n";

}

close(INF);

