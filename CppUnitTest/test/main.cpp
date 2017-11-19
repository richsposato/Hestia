// ----------------------------------------------------------------------------
// The C++ Unit Test Library
// Copyright (c) 2007 by Rich Sposato
//
// Permission to use, copy, modify, distribute and sell this software for any
// purpose is hereby granted under the terms stated in the GNU Library Public
// License, provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear in
// supporting documentation.
//
// ----------------------------------------------------------------------------

// $Header: /cvsroot/hestia/CppUnitTest/test/main.cpp,v 1.28 2007/07/19 19:59:45 rich_sposato Exp $


// ----------------------------------------------------------------------------

#include "UnitTest.hpp"

#include <assert.h>
#include <iostream>

#include "Thingy.hpp"

using namespace std;


// ----------------------------------------------------------------------------

void TestThingy1( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Test 1 Thingy");

    Thingy justOne( 1 );

    // Passing tests.
    UNIT_TEST( u, !justOne.IsZero() );
    UNIT_TEST_WITH_MSG( u, !justOne.IsZero(), "This is not zero." );

    UNIT_TEST_JUST_MSG( u,
        "You can output any message directly into the test results!" );

    // Warning tests.
    UNIT_TEST_WARN( u, justOne.IsZero() );
    UNIT_TEST_WARN( u, !justOne.IsZero() );
    UNIT_TEST_WARN_MSG( u, justOne.IsZero(),  "This is just a warning." );
    UNIT_TEST_WARN_MSG( u, !justOne.IsZero(), "This is just a warning." );

    // Failing tests.
    UNIT_TEST( u, justOne.IsZero() );
    UNIT_TEST_WITH_MSG( u, justOne.IsZero(), "This is not zero." );

    /** @par Using the tested expression - part 1.
     If you need to store the value of the tested expression, then you can
     call UnitTest::DoTest function directly instead of calling it through one
     of UNIT_TEST macros.  Just assign UnitTest::DoTest's return value to a
     boolean variable.  You can then use that variable later.
     */
    bool passed = u->DoTest( __FILE__, __LINE__, u->Checked,
        justOne.IsZero(), "justOne.IsZero()" );
    if ( !passed )
    {
        UNIT_TEST_JUST_MSG( u, "Call to justOne.IsZero() failed!" );
    }

    /** @par Using the tested expression - part 2.
     Another way to store the test result is to perform the test first, assign
     the result to a boolean variable. You can then call UNIT_TEST_WITH_MSG
     with the boolean variable.  I recommend calling UNIT_TEST_WITH_MSG instead
     of just UNIT_TEST so the intent of the test is clear within the output.
     */
    passed = justOne.IsZero();
    UNIT_TEST_WITH_MSG( u, passed, "Call to justOne.IsZero() failed!" );

    UNIT_TEST( u, justOne.ThrowsBadly( false ) );
    UNIT_TEST( u, justOne.ThrowsBadly( true ) );
    UNIT_TEST_WITH_MSG( u, justOne.ThrowsBadly( false ), "This should not throw!" );
    UNIT_TEST_WITH_MSG( u, justOne.ThrowsBadly( true ), "This can throw!" );
}

// ----------------------------------------------------------------------------

#ifndef UNIT_TEST_GOOD_THROW
#define UNIT_TEST_GOOD_THROW( u, test ) \
    try { \
        u->DoTest( __FILE__, __LINE__, u->Checked, (test) && false, #test ); \
    } catch ( const std::exception & ex ) { \
        u->DoTest( __FILE__, __LINE__, u->Checked, true, #test, ex.what() ); \
    } catch (...) { \
        u->OnException( __FILE__, __LINE__, u->Checked, #test, 0 ); \
    }
#endif

#ifndef UNIT_TEST_THROW_STUFF
#define UNIT_TEST_THROW_STUFF( u, test ) \
    try { \
        u->DoTest( __FILE__, __LINE__, u->Checked, (test) && false, #test ); \
    } catch ( const Stuff & e1 ) { \
        u->DoTest( __FILE__, __LINE__, u->Checked, true, #test, e1.what() ); \
    } catch ( const std::exception & e2 ) { \
        u->OnException( __FILE__, __LINE__, u->Checked, #test, e2.what() ); \
    } catch ( ... ) { \
        u->OnException( __FILE__, __LINE__, u->Checked, #test, 0 ); \
    }
#endif

// ----------------------------------------------------------------------------

void ExceptionTest( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Exception Test");
    Thingy empty;
    UNIT_TEST( u, empty.ThrowsBadly( false ) );
    UNIT_TEST( u, empty.ThrowsBadly( true ) );

    /** @par Question: What If Throwing Is Good?
     What if I want a particular test to throw an exception?  In this case,
     throwing a particular exception is correct behavior, and not throwing one
     is wrong behavior.  Also, throwing a different type of exception is wrong.

     @par Answer: How To Catch Passing Throws.
     These steps demostrate how to setup unit tests that pass when throwing
     expected exceptions.  These steps show how to make a macro similar to
     UNIT_TEST_GOOD_THROW shown above.
     -# Copy the macro and give it a unique name.
     -# Put the expected exception type within the first catch block - perhaps
        a name that reflects the type of exception the test will throw.
     -# Make sure the first call to unit->DoTest always fails by using the
        "(test) && false" expression.  That way if the test returns without
        throwing an exception it will automatically fail.
     -# Set the test parameter inside the call to DoTest as true to indicate
        that the test passes.
     -# Optionally add a message to the unit->DoTest function by calling the
        what() function in the caught exception.
     -# Optionally add further catch blocks for each type of exception the test
        may throw.  If an additional exception indicates a passing test, call
        unit->DoTest as described above.  If an additional exception indicates
        a failing test, call unit->DoException so it gets counted as an
        unexpected exception rather than a mere failing test.
     -# Then add the catch-all block last.
     */

    // This test does not throw and that is now considered a failing test.
    UNIT_TEST_GOOD_THROW( u, empty.ThrowsBadly( false ) );
    // When this test throws a std::exception, the result is a pass.
    UNIT_TEST_GOOD_THROW( u, empty.ThrowsBadly( true ) );

    // These tests don't throw and that is now considered a failing test.
    UNIT_TEST_THROW_STUFF( u, empty.ThrowsBadly( false, false ) );
    UNIT_TEST_THROW_STUFF( u, empty.ThrowsBadly( false, true  ) );

    // This test throws, but throws the wrong type of exception.
    UNIT_TEST_THROW_STUFF( u, empty.ThrowsBadly( true,  false ) );
    // This ones throws the correct type of exception, so it passes.
    UNIT_TEST_THROW_STUFF( u, empty.ThrowsBadly( true,  true  ) );
}

// ----------------------------------------------------------------------------

void TestThingy2( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Test 2 Thingys");
    Thingy empty;
    Thingy negative( -1 );
    Thingy positive( 1 );

    // Passing tests.
    UNIT_TEST( u, empty == empty );
    UNIT_TEST( u, empty != negative );
    UNIT_TEST( u, empty != positive );
    UNIT_TEST( u, negative == negative );
    UNIT_TEST( u, negative != positive );
    UNIT_TEST( u, negative != empty );
    UNIT_TEST( u, positive == positive );
    UNIT_TEST( u, positive != negative );
    UNIT_TEST( u, positive != empty );

    // Failing tests.
    UNIT_TEST( u, empty != empty );
    UNIT_TEST( u, empty == negative );
    UNIT_TEST( u, empty == positive );
    UNIT_TEST( u, negative != negative );
    UNIT_TEST( u, negative == positive );
    UNIT_TEST( u, positive != positive );
    UNIT_TEST( u, positive == empty );
}

// ----------------------------------------------------------------------------

void PassingThingyTest( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Passing Test");
    Thingy empty;
    Thingy negative( -1 );

    // Passing tests.
    UNIT_TEST( u, empty == empty );
    UNIT_TEST( u, empty != negative );
    UNIT_TEST( u, negative == negative );
    UNIT_TEST( u, negative != empty );
}

// ----------------------------------------------------------------------------

void PassWithWarningsThingyTest( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Just Warnings Test");
    Thingy empty;
    Thingy negative( -1 );
    Thingy positive( 1 );

    // Passing tests.
    UNIT_TEST( u, empty == empty );
    UNIT_TEST( u, empty != negative );
    UNIT_TEST( u, empty != positive );
    UNIT_TEST( u, negative == negative );
    UNIT_TEST( u, negative != positive );
    UNIT_TEST( u, negative != empty );
    UNIT_TEST( u, positive == positive );
    UNIT_TEST( u, positive != negative );
    UNIT_TEST( u, positive != empty );

    // Warning tests.
    UNIT_TEST_WARN( u, empty != empty );
    UNIT_TEST_WARN( u, empty == negative );
    UNIT_TEST_WARN( u, empty == positive );
}

// ----------------------------------------------------------------------------

void EmptyThingyTest( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Empty Thingy Test");
    (void)u;
}

// ----------------------------------------------------------------------------

void FatalThingyTest( void )
{
    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    ut::UnitTest * u = uts.AddUnitTest ("Fatal Thingy Test");
    Thingy empty;

    UNIT_TEST_REQUIRE( u, empty != empty );
    // If this shows up in output, then program did not exit on previous line.
    UNIT_TEST_WITH_MSG( u, false, "Why is this program still running?" );
}

// ----------------------------------------------------------------------------

/** @class UnitTestAsserter
 @brief An example observer class which implements the UnitTestResultReceiver
  interface.

 @par Example
  This class demonstrates how programs can receive event notices from the
  UnitTestSet.  This particular example merely asserts when a test fails.
  Other observers could display a message box to the user, start a debugger,
  send an email, or display messages onto a window.

 @par Notes
  An observer must return true when called if it wants to continue receiving
  event notifications.  If it ever returns false or throws an exception, then
  UnitTestSet will stop calling it.
 */
class UnitTestAsserter : public ::ut::UnitTestResultReceiver
{
    virtual bool Start( const char * )
    {
        // These lines just check if UnitTestSet singleton will not allow host
        // program to add or remove a receiver while it notifies receivers of
        // events.
        ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
        bool okay = uts.AddReceiver( this );
        assert( !okay );
        okay = uts.RemoveReceiver( this );
        assert( !okay );
        return true;
    }

    virtual bool ShowLine( const char * )
    {
        // These lines demonstrate that host programs may not exercise unit
        // tests when the UnitTestSet singleton is calling observers.
        ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
        ut::UnitTest * u = uts.AddUnitTest ("Test 1 Thingy");
        Thingy justOne( 1 );
        UNIT_TEST( u, !justOne.IsZero() );
        UNIT_TEST_JUST_MSG( u,
            "You may not output messages when it is calling receivers!" );
        return true;
    }

    virtual bool ShowTestLine( const ut::UnitTest * test,
        ::ut::TestResult::EnumType result, const char * fileName,
        unsigned int line, const char * expression, const char * message )
    {
        (void)test;
        (void)fileName;
        (void)line;
        (void)expression;
        (void)message;
        if ( ( result != ut::TestResult::Passed )
          && ( result != ut::TestResult::Warning ) )
        {
            assert( false );
        }
        return true;
    }

};

// MainArgs -------------------------------------------------------------------

/** @class MainArgs
 @brief Parses command line arguements, determines if they are valid, and then
  makes the information encoded within the arguements available.
 */
class MainArgs
{
public:

    /** Parses through and validates the command line parameters.
     @param[in] argc Count of parameters.
     @param[in] argv Array of parameters.
     */
    MainArgs( unsigned int argc, const char * const argv[] );

    inline ~MainArgs( void ) {}

    void ShowHelp( void ) const;

    inline bool IsValid( void ) const { return m_valid; }

    inline bool DoShowHelp( void ) const { return m_doShowHelp; }

    inline bool DoOnlyPassingTest( void ) const
    { return m_doOnlyPassingTests; }

    inline bool DoNoTests( void ) const { return m_doNoTests; }

    inline bool DoFatalTest( void ) const { return m_doFatalTest; }

    inline bool DoAssertOnFail( void ) const { return m_doAssertOnFail; }

    inline bool DoRepeatTests( void ) const { return m_doRepeatTests; }

    inline bool DeleteAtExitTime( void ) const { return m_deleteAtExitTime; }

    inline bool DoMakeTableAtExitTime( void ) const
    { return m_tableAtExitTime; }

    ut::UnitTestSet::OutputOptions GetOutputOptions( void ) const
    {
        return static_cast< ut::UnitTestSet::OutputOptions >
            ( m_outputOptions );
    }

    inline const char * GetHtmlFileName( void ) const
    { return m_htmlFileName; }

    inline const char * GetTextFileName( void ) const
    { return m_textFileName; }

    inline const char * GetXmlFileName( void ) const
    { return m_xmlFileName; }

    inline const char * GetExeName( void ) const { return m_exeName; }

private:

    MainArgs( void );
    MainArgs( const MainArgs & );
    MainArgs & operator = ( const MainArgs & );

    bool ParseOutputOptions( const char * ss );

    bool m_valid;        ///< True if all command line parameters are valid.
    bool m_doShowHelp;
    bool m_doFatalTest;
    bool m_doNoTests;
    bool m_doOnlyPassingTests;
    bool m_doAssertOnFail;
    bool m_doRepeatTests;
    bool m_tableAtExitTime;
    bool m_deleteAtExitTime;
    unsigned int m_outputOptions;
    const char * m_exeName;
    const char * m_xmlFileName;
    const char * m_htmlFileName;
    const char * m_textFileName;
};

// ----------------------------------------------------------------------------

void MainArgs::ShowHelp( void ) const
{
    cout << "Usage: " << m_exeName << endl;
    cout << " [-f] [-p] [-z] [-a] [-o:[ndhmptw]]" << endl;
    cout << " [-s] [-t:file] [-h:file] [-x:file] [-?] [--help]" << endl;
    cout << endl;
    cout << "Parameters: (order of parameters does not matter)" << endl;
    cout << "  -f  Do fatal tests.  Causes program to end abruptly." << endl;
    cout << "      Incompatible with -p and -z." << endl;
    cout << "  -p  Do only passing tests." << endl;
    cout << "      Incompatible with -f and -z." << endl;
    cout << "  -z  Do not do any tests - just make summary table." << endl;
    cout << "      Incompatible with -f, -p, and -r." << endl;
    cout << "  -a  Assert when test fails." << endl;
    cout << "  -L  Let singleton live at exit time, do not delete it." << endl;    
    cout << "  -o  Set output options." << endl;
    cout << "      n  No extra output options." << endl;
    cout << "         This is incompatible with any other output option."
         << endl;
    cout << "      f  Use full weekday names, not abbreviations." << endl;
    cout << "      h  Show headers for each unit test that fails." << endl;
    cout << "      i  Show test index in each unit test output line." << endl;
    cout << "      m  Show messages even if no test with message." << endl;
    cout << "      p  Show contents of passing tests." << endl;
    cout << "      t  Show beginning and ending timestamps." << endl;
    cout << "      T  Show summary table once tests are done." << endl;
    cout << "      w  Show failing warnings." << endl;
    cout << "      d  Show divider lines in table and sections." << endl;
    cout << "      D  Show default output options." << endl;
    cout << "         You may combine this with other options." << endl;
    cout << "         Default output option is same as -o:hmtTw." << endl;
    cout << "      E  Send test results to standard error." << endl;
    cout << "      S  Send test results to standard output." << endl;
    cout << "  -t  Send test results to text file." << endl;
    cout << "        \"file\" is a partial file name." << endl;
    cout << "  -h  Send test results to HTML file." << endl;
    cout << "        \"file\" is a partial file name for main HTML page."
         << endl;
    cout << "  -x  Send test results to xml file." << endl;
    cout << "        \"file\" is a partial file name." << endl;
    cout << "  -e  Show summary table at program exit time." << endl;
    cout << "  -r  Show summary table and then repeat tests." << endl;
    cout << "      Incompatible with -z." << endl;
    cout << "  -?  Show this help information." << endl;
    cout << "      Help is mutually exclusive with any other arguement."
         << endl;
    cout << "  --help  Show this help information." << endl;
}

// ----------------------------------------------------------------------------

MainArgs::MainArgs( unsigned int argc, const char * const argv[] ) :
    m_valid( false ),
    m_doShowHelp( false ),
    m_doFatalTest( false ),
    m_doNoTests( false ),
    m_doOnlyPassingTests( false ),
    m_doAssertOnFail( false ),
    m_doRepeatTests( false ),
    m_tableAtExitTime( false ),
    m_deleteAtExitTime( true ),
    m_outputOptions( ut::UnitTestSet::Nothing ),
    m_exeName( argv[0] ),
    m_xmlFileName( NULL ),
    m_htmlFileName( NULL ),
    m_textFileName( NULL )
{

    if ( 1 == argc )
    {
        return;
    }

    bool okay = true;
    bool parsedOutput = false;

    for ( unsigned int ii = 1; ( okay ) && ( ii < argc ); ++ii )
    {
        const char * ss = argv[ ii ];
        if ( ( NULL == ss ) || ( '-' != ss[0] ) )
        {
            okay = false;
            break;
        }
        const unsigned int length =
            static_cast< unsigned int >( ::strlen( ss ) );

        const char cc = ss[1];
        switch ( cc )
        {
            case '-':
                okay = ( ::strcmp( ss, "--help" ) == 0 );
                if ( okay )
                    okay = !m_doShowHelp;
                if ( okay )
                    m_doShowHelp = true;
                break;
            case '?':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doShowHelp;
                if ( okay )
                    m_doShowHelp = true;
                break;
            case 'a':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doAssertOnFail;
                if ( okay )
                    m_doAssertOnFail = true;
                break;
            case 'e':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_tableAtExitTime;
                if ( okay )
                    m_tableAtExitTime = true;
                break;
            case 'f':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doFatalTest;
                if ( okay )
                    m_doFatalTest = true;
                break;
            case 'h':
                okay = ( 3 < length ) && ( ':' == ss[2] );
                if ( okay )
                    okay = ( NULL == m_htmlFileName );
                if ( okay )
                    m_htmlFileName = ss + 3;
                break;
            case 'L':
                okay = ( length == 2 );
                if ( okay )
                    okay = m_deleteAtExitTime;
                if ( okay )
                    m_deleteAtExitTime = false;
                break;
            case 'o':
                okay = ( 3 < length ) && ( ':' == ss[2] );
                if ( okay )
                    okay = !parsedOutput;
                if ( okay )
                    okay = ParseOutputOptions (ss+3);
                parsedOutput = true;
                break;
            case 'p':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doOnlyPassingTests;
                if ( okay )
                    m_doOnlyPassingTests = true;
                break;
            case 'r':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doRepeatTests;
                if ( okay )
                    m_doRepeatTests = true;
                break;
            case 't':
                okay = ( 3 < length ) && ( ':' == ss[2] );
                if ( okay )
                    okay = ( NULL == m_textFileName );
                if ( okay )
                    m_textFileName = ss + 3;
                break;
            case 'x':
                okay = ( 3 < length ) && ( ':' == ss[2] );
                if ( okay )
                    okay = ( NULL == m_xmlFileName );
                if ( okay )
                    m_xmlFileName = ss + 3;
                break;
            case 'z':
                okay = ( length == 2 );
                if ( okay )
                    okay = !m_doNoTests;
                if ( okay )
                    m_doNoTests = true;
                break;
            default:
                okay = false;
                break;
        }
    }

    const bool standardError =
        ( 0 != ( m_outputOptions & ut::UnitTestSet::SendToCerr ) );
    const bool standardOutput =
        ( 0 != ( m_outputOptions |= ut::UnitTestSet::SendToCout ) );
    const bool noOutput = ( !m_doFatalTest ) && ( NULL == m_xmlFileName )
            && ( !standardError ) && ( !standardOutput )
            && ( NULL == m_textFileName ) && ( NULL == m_htmlFileName );
    if ( m_doShowHelp && okay )
    {
        okay = noOutput;
    }
    else
    {
        if ( m_doOnlyPassingTests && m_doFatalTest )
            okay = false;
        if ( m_doNoTests && m_doFatalTest )
            okay = false;
        if ( m_doNoTests && m_doOnlyPassingTests )
            okay = false;
        if ( m_doNoTests && m_doRepeatTests )
            okay = false;
        if ( noOutput )
            okay = false;
    }
    m_valid = okay;
}

// ----------------------------------------------------------------------------

bool MainArgs::ParseOutputOptions( const char * ss )
{

    bool okay = true;
    bool useDefault = false;
    bool useFullNames = false;
    bool showNothing = false;
    bool showHeader = false;
    bool showMessages = false;
    bool showPasses = false;
    bool showTimeStamp = false;
    bool showWarnings = false;
    bool showDividers = false;
    bool showIndexes = false;
    bool showSummaryTable = false;
    bool standardError = false;
    bool standardOutput = false;

    while ( okay && ( *ss != '\0' ) )
    {
        const char cc = *ss;
        switch ( cc )
        {
            case 'D':
                if ( useDefault )
                    okay = false;
                else
                    useDefault = true;
                break;
            case 'd':
                if ( showDividers )
                    okay = false;
                else
                    showDividers = true;
                break;
            case 'E':
                if ( standardError )
                    okay = false;
                else
                    standardError = true;
                break;
            case 'S':
                if ( standardOutput )
                    okay = false;
                else
                    standardOutput = true;
                break;
            case 'f':
                if ( useFullNames )
                    okay = false;
                else
                    useFullNames = true;
                break;
            case 'i':
                if ( showIndexes )
                    okay = false;
                else
                    showIndexes = true;
                break;
            case 'n':
                if ( showNothing )
                    okay = false;
                else
                    showNothing = true;
                break;
            case 'h':
                if ( showHeader )
                    okay = false;
                else
                    showHeader = true;
                break;
            case 'm':
                if ( showMessages )
                    okay = false;
                else
                    showMessages = true;
                break;
            case 'p':
                if ( showPasses )
                    okay = false;
                else
                    showPasses = true;
                break;
            case 't':
                if ( showTimeStamp )
                    okay = false;
                else
                    showTimeStamp = true;
                break;
            case 'T':
                if ( showSummaryTable )
                    okay = false;
                else
                    showSummaryTable = true;
                break;
            case 'w':
                if ( showWarnings )
                    okay = false;
                else
                    showWarnings = true;
                break;
            default:
                okay = false;
                break;
        }
        ++ss;
    }

    const bool noOptions =
        ( !showTimeStamp ) && ( !showDividers )
        && ( !showPasses ) && ( !showMessages )
        && ( !showHeader ) && ( !showWarnings );
    if ( showNothing )
        okay = noOptions;
    if ( noOptions && ( !showNothing ) )
        useDefault = true;
    if ( useDefault )
        m_outputOptions = ut::UnitTestSet::Default;
    if ( showDividers )
        m_outputOptions |= ut::UnitTestSet::Dividers;
    if ( useFullNames )
        m_outputOptions |= ut::UnitTestSet::FullDayName;
    if ( showHeader )
        m_outputOptions |= ut::UnitTestSet::Headers;
    if ( showMessages )
        m_outputOptions |= ut::UnitTestSet::Messages;
    if ( showPasses )
        m_outputOptions |= ut::UnitTestSet::Passes;
    if ( showTimeStamp )
        m_outputOptions |= ut::UnitTestSet::TimeStamp;
    if ( showWarnings )
        m_outputOptions |= ut::UnitTestSet::Warnings;
    if ( showIndexes )
        m_outputOptions |= ut::UnitTestSet::AddTestIndex;
    if ( showSummaryTable )
        m_outputOptions |= ut::UnitTestSet::SummaryTable;
    if ( standardError )
        m_outputOptions |= ut::UnitTestSet::SendToCerr;
    if ( standardOutput )
        m_outputOptions |= ut::UnitTestSet::SendToCout;

    return okay;
}

// ----------------------------------------------------------------------------

int main( unsigned int argc, const char * const argv[] )
{

    const MainArgs args( argc, argv );
    if ( !args.IsValid() )
    {
        cout << "Your command line arguements are invalid!" << endl;
        args.ShowHelp();
        return 1;
    }
    if ( args.DoShowHelp() )
    {
        args.ShowHelp();
        return 0;
    }

    const bool deleteAtExitTime = args.DeleteAtExitTime();
    const ut::UnitTestSet::OutputOptions options = args.GetOutputOptions();
    const ut::UnitTestSet::ErrorState status = ut::UnitTestSet::Create(
        "Thingy Tests", args.GetTextFileName(), args.GetHtmlFileName(),
        args.GetXmlFileName(), options, deleteAtExitTime );
    if ( ( status != ut::UnitTestSet::Success )
      && ( status != ut::UnitTestSet::AlreadyExists ) )
    {
        cout << "An error occurred when creating the UnitTestSet singleton!"
             << endl;
        return 2;
    }

    ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
    if ( !uts.DoesOutputOption( options ) )
    {
        cout << "The UnitTestSet singleton does not apply the same output "
             << "options requested." << endl;
        return 3;
    }

    bool okay = true;
    if ( args.DoAssertOnFail() )
    {
        // The assertions here check if the functions to add and remove
        // receivers work properly.
        okay = uts.AddReceiver( NULL );
        assert( !okay );
        okay = uts.RemoveReceiver( NULL );
        assert( !okay );
        UnitTestAsserter * asserter = new UnitTestAsserter;
        okay = uts.AddReceiver( asserter );
        assert( okay );
        okay = uts.AddReceiver( asserter );
        assert( okay );
        okay = uts.RemoveReceiver( asserter );
        assert( okay );
        okay = uts.AddReceiver( asserter );
        assert( okay );
    }

    if ( args.DoNoTests() )
    {
        if ( !args.DoMakeTableAtExitTime() )
        {
            uts.OutputSummary();
        }
        return 0;
    }

    PassingThingyTest();
    PassWithWarningsThingyTest();
    if ( !args.DoOnlyPassingTest() )
    {
        TestThingy1();
        TestThingy2();
        EmptyThingyTest();
        ExceptionTest();
        if ( args.DoFatalTest() )
            FatalThingyTest();
    }

    if ( args.DoRepeatTests() )
    {
        uts.OutputSummary();
        PassingThingyTest();
        PassWithWarningsThingyTest();
        if ( !args.DoOnlyPassingTest() )
        {
            TestThingy1();
            TestThingy2();
            EmptyThingyTest();
            ExceptionTest();
            if ( args.DoFatalTest() )
                FatalThingyTest();
        }
    }

    if ( !args.DoMakeTableAtExitTime() )
    {
        uts.OutputSummary();
    }

    return 0;
}

// ----------------------------------------------------------------------------

// $Log: main.cpp,v $
// Revision 1.28  2007/07/19 19:59:45  rich_sposato
// Added output option to show summary table.
//
// Revision 1.27  2007/07/17 23:55:44  rich_sposato
// Added ability to do unit tests safelt at exit time.
//
// Revision 1.26  2007/07/17 22:58:13  rich_sposato
// Moved sendToCerr & sendToCout into OutputOptions enum family.
//
// Revision 1.25  2007/07/17 21:11:20  rich_sposato
// Added ability to send test results to xml output file.
//
// Revision 1.24  2007/06/07 21:51:54  rich_sposato
// Added output option to show unit test index in each result line.
//
// Revision 1.23  2007/06/07 20:38:24  rich_sposato
// Renamed parameter.
//
// Revision 1.22  2007/06/07 17:16:41  rich_sposato
// Added default output option.
//
// Revision 1.21  2007/06/07 01:37:45  rich_sposato
// Added output option to show dividing lines in text.
//
// Revision 1.20  2007/06/04 20:44:08  rich_sposato
// Added ability to send output to standard error.
//
// Revision 1.19  2007/06/02 00:05:59  rich_sposato
// Minor change to usage message.
//
// Revision 1.18  2007/05/17 18:14:56  rich_sposato
// Added output option so program can choose full weekday name, or just an
// abbreviated name.
//
// Revision 1.17  2007/05/17 17:27:16  rich_sposato
// Changed observer functions to no longer pure virtual, and added default
// function bodies so child classes do not have to implement all functions,
// just the ones they need.
//
// Revision 1.16  2007/04/25 20:45:01  rich_sposato
// Added more example tests.  Added more documentation comments.
//
// Revision 1.15  2007/04/04 00:15:33  rich_sposato
// Added UnitTest which only fails with exceptions.
//
// Revision 1.14  2007/04/03 18:53:23  rich_sposato
// Added more command line options.  Added checks for re-entrancy.  Added a
// demostration test.
//
// Revision 1.13  2007/04/02 22:55:52  rich_sposato
// Added ability to send test result info to external observers in addition to
// text and html files.
//
// Revision 1.12  2007/03/21 23:13:16  rich_sposato
// Various minor changes.
//
// Revision 1.11  2007/03/15 00:45:29  rich_sposato
// Added ability to specify output behavior via command line args.
//
// Revision 1.10  2007/03/10 01:40:05  rich_sposato
// Added ability to say that some tests are required.
//
// Revision 1.9  2007/03/09 23:44:32  rich_sposato
// Added invariant checking.  Added ability to test warnings.
//
// Revision 1.8  2007/03/09 18:22:55  rich_sposato
// Added output option for messages.
//
// Revision 1.7  2007/03/09 01:53:48  rich_sposato
// Added begin and end timestamps to output.
//
// Revision 1.6  2007/03/08 22:04:44  rich_sposato
// Added ability to send messages directly to test results output.
//
// Revision 1.5  2007/03/08 18:33:35  rich_sposato
// Changed return type of Create function from bool to enum.
//
// Revision 1.4  2007/03/08 18:18:13  rich_sposato
// Converted UnitTestSet to a singleton.
//
// Revision 1.3  2007/03/08 01:56:36  rich_sposato
// Now passes in partial names of html and text files.
//
// Revision 1.2  2007/03/07 22:56:30  rich_sposato
// Added copyright notices and keywords.
//
