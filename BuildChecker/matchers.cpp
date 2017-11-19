// $Header: /cvsroot/hestia/BuildChecker/matchers.cpp,v 1.18 2008/12/18 23:10:15 rich_sposato Exp $

/// @file matchers.cpp Defines classes used to check lines within input file.

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

#include "matchers.hpp"

#include <assert.h>
#include <iostream>
#include <fstream>

#include "utility.hpp"


// Namespace Declarations ---------------------------------------------------------------------

using namespace std;

namespace hestia
{

// ConvertToColor -----------------------------------------------------------------------------

const char * ConvertToColor( LineType::Type lineType )
{
    const Colors & colors = Colors::GetIt();
    const char * color = colors.GetColor( Colors::Text );

    switch ( lineType )
    {
        case LineType::Error:
            color = colors.GetColor( Colors::Errors );
            break;
        case LineType::Warning:
            color = colors.GetColor( Colors::Warnings );
            break;
        default:
            break;
    }

    return color;
}

// SubStringMatcher::Matches ------------------------------------------------------------------

bool SubStringMatcher::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    const char * place = FindSubString( content, m_sub );
    const bool isEmpty = IsEmptyString( place );
    return !isEmpty;
}

// SubStringMatcher::Output -------------------------------------------------------------------

void SubStringMatcher::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    m_count++;
    output << "<span style=\"color: " << m_color << "\">" << content
           << "</span><br>" << endl;
}

// LineMatcherAndNotifier::LineMatcherAndNotifier ---------------------------------------------

LineMatcherAndNotifier::LineMatcherAndNotifier( ProjectMatcher * matcher,
    LineType::Type lineType, const char * sub, const char * color ) :
    OutputHandler(),
    m_sub( sub ),
    m_color( color ),
    m_matcher( matcher ),
    m_lineType( lineType )
{
    assert( nullptr != this );
}

// LineMatcherAndNotifier::Matches ------------------------------------------------------------

bool LineMatcherAndNotifier::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    const char * place = FindSubString( content, m_sub );
    const bool isEmpty = IsEmptyString( place );
    return !isEmpty;
}

// LineMatcherAndNotifier::Output -------------------------------------------------------------

void LineMatcherAndNotifier::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    (void)output;
    const unsigned int index = ::atoi( content );
    if ( index != 0 )
        content = ::strchr( content, '>' ) + 1;
    m_matcher->IncrementProjectInfo( m_lineType, content, m_color, index );
}

// ProjectMatcher::ProjectMatcher -------------------------------------------------------------

ProjectMatcher::ProjectMatcher( void ) :
    OutputHandler(),
    m_doesIndexing( false ),
    m_skipped( 0 ),
    m_up_to_date( 0 ),
    m_cleanPasses( 0 ),
    m_warnedPasses( 0 ),
    m_project_fails( 0 ),
    m_errorCount( 0 ),
    m_warnCount( 0 ),
    m_projects()
{
    assert( nullptr != this );
    m_projects.reserve( 64 );
    assert( 0 == m_projects.size() );
}

// ProjectMatcher::~ProjectMatcher ------------------------------------------------------------

ProjectMatcher::~ProjectMatcher( void )
{
    assert( nullptr != this );

    ProjectInfoHolderIter end( m_projects.end() );
    for ( ProjectInfoHolderIter it( m_projects.begin() ); it != end; ++it )
    {
        ProjectInfo * info = *it;
        assert( nullptr != info );
        delete info;
    }
}

// ProjectMatcher::GetProjectCount ------------------------------------------------------------

unsigned int ProjectMatcher::GetProjectCount( void ) const
{
    const unsigned int count = static_cast< unsigned int >( m_projects.size() );
    return count;
}

// ProjectMatcher::AddProject -----------------------------------------------------------------

bool ProjectMatcher::AddProject( ProjectStatus::Status status, const ::std::string & name,
    const ::std::string & config, unsigned int errors, unsigned int warnings )
{
    assert( nullptr != this );

    if ( name.empty() )
        return false;
    if ( config.empty() )
        return false;

    ProjectInfo * info = new ProjectInfo( status, name, config, errors, warnings );
    assert( nullptr != info );
    m_projects.push_back( info );

    return true;
}

// ProjectMatcher::AddProject -----------------------------------------------------------------

bool ProjectMatcher::AddProject( ProjectStatus::Status status, const char * name,
    const char * config, unsigned int errors, unsigned int warnings )
{
    assert( nullptr != this );

    if ( IsEmptyString( name ) )
        return false;
    if ( IsEmptyString( config ) )
        return false;

    ProjectInfo * info = new ProjectInfo( status, name, config, errors, warnings );
    assert( nullptr != info );
    m_projects.push_back( info );

    return true;
}

// ProjectMatcher::GetProjectOutput -----------------------------------------------------------

ProjectMatcher::OutputReceiver * ProjectMatcher::GetProjectOutput( unsigned int index )
{
    assert( nullptr != this );
    const unsigned int count = m_projects.size();
    if ( 0 == count )
        return nullptr;
    if ( count < index )
        return nullptr;
    assert( index <= count );

    ProjectInfoHolderIter it;
    if ( 0 == index )
    {
        it = m_projects.end();
        --it;
    }
    else
    {
        it = m_projects.begin();
        advance( it, index - 1 );
    }

    assert( it != m_projects.end() );
    ProjectInfo * info = *it;
    assert( nullptr != info );
    if ( nullptr == info )
        return nullptr;

    return &( info->m_output );
}

// ProjectMatcher::SetTotalCounts -------------------------------------------------------------

void ProjectMatcher::SetTotalCounts( unsigned int passCount, unsigned int failCount,
    unsigned int upToDateCount, unsigned int skipCount )
{
    assert( nullptr != this );
    (void)passCount;
    (void)failCount;
    m_up_to_date = upToDateCount;
    m_skipped = skipCount;
}

// ProjectMatcher::UpdateTotalCounts ----------------------------------------------------------

void ProjectMatcher::UpdateTotalCounts( void )
{
    assert( nullptr != this );
    if ( 0 == m_projects.size() )
    {
        return;
    }

    ProjectInfoHolderIter end( m_projects.end() );
    for ( ProjectInfoHolderIter it( m_projects.begin() ); it != end; ++it )
    {
        ProjectInfo * info = *it;
        assert( nullptr != info );
        if ( 0 < info->m_errors )
        {
            info->m_status = ProjectStatus::Failed;
            m_project_fails++;
        }
        else if ( ( ProjectStatus::Unknown == info->m_status )
               || ( ProjectStatus::Passed  == info->m_status ) )
        {
            info->m_status = ProjectStatus::Passed;
            if ( 0 < info->m_warnings )
                m_warnedPasses++;
            else
                m_cleanPasses++;
        }
        m_errorCount += info->m_errors;
        m_warnCount += info->m_warnings;

        switch ( info->m_status )
        {
            case ProjectStatus::Passed:
            case ProjectStatus::Failed:
                break;
            case ProjectStatus::UpToDate:
                m_up_to_date++;
                break;
            case ProjectStatus::Skipped:
                m_skipped++;
                break;
            default:
                assert( false );
                break;
        }
    }
}

// ProjectMatcher::OutputProjectInfo ----------------------------------------------------------

void ProjectMatcher::OutputProjectInfo( ::std::ofstream & output )
{
    assert( nullptr != this );
    if ( 0 == m_projects.size() )
        return;

    ProjectInfoHolderIter end( m_projects.end() );
    for ( ProjectInfoHolderIter it( m_projects.begin() ); it != end; ++it )
    {
        ProjectInfo * info = *it;
        assert( nullptr != info );
        const string & str = info->m_output.str();
        const char * s = str.c_str();
        if ( !IsEmptyString( s ) )
            output << s;
    }
}

// ProjectMatcher::UpdateProjectCounts --------------------------------------------------------

bool ProjectMatcher::UpdateProjectCounts( LineType::Type lineType, unsigned int errorCount,
    unsigned int warningCount, unsigned int index )
{
    assert( nullptr != this );
    const unsigned int count = m_projects.size();
    if ( 0 == count )
        return false;
    if ( count < index )
        return false;
    assert( index <= count );

    ProjectInfoHolderIter it;
    if ( 0 == index )
    {
        it = m_projects.end();
        --it;
    }
    else
    {
        it = m_projects.begin();
        advance( it, index - 1 );
    }
    assert( it != m_projects.end() );
    ProjectInfo * info = *it;
    assert( nullptr != info );

    switch ( lineType )
    {
        case LineType::Skipped:
            info->m_status = ProjectStatus::Skipped;
            assert( 0 == errorCount );
            assert( 0 == warningCount );
            break;
        case LineType::UpToDate:
            info->m_status = ProjectStatus::UpToDate;
            assert( 0 == errorCount );
            assert( 0 == warningCount );
            break;
        default:
            break;
    }
    info->m_errors = errorCount;
    info->m_warnings = warningCount;

    return true;
}

// ProjectMatcher::IncrementProjectInfo -------------------------------------------------------

bool ProjectMatcher::IncrementProjectInfo( LineType::Type lineType, const char * content,
    const char * color, unsigned int index )
{
    assert( nullptr != this );
    const unsigned int count = m_projects.size();
    if ( 0 == count )
        return false;
    if ( count < index )
        return false;
    assert( index <= count );

    ProjectInfoHolderIter it;
    if ( 0 == index )
    {
        it = m_projects.end();
        --it;
    }
    else
    {
        it = m_projects.begin();
        advance( it, index - 1 );
    }
    assert( it != m_projects.end() );
    ProjectInfo * info = *it;
    assert( nullptr != info );

    switch ( lineType )
    {
        case LineType::Totals:
            info->m_status = ProjectStatus::Passed;
            break;
        case LineType::UpToDate:
            info->m_status = ProjectStatus::UpToDate;
            break;
        case LineType::Skipped:
            info->m_status = ProjectStatus::Skipped;
            break;
        case LineType::Warning:
            info->m_warnings++;
            break;
        case LineType::Error:
            info->m_errors++;
            break;
        default:
            break;
    }
    info->m_output << "<span style=\"color: " << color << "\">" << content << "</span><br>" << endl;

    return true;
}

// ProjectMatcher::MakeProjectTable -----------------------------------------------------------

bool ProjectMatcher::MakeProjectTable( ::std::ofstream & output )
{
    assert( nullptr != this );

    unsigned int projectCount = 0;
    bool anyErrors = false;
    bool anyWarnings = false;
    const bool noProjects = ( 0 == m_projects.size() );
    const char * buildResult = nullptr;
    const char * buildColor  = nullptr;
    const char * errorColor  = nullptr;
    const char * warnColor   = nullptr;
    const Colors & colors = Colors::GetIt();
    const char * colorForError   = colors.GetColor( Colors::Errors   );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses  = colors.GetColor( Colors::Passes   );
    const char * colorForInfo    = colors.GetColor( Colors::Text     );

    output << "<br><table border=1 cellspacing=0 cellpadding=3>" << endl;
    output << "<caption><em>Project Build Results</em></caption>" << endl;
    output << "<tr><th>-</th><th>Project</th><th>Configuration</th><th>Errors</th>"
           << "<th>Warnings</th><th>Result</th></tr>" << endl;

    ProjectInfoHolderCIter stop( m_projects.end() );
    for ( ProjectInfoHolderCIter here( m_projects.begin() ); here != stop; ++here )
    {
        ++projectCount;
        const ProjectInfo * info = *here;
        assert( nullptr != info );
        anyErrors = ( 0 < info->m_errors );
        anyWarnings = ( 0 < info->m_warnings );
        errorColor = anyErrors ? colorForError : colorForPasses;
        warnColor = anyWarnings ? colorForWarning : colorForPasses;
        switch ( info->m_status )
        {
            case ProjectStatus::Failed:
                buildColor = colorForError;
                buildResult = "FAILED";
                break;
            case ProjectStatus::Passed:
                buildColor = anyWarnings ? colorForWarning : colorForPasses;
                buildResult = "Passed";
                break;
            case ProjectStatus::UpToDate:
                buildColor = colorForInfo;
                buildResult = "Up To Date";
                break;
            case ProjectStatus::Skipped:
                buildColor = colorForInfo;
                buildResult = "Skipped";
                break;
            case ProjectStatus::Unknown:
                buildColor = colorForInfo;
                buildResult = "unknown";
                break;
            case ProjectStatus::Other:
            default:
                assert( false );
                break;
        }

        output << "<tr><td>" << projectCount << "</td>" << endl;
        output << "<td><span style=\"color: " << buildColor
               << "\">" << info->m_project << "</span></td>" << endl;
        output << "<td><span style=\"color: " << buildColor
               << "\">" << info->m_config << "</span></td>" << endl;
        output << "<td><span style=\"color: " << errorColor
               << "\">" << info->m_errors << "</span></td>" << endl;
        output << "<td><span style=\"color: " << warnColor
               << "\">" << info->m_warnings << "</span></td>" << endl;
        output << "<td><span style=\"color: " << buildColor
               << "\">" << buildResult << "</span></td></tr>" << endl;
    }

    anyErrors = ( 0 < m_errorCount );
    anyWarnings = ( 0 < m_warnCount );
    buildColor = anyErrors || noProjects ? colorForError : anyWarnings ? colorForWarning : colorForPasses;
    errorColor = anyErrors || noProjects ? colorForError : colorForPasses;
    warnColor = anyWarnings ? colorForWarning : colorForPasses;
    buildResult = anyErrors ? "FAILED" : noProjects ? "EMPTY!" : "Passed";
    output << "<tr><th>-</th><th>Overall Totals</th><th>-</th>" << endl;
    output << "<th><span style=\"color: " << errorColor << "\">" << m_errorCount << "</th>" << endl;
    output << "<th><span style=\"color: " << warnColor  << "\">" << m_warnCount  << "</th>" << endl;
    output << "<th><span style=\"color: " << buildColor << "\">" << buildResult  << "</th>" << endl;
    output << "</tr></table><br>" << endl;

//    const bool okay = MakeProjectCountTable( output, errorColor );
//    return okay;
    return true;
}

// ProjectMatcher::MakeProjectCountTable ------------------------------------------------------

bool ProjectMatcher::MakeProjectCountTable( ofstream & output, const char * errorColor )
{
    assert( nullptr != this );

    const unsigned int projectCount = m_projects.size();
    const Colors & colors = Colors::GetIt();
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses = colors.GetColor( Colors::Passes );

    output << "<table><tr><td><span style=\"color: "
           << errorColor << "\">Failed Projects:</span></td><td><span style=\"color: "
           << errorColor << "\">" << m_project_fails << "</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: " << colorForWarning << "\">Passed With Warning Projects:</span></td>"
           << "<td><span style=\"color: " << colorForWarning << "\">" << m_warnedPasses
           << "</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: " << colorForPasses << "\">Cleanly Passing Projects:</span></td>"
           << "<td><span style=\"color: " << colorForPasses << "\">" << m_cleanPasses
           << "</span></td></tr>" << endl;
    output << "<tr><td>Already Up-To-Date Projects:</td><td>" << m_up_to_date
           << "</td></tr>" << endl;
    output << "<tr><td>Skipped Projects:</td><td>" << m_skipped
           << "</td></tr>" << endl;
    output << "<td>Total Projects:</td><td>" << projectCount
           << "</td></tr></table><br>" << endl;

    return true;
}

// ProjectMatcher::ProjectInfo::ProjectInfo ---------------------------------------------------

ProjectMatcher::ProjectInfo::ProjectInfo( ProjectStatus::Status status,
    const string & name, const string & config,
    unsigned int errors, unsigned int warnings ) :
    m_status( status ),
    m_output(),
    m_project( name ),
    m_config( config ),
    m_errors( errors ),
    m_warnings( warnings )
{
    assert( nullptr != this );
}

// ProjectMatcher::ProjectInfo::~ProjectInfo --------------------------------------------------

ProjectMatcher::ProjectInfo::~ProjectInfo( void )
{
    assert( nullptr != this );
}

// CodeBlocksProjectMatcher::CodeBlocksProjectMatcher -----------------------------------------

CodeBlocksProjectMatcher::CodeBlocksProjectMatcher( void ) :
    ProjectMatcher()
{
    assert( nullptr != this );
}

// CodeBlocksProjectMatcher::~CodeBlocksProjectMatcher ----------------------------------------

CodeBlocksProjectMatcher::~CodeBlocksProjectMatcher( void )
{
    assert( nullptr != this );
}

// CodeBlocksProjectMatcher::Matches ----------------------------------------------------------

bool CodeBlocksProjectMatcher::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    const char * place = FindSubString( content, "Build: " );
    const bool isEmpty = IsEmptyString( place );
    return !isEmpty;
}

// CodeBlocksProjectMatcher::Output -----------------------------------------------------------

void CodeBlocksProjectMatcher::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    const char * configStart = FindSubString( content, "Build: " );
    const char * configEnd = FindSubString( configStart, " in " );
    configStart += 7;
    assert( configStart < configEnd );
    const string config( configStart, configEnd - configStart );
    const char * nameStart = configEnd + 4;
    const char * nameEnd = FindSubString( nameStart, " --" );
    assert( nameStart < nameEnd );
    string projectName( nameStart, nameEnd - nameStart );

    const bool added = AddProject( ProjectStatus::Unknown, projectName, config, 0, 0 );
    assert( added );
    OutputReceiver * pReceiver = nullptr;
    if ( added )
        pReceiver = GetProjectOutput( 0 );
    if ( pReceiver == nullptr )
        pReceiver = &output;
    OutputReceiver & receiver = *pReceiver;
    receiver << "<br>" << endl;
    receiver << "Project: " << projectName << "<br>" << endl;
    receiver << "Configuration: " << config << "<br>" << endl;
}

// MsvcProjectMatcher::MsvcProjectMatcher -----------------------------------------------------

MsvcProjectMatcher::MsvcProjectMatcher( void ) :
    ProjectMatcher()
{
    assert( nullptr != this );
}

// MsvcProjectMatcher::~MsvcProjectMatcher ----------------------------------------------------

MsvcProjectMatcher::~MsvcProjectMatcher( void )
{
    assert( nullptr != this );
}

// MsvcProjectMatcher::Matches ----------------------------------------------------------------

bool MsvcProjectMatcher::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    const char * place = FindSubString( content, "Project: " );
    const bool isEmpty = IsEmptyString( place );
    return !isEmpty;
}

// MsvcProjectMatcher::Output -----------------------------------------------------------------

void MsvcProjectMatcher::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    const unsigned int index = ::atoi( content );
    if ( index != 0 )
    {
        SetIndexing( true );
        content = ::strchr( content, '>' ) + 1;
    }

    const char * projectPlace = FindSubString( content, "Project: " );
    const char * configPlace  = FindSubString( projectPlace, ", Configuration: " );
    assert( projectPlace < configPlace );
    string projectOutput( projectPlace, configPlace-projectPlace );

    const char * namePlace = projectPlace + 9;
    assert( namePlace < configPlace );
    string name( namePlace, configPlace-namePlace );

    namePlace = configPlace + 2;
    configPlace = FindSubString( namePlace, " -" );
    projectPlace += 11 + name.size();
    string configOutput( projectPlace, configPlace - projectPlace );

    projectPlace += 15;
    const string config( projectPlace, configPlace - projectPlace );
    AddProject( ProjectStatus::Unknown, name.c_str(), config.c_str(), 0, 0 );
    OutputReceiver * pReceiver = GetProjectOutput( index );
    if ( pReceiver == nullptr )
        pReceiver = &output;
    OutputReceiver & receiver = *pReceiver;
    receiver << "<br>" << endl << projectOutput << "<br>" << endl;
    receiver << configOutput << "<br>" << endl;
}

// TotalLineMatcher::TotalLineMatcher ---------------------------------------------------------

TotalLineMatcher::TotalLineMatcher( void ) :
    m_matcher( nullptr ),
    m_lineType( LineType::Unknown )
{
    assert( nullptr != this );
}

// TotalLineMatcher::~TotalLineMatcher --------------------------------------------------------

TotalLineMatcher::~TotalLineMatcher( void )
{
    assert( nullptr != this );
}

// TotalLineMatcher::Matches ------------------------------------------------------------------

bool TotalLineMatcher::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    const char * place = FindSubString( content, " error(s), " );
    if ( !IsEmptyString( place ) )
    {
        m_lineType = LineType::Totals;
        return true;
    }

    place = FindSubString( content, " - up-to-date." );
    if ( !IsEmptyString( place ) )
    {
        m_lineType = LineType::UpToDate;
        return true;
    }

    place = FindSubString( content, " - skipped " );
    if ( !IsEmptyString( place ) )
    {
        m_lineType = LineType::Skipped;
        return true;
    }

    m_lineType = LineType::Unknown;
    return false;
}

// TotalLineMatcher::Output -------------------------------------------------------------------

void TotalLineMatcher::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    switch ( m_lineType )
    {
        case LineType::Totals:
            DoTotalOutput( output, content );
            break;
        case LineType::UpToDate:
            DoSimpleOutput( output, content );
            break;
        case LineType::Skipped:
            DoSimpleOutput( output, content );
            break;
        case LineType::Unknown:
        default:
            assert( false );
            break;
    }
}

// TotalLineMatcher::DoSimpleOutput -----------------------------------------------------------

void TotalLineMatcher::DoSimpleOutput( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    (void)output;

    const unsigned int index = ::atoi( content );
    if ( index != 0 )
    {
        content = ::strchr( content, '>' ) + 1;
    }
    OutputReceiver * receiver = m_matcher->GetProjectOutput( index );
    *receiver << content << "<br>" << endl;
    m_matcher->UpdateProjectCounts( m_lineType, 0, 0, index );
}

// TotalLineMatcher::DoTotalOutput ------------------------------------------------------------

void TotalLineMatcher::DoTotalOutput( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    (void)output;

    const unsigned int index = ::atoi( content );
    const char * error_spot = FindSubString( content, "error(s)" );
    const char * warning_spot = FindSubString( content, "warning(s)" );
    error_spot = BackupToNumber( error_spot );
    warning_spot = BackupToNumber( warning_spot );
    const unsigned int errorCount = ::atoi( error_spot );
    const unsigned int warnCount  = ::atoi( warning_spot );
    const bool hasErrors = ( 0 < errorCount );
    const bool hasWarnings = ( 0 < warnCount );
    const Colors & colors = Colors::GetIt();
    const char * colorForError = colors.GetColor( Colors::Errors );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForPasses = colors.GetColor( Colors::Passes );
    const char * errorColor = hasErrors ? colorForError : colorForPasses;
    const char * warnColor  = hasWarnings ? colorForWarning : colorForPasses;

    if ( index != 0 )
    {
        content = ::strchr( content, '>' ) + 1;
    }
    OutputReceiver * receiver = m_matcher->GetProjectOutput( index );

    string temp( content, error_spot-content );
    *receiver << temp;
    temp.assign( error_spot, warning_spot-error_spot );
    *receiver << "<span style=\"color: " << errorColor << "\">" << temp << "</span>";
    temp.assign( warning_spot );
    *receiver << "<span style=\"color: " << warnColor << "\">" << temp << "</span><br>"
           << endl;

    m_matcher->UpdateProjectCounts( m_lineType, errorCount, warnCount, index );
}

// GrandTotalMatcher::GrandTotalMatcher -------------------------------------------------------

GrandTotalMatcher::GrandTotalMatcher( ProjectMatcher * projectMatcher ) :
    OutputHandler(),
    m_projectMatcher( projectMatcher )
{
    assert( nullptr != this );
    assert( nullptr != projectMatcher );
}

// GrandTotalMatcher::~GrandTotalMatcher ------------------------------------------------------

GrandTotalMatcher::~GrandTotalMatcher( void )
{
    assert( nullptr != this );
}

// GrandTotalMatcher::Matches -----------------------------------------------------------------

bool GrandTotalMatcher::Matches( const char * content ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );

    const char * place = FindSubString( content, "Build: " );
    bool isEmpty = IsEmptyString( place );
    if ( isEmpty )
        return false;

    place = FindSubString( place, "succeeded" );
    isEmpty = IsEmptyString( place );
    if ( isEmpty )
        return false;

    place = FindSubString( place, "failed" );
    isEmpty = IsEmptyString( place );
    if ( isEmpty )
        return false;

    place = FindSubString( place, "up-to-date" );
    isEmpty = IsEmptyString( place );
    if ( isEmpty )
        return false;

    place = FindSubString( place, "skipped" );
    isEmpty = IsEmptyString( place );
    return !isEmpty;
}

// GrandTotalMatcher::Output ------------------------------------------------------------------

void GrandTotalMatcher::Output( OutputReceiver & output, const char * content )
{
    assert( nullptr != this );
    assert( !IsEmptyString( content ) );
    (void)output;

    const char * place = FindSubString( content, "succeeded" );
    assert( !IsEmptyString( place ) );
    const char * number = BackupToNumber( place );
    const unsigned int passCount = ::atoi( number );

    place = FindSubString( content, "failed" );
    assert( !IsEmptyString( place ) );
    number = BackupToNumber( place );
    const unsigned int failCount = ::atoi( number );

    place = FindSubString( content, "up-to-date" );
    assert( !IsEmptyString( place ) );
    number = BackupToNumber( place );
    const unsigned int upToDateCount = ::atoi( number );

    place = FindSubString( content, "skipped" );
    assert( !IsEmptyString( place ) );
    number = BackupToNumber( place );
    const unsigned int skipCount = ::atoi( number );

    m_projectMatcher->SetTotalCounts( passCount, failCount, upToDateCount, skipCount );
}

// MatcherSet::MatchingRule -------------------------------------------------------------------

MatcherSet::MatchingRule::MatchingRule( const ::std::string & content,
    LineType::Type lineType ) :
    m_content( content ),
    m_lineType( lineType )
{
    assert( nullptr != this );
}

// MatcherSet::MatcherSet ---------------------------------------------------------------------

MatcherSet::MatcherSet( const char * compiler ) :
    m_handlers(),
    m_projectMatcher( nullptr ),
    m_compiler( nullptr )
{
    assert( nullptr != this );

    if ( IsEmptyString( compiler ) )
        compiler = "msvc9";
    m_handlers.reserve( 8 );
    SetupProjectMatcher( compiler );
    SetupCompilerRules( compiler );

    assert( nullptr != m_projectMatcher );
    assert( nullptr != m_compiler );
    assert( 0 != m_handlers.size() );
}

// MatcherSet::MatcherSet ---------------------------------------------------------------------

MatcherSet::MatcherSet( const char * compiler, const MatchingRules & rules ) :
    m_handlers(),
    m_projectMatcher( nullptr ),
    m_compiler( nullptr )
{
    assert( nullptr != this );

    if ( IsEmptyString( compiler ) )
        compiler = "msvc9";
    m_handlers.reserve( 8 );
    SetupProjectMatcher( compiler );
    AddMatchingRules( rules );
    SetupCompilerRules( compiler );

    assert( nullptr != m_projectMatcher );
    assert( nullptr != m_compiler );
    assert( 0 != m_handlers.size() );
}

// MatcherSet::SetupProjectMatcher ------------------------------------------------------------

void MatcherSet::SetupProjectMatcher( const char * compiler )
{
    assert( nullptr != this );

    if ( ::strcmp( compiler, "codeblocks" ) == 0 )
    {
        m_compiler = "Code::Blocks";
        m_projectMatcher = new CodeBlocksProjectMatcher;
        assert( nullptr != m_projectMatcher );
        m_handlers.push_back( m_projectMatcher );
    }
    else
    {
        m_compiler = "MSVC 9";
        m_projectMatcher = new MsvcProjectMatcher;
        assert( nullptr != m_projectMatcher );
        m_handlers.push_back( m_projectMatcher );
    }
}

// MatcherSet::AddMatchingRules ---------------------------------------------------------------

void MatcherSet::AddMatchingRules( const MatchingRules & rules )
{
    assert( nullptr != this );

    OutputHandler * matcher = nullptr;
    MatchingRuleCIter end( rules.end() );
    for ( MatchingRuleCIter cit( rules.begin() ); cit != end; ++cit )
    {
        const MatchingRule & rule = *cit;
        const char * color = ConvertToColor( rule.m_lineType );
        matcher = new LineMatcherAndNotifier( m_projectMatcher, rule.m_lineType,
            rule.m_content.c_str(), color );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );
    }
}

// MatcherSet::SetupCompilerRules -------------------------------------------------------------

void MatcherSet::SetupCompilerRules( const char * compiler )
{
    assert( nullptr != this );

    const Colors & colors = Colors::GetIt();
    const char * colorForError   = colors.GetColor( Colors::Errors   );
    const char * colorForWarning = colors.GetColor( Colors::Warnings );
    const char * colorForInfo    = colors.GetColor( Colors::Text     );
    const char * colorForPasses  = colors.GetColor( Colors::Passes   );

    OutputHandler * matcher = nullptr;
    if ( ::strcmp( compiler, "codeblocks" ) == 0 )
    {
        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Warning,
            ": warning: ", colorForWarning );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Error,
            ": error: ", colorForError );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Totals,
            "Output size is", colorForPasses );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::UpToDate,
            "Target is up to date.", colorForInfo );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Skipped,
            "Nothing to be done.", colorForInfo );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );
    }
    else
    {
        TotalLineMatcher * totalMatcher = new TotalLineMatcher;
        m_handlers.push_back( totalMatcher );
        assert( nullptr != totalMatcher );
        totalMatcher->SetProjectMatcher( m_projectMatcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Warning, " : warning ", colorForWarning );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Error, " : error", colorForError );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new LineMatcherAndNotifier( m_projectMatcher, LineType::Error, "fatal error", colorForError );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );

        matcher = new GrandTotalMatcher( m_projectMatcher );
        assert( nullptr != matcher );
        m_handlers.push_back( matcher );
    }
}

// MatcherSet::~MatcherSet --------------------------------------------------------------------

MatcherSet::~MatcherSet( void )
{
    assert( nullptr != this );

    const HandlerSetIter end( m_handlers.end() );
    for( HandlerSetIter it( m_handlers.begin() ); it != end; ++it )
    {
        OutputHandler * handler = *it;
        assert( nullptr != handler );
        delete handler;
    }
}

// MatcherSet::MatchLine ----------------------------------------------------------------------

bool MatcherSet::MatchLine( const char * line, stringstream & streamer )
{
    assert( nullptr != this );
    if ( IsEmptyString( line ) )
        return false;

    bool found = false;
    const HandlerSetIter end( m_handlers.end() );
    for( HandlerSetIter it( m_handlers.begin() ); it != end; ++it )
    {
        OutputHandler * handler = *it;
        assert( nullptr != handler );
        if ( handler->Matches( line ) )
        {
            handler->Output( streamer, line );
            found = true;
            break;
        }
    }

    return found;
}

// --------------------------------------------------------------------------------------------

} // end namespace hestia

// $Log: matchers.cpp,v $
// Revision 1.18  2008/12/18 23:10:15  rich_sposato
// Now shows no-projects as error condition.
//
// Revision 1.17  2008/12/11 20:48:58  rich_sposato
// Added ability to specify matching rules at runtime.
//
// Revision 1.16  2008/12/09 22:48:39  rich_sposato
// Added another constructor, and a bit of refactoring.
//
// Revision 1.15  2008/12/04 20:23:35  rich_sposato
// Fixed minor bug in project counting function.
//
// Revision 1.14  2008/12/04 00:05:43  rich_sposato
// Improved parsing abilities so this can parse final line with grand totals.
//
// Revision 1.13  2008/12/03 22:07:56  rich_sposato
// Improved parsing for codeblocks output files.
//
// Revision 1.12  2008/12/03 17:47:55  rich_sposato
// Changed output handling when parsing build results made from sequential
// compiling instead of threaded compiling.
//
// Revision 1.11  2008/12/03 01:36:59  rich_sposato
// Changed matching classes so this can match MSVC9 build results with
// threaded output instead of just sequential output.
//
// Revision 1.10  2008/11/21 21:25:00  rich_sposato
// Removed superfluous local variables.
//
// Revision 1.9  2008/11/21 00:35:13  rich_sposato
// Minor changes.
//
// Revision 1.8  2008/11/20 23:19:31  rich_sposato
// Added ability for user to specify colors on the command line.
//
// Revision 1.7  2008/11/20 00:51:33  rich_sposato
// Added ability to parse output from code::blocks.
//
// Revision 1.6  2008/11/19 18:13:18  rich_sposato
// Moved code from TotalLineMatcher to ProjectMatcher.
//
// Revision 1.5  2008/11/19 01:00:15  rich_sposato
// Refactored project and total-line matchers.
//
// Revision 1.4  2008/11/18 18:23:19  rich_sposato
// Changed so program can show compiler name in output files.
//
// Revision 1.3  2008/11/17 23:31:48  rich_sposato
// Moved code which makes project counts to separate function.
//
// Revision 1.2  2008/11/17 22:21:53  rich_sposato
// Moved some ctors and dtors to source code file from header file.
//
// Revision 1.1  2008/11/17 21:00:14  rich_sposato
// Moved some classes and functions from main.cpp to a separate files.
//
