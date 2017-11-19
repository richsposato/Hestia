// $Header: /cvsroot/hestia/BuildChecker/main.cpp,v 1.26 2008/12/13 00:22:40 rich_sposato Exp $

/// @file main.cpp Entry point for BuildChecker console program.

/* --------------------------------------------------------------------------------------------

Auto Build Checker  (a part of the Hestia project)
http://sourceforge.net/projects/hestia/

Copyright (c) 2007, 2008 by Rich Sposato

Permission to use, copy, modify, distribute and sell this software for any purpose is hereby
granted without fee, provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in supporting documentation.

The author makes no representations about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

------------------------------------------------------------------------------------------- */


// Included Files -----------------------------------------------------------------------------

#include <time.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "command.hpp"
#include "utility.hpp"
#include "matchers.hpp"
#include "outputters.hpp"


// Description --------------------------------------------------------------------------------

/** @par Automated Build Checker
  Parses through compiler build results text output file to create html files.
 */

/** @par Possible Policies
   These policies could be specified on the command line or in a config file.
 - Show skipped / up-to-date projects as passed.  Or ignore skipped and up-to-date projects.
 - Allow user to specify a caption for each table.
 - Allow user to choose if color legend table is shown.
 - Allow user to choose which tables are shown.
 - Make it ignore warnings.  (not show them or count them)
 - Make links within current build result page.
 - Copy build-result input file to output directory, and rename it to have date in name.
 - Add link from current build result page to input file.
 - Remove old build-result files when corresponding rows are truncated from long-term file.
 - Make a command line option to specify ways to recognize error/warnings lines.
 */


// Namespace Declarations ---------------------------------------------------------------------

using namespace std;

namespace hestia
{


// DoesFileExist ------------------------------------------------------------------------------

bool DoesFileExist( const char * filename )
{
    if ( IsEmptyString( filename ) )
        return false;
	struct stat data;
	const int result = ::stat( filename, &data );
	const bool exists = ( 0 == result );
	return exists;
}

// CheckForSameFileName -----------------------------------------------------------------------

void CheckForSameFileName( string & filename )
{
    if ( !DoesFileExist( filename.c_str() ) )
        return;

    unsigned int count = 0;
    char buffer[ 256 ];
    assert( 6 < filename.size() );
    const unsigned int baseSize = filename.size() - 5;
    assert( baseSize + 20 < sizeof(buffer) );
    ::strncpy( buffer, filename.c_str(), baseSize );
    char * place = buffer + baseSize;
    bool found = false;

    do
    {
        ++count;
        ::sprintf( place, "_%u.html", count );
    } while ( DoesFileExist( buffer ) );

    filename = buffer;
}

// MakeCurrentFileName ------------------------------------------------------------------------

/** Creates file name for current output file using timestamp and path to main file.
 @param today [input] Current timestamp.
 @param current [output] Name of file which will display current build results.
 @param mainPage [input] Name of file which stores longterm summary of build results.
 @return False for error, true for success.
 */
bool MakeCurrentFileName( const struct tm * today, string & current, const char * mainPage )
{

    current.clear();
    if ( nullptr == today )
    {
        cerr << "Internal error!  Pointer to timestamp is NULL!" << endl;
        return false;
    }
    if ( IsEmptyString( mainPage ) )
    {
        cerr << "Error!  Name of main html file is invalid!" << endl;
        return false;
    }

    const char * mainFile = FindFileNameInPath( mainPage );
    if ( IsEmptyString( mainFile ) )
    {
        cerr << "Error!  Could not find name of main file!" << endl;
        return false;
    }
    assert( mainPage <= mainFile );
    string temp( mainPage, mainFile - mainPage );

    char buffer[ 128 ];
    ::strftime( buffer, sizeof(buffer)-1, "%Y_%m_%d_%H_%M_%S", today );
    temp.append( "build_result_" );
    temp.append( buffer );
    temp.append( ".html" );

    // Now check if any files have the same name.  (i.e. - same timestamp)
    CheckForSameFileName( temp );
    temp.swap( current );

    return true;
}

// --------------------------------------------------------------------------------------------

/// @class BuildCheckerFailures Stores enum family for describing program failures.
class BuildCheckerFailures
{
public:
    enum FailureTypes
    {
        Success = 0,
        InvalidArgs,
        EmptyInputFile,
        CantMakeFileNames,
        CantReadInputFile,
        CantReadMainPageFile,
        CantOpenOutputFile,
        CantParseInputFile,
        CantMakeMainPageFile,
        CantAddToMainPageFile,
        StdException,
        UnknownException,
        UnknownError
    };
};

// SetupColors --------------------------------------------------------------------------------

void SetupColors( const CommandLineArgs & args )
{
    Colors & colors = Colors::GetIt();
    if ( nullptr != args.GetColorBackground() )
        colors.SetColor( Colors::Background, args.GetColorBackground() );
    if ( nullptr != args.GetColorText() )
        colors.SetColor( Colors::Text, args.GetColorText() );
    if ( nullptr != args.GetColorPasses() )
        colors.SetColor( Colors::Passes, args.GetColorPasses() );
    if ( nullptr != args.GetColorWarnings() )
        colors.SetColor( Colors::Warnings, args.GetColorWarnings() );
    if ( nullptr != args.GetColorErrors() )
        colors.SetColor( Colors::Errors, args.GetColorErrors() );
}

// --------------------------------------------------------------------------------------------

} // end namespace hestia

using namespace hestia;

// DoWork -------------------------------------------------------------------------------------

int DoWork( const CommandLineArgs & args )
{

    const char * inputFilename  = args.GetInputFile();
    const char * mainPageFilename = args.GetOutputFile();
    const char * buildName = args.GetBuildName();
    const char * buildType = args.GetBuildType();
    const char * title = args.GetTitle();
    unsigned int keepCount = args.GetKeepCount();
    assert( !IsEmptyString( inputFilename ) );
    assert( !IsEmptyString( mainPageFilename ) );
    assert( !IsEmptyString( buildName ) );
    assert( !IsEmptyString( buildType ) );
    assert( !IsEmptyString( title ) );

    string fileContent;
    if ( !ReadFileIntoString( inputFilename, fileContent ) )
    {
        cerr << "Error!  Could not open input file: " << inputFilename << endl;
        return BuildCheckerFailures::CantReadInputFile;
    }
    if ( fileContent.size() == 0 )
    {
        cerr << "Error!  No content in input file: " << inputFilename << ". Is file empty?" << endl;
        return BuildCheckerFailures::EmptyInputFile;
    }

    time_t timestamp;
    time( &timestamp );
    const struct tm * today = ::localtime( &timestamp );

    string currentFileName;
    if ( !MakeCurrentFileName( today, currentFileName, mainPageFilename ) )
    {
        cerr << "Error!  Could not make output file names." << endl;
        return BuildCheckerFailures::CantMakeFileNames;
    }

    const char * currentFile = currentFileName.c_str();
    OutputMaker output( buildName, buildType, currentFile, mainPageFilename );

    if ( !output.IsOpen() )
    {
        cerr << "Error!  Could not open output file: " << currentFileName << endl;
        return BuildCheckerFailures::CantOpenOutputFile;
    }

    MatcherSet matcher( args.GetCompiler() );
    const MatcherSet::MatchingRules & rules = args.GetMatchingRules();
    if ( 0 < rules.size() )
    {
        matcher.AddMatchingRules( rules );
    }

    if ( !output.Process( matcher, today, inputFilename, fileContent.c_str() ) )
    {
        cerr << "Error!  Could not produce output file: " << inputFilename << endl;
        return BuildCheckerFailures::CantParseInputFile;
    }

    const bool read_file = ReadFileIntoString( mainPageFilename, fileContent );
    MainPageMaker mainPage( title, buildName, buildType, today, matcher,
        mainPageFilename, currentFile, keepCount+1 );
    if ( read_file )
    {
        if ( !mainPage.AddToContent( fileContent ) )
        {
            cerr << "Error!  Could not add content to main summary file: "
                 << mainPageFilename << endl;
            return BuildCheckerFailures::CantAddToMainPageFile;
        }
    }
    else if ( !mainPage.CreateFile() )
    {
        cerr << "Error!  Could not add create long-term file: " << mainPageFilename << endl;
        return BuildCheckerFailures::CantMakeMainPageFile;
    }

    return BuildCheckerFailures::Success;
}

// main ---------------------------------------------------------------------------------------

int main( unsigned int argc, const char * const argv[] )
{

    int result = BuildCheckerFailures::UnknownError;
    try
    {
        const CommandLineArgs args( argc, argv );
        const char * exeName = args.GetExeName();
        if ( !args.IsValid() || args.ShowHelp() )
        {
            ShowUsageMessage( exeName );
            result = ( args.IsValid() ) ? BuildCheckerFailures::Success : BuildCheckerFailures::InvalidArgs;
        }
        else
        {
            SetupColors( args );
            result = DoWork( args );
        }
    }
    catch ( const ::std::exception & ex )
    {
        cerr << "Build checker stopped because of exception! " << ex.what() << endl;
        result = BuildCheckerFailures::StdException;
    }
    catch ( ... )
    {
        cerr << "Build checker stopped because of unknown exception!" << endl;
        result = BuildCheckerFailures::UnknownException;
    }

    return result;
}

// --------------------------------------------------------------------------------------------

// $Log: main.cpp,v $
// Revision 1.26  2008/12/13 00:22:40  rich_sposato
// Added 2 functions to deal with situation when two output files have the
// same name and timestamp.
//
// Revision 1.25  2008/12/11 20:50:24  rich_sposato
// Moved command-line arg parsing class to separate file.  Added ability to
// get args from a config file instead of just command line.
//
// Revision 1.24  2008/11/21 21:31:24  rich_sposato
// Added seconds to timestamp.
//
// Revision 1.23  2008/11/21 00:35:13  rich_sposato
// Minor changes.
//
// Revision 1.22  2008/11/20 23:19:31  rich_sposato
// Added ability for user to specify colors on the command line.
//
// Revision 1.21  2008/11/19 00:48:46  rich_sposato
// Added some asserts and error messages.
//
// Revision 1.20  2008/11/18 21:40:06  rich_sposato
// Changed names to emphasize that this deals with main file.
//
// Revision 1.19  2008/11/18 21:21:18  rich_sposato
// Changed formatting of usage message.  Changed command line arg to require
// output file instead of output directory.
//
// Revision 1.18  2008/11/18 20:17:51  rich_sposato
// Error messages now go to cerr instead of cout.
//
// Revision 1.17  2008/11/18 20:01:06  rich_sposato
// Clarified usage message with regards to build type.
//
// Revision 1.16  2008/11/18 18:23:19  rich_sposato
// Changed so program can show compiler name in output files.
//
// Revision 1.15  2008/11/18 17:37:34  rich_sposato
// Added title for long-term page. Fixed minor bug in parsing command line.
//
// Revision 1.14  2008/11/18 00:42:55  rich_sposato
// Added build type to command line and to output.
//
// Revision 1.13  2008/11/17 22:53:36  rich_sposato
// Minor change for keep-count.
//
// Revision 1.12  2008/11/17 21:50:16  rich_sposato
// Moved outputter classes from main.cpp to separate files.
//
// Revision 1.11  2008/11/17 21:00:14  rich_sposato
// Moved some classes and functions from main.cpp to a separate files.
//
// Revision 1.10  2008/11/17 18:53:49  rich_sposato
// Added ability to keep all previous results.
//
// Revision 1.9  2008/11/17 18:39:38  rich_sposato
// First steps towards making this program able to parse results from other
// compilers.
//
// Revision 1.8  2008/11/15 01:49:17  rich_sposato
// Updated usage message.
//
// Revision 1.7  2008/11/15 01:35:53  rich_sposato
// Changed how command line options get parsed.
//
// Revision 1.6  2008/11/15 00:31:44  rich_sposato
// Added documentation comments.  Minor coding changes.  Some style changes.
//
// Revision 1.5  2008/09/23 18:57:23  rich_sposato
// Minor changes to make code more portable.
//
// Revision 1.4  2008/06/19 07:13:05  rich_sposato
// Added const_cast's since GCC version of is_open is non-const.
//
// Revision 1.3  2008/05/19 06:08:37  rich_sposato
// Added copyright notice.  Changed name of namespace.  Minor fix.
//
// Revision 1.2  2008/05/15 20:09:15  rich_sposato
// Added keep-count to say how many prior results to keep around.
//
// Revision 1.1.1.1  2008/05/02 05:40:14  rich_sposato
// Initial Import
//
