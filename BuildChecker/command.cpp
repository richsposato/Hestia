// $Header: /cvsroot/hestia/BuildChecker/command.cpp,v 1.3 2008/12/12 22:56:42 rich_sposato Exp $

/// @file command.cpp Defines classes used to parse input parameters.

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

#include "command.hpp"

#include <assert.h>
#include <iostream>

#if defined( ABC_USES_INI_FILE )
    #include <UtilParsers/Util/include/ErrorReceiver.hpp>
    #include <UtilParsers/Config/include/ConfigParser.hpp>
#endif

#include "utility.hpp"


using namespace ::std;

#if defined( ABC_USES_INI_FILE )
    using namespace ::Parser;
#endif


namespace hestia
{

static unsigned int s_defaultKeepCount = 60;
static unsigned int s_maxKeepCount = 0xFFFFFFFF;


// ShowUsageMessage ---------------------------------------------------------------------------

void ShowUsageMessage( const char * exeName )
{
    cout << "Automated Build Checker parses compiler build output for error" << endl;
    cout << "   and warning messages, sends those to an html output file," << endl;
    cout << "   and makes a summary html file of the results." << endl;
    cout << endl;
    cout << "Usage: " << exeName << " -i input_file -o output_file -b build_name" << endl;
    cout << "   [-t build_type] [-t title] [-c compiler] [-k=#] [-C:type=color]" << endl;
#if defined( ABC_USES_INI_FILE )
    cout << "Usage: " << exeName << " -f config_file" << endl;
#endif
    cout << "Usage: " << exeName << " [-?] [--help]" << endl;
    cout << "  Arg  Meaning:" << endl;
    cout << "   -i  Input file in text form. (This is the result file from a compiler.)" << endl;
    cout << "   -o  Main html output file showing summary of build results." << endl;
    cout << "       Other output files go into the same directory as this file." << endl;
    cout << endl;
    cout << "   -c  Name and version of compiler that made result file." << endl;
    cout << "       codeblocks = Code::Blocks IDE" << endl;
    cout << "       msvc9 = Microsoft Visual Studio 9" << endl;
    cout << "       Default is msvc9." << endl;
    cout << "   -n  Build name. Overall name for collection of projects." << endl;
    cout << "       Could be name of a branch, release candidate, product line, etc..." << endl;
    cout << "   -b  Build or configuration type. (e.g. Debug, Release, Test, etc...)" << endl;
    cout << endl;
    cout << "   -k  Optional number of previous build results to keep before pruning older records." << endl;
    cout << "       Keep count can be a number (e.g. -k=100 or -k=365) or all (-k=all)." << endl;
    cout << "   -t  Title of main page.  This will not change the title if main page already exists." << endl;
    cout << "   -C  Specify color used in html pages.  Instead of spelling out name of type," << endl;
    cout << "       you can use the first initial. (e.g. - b for back)" << endl;
    cout << "       Types    Default Color & Purpose" << endl;
    cout << "       b|back   black   Background color for html pages." << endl;
    cout << "       t|text   white   Color of plain text and informational messages." << endl;
    cout << "       p|pass   green   Color of passing projects." << endl;
    cout << "       w|warn   yellow  Color of warning messages and passing projects with warnings." << endl;
    cout << "       e|error  red     Color of error messages and failed projects." << endl;
    cout << endl;
#if defined( ABC_USES_INI_FILE )
    cout << "   -f   Use config file.  Overrides all other options." << endl;
#endif
    cout << "   -? or --help  Show this help info.  Overrides all other options." << endl;
    cout << endl;
    cout << "   Output file for current build is build_result_YYYY_MM_DD_hh_mm.html." << endl;
    cout << "   Where YYYY = year, MM = month, DD = day, hh = hour, mm = minute." << endl;
    cout << "   This also makes or adds to a main file with a summary of previous build results." << endl;
    cout << "   It will keep " << s_defaultKeepCount
         << " build records unless you specify a different number to keep." << endl;
}

#if defined( ABC_USES_INI_FILE )

// ConfigReceiver -----------------------------------------------------------------------------

class ConfigReceiver : private IConfigReceiver, private IParseErrorReceiver
{
public:

    static ConfigReceiver & GetIt( void );

    inline IConfigReceiver * AsConfigReceiver( void )
    {
        return dynamic_cast< IConfigReceiver * >( this );
    }

    inline IParseErrorReceiver * AsErrorReceiver( void )
    {
        return dynamic_cast< IParseErrorReceiver * >( this );
    }

	bool IsValid( void ) const;

	inline const char * GetInputFile( void ) const { return m_inputFile.c_str(); }

	inline const char * GetOutputFile( void ) const { return m_outputFile.c_str(); }

	inline const char * GetBuildName( void ) const { return m_buildName.c_str(); }

	inline const char * GetBuildType( void ) const { return m_buildType.c_str(); }

	inline const char * GetCompiler( void ) const { return m_compiler.c_str(); }

	inline const char * GetTitle( void ) const { return m_title.c_str(); }

    inline const char * GetColorBackground( void ) const { return m_colorBackground.c_str(); }

    inline const char * GetColorText( void ) const { return m_colorText.c_str(); }

    inline const char * GetColorPasses( void ) const { return m_colorPasses.c_str(); }

    inline const char * GetColorWarnings( void ) const { return m_colorWarnings.c_str(); }

    inline const char * GetColorErrors( void ) const { return m_colorErrors.c_str(); }

	inline unsigned int GetKeepCount( void ) const { return m_keepCount; }

    inline MatcherSet::MatchingRules & GetMatchingRules( void ) { return m_rules; }

private:

    enum Sections
    {
        None   = 0x00,
        Input  = 0x01,
        Output = 0x02,
        Rules  = 0x04,
        Colors = 0x08
    };

    /// Not implemented.
    ConfigReceiver( const ConfigReceiver & );
    /// Not implemented.
    ConfigReceiver & operator = ( const ConfigReceiver & );

    static void Destroy( void );

    ConfigReceiver( void );

    virtual ~ConfigReceiver( void );

	virtual bool AddGlobalKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

	virtual bool AddSection( const char * nameStart, const char * nameEnd );

	virtual bool AddSectionKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

	virtual void ParsedConfigFile( bool valid );

    virtual bool GiveParseMessage( ::Parser::ErrorLevel::Levels level,
        const ::Parser::CharType * message );

    virtual bool GiveParseMessage( ::Parser::ErrorLevel::Levels level,
        const ::Parser::CharType * message, unsigned long line );

    virtual bool GiveParseMessage( ::Parser::ErrorLevel::Levels level,
        const ::Parser::CharType * message, const char * filename, unsigned long line );

    void ProcessInputKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

    void ProcessOutputKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

    void ProcessRulesKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

    void ProcessColorsKey( const char * keyStart, const char * keyEnd,
        const char * valueStart, const char * valueEnd );

    static ConfigReceiver * s_instance;

    /// True if command line options are valid.
	bool m_valid;
    /// True if the config file has an Input section.
    bool m_hasInputSection;
    /// True if the config file has an Output section.
    bool m_hasOutputSection;
	/// Current section being read from file.
	Sections m_currentSection;

    /// Pointer to input filename.
	string m_inputFile;
    /// Pointer to output directory.
	string m_outputFile;
    /// Pointer to name of build.
    string m_buildName;
    /// Pointer to type of build.
    string m_buildType;
    /// Name of compiler which made output messages.
    string m_compiler;
    /// Title shown on main page.
    string m_title;
    /// Background color of html pages.
    string m_colorBackground;
    /// Color of plain text on html pages.
    string m_colorText;
    /// Color of passing projects on html pages.
    string m_colorPasses;
    /// Color of warning messages on html pages.
    string m_colorWarnings;
    /// Color of error messages on html pages.
    string m_colorErrors;
    /// Number of older records to keep.
    unsigned int m_keepCount;
    /// Additional matching rules specified by user.
    MatcherSet::MatchingRules m_rules;
};

ConfigReceiver * ConfigReceiver::s_instance = nullptr;

#endif


// GetStringArg -------------------------------------------------------------------------------

bool GetStringArg( const char * & target, unsigned int argc, const char * const argv[],
    unsigned int ii, unsigned int length, const char * msg1, const char * msg2, const char * msg3 )
{
    if ( nullptr != target )
    {
        cerr << msg1 << endl;
        return false;
    }
    if ( 2 != length )
    {
        cerr << msg2 << endl;
        return false;
    }
    ++ii;
    if ( ii == argc )
    {
        cerr << msg3 << endl;
        return false;
    }
    target = argv[ ii ];
    return true;
}

// CommandLineArgs::CommandLineArgs -----------------------------------------------------------

CommandLineArgs::CommandLineArgs( unsigned int argc, const char * const argv[] ) :
	m_valid( false ),
	m_showHelp( false ),
	m_exeName( FindFileNameInPath( argv[0] ) ),
	m_inputFile( nullptr ),
	m_outputFile( nullptr ),
	m_buildName( nullptr ),
	m_buildType( nullptr ),
	m_compiler( nullptr ),
	m_title( nullptr ),
    m_colorBackground( nullptr ),
    m_colorText( nullptr ),
    m_colorPasses( nullptr ),
    m_colorWarnings( nullptr ),
    m_colorErrors( nullptr ),
	m_keepCount( 0 ),
	m_rules()
{
	assert( nullptr != this );

    bool okay = true;
    bool useConfigFile = false;
    unsigned int ii = 1;
    do
    {
		const char * arg = argv[ ii ];
		if ( *arg != '-' )
		{
			cerr << "Invalid command line option!  All options must begin with dash." << endl;
			okay = false;
			break;
		}
		const unsigned int length = ::strlen( arg );

        switch ( arg[1] )
        {
            case '-':
            {
                if ( strcmp( arg, "--help" ) )
                    m_showHelp = true;
                else
                {
                    cerr << "Invalid command line option: " << arg << endl;
                    okay = false;
                }
				break;
            }
			case '?':
				m_showHelp = true;
				break;
			case 'i':
                okay = GetStringArg( m_inputFile, argc, argv, ii, length,
                    "Invalid command line option!  Input file already defined.",
                    "Invalid command line option!  Input file option is: -i filename",
                    "Could not find filename for input file. Syntax is: -i filename" );
                ++ii;
				break;
			case 'o':
                okay = GetStringArg( m_outputFile, argc, argv, ii, length,
					"Invalid command line option!  Output file already defined.",
					"Invalid command line option!  Output file option is: -o filename.",
					"Could not find filename for output file. Syntax is: -o filename" );
                ++ii;
				break;
            case 'n':
				okay = GetStringArg( m_buildName, argc, argv, ii, length,
					"Invalid command line option!  Build name already defined.",
					"Invalid command line option!  Build name option is: -b name.",
					"Could not find name of build. Syntax is: -b name" );
                ++ii;
                break;
            case 'b':
				okay = GetStringArg( m_buildType, argc, argv, ii, length,
					"Invalid command line option!  Build type already defined.",
					"Invalid command line option!  Build type option is: -b type.",
					"Could not find name of build. Syntax is: -b type" );
                ++ii;
                break;
            case 'c':
				okay = GetStringArg( m_compiler, argc, argv, ii, length,
                    "Invalid command line option!  Compiler already defined.",
					"Invalid command line option!  Compiler name option is: -c compiler.",
					"Could not find name of compiler. Syntax is: -c compiler" );
                ++ii;
                break;
            case 't':
				okay = GetStringArg( m_title, argc, argv, ii, length,
                    "Invalid command line option!  Title already defined.",
					"Invalid command line option!  Title option is: -t title.",
					"Could not find title. Syntax is: -t title" );
                ++ii;
                break;
            case 'f':
            {
#if defined( ABC_USES_INI_FILE )
                useConfigFile = true;
                if ( length != 2 )
                {
                    cerr << "Invalid parameter!  Syntax for config-file option is -f filename" << endl;
                    okay = false;
                    break;
                }
                ++ii;
                if ( ii == argc )
                {
                    cerr << "Invalid parameter!  Missing name for config-file. Syntax is -f filename" << endl;
                    okay = false;
                }
                const char * filename = argv[ ii ];
                okay = GetInfoFromConfigFile( filename );
#else
                okay = false;
                cerr << "Invalid parameter! This version of " << m_exeName << " does not support the config-file option." << endl;
#endif
                break;
            }
            case 'C':
            {
                if ( length <= 8 )
                {
                    cerr << "Invalid parameter! Length is too short. Syntax for color option is -C:type=color" << endl;
                    okay = false;
                    break;
                }
                if ( ':' != arg[2] )
                {
                    cerr << "Invalid parameter! Missing colon. Syntax for color option is -C:type=color" << endl;
                    okay = false;
                    break;
                }
                const char * spot = ::strchr( arg+4, '=' );
                if ( IsEmptyString( spot ) )
                {
                    cerr << "Invalid parameter! Missing equal sign. Syntax for color option is -C:type=color" << endl;
                    okay = false;
                    break;
                }
                ++spot;
                if ( IsEmptyString( spot ) )
                {
                    cerr << "Invalid parameter! No name of color after equal sign. Syntax for color option is -C:type=color" << endl;
                    okay = false;
                    break;
                }
                switch ( arg[3] )
                {
                    case 'b':
                    {
                        m_colorBackground = spot;
                        break;
                    }
                    case 't':
                    {
                        m_colorText = spot;
                        break;
                    }
                    case 'p':
                    {
                        m_colorPasses = spot;
                        break;
                    }
                    case 'e':
                    {
                        m_colorErrors = spot;
                        break;
                    }
                    case 'w':
                    {
                        m_colorWarnings = spot;
                        break;
                    }
                    default:
                    {
                        cerr << "Invalid parameter!  Syntax for color option is -C:type=color" << endl;
                        okay = false;
                        break;
                    }
                }
                break;
            }
            case 'k':
            {
                if ( 0 != m_keepCount )
                {
                    cerr << "Invalid parameter!  The keep count was already set!" << endl;
                    okay = false;
                    break;
                }
                if ( length <= 3 )
                {
                    cerr << "Invalid parameter!  Keep count option is -k=# or -k=all." << endl;
                    okay = false;
                    break;
                }
                if ( arg[2] != '=' )
                {
                    cerr << "Invalid parameter!  Keep count option is -k=# or -k=all." << endl;
                    okay = false;
                    break;
                }
                const char * option = arg + 3;
                if ( ::isdigit( *option ) == 0 )
                {
                    if ( ::strcmp( option, "all" ) == 0 )
                        m_keepCount = s_maxKeepCount;
                    else
                    {
                        cerr << "Invalid parameter!  Keep count option is -k=# or -k=all." << endl;
                        okay = false;
                    }
                }
                else
                {
                    m_keepCount = ::atoi( arg + 3 );
                    if ( 0 == m_keepCount )
                    {
                        cerr << "Invalid parameter!  Keep count should not be zero!" << endl;
                        okay = false;
                    }
                }
                break;
            }
            default:
                okay = false;
        }
        ++ii;
    } while ( ( ii < argc ) && okay && ( !m_showHelp ) && ( !useConfigFile ) );

    if ( okay && ( ii < argc ) )
    {
        if ( useConfigFile || m_showHelp )
        {
            cerr << "Error!  The -?, --help and -f parameters are not valid with any others." << endl;
            okay = false;
        }
    }

    if ( !m_showHelp && okay )
    {
        if ( nullptr == m_buildName )
        {
            cerr << "Invalid parameter!  No build name specified!  Syntax is -b build" << endl;
            okay = false;
        }
        else if ( nullptr == m_inputFile )
        {
            cerr << "Invalid parameter!  No input file specified!  Syntax is -i input_file" << endl;
            okay = false;
        }
        else if ( nullptr == m_outputFile )
        {
            cerr << "Invalid parameter!  No output path specified!  Syntax is -o output_directory" << endl;
            okay = false;
        }
        if ( 0 == m_keepCount )
            m_keepCount = s_defaultKeepCount;
        if ( nullptr == m_title )
            m_title = "Build Results";
        if ( nullptr == m_buildType )
            m_buildType = "?";
    }

    m_valid = okay;
}

#if defined( ABC_USES_INI_FILE )

// CommandLineArgs::GetInfoFromConfigFile -------------------------------------------------------------

bool CommandLineArgs::GetInfoFromConfigFile( const char * filename )
{
	assert( nullptr != this );
	if ( IsEmptyString( filename ) )
	{
	    cerr << "Error!  Could not find name of config file!" << endl;
        return false;
	}

	ConfigReceiver & receiver = ConfigReceiver::GetIt();
    ConfigParser parser;
    parser.SetMessageReceiver( receiver.AsErrorReceiver() );
    const ConfigParser::ParseResults result = parser.Parse( filename, receiver.AsConfigReceiver() );
    if ( result != ConfigParser::AllValid )
    {
	    cerr << "Error!  Could not parse config file.  " << ConfigParser::Name( result ) << endl;
        return false;
    }

	if ( !receiver.IsValid() )
        return false;

    m_buildName  = receiver.GetBuildName();
    m_buildType  = receiver.GetBuildType();
    m_compiler   = receiver.GetCompiler();
    m_inputFile  = receiver.GetInputFile();
    m_outputFile = receiver.GetOutputFile();
    m_keepCount  = receiver.GetKeepCount();

    if ( !IsEmptyString( receiver.GetColorBackground() ) )
        m_colorBackground = receiver.GetColorBackground();
    if ( !IsEmptyString( receiver.GetColorText() ) )
        m_colorText = receiver.GetColorText();
    if ( !IsEmptyString( receiver.GetColorPasses() ) )
        m_colorPasses = receiver.GetColorPasses();
    if ( !IsEmptyString( receiver.GetColorWarnings() ) )
        m_colorWarnings = receiver.GetColorWarnings();
    if ( !IsEmptyString( receiver.GetColorErrors() ) )
        m_colorErrors = receiver.GetColorErrors();

    if ( !IsEmptyString( receiver.GetTitle() ) )
        m_title = receiver.GetTitle();
    MatcherSet::MatchingRules & rules = receiver.GetMatchingRules();
    if ( rules.size() != 0 )
        m_rules.swap( rules );

	return true;
}

// ConfigReceiver::GetIt ----------------------------------------------------------------------

ConfigReceiver & ConfigReceiver::GetIt( void )
{
    if ( nullptr == s_instance )
    {
        s_instance = new ConfigReceiver;
        assert( nullptr != s_instance );
        ::atexit( &Destroy );
    }

    return *s_instance;
}

// ConfigReceiver::Destroy --------------------------------------------------------------------

void ConfigReceiver::Destroy( void )
{
    if ( nullptr != s_instance )
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

// ConfigReceiver::ConfigReceiver -------------------------------------------------------------

ConfigReceiver::ConfigReceiver( void ) :
	m_valid( true ),
    m_hasInputSection( false ),
    m_hasOutputSection( false ),
	m_currentSection( None ),
	m_inputFile(),
	m_outputFile(),
	m_buildName(),
	m_buildType(),
	m_compiler(),
	m_title(),
    m_colorBackground(),
    m_colorText(),
    m_colorPasses(),
    m_colorWarnings(),
    m_colorErrors(),
	m_keepCount( s_defaultKeepCount ),
	m_rules()
{
	assert( nullptr != this );
}

// ConfigReceiver::ConfigReceiver -------------------------------------------------------------

ConfigReceiver::~ConfigReceiver( void )
{
	assert( nullptr != this );
}

// ConfigReceiver::IsValid --------------------------------------------------------------------

bool ConfigReceiver::IsValid( void ) const
{
	assert( nullptr != this );
	if ( !m_valid || !m_hasInputSection || !m_hasOutputSection )
        return false;
    if ( ( None == m_currentSection ) || m_compiler.empty() )
        return false;
    if ( m_inputFile.empty() || m_outputFile.empty() )
        return false;
    if ( m_buildName.empty() || m_buildType.empty() )
        return false;
    return true;
}

// ConfigReceiver::ProcessInputKey ------------------------------------------------------------

void ConfigReceiver::ProcessInputKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
	assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );
	assert( m_currentSection == Input );

    const unsigned int keyLength = keyEnd - keyStart;
    const unsigned int valueLength = valueEnd - valueStart;
    if ( 0 == keyLength )
    {
        cerr << "Error!  Empty key found in Input section of config file!" << endl;
        m_valid = false;
    }
    else if ( 0 == valueLength )
    {
        cerr << "Error!  Empty value found in Input section of config file!" << endl;
        m_valid = false;
    }
    else if ( ::strncmp( keyStart, "File", keyLength ) == 0 )
        m_inputFile.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Compiler", keyLength ) == 0 )
        m_compiler.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "BuildName", keyLength ) == 0 )
        m_buildName.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "BuildType", keyLength ) == 0 )
        m_buildType.assign( valueStart, valueLength );
    else
    {
        cerr << "Error!  Invalid key found in Input section of config file!" << endl;
        m_valid = false;
    }
}

// ConfigReceiver::ProcessOutputKey -----------------------------------------------------------

void ConfigReceiver::ProcessOutputKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
	assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );
	assert( m_currentSection == Output );

    const unsigned int keyLength = keyEnd - keyStart;
    const unsigned int valueLength = valueEnd - valueStart;
    if ( 0 == keyLength )
    {
        cerr << "Error!  Empty key found in Output section of config file!" << endl;
        m_valid = false;
    }
    else if ( 0 == valueLength )
    {
        cerr << "Error!  Empty value found in Output section of config file!" << endl;
        m_valid = false;
    }
    else if ( ::strncmp( keyStart, "File", keyLength ) == 0 )
        m_outputFile.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Title", keyLength ) == 0 )
        m_title.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Keep", keyLength ) == 0 )
        m_keepCount = ::atoi( valueStart );
    else
    {
        cerr << "Error!  Invalid key found in Output section of config file!" << endl;
        m_valid = false;
    }
}

// ConfigReceiver::ProcessRulesKey ------------------------------------------------------------

void ConfigReceiver::ProcessRulesKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
	assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );
	assert( m_currentSection == Rules );

    const unsigned int keyLength = keyEnd - keyStart;
    const unsigned int valueLength = valueEnd - valueStart;
    if ( 0 == keyLength )
    {
        cerr << "Error!  Empty key found in Rules section of config file!" << endl;
        m_valid = false;
    }
    else if ( 0 == valueLength )
    {
        cerr << "Error!  Empty value found in Rules section of config file!" << endl;
        m_valid = false;
    }
    else
    {
        LineType::Type lineType = LineType::Unknown;
        if ( ::strncmp( keyStart, "UpToDate", keyLength ) == 0 )
            lineType = LineType::UpToDate;
        else if ( ::strncmp( keyStart, "Skipped", keyLength ) == 0 )
            lineType = LineType::Skipped;
        else if ( ::strncmp( keyStart, "Info", keyLength ) == 0 )
            lineType = LineType::Info;
        else if ( ::strncmp( keyStart, "Warning", keyLength ) == 0 )
            lineType = LineType::Warning;
        else if ( ::strncmp( keyStart, "Error", keyLength ) == 0 )
            lineType = LineType::Error;

        if ( LineType::Unknown == lineType )
        {
            cerr << "Error!  Invalid key found in Output section of config file!" << endl;
            m_valid = false;
        }
        else
        {
            const string content( valueStart, valueLength );
            const MatcherSet::MatchingRule rule( content, lineType );
            m_rules.push_back( rule );
        }
    }
}

// ConfigReceiver::ProcessColorsKey -----------------------------------------------------------

void ConfigReceiver::ProcessColorsKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
	assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );
	assert( m_currentSection == Colors );

    const unsigned int keyLength = keyEnd - keyStart;
    const unsigned int valueLength = valueEnd - valueStart;
    if ( 0 == keyLength )
    {
        cerr << "Error!  Empty key found in Colors section of config file!" << endl;
        m_valid = false;
    }
    else if ( 0 == valueLength )
    {
        cerr << "Error!  Empty value found in Colors section of config file!" << endl;
        m_valid = false;
    }
    else if ( ::strncmp( keyStart, "Back", keyLength ) == 0 )
        m_colorBackground.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Text", keyLength ) == 0 )
        m_colorText.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Pass", keyLength ) == 0 )
        m_colorPasses.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Warn", keyLength ) == 0 )
        m_colorWarnings.assign( valueStart, valueLength );
    else if ( ::strncmp( keyStart, "Fail", keyLength ) == 0 )
        m_colorErrors.assign( valueStart, valueLength );
    else
    {
        cerr << "Error!  Invalid key found in Colors section of config file!" << endl;
        m_valid = false;
    }
}

// ConfigReceiver::AddGlobalKey ---------------------------------------------------------------

bool ConfigReceiver::AddGlobalKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
    assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );

    string s( keyStart, keyEnd - keyStart );
    cerr << "Error!  Unrecognized key from ini file.  Ini file should not have global keys." << endl;
    cerr << "\t Key:   (" << s << ')' << endl;
    s.assign( valueStart, valueEnd - valueStart );
    cerr << "\t Value: (" << s << ')' << endl;
    m_valid = false;

    return false;
}

// ConfigReceiver::AddSection -----------------------------------------------------------------

bool ConfigReceiver::AddSection( const char * nameStart, const char * nameEnd )
{
    assert( nullptr != this );
    assert( nullptr != nameStart );
    assert( nullptr != nameEnd );
    assert( '\0' != *nameStart );
    assert( '\0' != *nameEnd );

    const unsigned int length = nameEnd - nameStart;
    if ( 0 == length )
    {
        cerr << "Error!  Section in config file has no name!" << endl;
        m_currentSection = None;
        m_valid = false;
    }

    else if ( ::strncmp( nameStart, "Input", length ) == 0 )
    {
        m_currentSection = Input;
        m_hasInputSection = true;
    }
    else if ( ::strncmp( nameStart, "Output", length ) == 0 )
    {
        m_currentSection = Output;
        m_hasOutputSection = true;
    }
    else if ( ::strncmp( nameStart, "Rules", length ) == 0 )
        m_currentSection = Rules;
    else if ( ::strncmp( nameStart, "Colors", length ) == 0 )
        m_currentSection = Colors;
    else
    {
        cerr << "Error!  Invalid name of section found in config file." << endl;
        m_currentSection = None;
        m_valid = false;
    }

    return m_valid;
}

// ConfigReceiver::AddSectionKey --------------------------------------------------------------

bool ConfigReceiver::AddSectionKey( const char * keyStart, const char * keyEnd,
    const char * valueStart, const char * valueEnd )
{
    assert( nullptr != this );
    assert( nullptr != keyStart );
    assert( nullptr != keyEnd );
    assert( nullptr != valueStart );
    assert( nullptr != valueEnd );
    assert( '\0' != *keyStart );
    assert( '\0' != *keyEnd );
    assert( '\0' != *valueStart );
    assert( '\0' != *valueEnd );

    switch ( m_currentSection )
    {
        case Input:
        {
            ProcessInputKey( keyStart, keyEnd, valueStart, valueEnd );
            break;
        }
        case Output:
        {
            ProcessOutputKey( keyStart, keyEnd, valueStart, valueEnd );
            break;
        }
        case Rules:
        {
            ProcessRulesKey( keyStart, keyEnd, valueStart, valueEnd );
            break;
        }
        case Colors:
        {
            ProcessColorsKey( keyStart, keyEnd, valueStart, valueEnd );
            break;
        }
        default:
        {
            cerr << "Error!  Invalid section found in config file." << endl;
            m_currentSection = None;
            m_valid = false;
            break;
        }
    }

    return m_valid;
}

// ConfigReceiver::ParsedConfigFile -----------------------------------------------------------

void ConfigReceiver::ParsedConfigFile( bool valid )
{
    assert( nullptr != this );
    if ( !valid )
    {
        cerr << "Error!  Unable to parse config file." << endl;
        m_valid = false;
    }
    if ( !m_hasInputSection || !m_hasOutputSection )
    {
        cerr << "Error!  Config file requires both an Input and Output section." << endl;
        m_valid = false;
    }
}

// ConfigReceiver::GiveParseMessage -----------------------------------------------------------

bool ConfigReceiver::GiveParseMessage( ::Parser::ErrorLevel::Levels level,
    const ::Parser::CharType * message )
{
    assert( nullptr != this );
    assert( nullptr != message );
    assert( '\0' != *message );

    if ( ErrorLevel::Minor <= level )
    {
        cerr << ErrorLevel::Name( level ) << ": " << message << "\n";
        m_valid = false;
    }
    return true;
}

// ConfigReceiver::GiveParseMessage -----------------------------------------------------------

bool ConfigReceiver::GiveParseMessage( ::Parser::ErrorLevel::Levels level,
    const ::Parser::CharType * message, unsigned long line )
{
    assert( nullptr != this );
    assert( nullptr != message );
    assert( '\0' != *message );

    if ( ErrorLevel::Minor <= level )
    {
        cerr << ErrorLevel::Name( level ) << ": Line: " << line << " " << message << "\n";
        m_valid = false;
    }
    return true;
}

// ConfigReceiver::GiveParseMessage -----------------------------------------------------------

bool ConfigReceiver::GiveParseMessage( ::Parser::ErrorLevel::Levels level,
    const ::Parser::CharType * message, const char * filename, unsigned long line )
{
    assert( nullptr != this );
    assert( nullptr != message );
    assert( nullptr != filename );
    assert( '\0' != *message );
    assert( '\0' != *filename );

    if ( ErrorLevel::Minor <= level )
    {
        cerr << ErrorLevel::Name( level ) << "   File: " << filename << " Line: " << line << " " << message << "\n";
        m_valid = false;
    }
    return true;
}

#endif

// --------------------------------------------------------------------------------------------

} // end namespace hestia
