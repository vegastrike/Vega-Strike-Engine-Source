#!/usr/bin/perl
# strips out the light= attributes

while($foo=<>) {
  $foo =~ s/light\s*=\s*"[^"]"//;
  print $foo;
}

