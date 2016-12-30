#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <unistd.h>

#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define MD5_Init CC_MD5_Init
#  define MD5_Update CC_MD5_Update
#  define MD5_Final CC_MD5_Final
#else
#include <openssl/md5.h>
#  include <openssl/sha.h>
#endif

#include "common.hpp"
#include "machine.hpp"
#include "cell.hpp"
#include "sys.hpp"

namespace Ginger {
using namespace std;

class MD5Digester {
private:
    MD5_CTX c;
    char buf[512];
    
public:
    MD5Digester() {
        MD5_Init( &c );
    }

    void update() {
        ssize_t bytes;
    
        bytes = read( STDIN_FILENO, buf, 512 ) ;
        while( bytes > 0 ) {
            MD5_Update( &c, buf, bytes );
            bytes = read( STDIN_FILENO, buf, 512 );
        }
    }

    void update( const void * p, int n ) {
        MD5_Update( &c, p, n );
    }

    void update( const std::string & str ) {
        MD5_Update( &c, str.c_str(), str.size() );
    }

    std::string digest() {
        unsigned char out[ MD5_DIGEST_LENGTH ];
        MD5_Final( out, &c );
        std::stringstream out_stream;
        out_stream.setf( std::ios::hex, std::ios::basefield );
        out_stream.fill( '0' );
        for( int n = 0; n < MD5_DIGEST_LENGTH; n++ ) {
            out_stream << std::setw( 2 ) << static_cast< int >( out[ n ] );
        }  
        return out_stream.str();
    }

};


//  Arity: 2 -> 0
Ref * sysMD5( Ref * pc, class MachineClass * vm ) {
    if ( vm->count != 1 ) throw Ginger::Mishap( "ArgsMismatch" );
    StringObject input = Cell( vm->fastPop() ).asStringObject();
    
    MD5Digester digester;
    digester.update( input.getCharPtr(), input.length() );

    vm->fastPush( vm->heap().copyString( pc, digester.digest().c_str() ) );
    
    return pc;
}
SysInfo infoSysMD5( 
    FullName( "sysMD5" ), 
    Ginger::Arity( 1 ), 
    Ginger::Arity( 1 ), 
    sysMD5, 
    "Computes the MD5 hash of a string."
);

} // namespace Ginger
