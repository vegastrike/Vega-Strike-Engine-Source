#!/usr/bin/perl -w
#
# Boost.Function library
#
# Copyright (C) 2001 Doug Gregor (gregod@cs.rpi.edu)
#
# Permission to copy, use, sell and distribute this software is granted
# provided this copyright notice appears in all copies.
# Permission to modify the code and to distribute modified code is granted
# provided this copyright notice appears in all copies, and a notice
# that the code was modified is included with the copyright notice.
#
# This software is provided "as is" without express or implied warranty,
# and with no claim as to its suitability for any purpose.
#
# For more information, see http://www.boost.org
use English;

if ($#ARGV < 0) {
  print "Usage: perl gen_function_N <number of arguments>\n";
  exit;
}


$totalNumArgs = $ARGV[0];
for ($numArgs = 0; $numArgs <= $totalNumArgs; ++$numArgs) {
  open OUT, ">function$numArgs.hpp";
  print OUT "// Boost.Function library\n";
  print OUT "//\n";
  print OUT "// Copyright (C) 2001 Doug Gregor (gregod\@cs.rpi.edu)\n";
  print OUT "//\n";
  print OUT "// Permission to copy, use, sell and distribute this software is granted\n";
  print OUT "// provided this copyright notice appears in all copies.\n";
  print OUT "// Permission to modify the code and to distribute modified code is granted\n";
  print OUT "// provided this copyright notice appears in all copies, and a notice\n";
  print OUT "// that the code was modified is included with the copyright notice.\n";
  print OUT "//\n";
  print OUT "// This software is provided \"as is\" without express or implied warranty,\n";
  print OUT "// and with no claim as to its suitability for any purpose.\n";
  print OUT " \n";
  print OUT "// For more information, see http://www.boost.org\n";
  print OUT "\n";
  print OUT "#ifndef BOOST_FUNCTION_FUNCTION" . $numArgs . "_HEADER\n";
  print OUT "#define BOOST_FUNCTION_FUNCTION" , $numArgs . "_HEADER\n";
  print OUT "\n";
  print OUT "#define BOOST_FUNCTION_NUM_ARGS $numArgs\n";

  $templateParms = "";
  for ($i = 0; $i < $numArgs; ++$i) {
    if ($i > 0) {
      $templateParms .= ", ";
    }
    $templateParms .= "typename T$i";
  }
  print OUT "#define BOOST_FUNCTION_TEMPLATE_PARMS $templateParms\n";

  $_ = $templateParms;
  s/typename //g;
  $templateArgs = $_;
  print OUT "#define BOOST_FUNCTION_TEMPLATE_ARGS $templateArgs\n";

  $parms = "";
  for ($i = 0; $i < $numArgs; ++$i) {
    if ($i > 0) {
      $parms .= ", ";
    }
    $parms .= "T$i a$i";
  }
  print OUT "#define BOOST_FUNCTION_PARMS $parms\n";

  $args = "";
  for ($i = 0; $i < $numArgs; ++$i) {
    if ($i > 0) {
      $args .= ", ";
    }
    $args .= "a$i";
  }
  print OUT "#define BOOST_FUNCTION_ARGS $args\n";

  $not0Parms = "";
  for ($i = 1; $i < $numArgs; ++$i) {
    if ($i > 1) {
      $not0Parms .= ", ";
    }
    $not0Parms .= "T$i a$i";
  }
  print OUT "#define BOOST_FUNCTION_NOT_0_PARMS $not0Parms\n";

  $not0Args = "";
  for ($i = 1; $i < $numArgs; ++$i) {
    if ($i > 1) {
      $not0Args .= ", ";
    }
    $not0Args .= "a$i";
  }
  print OUT "#define BOOST_FUNCTION_NOT_0_ARGS $not0Args\n";

  print OUT "\n";
  print OUT "#include <boost/function/function_template.hpp>\n";
  print OUT "\n";
  print OUT "#undef BOOST_FUNCTION_NOT_0_ARGS\n";
  print OUT "#undef BOOST_FUNCTION_NOT_0_PARMS\n";
  print OUT "#undef BOOST_FUNCTION_ARGS\n";
  print OUT "#undef BOOST_FUNCTION_PARMS\n";
  print OUT "#undef BOOST_FUNCTION_TEMPLATE_ARGS\n";
  print OUT "#undef BOOST_FUNCTION_TEMPLATE_PARMS\n";
  print OUT "#undef BOOST_FUNCTION_NUM_ARGS\n";
  print OUT "\n";
  print OUT "#endif // BOOST_FUNCTION_FUNCTION" . $numArgs . "_HEADER\n";
  close OUT;
}
