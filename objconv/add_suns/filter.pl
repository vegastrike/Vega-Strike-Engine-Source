#!/usr/bin/perl

# filters input and spits it out again

$file = shift @ARGV;
$command = shift @ARGV;

open(OUT,"$command $file|") or die "Could not execute $command";
$out = "";
while($in=<OUT>) {
  $out .= $in;
}
open(FILE,">$file");
print FILE $out;

