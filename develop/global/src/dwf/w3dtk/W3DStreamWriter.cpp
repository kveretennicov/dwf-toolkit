//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
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

#include "dwf/w3dtk/W3DStreamWriter.h"


W3DStreamWriter::W3DStreamWriter( DWFOutputStream&      rW3DStream,
                                  BStreamFileToolkit&   rToolkit )
throw()
               : _rStream( rW3DStream )
               , _rToolkit( rToolkit )
               , _bOpen( false )
               , _nRequiredVersion( DWFW3D_STREAM_WRITER_EARLIEST_VERSION )
#ifndef DWFW3D_STREAM_WRITER_USE_STACK_BUFFER
               , _pBuffer( NULL )
#endif
{
}

W3DStreamWriter::~W3DStreamWriter()
throw()
{
    if (_bOpen)
    {
        try { W3DStreamWriter::close(); } catch (...) {;}
    }
}

void
W3DStreamWriter::open( unsigned int nRequestedVersion )
throw( DWFException )
{
#ifndef DWFW3D_STREAM_WRITER_USE_STACK_BUFFER

    _pBuffer = DWFCORE_ALLOC_MEMORY( char, DWFW3D_STREAM_WRITER_BUFFER_BYTES );

    if (_pBuffer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate internal buffer" );
    }

#endif

    //
    // required first step (pre-writing)
    //
    _rToolkit.PrepareBuffer( _pBuffer, DWFW3D_STREAM_WRITER_BUFFER_BYTES );

        //
        // if applicable, tell the hsf streamer to constrain its output format
        // also, don't bother making the call if the request is for a version
        // higher than (or equal to) the current one (w3dtk/BStream.h)
        //
    if ((nRequestedVersion > 0) && (nRequestedVersion <= TK_File_Format_Version))
    {
        _rToolkit.SetTargetVersion( nRequestedVersion );
    }

    _bOpen = true;
}

unsigned int
W3DStreamWriter::close()
throw( DWFException )
{
        //
        // flush anything remaining in the toolkit buffer
        //
    if (_pBuffer)
    {
        //
        // final write
        //
        int nTKBufferBytes = _rToolkit.CurrentBufferLength();
        if (nTKBufferBytes > 0)
        {
            _rStream.write( _pBuffer, nTKBufferBytes );
            _rToolkit.PrepareBuffer( _pBuffer, DWFW3D_STREAM_WRITER_BUFFER_BYTES );
        }

        //
        // must always flush the stream!!!
        //
        _rStream.flush();

        //
        // cleanup
        //
#ifndef DWFW3D_STREAM_WRITER_USE_STACK_BUFFER

        DWFCORE_FREE_MEMORY( _pBuffer );
        _pBuffer = NULL;
#endif

    }

    _bOpen = false;

    return ((_nRequiredVersion < (unsigned int)_rToolkit.GetTargetVersion()) ? _nRequiredVersion : 0);
}

void
W3DStreamWriter::notify( BBaseOpcodeHandler* pHandler,
                         const void*         pTag )
throw( DWFException )
{
    try
    {
        //
        // ask the toolkit to write out the object; 
        // the toolkit will write out as much as the current buffer can hold
        //
        TK_Status eStatus = pHandler->Write( _rToolkit );

            //
            // if the opcode requires a stream version greater than the
            // current setting, adjust the requirement here
            //
        if (pHandler->version() > _nRequiredVersion)
        {
            _nRequiredVersion = pHandler->version();
        }

            //
            // if there is still more data left to write; we have to loop through 
            // generating and writing buffers of data until we're done
            //
        while (eStatus == TK_Pending) 
        {
                //
                // due to compression, preparing a new buffer might immediately fill that buffer
                // so we need an extra check to possibly export that buffer and prepare a new buffer
                //
            while (eStatus  == TK_Pending) 
            {
                //
                // write the buffer to the stream -
                // ask the toolkit how much of the buffer is currently filled
                // this will normally be the same as the buffer size that we passed in
                //
                _rStream.write( _pBuffer, _rToolkit.CurrentBufferLength() );

                //
                // tell the toolkit where it can resume writing data; in this case, we
                // can reuse the same buffer once we're done with it
                //
                eStatus = _rToolkit.PrepareBuffer( _pBuffer, DWFW3D_STREAM_WRITER_BUFFER_BYTES );
            }

            //
            // ask the toolkit to continue trying to write out the object's data
            //
            eStatus = pHandler->Write( _rToolkit );
        }
    }
    catch (...)
    {
        //
        // if a streaming exception occurs, i think the handler
        // should still be reset in the case that the caller
        // wants to try and recover from the error. if so, we
        // can have old data laying around...
        //
        pHandler->Reset();

        throw;
    }

        //
        // tag requested (for segments & shells only)
        //
    if (pTag)
    {
        //
        // key and tag
        //
        _rToolkit.SetLastKey( (ID_Key)pTag );
        pHandler->Tag( _rToolkit );
    }

    //
    // free up the handler for reuse
    //
    pHandler->Reset();
}

