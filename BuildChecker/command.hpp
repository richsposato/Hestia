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

// $Header: /cvsroot/hestia/BuildChecker/command.hpp,v 1.1 2008/12/11 20:50:24 rich_sposato Exp $

/// @file command.hpp Defines functions and class used for command line args.


#if !defined( HESTIA_ABC_COMMAND_HPP_INCLUDED )
#define HESTIA_ABC_COMMAND_HPP_INCLUDED


// --------------------------------------------------------------------------------------------

#include "matchers.hpp"

namespace hestia
{


// --------------------------------------------------------------------------------------------

/** Shows message on how to use this program.
 @param exeName Name of executable file.
 */
void ShowUsageMessage( const char * exeName );

class CommandLineArgs
{
public:

    /** Parses command line for parameters.
     @param argc [input] Number of command line args.
     @param argv [input] Point to command line args.
     */
	CommandLineArgs( unsigned int argc, const char * const argv[] );

	inline virtual ~CommandLineArgs( void ) {}

	inline bool IsValid( void ) const { return m_valid; }

	inline bool ShowHelp( void ) const { return m_showHelp; }

	inline const char * GetExeName( void ) const { return m_exeName; }

	inline const char * GetInputFile( void ) const { return m_inputFile; }

	inline const char * GetOutputFile( void ) const { return m_outputFile; }

	inline const char * GetBuildName( void ) const { return m_buildName; }

	inline const char * GetBuildType( void ) const { return m_buildType; }

	inline const char * GetCompiler( void ) const { return m_compiler; }

	inline const char * GetTitle( void ) const { return m_title; }

    inline const char * GetColorBackground( void ) const { return m_colorBackground; }

    inline const char * GetColorText( void ) const { return m_colorText; }

    inline const char * GetColorPasses( void ) const { return m_colorPasses; }

    inline const char * GetColorWarnings( void ) const { return m_colorWarnings; }

    inline const char * GetColorErrors( void ) const { return m_colorErrors; }

	inline unsigned int GetKeepCount( void ) const { return m_keepCount; }

    inline const MatcherSet::MatchingRules GetMatchingRules( void ) const { return m_rules; }

private:
    /// Copy-constructor is not implemented.
	CommandLineArgs( const CommandLineArgs & );
    /// Copy-assignment operator is not implemented.
	CommandLineArgs & operator = ( const CommandLineArgs & );

	bool GetInfoFromConfigFile( const char * filename );

    /// True if command line options are valid.
	bool m_valid;
	/// True if user wants to see help info.
	bool m_showHelp;
	/// Name of this executable.
	const char * m_exeName;
    /// Pointer to input filename.
	const char * m_inputFile;
    /// Pointer to output directory.
	const char * m_outputFile;
    /// Pointer to name of build.
    const char * m_buildName;
    /// Pointer to type of build.
    const char * m_buildType;
    /// Name of compiler which made output messages.
    const char * m_compiler;
    /// Title shown on main page.
    const char * m_title;
    /// Background color of html pages.
    const char * m_colorBackground;
    /// Color of plain text on html pages.
    const char * m_colorText;
    /// Color of passing projects on html pages.
    const char * m_colorPasses;
    /// Color of warning messages on html pages.
    const char * m_colorWarnings;
    /// Color of error messages on html pages.
    const char * m_colorErrors;
    /// Number of older records to keep.
    unsigned int m_keepCount;
    /// Additional matching rules specified by user.
    MatcherSet::MatchingRules m_rules;
};

// --------------------------------------------------------------------------------------------

} // end namespace hestia

#endif

// $Log: command.hpp,v $
// Revision 1.1  2008/12/11 20:50:24  rich_sposato
// Moved command-line arg parsing class to separate file.  Added ability to
// get args from a config file instead of just command line.
//
