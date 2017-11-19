// $Header: /cvsroot/hestia/BuildChecker/utility.cpp,v 1.3 2008/11/21 00:35:13 rich_sposato Exp $

/// @file utility.cpp Utility classes and functions for BuildChecker console program.

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

#include "utility.hpp"

#include <assert.h>
#include <fstream>
#include <iterator>


using namespace std;

namespace hestia
{


// GetNextLine --------------------------------------------------------------------------------

const char * GetNextLine( const char * pc )
{

    if ( IsEmptyString( pc ) )
        return nullptr;

    const char * next_cr = ::strchr( pc, '\r' );
    const char * next_lf = ::strchr( pc, '\n' );
    if ( ( nullptr == next_lf ) && ( nullptr == next_cr ) )
        // If there are no more end-of-line characters, this must be
        // the last line, so just point to end of string.
        return pc + ::strlen( pc );

    const char * nextLine =
        ( nullptr == next_lf ) ? next_cr :
        ( nullptr == next_cr ) ? next_lf : min( next_cr, next_lf );
    if ( next_cr+1 == next_lf ) nextLine = next_lf;
    ++nextLine;

    return nextLine;
}

// FindSubString ------------------------------------------------------------------------------

const char * FindSubString( const char * full, const char * sub )
{
    if ( IsEmptyString( full ) )
        return nullptr;
    if ( IsEmptyString( sub ) )
        return nullptr;

    const unsigned int size = static_cast< unsigned int >( ::strlen( sub ) );
    const char * place = full;
    while ( *place != Nil )
    {
        if ( ::strncmp( place, sub, size ) == 0 )
            break;
        ++place;
    }

    return place;
}

// BackupToNumber -----------------------------------------------------------------------------

const char * BackupToNumber( const char * ss )
{
    if ( IsEmptyString( ss ) )
        return nullptr;

    const char * pp = ss;
    while ( ( Nil != *pp ) && ( 0 == ::isdigit( *pp ) ) )
        --pp;
    if ( Nil == *pp )
        return nullptr;

    while ( ( Nil != *pp ) && ( 0 != ::isdigit( *pp ) ) )
        --pp;
    if ( Nil == *pp )
        return nullptr;

    return ++pp;
}

// FindFileNameInPath -------------------------------------------------------------------------

const char * FindFileNameInPath( const char * path )
{

    if ( IsEmptyString( path ) )
        return nullptr;
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

// ChangeEmbeddedNils -------------------------------------------------------------------------

bool ChangeEmbeddedNils( string & ss, char ch )
{

    if ( ch == Nil )
        return false;
    bool found = false;
    string::iterator here( ss.begin() );
    string::iterator last( ss.end() );
    while ( here != last )
    {
        if ( Nil == *here )
        {
            found = true;
            *here = ch;
        }
        ++here;
    }

    return found;
}

// ReadFileIntoString -------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------------------

Colors & Colors::GetIt( void )
{
    static Colors colors;
    return colors;
}

// --------------------------------------------------------------------------------------------

Colors::Colors( void ) :
    m_colors()
{
    assert( nullptr != this );
    m_colors[ Background ] = "black";
    m_colors[ Text       ] = "white";
    m_colors[ Passes     ] = "green";
    m_colors[ Warnings   ] = "yellow";
    m_colors[ Errors     ] = "red";
}

// --------------------------------------------------------------------------------------------

Colors::~Colors( void )
{
    assert( nullptr != this );
}

// --------------------------------------------------------------------------------------------

bool Colors::SetColor( ColorPurpose purpose, const char * color )
{
    assert( nullptr != this );
    if ( IsEmptyString( color ) )
        return false;
    if ( ( purpose < Background ) || ( Count <= purpose ) )
        return false;
    m_colors[ purpose ] = color;
    return true;
}

// --------------------------------------------------------------------------------------------

const char * Colors::GetColor( ColorPurpose purpose ) const
{
    assert( nullptr != this );
    if ( ( purpose < Background ) || ( Count <= purpose ) )
        return nullptr;
    return m_colors[ purpose ];
}

// --------------------------------------------------------------------------------------------

} // end namespace hestia

// $Log: utility.cpp,v $
// Revision 1.3  2008/11/21 00:35:13  rich_sposato
// Minor changes.
//
// Revision 1.2  2008/11/20 23:19:31  rich_sposato
// Added ability for user to specify colors on the command line.
//
// Revision 1.1  2008/11/17 21:00:14  rich_sposato
// Moved some classes and functions from main.cpp to a separate files.
//
