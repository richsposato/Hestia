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

// $Header$


// ----------------------------------------------------------------------------

#include "../Dump.hpp"

#include <string.h>
#include <exception>

#define QUOTE_ME_HELPER(x) #x
#define QUOTE_ME(x) QUOTE_ME_HELPER(x)


// ----------------------------------------------------------------------------

int main( const unsigned int argc, const char * const argv[] )
{

	const char * appName = "TestDump";
	const char * exeName = argv[0];
	const char * directory = "C:/tmp";

#if defined( _DEBUG ) || defined( DEBUG )
	const char * buildType = "Debug";
#else
	const char * buildType = "Release";
#endif

#if defined( RELEASE_FOR_CUSTOMER )
	const char * releaseType = "Release";
#elif defined( INTERNAL_RELEASE )
	const char * releaseType = "Internal";
#elif defined( SQA_TEST_RELEASE )
	const char * releaseType = "SQA Test Version";
#else
	const char * releaseType = "Other";
#endif

	const char * keys[] =
	{
		"Version",
		"BuildType",
		"ReleaseType",
		"BuildDate",
		"BuildTime",
		NULL
	};
	const char * values[] =
	{
		"3.0",
		buildType,
        releaseType,
		__DATE__,
		__TIME__,
		NULL
	};
	const unsigned int keyCount = sizeof( keys ) / sizeof( keys[0] ) - 1;
	const bool canDoDump = ::hestia::SetupForDump( appName, exeName, directory, keys, values, keyCount );

    /** This code tries to invoke the Dump facility by doing an illegal action (i.e. - copy to nullptr)
     or by throwing an exception which will not get caught.  These are not the kinds of actions the
     author of this library recommends for production code.
     */
	if ( canDoDump )
	{
		if ( 1 < argc )
		{
			char * p = NULL;
			::strcat( p, argv[1] );
		}
		throw ::std::exception( "Dead code" );
	}

	return 0;
}

// ----------------------------------------------------------------------------

// $Log$
