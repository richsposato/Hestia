// $Header: /cvsroot/hestia/BuildChecker/outputters.hpp,v 1.6 2008/11/19 18:13:18 rich_sposato Exp $

/// @file outputters.hpp Defines classes used to make output files.


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

#if !defined( HESTIA_ABC_OUTPUTTERS_HPP_INCLUDED )
#define HESTIA_ABC_OUTPUTTERS_HPP_INCLUDED


// --------------------------------------------------------------------------------------------

#include <fstream>
#include <sstream>


struct tm;

namespace hestia
{

    class ProjectMatcher;
    class MatcherSet;


// class OutputMaker --------------------------------------------------------------------------

class OutputMaker
{
public:

    OutputMaker( const char * buildName, const char * buildType,
        const char * outputFilename, const char * longtermFilename );
    ~OutputMaker( void );

    bool IsOpen( void ) const;
    bool Process( MatcherSet & matcher, const struct tm * today, const char * inputFilename,
        const char * content );

private:

    OutputMaker( const OutputMaker & );
    OutputMaker & operator = ( const OutputMaker & );

    bool MakeHeader( const MatcherSet & matcher, const struct tm * today, const char * inputFilename );
    bool ParseContents( MatcherSet & matcher, const char * content );
    bool MakeSummaryTable( void );
    bool OutputContents( void );
    bool Last( void );

    ::std::ofstream m_output;
    ::std::stringstream m_streamer;
    ProjectMatcher * m_projects;
    const char * m_longtermFilename;
    const char * m_buildName;
    const char * m_buildType;

};

// class MainPageMaker ------------------------------------------------------------------------

/** @class MainPageMaker Creates file containing summaries of many build results so users can
 have a long-term view of the build process.
 */
class MainPageMaker
{
public:

    MainPageMaker( const char * title, const char * buildName, const char * buildType,
        const struct tm * today, const MatcherSet & matcher, const char * longtermFilename,
        const char * outputFilename, unsigned int keepCount );

    ~MainPageMaker( void );

    /// Returns true if output file is open.
    bool IsOpen( void ) const;

    bool AddToContent( ::std::string & fileContent );

    bool CreateFile( void );

private:

    /// Default constructor is not implemented.
    MainPageMaker( void );
    /// Copy-constructor is not implemented.
    MainPageMaker( const MainPageMaker & );
    /// Copy-assignment operator is not implemented.
    MainPageMaker & operator = ( const MainPageMaker & );

    bool AddCurrentOutput( void );
    bool MakeEndOfTable( void );

    const MatcherSet & m_matcher;
    const ProjectMatcher * m_totalMatcher;
    ::std::ofstream m_output;
    const char * m_outputFilename;
    const char * m_buildName;
    const char * m_buildType;
    const char * m_title;
    const struct tm * m_today;
    unsigned int m_keepCount;
};

// --------------------------------------------------------------------------------------------

} // end namespace hestia

#endif

// $Log: outputters.hpp,v $
// Revision 1.6  2008/11/19 18:13:18  rich_sposato
// Moved code from TotalLineMatcher to ProjectMatcher.
//
// Revision 1.5  2008/11/18 21:40:06  rich_sposato
// Changed names to emphasize that this deals with main file.
//
// Revision 1.4  2008/11/18 18:23:19  rich_sposato
// Changed so program can show compiler name in output files.
//
// Revision 1.3  2008/11/18 17:36:33  rich_sposato
// Added title for long-term page.
//
// Revision 1.2  2008/11/18 00:42:55  rich_sposato
// Added build type to command line and to output.
//
// Revision 1.1  2008/11/17 21:49:31  rich_sposato
// Moved outputter classes from main.cpp to separate files.
//
