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

// $Header: /cvsroot/hestia/Dump/Dump.cpp,v 1.2 2008/05/19 06:17:00 rich_sposato Exp $


// @file Dump.cpp : Implementation file for Dump facility.

// ----------------------------------------------------------------------------

#include "Dump.hpp"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <windows.h>
#include <dbghelp.h>


// ----------------------------------------------------------------------------

using namespace ::std;

#ifndef nullptr
    #define nullptr NULL
#endif

// function pointer typedef based on dbghelp.h
typedef BOOL ( WINAPI * MINIDUMPWRITEDUMP )(
	HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);


namespace
{

// ----------------------------------------------------------------------------

class DumpInfo
{
public:

	static DumpInfo & GetIt( void );

	bool Setup( const char * appName, const char * appFileName, const char * directory,
		const char * keys[], const char * values[], unsigned int keyCount );

	void MakeInfoFile( const char * message );

    inline const char * GetAppName( void ) const { return m_appName.c_str(); }
    inline const char * GetDumpFileName( void ) const { return m_dumpFileName.c_str(); }
    inline const char * GetInfoFileName( void ) const { return m_infoFileName.c_str(); }
    inline const char * GetAppFileName( void ) const { return m_appFileName.c_str(); }
	inline bool IsSetup( void ) const { return m_setup; }
	inline bool IsDone( void ) const { return m_done; }
	inline bool IsBusy( void ) const { return m_busy; }

	inline void SetBusy( bool busy ) { m_busy = busy; }

private:

	struct KeyValueInfo
	{
		::std::string m_key;
		::std::string m_value;
	};
	typedef ::std::vector< KeyValueInfo > BuildInfo;
	typedef BuildInfo::iterator BuildInfoIter;
	typedef BuildInfo::const_iterator BuildInfoCIter;

	static LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS * pExceptionInfo );

	bool MakeDumpFiles( struct _EXCEPTION_POINTERS * pExceptionInfo );

	bool SetupFileNames( const char * appName, const char * directory );

	bool SetupBuildInfo( const char * keys[], const char * values[], unsigned int keyCount );

    LPTOP_LEVEL_EXCEPTION_FILTER m_prevFilter;
	::std::string m_appName;
    ::std::string m_dumpFileName;
    ::std::string m_infoFileName;
    ::std::string m_appFileName;
	BuildInfo m_buildInfo;
    bool m_setup;
	bool m_busy;
	bool m_done;

    DumpInfo( void );

    ~DumpInfo( void );

};

// ----------------------------------------------------------------------------

DumpInfo & DumpInfo::GetIt( void )
{
	static DumpInfo info;
	return info;
}

// ----------------------------------------------------------------------------

DumpInfo::DumpInfo( void ) :
    m_prevFilter(),
    m_appName(),
	m_dumpFileName(),
	m_infoFileName(),
    m_appFileName(),
	m_buildInfo(),
	m_setup( false ),
	m_busy( false ),
	m_done( false )
{
	assert( nullptr != this );
}

// ----------------------------------------------------------------------------

DumpInfo::~DumpInfo( void )
{
	assert( nullptr != this );
}

// ----------------------------------------------------------------------------

void DumpInfo::MakeInfoFile( const char * message )
{
	assert( nullptr != this );
	if ( m_done )
		return;

	if ( NULL == message )
		message = "User chose not to make a dump file.";
	ofstream file( m_infoFileName.c_str(), ios_base::out );
    file << "Application Name: " << m_appName << endl;
    file << "Dump File Name: " << m_dumpFileName << endl;
    file << "App File Name: " << m_appFileName << endl;
	file << "Dead Thread: " << ::GetCurrentThreadId() << endl << endl;

	BuildInfoCIter cend( m_buildInfo.end() );
	for ( BuildInfoCIter cit = m_buildInfo.begin(); cit != cend; ++cit )
	{
		const KeyValueInfo & kvInfo = *cit;
		assert( kvInfo.m_key.size() != 0 );
		file << kvInfo.m_key << ":\t" << kvInfo.m_value << endl;
	}

	file << endl << "Message: " << endl << message << endl;
	m_done = true;
}

// ----------------------------------------------------------------------------

bool LoadDebugLibrary( HMODULE & hDll )
{

	static const char * s_debugDll = "DBGHELP.DLL";
	char dllPath[ _MAX_PATH + 256 ];

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	if ( ::GetModuleFileName( NULL, dllPath, _MAX_PATH ))
	{
        char * pSlash = ::strchr( dllPath, '\\' );
		if ( NULL != pSlash )
		{
            ::strcpy( pSlash+1, s_debugDll );
			hDll = ::LoadLibrary( dllPath );
		}
	}

	if ( NULL == hDll )
	{
		// load any version we can
		hDll = ::LoadLibrary( s_debugDll );
	}

	return ( NULL != hDll );
}

// ----------------------------------------------------------------------------

bool DumpInfo::MakeDumpFiles( struct _EXCEPTION_POINTERS * pExceptionInfo )
{
	assert( NULL != this );

	bool madeDumpFile = false;
	const char * message = NULL;
	char buffer[ 2 * _MAX_PATH + 2048 ];
	HMODULE hDll = NULL;
	const MINIDUMP_TYPE dumpType = static_cast< MINIDUMP_TYPE >( MiniDumpWithDataSegs | MiniDumpWithHandleData
		| MiniDumpFilterMemory | MiniDumpScanMemory | MiniDumpWithoutOptionalData | MiniDumpWithFullMemory );
	const bool loaded = LoadDebugLibrary( hDll );

	if ( loaded )
	{
		assert( NULL != hDll );
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if ( NULL != pDump )
		{

			// ask the user if they want to save a dump file
			::_snprintf( buffer, sizeof(buffer), "The program %s crashed.\n"
				"Would you like to create a file to diagnose the cause of the crash?\n"
				"The diagnostic file will help improve the software.", GetAppName() );
			const int answer = ::MessageBox( NULL, buffer, GetAppName(), MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL );
			if ( IDYES == answer )
			{

				// create the file
				HANDLE hFile = ::CreateFile( GetDumpFileName(), GENERIC_WRITE, FILE_SHARE_WRITE,
					NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				if ( hFile != INVALID_HANDLE_VALUE )
				{

					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = FALSE;
					const HANDLE currentProcess = ::GetCurrentProcess();
					const DWORD currentProcessId = ::GetCurrentProcessId();

					// write the dump
					BOOL okay = pDump( currentProcess, currentProcessId, hFile, dumpType, &ExInfo, NULL, NULL );
					if ( okay )
					{
						::_snprintf( buffer, sizeof(buffer),
							"Saved diagnostic info to files:\n"
							"%s\n%s\n"
							"Please send both files to customer support.",
							GetInfoFileName(), GetDumpFileName() );
						message = buffer;
						madeDumpFile = true;
					}
					else
					{
						const DWORD lastError = ::GetLastError();
						if ( ( ERROR_INVALID_PARAMETER & lastError ) != 0x0 )
						{
							::_snprintf( buffer, sizeof(buffer),
								"Failed to save diagnostic file because of error %u due to invalid parameter.\n"
								"Please send this file to customer support.\n%s",
								lastError, GetInfoFileName() );
						}
						else
						{
							::_snprintf( buffer, sizeof(buffer),
								"Failed to save diagnostic file because of error %u due to invalid parameter.\n"
								"Please send this file to customer support.\n%s",
								lastError, GetInfoFileName() );
						}
						message = buffer;
					}
					::CloseHandle( hFile );
				}
				else
				{
                    ::_snprintf( buffer, sizeof(buffer),
                        "Failed to create diagnostic file because of error %u.\n"
						"Please send this file to customer support.\n%s",
						::GetLastError(), GetInfoFileName() );
					message = buffer;
				}
			}
		}
		else
		{
            ::_snprintf( buffer, sizeof(buffer),
			    "The DbgHelp.dll file is too old to save dump files.\n"
				"Please send this file to customer support.\n%s",
				GetInfoFileName() );
			message = buffer;
		}
	}
	else
	{
        ::_snprintf( buffer, sizeof(buffer),
			"Could not find DbgHelp.dll to save dump file.\n"
			"Please send this file to customer support.\n%s",
			GetInfoFileName() );
		message = buffer;
	}

    MakeInfoFile( message );
	if ( NULL != message )
		::MessageBox( NULL, message, GetAppName(), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL );

	return madeDumpFile;
}

// ----------------------------------------------------------------------------

LONG WINAPI DumpInfo::TopLevelFilter( struct _EXCEPTION_POINTERS * pExceptionInfo )
{

	LONG result = EXCEPTION_CONTINUE_SEARCH;
	DumpInfo & info = DumpInfo::GetIt();
	if ( info.IsDone() || info.IsBusy() )
		return result;
	try
	{
		info.SetBusy( true );
		if ( info.MakeDumpFiles( pExceptionInfo ) )
			result = EXCEPTION_EXECUTE_HANDLER;
	}
	catch ( ... )
	{
		const char * message = "Unable to save crash information to diagnostic file!";
		info.MakeInfoFile( message );
		::MessageBox( NULL, message, info.GetAppName(), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );
	}
	info.SetBusy( false );

	return result;
}

// ----------------------------------------------------------------------------

bool MakeDirectoryIfNeeded( const char * directory )
{
	if ( ( NULL == directory ) || ( '\0' == *directory ) )
		return false;

	const unsigned int length = ::strlen( directory );
	assert( 0 < length );
	const char lastChar = directory[ length-1 ];
	const bool endsWithSlash = ( ( '\\' == lastChar ) || ( '/' == lastChar ) );
	string temp;
	if ( endsWithSlash )
	{
		temp.assign( directory );
		temp.erase( temp.size() - 1, 1 );
		directory = temp.c_str();
	}

	WIN32_FIND_DATA findInfo;
	memset( &findInfo, 0, sizeof(findInfo) );
	HANDLE findHandle = ::FindFirstFile( directory, &findInfo );
	if ( INVALID_HANDLE_VALUE != findHandle )
	{
		const bool isDirectory = ( 0 != ( findInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) );
		if ( !isDirectory )
			return false;
	}
	else
	{
		// If the directory does not exist, then create it.
		const BOOL created = ::CreateDirectory( directory, NULL );
		if ( created == FALSE )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------------

bool IsIllegalFilenameChar( const unsigned char ch )
{
	// Check for unprintable chars.
	if ( ( ch < 0x20 ) || ( 0x7F <= ch ) )
		return true;
	// Check for chars with special meanings inside paths, filenames, and command line options.
	if ( ( '*' == ch ) || ( '/' == ch ) || ( '\\' == ch ) 
	  || ( '<' == ch ) || ( '>' == ch ) || ( '\"' == ch )
	  || ( '?' == ch ) || ( ':' == ch ) || ( '|'  == ch ) )
		return true;
	return false;
}

// ----------------------------------------------------------------------------

void CheckForBadFilenameChars( char * s )
{
	if ( ( NULL == s ) || ( '\0' == *s ) )
		return;
	while ( '\0' != *s )
	{
		if ( IsIllegalFilenameChar( *s ) )
			*s = '_';
		++s;
	}
}

// ----------------------------------------------------------------------------

bool DumpInfo::SetupFileNames( const char * appName, const char * directory )
{
	assert( nullptr != this );

	char dumpFileName[ 2000 ];
	char currentDir[ 2000 ];
    SYSTEMTIME stLocalTime;
	::GetLocalTime( &stLocalTime );
	::_snprintf( dumpFileName, sizeof(dumpFileName), "%s_%04d%02d%02d_%02d%02d%02d_%ld_%ld.dmp",
		appName, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
		::GetCurrentProcessId(), ::GetCurrentThreadId() );
	CheckForBadFilenameChars( dumpFileName );

	const unsigned int nameLength = ::strlen( dumpFileName );
	for ( unsigned int ii = 0; ii < nameLength; ++ii )
	{
		if ( ' ' == dumpFileName[ ii ] )
			dumpFileName[ ii ] = '_';
	}

	const bool gotDirectory = ( NULL != directory ) && ( '\0' != *directory );
	bool useCurrentDirectory = !gotDirectory;
	if ( gotDirectory )
	{
		if ( !MakeDirectoryIfNeeded( directory ) )
			useCurrentDirectory = true;
	}
	if ( useCurrentDirectory )
	{
		const DWORD bytesWritten = ::GetCurrentDirectory( sizeof(currentDir)-1, currentDir );
		if ( 0 == bytesWritten )
		{
			return false;
		}
		directory = currentDir;
	}
	m_dumpFileName.reserve( ::strlen( directory ) + 2 + nameLength );
	m_dumpFileName = directory;

	const unsigned int length = m_dumpFileName.size();
	assert( 0 < length );
	const char lastChar = m_dumpFileName[ length-1 ];
	const bool endsWithSlash = ( ( '\\' == lastChar ) || ( '/' == lastChar ) );
	if ( !endsWithSlash )
	{
		m_dumpFileName.append( "\\" );
	}

	m_dumpFileName.append( dumpFileName );
	m_infoFileName.reserve( m_dumpFileName.size() + 5 );
	m_infoFileName.append( m_dumpFileName );
	m_infoFileName.append( ".txt" );

	return true;
}

// ----------------------------------------------------------------------------

bool DumpInfo::SetupBuildInfo( const char * keys[], const char * values[], unsigned int keyCount )
{
	assert( nullptr != this );

	if ( ( NULL == keys ) || ( NULL == values ) || ( 0 == keyCount ) )
		return true;

	KeyValueInfo kvInfo;
	m_buildInfo.reserve( keyCount );
	for ( unsigned int ii = 0; ii < keyCount; ++ii )
	{
		const char * key = *keys;
		if ( ( NULL == key ) || ( '\0' == *key ) )
			return ( ii == keyCount - 1 );
		kvInfo.m_key = key;
		const char * value = *values;
		if ( ( NULL == value ) || ( '\0' == *value ) )
			value = " ";
		kvInfo.m_value = value;
		m_buildInfo.push_back( kvInfo );
		++keys;
		++values;
	}

	return true;
}

// ----------------------------------------------------------------------------

bool DumpInfo::Setup( const char * appName, const char * appFileName, const char * directory,
	const char * keys[], const char * values[], unsigned int keyCount )
{
	assert( nullptr != this );

	if ( !SetupFileNames( appName, directory ) )
		return false;

	if ( !SetupBuildInfo( keys, values, keyCount ) )
		return false;

    m_appName = appName;
    m_appFileName = appFileName;
	m_prevFilter = ::SetUnhandledExceptionFilter( &DumpInfo::TopLevelFilter );

	return true;
}

// ----------------------------------------------------------------------------

}

namespace hestia
{

// ----------------------------------------------------------------------------

bool SetupForDump( const char * appName, const char * appFileName, const char * directory,
    const char * keys[], const char * values[], unsigned int keyCount )
{

	bool okay = false;
	try
	{
		if ( ( NULL == appName       ) || ( '\0' == appName       )
		  || ( NULL == appFileName   ) || ( '\0' == appFileName   ) )
		{
			return false;
		}
		DumpInfo & info = DumpInfo::GetIt();
		if ( info.IsSetup() ) {
			return true;
		}
		okay = info.Setup( appName, appFileName, directory, keys, values, keyCount );
	}
	catch ( ... )
	{
		okay = false;
	}
	return okay;
}

// ----------------------------------------------------------------------------

}; // end namespace hestia

// $Log: Dump.cpp,v $
// Revision 1.2  2008/05/19 06:17:00  rich_sposato
// Added copyright notice.  Added CVS keywords.  Add comments.
//
