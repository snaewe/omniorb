#!/usr/bin/env perl

# -------------------------------------------------------
# Regression tester for the new IDL compiler
#
# Takes a set of input files and compares the output of
# the old (trusted) compiler with that of the new 
# (untrusted) compiler.



# -------------------------------------------------------
# CONFIGURATION DATA
# -------------------------------------------------------

# Location of the trusted old compiler binary
$old_compiler = "/mnt/scratch/djs/cvs/build/i586_linux_2.0_glibc/".
    "omni/src/tool/omniidl2/omniidl3";
$old_args = "";

# Location of the untrusted new compiler binary
$new_compiler = "/home/djs/src/omni/cvs/bin/scripts/omniidl";
$new_args = "-bcxx";

# Output compare command
$compare_program = "/home/djs/src/omni/cvs/src".
    "/omniidl/python/be/cxx/tools/compare";

# Default location of idl test data. Overridable with -idldir
# command line option
$test_suite = "/home/djs/src/idl-test-suite";

$tests = [ 
           ["", "", ""],
	   ["-tp",  "-Wbtp", "Tie Templates"],
           ["-tf",  "-Wbtf", "Flattened tie templates"],
           ["-a",   "-Wba",  "TypeCode and Any"] 
];


# ------------------------------------------------------
# Don't need to edit anything below this line
#

sub usage(){
    print <<EOT;
IDL C++ backend regression tester.

Usage:

regress.pl [-idldir <idl directory> ] [number of tests]

Tests are performed in reverse order of last modification. All tests
are performed unless an integer argument is read. In that case, only
the latest n tests are run.
EOT
}

my $numTests = -1;

# grab the -idldir argument if available
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
die("Couldn't find executable for compare program: $!") 
    if (!(-x $compare_program));

# perform sort of tests into sensible order
#
my %times = ();
foreach $idl (@test_idl){
    my $time = (stat($idl))[9];
    $times{$idl} = $time;
}
my @sorted_test_idl = sort { $times{$b} <=> $times{$a} } @test_idl; 
    

$| = 1;

print <<EOT;
IDL compiler regression tester.
Old compiler set to:    "$old_compiler"
New compiler set to:    "$new_compiler"
Compare command set to: "$compare_program"
Location of IDL set to: "$test_suite"
EOT

if ($numTests == -1){
    print "Performing -ALL- tests\n";
}else{
    print "Performing $numTests most recent tests\n";
}

sub padto{
    my $text = shift;
    my $level = shift;
    my $need = $level - length($text);
    return $text.(" "x$need);
}

sub jright{
    my $text = shift;
    my $space = shift;
    return " "x($space - length($text)) . $text;
}

sub member{
    my $x = shift;
    my @list = @_;
    foreach $y (@list){
	return 1 if ($x eq $y);
    }
    return 0;
}

chdir($test_suite);
if (!-e "$test_suite/old") { mkdir("$test_suite/old", 0775); }
if (!-e "$test_suite/new") { mkdir("$test_suite/new", 0775); }

my $RED = "\033[01;31m";
my $GREEN = "\033[01;32m";
my $YELLOW = "\033[01;33m";
my $RESET = "\033[0m";

%failed = ();
sub getFailList{
    my $key = shift;
    if (!$failed{$key}){
	$failed{$key} = [];
    }
    return $failed{$key};
}

my $numFailed = 0;
my $numTried = 0;
my $totalTests = 0;
foreach $idl (@sorted_test_idl){
    last if ($numTests == 0);
    $numTests --;
    $numTried ++;

    foreach $test (@$tests){
	$totalTests ++;

	my @options = @$test;
	if (!$options[0]){
	    print padto("\nFile: $idl",40);
	}else{
	    print padto("\n  $options[2]", 40);
	}
	$old_opt = $options[0];
	$new_opt = $options[1];
	$idl =~ /(.+)\.idl$/;
	$basename = $1;

	chdir("$test_suite/old");
	print jright("OLD",10);
	if (system("$old_compiler $old_args $old_opt ../$idl 2>/dev/null") != 0){
	    my $failures = getFailList("Old compiler");
	    push @$failures, $idl;
	    print jright("[ ".$RED."FAIL".$RESET." ]",20);
	    $numFailed ++;
	    next;
	}

	chdir("$test_suite/new");
	print jright("NEW",10);
	if (system("$new_compiler $new_args $new_opt ../$idl 2>/dev/null") != 0){
	    my $failures = getFailList("New compiler");
	    push @$failures, $idl;
	    print jright("[ ".$RED."FAIL".$RESET." ]",25);
	    $numFailed ++;
	    next;
	}

	chdir("$test_suite");
	foreach $suffix (".hh", "SK.cc", "DynSK.cc"){
	    my $failures = getFailList($suffix);
	    if (!-e "old/$basename$suffix"){
		# the old system didn't do anything, this is ok :)
		print jright("", 30);
		next;
	    }elsif (!-e "new/$basename$suffix"){
		print jright("[ ".$YELLOW.$suffix.$RESET." ]", 30);
		
	    #if (!member($idl, @failures)){
	    #push @$failures, $idl;
	    #}
		$numFailed ++;
		next;
	    }
	    if (`$compare_program old/$basename$suffix new/$basename$suffix`){
		print jright("[ ".$RED.$suffix.$RESET." ]", 30);
		push @$failures, $idl;
		$numFailed ++;
	    }else{
		print jright("[ ".$GREEN.$suffix.$RESET." ]", 30);
	    }
	}
    }
}

if ($numFailed == 0){
    print "\n\nAll tests successful.\n";
}else{
    print "\n\nThe following tests failed:\n";
    foreach $key (keys(%failed)){
	my $failures = $failed{$key};
	my @f = @$failures;
	if (@$failures){
	    print padto("[ ".($#f+1)." : $key ]", 20);
	    print "-->\t  @$failures\n";
	}
    }
}

if ($numTried == ($#sorted_test_idl+1)){
    open(FILE, ">>$test_suite/results.txt");
    print FILE "$totalTests ";
    my $hh_ = getFailList(".hh"); my @hh  = @$hh_;
    my $sk_ = getFailList("SK.cc"); my @sk = @$sk_;
    my $dyn_ = getFailList("DynSK.cc"); my @dyn = @$dyn_;
    print FILE ($#hh+1)." ".($#sk+1)." ".($#dyn+1)."  ";
    print FILE time();
    print FILE "\n";
    close(FILE);
}

