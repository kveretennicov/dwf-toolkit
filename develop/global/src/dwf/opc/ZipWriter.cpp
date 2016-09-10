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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/ZipWriter.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/ZipWriter.h"

using namespace DWFToolkit;

    //
    // used for temporary buffer to transfer bytes
    // from a an input stream to the zip archive
    //
#define _DWFTK_OPC_ZIP_WRITER_STREAM_BUFFER_BYTES  16384


_DWFTK_API
OPCZipWriter::OPCZipWriter(DWFZipFileDescriptor *pZipFileDescr)
    throw()
    : _pZipFileDescr(pZipFileDescr)
{

}

_DWFTK_API
OPCZipWriter::~OPCZipWriter()
    throw()
{
    DWFCORE_FREE_OBJECT(_pZipFileDescr);
}

_DWFTK_API
void OPCZipWriter::open()
    throw(DWFException)
{
    if(_pZipFileDescr != NULL)
    {
        if(_pZipFileDescr->mode() == DWFZipFileDescriptor::eUnzip)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"This archive open for reading, not writing" );
        }

        _pZipFileDescr->open();
    }
}

_DWFTK_API
void OPCZipWriter::close()
    throw(DWFException)
{
    if(_pZipFileDescr != NULL)
    {
        _pZipFileDescr->close();
    }
}

_DWFTK_API
void OPCZipWriter::write( const DWFString &zName, DWFInputStream* pInpStream, DWFZipFileDescriptor::teFileMode eZipMode)
    throw(DWFException)
{
    if(_pZipFileDescr == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No archive open for writing" );
    }

    DWFString zItemName( zName );
    if (zName.substring(0,1) == /*NOXLATE*/L"/")
    {
        zItemName.assign( zName.substring(1) );
    }

    // Get a zip stream to write to
    DWFOutputStream* pZipStream = _pZipFileDescr->zip(zItemName, eZipMode);

    // copy the passed in stream to the zip stream
    size_t  nBytesRead = 0;
    char    aBuffer[_DWFTK_OPC_ZIP_WRITER_STREAM_BUFFER_BYTES];
    while (pInpStream->available() > 0)
    {
        nBytesRead = pInpStream->read( aBuffer, _DWFTK_OPC_ZIP_WRITER_STREAM_BUFFER_BYTES );
        pZipStream->write( aBuffer, nBytesRead );
    }

    //
    // release the file stream
    //
    pZipStream->flush();

    DWFCORE_FREE_OBJECT( pZipStream );
}


_DWFTK_API
void OPCZipWriter::write( const DWFString &zName, DWFInputStream* pInpStream )
    throw(DWFException)
{
    write( zName, pInpStream, _pZipFileDescr->mode() );
}
