#!/usr/bin/env perl

# Performance tester for the new IDL compiler
#


sub usage(){
    print <<EOT;
IDL C++ backend performance comparator.

Usage:

perftest.pl [-idldir <idl directory> ] [number of tests]

Tests are performed in reverse order of last modification. All tests
are performed unless an integer argument is read. In that case, only
the latest n tests are run.
EOT
}

my $numTests = -1;
# directory contains lots of lovely .idl files
$test_suite = "/home/djs/src/idl-test-suite";

my @args = @ARGV;
for ($i = 0; $i <= $#ARGV; $i++){
    if ($ARGV[$i] eq "-idldir"){
	if ($i == $#ARGV){
	    usage();
	    exit(1);
	}
	$test_suite = $ARGV[$i+1];
	$i++;
	next;
    }
    $numTests = $ARGV[$i];
}


#$old_compiler = "/home/djs/src/omniidl2/bin/i586_linux_2.0_glibc/omniidl3";
# omni3 develop cvs one:
$old_compiler = "/mnt/scratch/djs/cvs/build/i586_linux_2.0_glibc/omni/src/tool/omniidl2/omniidl3";
#$old_compiler = "/home/djs/src/omni/cvs/build/i586_linux_2.0_glibc/omni/src/tool/omniidl2/omniidl2";
$new_compiler = "/home/djs/src/omni/cvs/bin/scripts/omniidl";


opendir(DIR, $test_suite) or die("Couldn't find the test suite directory: $!");
@contents = readdir DIR;
closedir(DIR);

@test_idl = ();
foreach $idl (@contents){
    next if (($idl eq ".") or ($idl eq ".."));
    next if ($idl !~ /\.idl$/);
    push @test_idl, $idl;
}

die("Couldn't find executable for old compiler: $!") if (!(-x $old_compiler));
die("Couldn't find executable for new compiler: $!") if (!(-x $new_compiler));

# perform sort of tests into sensible order
#
my %times = ();
foreach $idl (@test_idl){
    my $time = (stat($idl))[9];
    $times{$idl} = $time;
}
my @sorted_test_idl = sort { $times{$b} <=> $times{$a} } @test_idl; 
    

$| = 1;


my $n = 5;

sub timecmd{
    my $command = shift;
    my $i;
    my $start = time();
    for ($i = 0; $i < $n; $i ++){
#	print "running [$command]\n";
	system($command);
#	print "finished\n";
    }
    my $end = time();

    return ($end - $start) / $n;
}

chdir($test_suite);
@failed = ();
foreach $idl (@sorted_test_idl){
    last if ($numTests == 0);
    $numTests --;

    $idl =~ /(.+)\.idl$/;
    $basename = $1;

    my $old = timecmd("$old_compiler $idl >/dev/null");
    my $new = timecmd("$new_compiler -bcxx $idl >/dev/null");

    print "$old, $new\n";
}

