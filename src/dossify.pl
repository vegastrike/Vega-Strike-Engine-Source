while(@ARGV) {
$filename = shift @ARGV;
`copy $filename $filename.foo`;
open(FOO,"<$filename.foo");<BR>open(OUT,">$filename");
while(<FOO>) {
  s/(\015|\012)+$/\n/;
  print OUT;
}
close(FOO);
close(OUT);
`del $filename.foo`;
}
