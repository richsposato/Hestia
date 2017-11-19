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

// $Header: /cvsroot/hestia/CppUnitTest/include/UnitTest.hpp,v 1.35 2008/03/22 21:36:38 rich_sposato Exp $


// ----------------------------------------------------------------------------

#ifndef _UNIT_TEST_H_INCLUDED_
#define _UNIT_TEST_H_INCLUDED_


// ----------------------------------------------------------------------------

#ifndef UNIT_TEST_WARN
    #define UNIT_TEST_WARN( u, test ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Warning, test, #test ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Warning, #test, 0 ); \
        }
#endif

#ifndef UNIT_TEST_WARN_MSG
    #define UNIT_TEST_WARN_MSG( u, test, msg ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Warning, test, #test, msg ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Warning, #test, msg ); \
        }
#endif

#ifndef UNIT_TEST
    #define UNIT_TEST( u, test ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Checked, test, #test ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Checked, #test, 0 ); \
        }
#endif

#ifndef UNIT_TEST_WITH_MSG
    #define UNIT_TEST_WITH_MSG( u, test, msg ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Checked, test, #test, msg ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Checked, #test, msg ); \
        }
#endif

#ifndef UNIT_TEST_REQUIRE
    #define UNIT_TEST_REQUIRE( u, test ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Require, test, #test ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Require, #test, 0 ); \
        }
#endif

#ifndef UNIT_TEST_REQUIRE_MSG
    #define UNIT_TEST_REQUIRE_MSG( u, test, msg ) \
        try { \
            u->DoTest( __FILE__, __LINE__, u->Require, test, #test, msg ); \
        } catch (...) { \
            u->OnException( __FILE__, __LINE__, u->Require, #test, msg ); \
        }
#endif

#ifndef UNIT_TEST_JUST_MSG
    #define UNIT_TEST_JUST_MSG( unit, msg ) \
        u->OutputMessage( __FILE__, __LINE__, msg );
#endif


namespace ut
{

// ----------------------------------------------------------------------------

class UnitTestSet;
class UnitTestSetImpl;

/** @class UnitTest
 @brief Maintains counts of test results for a specific unit test.

 The constructors and destructor of this class remain private to prevent host
 programs from creating or accidentally deleting UnitTest's.
 */
class UnitTest
{
public:

    /** Severity of unit test item if it fails.  If a test at either the
     Checked or Required level fails, a message is always placed into output.
     If a Warning level test fails, a message is placed into output only if
     that option is turned on via UnitTestSet::OutputOptions::Warnings.  If a
     required test fails, the program generates a special message, exits
     immediately, and completes output for UnitTestSet.
     */
    enum TestLevel
    {
        Warning = 0, ///< Does not cause entire UnitTest to fail.
        Checked = 1, ///< Causes UnitTest to fail, and further tests are done.
        Require = 2  ///< Causes UnitTest to fail, but no more tests are done.
    };

    inline const char * GetName( void ) const { return m_name; }
    inline unsigned int GetIndex( void ) const { return m_index; }

    inline unsigned int GetItemCount( void ) const { return m_itemCount; }
    inline unsigned int GetFailCount( void ) const { return m_failCount; }
    inline unsigned int GetPassCount( void ) const { return m_passCount; }
    inline unsigned int GetWarnCount( void ) const { return m_warnCount; }
    inline unsigned int GetExceptionCount( void ) const { return m_exceptions; }

    inline bool DidPass( void ) const
    { return ( 0 == m_failCount ) && ( 0 == m_exceptions ); }

    inline bool DoTest( const char * file, unsigned int line, TestLevel level,
        bool pass, const char * expression )
    { return DoTest( file, line, level, pass, expression, 0 ); }

    /** Called to tabulate result for each individual unit test item.
     @param file Name of source code file.
     @param line Source code line.
     @param level Severity level of test item.
     @param pass True if test item passed.
     @param expression Conditional expression that was evaluated.
     @param message Optional message placed into output.
     @return True if test item passed.
     */
    bool DoTest( const char * file, unsigned int line, TestLevel level,
        bool pass, const char * expression, const char * message );

    /** Called when a unit test item threw an exception.  The exception was
     caught, and this function was called instead of OnTest.  Exceptions are
     counted separately from failures, and also prevent a unit test from
     passing.
     @param file Name of source code file.
     @param line Source code line.
     @param level Severity level of test item.
     @param expression Conditional expression that threw an exception.
     @param message Optional message placed into output.
     */
    void OnException( const char * file, unsigned int line, TestLevel level,
        const char * expression, const char * message );

    /** Places message directly into test result output if UnitTestSet::Create
     function was called with UnitTestSet::OutputOptions::Messages option.
     @param file Name of source code file.
     @param line Source code line.
     @param message placed into output.
     */
    void OutputMessage( const char * file, unsigned int line,
        const char * message );

    /// Checks if this object fulfills all class invariants.
    void CheckInvariants( void ) const;

private:

    enum Constants
    {
        MaxNameSize = 23
    };

    friend class UnitTestSet;
    friend class UnitTestSetImpl;

    /// Not implemented.
    UnitTest( void );
    /// Not implemented.
    UnitTest( const UnitTest & that );
    /// Not implemented.
    UnitTest & operator = ( const UnitTest & that );

    /** Constructs a UnitTest object with given name.  This constructor should
      not throw any exceptions.
      @param unitTestName Name of test may not be NULL or empty string.
      */
    explicit UnitTest( const char * unitTestName );

    /** The destructor is private so host programs do not attempt to delete a
     UnitTest.  Only the UnitTestSet singleton constructs and destructs each
     UnitTest instance.
     */
    ~UnitTest( void );

    bool m_madeHeader;            ///< True if unit-test header was outputted.
    char m_name[ MaxNameSize+1 ]; ///< Unique name of unit test.
    unsigned int m_index;         ///< Index # of test within UnitTestSet.
    unsigned int m_itemCount;     ///< Total # of items tested.
    unsigned int m_failCount;     ///< # of failed items.
    unsigned int m_warnCount;     ///< # of failed warning items.
    unsigned int m_passCount;     ///< # of passed items.
    unsigned int m_exceptions;    ///< # of items that threw exceptions.
};

// ----------------------------------------------------------------------------

/** @class TestResult
 @brief Provides an enum family describing various test results and functions
  to work with the enum values.
 */
class TestResult
{
public:
    enum EnumType
    {
        Passed  = 0x01, ///< Test or warning expression passed.
        Warning = 0x02, ///< Warning failed.
        Failed  = 0x04, ///< A normal test failed.
        Fatal   = 0x08, ///< A required test failed, program must end.
        Thrown  = 0x10  ///< The test threw an exception.
    };

    /// Returns a simple string name of the test result.
    static const char * GetName (EnumType result);
};

// ----------------------------------------------------------------------------

/** @class UnitTestResultReceiver
 @brief Observer interface class for all output strategy classes.

 @par Purpose
  This class provides an observer interface for programs that want to receive
  event notices from the UnitTestSet.  Observers could display a message box to
  the user, start a debugger, assert, send an email, or display messages onto a
  window.

 @par Order of Calls
  UnitTestSet calls functions in the observer in a particular order.
  -# Start is called before all others so the observer can prepare itself.
  -# ShowTestHeader is called at most once for each UnitTest, and before calls
    to ShowMessage or ShowTestLine.  If this function is not called for a
    particular UnitTest, then that UnitTest never had to send a message or a
    test result to an observer.  The next three functions may get called any
    number of times and in any order between calls to ShowTestHeader.
    - ShowMessage
    - ShowTestLine
    .
  -# ShowTimeStamp is called twice if the host program requests timestamps.
    One call is for the test starting time, and the other is for the test
    stopping time.
  -# StartSummaryTable is called once all unit tests are done, and just before
    UnitTestSet makes the summary table.  This gets called in case observers
    want to do any cleanup once all the unit tests are done, or if they want
    to do any calculations or preparations before handling the summary table.
    All calls from this one through EndSummaryTable are done only if the host
    program requested a summary table via the OutputOptions.
  -# ShowTableLine is called once per UnitTest so the observer can prepare one
    line per UnitTest in the summary table.  If no UnitTests exist, then this
    never gets called.
  -# ShowTotalLine is called to provide totals for items in all UnitTest's.
  -# ShowSummaryLine is called to provide counts for the number of UnitTest's.
  -# EndSummaryTable is called after UnitTestSet has completed the summary
    table so the observers can clean up any resources.
  -# FinalEnd is called from the UnitTestSet's destructor to inform any
    remaining receivers not to call any functions in UnitTestSet.

 @par Behaviors
  An observer must return true when called if it wants to continue receiving
  event notifications.  If it ever returns false or throws an exception, then
  UnitTestSet will stop calling it.  UnitTestSet has internal observers in
  addition to any external one provided.  The internal observers are always
  called before any external one.  UnitTestSet will never attempt to delete an
  observer - even when UnitTestSet's destructor is called.  The default
  implementation of each function returns true and does nothing else.
 */
class UnitTestResultReceiver
{
public:

    /** Informs observer to prepare itself to receive test result notices.
     @param name Overall name for unit tests.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool Start( const char * name );

    /** Places a text message into output.  This call only occurs if the host
     program requests messages in the OutputOptions.
     @param test Pointer to UnitTest that needs header.
     @param fileName Name of source code file.
     @param line Line number in source file that generated message.
     @param message Text message for output.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowMessage( const ut::UnitTest * test,
        const char * fileName, unsigned int line, const char * message );

    /** Displays header lines for specific UnitTest.
     @param test Pointer to UnitTest that needs header.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowTestHeader( const ut::UnitTest * test );

    /** Shows result for specific test item.
     @param test Pointer to UnitTest that needs header.
     @param result Effect of test.
     @param fileName Name of source code file.
     @param line Line number in source file where test was done.
     @param expression Text of boolean expression that was evaluated as unit
      test item.
     @param message Optional text message for output.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowTestLine( const ut::UnitTest * test,
        TestResult::EnumType result, const char * fileName, unsigned int line,
        const char * expression, const char * message );

    /** Provides starting and stopping timestamps.  This call occurs after all
     tests are done so receivers can use this call as a signal they may now
     clean up after any tests.
     @param isStartTime True if timestamp says when testing started, false if
      timestamp says when testing stopped.
     @param timestamp String containing time and date.
     @return True if the observer wants to receive more event notices.  If a
      receiver only wants to get event notices for actual test results, and
      does not a summary table, it can return false from this function call.
     */
    virtual bool ShowTimeStamp( bool isStartTime, const char * timestamp );

    /** Called when ready to make summary table of unit test results, and if
     host program requested a summary table via the OutputOptions.  This call
     occurs after all tests are done so receivers can use this call as a signal
     they may now clean up after any tests as well as to prepare for the
     summary table.
     @return True if the observer wants to receive more event notices.  If a
      receiver only wants to get event notices for actual test results, and
      does not a summary table, it can return false from this function call.
     */
    virtual bool StartSummaryTable( void );

    /** Displays line within summary table for specific UnitTest.
     @param test Pointer to UnitTest that needs header.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowTableLine( const ut::UnitTest * test );

    /** Shows item counts in line in summary table.
     @param passCount Total # of items that passed in all UnitTest's.
     @param warnCount Total # of items that made warnings in all UnitTest's.
     @param failCount Total # of items that failed in all UnitTest's.
     @param exceptCount Total # of items throwing exceptions in all UnitTest's.
     @param itemCount Total # of all items in all UnitTest's.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowTotalLine( unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int exceptCount, unsigned int itemCount );

    /** Shows item counts in line in summary table.
     @param passCount Total # of UnitTest's that passed.
     @param warnCount Total # of UnitTest's with warnings.
     @param failCount Total # of UnitTest's that failed.
     @param exceptCount Total # of UnitTest's which tossed exceptions.
     @param testCount Total # of UnitTest's, should be same as number of calls
      to UnitTestSet::AddUnitTest.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool ShowSummaryLine( unsigned int passCount,
        unsigned int warnCount, unsigned int failCount,
        unsigned int exceptCount, unsigned int testCount );

    /** Called after the UnitTestSet has calculated all the summary data,
     and so observer can know no more calls will be made and it can do any
     cleanup necessary.
     @return True if the observer wants to receive more event notices.
     */
    virtual bool EndSummaryTable( void );

    /** Called from UnitTestSet's destructor so no further calls are made from
     any receiver or other parts of host program.  The host program should not
     call UnitTestSet::RemoveReceiver during or after a call of FinalEnd.  You
     don't need to implement FinalEnd if all receivers are removed from
     UnitTestSet before the program exits.
     */
    virtual void FinalEnd( void );

protected:

    /// Default constructor is trivial, inline, and empty.
    inline UnitTestResultReceiver( void ) {}

    /** Destructor is trivial, inline, and empty.  It does not need to be
     virtual since it is protected instead of public.
     */
    inline virtual ~UnitTestResultReceiver( void ) {}

private:
    /// Copy-constructor is not implemented.
    UnitTestResultReceiver( const UnitTestResultReceiver & );
    /// Copy-assignment operator is not implemented.
    UnitTestResultReceiver & operator = ( const UnitTestResultReceiver & );
};

// ----------------------------------------------------------------------------

/** @class UnitTestSet
 @brief Maintains collection of unit tests, and sends test results to output
  observers.

 @par Interaction With Host Program
 This singleton should be created soon after the host program begins execution.
 The host program does not need to call anything to destroy the singleton since
 it will destroy itself at exit time.  The singleton also sends test results to
 various forms of output (e.g. - text file, HTML files, and standard output) so
 the host program will not need to handle that separately.  The minimum a host
 program needs to do is call UnitTestSet::Create, call UnitTestSet::AddUnitTest
 occasionally and then exercise the UnitTest's.

 All implementation details are hidden from the host program using the Cheshire
 Cat technique (a.k.a. Pimpl Idiom).

 @par Exit Time
 The singleton typically registers its Destroy function with ::atexit so it can
 output a summary table and perform final cleanup at exit time.  By then, most
 programs complete their unit tests.  You can make a summary table before exit
 time by calling UnitTestSet::OutputSummary directly.  You can perform unit
 tests at exit time without worrying about whether the singleton remains alive
 by passing in false for deleteAtExitTime in the UnitTestSet::Create function.
 Once the host has completed all unit tests, call UnitTestSet::OutputSummary.
 If a required unit test fails, the unit-tester will create the summary table
 and end the program.

 @par Container of UnitTest's
 The singleton owns each UnitTest, and only it may create or destroy a UnitTest
 instance.  Each UnitTest has a unique name, and is identified by that name.

 @par Test Result Observers
 UnitTestSet maintains a container of pointers to test result observers.  At
 appropriate moments, it iterates through the container to call each observer.
 If an observer returns the function call with false, it will not be called by
 UnitTestSet again.  The host program may call AddReceiver and RemoveReceiver
 to specify if it will receive event notices.  UnitTestSet also maintains two
 internal observers, one which sends output to an HTML file, and another which
 sends output to both standard out and a text file.  UnitTestSet will never
 attempt to delete an external observer - even when UnitTestSet's destructor is
 called.

 @par Text Output
 This produces identical content for the text result file, the standard-output
 stream, and the standard-error stream.  The host program may choose to send
 output to any combination of those three (file, cout, cerr).  The cout option
 allows a person to exercise unit tests on the command line and see the results
 directly.  The cerr option exists in case programs want to reserve cout for
 other purposes.  The text file option exists in case programs use both cout
 and cerr.

 @par Output File Names
 The strings passed into the Create function should not contain complete file
 names.  The strings should contain just partial file names - a prefix onto
 which the rest of the name shall be added.  The strings may contain relative
 or absolute paths.  This singleton will embed a timestamp into the file name
 and add the appropriate extension.  That timestamp represents when the unit
 tests were executed.  For example, if the string contains "./MyCo_UnitTests"
 and if the unit tests began on 2007-March-14 at 1:59:26 AM, then the text file
 name is "./MyCo_UnitTests_2007_03_14_01_59_26.txt".

 @par HTML File Names
 The UnitTestSet produces two html files - a main page which contains a table
 of showing multiple unit test iterations, and a current page showing details
 of the current iteration.  The HTML file name parameter specifies a location
 and partial name for both the main and current html pages.  Each execution of
 unit tests produces a new HTML file for the current test results.  Each test
 result page has a link back to the main page, and contain detailed stats for
 all unit tests executed.  The main page has a table showing the history of
 prior tests so readers can quickly see long-term patterns of test results.
 Each row in that table has a link to the corresponding html file.

 @par Legal examples of partial file names for text or HTML files are:
 - "./MyCo_UnitTests"
 - "c:/test/results/current"
 - "//mars/olympus/zp/rev_5_2"
 - "../../elysium/mainline"
 */
class UnitTestSet
{
public:

    /// Type of error that occurred when calling Create function.
    enum ErrorState
    {
        Success = 0,   ///< No error occurred.
        AlreadyExists, ///< Singleton already created.
        CantCreate     ///< Could not create singleton.
    };

    /** This enum family defines output options which programs can use to
     specify what gets added to the test result output.  A bitfield can contain
     all the options so options can be bitwise ORed together.
     */
    enum OutputOptions
    {
        Nothing      = 0x0000, ///< Skip all of these options.
        Passes       = 0x0001, ///< Show all passing unit test items.
        Warnings     = 0x0002, ///< Show failed warnings in output.
        Headers      = 0x0004, ///< Add headers to unit tests.
        Messages     = 0x0008, ///< Send plain messages to output.
        TimeStamp    = 0x0010, ///< Place begin & end timestamps into output.
        FullDayName  = 0x0020, ///< Use full weekday names, not abbreviations.
        Dividers     = 0x0040, ///< Show divider lines in tables and sections.
        AddTestIndex = 0x0080, ///< Add unit test index to item lines.
        SummaryTable = 0x0100, ///< Send summary table to output.
        SendToCout   = 0x4000, ///< Send output to standard output.
        SendToCerr   = 0x8000, ///< Send output to standard error.
        Default      = 0x015E  ///< Bitflags for default output options.
    };

    /** Creates a singleton for executing sets of unit tests.
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
     @xmlFileName Path and part of filename used to store test results
      in XML form.  If neither NULL nor empty string, then test results are
      sent to an XML file.  This adds a timestamp into the filename and then
      appends ".xml" extension onto the filename.
     @param options A bitfield of all possible output options - each bit is a
      different option.
     @param deleteAtExitTime True if host program wants this to clean up
      itself at exit time, else false if singleton is not deleted ever.  If not
      deleted, then host program must call OutputSummary directly to create
      summary table.
     @return Error condition indicating result of attempt to create singleton.
     */
    static ErrorState Create( const char * testName, const char * textFileName,
        const char * htmlFileName, const char * xmlFileName,
        UnitTestSet::OutputOptions options, bool deleteAtExitTime );

    /// Returns true if singleton exists.
    inline static bool Exists( void ) { return ( s_instance != 0 ); }

    /// Returns reference to singleton.
    inline static UnitTestSet & GetIt( void )
    {
        return *s_instance;
    }

    /** This function either adds a new UnitTest to the set, or returns an
      existing one with the same name.  The function performs O(N) operations
      in the worst case scenario where N is the number of existing UnitTest's.
      If this can't allocate a new UnitTest or add the new UnitTest to a
      container, this may throw an exception.  This function provides strong
      exception safety in that internal data does not change if an exception
      occurs.
     @param unitTestName Name of new UnitTest.  Any name longer than the max
      allowed is truncated.  If the string is NULL or empty, this returns NULL.
     @return Pointer to new UnitTest, or pointer to existing one if it matches
      the name.
     */
    UnitTest * AddUnitTest( const char * unitTestName );

    /** Returns pointer to UnitTest that matches given name.  This function
     takes O(N) operations where N is the number of unit tests.  A NULL or
     empty name causes this to return NULL.  If no name matches the parameter,
     this returns NULL.
     */
    const UnitTest * GetUnitTest( const char * unitTestName ) const;
    inline UnitTest * GetUnitTest( const char * unitTestName )
    {
        return const_cast< UnitTest * >(
            const_cast< const UnitTestSet * >( this )
                ->GetUnitTest( unitTestName ) );
    }

    /** Returns pointer to UnitTest for a given index.  Index may range from
     zero to less than count of unit tests.  If the index is too high, this
     returns NULL.
     */
    const UnitTest * GetUnitTest( unsigned int index ) const;
    inline UnitTest * GetUnitTest( unsigned int index )
    {
        return const_cast< UnitTest * >(
            const_cast< const UnitTestSet * >( this )->GetUnitTest( index ) );
    }

    /** Tells the singleton to notify a receiver of test result and output
      events.  This function may throw exceptions, but will not leak resources
      if it throws.  Complexity is O(N) where N is the number of observers
      already added to the container.  This will not add the observer if
      UnitTestSet is already calling observers.
     @param receiver Pointer to observer of test result events.
     @return True if pointer was not NULL, and was added to container of
      receivers.  Also false if receivers are currently being called since the
      singleton will not add or remove receiver during those calls.
     */
    bool AddReceiver( UnitTestResultReceiver * receiver );

    /** Tells the singleton to remove a receiver from its container of
      receivers so it will no longer receiver notices of test result and
      output events.  Complexity is O(N) where N is the number of observers
      already added to the container.  This should not throw.  This will not
      remove the observer if UnitTestSet is already calling observers.
     @param receiver Pointer to observer of test result events.
     @return True if receiver is not NULL and is in container.  False if
      pointer is NULL or is not in container of receivers.  Also false if
      receivers are currently being called since the singleton will not add or
      remove receiver during those calls.
     */
    bool RemoveReceiver( UnitTestResultReceiver * receiver );

    /** Returns true if this sends test results to text file, or false if host
     program did not specify file name for text output, or could not open file.
     */
    bool DoesSendToTextFile( void ) const;

    /** Returns true if this sends test results to html files, or false if host
     program did not specify name for html file, or could not open either file.
     */
    bool DoesSendToHtmlFile( void ) const;

    /** Returns true if this sends test results to an xml file, or false if
     host program did not specify name for xml file, or could not open file.
     */
    bool DoesSendToXmlFile( void ) const;

    /// Returns true if this sends test results to standard output.
    bool DoesSendToCout( void ) const;

    /// Returns true if this sends test results to standard error.
    bool DoesSendToCerr( void ) const;

    /** This provides name of output text file, but returns NULL if this does
     not send output to text file, or could not open text file.
     */
    const char * GetTextFileName( void ) const;

    /** This provides name of output html file for current tests, but returns
     NULL if this does not send output to html file, or could not open file.
     */
    const char * GetHtmlFileName( void ) const;

    /** This provides name of main output html file, but returns NULL if this
     does not send output to html file, or could not open file.
     */
    const char * GetMainHtmlFileName( void ) const;

    /** This provides name of output xml file for current tests, but returns
     NULL if this does not send output to xml file, or could not open file.
     */
    const char * GetXmlFileName( void ) const;

    /// Returns true if this has the output option(s) specified.
    bool DoesOutputOption( UnitTestSet::OutputOptions options ) const;

    /** Creates a summary table of unit test results and sends table info to
     the receivers.  Once this makes the final summary table, it clears the
     contents of all unit tests so that the host program can either start over
     or exit.  If the main function exits and the host program has not called
     this function, then UnitTestSet will do the output at exit time.
     If this throws, no resources are lost.  Complexity is O(R * U) where R is
     the number of receivers and U is the number of unit tests.
     */
    void OutputSummary( void );

    unsigned int GetUnitTestCount( void ) const;

private:

    friend class UnitTest;

    /** This function sends test result summary tables to output and then
     destroys the singleton at exit time.
     */
    static void Destroy( void );

    UnitTestSet( const char * testName, const char * textFileName,
        const char * htmlFileName, const char * xmlFileName,
        UnitTestSet::OutputOptions info );

    ~UnitTestSet( void);

    /// Not implemented.
    UnitTestSet( void );
    /// Not implemented.
    UnitTestSet( const UnitTestSet & that );
    /// Not implemented.
    UnitTestSet & operator = ( const UnitTestSet & that );

    /// Pointer to one and only singleton.
    static UnitTestSet * s_instance;

    /// Pointer to implementation details.
    UnitTestSetImpl * m_impl;
};

// ----------------------------------------------------------------------------

} // end namespace ut

#endif // end file guardian

// $Log: UnitTest.hpp,v $
// Revision 1.35  2008/03/22 21:36:38  rich_sposato
// Adding more files to unit test project.
//
// Revision 1.34  2007/07/19 20:00:49  rich_sposato
// Added/improved documentation comments.  Added function to get UnitTest ptr
// by index instead of just by name.  Added output option to show final table.
//
// Revision 1.33  2007/07/18 00:46:20  rich_sposato
// Changed how test name gets passed into Start function.  Cleaned up some code.
//
// Revision 1.32  2007/07/17 23:55:43  rich_sposato
// Added ability to do unit tests safelt at exit time.
//
// Revision 1.31  2007/07/17 23:04:40  rich_sposato
// Added functions to get info about output to xml, cerr, and cout.
//
// Revision 1.30  2007/07/17 22:58:12  rich_sposato
// Moved sendToCerr & sendToCout into OutputOptions enum family.
//
// Revision 1.29  2007/07/17 21:51:32  rich_sposato
// Changed function names to better match their purposes.  Added function to
// notify receivers when singleton dies.
//
// Revision 1.28  2007/07/17 21:11:18  rich_sposato
// Added ability to send test results to xml output file.
//
// Revision 1.27  2007/06/07 21:51:54  rich_sposato
// Added output option to show unit test index in each result line.
//
// Revision 1.26  2007/06/07 20:44:12  rich_sposato
// To improve efficiency of UnitTestSetImpl::OutputTestHeader, I placed index
// inside UnitTest instead of calculating it each time.  Also removed index
// parameter from some functions calls since no longer needed.
//
// Revision 1.25  2007/06/07 17:16:41  rich_sposato
// Added default output option.
//
// Revision 1.24  2007/06/07 01:37:44  rich_sposato
// Added output option to show dividing lines in text.
//
// Revision 1.23  2007/06/04 20:44:07  rich_sposato
// Added ability to send output to standard error.
//
// Revision 1.22  2007/05/17 19:36:59  rich_sposato
// Minor changes.
//
// Revision 1.21  2007/05/17 18:14:55  rich_sposato
// Added output option so program can choose full weekday name, or just an
// abbreviated name.
//
// Revision 1.20  2007/05/17 17:27:16  rich_sposato
// Changed observer functions to no longer pure virtual, and added default
// function bodies so child classes do not have to implement all functions,
// just the ones they need.
//
// Revision 1.19  2007/04/26 00:01:28  rich_sposato
// Added functions to get names of html and text files.  Added even more
// documentation comments.
//
// Revision 1.17  2007/04/19 00:44:24  rich_sposato
// Added documentation comments explaining order of calls to observers.
//
// Revision 1.16  2007/04/03 23:03:35  rich_sposato
// Updated documentation comment.
//
// Revision 1.15  2007/04/03 17:27:56  rich_sposato
// Added more documentation comments.
//
// Revision 1.14  2007/04/02 22:55:52  rich_sposato
// Added ability to send test result info to external observers in addition to
// text and html files.
//
// Revision 1.13  2007/03/21 23:13:49  rich_sposato
// Changed documentation comment.
//
// Revision 1.12  2007/03/15 00:44:50  rich_sposato
// Fixed bug in how warnings are outputted.  Changed how host program can ask
// which output options are applied.
//
// Revision 1.11  2007/03/14 21:15:44  rich_sposato
// Added documentation comments.
//
// Revision 1.10  2007/03/10 01:40:02  rich_sposato
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
// Revision 1.2  2007/03/07 22:41:29  rich_sposato
// Added exception handling.
//
// Revision 1.1.1.1  2005/07/22 21:44:40  rich_sposato
// Initial Import
//
