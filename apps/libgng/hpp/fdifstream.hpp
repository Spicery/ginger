#ifndef SAFE_IFSTREAM_HPP
#define SAFE_IFSTREAM_HPP

#include <istream>

class FileDescriptorIFStreamBuf : 
    public std::streambuf
{
private:
    void initSetG( const int n_putback = 0, const int n_read = 0 );
    char * position();

protected:
    static const int lookbehind_size = 16;
    static const int buffer_size = 512 + lookbehind_size;
    char buffer[buffer_size];

    virtual int_type underflow();
    virtual pos_type seekoff( off_type, std::ios_base::seekdir, std::ios_base::openmode );
    virtual pos_type seekpos( pos_type, std::ios_base::openmode );

public:
    FileDescriptorIFStreamBuf( const int f );
    int fd;
};

//  This class is exploited in an initialisation idiom. By making it
//  a base class of FileDescriptorIFStream, the FileDescriptorIFStreamBuf
//  can be guaranteed to be initialised in time to be used by the
//  constructor of std::istream.
//  
class FileDescriptorIFStreamBase
{
protected:
    FileDescriptorIFStreamBuf buf;

public:
    FileDescriptorIFStreamBase( const int fd );
};

//  This class inherits from FileDescriptorIFStreamBase so that the
//  protected field buf is initialised before the constructor runs.
class FileDescriptorIFStream : 
    protected FileDescriptorIFStreamBase, 
    public std::istream 
{
private:
    const bool needs_close;

public:
    explicit FileDescriptorIFStream( const int fd );
    ~FileDescriptorIFStream();
};

#endif
