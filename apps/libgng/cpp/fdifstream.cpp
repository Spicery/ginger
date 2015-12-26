#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <cstring>
#include <errno.h>
#include <exception>

#include "mishap.hpp"
#include "fdifstream.hpp"


char * FileDescriptorIFStreamBuf::position() {
    return this->buffer + this->lookbehind_size;
}

void FileDescriptorIFStreamBuf::initSetG( const int n_putback, const int n_read ) {
    this->setg( this->position() - n_putback, this->position(), this->position() + n_read );
}

FileDescriptorIFStreamBuf::FileDescriptorIFStreamBuf( const int f ) :
    fd( f )
{
    this->initSetG();
    underflow();
}

FileDescriptorIFStreamBuf::int_type FileDescriptorIFStreamBuf::underflow() 
{
    if ( this->gptr() < this->egptr() ) return traits_type::to_int_type( *this->gptr() );

    int n_putback = this->gptr() - this->eback();
    if ( n_putback > this->lookbehind_size ) {
        n_putback = this->lookbehind_size;
    }

    std::memmove( this->position() - n_putback, this->gptr() - n_putback, n_putback );

    const int n_read = read( this->fd, this->position(), buffer_size - lookbehind_size );
    if ( -1 == n_read ) {
        throw Ginger::Mishap( "Error reading from file descriptor" );
    } else if ( 0 == n_read ) {
        return traits_type::eof();
    }

    this->initSetG( n_putback, n_read );
    return traits_type::to_int_type( *this->gptr() );
}

FileDescriptorIFStreamBuf::pos_type 
FileDescriptorIFStreamBuf::seekoff( off_type off, std::ios_base::seekdir dir, std::ios_base::openmode )
{
    off_t result;
    if ( dir == std::ios_base::beg ) {
        result = lseek( this->fd, off, SEEK_SET );
    } else if ( dir == std::ios_base::cur ) {
        result = lseek( this->fd, off, SEEK_CUR );
    } else if ( dir == std::ios_base::end ) {
        result = lseek( this->fd, off, SEEK_END );
    } else {
        throw Ginger::Mishap( "Got bad std::ios_base::seekdir" );
    }

    if ( -1 == result ) {
        throw Ginger::Mishap( "Error seeking to offset with file descriptor" );
    }

    this->initSetG();
    return result;
}

FileDescriptorIFStreamBuf::pos_type
FileDescriptorIFStreamBuf::seekpos( pos_type p, std::ios_base::openmode )
{
    off_t result( lseek( fd, p, SEEK_SET ) );
    if ( -1 == result ) {
        throw Ginger::Mishap( "Error seeking with file descriptor" );
    }

    this->initSetG();
    return p;
}

FileDescriptorIFStreamBase::FileDescriptorIFStreamBase( const int f ) :
    buf( f )
{
}

FileDescriptorIFStream::FileDescriptorIFStream( const int _fd ) :
    FileDescriptorIFStreamBase( _fd ),
    std::istream( &buf ),
    needs_close( false )
{
}

FileDescriptorIFStream::~FileDescriptorIFStream()
{
    if ( needs_close ) {
        ::close( this->buf.fd );
    }
}


