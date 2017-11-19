// ----------------------------------------------------------------------------
// The C++ Unit Test Library
// Copyright (c) 2005, 2006, 2007, 2008 by Rich Sposato
//
// Permission to use, copy, modify, distribute and sell this software for any
// purpose is hereby granted under the terms stated in the GNU Library Public
// License, provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear in
// supporting documentation.
//
// ----------------------------------------------------------------------------

// $Header: /cvsroot/hestia/CppUnitTest/src/UnitTest.cpp,v 1.44 2008/03/22 21:36:39 rich_sposato Exp $


// ----------------------------------------------------------------------------

#include "../include/UnitTest.hpp"

#include <time.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <vector>
#include <iomanip>
#include <ostream>
#include <fstream>
#include <iterator>
#include <iostream>
#include <strstream>
#include <algorithm>
#include <functional>


// ----------------------------------------------------------------------------

#if defined( DEBUG ) || defined( _DEBUG )
    #define DEBUG_CODE( code ) code;
#else
    #define DEBUG_CODE( code ) ;
#endif

// define nullptr even though new compilers will have this keyword just so we
// have a consistent and easy way of identifying which uses of 0 mean null.
#define nullptr 0

using namespace ::std;

namespace
{


// ----------------------------------------------------------------------------

static const char * const s_DividerLine =
"-------------------------------------------------------------------------------";

static const char * s_testTimeStampFormatSpec  = "%A, %Y - %B - %d, %H : %M : %S";
static const char * s_timeStampFormatSpec = "%A, %Y %b %d at %H:%M:%S in %z";
static const char * s_beginTimeMessage    = "Unit Tests Began at: ";
static const char * s_endTimeStampMessage = "Unit Tests Ended at: ";

inline bool IsEmptyString( const char * s )
{
    return ( ( nullptr == s ) || ( '\0' == *s ) );
}

// ----------------------------------------------------------------------------

/// Actual container of UnitTest pointers.
typedef std::vector< ut::UnitTest * > TUnitTestChildren;

/// Iterator across container of UnitTest pointers.
typedef TUnitTestChildren::iterator TUnitTestChildrenIter;

/// Iterator across container of UnitTest pointers.
typedef TUnitTestChildren::const_iterator TUnitTestChildrenCIter;

/// Actual container of UnitTest pointers.
typedef std::vector< ut::UnitTestResultReceiver * > TUnitTestReceiverSet;

/// Iterator across container of UnitTest pointers.
typedef TUnitTestReceiverSet::iterator TUnitTestReceiverSetIter;

/// Iterator across container of UnitTest pointers.
typedef TUnitTestReceiverSet::const_iterator TUnitTestReceiverSetCIter;

// ----------------------------------------------------------------------------

/// Returns the color associated with the test result.
const char * GetColor( ut::TestResult::EnumType result )
{
    switch ( result )
    {
        case ut::TestResult::Passed:  return "green";
        case ut::TestResult::Warning: return "yellow";
        case ut::TestResult::Failed:  return "red";
        case ut::TestResult::Fatal:   return "red";
        case ut::TestResult::Thrown:  return "red";
        default: break;
    }
    return "white";
}

// ----------------------------------------------------------------------------

/** Returns the test result that occurs with the test severity level and
 whether the test passed.
 */
ut::TestResult::EnumType Convert( ut::UnitTest::TestLevel level, bool pass )
{
    if ( pass )
    {
        return ut::TestResult::Passed;
    }
    switch ( level )
    {
        case ut::UnitTest::Warning: return ut::TestResult::Warning;
        case ut::UnitTest::Checked: return ut::TestResult::Failed;
        case ut::UnitTest::Require: return ut::TestResult::Fatal;
        default: break;
    }
    return ut::TestResult::Failed;
}

//  struct HtmlResultColors ---------------------------------------------------

/** @struct HtmlResultColors
 @brief Stores names of colors placed on parts of HTML output pages.
 */
struct HtmlResultColors
{
    HtmlResultColors( unsigned int itemCount, unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int tossCount );

    const char * failColor;
    const char * warnColor;
    const char * passColor;
    const char * tossColor;
    const char * testColor;
    const char * textColor;  ///< Default text color.
    const char * testResult;
};

//  HtmlResultColors::HtmlResultColors ----------------------------------------

HtmlResultColors::HtmlResultColors( unsigned int itemCount,
    unsigned int passCount, unsigned int warnCount, unsigned int failCount,
    unsigned int tossCount ) :
    failColor( ( 0 == failCount ) ? "green" : "red" ),
    warnColor( ( 0 == warnCount ) ? "green" : "yellow" ),
    passColor( ( 0 <  passCount ) ? "green" : "red" ),
    tossColor( ( 0 == tossCount ) ? "green" : "red" ),
    testColor( "green" ),
    textColor( "white" ),
    testResult( "Passed" )
{
    if ( ( 0 < tossCount ) || ( 0 < failCount ) )
    {
        testColor = "red";
        testResult = "FAILED";
    }
    else if ( 0 < warnCount )
    {
        testColor = "yellow";
    }
    else if ( 0 == itemCount )
    {
        testColor = "white";
        testResult = "Empty!";
    }
}

// FindSubString --------------------------------------------------------------

/** Finds a sub-string within another string.
 @param[in] full String to be searched within.
 @param[in] sub Sub-string used to search inside full string.
 @return NULL if either parameter is NULL or empty.  Terminating NIL char of full
  string if the sub-string is not found.  Otherwise, pointer to first occurrence of
  sub-string within full string.
 */
const char * FindSubString( const char * full, const char * sub )
{
    if ( IsEmptyString( full ) )
        return nullptr;
    if ( IsEmptyString( sub ) )
        return nullptr;

    const unsigned int size = static_cast< unsigned int >( ::strlen( sub ) );
    const char * place = full;
    while ( *place != '\0' )
    {
        if ( ::strncmp( place, sub, size ) == 0 )
            return place;
        ++place;
    }

    return place;
}

// FindSubString ------------------------------------------------------------------------------

/// Non-const version of FindSubString.
inline char * FindSubString( char * full, const char * sub )
{
    return const_cast< char * >( FindSubString( full, sub ) );
}

// IsPathDelimiter ----------------------------------------------------------------------------

/// Returns true if char is a path delimiter.
bool IsPathDelimiter( char ch )
{
    return ( ':'  == ch )
        || ( '/'  == ch )
        || ( '\\' == ch );
}

// FindFileNameInPath ---------------------------------------------------------

/** Returns pointer to last part of file path - which is assumed to contain
 the path name.
 */
const char * FindFileNameInPath( const char * path )
{

    if ( IsEmptyString( path ) )
    {
        return nullptr;
    }
    const char * pc = path + ::strlen( path );
    for ( --pc; pc != path; --pc )
    {
        if ( IsPathDelimiter( *pc ) )
        {
            ++pc;
            break;
        }
    }
    return pc;
}

// FindFileNameInPath ---------------------------------------------------------

/// Non-const version of FindFileNameInPath.
inline char * FindFileNameInPath( char * path )
{
    return const_cast< char * >( FindFileNameInPath( path ) );
}

// --------------------------------------------------------------------------------------------

void PrepFileName( const char * partialName, const char * timestamp,
    const char * extension, string & output )
{
    string temp( partialName );
    temp.append( timestamp );
    temp.append( extension );
    temp.swap( output );
}

// --------------------------------------------------------------------------------------------

/** Makes filenames for text file, current HTML file, and for main HTML page.
 @param[in] timestamp Pointer to timestamp information.
 @param[in] textFilePartialName Path and partial name of text file.
 @param[in] htmlFilePartialName Path and partial name of main html page.
 @param[in] xmlFilePartialName Path and partial name of xml page.
 @param[out] textFileName Name of text file for current tests.  The text file
   name is "name_YYYY_MM_DD_hh_mm.txt" where "name" is the input parameter.
 @param[out] currentHtmlFileName Name of HTML file for current tests.  The
  current HTML file name is "name_YYYY_MM_DD_hh_mm.html".
 @param[out] mainHtmlFileName Name of HTML file for main page.  The main HTML
  file name is "name.html" where "name" is the input parameter.
 @param[out] xmlFileName Name of XML file that stores unit test results.  The
  filename has this format: "name_YYYY_MM_DD_hh_mm.xml".
 */
bool MakeFileNames( const time_t * startTime, const char * textFilePartialName,
    const char * htmlFilePartialName, const char * xmlFilePartialName,
    string & textFileName, string & currentHtmlFileName,
    string & mainHtmlFileName, string & xmlFileName )
{

    textFileName.clear();
    currentHtmlFileName.clear();
    mainHtmlFileName.clear();
    xmlFileName.clear();

    const struct tm * timeStamp = ::localtime( startTime );
    char buffer[ 128 ];
    ::strftime( buffer, sizeof(buffer)-1, "_%Y_%m_%d_%H_%M_%S", timeStamp );

    if ( !IsEmptyString( htmlFilePartialName ) )
    {
        PrepFileName( htmlFilePartialName, buffer, ".html",
            currentHtmlFileName );
        mainHtmlFileName.assign( htmlFilePartialName );
        mainHtmlFileName.append( ".html" );
    }

    if ( !IsEmptyString( textFilePartialName ) )
    {
        PrepFileName( textFilePartialName, buffer, ".txt", textFileName );
    }

    if ( !IsEmptyString( xmlFilePartialName ) )
    {
        PrepFileName( xmlFilePartialName, buffer, ".xml", xmlFileName );
    }

    return true;
}

// ChangeEmbeddedNils -------------------------------------------------------------------------

/** Replaces NIL chars ( '\0' ) within a C++ string.
 @param[in-out] ss C++ string to be searched for embedded NIL chars.
 @param[in] ch Replacement char - default is a space.
 @return True if any NIL char found.
 */
bool ChangeEmbeddedNils( string & ss, char ch = ' ' )
{

    bool found = false;
    string::iterator here( ss.begin() );
    string::iterator last( ss.end() );
    while ( here != last )
    {
        if ( '\0' == *here )
        {
            found = true;
            *here = ch;
        }
        ++here;
    }

    return found;
}

// ReadFileIntoString -------------------------------------------------------------------------

/** Reads the entire contents of a text file into a C++ string.  Whitespaces are
  not skipped.  Any embedded NIL chars in the file are replaced with spaces.
 @param[in] filename Name of file to be opened.
 @target[out] String where file contents are placed.
 @return True if file was opened for reading, else false.
 */
bool ReadFileIntoString( const char * filename, string & target )
{

    target.clear();
    ifstream input( filename );
    if ( input.fail() )
        return false;
    input.unsetf( ios::skipws ); // Must turn off whitespace skipping.

    istream_iterator< char > first( input );
    istream_iterator< char > last;
    string temp( first, last );
    ChangeEmbeddedNils( temp );
    temp.swap( target );

    return true;
}

// MakeColorTable -----------------------------------------------------------------------------

/** Makes an html table describing purpose of colors.
 @param[in-out] output File-stream receiving HTML contents.
 */
void MakeColorTable( ofstream & output )
{

    output << "<table border=1 cellspacing=0 cellpadding=3>" << endl;
    output << "<caption><em>Legend</em></caption>" << endl;
    output << "<tr><th>Color</th><th>Purpose</th></tr>" << endl;
    output << "<tr><td>white</td><td>information</td></tr>" << endl;
    output << "<tr><td><span style=\"color: green\">green</span></td>"
           <<     "<td><span style=\"color: green\">passed</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: yellow\">yellow</span></td>"
           <<     "<td><span style=\"color: yellow\">warning</span></td></tr>" << endl;
    output << "<tr><td><span style=\"color: red\">red</span></td>"
           <<     "<td><span style=\"color: red\">failure</span></td></tr>" << endl;
    output << "</table><br>" << endl;
}

// ----------------------------------------------------------------------------

/** struct FChildNameFinder
 @brief Comparator decides if a UnitTest has a specific name.
 */
struct FChildNameFinder : public unary_function< const ut::UnitTest *, bool >
{
    inline explicit FChildNameFinder( const char * name )
        : m_name( name ) {}
    inline bool operator () ( const ut::UnitTest * pTest ) const
    { return ( 0 == ::strcmp( pTest->GetName(), m_name ) ); }
    const char * m_name;
};

// ----------------------------------------------------------------------------

template < typename T >
class InvariantChecker
{
public:
    inline explicit InvariantChecker( const T * p ) : m_p( p ) {}
    inline ~InvariantChecker( void ) { m_p->CheckInvariants(); }
    const T * m_p;
};

typedef InvariantChecker< ::ut::UnitTest > UnitTestChecker;
typedef InvariantChecker< ::ut::UnitTestSetImpl > UnitTestSetChecker;


// ----------------------------------------------------------------------------

class FileOutputter
{
public:

    /** Tries to open output file.
     @param fileName Path and full name of output file.
     @return True if file was opened.  False if name is NULL or empty, or an
      error occurred when opening file.
     */
    bool Open( const char * fileName );

    inline bool IsOpen( void ) const { return m_isOpen; }

    void Close( void );

protected:

    /// Constructor does not open output stream.
    FileOutputter( void );

    /// Destructor closes output stream.
    virtual ~FileOutputter( void );

    /// Output file stream.
    ofstream m_outFile;

    /// True if stream can receive output.
    bool m_isOpen;

private:
    /// Copy-constructor is not implemented.
    FileOutputter( const FileOutputter & );
    /// Copy-assignment operator is not implemented.
    FileOutputter & operator = ( const FileOutputter & );
};

// ----------------------------------------------------------------------------

/** @class TextOutputter
 @brief A strategy class for sending unit test results to either a text file or
 to standard output.  Since the text file has identical content as standard
 output, the same strategy sends it output to both.
 */
class TextOutputter : public FileOutputter, public ut::UnitTestResultReceiver
{
public:

    /** Creates text-outputting strategy.
     @param options Options for text output.
     */
    explicit TextOutputter( ::ut::UnitTestSet::OutputOptions options );
    virtual ~TextOutputter( void );

    virtual bool Start( const char * name );
    virtual bool ShowMessage( const ut::UnitTest * test,
        const char * fileName, unsigned int line, const char * message );
    virtual bool ShowTestHeader( const ut::UnitTest * test );
    virtual bool ShowTestLine( const ut::UnitTest * test,
        ut::TestResult::EnumType result, const char * fileName,
        unsigned int line, const char * expression, const char * message );
    virtual bool ShowTimeStamp( bool isStartTime, const char * timestamp );
    virtual bool StartSummaryTable( void );
    virtual bool ShowTableLine( const ut::UnitTest * test );
    virtual bool ShowTotalLine( unsigned int passCount, unsigned int warnCount,
        unsigned int failCount, unsigned int exceptCount,
        unsigned int itemCount );
    virtual bool ShowSummaryLine( unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int exceptCount, unsigned int testCount );
    virtual bool EndSummaryTable( void );

    inline void SetFileName( const char * name ) { m_filename = name; }

    inline const char * GetFileName( void ) const
    { return ( m_isOpen ) ? m_filename.c_str() : nullptr; }

    inline bool DoesOutput( void ) const
    {
        return ( m_isOpen || m_sendToCout || m_sendToCerr );
    }
    inline bool DoesSendToCout( void ) const { return m_sendToCout; }
    inline bool DoesSendToCerr( void ) const { return m_sendToCerr; }

    inline bool ShowDividers( void ) const { return m_showDividers; }
    inline bool DoesShowIndexes( void ) const { return m_showIndexes; }

private:
    /// Default-constructor is not implemented.
    TextOutputter( void );
    /// Copy-constructor is not implemented.
    TextOutputter( const TextOutputter & );
    /// Copy-assignment operator is not implemented.
    TextOutputter & operator = ( const TextOutputter & );

    /// True if content is sent to standard output.
    bool m_sendToCout;

    /// True if content is sent to standard error.
    bool m_sendToCerr;

    /// True if dividing lines are added to output.
    bool m_showDividers;

    /// True if this adds UnitTest index to each output line.
    bool m_showIndexes;

    /// Path and name of text output file.
    string m_filename;
};

// ----------------------------------------------------------------------------

/** @class HtmlOutputter
 @brief A strategy class for sending unit test results to an html file.
 */
class HtmlOutputter : public FileOutputter, public ut::UnitTestResultReceiver
{
public:
    HtmlOutputter( const ut::UnitTestSetImpl * info,
        ::ut::UnitTestSet::OutputOptions options );
    virtual ~HtmlOutputter( void );

    virtual bool Start( const char * name );
    virtual bool ShowMessage( const ut::UnitTest * test,
        const char * fileName, unsigned int line, const char * message );
    virtual bool ShowTestHeader( const ut::UnitTest * test );
    virtual bool ShowTestLine( const ut::UnitTest * test,
        ut::TestResult::EnumType result, const char * fileName,
        unsigned int line, const char * expression, const char * message );
    virtual bool ShowTimeStamp( bool isStartTime, const char * timestamp );
    virtual bool StartSummaryTable( void );
    virtual bool ShowTableLine( const ut::UnitTest * test );
    virtual bool ShowTotalLine( unsigned int passCount, unsigned int warnCount,
        unsigned int failCount, unsigned int exceptCount,
        unsigned int itemCount );
    virtual bool ShowSummaryLine( unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int exceptCount, unsigned int testCount );
    virtual bool EndSummaryTable( void );

    inline bool DoesOutput( void ) const { return m_isOpen; }

    /// Provides path and name of main page for HTML output.
    inline void SetMainPageName( const char * filename )
    { m_mainPageName = filename; }

    inline const char * GetMainPageName( void ) const
    { return ( m_isOpen ) ? m_mainPageName.c_str() : nullptr; }

    /// Provides path and name of current page for HTML output.
    inline void SetCurrentPageName( const char * filename )
    { m_htmlFileName = filename; }

    inline const char * GetCurrentPageName( void ) const
    { return ( m_isOpen ) ? m_htmlFileName.c_str() : nullptr; }

    /// Sets pointer to timestamp info.
    inline void SetTimeStampInfo( const time_t * timestamp )
    { m_startTime = timestamp; }

    inline bool DoesShowIndexes( void ) const { return m_showIndexes; }

private:

    /// Default-constructor is not implemented.
    HtmlOutputter( void );
    /// Copy-constructor is not implemented.
    HtmlOutputter( const HtmlOutputter & );
    /// Copy-assignment operator is not implemented.
    HtmlOutputter & operator = ( const HtmlOutputter & );

    /// Pointer to implementation details for unit tests.
    const ut::UnitTestSetImpl * m_info;

    /// Pointer to timestamp info.
    const time_t * m_startTime;

    /// True if this adds UnitTest index to each output line.
    bool m_showIndexes;

    /// Name of file with current HTML page.
    string m_htmlFileName;

    /// Name of file containing main HTML page.
    string m_mainPageName;
};

// ----------------------------------------------------------------------------

/** @class XmlOutputter
 @brief A strategy class for sending unit test results to an xml file.
 */
class XmlOutputter : public FileOutputter, public ut::UnitTestResultReceiver
{
public:

    /** Creates text-outputting strategy.
     @param options Field of boolean flags for output options.
     */
    explicit XmlOutputter( ::ut::UnitTestSet::OutputOptions options );
    virtual ~XmlOutputter( void );

    virtual bool Start( const char * name );
    virtual bool ShowMessage( const ut::UnitTest * test,
        const char * fileName, unsigned int line, const char * message );
    virtual bool ShowTestHeader( const ut::UnitTest * test );
    virtual bool ShowTestLine( const ut::UnitTest * test,
        ut::TestResult::EnumType result, const char * fileName,
        unsigned int line, const char * expression, const char * message );
    virtual bool ShowTimeStamp( bool isStartTime, const char * timestamp );
    virtual bool StartSummaryTable( void );
    virtual bool ShowTableLine( const ut::UnitTest * test );
    virtual bool ShowTotalLine( unsigned int passCount, unsigned int warnCount,
        unsigned int failCount, unsigned int exceptCount,
        unsigned int itemCount );
    virtual bool ShowSummaryLine( unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int exceptCount, unsigned int testCount );
    virtual bool EndSummaryTable( void );

    inline void SetFileName( const char * name ) { m_filename = name; }

    inline const char * GetFileName( void ) const
    { return ( m_isOpen ) ? m_filename.c_str() : nullptr; }

    inline bool DoesOutput( void ) const { return ( m_isOpen ); }

    inline bool DoesShowIndexes( void ) const { return m_showIndexes; }

private:
    /// Default-constructor is not implemented.
    XmlOutputter( void );
    /// Copy-constructor is not implemented.
    XmlOutputter( const XmlOutputter & );
    /// Copy-assignment operator is not implemented.
    XmlOutputter & operator = ( const XmlOutputter & );

    /// True if this adds UnitTest index to each output line.
    bool m_showIndexes;

    /// Path and name of text output file.
    string m_filename;
};

// ----------------------------------------------------------------------------

}; // end anonymous namespace

namespace ut
{

UnitTestSet * UnitTestSet::s_instance = nullptr;


// ----------------------------------------------------------------------------

const char * ut::TestResult::GetName( ut::TestResult::EnumType result )
{
    switch ( result )
    {
        case ut::TestResult::Passed:  return "Passed";
        case ut::TestResult::Warning: return "Warning";
        case ut::TestResult::Failed:  return "FAILED";
        case ut::TestResult::Fatal:   return "FATAL";
        case ut::TestResult::Thrown:  return "THROWN";
        default: break;
    }
    return "UNKNOWN";
}

// ----------------------------------------------------------------------------

/** @class UnitTestSetImpl
 @brief Contains implementation details for UnitTestSet singleton so those
 details are not exposed to host program.
 */
class UnitTestSetImpl
{
public:

    /** Constructs storage for unit tests, and how to present test results.
     @param testName Name of overall set of unit tests.
     @param textFileName Path and part of filename used to store test results
      in text form.  If neither NULL nor empty string, then test results are
      sent to a text file.  This adds a timestamp into the filename and then
      appends ".txt" extension onto the filename.
     @param htmlFileName Path and part of filename used to store test results
      in HTML form.  This is the filename of a "main page" of test results -
      which contains links to one or more HTML files showing results of unit
      test executions.  If neither NULL nor empty string, then test results are
      sent to an html file.  This adds a timestamp into the filename and then
      appends ".html" extension onto the filename.
     @param xmlFileName Path and part of filename used to store test results
      in XML form.  If neither NULL nor empty string, then test results are
      sent to an XML file.  This adds a timestamp into the filename and then
      appends ".xml" extension onto the filename.
     @param options A bitfield of all possible output options - each bit is a
      different option.
     */
    UnitTestSetImpl( const char * testName, const char * textFileName,
        const char * htmlFileName, const char * xmlFileName,
        ut::UnitTestSet::OutputOptions info );

    /// Destructor goes through all the UnitTest's and deletes them.
    ~UnitTestSetImpl( void );

    /// Returns pointer to UnitTest that matches given name.
    const ut::UnitTest * GetUnitTest( const char * unitTestName ) const;

    /// Creates initial page header that precedes all test result output.
    void StartOutput( void );

    /// Creates the header for a specific unit test.
    void OutputTestHeader( const ut::UnitTest * test );

    /** Called to output test results for an individual unit test item.
     @param test Pointer to UnitTest used to tabulate results.
     @param result Severity level of test item.
     @param fileName Name of source code file.
     @param line Source code line.
     @param expression Conditional expression that was evaluated.
     @param message Optional message placed into output.
     */
    void OutputTestLine( ut::UnitTest * test, ut::TestResult::EnumType result,
        const char * fileName, unsigned int line, const char * expression,
        const char * message );

    /** Called to output a text message, but not as part of test.
     @param test Pointer to UnitTest used to tabulate results.
     @param fileName Name of source code file.
     @param line Source code line.
     @param message Text message placed into output.
     */
    void OutputMessage( UnitTest * test, const char * fileName,
        unsigned int line, const char * message );

    /// Sends summary information (timestamps & table) to output.
    void OutputSummaryInfo( void );

    inline bool ShowPasses( void ) const { return m_showPasses; }
    inline bool ShowWarnings( void ) const { return m_showWarnings; }

    inline unsigned int GetTestCount( void ) const { return m_testCount; }
    inline unsigned int GetTestPassCount( void ) const { return m_testPassCount; }
    inline unsigned int GetTestWarnCount( void ) const { return m_testWarnCount; }
    inline unsigned int GetTestFailCount( void ) const { return m_testFailCount; }
    inline unsigned int GetTestExceptCount( void ) const { return m_testExceptCount; }

    inline unsigned int GetItemCount( void ) const { return m_itemCount; }
    inline unsigned int GetItemPassCount( void ) const { return m_itemPassCount; }
    inline unsigned int GetItemWarnCount( void ) const { return m_itemWarnCount; }
    inline unsigned int GetItemFailCount( void ) const { return m_itemFailCount; }
    inline unsigned int GetItemExceptCount( void ) const { return m_itemExceptCount; }

    const char * GetTestName( void ) const { return m_testName.c_str(); }

    /** True if currently calling test result receivers or modifying container
     of pointers to receivers.
     */
    inline bool IsUsingReceivers( void ) const { return m_usingReceivers; }

    /// Checks if any class invariants were broken.
    void CheckInvariants( void ) const;

private:

    friend class ut::UnitTestSet;

    /// Default-constructor is not implemented.
    UnitTestSetImpl( void );
    /// Copy-constructor is not implemented.
    UnitTestSetImpl( const UnitTestSetImpl & );
    /// Copy-assignment operator is not implemented.
    UnitTestSetImpl & operator = ( const UnitTestSetImpl & );

    /// Clears contents associated with unit test results.
    void Clear( void );

    /// Sets up text file, standard-output, and html file receivers.
    void SetupInternalReceivers( void );

    /// Adds start and stop timestamps to output.
    void ShowTimeStamp( void );

    /// Creates detail rows in the summary table - one row for each unit test.
    void MakeTableRows( void );

    /// Creates summary table for output.
    void OutputSummaryTable( void );

    /// Overall name of test.
    string m_testName;

    /// First part of name of text test result file.
    string m_textFilePartialName;

    /// First part of name of html test result files.
    string m_htmlFilePartialName;

    /// First part of name of xml test result files.
    string m_xmlFilePartialName;

    /// Container of UnitTest's.
    TUnitTestChildren m_tests;

    /// Output handler for text files and standard output.
    TextOutputter m_textOutput;

    /// Output handler for HTML files.
    HtmlOutputter m_htmlOutput;

    /// Output handler for xml files.
    XmlOutputter m_xmlOutput;

    /// Container of output receivers.
    TUnitTestReceiverSet m_receivers;

    /// True if messages without test items are sent to output.
    bool m_showMessages;

    /// True if this sends passing items to output.
    bool m_showPasses;

    /// True if this sends warning items to output.
    bool m_showWarnings;

    /// True if this shows UnitTest headers.
    bool m_showHeaders;

    /// True if this adds beginning and ending timestamps.
    bool m_showTimeStamp;

    /// True if this uses full weekday name (e.g. - "Friday", not "Fri").
    bool m_useFullDayName;

    /// True if host program wants a summary table at end of output.
    bool m_showFinalTable;

    /// True if this made a page header already.
    bool m_didPageHeader;

    /** True if currently calling test result receivers or modifying container
     of pointers to receivers.  UnitTestSetImpl will check this boolean flag to
     prevent re-entrancy in case a receiver calls a function in UnitTest or
     UnitTestSet.
     */
    bool m_usingReceivers;

    /// True if any unit test was exercised.
    bool m_didAnyTest;

    /// True if ran through entire set of tests already.
    bool m_didFirstRun;

    /// Storage for timestamp.
    time_t m_startTime;

    /// Number of tests.
    unsigned int m_testCount;

    /// Number of tests that passed.
    unsigned int m_testPassCount;

    /// Number of test with at least 1 warning.
    unsigned int m_testWarnCount;

    /// Number of tests that failed.
    unsigned int m_testFailCount;

    /// Number of tests with exceptions.
    unsigned int m_testExceptCount;

    /// Total # of items among all tests.
    unsigned int m_itemCount;

    /// Total # of passing items among all tests.
    unsigned int m_itemPassCount;

    /// Total # of warning items among all tests.
    unsigned int m_itemWarnCount;

    /// Total # of failing items among all tests.
    unsigned int m_itemFailCount;

    /// Total # of items that had exceptions among all tests.
    unsigned int m_itemExceptCount;

    /// Buffer contains timestamp of when tests began.
    char m_timeString[ 160 ];
};

// ----------------------------------------------------------------------------

} // end namespace ut

namespace
{

// MainPageMaker -----------------------------------------------------------------------------

/** @class MainPageMaker Creates the main unit-test-result page with links to the
 other pages.
 */
class MainPageMaker
{
public:

    /// Max # of rows allowed in main HTML page.
    static const unsigned int s_MaxRowCount;

    /** Constructor provides the parameters needed to make the main page.
     @param[in] info Information about current test results.
     @param[in] today Pointer to timestamp.
     @param[in] mainpageFilename Name of html file for main page.
     @param[in] outputFilename Name of html file for current test page.
     */
    MainPageMaker( const ut::UnitTestSetImpl * info, const time_t * timestamp,
        const char * mainpageFilename, const char * outputFilename );

    /// Destructor.
    ~MainPageMaker( void );

    /// Return true if file is opened.
    bool IsOpen( void ) const;

    /** Adds current info to file content.  It first sends some of the existing file
     contents into the output file stream, overwriting the existing file in the
     process.  Once it gets to the first row in the test table, it adds the current
     test information to the file stream.  Then it sends the next however many rows
     from the table to output - but not more than the upper limit for the table.
     The remaining rows are skipped, and any content after the table is sent to output.
     @param[in] Contents of existing main page.
     */
    bool AddToContent( const string & fileContent );

    /// Creates HTML file of main test page.
    bool CreateFile( void );

private:
    /// Not implemented.
    MainPageMaker( void );
    /// Copy-constructor is not implemented.
    MainPageMaker( const MainPageMaker & );
    /// Copy-assignment operator is not implemented.
    MainPageMaker & operator = ( const MainPageMaker & );

    /** Adds project totals from current test to table.  The row is color-coded
     to match overall test results.  A link is placed in the row for the current
     test page.
     */
    bool AddCurrentOutput( void );

    /// Makes ending tags for HTML table.
    void MakeEndOfTable( void );

    /// File output stream used to make HTML file.
    ofstream m_output;

    /// Name of output file.
    const char * m_outputFilename;

    /// Pointer to timestamp.
    const time_t * m_startTime;

    /// Pointer to implementation details for unit tests.
    const ut::UnitTestSetImpl * m_info;
};

const unsigned int MainPageMaker::s_MaxRowCount = 90;

// MainPageMaker::MainPageMaker ---------------------------------------------------------------

MainPageMaker::MainPageMaker( const ut::UnitTestSetImpl * info,
    const time_t * timestamp, const char * mainpageFilename,
    const char * outputFilename ) :
    m_output( mainpageFilename ),
    m_outputFilename( outputFilename ),
    m_startTime( timestamp ),
    m_info( info )
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
#if defined (_MSC_VER)
    return m_output.is_open();
#else
    // Have to cast away const because implementation for std::ofstream
    // does not provide const version of is_open.
    MainPageMaker * pThis = const_cast< MainPageMaker * >( this );
    return pThis->m_output.is_open();
#endif
}

// MainPageMaker::AddToContent ----------------------------------------------------------------

bool MainPageMaker::AddToContent( const string & fileContent )
{
    assert( nullptr != this );

    const char * content = fileContent.c_str();
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

    // decide if file already has more than enough rows.
    const char * rowNow = firstRow;
    const char * nextRow = rowNow;
    const char * endOfTable = FindSubString( nextRow, "</table>" );
    unsigned int count = 1; 
    while ( count < s_MaxRowCount )
    {
        nextRow = FindSubString( rowNow, "<tr>" );
        if ( ( endOfTable < nextRow ) || IsEmptyString( nextRow ) )
        {
            // next row is past end of current table, so it must be inside next table.
            break;
        }
        rowNow = nextRow+1;
        ++count;
    }

    if ( ( count < s_MaxRowCount ) || IsEmptyString( nextRow ) )
        m_output << firstRow;
    else
    {
        temp.assign( firstRow, nextRow-firstRow );
        m_output << temp;
        m_output << endOfTable;
    }

    return true;
}

// MainPageMaker::AddCurrentOutput ------------------------------------------------------------

bool MainPageMaker::AddCurrentOutput( void )
{
    assert( nullptr != this );

    char buffer[ 128 ];
    const unsigned int testCount = m_info->GetTestCount();
    const unsigned int testFailCount = m_info->GetTestFailCount();
    const unsigned int testTossCount = m_info->GetTestExceptCount();
    const unsigned int testWarnCount = m_info->GetTestWarnCount();
    const unsigned int testPassCount = m_info->GetTestPassCount();
    const unsigned int itemPassCount = m_info->GetItemPassCount();
    const unsigned int itemWarnCount = m_info->GetItemWarnCount();
    const unsigned int itemFailCount = m_info->GetItemFailCount();
    const unsigned int itemTossCount = m_info->GetItemExceptCount();
    const unsigned int itemCount = m_info->GetItemCount();
    HtmlResultColors colors( itemCount, itemPassCount, itemWarnCount,
        itemFailCount, itemTossCount);

    const char * testName = m_info->GetTestName();
    string filenameLink( "./");
    filenameLink += FindFileNameInPath( m_outputFilename );
    const struct tm * timeStamp = ::localtime( m_startTime );
    ::strftime( buffer, sizeof(buffer)-1, s_testTimeStampFormatSpec,
        timeStamp );
    m_output << "<tr><td><a href=\'" << filenameLink << "\'>"
             << buffer << "</a></td>" << endl;
    m_output << "<td>" << testName << "</td>" << endl;
    m_output << "<td><span style=\"color: " << colors.testColor << "\">"
             << colors.testResult << "</span></td>" << endl;

    m_output << "<td><span style=\"color: " << colors.passColor << "\">"
             << itemPassCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.warnColor << "\">"
             << itemWarnCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.failColor << "\">"
             << itemFailCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.tossColor << "\">"
             << itemTossCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.textColor << "\">"
             << itemCount << "</span></td>" << endl;

    m_output << "<td><span style=\"color: " << colors.passColor << "\">"
             << testPassCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.warnColor << "\">"
             << testWarnCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.failColor << "\">"
             << testFailCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.tossColor << "\">"
             << testTossCount << "</span></td>" << endl;
    m_output << "<td><span style=\"color: " << colors.textColor << "\">"
             << testCount << "</span></td></tr>" << endl;

    return true;
}

// --------------------------------------------------------------------------------------------

bool MainPageMaker::CreateFile( void )
{
    assert( nullptr != this );

// Time Stamp   Test Name   Result   Items   Items   Total   Tests   Tests   Total
//                                   Passed  Failed  Items   Passed  Failed  Tests

    m_output << "<html><head><title>Unit Test Results</title></head>" << endl
             << "<body bgcolor=black text=white>" << endl;

    m_output << "<br><table border=1 cellspacing=0 cellpadding=3>" << endl;
    m_output << "<caption><em>Unit Test Results</em></caption>" << endl;
    m_output << "<tr><th>Time Stamp</th><th>Test Name</th><th>Result</th>"
             << "<th>Items<br>Passed</th><th>Warning<br>Items</th>"
             << "<th>Items<br>Failed</th>"
             << "<th>Items<br>Thrown</th><th>Total<br>Items</th>"
             << "<th>Tests<br>Passed</th><th>Tests w/<br>Warnings</th>"
             << "<th>Tests<br>Failed</th>"
             << "<th>Tests<br>Thrown</th><th>Total<br>Tests</th></tr>" << endl;
    if ( !AddCurrentOutput() )
        return false;
    MakeEndOfTable();

    return true;
}

// --------------------------------------------------------------------------------------------

void MainPageMaker::MakeEndOfTable( void )
{
    assert( nullptr != this );

    m_output << "</table><br>" << endl;
    MakeColorTable( m_output );
    m_output << "</body>" << endl << "</html>" << endl;
}

// ----------------------------------------------------------------------------

FileOutputter::FileOutputter( void ) :
    m_outFile(),
    m_isOpen( false )
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

FileOutputter::~FileOutputter( void )
{
    assert( nullptr != this );
    Close();
}

// ----------------------------------------------------------------------------

bool FileOutputter::Open( const char * fileName )
{
    assert( nullptr != this );
    if ( IsEmptyString( fileName ) )
        return false;
    m_outFile.open( fileName, ios::out | ios::trunc );
    m_isOpen = m_outFile.is_open();
    return m_isOpen;
}

// ----------------------------------------------------------------------------

void FileOutputter::Close( void )
{
    assert( nullptr != this );
    if ( m_isOpen )
    {
        m_outFile.close();
        m_isOpen = false;
    }
}

// ----------------------------------------------------------------------------

TextOutputter::TextOutputter( ::ut::UnitTestSet::OutputOptions options ) :
    FileOutputter(),
    UnitTestResultReceiver(),
    m_sendToCout( 0 != ( options & ut::UnitTestSet::SendToCout ) ),
    m_sendToCerr( 0 != ( options & ut::UnitTestSet::SendToCerr ) ),
    m_showDividers( 0 != ( options & ut::UnitTestSet::Dividers ) ),
    m_showIndexes( 0 != ( options & ut::UnitTestSet::AddTestIndex ) ),
    m_filename()
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

TextOutputter::~TextOutputter( void )
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

bool TextOutputter::Start( const char * name )
{
    assert( nullptr != this );
    assert( !IsEmptyString( name ) );
    assert( !m_isOpen );

    Open( m_filename.c_str() );
    if ( !DoesOutput() )
        return false;
    const char * nameHeader = "Name of Unit Tests: ";
    if ( m_sendToCout )
    {
        cout << nameHeader << name << endl << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << nameHeader << name << endl << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << nameHeader << name << endl << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

const char * CheckFilenameSize( const char * fileName, unsigned int & fieldSize )
{
    static const unsigned int s_maxfileNameSize = 256;
    assert( fileName != nullptr );
    const unsigned int fileNameSize = static_cast< unsigned int >
        ( ::strlen( fileName ) );
    fieldSize = min( fileNameSize, s_maxfileNameSize );
    if ( fieldSize == s_maxfileNameSize )
        fileName += ( fileNameSize - s_maxfileNameSize );
    return fileName;
}

// ----------------------------------------------------------------------------

unsigned int CheckMessageSize( const char * s )
{
    static const unsigned int s_maxMessageSize = 400;
    assert( s != nullptr );
    const unsigned int messageSize = static_cast< unsigned int >( ::strlen( s ) );
    const unsigned int fieldSize = min( messageSize, s_maxMessageSize );
    return fieldSize;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowMessage( const ut::UnitTest * test,
    const char * fileName, unsigned int line, const char * message )
{
    assert( nullptr != this );
    if ( !DoesOutput() )
        return false;

    char buffer[ 2048 ];
    strstream content( buffer, sizeof(buffer) );
    unsigned int fieldSize = 0;
    fileName = CheckFilenameSize( fileName, fieldSize );
    content << setw( fieldSize ) << fileName << '(' << line << ") : ";
    if ( m_showIndexes )
        content << test->GetIndex() << ' ';
    fieldSize = CheckMessageSize( message );
    content << setw( fieldSize ) << message << endl << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowTestHeader( const ut::UnitTest * test )
{
    assert( nullptr != this );
    if ( !DoesOutput() )
        return false;

    const unsigned int index = test->GetIndex();
    const char * testName = test->GetName();
    char buffer[ 1024 ];
    strstream content( buffer, sizeof(buffer) );
    content << "\nTest: " << setw( 3 ) << setfill( ' ' ) << index
            << "\t\t" << testName << endl;
    if ( m_showDividers )
        content << s_DividerLine << endl;
    content << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }
    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowTestLine( const ut::UnitTest * test,
    ut::TestResult::EnumType result, const char * fileName, unsigned int line,
    const char * expression, const char * message )
{
    assert( nullptr != this );

    if ( !DoesOutput() )
        return false;

    const char * pResult = ut::TestResult::GetName( result );
    char buffer[ 2048 ];
    strstream content( buffer, sizeof(buffer) );
    unsigned int fieldSize = 0;
    fileName = CheckFilenameSize( fileName, fieldSize );
    content << setw( fieldSize ) << fileName << '(' << line << ") : "
        << setw( 8 ) << setfill( ' ' ) << pResult << "  ";
    if ( m_showIndexes )
    {
        content << "  " << test->GetIndex() << ':' << test->GetItemCount();
    }
    else
    {
        content << setw( 4 ) << setfill( ' ' ) << test->GetItemCount();
    }
    fieldSize = CheckMessageSize( expression );
    content << "  (" << setw( fieldSize ) << expression << ')';
    if ( !IsEmptyString( message ) )
    {
        fieldSize = CheckMessageSize( message );
        content << "  " << setw( fieldSize ) << message;
    }
    content << endl << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }

    if ( ut::TestResult::Fatal == result )
    {
        char buffer2[ 256 ];
        strstream content2( buffer2, sizeof(buffer2) );
        content2 << "A required test failed!" << endl
                 << "Unable to safely execute further tests!" << endl
                 << "Program must exit now!" << endl << ends;
        if ( m_sendToCout )
        {
            cout << buffer2 << flush;
        }
        if ( m_sendToCerr )
        {
            cerr << buffer2 << flush;
        }
        if ( m_isOpen )
        {
            m_outFile << buffer2 << flush;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowTimeStamp( bool isStartTime, const char * timestamp )
{
    assert( nullptr != this );
    assert( !IsEmptyString( timestamp ) );

    if ( !DoesOutput() )
        return false;
    const char * description =
        ( isStartTime ) ? s_beginTimeMessage : s_endTimeStampMessage;
    if ( m_sendToCout )
    {
        if ( isStartTime )
            cout << endl;
        cout << description << timestamp << endl << flush;
    }
    if ( m_sendToCerr )
    {
        if ( isStartTime )
            cerr << endl;
        cerr << description << timestamp << endl << flush;
    }
    if ( m_isOpen )
    {
        if ( isStartTime )
            m_outFile << endl;
        m_outFile << description << timestamp << endl << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::StartSummaryTable( void )
{
    assert( nullptr != this );
    if ( !DoesOutput() )
        return false;

    static const char * const s_titleLine =
        "\n\t#  Unit Test Name\t\tPassed\tWarning\tFailed\tThrown\tTested";
    char buffer[ 256 ];
    strstream content( buffer, sizeof(buffer) );
    content << s_titleLine << endl;
    if ( m_showDividers )
        content << s_DividerLine << endl;
    content << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }
    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowTableLine( const ut::UnitTest * test )
{
    assert( nullptr != this );

    if ( !DoesOutput() )
        return false;

    const unsigned int index = test->GetIndex();
    const unsigned int failCount = test->GetFailCount();
    const unsigned int warnCount = test->GetWarnCount();
    const unsigned int passCount = test->GetPassCount();
    const unsigned int itemCount = test->GetItemCount();
    const unsigned int exceptCount = test->GetExceptionCount();
    const char * name = test->GetName();
    const char * result = "Passed";
    if ( ( 0 < exceptCount ) || ( 0 < failCount ) )
    {
        result = "FAILED";
    } else if ( 0 == itemCount )
    {
        result = "Empty!";
    }

    char buffer[ 1024 ];
    strstream content( buffer, sizeof(buffer) );
    content << result << "  "
        << setw(  3 ) << setfill( ' ' ) << right << index << "  "
        << setw( 24 ) << setfill( ' ' ) << left  << name << '\t'
        << setw(  6 ) << setfill( ' ' ) << right << passCount << '\t'
        << setw(  6 ) << setfill( ' ' ) << right << warnCount << '\t'
        << setw(  6 ) << setfill( ' ' ) << right << failCount << '\t'
        << setw(  6 ) << setfill( ' ' ) << right << exceptCount << '\t'
        << setw(  6 ) << setfill( ' ' ) << right << itemCount << endl << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowTotalLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount, unsigned int exceptCount,
    unsigned int itemCount )
{
    assert( nullptr != this );

    if ( !DoesOutput() )
        return false;

    char buffer[ 1024 ];
    const bool passed = ( ( 0 == failCount ) && ( 0 == exceptCount ) );
    const char * pResult = ( passed ) ? "Pass" : "FAIL";
    strstream content( buffer, sizeof(buffer) );
    if ( m_showDividers )
        content << s_DividerLine << endl;
    content
        << pResult << "	   Item Totals             \t"
        << setw( 6 ) << setfill( ' ' ) << right << passCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << warnCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << failCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << exceptCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << itemCount
        << endl << endl << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::ShowSummaryLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount, unsigned int exceptCount,
    unsigned int testCount )
{
    assert( nullptr != this );

    if ( !DoesOutput() )
        return false;

    char buffer[ 1024 ];
    const bool passed = ( ( 0 == failCount ) && ( 0 == exceptCount ) );
    const char * pResult = ( passed ) ? "Pass" : "FAIL";
    strstream content( buffer, sizeof(buffer) );
    if ( m_showDividers )
        content << s_DividerLine << endl;
    content
        << pResult << "	   Unit Test Totals        \t"
        << setw( 6 ) << setfill( ' ' ) << right << passCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << warnCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << failCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << exceptCount << '\t'
        << setw( 6 ) << setfill( ' ' ) << right << testCount
        << endl << endl << ends;

    if ( m_sendToCout )
    {
        cout << buffer << flush;
    }
    if ( m_sendToCerr )
    {
        cerr << buffer << flush;
    }
    if ( m_isOpen )
    {
        m_outFile << buffer << flush;
    }

    return true;
}

// ----------------------------------------------------------------------------

bool TextOutputter::EndSummaryTable( void )
{
    assert( nullptr != this );
    Close();
    return true;
}

// ----------------------------------------------------------------------------

HtmlOutputter::HtmlOutputter( const ut::UnitTestSetImpl * info,
    ::ut::UnitTestSet::OutputOptions options ) :
    FileOutputter(),
    UnitTestResultReceiver(),
    m_info( info ),
    m_startTime( nullptr ),
    m_showIndexes( 0 != ( options & ut::UnitTestSet::AddTestIndex ) ),
    m_htmlFileName(),
    m_mainPageName()
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

HtmlOutputter::~HtmlOutputter( void )
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::Start( const char * name )
{
    assert( nullptr != this );
    assert( !IsEmptyString( name ) );
    assert( !m_isOpen );

    Open( m_htmlFileName.c_str() );
    if ( !m_isOpen )
        return false;

    string filenameLink( "./");
    filenameLink += FindFileNameInPath( m_mainPageName.c_str() );
    m_outFile
        << "<html><head><title>Unit Test Results</title></head>" << endl
        << endl << "<body bgcolor=black text=white>" << endl
        << "<a href=\'" << filenameLink
        << "\'>Unit Test Main Page</a><br>" << endl;
    m_outFile << "Name of Unit Tests: " << name << "<br>" << endl;

    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowMessage( const ut::UnitTest * test,
    const char * fileName, unsigned int line, const char * message )
{
    assert( nullptr != this );
    if ( !m_isOpen )
        return false;
    m_outFile << fileName << '(' << line << ") : ";
    if ( m_showIndexes )
        m_outFile << test->GetIndex() << ' ';
    m_outFile << message << "<br>" << endl;
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowTestHeader( const ut::UnitTest * test )
{
    assert( nullptr != this );
    if ( !m_isOpen )
        return false;
    const unsigned int index = test->GetIndex();
    m_outFile
        << "<br>Test Number: " << index << "  " << test->GetName()
        << "<br>" << endl;
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowTestLine( const ut::UnitTest * test,
    ut::TestResult::EnumType result, const char * fileName, unsigned int line,
    const char * expression, const char * message )
{
    assert( nullptr != this );

    if ( !m_isOpen )
        return false;

    const char * status = ut::TestResult::GetName( result );
    const char * lineColor = GetColor( result );
    m_outFile << "<span style=\"color: " << lineColor << "\">"
        << fileName << '(' << line << ") : " << status << "    ";
    if ( m_showIndexes )
        m_outFile << test->GetIndex() << ':';
    m_outFile << test->GetItemCount() << "  (" << expression << ")";
    if ( !IsEmptyString( message ) )
        m_outFile << "  " << message;
    m_outFile << "</span><br>" << endl;

    if ( ut::TestResult::Fatal == result )
    {
        m_outFile
            << "<span style=\"color: red\">A required test failed!<br>" << endl
            << "Unable to safely execute further tests!<br>" << endl
            << "Program must exit now!</span><br>" << endl;
    }
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowTimeStamp( bool isStartTime, const char * timestamp )
{
    assert( nullptr != this );
    assert( !IsEmptyString( timestamp ) );

    if ( !m_isOpen )
        return false;
    const char * description =
        ( isStartTime ) ? s_beginTimeMessage : s_endTimeStampMessage;
    if ( isStartTime )
        m_outFile << "<br>" << endl;
    m_outFile << description << timestamp << "<br>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::StartSummaryTable( void )
{
    assert( nullptr != this );
    if ( !m_isOpen )
        return false;
    m_outFile << endl << "<br><table border=1 cellspacing=0 cellpadding=3>" << endl;
    m_outFile << "<caption><em>Unit Test Results</em></caption>" << endl;
    m_outFile << "<tr><th>Result</th><th>#</th><th>Unit Test Name</th>"
              << "<th>Passed</th><th>Warnings</th><th>Failed</th>"
              << "<th>Exceptions</th><th>Tested</th></tr>" << endl;
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowTableLine( const ut::UnitTest * test )
{
    assert( nullptr != this );

    if ( !m_isOpen )
        return false;

    const unsigned int index = test->GetIndex();
    const unsigned int failCount = test->GetFailCount();
    const unsigned int warnCount = test->GetWarnCount();
    const unsigned int passCount = test->GetPassCount();
    const unsigned int itemCount = test->GetItemCount();
    const unsigned int exceptCount = test->GetExceptionCount();
    HtmlResultColors colors( itemCount, passCount, warnCount,
        failCount, exceptCount);

    m_outFile
        << "<tr><td><span style=\"color: " << colors.testColor << "\">" << colors.testResult << "</span></td>"
        << "<td>" << index << "</td>"
        << "<td>" << test->GetName() << "</td>"
        << "<th><span style=\"color: " << colors.passColor << "\">" << passCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.warnColor << "\">" << warnCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.failColor << "\">" << failCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.tossColor << "\">" << exceptCount << "</span></th>"
        << "<th>" << itemCount << "</th></tr>" << endl;
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowTotalLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount, unsigned int exceptCount,
    unsigned int itemCount )
{
    assert( nullptr != this );

    if ( !m_isOpen )
        return false;

    HtmlResultColors colors( itemCount, passCount, warnCount,
        failCount, exceptCount );
    m_outFile
        << "<tr><th><span style=\"color: " << colors.testColor << "\">"
        << colors.testResult << "</span></th><th>-</th><th>Item Totals</th>"
        << "<th><span style=\"color: " << colors.passColor << "\">" << passCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.warnColor << "\">" << warnCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.failColor << "\">" << failCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.tossColor << "\">" << exceptCount << "</span></th>"
        << "<th>" << itemCount << "</th></tr>" << endl;
    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::ShowSummaryLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount, unsigned int exceptCount,
    unsigned int testCount )
{
    assert( nullptr != this );

    if ( !m_isOpen )
        return false;

    HtmlResultColors colors( testCount, passCount, warnCount,
        failCount, exceptCount );
    m_outFile
        << "<tr><th><span style=\"color: " << colors.testColor << "\">" << colors.testResult 
        << "</span></th><th>-</th><th>Unit Test Results</th>"
        << "<th><span style=\"color: " << colors.passColor << "\">" << passCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.warnColor << "\">" << warnCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.failColor << "\">" << failCount   << "</span></th>"
        << "<th><span style=\"color: " << colors.tossColor << "\">" << exceptCount << "</span></th>"
        << "<th>" << testCount << "</th></tr>" << endl << "</table><br>" << endl;

    return true;
}

// ----------------------------------------------------------------------------

bool HtmlOutputter::EndSummaryTable( void )
{
    assert( nullptr != this );

    if ( !m_isOpen )
        return false;

    string contents;
    const bool read_file = ReadFileIntoString( m_mainPageName.c_str(),
        contents );
    MainPageMaker mainPage( m_info, m_startTime,
        m_mainPageName.c_str(), m_htmlFileName.c_str() );
    const bool okay = read_file ?
        mainPage.AddToContent( contents ) : mainPage.CreateFile();
    assert( okay );
    (void)okay;

    MakeColorTable( m_outFile );
    m_outFile << "</body>" << endl << "</html>" << endl;
    Close();

    return true;
}

// ----------------------------------------------------------------------------

XmlOutputter::XmlOutputter( ::ut::UnitTestSet::OutputOptions options ) :
    FileOutputter(),
    UnitTestResultReceiver(),
    m_showIndexes( 0 != ( options & ut::UnitTestSet::AddTestIndex ) ),
    m_filename()
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

XmlOutputter::~XmlOutputter( void )
{
    assert( nullptr != this );
}

// ----------------------------------------------------------------------------

bool XmlOutputter::Start( const char * name )
{
    assert( nullptr != this );
    assert( !IsEmptyString( name ) );
    assert( !m_isOpen );

    Open( m_filename.c_str() );
    if ( !m_isOpen )
        return false;
    m_outFile
        << "<UnitTestResults" << endl
        << "\tname=\"" << name << "\">" << endl
        << "\t<Tests>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowMessage( const ut::UnitTest * test,
    const char * fileName, unsigned int line, const char * message )
{
    assert( nullptr != this );
    assert( test != nullptr );
    assert( !IsEmptyString( fileName ) );
    assert( !IsEmptyString( message ) );
    if ( !m_isOpen )
        return false;

    m_outFile
        << "\t\t<Message" << endl
        << "\t\t\tfile=\"" << fileName << '\"' << endl
        << "\t\t\tline=\"" << line << '\"' << endl
        << "\t\t\tunit=\"" << test->GetIndex() << "\">" << endl
        << "\t\t\t<message>\"" << message << "\"</message>" << endl
        << "\t\t</Message>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowTestHeader( const ut::UnitTest * test )
{
    assert( nullptr != this );
    assert( test != nullptr );
    if ( !m_isOpen )
        return false;

    m_outFile
        << "\t\t<Unit" << endl
        << "\t\t\tname=\"" << test->GetName() << '\"' << endl
        << "\t\t\tunit=\"" << test->GetIndex() << "\">" << endl
        << "\t\t</Unit>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowTestLine( const ut::UnitTest * test,
    ut::TestResult::EnumType result, const char * fileName,
    unsigned int line, const char * expression, const char * message )
{
    assert( nullptr != this );
    assert( test != nullptr );
    assert( !IsEmptyString( fileName ) );
    assert( !IsEmptyString( expression ) );
    if ( !m_isOpen )
        return false;

    const char * resultName = ut::TestResult::GetName( result );
    m_outFile
        << "\t\t<Test" << endl
        << "\t\t\tfile=\"" << fileName << '\"' << endl
        << "\t\t\tline=\"" << line << '\"' << endl
        << "\t\t\tresult=\"" << resultName << '\"' << endl
        << "\t\t\tunit=\"" << test->GetIndex() << '\"' << endl
        << "\t\t\tindex=\"" << test->GetItemCount() << "\">" << endl
        << "\t\t\t<expression>\"" << expression << "\"</expression>" << endl;
    if ( !IsEmptyString( message ) )
        m_outFile << "\t\t\t<message>\"" << message << "\"</message>" << endl;
    if ( ut::TestResult::Fatal == result )
    {
        m_outFile
            << "\t\t\t<fatal>\"A required test failed!  "
            << "Unable to safely execute further tests!  "
            << "Program must exit now!\"</fatal>" << endl;
    }
     m_outFile << "\t\t</Test>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowTimeStamp( bool isStartTime, const char * timestamp )
{
    assert( nullptr != this );
    assert( !IsEmptyString( timestamp ) );
    if ( !m_isOpen )
        return false;

    const char * name = ( isStartTime ) ? "\t\tstart=\"" : "\t\tstop=\"";
    if ( isStartTime )
        m_outFile << "\t</Tests>" << endl << "\t<TimeStamps" << endl;
    m_outFile << name << timestamp << '\"';
    if ( !isStartTime )
        m_outFile << '>' << endl << "\t</TimeStamps>";
    m_outFile << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::StartSummaryTable( void )
{
    assert( nullptr != this );
    return m_isOpen;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowTableLine( const ut::UnitTest * test )
{
    assert( nullptr != this );
    assert( test != nullptr );
    if ( !m_isOpen )
        return false;

    const bool passed = test->DidPass();
    const char * result = ( passed ) ? "Passed" : "FAILED";
    if ( test->GetIndex() == 1 )
        m_outFile << "\t<SummaryTable>" << endl;
    m_outFile
        << "\t\t<UnitTest" << endl
        << "\t\t\tname=\"" << test->GetName() << '\"' << endl
        << "\t\t\tindex=\"" << test->GetIndex() << '\"' << endl
        << "\t\t\tresult=\"" << result << '\"' << endl
        << "\t\t\tpassed=\"" << test->GetPassCount() << '\"' << endl
        << "\t\t\twarnings=\"" << test->GetWarnCount() << '\"' << endl
        << "\t\t\tfailed=\"" << test->GetFailCount() << '\"' << endl
        << "\t\t\texceptions=\"" << test->GetExceptionCount() << '\"' << endl
        << "\t\t\ttested=\"" << test->GetItemCount() << "\">" << endl
        << "\t\t</UnitTest>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowTotalLine( unsigned int passCount, unsigned int warnCount,
    unsigned int failCount, unsigned int exceptCount,
    unsigned int itemCount )
{
    assert( nullptr != this );
    if ( !m_isOpen )
        return false;

    const bool passed = ( failCount == 0 ) && ( exceptCount == 0 );
    const char * result = ( passed ) ? "Passed" : "FAILED";
    m_outFile
        << "\t\t<ItemTotals" << endl
        << "\t\t\tresult=\"" << result << '\"' << endl
        << "\t\t\tpassed=\"" << passCount << '\"' << endl
        << "\t\t\twarnings=\"" << warnCount << '\"' << endl
        << "\t\t\tfailed=\"" << failCount << '\"' << endl
        << "\t\t\texceptions=\"" << exceptCount << '\"' << endl
        << "\t\t\ttested=\"" << itemCount << "\">" << endl
        << "\t\t</ItemTotals>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::ShowSummaryLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount,
    unsigned int exceptCount, unsigned int testCount )
{
    assert( nullptr != this );
    if ( !m_isOpen )
        return false;

    const bool passed = ( failCount == 0 ) && ( exceptCount == 0 );
    const char * result = ( passed ) ? "Passed" : "FAILED";
    m_outFile
        << "\t\t<TestTotals" << endl
        << "\t\t\tresult=\"" << result << '\"' << endl
        << "\t\t\tpassed=\"" << passCount << '\"' << endl
        << "\t\t\twarnings=\"" << warnCount << '\"' << endl
        << "\t\t\tfailed=\"" << failCount << '\"' << endl
        << "\t\t\texceptions=\"" << exceptCount << '\"' << endl
        << "\t\t\ttested=\"" << testCount << "\">" << endl
        << "\t\t</TestTotals>" << endl << flush;
    m_outFile << "\t</SummaryTable>" << endl << flush;

    return true;
}

// ----------------------------------------------------------------------------

bool XmlOutputter::EndSummaryTable( void )
{
    assert( nullptr != this );
    m_outFile << "</UnitTestResults>" << endl << flush;
    Close();
    return true;
}

// ----------------------------------------------------------------------------

}; // end anonymous namespace

namespace ut
{

// ----------------------------------------------------------------------------

UnitTest::UnitTest( const char * name ) :
    m_madeHeader( false ),
    m_name(),
    m_index( 0 ),
    m_itemCount( 0 ),
    m_failCount( 0 ),
    m_warnCount( 0 ),
    m_passCount( 0 ),
    m_exceptions( 0 )
{
    assert( nullptr != this );
    ::strncpy( m_name, name, MaxNameSize );
    m_name[ MaxNameSize ] = '\0';
    DEBUG_CODE( CheckInvariants() );
}

// ----------------------------------------------------------------------------

UnitTest::~UnitTest( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
}

// ----------------------------------------------------------------------------

bool UnitTest::DoTest( const char * filename, unsigned int line,
    TestLevel level, bool pass, const char * expression, const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestChecker guard( this ); (void)guard; );

    UnitTestSet & uts = UnitTestSet::GetIt();
    if ( uts.m_impl->IsUsingReceivers() )
        return pass;
    if ( IsEmptyString( filename ) || IsEmptyString( expression ) )
        return pass;

    const TestResult::EnumType result = Convert( level, pass );
    if ( 0 == m_itemCount )
        uts.m_impl->StartOutput();
    ++m_itemCount;
    bool showItem = true;
    switch ( result )
    {
        default:                  // fall into next case.
        case TestResult::Failed:  // fall into next case.
        case TestResult::Fatal:
            ++m_failCount;
            break;
        case TestResult::Thrown:
            assert(false);  // Why was result set to Thrown?
            break;
        case TestResult::Passed:
            ++m_passCount;
            showItem = uts.m_impl->ShowPasses();
            break;
        case TestResult::Warning:
            ++m_warnCount;
            showItem = uts.m_impl->ShowWarnings();
            break;
    }

    if ( showItem )
    {
        if ( !m_madeHeader )
        {
            uts.m_impl->OutputTestHeader( this );
            m_madeHeader = true;
        }
        uts.m_impl->OutputTestLine(
            this, result, filename, line, expression, message );
        if ( result == TestResult::Fatal )
        {
            uts.OutputSummary();
            ::exit( 1 );
        }
    }

    return pass;
}

// ----------------------------------------------------------------------------

void UnitTest::OnException( const char * filename, unsigned int line,
    TestLevel level, const char * expression, const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestChecker guard( this ); (void)guard; );

    UnitTestSet & uts = UnitTestSet::GetIt();
    if ( uts.m_impl->IsUsingReceivers() )
        return;
    if ( IsEmptyString( filename ) || IsEmptyString( expression ) )
        return;

    const TestResult::EnumType result = ( UnitTest::Require == level )
        ? TestResult::Fatal : TestResult::Thrown;
    if ( 0 == m_itemCount )
        uts.m_impl->StartOutput();
    ++m_itemCount;
    ++m_exceptions;
    if ( !m_madeHeader )
    {
        uts.m_impl->OutputTestHeader( this );
        m_madeHeader = true;
    }
    uts.m_impl->OutputTestLine(
        this, result, filename, line, expression, message );
    if ( result == TestResult::Fatal )
    {
        ::exit( 1 );
    }
}

// ----------------------------------------------------------------------------

void UnitTest::OutputMessage( const char * filename, unsigned int line,
    const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestChecker guard( this ); (void)guard; );

    UnitTestSet & uts = UnitTestSet::GetIt();
    if ( uts.m_impl->IsUsingReceivers() )
        return;
    if ( IsEmptyString( filename ) || IsEmptyString( message ) )
        return;
    uts.m_impl->OutputMessage( this, filename, line, message );
}

// ----------------------------------------------------------------------------

void UnitTest::CheckInvariants( void ) const
{
    assert( nullptr != this );
    assert( !IsEmptyString( GetName() ) );
    assert( ::strlen( GetName() ) <= ut::UnitTest::MaxNameSize );
    const unsigned int total =
        m_failCount + m_passCount + m_warnCount + m_exceptions;
    assert( m_itemCount == total );
    (void)total;
    if ( ( 0 < m_failCount ) || ( 0 < m_exceptions ) )
    {
        assert( m_madeHeader );
    }
}

// ----------------------------------------------------------------------------

UnitTestSetImpl::UnitTestSetImpl( const char * testName,
    const char * textFilePartialName, const char * htmlFilePartialName,
    const char * xmlFilePartialName, UnitTestSet::OutputOptions info ) :
    m_testName( testName ),
    m_textFilePartialName(),
    m_htmlFilePartialName(),
    m_xmlFilePartialName(),
    m_tests(),
    m_textOutput( info ),
    m_htmlOutput( this, info ),
    m_xmlOutput( info ),
    m_receivers(),
    m_showMessages( 0 != ( info & UnitTestSet::Messages ) ),
    m_showPasses( 0 != ( info & UnitTestSet::Passes ) ),
    m_showWarnings( 0 != ( info & UnitTestSet::Warnings ) ),
    m_showHeaders( 0 != ( info & UnitTestSet::Headers ) ),
    m_showTimeStamp( 0 != ( info & UnitTestSet::TimeStamp ) ),
    m_useFullDayName( 0 != ( info & UnitTestSet::FullDayName ) ),
    m_showFinalTable( 0 != ( info & UnitTestSet::SummaryTable ) ),
    m_didPageHeader( false ),
    m_usingReceivers( false ),
    m_didAnyTest( false ),
    m_didFirstRun( false ),
    m_startTime(),
    m_testCount( 0 ),
    m_testPassCount( 0 ),
    m_testWarnCount( 0 ),
    m_testFailCount( 0 ),
    m_testExceptCount( 0 ),
    m_itemCount( 0 ),
    m_itemPassCount( 0 ),
    m_itemWarnCount( 0 ),
    m_itemFailCount( 0 ),
    m_itemExceptCount( 0 )
{
    assert( nullptr != this );

    if ( !IsEmptyString( xmlFilePartialName ) )
        m_xmlFilePartialName = xmlFilePartialName;
    if ( !IsEmptyString( textFilePartialName ) )
        m_textFilePartialName = textFilePartialName;
    if ( !IsEmptyString( htmlFilePartialName ) )
    {
        m_htmlFilePartialName = htmlFilePartialName;
    }
    if ( !IsEmptyString( xmlFilePartialName ) )
    {
        m_xmlFilePartialName = xmlFilePartialName;
    }

    if ( !m_useFullDayName )
    {
        s_testTimeStampFormatSpec = "%a, %Y - %B - %d, %H : %M : %S";
        s_timeStampFormatSpec     = "%a, %Y %b %d at %H:%M:%S in %z";
    }

    DEBUG_CODE( CheckInvariants() );
}

// ----------------------------------------------------------------------------

UnitTestSetImpl::~UnitTestSetImpl( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    Clear();

    assert( !m_usingReceivers );
    m_usingReceivers = true;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            receiver->FinalEnd();
            *it = nullptr;
        }
        catch ( ... ) { }
    }
    assert( m_usingReceivers );
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::Clear( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    TUnitTestChildrenIter last( m_tests.end() );
    for ( TUnitTestChildrenIter it( m_tests.begin() ); it != last; ++it )
    {
        UnitTest * pTest = *it;
        if ( nullptr != pTest )
            delete pTest;
    }
    m_tests.clear();
    m_didAnyTest = false;
    m_didPageHeader = false;
    m_testCount = 0;
    m_testPassCount = 0;
    m_testWarnCount = 0;
    m_testFailCount = 0;
    m_testExceptCount = 0;
    m_itemCount = 0;
    m_itemPassCount = 0;
    m_itemWarnCount = 0;
    m_itemFailCount = 0;
    m_itemExceptCount = 0;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputTestLine( UnitTest * test,
    TestResult::EnumType result, const char * fileName, unsigned int line,
    const char * expression, const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    bool keep = false;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep = receiver->ShowTestLine( test, result, fileName, line,
                expression, message );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *it = nullptr;
    }
    assert( m_usingReceivers );
    m_usingReceivers = false;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputMessage( UnitTest * test, const char * fileName,
    unsigned int line, const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    if ( !m_showMessages )
    {
        return;
    }
    if ( !test->m_madeHeader )
    {
        OutputTestHeader( test );
        test->m_madeHeader = true;
    }

    bool keep = false;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep = receiver->ShowMessage( test, fileName, line, message );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *it = nullptr;
    }
    assert( m_usingReceivers );
    m_usingReceivers = false;
}

// ----------------------------------------------------------------------------

const UnitTest * UnitTestSetImpl::GetUnitTest( const char * unitTestName ) const
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    FChildNameFinder oFinder( unitTestName );
    TUnitTestChildrenCIter start( m_tests.begin() );
    TUnitTestChildrenCIter last( m_tests.end() );
    TUnitTestChildrenCIter cit( find_if( start, last, oFinder ) );
    if ( last == cit )
        return nullptr;
    return *cit;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::SetupInternalReceivers( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    ::memset( m_timeString, 0, sizeof(m_timeString) );
    time( &m_startTime );
    if ( m_showTimeStamp )
    {
        const struct tm * timeStamp = ::localtime( &m_startTime );
        ::strftime( m_timeString, sizeof(m_timeString)-1,
            s_timeStampFormatSpec, timeStamp );
    }

    const bool sendToText = ( m_textFilePartialName.size() != 0 );
    const bool sendToHtml = ( m_htmlFilePartialName.size() != 0 );
    const bool sendToXml  = ( m_xmlFilePartialName.size()  != 0 );

    if ( sendToText || sendToHtml || sendToXml )
    {
        string xmlFileName;
        string textFileName;
        string currentHtmlFileName;
        string mainHtmlFileName;
        MakeFileNames( &m_startTime, m_textFilePartialName.c_str(),
            m_htmlFilePartialName.c_str(), m_xmlFilePartialName.c_str(),
            textFileName, currentHtmlFileName, mainHtmlFileName, xmlFileName );

        if ( sendToText )
            m_textOutput.SetFileName( textFileName.c_str() );
        if ( sendToHtml )
        {
            m_htmlOutput.SetMainPageName( mainHtmlFileName.c_str() );
            m_htmlOutput.SetCurrentPageName( currentHtmlFileName.c_str() );
            m_htmlOutput.SetTimeStampInfo( &m_startTime );
            if ( !m_didFirstRun )
                m_receivers.push_back( &m_htmlOutput );
        }
        if ( sendToXml )
        {
            m_xmlOutput.SetFileName( xmlFileName.c_str() );
            if ( !m_didFirstRun )
                m_receivers.push_back( &m_xmlOutput );
        }
    }
    if ( ( m_textOutput.DoesSendToCout() || sendToText )
      && ( !m_didFirstRun ) )
        m_receivers.push_back( &m_textOutput );
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::StartOutput( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    if ( m_didPageHeader )
        return;

    assert( !m_usingReceivers );
    m_usingReceivers = true;
    m_didAnyTest = true;
    SetupInternalReceivers();

    bool keep = false;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep = receiver->Start( m_testName.c_str() );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *it = nullptr;
    }
    assert( m_usingReceivers );
    m_usingReceivers = false;
    m_didPageHeader = true;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputTestHeader( const UnitTest * test )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    if ( !m_showHeaders )
        return;
    m_didAnyTest = true;

    bool keep = false;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep = receiver->ShowTestHeader( test );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *it = nullptr;
    }
    assert( m_usingReceivers );
    m_usingReceivers = false;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::ShowTimeStamp( void )
{
    assert( nullptr != this );
    assert( m_usingReceivers );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    time_t timeNow;
    time( &timeNow );
    const tm * endTime = ::localtime( &timeNow );
    char buffer[ 160 ];
    ::strftime( buffer, sizeof(buffer)-1, s_timeStampFormatSpec, endTime );

    bool keep = false;
    TUnitTestReceiverSetIter end( m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_receivers.begin() ); it != end; ++it )
    {
        UnitTestResultReceiver * receiver = *it;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep  = receiver->ShowTimeStamp( true, m_timeString );
            keep &= receiver->ShowTimeStamp( false, buffer );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *it = nullptr;
    }
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::MakeTableRows( void )
{
    assert( nullptr != this );
    assert( m_showFinalTable );
    assert( m_usingReceivers );

    bool keep = false;
    const char * pResult = nullptr;
    UnitTestResultReceiver * receiver = nullptr;
    TUnitTestReceiverSetIter recvEnd( m_receivers.end() );
    TUnitTestReceiverSetIter recvIt;
    TUnitTestChildrenCIter last( m_tests.end() );

    for ( TUnitTestChildrenCIter it( m_tests.begin() ); it != last; ++it )
    {
        const UnitTest * pTest = *it;
        if ( nullptr == pTest )
            continue;

        ++m_testCount;
        const bool pass = pTest->DidPass();
        if ( pass )
            ++m_testPassCount;
        else
            ++m_testFailCount;
        if ( 0 < pTest->GetWarnCount() )
            ++m_testWarnCount;
        pResult = pass ? "Pass" : "FAIL";

        m_itemPassCount += pTest->GetPassCount();
        m_itemWarnCount += pTest->GetWarnCount();
        m_itemFailCount += pTest->GetFailCount();
        m_itemCount += pTest->GetItemCount();
        unsigned int exceptCount = pTest->GetExceptionCount();
        m_itemExceptCount += exceptCount;
        if ( exceptCount != 0 )
            ++m_testExceptCount;

        for ( recvIt = m_receivers.begin(); recvIt != recvEnd; ++recvIt )
        {
            receiver = *recvIt;
            if ( receiver == nullptr )
                continue;
            keep = true;
            try
            {
                keep = receiver->ShowTableLine( pTest );
            }
            catch ( ... )
            {
                keep = false;
            }
            if ( !keep )
                *recvIt = nullptr;
        }
    }
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputSummaryTable( void )
{
    assert( nullptr != this );
    assert( m_showFinalTable );
    assert( m_usingReceivers );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    const bool hasAnyTests = ( 0 < m_tests.size() );
    bool keep = false;
    UnitTestResultReceiver * receiver = nullptr;
    TUnitTestReceiverSetIter recvEnd( m_receivers.end() );
    TUnitTestReceiverSetIter recvIt;
    for ( recvIt = m_receivers.begin(); recvIt != recvEnd; ++recvIt )
    {
        receiver = *recvIt;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep = receiver->StartSummaryTable();
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *recvIt = nullptr;
    }

    if ( hasAnyTests )
    {
        MakeTableRows();
    }

    for ( recvIt = m_receivers.begin(); recvIt != recvEnd; ++recvIt )
    {
        receiver = *recvIt;
        if ( receiver == nullptr )
            continue;
        try
        {
            keep  = receiver->ShowTotalLine( m_itemPassCount,
                m_itemWarnCount, m_itemFailCount, m_itemExceptCount,
                m_itemCount );
            keep &= receiver->ShowSummaryLine( m_testPassCount,
                m_testWarnCount, m_testFailCount, m_testExceptCount,
                m_testCount );
            keep &= receiver->EndSummaryTable();
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            *recvIt = nullptr;
    }
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputSummaryInfo( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    if ( !m_didAnyTest )
    {
        StartOutput();
    }

    assert( !m_usingReceivers );
    m_usingReceivers = true;
    if ( m_showTimeStamp )
    {
        ShowTimeStamp();
    }
    if ( m_showFinalTable )
    {
        OutputSummaryTable();
    }

    Clear();
    assert( m_usingReceivers );
    m_usingReceivers = false;
    m_didFirstRun = true;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::CheckInvariants( void ) const
{
    assert( nullptr != this );
    assert( m_testName.size() != 0 );
    unsigned int total = m_itemPassCount + m_itemWarnCount
        + m_itemFailCount + m_itemExceptCount;
    (void)total;
    assert( m_itemCount == total );
    assert( m_testWarnCount <= m_testPassCount );
    assert( m_testExceptCount <= m_testFailCount );
    total = m_testPassCount + m_testFailCount;
    (void)total;
    assert( total == m_testCount );

    unsigned int index = 1;
    TUnitTestChildrenCIter last( m_tests.end() );
    for ( TUnitTestChildrenCIter it( m_tests.begin() ); it != last; ++it )
    {
        const UnitTest * pTest = *it;
        (void)pTest;
        assert( nullptr != pTest );
        assert( pTest->GetIndex() == index );
        ++index;
    }
}

// ----------------------------------------------------------------------------

UnitTestSet::ErrorState UnitTestSet::Create( const char * testName,
    const char * textFileName, const char * htmlFileName,
    const char * xmlFileName, UnitTestSet::OutputOptions info,
    bool deleteAtExitTime )
{
    if ( s_instance != nullptr )
    {
        return UnitTestSet::AlreadyExists;
    }

    if ( IsEmptyString( testName ) )
    {
        testName = "Unit Tests";
    }
    s_instance = new UnitTestSet( testName, textFileName, htmlFileName,
        xmlFileName, info );
    if ( s_instance == nullptr )
    {
        return UnitTestSet::CantCreate;
    }
    if ( deleteAtExitTime )
    {
        ::atexit( Destroy );
    }

    return UnitTestSet::Success;
}

// ----------------------------------------------------------------------------

void UnitTestSet::Destroy( void )
{
    if ( s_instance != nullptr )
    {
        assert( s_instance->m_impl != nullptr );
        s_instance->OutputSummary();
        delete s_instance;
        s_instance = nullptr;
    }
}

// ----------------------------------------------------------------------------

UnitTestSet::UnitTestSet( const char * testName, const char * textFileName,
    const char * htmlFileName, const char * xmlFileName,
    UnitTestSet::OutputOptions info ) :
    m_impl( nullptr )
{
    assert( nullptr != this );
    m_impl = new UnitTestSetImpl( testName, textFileName, htmlFileName,
        xmlFileName, info );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
}

// ----------------------------------------------------------------------------

UnitTestSet::~UnitTestSet( void )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    delete m_impl;
}

// ----------------------------------------------------------------------------

bool UnitTestSet::AddReceiver( UnitTestResultReceiver * receiver )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( m_impl ); (void)guard; );

    if ( nullptr == receiver )
        return false;
    if ( m_impl->m_usingReceivers )
        return false;

    assert( !m_impl->m_usingReceivers );
    m_impl->m_usingReceivers = true;
    bool hasReceiver = false;
    bool hasHole = false;
    TUnitTestReceiverSetIter hole;
    TUnitTestReceiverSetIter end( m_impl->m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_impl->m_receivers.begin() );
        it != end; ++it )
    {
        UnitTestResultReceiver * recv = *it;
        if ( recv == receiver )
        {
            hasReceiver = true;
            break;
        }
        if ( recv == nullptr )
            hole = it;
    }

    if ( !hasReceiver )
    {
        if ( hasHole )
            *hole = receiver;
        else
            m_impl->m_receivers.push_back( receiver );
    }
    assert( m_impl->m_usingReceivers );
    m_impl->m_usingReceivers = false;

    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestSet::RemoveReceiver( UnitTestResultReceiver * receiver )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( m_impl ); (void)guard; );

    if ( nullptr == receiver )
        return false;
    if ( m_impl->m_usingReceivers )
        return false;

    bool found = false;
    assert( !m_impl->m_usingReceivers );
    m_impl->m_usingReceivers = true;
    TUnitTestReceiverSetIter end( m_impl->m_receivers.end() );
    for ( TUnitTestReceiverSetIter it( m_impl->m_receivers.begin() );
        it != end; ++it )
    {
        UnitTestResultReceiver * recv = *it;
        if ( recv != receiver )
            continue;
        *it = nullptr;
        found = true;
        break;
    }
    assert( m_impl->m_usingReceivers );
    m_impl->m_usingReceivers = false;

    return found;
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesSendToTextFile( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_textOutput.IsOpen();
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesSendToHtmlFile( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_htmlOutput.IsOpen();
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesSendToXmlFile( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_xmlOutput.IsOpen();
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesSendToCout( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_textOutput.DoesSendToCout();
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesSendToCerr( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_textOutput.DoesSendToCerr();
}

// ----------------------------------------------------------------------------

const char * UnitTestSet::GetTextFileName( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_textOutput.GetFileName();
}

// ----------------------------------------------------------------------------

const char * UnitTestSet::GetHtmlFileName( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_htmlOutput.GetCurrentPageName();
}

// ----------------------------------------------------------------------------

const char * UnitTestSet::GetMainHtmlFileName( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_htmlOutput.GetMainPageName();
}

// ----------------------------------------------------------------------------

const char * UnitTestSet::GetXmlFileName( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_xmlOutput.GetFileName();
}

// ----------------------------------------------------------------------------

void UnitTestSet::OutputSummary( void )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    const bool hasAnyTests = ( 0 < m_impl->m_tests.size() );
    if ( hasAnyTests || ( !m_impl->m_didFirstRun ) )
    {
        m_impl->OutputSummaryInfo();
    }
}

// ----------------------------------------------------------------------------

bool UnitTestSet::DoesOutputOption( UnitTestSet::OutputOptions options ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );

    if ( ( 0 != ( options & UnitTestSet::Passes ) )
      && ( !m_impl->m_showPasses ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::Warnings ) )
      && ( !m_impl->m_showWarnings ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::Messages ) )
      && ( !m_impl->m_showMessages ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::Headers ) )
      && ( !m_impl->m_showHeaders ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::TimeStamp ) )
      && ( !m_impl->m_showTimeStamp ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::FullDayName ) )
      && ( !m_impl->m_useFullDayName ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::Dividers ) )
      && ( !m_impl->m_textOutput.ShowDividers() ) )
    {
        return false;
    }
    if ( ( 0 != ( options & UnitTestSet::AddTestIndex ) )
        && ( !m_impl->m_textOutput.DoesShowIndexes() ) )
    {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------

unsigned int UnitTestSet::GetUnitTestCount( void ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return static_cast< unsigned int >( m_impl->m_tests.size() );
}

// ----------------------------------------------------------------------------

UnitTest * UnitTestSet::AddUnitTest( const char * unitTestName )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( m_impl ); (void)guard; );

    if ( IsEmptyString( unitTestName ) )
        return nullptr;
    UnitTest * test = const_cast< UnitTest * >
        ( m_impl->GetUnitTest( unitTestName ) );
    if ( nullptr != test )
        return test;

    try
    {
        test = new UnitTest( unitTestName );
        m_impl->m_tests.push_back( test );
        test->m_index = static_cast< unsigned int >( m_impl->m_tests.size() );
    }
    catch ( ... )
    {
        delete test;
        throw;
    }

    return test;
}

// ----------------------------------------------------------------------------

const UnitTest * UnitTestSet::GetUnitTest( const char * unitTestName ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    if ( IsEmptyString( unitTestName ) )
        return nullptr;
    return m_impl->GetUnitTest( unitTestName );
}

// ----------------------------------------------------------------------------

const UnitTest * UnitTestSet::GetUnitTest( unsigned int index ) const
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    if ( m_impl->m_tests.size() <= index )
    {
        return nullptr;
    }
    const UnitTest * test = m_impl->m_tests[ index ];
    return test;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::Start( const char * name )
{
    (void)name;
    return true;
}

// ----------------------------------------------------------------------------
bool UnitTestResultReceiver::ShowTimeStamp( bool isStartTime,
    const char * timestamp )
{
    (void)isStartTime;
    (void)timestamp;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowMessage( const ut::UnitTest * test,
    const char * fileName, unsigned int line, const char * message )
{
    (void)test;
    (void)fileName;
    (void)line;
    (void)message;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowTestHeader( const ut::UnitTest * test )
{
    (void)test;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowTestLine( const ut::UnitTest * test,
    TestResult::EnumType result, const char * fileName, unsigned int line,
    const char * expression, const char * message )
{
    (void)test;
    (void)result;
    (void)fileName;
    (void)line;
    (void)expression;
    (void)message;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::StartSummaryTable( void )
{
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowTableLine( const ut::UnitTest * test )
{
    (void)test;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowTotalLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount,
    unsigned int exceptCount, unsigned int itemCount )
{
    (void)passCount;
    (void)warnCount;
    (void)failCount;
    (void)exceptCount;
    (void)itemCount;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::ShowSummaryLine( unsigned int passCount,
    unsigned int warnCount, unsigned int failCount,
    unsigned int exceptCount, unsigned int testCount )
{
    (void)passCount;
    (void)warnCount;
    (void)failCount;
    (void)exceptCount;
    (void)testCount;
    return true;
}

// ----------------------------------------------------------------------------

bool UnitTestResultReceiver::EndSummaryTable( void )
{
    return true;
}

// ----------------------------------------------------------------------------

void UnitTestResultReceiver::FinalEnd( void )
{
}

// ----------------------------------------------------------------------------

}; // end namespace ut

// $Log: UnitTest.cpp,v $
// Revision 1.44  2008/03/22 21:36:39  rich_sposato
// Adding more files to unit test project.
//
// Revision 1.43  2007/07/19 20:02:28  rich_sposato
// Added output option to show summary table.  Added function to get UnitTest
// pointer by index instead of by name.  Moved code that makes summary table
// into a separate function.
//
// Revision 1.42  2007/07/18 00:46:20  rich_sposato
// Changed how test name gets passed into Start function.  Cleaned up some code.
//
// Revision 1.41  2007/07/17 23:55:43  rich_sposato
// Added ability to do unit tests safelt at exit time.
//
// Revision 1.40  2007/07/17 23:04:41  rich_sposato
// Added functions to get info about output to xml, cerr, and cout.
//
// Revision 1.39  2007/07/17 22:58:13  rich_sposato
// Moved sendToCerr & sendToCout into OutputOptions enum family.
//
// Revision 1.38  2007/07/17 21:51:33  rich_sposato
// Changed function names to better match their purposes.  Added function to
// notify receivers when singleton dies.
//
// Revision 1.37  2007/07/17 21:11:19  rich_sposato
// Added ability to send test results to xml output file.
//
// Revision 1.36  2007/06/07 21:51:54  rich_sposato
// Added output option to show unit test index in each result line.
//
// Revision 1.35  2007/06/07 20:44:12  rich_sposato
// To improve efficiency of UnitTestSetImpl::OutputTestHeader, I placed index
// inside UnitTest instead of calculating it each time.  Also removed index
// parameter from some functions calls since no longer needed.
//
// Revision 1.34  2007/06/07 01:37:45  rich_sposato
// Added output option to show dividing lines in text.
//
// Revision 1.33  2007/06/05 20:33:58  rich_sposato
// Changed output preparation to reduce number of calls to cout and cerr.
//
// Revision 1.32  2007/06/05 20:23:37  rich_sposato
// Added checking for buffer overruns in text output functions.
//
// Revision 1.31  2007/06/05 19:02:25  rich_sposato
// Improved how text output is prepared.
//
// Revision 1.30  2007/06/04 20:44:08  rich_sposato
// Added ability to send output to standard error.
//
// Revision 1.29  2007/06/04 19:11:00  rich_sposato
// Minor changes to code.
//
// Revision 1.28  2007/06/02 00:05:00  rich_sposato
// Replaced calls to _snprintf with strstream.
//
// Revision 1.27  2007/05/17 19:37:00  rich_sposato
// Minor changes.
//
// Revision 1.26  2007/05/17 18:14:55  rich_sposato
// Added output option so program can choose full weekday name, or just an
// abbreviated name.
//
// Revision 1.25  2007/05/17 17:27:16  rich_sposato
// Changed observer functions to no longer pure virtual, and added default
// function bodies so child classes do not have to implement all functions,
// just the ones they need.
//
// Revision 1.24  2007/05/01 19:02:48  rich_sposato
// Renamed variable to be more consistent.
//
// Revision 1.23  2007/04/26 00:01:05  rich_sposato
// Added functions to get names of html and text files.  Improved consistency
// of source code.
//
// Revision 1.22  2007/04/25 20:35:26  rich_sposato
// Fix for bug 1707690.  Now creates summary table even when no tests provided.
//
// Revision 1.21  2007/04/09 23:44:08  rich_sposato
// Made passing conditions consistent between text and html outputs.
//
// Revision 1.20  2007/04/04 00:14:27  rich_sposato
// Added more assertions to function that checks invariants.  Changed how
// simple messages are outputted.
//
// Revision 1.19  2007/04/03 23:13:39  rich_sposato
// Added assertions to check using-receiver flag.  Changed how code deals with
// timestamps and opening files.  Other minor changes.
//
// Revision 1.18  2007/04/03 18:54:54  rich_sposato
// Changed when text and html outputters open files.
//
// Revision 1.17  2007/04/03 17:27:32  rich_sposato
// Changed html output class so it updates the main page itself.
//
// Revision 1.16  2007/04/02 22:55:52  rich_sposato
// Added ability to send test result info to external observers in addition to
// text and html files.
//
// Revision 1.15  2007/03/15 00:44:50  rich_sposato
// Fixed bug in how warnings are outputted.  Changed how host program can ask
// which output options are applied.
//
// Revision 1.14  2007/03/14 21:15:44  rich_sposato
// Added documentation comments.
//
// Revision 1.13  2007/03/12 23:57:42  rich_sposato
// Minor coding style changes.
//
// Revision 1.12  2007/03/12 23:42:22  rich_sposato
// Changed preparation for links to files in HTML pages.
//
// Revision 1.11  2007/03/12 23:09:22  rich_sposato
// Consolidated code used to assign colors for tables in HTML output.
//
// Revision 1.10  2007/03/10 01:40:03  rich_sposato
// Added ability to say that some tests are required.
//
// Revision 1.9  2007/03/09 23:44:32  rich_sposato
// Added invariant checking.  Added ability to test warnings.
//
// Revision 1.8  2007/03/09 18:22:55  rich_sposato
// Added output option for messages.
//
// Revision 1.7  2007/03/09 01:53:47  rich_sposato
// Added begin and end timestamps to output.
//
// Revision 1.6  2007/03/08 22:04:44  rich_sposato
// Added ability to send messages directly to test results output.
//
// Revision 1.5  2007/03/08 18:33:34  rich_sposato
// Changed return type of Create function from bool to enum.
//
// Revision 1.4  2007/03/08 18:18:12  rich_sposato
// Converted UnitTestSet to a singleton.
//
// Revision 1.3  2007/03/08 01:55:52  rich_sposato
// Added ability to specify name of text file and html files.
//
// Revision 1.2  2007/03/07 22:41:08  rich_sposato
// Added exception handling.
//
// Revision 1.1.1.1  2005/07/22 21:44:40  rich_sposato
// Initial Import
//
