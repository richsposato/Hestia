// ----------------------------------------------------------------------------
// The C++ Unit Test Library
// Copyright (c) 2008 by Rich Sposato
//
// Permission to use, copy, modify, distribute and sell this software for any
// purpose is hereby granted under the terms stated in the GNU Library Public
// License, provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear in
// supporting documentation.
//
// ----------------------------------------------------------------------------

// $Header: /cvsroot/hestia/CppUnitTest/src/UnitTest_E.cpp,v 1.1 2008/03/22 21:36:39 rich_sposato Exp $


// ----------------------------------------------------------------------------

#include "../include/UnitTest_E.hpp"

#include <time.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <vector>
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

static const char * s_timeStampFormatSpec = "%A, %Y %b %d at %H:%M:%S in %z";

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
     @param options A bitfield of all possible output options - each bit is a
      different option.
     */
    UnitTestSetImpl( const char * testName, UnitTestResultReceiver * receiver,
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

    /// Adds start and stop timestamps to output.
    void ShowTimeStamp( void );

    /// Creates detail rows in the summary table - one row for each unit test.
    void MakeTableRows( void );

    /// Creates summary table for output.
    void OutputSummaryTable( void );

    /// Overall name of test.
    string m_testName;

    /// Container of UnitTest's.
    TUnitTestChildren m_tests;

    /// Container of output receivers.
    ut::UnitTestResultReceiver * m_receiver;

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
    (void)total;
    assert( m_itemCount == total );
    if ( ( 0 < m_failCount ) || ( 0 < m_exceptions ) )
    {
        assert( m_madeHeader );
    }
}

// ----------------------------------------------------------------------------

UnitTestSetImpl::UnitTestSetImpl( const char * testName,
    UnitTestResultReceiver * receiver, UnitTestSet::OutputOptions info ) :
    m_testName( testName ),
    m_tests(),
    m_receiver( receiver ),
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
    DEBUG_CODE( CheckInvariants() );
}

// ----------------------------------------------------------------------------

UnitTestSetImpl::~UnitTestSetImpl( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    Clear();

    if ( nullptr == m_receiver )
        return;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    try
    {
        m_receiver->FinalEnd();
    }
    catch ( ... ) { }
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
    if ( nullptr == m_receiver )
        return;
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    bool keep = false;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    try
    {
        keep = m_receiver->ShowTestLine( test, result, fileName, line,
            expression, message );
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
    assert( m_usingReceivers );
    m_usingReceivers = false;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputMessage( UnitTest * test, const char * fileName,
    unsigned int line, const char * message )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    if ( nullptr == m_receiver )
        return;
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
    try
    {
        keep = m_receiver->ShowMessage( test, fileName, line, message );
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
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

void UnitTestSetImpl::StartOutput( void )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    if ( nullptr == m_receiver )
        return;
    if ( m_didPageHeader )
        return;
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    assert( !m_usingReceivers );
    m_usingReceivers = true;
    m_didAnyTest = true;

    bool keep = false;
    try
    {
        keep = m_receiver->Start( m_testName.c_str() );
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
    assert( m_usingReceivers );
    m_usingReceivers = false;
    m_didPageHeader = true;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputTestHeader( const UnitTest * test )
{
    assert( nullptr != this );
    DEBUG_CODE( CheckInvariants() );
    if ( nullptr == m_receiver )
        return;
    if ( !m_showHeaders )
        return;
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    m_didAnyTest = true;
    bool keep = false;
    assert( !m_usingReceivers );
    m_usingReceivers = true;
    try
    {
        keep = m_receiver->ShowTestHeader( test );
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
    assert( m_usingReceivers );
    m_usingReceivers = false;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::ShowTimeStamp( void )
{
    assert( nullptr != this );
    if ( nullptr == m_receiver )
        return;
    assert( m_usingReceivers );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    time_t timeNow;
    time( &timeNow );
    const tm * endTime = ::localtime( &timeNow );
    char buffer[ 160 ];
    ::strftime( buffer, sizeof(buffer)-1, s_timeStampFormatSpec, endTime );

    bool keep = false;
    try
    {
        keep  = m_receiver->ShowTimeStamp( true, m_timeString );
        keep &= m_receiver->ShowTimeStamp( false, buffer );
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::MakeTableRows( void )
{
    assert( nullptr != this );
    if ( nullptr == m_receiver )
        return;
    assert( m_showFinalTable );
    assert( m_usingReceivers );

    bool keep = false;
    const char * pResult = nullptr;
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

        keep = true;
        try
        {
            keep = m_receiver->ShowTableLine( pTest );
        }
        catch ( ... )
        {
            keep = false;
        }
        if ( !keep )
            m_receiver = nullptr;
    }
}

// ----------------------------------------------------------------------------

void UnitTestSetImpl::OutputSummaryTable( void )
{
    assert( nullptr != this );
    if ( nullptr == m_receiver )
        return;
    assert( m_showFinalTable );
    assert( m_usingReceivers );
    DEBUG_CODE( CheckInvariants() );
    DEBUG_CODE( UnitTestSetChecker guard( this ); (void)guard; );

    const bool hasAnyTests = ( 0 < m_tests.size() );
    bool keep = false;
    try
    {
        keep = m_receiver->StartSummaryTable();
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
    {
        m_receiver = nullptr;
        return;
    }

    if ( hasAnyTests )
    {
        MakeTableRows();
    }

    try
    {
        keep  = m_receiver->ShowTotalLine( m_itemPassCount,
            m_itemWarnCount, m_itemFailCount, m_itemExceptCount,
            m_itemCount );
        keep &= m_receiver->ShowSummaryLine( m_testPassCount,
            m_testWarnCount, m_testFailCount, m_testExceptCount,
            m_testCount );
        keep &= m_receiver->EndSummaryTable();
    }
    catch ( ... )
    {
        keep = false;
    }
    if ( !keep )
        m_receiver = nullptr;
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
    assert( total == m_testCount );
    (void)total;

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
    UnitTestResultReceiver * receiver, UnitTestSet::OutputOptions info,
    bool deleteAtExitTime )
{
    if ( s_instance != nullptr )
    {
        return UnitTestSet::AlreadyExists;
    }
    if ( nullptr == receiver )
    {
        return UnitTestSet::NoReceiver;
    }
    if ( UnitTestSet::NoOptions == info )
    {
        return UnitTestSet::NoOptions;
    }
    if ( IsEmptyString( testName ) )
    {
        testName = "Unit Tests";
    }
    s_instance = new UnitTestSet( testName, receiver, info );
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

UnitTestSet::UnitTestSet( const char * testName,
    UnitTestResultReceiver * receiver, UnitTestSet::OutputOptions info ) :
    m_impl( nullptr )
{
    assert( nullptr != this );
    m_impl = new UnitTestSetImpl( testName, receiver, info );
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

UnitTestResultReceiver * UnitTestSet::GetReceiver( void )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    return m_impl->m_receiver;
}

// ----------------------------------------------------------------------------

bool UnitTestSet::SetReceiver( UnitTestResultReceiver * receiver )
{
    assert( nullptr != this );
    assert( nullptr != m_impl );
    DEBUG_CODE( m_impl->CheckInvariants() );
    if ( nullptr == receiver )
        return false;
    if ( m_impl->m_receiver != nullptr )
        return false;
    m_impl->m_receiver = receiver;
    return true;
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
        test->m_index = m_impl->m_tests.size();
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

// $Log: UnitTest_E.cpp,v $
// Revision 1.1  2008/03/22 21:36:39  rich_sposato
// Adding more files to unit test project.
//
