// $Header: /cvsroot/hestia/BuildChecker/matchers.hpp,v 1.12 2008/12/11 20:48:58 rich_sposato Exp $

/// @file matchers.hpp Defines classes used to check lines within input file.


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


#if !defined( HESTIA_ABC_MATCHERS_HPP_INCLUDED )
#define HESTIA_ABC_MATCHERS_HPP_INCLUDED


// --------------------------------------------------------------------------------------------

#include <sstream>
#include <string>
#include <vector>

namespace hestia
{


// class OutputHandler ------------------------------------------------------------------------

/** Interface class for matching lines from input file and sending content to output file.
 This has no data members and all the public functions are purely virtual.
 */
class OutputHandler
{
public:

    typedef ::std::stringstream OutputReceiver;

    inline virtual ~OutputHandler( void ) {}

    virtual bool Matches( const char * content ) const = 0;
    virtual void Output( OutputReceiver & output, const char * content ) = 0;

protected:
    inline OutputHandler( void ) {}

private:

    /// Copy-constructor is not implemented.
    OutputHandler( const OutputHandler & );
    /// Copy-assignment operator is not implemented.
    OutputHandler & operator = ( const OutputHandler & );

};

// SubStringMatcher ---------------------------------------------------------------------------

class SubStringMatcher : public OutputHandler
{
public:

    inline SubStringMatcher( const char * sub, const char * color ) :
        m_sub( sub ), m_color( color ), m_count( 0 ) {}
    inline virtual ~SubStringMatcher( void ) {}

    inline unsigned int GetCount( void ) { return m_count; }
    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Copy-constructor is not implemented.
    SubStringMatcher( const SubStringMatcher & );
    /// Copy-assignment operator is not implemented.
    SubStringMatcher & operator = ( const SubStringMatcher & );

    const char * m_sub;
    const char * m_color;
    unsigned int m_count;
};

// class LineType -----------------------------------------------------------------------------

class LineType
{
public:
    enum Type
    {
        Unknown  = 0x0000,
        Totals   = 0x0001,
        UpToDate = 0x0002,
        Skipped  = 0x0004,
        Info     = 0x0008,
        Warning  = 0x0010,
        Error    = 0x0020,
        Other    = 0x0040
    };
};

// class ProjectStatus ------------------------------------------------------------------------

class ProjectStatus
{
public:
    enum Status
    {
        Unknown  = 0x0000,
        Passed   = 0x0001,
        Failed   = 0x0002,
        UpToDate = 0x0004,
        Skipped  = 0x0008,
        Other    = 0x0010
    };
};

// class ProjectMatcher -----------------------------------------------------------------------

class ProjectMatcher : public OutputHandler
{
public:

    virtual ~ProjectMatcher( void );

    inline bool DoesIndexing( void ) const { return m_doesIndexing; }

    inline unsigned int GetSkippedCount( void ) const { return m_skipped; }
    inline unsigned int GetUpToDateCount( void ) const { return m_up_to_date; }
    inline unsigned int GetCleanPassCount( void ) const { return m_cleanPasses; }
    inline unsigned int GetWarnPassCount( void ) const { return m_warnedPasses; }
    inline unsigned int GetFailCount( void ) const { return m_project_fails; }
    inline unsigned int GetErrorCount( void ) const { return m_errorCount; }
    inline unsigned int GetWarningCount( void ) const { return m_warnCount; }

    unsigned int GetProjectCount( void ) const;

    bool MakeProjectTable( ::std::ofstream & output );

    bool AddProject( ProjectStatus::Status status, const ::std::string & name,
            const ::std::string & config, unsigned int errors, unsigned int warnings );

    bool AddProject( ProjectStatus::Status status, const char * name, const char * config,
        unsigned int errors, unsigned int warnings );

    bool UpdateProjectCounts( LineType::Type lineType, unsigned int errorCount,
        unsigned int warningCount, unsigned int index = 0 );

    bool IncrementProjectInfo( LineType::Type lineType, const char * content,
        const char * color, unsigned int index = 0 );

    OutputReceiver * GetProjectOutput( unsigned int index = 0 );

    void SetTotalCounts( unsigned int passCount, unsigned int failCount,
        unsigned int upToDateCount, unsigned int skipCount );

    void UpdateTotalCounts( void );

    void OutputProjectInfo( ::std::ofstream & output );

protected:

    ProjectMatcher( void );

    inline void SetIndexing( bool indexes ) { m_doesIndexing = indexes; }

private:

    struct ProjectInfo
    {
        ProjectInfo( ProjectStatus::Status status, const ::std::string & name,
            const ::std::string & config, unsigned int errors, unsigned int warnings );
        ~ProjectInfo( void );

        ProjectStatus::Status m_status;
        OutputReceiver m_output;
        ::std::string m_project;
        ::std::string m_config;
        unsigned int m_errors;
        unsigned int m_warnings;
    };

    typedef ::std::vector< ProjectInfo * > ProjectInfoHolder;
    typedef ProjectInfoHolder::iterator ProjectInfoHolderIter;
    typedef ProjectInfoHolder::const_iterator ProjectInfoHolderCIter;

    /// Copy-constructor is not implemented.
    ProjectMatcher( const ProjectMatcher & );
    /// Copy-assignment operator is not implemented.
    ProjectMatcher & operator = ( const ProjectMatcher & );

    bool MakeProjectCountTable( ::std::ofstream & output, const char * errorColor );

    bool m_doesIndexing;
    unsigned int m_skipped;
    unsigned int m_up_to_date;
    unsigned int m_cleanPasses;
    unsigned int m_warnedPasses;
    unsigned int m_project_fails;
    unsigned int m_errorCount;
    unsigned int m_warnCount;

    ProjectInfoHolder m_projects;
};

// MsvcProjectMatcher -------------------------------------------------------------------------

class MsvcProjectMatcher : public ProjectMatcher
{
public:

    MsvcProjectMatcher( void );
    virtual ~MsvcProjectMatcher( void );

    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Copy-constructor is not implemented.
    MsvcProjectMatcher( const MsvcProjectMatcher & );
    /// Copy-assignment operator is not implemented.
    MsvcProjectMatcher & operator = ( const MsvcProjectMatcher & );
};

// class CodeBlocksProjectMatcher -------------------------------------------------------------

class CodeBlocksProjectMatcher : public ProjectMatcher
{
public:

    CodeBlocksProjectMatcher( void );
    virtual ~CodeBlocksProjectMatcher( void );

    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Copy-constructor is not implemented.
    CodeBlocksProjectMatcher( const CodeBlocksProjectMatcher & );
    /// Copy-assignment operator is not implemented.
    CodeBlocksProjectMatcher & operator = ( const CodeBlocksProjectMatcher & );
};

// LineMatcherAndNotifier ---------------------------------------------------------------------

class LineMatcherAndNotifier : public OutputHandler
{
public:

    LineMatcherAndNotifier( ProjectMatcher * matcher, LineType::Type lineType,
        const char * sub, const char * color );
    inline virtual ~LineMatcherAndNotifier( void ) {}

    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Copy-constructor is not implemented.
    LineMatcherAndNotifier( const LineMatcherAndNotifier & );
    /// Copy-assignment operator is not implemented.
    LineMatcherAndNotifier & operator = ( const LineMatcherAndNotifier & );

    const char * m_sub;
    const char * m_color;
    ProjectMatcher * m_matcher;
    LineType::Type m_lineType;

};

// class TotalLineMatcher ---------------------------------------------------------------------

class TotalLineMatcher : public OutputHandler
{
public:

    TotalLineMatcher( void );
    virtual ~TotalLineMatcher( void );

    inline void SetProjectMatcher( ProjectMatcher * p ) { m_matcher = p; }

    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Copy-constructor is not implemented.
    TotalLineMatcher( const TotalLineMatcher & );
    /// Copy-assignment operator is not implemented.
    TotalLineMatcher & operator = ( const TotalLineMatcher & );

    void DoSimpleOutput( OutputReceiver & output, const char * content );
    void DoTotalOutput( OutputReceiver & output, const char * content );

    ProjectMatcher * m_matcher;
    mutable LineType::Type m_lineType;
};

// GrandTotalMatcher --------------------------------------------------------------------------

class GrandTotalMatcher : public OutputHandler
{
public:

    explicit GrandTotalMatcher( ProjectMatcher * projectMatcher );
    virtual ~GrandTotalMatcher( void );

    virtual bool Matches( const char * content ) const;
    virtual void Output( OutputReceiver & output, const char * content );

private:

    /// Default constructor is not implemented.
    GrandTotalMatcher( void );
    /// Copy-constructor is not implemented.
    GrandTotalMatcher( const GrandTotalMatcher & );
    /// Copy-assignment operator is not implemented.
    GrandTotalMatcher & operator = ( const GrandTotalMatcher & );

    ProjectMatcher * m_projectMatcher;
};

// class MatcherSet ---------------------------------------------------------------------------

class MatcherSet
{
public:

    struct MatchingRule
    {
        ::std::string m_content;
        LineType::Type m_lineType;

        MatchingRule( const ::std::string & content, LineType::Type lineType );
    };

    typedef ::std::vector< MatchingRule > MatchingRules;
    typedef MatchingRules::const_iterator MatchingRuleCIter;

    explicit MatcherSet( const char * compiler );
    MatcherSet( const char * compiler, const MatchingRules & rules );
    ~MatcherSet( void );

    void AddMatchingRules( const MatchingRules & rules );

    bool MatchLine( const char * line, ::std::stringstream & output );

    inline const char * GetCompiler( void ) const
    {
        return m_compiler;
    }

    inline ProjectMatcher * GetProjectMatcher( void )
    {
        return m_projectMatcher;
    }

    inline const ProjectMatcher * GetProjectMatcher( void ) const
    {
        return m_projectMatcher;
    }

private:

    typedef ::std::vector< OutputHandler * > HandlerSet;
    typedef HandlerSet::iterator HandlerSetIter;
    typedef HandlerSet::const_iterator HandlerSetCIter;

    MatcherSet( const MatcherSet & );
    MatcherSet & operator = ( const MatcherSet & );

    void SetupProjectMatcher( const char * compiler );

    void SetupCompilerRules( const char * compiler );

    HandlerSet m_handlers;
    ProjectMatcher * m_projectMatcher;
    const char * m_compiler;

};

// --------------------------------------------------------------------------------------------

} // end namespace hestia

#endif

// $Log: matchers.hpp,v $
// Revision 1.12  2008/12/11 20:48:58  rich_sposato
// Added ability to specify matching rules at runtime.
//
// Revision 1.11  2008/12/09 22:48:39  rich_sposato
// Added another constructor, and a bit of refactoring.
//
// Revision 1.10  2008/12/04 00:05:43  rich_sposato
// Improved parsing abilities so this can parse final line with grand totals.
//
// Revision 1.9  2008/12/03 17:47:55  rich_sposato
// Changed output handling when parsing build results made from sequential
// compiling instead of threaded compiling.
//
// Revision 1.8  2008/12/03 01:36:59  rich_sposato
// Changed matching classes so this can match MSVC9 build results with
// threaded output instead of just sequential output.
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
