//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/ZipFileReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/ZipFileReader.h"
#include "dwfcore/UnzippingInputStream.h"
using namespace DWFToolkit;


_DWFTK_API
OPCZipFileReader::OPCZipFileReader(DWFZipFileDescriptor *pZipFileDescr)
    throw()
    : _pZipFileDescr(pZipFileDescr)
    , _bOwnZipFileDescriptor( false )
{
    _pZipFileDescr->own( *this );
    _bOwnZipFileDescriptor = true;
}

_DWFTK_API
OPCZipFileReader::~OPCZipFileReader()
    throw()
{
    if (_bOwnZipFileDescriptor)
    {
        DWFCORE_FREE_OBJECT(_pZipFileDescr);
    }
}

_DWFTK_API
void OPCZipFileReader::open()
    throw(DWFException)
{
    if(_pZipFileDescr != NULL)
    {
        if(_pZipFileDescr->mode() != DWFZipFileDescriptor::eUnzip)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"This archive not open for reading." );
        }

        _pZipFileDescr->open();
    }
}

_DWFTK_API
void OPCZipFileReader::close()
    throw(DWFException)
{
    if(_pZipFileDescr != NULL)
    {
        _pZipFileDescr->close();
    }
}

_DWFTK_API
DWFInputStream*
OPCZipFileReader::read( const DWFString& zName )
    throw(DWFException)
{
    return read( zName, false );
}

_DWFTK_API
 DWFInputStream* OPCZipFileReader::read( const DWFString& zName, bool bAutoDestroy )
    throw(DWFException)
{
    if(_pZipFileDescr == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No archive open for writing" );
    }

#if 0
    DWFString zItemName( zName );
    if (zName.substring(0,1) == /*NOXLATE*/L"/")
    {
        zItemName.assign( zName.substring(1) );
    }
#endif

    //
    // Check for it first before opening. Otherwise we throw an exception.
    //
    if(!_pZipFileDescr->locate(zName))
    {
        return NULL;
    }

    DWFUnzippingInputStream* pUnzipStream = (DWFUnzippingInputStream*)_pZipFileDescr->unzip(zName);
    pUnzipStream->attach( _pZipFileDescr, bAutoDestroy );
    return pUnzipStream;
}

_DWFTK_API
void OPCZipFileReader::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
    throw( DWFException )
{
    _bOwnZipFileDescriptor = false;
}

_DWFTK_API
void OPCZipFileReader::notifyOwnableDeletion( DWFOwnable& /*rOwnable*/ )
    throw( DWFException )
{
    delete this;
}
