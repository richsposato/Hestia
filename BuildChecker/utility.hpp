// $Header: /cvsroot/hestia/BuildChecker/utility.hpp,v 1.2 2008/11/20 23:19:31 rich_sposato Exp $

/// @file utility.hpp Defines utility classes and functions in BuildChecker console program.


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


#if !defined( HESTIA_ABC_UTILITY_HPP_INCLUDED )
#define HESTIA_ABC_UTILITY_HPP_INCLUDED


// --------------------------------------------------------------------------------------------

#include <string>


// define nullptr even though new compilers will have this keyword just so we
// have a consistent and easy way of identifying which uses of 0 mean null.
#if !defined( nullptr )
    #define nullptr 0
#endif

#define Nil '\0'


namespace hestia
{

// --------------------------------------------------------------------------------------------

/// Returns true if string is NULL or empty.
inline bool IsEmptyString( const char * s )
{
    return ( ( nullptr == s ) || ( Nil == *s ) );
}

/** Parses input to find starting character of next line.  This
 @return nullptr if input is invalid, else start of next line.
 */
const char * GetNextLine( const char * pc );

/// Non-const overloaded version of GetNextLine.
inline char * GetNextLine( char * pc )
{
    const char * pcc = pc;
    return const_cast< char * >( GetNextLine( pcc ) );
}

/** Finds first instance of a substring within a string.
 @param [input] full Pointer to containing string.
 @param [input] sub Pointer to substring which might be within main string.
 @return nullptr if parameters are invalid. First instance of substring if it exists
  within main string, else pointer to terminating Nil character.
 */
const char * FindSubString( const char * full, const char * sub );

/// Non-const overloaded version of FindSubString.
inline char * FindSubString( char * full, const char * sub )
{
    const char * p = full;
    return const_cast< char * >( FindSubString( p, sub ) );
}

/** Searches backwards through string to find starting character of a number.
 @param ss String to be searched.
 @return nullptr if string is empty or nullptr, else pointer to first digit in number.
 */
const char * BackupToNumber( const char * ss );

/// Non-const overloaded version of BackupToNumber.
inline char * BackupToNumber( char * ss )
{
    const char * p = ss;
    return const_cast< char * >( BackupToNumber( p ) );
}

/// Returns true if character is a path delimiter on either Windows or Linux.
inline bool IsPathDelimiter( char ch )
{
    return ( ':' == ch )
        || ( '/' == ch )
        || ( '\\' == ch );
}

/** Finds pointer to first char in filename at end of path.
 @param path Can be an absolute path, a relative path, or just the filename.
 @return Pointer to filename, or nullptr if parameter is invalid.
 */
const char * FindFileNameInPath( const char * path );

/// Non-const overloaded version of FindFileNameInPath.
inline char * FindFileNameInPath( char * path )
{
    const char * p = path;
    return const_cast< char * >( FindFileNameInPath( p ) );
}

/** Converts any embedded nil characters within string to a different char.
 @param ss string to be searched.
 @param ch Character used to replace embedded nils.
 @return True if any nils found.
 */
bool ChangeEmbeddedNils( ::std::string & ss, char ch = ' ' );

/** Reads contents of a file into a string.
 @param filename File to be read.
 @param target Place where files are stored.
 @return True for success, false for failure.
 */
bool ReadFileIntoString( const char * filename, ::std::string & target );

// --------------------------------------------------------------------------------------------

class Colors
{
public:
    enum ColorPurpose
    {
        Background = 0,
        Text       = 1,
        Passes     = 2,
        Warnings   = 3,
        Errors     = 4,
        Count
    };

    static Colors & GetIt( void );

    bool SetColor( ColorPurpose purpose, const char * color );

    const char * GetColor( ColorPurpose purpose ) const;

private:

    Colors( void );

    ~Colors( void );

    const char * m_colors[ Count ];

};

// --------------------------------------------------------------------------------------------

} // end namespace hestia

#endif

// $Log: utility.hpp,v $
// Revision 1.2  2008/11/20 23:19:31  rich_sposato
// Added ability for user to specify colors on the command line.
//
// Revision 1.1  2008/11/17 21:00:14  rich_sposato
// Moved some classes and functions from main.cpp to a separate files.
//
