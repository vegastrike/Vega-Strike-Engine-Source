#!/usr/bin/perl
# adds a light and a star referring to that light
use POSIX qw(floor);

@color_table = ( [ 1,2,3 ], [4,5,6] ,[7,8,9],[10,11,12],[13,14,15],[16,17,18]);
$state = 0;
$sysname = $ARGV[0];
print STDERR $sysname;
$sysname =~ s/system$//;
$sysname =~ s{[/.]}{}g;
$red = $green = $blue = 0;
@colors = (\$red, \$green, \$blue);
while($foo=<>) {
  @tokens = split //,$foo;
      print $foo;
  while(@tokens) {
    $tok = shift @tokens;
    if($state==0 && $tok eq "<") {
      $state=1;
    } 
    elsif($state==1 && $tok eq ">") {
      $state=-1;
      srand(time());
      ($red,$green,$blue) = @{$color_table[floor(rand(1)*@color_table)]};
if(0) {
      ${$colors[floor(rand(3))]} = 1;
      for $colorref (@colors) {
        if($$colorref==0) {
          $$colorref = rand(1);
        } 
      }
}
      print STDERR "aardvark ($red,$green,$blue)\n";
      print <<FOO
<Light>
<ambient red=".3" green=".3" blue=".3"/>
<diffuse red="$red" green="$green" blue="$blue"/>
<specular red="$red" green="$green" blue="$blue"/>
</Light>

FOO
;
      print <<BAR
<Planet name="$sysname" file="sol/sun.png" radius="10000000" x="14594814.00
0000" y="14594814.000000" z="-14594814.000000"  Red="$red" Green="$green" Blue="$blue" ReflectNoLight="true" light="0"/>

BAR
;
    }
  }
}

