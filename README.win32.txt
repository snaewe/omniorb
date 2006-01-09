                        omniORB on Win32 platforms.
                        ===========================

This file contains information on installing, building, and using
omniORB on Win32 (Windows NT and Windows '9x) platforms.

If you are upgrading from 2.7.1 or earlier, please read CHANGES_280
for a description of the incompatibility change in the semantics of
Any since 2.8.0.

omniORB has been tested with the following software configuration:

- Operating System  : Windows NT 4.0, 9x, 2000, XP
- Architecture      : x86 (and alpha in earlier versions)
- Compiler          : Visual C++ 5 and Visual C++ 6

The omniORB binaries, when compiled using VC++ 6.0 on Windows NT, will
execute on Windows '95 machines and vice-versa.


Roadmap
=======

When the omniORB4 distribution is unpacked, the following are created:

<Top-Level Directory>\                     : Directory where distribution was 
                                             unpacked

<Top-Level Directory>\doc\                 : omniORB Documentation

<Top-Level Directory>\include\             : Include files
<Top-Level Directory>\include\omniORB4\    : Include files for ORB run-time 
                                             library
<Top-Level Directory>\include\omnithread.h : Main omnithread include file
<Top-Level Directory>\include\omnithread\  : Include files for thread library

<Top-Level Directory>\src\                 : Source files
<Top-Level Directory>\src\lib\omniORB\     : Source files for ORB run-time 
                                             library
<Top-Level Directory>\src\lib\omnithread\  : Source files for thread library
<Top-Level Directory>\src\tool\omniidl\    : Source files for IDL Compiler
<Top-Level Directory>\src\appl\omniNames\  : Source files for COS Naming 
                                             Service
<Top-Level Directory>\src\appl\utils\      : Source files for utilities
<Top-Level Directory>\src\examples\        : Source for example programs


Installation
============

If you downloaded the Win32 binary distribution of omniORB,
ready-built binaries are provided. The binaries are compiled with VC++
6, 7, or 8, depending on the version you downloaded. If you are using
any other version of VC++, the binaries will not work, and you must
compile omniORB from source.

The executables and DLLs are in <Top-Level Directory>\bin\x86_win32.
The libraries are in            <Top-Level Directory>\lib\x86_win32.

You should set up your PATH environment to include 
   <Top-Level Directory>\bin\x86_win32
otherwise the DLLs will not be picked up when omniORB programs are run.

If you have the source-only distribution, you will need to build
omniORB. Please read the "Building omniORB from the source files"
section, below.  (If you want the Win32 binary distribution, but don't
have it, you can download it from SourceForge via
http://omniorb.sourceforge.net/download.html)


Libraries
=========

To link against the DLL versions of the omniORB libraries, you should
use the following libraries:

  omnithread32_rt.lib     -- omnithread library
  omniORB407_rt.lib       -- omniORB runtime library
  omniDynamic407_rt.lib   -- omniORB runtime library for dynamic features
  omniCodeSets407_rt.lib  -- extra code sets for string transformation
  omnisslTP40_rt.lib      -- SSL transport (if OpenSSL is available)
  COS407_rt.lib           -- stubs and skeletons for the COS service interfaces
  COSDynamic407_rt.lib    -- dynamic stubs for the COS service interfaces

If you are building your application code in debug mode, you MUST use
the debug versions of the omniORB libraries, otherwise you will get
assertion errors from the Visual C++ runtime about invalid heap
pointers. The debug DLL versions can be used by linking with the
_rtd.lib libraries instead of the _rt.lib libraries.


To link with static libraries, pick the libraries without _rt in their
names: omnithread.lib, omniORB4.lib, omniDynamic4.lib,
omniCodeSets4.lib, omnisslTP.lib, COS4.lib and COSDynamic4.lib.
Again, if you are compiling your application code in debug mode, you
MUST use the debug versions of the omniORB libraries, omnithreadd.lib,
omniORB4d.lib, etc.



Configuring the naming service
==============================
 
You have to configure the omniORB runtime and the naming service,
consult the user guides in ./doc for details. For a quick start,
follow these steps:

    o Make sure that <Top-Level Directory>\bin\x86_win32 is in your
      PATH environment.

    o Set the environment variable OMNINAMES_LOGDIR to a directory where
      the naming service omniNames can store its data. For example:
          set OMNINAMES_LOGDIR=C:\OMNINAMES

    o Start omniNames. The binary is in <Install Location>\bin\win32_x86. 
      For example:
         omniNames -start

    o Start the tool REGEDT32.EXE (on NT) or REGEDIT.EXE (on Windows '95). 
      Select the key HKEY_LOCAL_MACHINE\SOFTWARE\omniORB\InitRef (or
      create it if it doesn't exist). Add a string value (known as a
      REG_SZ data type when using REGEDT32) with name "1", value
      "NameService=corbaname::my.host.name" (putting the correct host
      name in, of course).

    o Due to a bug in some versions of Windows, omniORB may crash if
      you have no entries in the SOFTWARE\omniORB key other than the
      InitRef sub-key. If this happens, set the name InitRef in the
      SOFTWARE\omniORB key, rather than using a sub-key, or add at
      least one configuration parameter to the SOFTWARE\omniORB key.

    o The registry entries must be duplicated on all machines that
      will run omniORB programs. It is only necessary to run one
      instance of omniNames on your network.

    o To save manually editing registry entries, you can modify the
      sample.reg file to contain the configuration you require, then
      import the whole file into the registry.

    o Once you are satisfied the naming service is running properly.
      You can choose to setup omniNames to run as an NT service. See
      the description below.


Compiling the examples with nmake
=================================

Once the installation is completed. You can try compiling and running
the examples in <Top-Level Directory>\src\examples.

Just do the following:

  cd <Top-Level Directory>\src\examples
  nmake /f dir.mak

Have a look at the dir.mak file in <Top-Level Directory>\src\examples,
it should give you some idea about the compiler flags and libraries to
compile and link omniORB programs.


Building Projects using omniORB and Visual C++ 5/6
==================================================

Before building anything (or running the examples), you should refer
to the omniORB documentation. In particular, you must configure the
naming service as described above.

To configure your project to us the omniORB DLLs, follow these steps:

a) Add the stub (SK.cpp) files generated by the IDL compiler to the project 
   (Insert->"Files into Project")

b) Set up the search paths for include and library files: 

   1. Under Tools->Options, select the Directories tab.
   2. In the "Show directories for" box, select "Include files".
   3. Add the directory where you installed the omniORB include files to the 
       list (this is <Top-Level Directory>\include).
   4. In the "Show directories for" box, select "Library files".
   5. Add the directory where you installed the omniORB library files to the
       list (this is <Top-Level Directory\lib\x86_win32).
    
c) Set up macros and libraries:

    1. Under Build->Settings, select the "C/C++" tab.
    2. In the "Category" box, select "C++ Language". Tick the 
        "Enable exception handling" box.
    3. In the "Category" box, select "Code Generation". In the 
        "Use run-time library" box, select "Multithreaded DLL".
    4. **This is an important step.** 
       In the "Category" box, select "Preprocessor". In the
       "Preprocessor" box, add the macros

          __WIN32__,__x86__,_WIN32_WINNT=0x0400

       If this is NT 4.0, Windows 2000 or XP (or something later), add
       the macros __NT__ and __OSVERSION__=4 as well.
    
    5. Select the "Link" tab.

    6. In the "Category" box, select "Input". In the "Object/library
       modules" box, add the following libraries: ws2_32.lib,
       mswsock.lib, advapi32.lib, omniORB407_rt.lib,
       omniDynamic407_rt.lib, omnithread32_rt.lib

       If you are building a debug executable, the debug version of
       the libraries: omniORB407_rtd.lib, omniDynamic407_rtd.lib and
       omnithread32_rtd.lib should be used.

d) Your project is now set up, and you can build it.


Building Projects using omniORB and Visual C++ 7 
================================================

Before building anything (or running the examples), you should refer
to the omniORB documentation. In particular, you must configure the
naming service as described above.

To configure your project to us the omniORB DLLs, follow these steps:


a) Add the stub (SK.cpp) files generated by the IDL compiler to the project
   Project ->"Add existing Item")

b) Set up the search paths for include and library files: 

   1. Open Project Properties Window
   2. Under C++ -> General(Additonal Include Directories), Enter The 
      directory where you installed the omniORB include files 
      (this is <Top-Level Directory>\include).

   3. Under Linker -> General (Additional Library Directories), Enter the
      directory where you installed the omniORB library files (this is 
      <Top-Level>\lib\win-32)  

c) Set up macros and libraries:

    1. Under Project Properties ->Settings, Select the "C/C++" tab.
    2. Select The Code Generaion Tab:
    3. Set "Enable exception Handling" To Yes
    4. Set Run-Time Library To "Mulithreaded DLL". 
    5. **This is an important step.**  
       Select The Code Generaion Tab: (Under C++ in project properties)
 
       In the Preprocessor Definitions Box Add The Macros:

       __WIN32__,__x86__,_WIN32_WINNT=0x0400

       If this is NT 4.0, Windows 2000 or XP (or something later), add
       the macros __NT__ and __OSVERSION__=4 as well.

    6. Expand The Linker Tab.
    7. Select The Input Tab
    8. add The following Libraries to "Additional Dependencies"

       ws2_32.lib, mswsock.lib, advapi32.lib, omniORB407_rt.lib,
       omniDynamic407_rt.lib, omnithread32_rt.lib

       If you are building a debug executable, the debug version of
       the libraries: omniORB407_rtd.lib, omniDynamic407_rtd.lib and
       omnithread32_rtd.lib should be used.

  d)   Your project is now set up, and you can build it.     


Important:
==========

If you want to use the static versions of omniORB and omnithread, you
must add the macro _WINSTATIC (see step (c) 4* or 5#, above), and
replace the libraries omniORB407_rt.lib, omniDynamic407_rt.lib and
omnithread32_rt.lib with omniORB4.lib, omniDynamic4.lib and
omnithread.lib (see step (c) 6* or 8#, above).

* If Using VC 5/6
# If Using VC 7 



Known Problems
==============

When compiling the stub files generated by omniidl, you may come
across some bugs in Microsoft Visual C++. The bugs are to do with the
handling of nested classes. You may come across these bugs if you use
modules in your IDL files, and in certain other circumstances.

This release generates stub code that works around most of the MS VC++
bugs. However, it is not possible to avoid the bugs in all cases. In
particular, the following sample IDL will fail to compile:

// IDL

module A {
  struct B {
    long x;
  };
  enum C { C_1, C_2 };
  module D {
     struct B {
       float y;
     };
     // The stub for struct E would fail to compile
     struct E {
       A::B e1;
       B    e2;
     };
  };
};



Building omniORB from the source files
======================================

omniORB should be compiled using Visual C++ 6.0 or higher. It may work
with Visual C++ 5.


 A. Pre-requisites
    --------------

    The omniORB source tree requires the Cygwin utilities to build. It
    also requires the scripting language Python to be able to compile
    IDL to C++.

    Cygwin
    ------

    The full Cygwin distribution is freely available at:

       http://www.cygwin.com/


    The toolkit is big and you don't need all of it to compile
    omniORB. Alternatively, you can download a cutdown version from:

       http://omniorb.sourceforge.net/att_packages/gnu-win32-lite.zip

    The utilities in this version are all you need to build omniORB.

    Assume that you have downloaded the toolkit and have
    unpacked/installed it in some sensible subdirectory, say
    C:\gnuwin32, you then have to go through the following checklist:

      1. In a command prompt window, make sure that you have the
         environment variables and path setup properly to use MS
         Visual C++.

	 The easiest way is to call the vcvars.bat file created by the
	 Visual Studio installer.

      2. If you have unpacked gnu-win32-lite.zip in C:\gnuwin32, add
         C:\gnuwin32\bin to your Path environment variable. 

         Warning: If you have installed posix utilities from the
         Windows resource kits, make sure that C:\gnuwin32 is searched
         before the directory containing these utilities.

         If you have installed the full Cygwin toolkit, just follow
         its installation instructions and you can skip 3.

      3. Now you have to run a small script *once* to setup in your
         registry the necessary 'mount' points. Basically, it tells
         the gnuwin32 runtime how to translate a path such as /bin/sh
         to the real path name Win32 (e.g. C:\gnuwin32\bin\sh.exe)
         understands.

         Just execute this command in a command prompt window:
  
            C:\gnuwin32\bin\checkmounts C:\gnuwin32

        If all goes well, this is what you see:

            C:\> C:\gnuwin32\bin\checkmounts C:\gnuwin32
            no /bin/sh.exe, mounting c:\gnuwin32\bin as /bin
            Completed successfully.

      4. Now you are ready. You can now build the distribution by
         following the instructions below.

    Python
    ------

    omniidl requires Python 1.5.2 or later. You can download the full
    Python distribution from

     http://www.python.org/download/download_windows.html


    Alternatively, for Windows on x86, you can install a minimal
    version of Python which contains just the functionality required
    by omniidl. Download it from Sourceforge, via

       http://omniorb.sourceforge.net/att_packages/omnipython-x86_win32.zip

    Unpack the zip file at the top of the omniORB tree. It places
    files in the bin, lib and include directories.


  B. Choose the right platform configuration file
     --------------------------------------------

     Edit <top>\config\config.mk to select one of the following:

     platform = x86_nt_4.0
     platform = x86_nt_4.0_vs_7
     platform = x86_nt_4.0_vs_8
     platform = x86_nt_4.0_mingw
     platform = x86_nt_3.5
     platform = x86_win95

     For newer Windows operating systems, you should use x86_nt_4.0.


  C. Set the location of the Python interpreter
     ------------------------------------------

     Edit <top>\mk\platforms\<platform>.mk

     where <platform> is the platform you just chose in config.mk,
     e.g. <top>\mk\platforms\x86_nt_4.0.mk.

     If you are using the omnipython minimal distribution, uncomment
     the line which reads

       PYTHON = $(ABSTOP)/$(BINDIR)/omnipython

     otherwise, set PYTHON to the location of your Python executable.
     Note that you must use a Unix-style Cygwin path.

     If you are using the mingw compiler, you must patch the
     omnipython distribution using the patch in the patches/
     directory.


  D. Building and installing
     --------------------------

     Go into the directory <top>\src and type 'make export'. If all
     goes well:
        1. The executables and DLLs will be installed into
               <top>\bin\x86_win32\
        2. The libraries will be installed into
               <top>\lib\x86_win32\

     If you are using mingw to build omniORB, before you build the
     main distribution, you must build the omkdepend tool first by
     going to the directory <top>\src\tool\omkdepend and typing 'make
     export'.



Running omniNames as an NT service
==================================

  o Use srvany from the NT resource kit.

  o Follow the instructions provided with srvany to run a program as a
    service.

  o E.g. The binary of omniNames.exe is in C:\omniNames\omniNames.exe
         The log directory is set to C:\omniNames
         The standard error output is to be redirected to 
             C:\omniNames\omniNames.errlog

         Set the following registry parameters with the service applet:
            Application:   REG_SZ C:\omniNames\omniNames.exe

            AppParameters: REG_SZ -logdir C:\omniNames -errlog
                           C:\omniNames\omniNames.errlog

            AppDirectory:  REG_SZ C:\omniNames



Mailing List
============

There is a mailing list for discussing the use and development of
omniORB. See README.FIRST for details on subscribing.
