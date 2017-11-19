/* ----------------------------------------------------------------------------

Dump  (a part of the Hestia project)
http://sourceforge.net/projects/hestia/

Copyright (c) 2009 by Rich Sposato

Permission to use, copy, modify, distribute and sell this software for any
purpose is hereby granted without fee, provided that the above copyright notice
appear in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

The author makes no representations about the suitability of this software for
any purpose. It is provided "as is" without express or implied warranty.

---------------------------------------------------------------------------- */

// $Header: /cvsroot/hestia/Dump/Dump.hpp,v 1.2 2008/05/19 06:17:00 rich_sposato Exp $


// ----------------------------------------------------------------------------

#ifndef HESTIA_DUMP_HPP_INCLUDED
#define HESTIA_DUMP_HPP_INCLUDED

#if defined( _MSC_VER ) && ( _MSC_VER >= 1000 )
  #pragma once
#endif

#if !defined( NULL )
	#define NULL 0
	#define HESTIA_DUMP_DECLARED_NULL
#endif

namespace hestia
{

// ----------------------------------------------------------------------------

/** Sets up a program so that any uncaught exception causes the program to
 save the current state to a dump file for later debugging.  It also makes an
 info file which stores some of the parameters passed into this function.
 @param appName Name of application.
 @param appFileName Name of executable file for application.
 @param directory Place to put dump file.  If this parameter is NULL, the file
  goes into the current directory.
 @param keys An array of pointers to strings containing keys for outputting to info file.
  If this parameter is NULL, the values and keyCount parameters should also be NULL and 0.
  The keys could contain strings like "BuildType", "BuildDate", "BuildNumber", "ProjectName", etc...
 @param values An array of pointers to strings containing keys for outputting to info file.
  Each value corresponds to exactly one key in the previous array of strings.  If this parameter
  is NULL, the keys and keyCount parameters should also be NULL and 0.  The values can contain
  strings like "Debug", "2009-05-28 07:18:21", "1.3.207", etc...
 @param keyCount Number of keys and values.
 */
bool SetupForDump( const char * appName, const char * appFileName, const char * directory = NULL,
    const char * keys[] = NULL, const char * values[] = NULL, unsigned int keyCount = 0 );

/**
@par Intent
This utility can help developers quickly find hard to duplicate crashing bugs.
The kind of bugs that only show up at customer sites or during sales demos.

@par Usage
Change your program to call SetupForDump shortly after the program starts up.
The examples code snippets below show various ways to call SetupForDump.
You will not need to make any other calls after that.  If the program crashes
because of an uncaught exception, this will ask the user whether to save the
program's current state to a dump file.  The user can send the info and dump
files to you.  The info file will help identify exactly which build and version
the customer used when the crash occurred.  You can then use the dump file - and the
map and pdb files from that build - with Visual Studio's debugger to determine what
line caused the crash and why.  If you have the map and pdb files, the debugger will
show you functions on the stack of each thread and values for local data members.
If this can't save the program state to a dump file, it will place an error message
into the info file explaining why.

@code
	// Code Snippet 1 shows how to provide keys and values for the info file.
	{
		const CWinApp * app = ::AfxGetApp();
		const char * appName = ::AfxGetAppName();
		const char * exeName = app->m_pszExeName;
		const char * directory = "C:/MyCompanyDir/ExtraFiles";
		const char * keys[] =
		{
			"Version",
			"BuildType",
			"BuildDate",
			"BuildNumber",
			"ReleaseType",
			NULL
		};
		const char * values[] =
		{
			::MyCompany::GetVersion(),
			::MyCompany::GetBuildType(),
			::MyCompany::GetBuildDate(),
			::MyCompany::GetBuildNumber(),
			::MyCompany::GetReleaseType(),
			NULL
		};
		const unsigned int keyCount = sizeof( keys ) / sizeof( keys[0] ) - 1;
		const bool canDoDump = ::hestia::SetupForDump( appName, exeName, directory, keys, values, keyCount );
	}
@endcode

@code
	// Code Snippet 2 shows a minimalist way to setup the Dump facility.
	{
		const CWinApp * app = ::AfxGetApp();
		const char * appName = ::AfxGetAppName();
		const char * exeName = app->m_pszExeName;
		const bool canDoDump = ::hestia::SetupForDump( appName, exeName, "c:/tmp"  );
	}
@endcode

@code
	// Code Snippet 3 shows an even more minimalist way to setup the Dump facility.
	{
		const bool canDoDump = ::hestia::SetupForDump( "SomeApp", argv[0] );
	}
@endcode

@par What You Need To Do
 - Include this header file.
 - Change your program to call SetupForDump when your program starts up.
 - Compile the Dump.cpp file into a library, or compile with your project.
 - Distribute with DbgHelp.dll version 5.1 or higher. (provided by Microsoft)
 - When compiling and linking, make both the pdb and map files and save them.  Do
   not give the map and pdb files to customers so customers do not learn proprietary
   internal details of the application.  Just store them for when a dump file comes
   back from a customer.
 - Send your program to customers.
*/


// ----------------------------------------------------------------------------

}; // end namespace hestia

#if defined( HESTIA_DUMP_DECLARED_NULL )
	#undef HESTIA_DUMP_DECLARED_NULL
    #undef NULL
#endif

#endif // end file guardian

// $Log: Dump.hpp,v $
// Revision 1.2  2008/05/19 06:17:00  rich_sposato
// Added copyright notice.  Added CVS keywords.  Add comments.
//
