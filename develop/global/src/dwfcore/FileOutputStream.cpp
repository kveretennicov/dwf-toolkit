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

#include "dwfcore/FileOutputStream.h"
using namespace DWFCore;


_DWFCORE_API
DWFFileOutputStream::DWFFileOutputStream()
throw()
        : _bOwner( false )
        , _pFileDescriptor( NULL )
{
    ;
}

_DWFCORE_API
DWFFileOutputStream::~DWFFileOutputStream()
throw()
{
    if (_bOwner && (_pFileDescriptor != NULL))
    {
        DWFCORE_FREE_OBJECT( _pFileDescriptor );
    }
}

_DWFCORE_API
void
DWFFileOutputStream::attach( DWFFileDescriptor* pFileDescriptor,
                             bool               bOwnDescriptor  )
throw( DWFException )
{
    if (pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file provided" );
    }

    _pFileDescriptor = pFileDescriptor;
    _bOwner = bOwnDescriptor;
}

_DWFCORE_API
void
DWFFileOutputStream::detach()
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    if (_bOwner)
    {
        DWFCORE_FREE_OBJECT( _pFileDescriptor );
    }

    _pFileDescriptor = NULL;
    _bOwner = false;
}

_DWFCORE_API
size_t
DWFFileOutputStream::write( const void*   pBuffer,
                            size_t        nBytesToWrite )
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    return _pFileDescriptor->write( pBuffer, nBytesToWrite );
}

_DWFCORE_API
void
DWFFileOutputStream::flush()
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    _pFileDescriptor->flush();
}

