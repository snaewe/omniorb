#!/usr/bin/env perl

# Regression tester for the new IDL compiler
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

# comparison command. Doesn't need to be overly whitespace sensitive
$compare_program = "/home/djs/src/omni/cvs/src/omniidl/python/be/cxx/tools/compare";
#$compare_program = "diff --ignore-all-space --ignore-blank-lines -u";

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

chdir($test_suite);
@failed = ();
foreach $idl (@sorted_test_idl){
    last if ($numTests == 0);
    $numTests --;

    print padto("\nFile: $idl",40);
    $idl =~ /(.+)\.idl$/;
    $basename = $1;
    # old compiler makes .hh etc files in cwd
    # FIXME: catch errors
    print jright("OLD",10);
    if (system("$old_compiler $idl 2>/dev/null") != 0){
	push @failed, $idl;
	print jright("[ \033[01;31mFAIL\033[0m ]",50);
	next;
    }
    print jright("NEW",10);
    if (system("$new_compiler -bcxx $idl > $basename.hh.new\n") != 0){
	push @failed, $idl;
	print jright("[ \033[01;31mFAIL\033[0m ]",40);
	next;
    }
    print jright("COMPARE", 10);
    $results = `$compare_program $basename.hh $basename.hh.new`;
    if ($results){
	# boo
	print jright("[ \033[01;31mFAIL\033[0m ]",30);
	push @failed, $idl;
    }else{
	# hooray
	print jright("[ \033[01;32mPASS\033[0m ]",30);
    }
}

if ($#failed == -1){
    print "\n\nAll tests successful.\n";
}else{
    print "\n\nThe following tests failed: @failed\n";
}
