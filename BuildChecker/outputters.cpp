// $Header: /cvsroot/hestia/BuildChecker/outputters.cpp,v 1.17 2008/12/18 23:10:16 rich_sposato Exp $

/// @file outputters.cpp Defines classes used to make output files.

/* --------------------------------------------------------------------------------------------

Auto Build Checker  (a part of the Hestia project)
http://sourceforge.net/projects/hestia/

Copyright (c) 2008 by Rich Sposato

Permission to use, copy, modify, distribute and sell this software for any purpose is hereby
granted without fee, provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in supporting documentation.

The author makes no representations about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

------------------------------------------------------------------------------------------- */


// Included Files -----------------------------------------------------------------------------

#include "outputters.hpp"

#include <assert.h>
#include <time.h>
#include <iostream>

#include "utility.hpp"
#include "matchers.hpp"


// Namespace Declarations ---------------------------------------------------------------------

using namespace std;

namespace hestia
{


// MakeColorTable -----------------------------------------------------------------------------

/** Puts html table into output stream.  The table describes which colors are used for which
 purpose in the html file.
 @return True to indicate no problems.
 */
bool MakeColorTable( ofstream & output )
{

    const Colors & colors = Colors::GetIt();
    const char * colorForError   = colors.GetColor( Colors::Errors   );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses  = colors.GetColor( Colors::Passes   );
    const char * colorForInfo    = colors.GetColor( Colors::Text     );

    output << "<table border=1 cellspacing=0 cellpadding=3>" << endl;
    output << "<caption><em>Legend</em></caption>" << endl;
    output << "<tr><th>Color</th><th>Purpose</th></tr>" << endl;
    output << "<tr><td>" << colorForInfo << "</td><td>information / not built</td></tr>" << endl;
    output << "<tr><td><span style=\"color: " << colorForPasses << "\">" << colorForPasses << "</span></td>"
           <<     "<td><span style=\"color: " << colorForPasses << "\">pass / no errors</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: " << colorForWarning << "\">" << colorForWarning << "</span></td>"
           <<     "<td><span style=\"color: " << colorForWarning << "\">warnings</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: " << colorForError << "\">" << colorForError << "</span></td>"
           <<     "<td><span style=\"color: " << colorForError << "\">failure / errors</span></td></tr>" << endl;
    output << "</table><br>" << endl;

    return true;
}

// OutputMaker::OutputMaker ------------------------------------------------------------------

OutputMaker::OutputMaker( const char * buildName, const char * buildType,
    const char * outputFilename, const char * longtermFilename ) :
    m_output( outputFilename ),
    m_streamer(),
    m_projects( nullptr ),
    m_longtermFilename( FindFileNameInPath( longtermFilename ) ),
    m_buildName( buildName ),
    m_buildType( buildType )
{
    assert( nullptr != this );
}

// OutputMaker::~OutputMaker ------------------------------------------------------------------

OutputMaker::~OutputMaker( void )
{
    assert( nullptr != this );
}

// OutputMaker::IsOpen ------------------------------------------------------------------------

bool OutputMaker::IsOpen( void ) const
{
    assert( nullptr != this );
#if defined( _MSC_VER )
    return m_output.is_open();
#else
    OutputMaker * pThis = const_cast< OutputMaker * >( this );
    return pThis->m_output.is_open();
#endif
}

// OutputMaker::Process -----------------------------------------------------------------------

bool OutputMaker::Process( MatcherSet & matcher, const struct tm * today,
    const char * inputFilename, const char * content )
{
    assert( nullptr != this );
    assert( nullptr != today );
    assert( !IsEmptyString( inputFilename ) );
    assert( !IsEmptyString( content ) );

    m_projects = matcher.GetProjectMatcher();
    assert( nullptr != m_projects );

    if ( IsEmptyString( content ) )
    {
        cerr << "Content string from input file is empty." << endl;
        return false;
    }
    if ( !IsOpen() )
    {
        cerr << "Could not open output file." << endl;
        return false;
    }
    if ( !MakeHeader( matcher, today, inputFilename ) )
    {
        cerr << "Could not make header for file." << endl;
        return false;
    }
    if ( !ParseContents( matcher, content ) )
    {
        cerr << "Could not parse content from input file file." << endl;
        return false;
    }
    m_projects->UpdateTotalCounts();
    if ( !MakeSummaryTable() )
        return false;
    if ( !m_projects->MakeProjectTable( m_output ) )
    {
        cerr << "Could not make project table in output file." << endl;
        return false;
    }
    if ( !MakeColorTable( m_output ) )
    {
        cerr << "Could not make color table in output file." << endl;
        return false;
    }
    m_projects->OutputProjectInfo( m_output );
    if ( !OutputContents() )
    {
        cerr << "Could not send contents to output file." << endl;
        return false;
    }
    if ( !Last() )
    {
        cerr << "Could not send final info to output file." << endl;
        return false;
    }

    m_streamer.str().clear();
    m_output.close();

    return true;
}

// OutputMaker::MakeHeader --------------------------------------------------------------------

bool OutputMaker::MakeHeader( const MatcherSet & matcher, const struct tm * today,
    const char * inputFilename )
{
    assert( nullptr != this );

    const Colors & colors = Colors::GetIt();
    const char * colorForInfo = colors.GetColor( Colors::Text       );
    const char * colorForBack = colors.GetColor( Colors::Background );

    char buffer[ 128 ];
    ::strftime( buffer, sizeof(buffer)-1, "%Y_%m_%d_%H_%M_%S", today );
    m_output << "<html><head><title>" << m_buildName << ' ' << m_buildType << "Build Results "
             << buffer << "</title></head>" << endl << endl
             << "<body bgcolor=" << colorForBack << " text=" << colorForInfo << '>' << endl;
    ::strftime( buffer, sizeof(buffer)-1, "%A, %Y - %B - %d, %H : %M : %S", today );
    m_output << "<a href=\'" << m_longtermFilename
             << "\'>Main Build Result Page</a><br><br>" << endl << endl;
    m_output << "Build Name: " << m_buildName << "<br>" << endl;
    m_output << "Build Type: " << m_buildType << "<br>" << endl;
    m_output << "Compiler: " << matcher.GetCompiler() << "<br>" << endl;
    m_output << "Start Time: " << buffer << "<br>" << endl;
    m_output << "From build results file: " << inputFilename
             << "<br>" << endl << endl;

    return true;
}

// OutputMaker::ParseContents -----------------------------------------------------------------

bool OutputMaker::ParseContents( MatcherSet & matcher, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    const char * nextLine = GetNextLine( content );
    bool hasNextLine = !IsEmptyString( nextLine );
    while ( hasNextLine )
    {
        assert( nullptr != nextLine );
        const unsigned int lineSize = static_cast< unsigned int >(
            ( nextLine - content ) - 1 );
        const string currentLine( content, lineSize );
        const char * line = currentLine.c_str();
        matcher.MatchLine( line, m_streamer );
        hasNextLine = !IsEmptyString( nextLine );
        if ( hasNextLine )
        {
            content = nextLine;
            nextLine = GetNextLine( content );
        }
    }

    return true;
}

// OutputMaker::MakeSummaryTable --------------------------------------------------------------

bool OutputMaker::MakeSummaryTable( void )
{
    assert( nullptr != this );

    const unsigned int warnCount       = m_projects->GetWarningCount();
    const unsigned int errorCount      = m_projects->GetErrorCount();
    const unsigned int project_fails   = m_projects->GetFailCount();
    const unsigned int projectCount    = m_projects->GetProjectCount();
    const unsigned int skippedCount    = m_projects->GetSkippedCount();
    const unsigned int warnPassCount   = m_projects->GetWarnPassCount();
    const unsigned int passed_projects = m_projects->GetCleanPassCount();
    const unsigned int up_to_dateCount = m_projects->GetUpToDateCount();
    const bool anyErrors = ( 0 < errorCount );
    const bool anyWarnings = ( 0 < warnCount );
    const bool noProjects = ( 0 == projectCount );
    const Colors & colors = Colors::GetIt();
    const char * colorForError   = colors.GetColor( Colors::Errors   );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses  = colors.GetColor( Colors::Passes   );
    const char * const buildResult = anyErrors ? "FAILED" : noProjects ? "EMPTY!" : "Passed";
    const char * const errorColor = anyErrors || noProjects ? colorForError : colorForPasses;
    const char * const buildColor = anyErrors || noProjects ? colorForError :
        anyWarnings ? colorForWarning : colorForPasses;
    const char * const warnColor = anyWarnings ? colorForWarning : colorForPasses;

    m_output << "<br><table border=1 cellspacing=0 cellpadding=3>" << endl;
    m_output << "<caption><em>Overall Build Results</em></caption>" << endl;

    m_output << "<tr><td><span style=\"color: " << errorColor
             << "\">Total Errors</span></td><td><span style=\"color: " << errorColor
             << "\">" << errorCount << "</span></td></tr>" << endl;

    m_output << "<tr><td><span style=\"color: " << warnColor
             << "\">Total Warnings</span></td><td><span style=\"color: " << warnColor
             << "\">" << warnCount << "</span></td></tr>" << endl;

    m_output << "<tr><td><span style=\"color: " << errorColor
             << "\">Failed Projects</span></td><td><span style=\"color: " << errorColor
             << "\">" << project_fails << "</span></td></tr>" << endl;

    m_output << "<tr><td><span style=\"color: " << colorForPasses << "\">Passed Cleanly Projects</span></td>"
             << "<td><span style=\"color: " << colorForPasses << "\">" << passed_projects
             << "</span></td></tr>" << endl;

    m_output << "<tr><td><span style=\"color: " << colorForWarning << "\">Passed With Warning Projects:</span></td>"
             << "<td><span style=\"color: " << colorForWarning << "\">" << warnPassCount
             << "</span></td></tr>" << endl;

    m_output << "<tr><td>Already Up-To-Date Projects:</td><td>" << up_to_dateCount
             << "</td></tr>" << endl;
    m_output << "<tr><td>Skipped Projects:</td><td>" << skippedCount
             << "</td></tr>" << endl;

    m_output << "<tr><td>Total Projects</td><td>"
             << projectCount << "</td></tr>" << endl;

    m_output << "<tr><td><span style=\"color: " << buildColor
             << "\">Build Results</span></td><td><span style=\"color: " << buildColor
             << "\">" << buildResult << "</span></td></tr>" << endl;

    m_output << "</table><br>" << endl;

    return true;
}

// OutputMaker::OutputContents ----------------------------------------------------------------

bool OutputMaker::OutputContents( void )
{
    assert( nullptr != this );
    m_output << m_streamer.str().c_str();
    return true;
}

// OutputMaker::Last --------------------------------------------------------------------------

bool OutputMaker::Last( void )
{
    assert( nullptr != this );
    m_output << "</body>" << endl << "</html>" << endl;
    return true;
}

// MainPageMaker::MainPageMaker ---------------------------------------------------------------

MainPageMaker::MainPageMaker( const char * title, const char * buildName,
    const char * buildType, const struct tm * today, const MatcherSet & matcher,
    const char * longtermFilename, const char * outputFilename, unsigned int keepCount ) :
    m_matcher( matcher ),
    m_output( longtermFilename ),
    m_outputFilename( FindFileNameInPath( outputFilename ) ),
    m_buildName( buildName ),
    m_buildType( buildType ),
    m_title( title ),
    m_today( today ),
    m_keepCount( keepCount )
{
    assert( nullptr != this );
}

// MainPageMaker::~MainPageMaker ---------------------------------------------------------------

MainPageMaker::~MainPageMaker( void )
{
    assert( nullptr != this );
}

// MainPageMaker::IsOpen ----------------------------------------------------------------------

bool MainPageMaker::IsOpen( void ) const
{
    assert( nullptr != this );
#if defined( _MSC_VER )
    return m_output.is_open();
#else
    MainPageMaker * pThis = const_cast< MainPageMaker * >( this );
    return pThis->m_output.is_open();
#endif
}

// MainPageMaker::AddToContent ----------------------------------------------------------------

bool MainPageMaker::AddToContent( string & fileContent )
{
    assert( nullptr != this );

    const char * content = fileContent.c_str();
    assert( !IsEmptyString( content ) );
    // This should find start of the header row ...
    const char * firstRow = FindSubString( content, "<tr>" );
    if ( IsEmptyString( firstRow ) )
        return CreateFile();

    ++firstRow;
    // and this should find start of the most recently added row.
    firstRow = FindSubString( firstRow, "<tr>" );
    if ( IsEmptyString( firstRow ) )
        return CreateFile();

    string temp( content, firstRow );
    m_output << temp;
    if ( !AddCurrentOutput() )
        return false;

    // Decide if table in file already has more than keep-count rows.
    const char * rowNow = firstRow;
    const char * nextRow = rowNow;
    const char * end_of_table = FindSubString( nextRow, "</table>" );
    unsigned int count = 1;
    while ( ( count < m_keepCount ) || ( 0 == m_keepCount ) )
    {
        nextRow = FindSubString( rowNow, "<tr>" );
        if ( ( end_of_table < nextRow ) || IsEmptyString( nextRow ) )
        {
            // next row is past end of current table, so it must be inside next table.
            break;
        }
        rowNow = nextRow+4;
        ++count;
    }

    if ( ( count < m_keepCount ) || IsEmptyString( nextRow ) )
        m_output << firstRow;
    else
    {
        // Now add other rows from the table.
        temp.assign( firstRow, nextRow-firstRow );
        m_output << temp;
        m_output << end_of_table;
    }

    return true;
}

// MainPageMaker::AddCurrentOutput ------------------------------------------------------------

bool MainPageMaker::AddCurrentOutput( void )
{
    assert( nullptr != this );

    char buffer[ 128 ];
    const ProjectMatcher * projects = m_matcher.GetProjectMatcher();
    assert( nullptr != projects );
    const unsigned int projectCount   = projects->GetProjectCount();
    const unsigned int failCount      = projects->GetFailCount();
    const unsigned int cleanPassCount = projects->GetCleanPassCount();
    const unsigned int warnPassCount  = projects->GetWarnPassCount();
    const unsigned int warnCount      = projects->GetWarningCount();
    const unsigned int errorCount     = projects->GetErrorCount();
    const unsigned int skippedCount   = projects->GetSkippedCount();
    const unsigned int uptodateCount  = projects->GetUpToDateCount();

    const Colors & colors = Colors::GetIt();
    const char * colorForError   = colors.GetColor( Colors::Errors   );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses  = colors.GetColor( Colors::Passes   );

    const bool anyErrors = ( 0 < errorCount );
    const bool anyWarnings = ( 0 < warnCount );
    const bool noProjects = ( 0 == projectCount );
    const char * const buildResult = anyErrors ? "FAILED" : noProjects ? "EMPTY!" : "Passed";
    const char * const errorColor = anyErrors || noProjects ? colorForError : colorForPasses;
    const char * const buildColor = anyErrors || noProjects ? colorForError :
        anyWarnings ? colorForWarning : colorForPasses;
    const char * const warnColor = anyWarnings ? colorForWarning : colorForPasses;

    ::strftime( buffer, sizeof(buffer)-1, "%A, %Y - %B - %d, %H : %M : %S", m_today );
    m_output << "<tr><td><a href=\'" << m_outputFilename << "\'>"
             << buffer << "</a></td>" << endl;
    m_output << "<td>" << m_buildName << "</td>" << endl;
    m_output << "<td>" << m_buildType << "</td>" << endl;
    m_output << "<td>" << m_matcher.GetCompiler() << "</td>" << endl;
    m_output << "<td>" << projectCount << "</td>" << endl;
    m_output << "<td><span style=\"color: " << colorForPasses << "\">" << cleanPassCount
             << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colorForWarning << "\">" << warnPassCount
             << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << errorColor << "\">"
             << failCount << "</span></td>" << endl;

    m_output << "<td>" << skippedCount << "</td>" << endl;
    m_output << "<td>" << uptodateCount << "</td>" << endl;

    m_output << "<td><span style=\"color: " << errorColor << "\">"
             << errorCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << warnColor << "\">"
             << warnCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << buildColor << "\">"
             << buildResult << "</span></td></tr>" << endl;

    return true;
}

// --------------------------------------------------------------------------------------------

bool MainPageMaker::CreateFile( void )
{
    assert( nullptr != this );

    const Colors & colors = Colors::GetIt();
    const char * colorForInfo = colors.GetColor( Colors::Text       );
    const char * colorForBack = colors.GetColor( Colors::Background );

    m_output << "<html><head><title>" << m_title << "</title></head>" << endl
             << "<body bgcolor=" << colorForBack << " text=" << colorForInfo << '>' << endl;

    m_output << "<br><table border=1 cellspacing=0 cellpadding=3>" << endl;
    m_output << "<caption><em>Build Results</em></caption>" << endl;
    m_output << "<tr><th>Time Stamp</th><th>Build Name</th><th>Build Type</th><th>Compiler</th><th>Projects</th>"
             << "<th>Passed<br>Cleanly</th><th>Passed With<br>Warnings</th>"
             << "<th>Failed</th><th>Skipped</th><th>Up-To-<br>Date</th>"
             << "<th>Errors</th><th>Warnings</th><th>Result</th></tr>" << endl;
    if ( !AddCurrentOutput() ) return false;
    if ( !MakeEndOfTable() ) return false;

    return true;
}

// --------------------------------------------------------------------------------------------

bool MainPageMaker::MakeEndOfTable( void )
{
    assert( nullptr != this );

    m_output << "</table><br>" << endl;
    if ( !MakeColorTable( m_output ) ) return false;
    m_output << "</body>" << endl << "</html>" << endl;
    return true;
}

// --------------------------------------------------------------------------------------------

} // end namespace hestia

// $Log: outputters.cpp,v $
// Revision 1.17  2008/12/18 23:10:16  rich_sposato
// Now shows no-projects as error condition.
//
// Revision 1.16  2008/12/12 22:56:42  rich_sposato
// Replaced cout with cerr for error message.
//
// Revision 1.15  2008/12/12 22:53:03  rich_sposato
// Fixed cosmetic issue in timestamp.
//
// Revision 1.14  2008/12/04 00:05:43  rich_sposato
// Improved parsing abilities so this can parse final line with grand totals.
//
// Revision 1.13  2008/12/03 01:36:59  rich_sposato
// Changed matching classes so this can match MSVC9 build results with
// threaded output instead of just sequential output.
//
// Revision 1.12  2008/11/21 21:31:24  rich_sposato
// Added seconds to timestamp.
//
// Revision 1.11  2008/11/21 21:25:00  rich_sposato
// Removed superfluous local variables.
//
// Revision 1.10  2008/11/21 00:35:13  rich_sposato
// Minor changes.
//
// Revision 1.9  2008/11/20 23:19:31  rich_sposato
// Added ability for user to specify colors on the command line.
//
// Revision 1.8  2008/11/20 00:51:33  rich_sposato
// Added ability to parse output from code::blocks.
//
// Revision 1.7  2008/11/19 18:13:18  rich_sposato
// Moved code from TotalLineMatcher to ProjectMatcher.
//
// Revision 1.6  2008/11/18 21:40:06  rich_sposato
// Changed names to emphasize that this deals with main file.
//
// Revision 1.5  2008/11/18 18:23:19  rich_sposato
// Changed so program can show compiler name in output files.
//
// Revision 1.4  2008/11/18 17:36:33  rich_sposato
// Added title for long-term page.
//
// Revision 1.3  2008/11/18 00:42:55  rich_sposato
// Added build type to command line and to output.
//
// Revision 1.2  2008/11/17 22:56:21  rich_sposato
// Changed how it counts off table rows for keeping. Changed so it only uses
// filename and not path when making links to other files.
//
// Revision 1.1  2008/11/17 21:49:31  rich_sposato
// Moved outputter classes from main.cpp to separate files.
//
