$orig = shift @ARGV;
$new = shift @ARGV;
@color_table = ("backgrounds/blue2","backgrounds/blue_galaxy1","backgrounds/bluegreen","backgrounds/darkgreen","backgrounds/emerald","backgrounds/fiery_galaxy","backgrounds/green","backgrounds/lightgreen","backgrounds/magic_galaxy2","backgrounds/magic_galaxy3","backgrounds/magic_galaxy","backgrounds/mustard","backgrounds/orange","backgrounds/plasma_galaxy","backgrounds/purple2","backgrounds/purple","backgrounds/red","backgrounds/verypurple","backgrounds/white","backgrounds/white_galaxy" );
use POSIX qw(floor);
while(@ARGV) {
  $file = shift @ARGV;
  open(FOO,"<$file");
  $tempfile = "/tmp/$file.$$";
  open(OUT,">$tempfile");
  while($in = <FOO>) {
    $cachunk =($color_table[floor(rand(1)*@color_table)]);
    print $cachunk;
    $in =~ s/$orig/$cachunk/g;
    print OUT $in;
  }
  close(FOO);
  `mv $tempfile $file`;
}

