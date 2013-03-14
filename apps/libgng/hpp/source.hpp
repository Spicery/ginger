/******************************************************************************\
    Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
    of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef GINGER_SOURCE_HPP
#define GINGER_SOURCE_HPP

#include <deque>
#include <fstream>
#include <iostream>

namespace Ginger {

class Source {
public:
    virtual ~Source() {}
public:
    virtual char next() = 0;
    virtual bool isValid() = 0;
    virtual char current() = 0;
    virtual void advance() = 0;
    virtual int peek( const unsigned int look_ahead ) = 0;
};

class PeekableAbsSource : public Source {
private:
    std::deque< int > already_read_buffer;

public:
    PeekableAbsSource() {}

    virtual ~PeekableAbsSource() {}

protected:
    virtual int getOneChar() = 0;

    int currentChar() {
        if ( this->already_read_buffer.empty() ) {
            const int ich = this->getOneChar();
            //  or push_front - choose push_back so that the common
            //  case has balanced push_back and pop_backs.
            this->already_read_buffer.push_back( ich );   
            return ich;
        } else {
            return this->already_read_buffer.back();
        }
    }

public:
    int peek( const unsigned int look_ahead ) {
        //  We know that push_back tends to perform better than
        //  push_front for deque. This is the unusual case so we
        //  don't care it is a bit slower.
        while ( this->already_read_buffer.size() <= look_ahead ) {
            const int ich = this->getOneChar();
            this->already_read_buffer.push_front( ich );
        }
        const ptrdiff_t d = this->already_read_buffer.size() - look_ahead;
        return this->already_read_buffer.at( d - 1 );
    }

    char next() {
        char ch = this->current();
        this->advance();
        return ch; 
    }

    bool isValid() {
        return this->currentChar() != EOF; 
    }

    char current() {
        return static_cast< char >( this->currentChar() );
    }

    void advance() { 
        this->already_read_buffer.pop_back();
    }
};

class IStreamSource : public PeekableAbsSource {
private:
    std::istream & input;
public:
    IStreamSource( std::istream & _input ) : input( _input ) {}
    virtual ~IStreamSource() {}
protected:
    int getOneChar() {
        return this->input.get();
    }
};

class FilePtrSource : public PeekableAbsSource {
private:
    FILE * fd;
    bool close_on_exit;

public:
    FilePtrSource( FILE * ptr, const bool coe = false ) :
        fd( ptr ),
        close_on_exit( coe )
    {
    }

    virtual ~FilePtrSource() {
        if ( this->fd != NULL && this->close_on_exit ) {
            fclose( this->fd );
            this->fd = NULL;
        }
    }

protected:
    int getOneChar() {
        return fgetc( this->fd );
    }

};


class FileDescriptorSource : public FilePtrSource {
private:
    FILE * fd;
    bool close_on_exit;

public:
    FileDescriptorSource( const int fd, const bool coe = false ) :
        FilePtrSource( fdopen( fd, "r" ), coe )
    {
    }

    virtual ~FileDescriptorSource() {
    }

};

class FileSource : public FilePtrSource {
public:
    FileSource( const std::string fname ) :
        FilePtrSource( fopen( fname.c_str(), "r" ), true )
    {
    }

    virtual ~FileSource() {}
};



} // namespace

#endif