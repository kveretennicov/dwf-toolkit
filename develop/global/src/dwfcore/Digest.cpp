//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

#include "dwfcore/Digest.h"
#include "dwfcore/digest/md5.h"
#include "dwfcore/digest/sha1.h"

using namespace DWFCore;

namespace DWFCore
{

//
// Wrapper declared here since md5_byte_t could not be forward
// declared in Digest.h
//
struct _DWFMD5Digest_t {
    md5_byte_t _oDigest[16];
};

}


_DWFCORE_API
DWFMD5Digest::DWFMD5Digest()
throw()
            : _pState( NULL )
{;}

_DWFCORE_API
DWFMD5Digest::~DWFMD5Digest()
throw()
{
    if (_pState)
    {
        char* pChar = (char*)_pState;
        DWFCORE_FREE_MEMORY( pChar );
        _pState = NULL;
    }
}

_DWFCORE_API 
void
DWFMD5Digest::update( const char* pBuffer, size_t nBytes )
throw( DWFException )
{
    // If this is the first call to update, then the state needs to be created
    if (_pState == NULL)
    {
        //
        //  Create and initialize the state that stores the digest computation
        //
        _pState = (md5_state_s*) DWFCORE_ALLOC_MEMORY( char, sizeof(md5_state_s) );

        if (_pState)
        {
            md5_init( _pState );
        }
        else
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate the digest state." );
        }
    }

    md5_append( _pState, (const md5_byte_t*)pBuffer, (int)nBytes );
}

_DWFCORE_API 
bool
DWFMD5Digest::isProgressive()
throw()
{
    return true;
}

void
DWFMD5Digest::_computeDigest( DWFCore::_DWFMD5Digest_t& oDigest )
throw( DWFException )
{
    // The state of the digest should not be NULL
    if (_pState == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The state must be initialized with at least one update call before calling digest." );
    }

    //
    //  Copy the current state info
    //
    md5_state_s* pCurrState = (md5_state_s*) DWFCORE_ALLOC_MEMORY( char, sizeof(md5_state_s) );
    if (pCurrState == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate memory for the current digest state." );
    }
    DWFCORE_COPY_MEMORY( pCurrState, _pState, sizeof(md5_state_s) );

    //
    //  Perform finish on the current state info
    //
    md5_finish( pCurrState, oDigest._oDigest );

    //
    //  Done with the current state - delete it
    //
    if (pCurrState)
    {
        char* pChar = (char*)pCurrState;
        DWFCORE_FREE_MEMORY( pChar );
        pCurrState = NULL;
    }
}

_DWFCORE_API 
DWFString
DWFMD5Digest::digest()
throw( DWFException )
{
    _DWFMD5Digest_t oDigest;
    _computeDigest( oDigest );

    //  The MD5 digest returns a 16 byte array that needs to be converted to hex
    wchar_t zHex[16*2 + 1] = {0};
    int i=0;
    for (; i<16; ++i)
    {
        _DWFCORE_SWPRINTF( zHex + 2*i, 2 + 1, /*NOXLATE*/L"%02x", oDigest._oDigest[i] );
    }

    return DWFString( zHex );
}

_DWFCORE_API
size_t
DWFMD5Digest::digestRawBytes( unsigned char*& rpBytes )
throw( DWFException )
{
    _DWFMD5Digest_t oDigest;
    _computeDigest( oDigest );

    //  The MD5 digest returns a 16 byte array
    rpBytes = DWFCORE_ALLOC_MEMORY( unsigned char, 16 );
    if (rpBytes == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate memory for a copy of the raw bytes of the digest." );
    }
    DWFCORE_COPY_MEMORY( rpBytes, oDigest._oDigest, 16 );

    return 16;
}

_DWFCORE_API
DWFString
DWFMD5Digest::digestBase64()
throw( DWFException )
{
    _DWFMD5Digest_t oDigest;
    _computeDigest( oDigest );

    //  The MD5 digest returns a 16 byte array that needs to be converted to Base64
    char encodedBase64[16*6] = {0};
    size_t nBytes = DWFString::EncodeBase64(
                        oDigest._oDigest, 16,
                        encodedBase64, 16*6,
                        true
                    );

    return DWFString(encodedBase64, nBytes);
}

_DWFCORE_API 
void
DWFMD5Digest::reset()
throw()
{
    //
    //  Delete the old state
    //
    if (_pState)
    {
        char* pChar = (char*)_pState;
        DWFCORE_FREE_MEMORY( pChar );
        _pState = NULL;
    }

    //
    //  Create and initialize a new state that stores the digest computation
    //
    _pState = (md5_state_s*) DWFCORE_ALLOC_MEMORY( char, sizeof(md5_state_s) );
    md5_init( _pState );
}

////////

_DWFCORE_API
DWFSHA1Digest::DWFSHA1Digest()
throw()
            : _pState( NULL )
{;}

_DWFCORE_API
DWFSHA1Digest::~DWFSHA1Digest()
throw()
{
    if (_pState)
    {
        char* pChar = (char*)_pState;
        DWFCORE_FREE_MEMORY( pChar );
        _pState = NULL;
    }
}

_DWFCORE_API 
void
DWFSHA1Digest::update( const char* pBuffer, size_t nBytes )
throw( DWFException )
{
    // If this is the first call to update, then the state needs to be created
    if (_pState == NULL)
    {
        //
        //  Create and initialize the state that stores the digest computation
        //
        _pState = (SHA1Context*) DWFCORE_ALLOC_MEMORY( char, sizeof(SHA1Context) );

        if (_pState)
        {
            SHA1Reset( _pState );
        }
        else
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate the digest state." );
        }
    }

    SHA1Input( _pState, (const unsigned char*)pBuffer, (unsigned int)nBytes );
}

_DWFCORE_API 
bool
DWFSHA1Digest::isProgressive()
throw()
{
    return true;
}

void
DWFSHA1Digest::_computeDigest( unsigned int oDigest[5] )
throw( DWFException )
{
    // The state of the digest should not be NULL
    if (_pState == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The state must be initialized with at least one update call before calling digest." );
    }

    //
    //  Copy the current state info
    //
    SHA1Context* pCurrState = (SHA1Context*) DWFCORE_ALLOC_MEMORY( char, sizeof(SHA1Context) );
    if (pCurrState == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate memory for the current digest state." );
    }
    DWFCORE_COPY_MEMORY( pCurrState, _pState, sizeof(SHA1Context) );

    if (!SHA1Result( pCurrState ))
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to compute the SHA1 digest." );
    }
    DWFCORE_COPY_MEMORY( oDigest, pCurrState->Message_Digest, 20 );

    //
    //  Done with the current state - delete it
    //
    if (pCurrState)
    {
        char* pChar = (char*)pCurrState;
        DWFCORE_FREE_MEMORY( pChar );
        pCurrState = NULL;
    }
}

_DWFCORE_API 
DWFString
DWFSHA1Digest::digest()
throw( DWFException )
{
    unsigned int oDigest[5];
    _computeDigest( oDigest );

    // The SHA1 digest returns an array of 5 unsigned ints (20 bytes)
    // that needs to be converted to a hex string
    wchar_t zHex[20*2 + 1] = {0};
    _DWFCORE_SWPRINTF( zHex, 20*2+1, /*NOXLATE*/L"%08x%08x%08x%08x%08x", oDigest[0], oDigest[1], oDigest[2], oDigest[3], oDigest[4] );
    return DWFString( zHex );
}

_DWFCORE_API
size_t
DWFSHA1Digest::digestRawBytes( unsigned char*& rpBytes )
throw( DWFException )
{
    unsigned int oDigest[5];
    _computeDigest( oDigest );

    // The SHA1 digest returns an array of 5 unsigned ints (20 bytes)
    rpBytes = DWFCORE_ALLOC_MEMORY( unsigned char, 20 );
    if (rpBytes == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Could not allocate memory for a copy of the raw bytes of the digest." );
    }
    DWFCORE_COPY_MEMORY( rpBytes, oDigest, 20 );

    return 20;
}

_DWFCORE_API
DWFString
DWFSHA1Digest::digestBase64()
throw( DWFException )
{
    unsigned int oDigest[5];
    _computeDigest( oDigest );

    // The SHA1 digest returns an array of 5 unsigned ints (20 bytes)
    // that needs to be converted to Base64
    char encodedBase64[20*6] = {0};
    size_t nBytes = DWFString::EncodeBase64(
                        oDigest, 20,
                        encodedBase64, 20*6,
                        true
                    );

    return DWFString(encodedBase64, nBytes);
}

_DWFCORE_API 
void
DWFSHA1Digest::reset()
throw()
{
    //
    //  Delete the old state
    //
    if (_pState)
    {
        char* pChar = (char*)_pState;
        DWFCORE_FREE_MEMORY( pChar );
        _pState = NULL;
    }

    //
    //  Create and initialize a new state that stores the digest computation
    //
    _pState = (SHA1Context*) DWFCORE_ALLOC_MEMORY( char, sizeof(SHA1Context) );
    SHA1Reset( _pState );
}
