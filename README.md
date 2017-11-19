# Hestia

Hestia provides utilities and tools for improving software development process. Auto Build Checker (ABC) parses build results and makes HTML pages summarizing the build attempt. C++ Unit Test which allows people to easily make unit tests.

by Rich Sposato

Home Page: https://github.com/richsposato/Hestia

## CppUnitTest

### Design Considerations for C++ Unit Test Library

1. Simple        Library should be easy to compile, easy to setup within a host program,
		 easy to add new tests, and easy to parse test results.  New users should
                 be able to learn how to use library quickly.
2. Robust        Library should not crash, should provide strong exception-safety, and
                 internal objects within library should never break class invariants.
                 Also, library should provide accurate output even when host program 
                 crashes.  It also protects itself against reentrant calls from the host
                 program.
3. Portable      Source code should compile as cleanly as possible with at least 2
                 different compilers, and behave the same no matter which compiler is used.
4. Small         Library should have as tiny a memory footprint as possible.
5. Efficient     Big-O complexity of each function should be as small as possible.  The
                 unit-test library should not degrade runtime performance of host program.
6. Exit-Time     Host program should be able to exercise unit tests even at exit time.
7. Detailed      Output should provide as many details as possible.
8. Configurable  Library should allow user to decide which details go into output.
                 Library should provide multiple forms of output - standard out, text,
                 html, xml, or to any type of output the host program wants.
9. Long-Term     Library should allow for long-term tracking of test results.  (It does
                 this by adding overall test result totals to a "main" html page as well as
                 to the html page for the current test results.)
10. Stoppable    User should be able to skip remaining tests if a required test fails.
11. Independent  Library should not rely on other libraries, only functions and classes
                 mentioned in the C++ Standard.

### Design Considerations for Embedded C++ Unit Test Library

The embedded version of the unit test library has all the above design considerations
except that ...
1. It allows only limited Configurable output options. 
2. It does not send output to any standard error, standard output, or any text or html files.  Instead it sends output only to single listener provided by the host program, which may then send output to an html webpage or through a socket. 
3. The embedded version of this program has a much small memory footprint than the normal version of the library.

### Important Features

1. Configurable output options for output types provided by library.  Also allow host
   program to receive notices about test results so it can send results to other output
   formats.
2. Handles exceptions that occur in unit tests - and even allow host program to test
   functions where exceptions are expected for some input.
3. Provides strong exception safety safety for all internal operations, and protects itself
   from exceptions thrown within host program..
4. Has 3 levels of test severity (warning, checked, required).  If a required test fails,
   the program ends.  If a warning fails, the failure is logged, but the unit test still
   passes.
5. Can send output to stdout, stderr, text file, xml file, and html files simultaneously.
6. Can send text messages directly into output.
7. Host program can group tests together.
8. Provides summary page of all tests.
9. Can exercise unit tests even as program exits.
10. Allows host program to add observers to receive test result events.
11. Created a separate unit-test library for embedded software.
12. Allows host program to make summary table on demand instead of just at exit time.
13. Allows host program to run multiple sets of unit tests.
14. Protection against re-entrancy in case observer calls function in UnitTest library.
15. Efficient functions.
16. Guarantees that internal objects do not violate class invariants.


## Auto Build Checker

### Intent of Automated Build Checker.

ABC was designed to parse the output files from Visual Studio and summarize the results of the build process
in an HTML file.  It maintains records of previous build results in an HTML table so programmers can see how
well the build process has done in the past.  The intent is so programmers can display build results online
or at least in an internal webpage so everyone can see if any project failed and why.

### Features

1. Recognizes results from more compilers.
2. You can now specify options in a config file.
3. You can now specify custom rules for matching result files.
4. Comes with Code::Blocks project file.
5. You can now specify more options (colors, title, etc...).


## Dump

### Design Considerations for C++ Dump Library

1. Simple	  Library should be easy to compile, and easy to setup within a host program. New users should be able to learn how to use library quickly.
2. Small      Library should have as tiny a memory footprint as possible.
3. Efficient  Big-O complexity of each function should be as small as possible.
4. Polite	  Imposes as few as possible requirements on host program.
