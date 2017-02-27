#!/usr/bin/perl -w

use strict;
use Cwd;


my $filename = "BFXMMake";
my $debug = 0;


sub traverse($);

traverse(".");



sub runcmd($) {
    my ($cmd) = @_;
    print $cmd, "\n";
    if (!$debug) {
	system $cmd;
    }
}

sub runmesh() {
    return unless -f $filename;
    print "Processing ", getcwd, "/$filename\n";
    open MAKE, $filename or die("open in " . getcwd . " failed: $!\n");

    while (<MAKE>) {
	my ($output, $rest, @inputs, $i);
	chomp;
	s/
$//;
	($output, $rest) = split /:/, $_, 2;
	@inputs = split /,/, $rest;

	runcmd("mesher " . $inputs[0] . " " . $output . " xbc");
	for ($i = 1; $i < @inputs; $i++) {
	    runcmd("mesher " . $inputs[$i] . " " . $output . " xba");
	}
    }

    close MAKE;
}

sub traverse($) {
    my ($dir) = @_;
    my (@dirs);

    chdir $dir or die "chdir $dir: $!\n";

    opendir DIR, "." or die "opendir: $!\n";
    @dirs = grep { $_ ne "." && $_ ne ".." && -d "$_" } readdir(DIR);
    closedir DIR;

    runmesh();

    foreach $dir (@dirs) {
	next if -l $dir;
	traverse($dir);
    }

    chdir ".." or die "chdir ..: $!\n";
}
