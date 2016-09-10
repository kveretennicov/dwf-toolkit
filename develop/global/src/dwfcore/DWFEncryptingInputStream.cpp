//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
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


#include "dwfcore/DWFEncryptingInputStream.h"
#include "dwfcore/zip/zip.h"
#include "dwfcore/zip/password.h"
using namespace DWFCore;


_DWFCORE_API
DWFEncryptingInputStream::DWFEncryptingInputStream( DWFInputStream* pInputStream,
                                                    const DWFString& rPassword )
throw()
                    : _pInputStream( pInputStream )
{
	char* pPwd = 0;
    rPassword.getUTF8( &pPwd );
    init_keys( pPwd, _aKeys );
    DWFCORE_FREE_MEMORY( pPwd );
}

_DWFCORE_API
DWFEncryptingInputStream::~DWFEncryptingInputStream()
throw()
{
	DWFCORE_FREE_OBJECT( _pInputStream );
}

_DWFCORE_API
size_t 
DWFEncryptingInputStream::available() const
    throw( DWFException )
{
    return (_pInputStream == NULL) ? 0 : _pInputStream->available();
}

_DWFCORE_API
size_t 
DWFEncryptingInputStream::read( void*   pBuffer,
                                size_t  nBytesToRead )
throw( DWFException )
{
	if (_pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An input stream has not been linked to the Encrypting stream." );
    }

	size_t n = _pInputStream->read( pBuffer, nBytesToRead );
	encrypt_buffer( (char*)pBuffer, (unsigned long)n, _aKeys );

	return n;
}

_DWFCORE_API
off_t
DWFEncryptingInputStream::seek( int     eOrigin,
                                off_t   nOffset )
throw( DWFException )
{
    if (_pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An input stream has not been linked to the Encrypting stream." );
    }

    return _pInputStream->seek( eOrigin, nOffset );
}


