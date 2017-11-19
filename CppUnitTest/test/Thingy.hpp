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

// $Header: /cvsroot/hestia/CppUnitTest/test/Thingy.hpp,v 1.4 2007/04/25 20:40:35 rich_sposato Exp $


// ----------------------------------------------------------------------------

#ifndef THINGY_H_INCLUDED
#define THINGY_H_INCLUDED

#include <exception>

// ----------------------------------------------------------------------------

class Stuff : public ::std::exception
{
public:

    inline Stuff( void ) : exception(), m_message( NULL ) {}

    inline Stuff( const char * message )
        : exception(), m_message( message ) {}

    inline Stuff( const Stuff & that )
        : exception( that ), m_message( that.m_message ) {}

    inline virtual ~Stuff( void ) throw() {}

    inline Stuff & operator = ( const Stuff & that )
    {
        m_message = that.m_message;
        return *this;
    }

    virtual const char * what( void ) const throw() { return m_message; }

private:

    const char * m_message;
};

// ----------------------------------------------------------------------------

class Thingy
{
public:

    Thingy( void ) : m_size( 0 ) {}

    Thingy( const Thingy & that ) : m_size( that.m_size ) {}

    explicit Thingy( int size ) : m_size( size ) {}

    ~Thingy( void ) {}

    Thingy & operator = ( const Thingy & that )
    {
        m_size = that.m_size;
        return *this;
    }

    bool operator == ( const Thingy & that ) const
    {
        return ( m_size == that.m_size );
    }

    bool operator != ( const Thingy & that ) const
    {
        return ( m_size != that.m_size );
    }

    int GetSize( void ) const { return m_size; }

    void SetSize( int size ) { m_size = size; }

    bool IsZero( void ) const { return ( m_size == 0 ); }

    bool ThrowsBadly( bool doThrow ) const
    {
        if ( doThrow )
            throw ::std::exception();
        return true;
    }

    bool ThrowsBadly( bool doThrow, bool stuff ) const
    {
        if ( doThrow )
        {
            if ( stuff )
                throw Stuff( "Message from within exception." );
            else
                throw ::std::exception();
        }
        return true;
    }

private:
    int m_size;
};

// ----------------------------------------------------------------------------

#endif

// $Log: Thingy.hpp,v $
// Revision 1.4  2007/04/25 20:40:35  rich_sposato
// Minor change so it compiles without errors on GCC.
//
// Revision 1.3  2007/04/25 19:16:25  rich_sposato
// Added exception to demonstrate unit test macros with multiple catch blocks.
//
// Revision 1.2  2007/03/07 22:56:30  rich_sposato
// Added copyright notices and keywords.
//
