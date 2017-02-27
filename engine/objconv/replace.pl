$orig = shift @ARGV;
$new = shift @ARGV;

while(@ARGV) {
  $file = shift @ARGV;
  open(FOO,"<$file");
  $tempfile = "/tmp/$file.$$";
  open(OUT,">$tempfile");
  while($in = <FOO>) {
    $in =~ s/$orig/$new/g;
    print OUT $in;
  }
  close(FOO);
  `mv $tempfile $file`;
}

