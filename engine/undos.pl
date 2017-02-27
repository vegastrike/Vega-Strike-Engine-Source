#!/usr/bin/perl 

$tempfile = "foo.$$";

print "$tempfile\n";

if(-e $tempfile) {
  print "File $tempfile found in current directory, please remove\n";
  exit(1);
}

`touch $tempfile` and die "Could not create $tempfile";

for $filename (@ARGV) {
  `rm $tempfile` and die "Could not remove $tempfile";
  open(IN, "<$filename");
  open(OUT, ">$tempfile");
  while(<IN>) {
    s|\015||g;
    print OUT "$_";
  }
  `cp $filename $tempfile.1` and die "could not create backup file";
  `cp $tempfile $filename` and die "could not move temp file onto original file";
  `rm $tempfile.1` and die "could not remove backup file";
}
`rm $tempfile`;

exit(0);

