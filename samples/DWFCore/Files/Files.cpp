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

// Files.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef  _DWFCORE_WIN32_SYSTEM
#include <crtdbg.h>
#endif

using namespace std;
using namespace DWFCore;


#define DUMP( s )       wcout << s << endl

int main()
{
#ifdef  _DWFCORE_WIN32_SYSTEM
    //
    // Enable memory leak reporting in Debug mode under Win32.
    //
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );

    // For mem leak debugging... Please do not delete.
    //long foo = 7221;
    //_CrtSetBreakAlloc(foo);
#endif

#define DATA_BUFFER_BYTES   4096

        DWFCore::DWFString zDataString( L"Autodesk, Inc. is engaged in the development and marketing of design and drafting software and multimedia tools, primarily for the business and professional environment. For the 3 months ended 4/30/04, net revenues rose 41% to $297.9M. Net income totalled $42.5M, up from $7.5M. Revenues reflect increased upgrade revenues and higher subscription bookings. Net income also reflects improved gross margins." );

        unsigned char* pDataBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, DATA_BUFFER_BYTES );
        if (pDataBuffer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"No memory for data buffer" );
        }

            //
            // fill up random buffer
            //
        unsigned int i = 0;
        uint64_t nData64 = _DWFCORE_LARGE_CONSTANT( 0U );
        for (; i < DATA_BUFFER_BYTES;)
        {
             nData64 = DWFTimer::Tick64();
             DWFCORE_COPY_MEMORY( (pDataBuffer+i), &nData64, sizeof(nData64) );
             i += sizeof(nData64);
        }

        //
        // create a text file
        //
        DWFCore::DWFFile oTextFilename( L"Files.txt" );
        DWFCore::DWFStreamFileDescriptor oTextFile( oTextFilename, L"w+" );
        oTextFile.open();

        //
        // create a binary file
        //
        DWFCore::DWFFile oBinFilename( L"Files.bin" );
        DWFCore::DWFStreamFileDescriptor* pBinFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oBinFilename, L"w+b") );
        if (pBinFile == NULL)
        {
            _DWFCORE_THROW( DWFCore::DWFMemoryException, L"No memory for file descriptor" );
        }
        pBinFile->open();

            //
            // see note with binary file descriptor below
            //
        {
            //
            // create a stream to write to the files
            //
            DWFCore::DWFFileOutputStream oFilestream;

            //
            // first, attach the descriptor for the text file to the stream
            // and write the text buffer to the file
            //
            oFilestream.attach( &oTextFile, false );
            oFilestream.write( (const wchar_t*)zDataString, zDataString.bytes() );
            oFilestream.detach();

            //
            // close the file
            //
            oTextFile.close();

            DUMP( L"Text file created." );

            //
            // second, attach the descriptor for the binary file to the stream
            // we can reuse the file stream; also note we pass ownership of the descriptor
            // so the stream can control the deletion of the object. we do this for illustration,
            // notice the use of oFilestream in it's own scope block for this reason
            //
            oFilestream.attach( pBinFile, true );

            //
            // let's buffer the file i/o...
            // we should get 2 writes
            //
            DWFCore::DWFBufferOutputStream oBufferedStream( &oFilestream, false, DATA_BUFFER_BYTES/2 );

            //
            // for this example, let's assume we buffered the fil i/o because we've
            // got some crappy writing code...
            //
            for (i = 0; i < DATA_BUFFER_BYTES; i += 4)
            {
                oBufferedStream.write( (pDataBuffer+i), 4 );
            }

            //
            // it's always a good idea to explicitly flush your output streams
            //
            oBufferedStream.flush();

            //
            // since the stream owns the descriptor,
            // detach is going to do all the clean up (and close the file)
            //
            oFilestream.detach();

            DUMP( L"Binary file created." );
        }

        //
        // create 2 temporary files
        // they will be automatically deleted when we are done
        //
        DWFCore::DWFString zTemplate( L"Files_" );
        DWFCore::DWFTempFile* pTempTextFile = DWFTempFile::Create( zTemplate, true );

        zTemplate.assign( L"Files_" );
        DWFCore::DWFTempFile* pTempBinFile = DWFTempFile::Create( zTemplate, true );

        //
        // let's open our sample files
        //
        DWFCore::DWFStreamFileDescriptor oTextFile2( oTextFilename, L"r" );
        DWFCore::DWFStreamFileDescriptor oBinFile2( oBinFilename, L"rb" );
        oTextFile2.open();
        oBinFile2.open();

        //
        // create some streams to read from the files
        //
        DWFCore::DWFFileInputStream oTextFilestream;
        DWFCore::DWFFileInputStream oBinFilestream;

        oTextFilestream.attach( &oTextFile2, false );
        oBinFilestream.attach( &oBinFile2, false );

        size_t nBytes = 0;

        {
            //
            // copy the files into the temps
            //
            DWFCore::DWFFileOutputStream& rTempFilestream = pTempTextFile->getOutputStream();

            while (oTextFilestream.available() > 0)
            {
                nBytes = oTextFilestream.read( pDataBuffer, DATA_BUFFER_BYTES );

                rTempFilestream.write( pDataBuffer, nBytes );
            }

            //
            // clean up
            //
            rTempFilestream.flush();

            DUMP( L"Copied text file into temporary file" );
        }
        {
            DWFCore::DWFFileOutputStream& rTempFilestream = pTempBinFile->getOutputStream();

            while (oBinFilestream.available() > 0)
            {
                nBytes = oBinFilestream.read( pDataBuffer, DATA_BUFFER_BYTES );

                rTempFilestream.write( pDataBuffer, nBytes );
            }

            //
            // clean up
            //
            rTempFilestream.flush();

            DUMP( L"Copied binary file into temporary file" );
        }

        //
        // let's create a ZIP file and archive all these files
        //
        DWFCore::DWFFile oZipFilename( L"FilesArchive.zip" );
        DWFCore::DWFZipFileDescriptor oZipFile( oZipFilename, DWFZipFileDescriptor::eZip );

        //
        // open the archive
        //
        oZipFile.open();

        DUMP( L"Created Zip file" );

        //
        // add the original text file (with a password, and non-pkzip compliant passwording (aka salting) )
        //
        DWFCore::DWFOutputStream* pZipStream = oZipFile.zip( oTextFilename.name(), L"a password", false );

        //
        // the filestream is still open, let's rewind it
        //
        oTextFilestream.seek( SEEK_SET, 0 );

            //
            // add to zip
            //
        while (oTextFilestream.available() > 0)
        {
            nBytes = oTextFilestream.read( pDataBuffer, DATA_BUFFER_BYTES );

            pZipStream->write( pDataBuffer, nBytes );
        }

        //
        // clean up
        //
        pZipStream->flush();
        DWFCORE_FREE_OBJECT( pZipStream );

        DUMP( L"Original text file added to Zip" );

        //
        // done with this file
        //
        oTextFilestream.detach();
        oTextFile2.close();

        //
        // the binary file is still open but for kicks open it again
        //
        DWFCore::DWFStreamFileDescriptor oBinFile3( oBinFilename, "rb" );
        oBinFile3.open();

        //
        // auto-detach old descriptor
        //
        oBinFilestream.attach( &oBinFile3, false );

        //
        // add the original binary file (with a password, with pkzip compliant passwording )
        //
        pZipStream = oZipFile.zip( oBinFilename.name(), L"a password", true );

            //
            // add to zip
            //
        while (oBinFilestream.available() > 0)
        {
            nBytes = oBinFilestream.read( pDataBuffer, DATA_BUFFER_BYTES );

            pZipStream->write( pDataBuffer, nBytes );
        }

        //
        // clean up
        //
        pZipStream->flush();
        DWFCORE_FREE_OBJECT( pZipStream );

        DUMP( L"Original binary file added to Zip" );

        //
        // close all open handles
        //
        oBinFilestream.detach();
        oBinFile3.close();
        oBinFile2.close();

        //
        // add our temp files
        //
        DWFCore::DWFFileInputStream* pTempInputStream = pTempTextFile->getInputStream();

        DWFCore::DWFString zTextTempName( pTempInputStream->descriptor()->file().name() );
                           zTextTempName.append( ".txt" );
        pZipStream = oZipFile.zip( zTextTempName );

            //
            // add to zip
            //
        while (pTempInputStream->available() > 0)
        {
            nBytes = pTempInputStream->read( pDataBuffer, DATA_BUFFER_BYTES );

            pZipStream->write( pDataBuffer, nBytes );
        }

        //
        // clean up
        //
        pZipStream->flush();
        DWFCORE_FREE_OBJECT( pZipStream );
        DWFCORE_FREE_OBJECT( pTempInputStream );

        DUMP( L"Temporary text file added to Zip" );

        pTempInputStream = pTempBinFile->getInputStream();

        DWFCore::DWFString zBinTempName( pTempInputStream->descriptor()->file().name() );
                          zBinTempName.append( ".bin" );
        pZipStream = oZipFile.zip( zBinTempName );

            //
            // add to zip
            //
        while (pTempInputStream->available() > 0)
        {
            nBytes = pTempInputStream->read( pDataBuffer, DATA_BUFFER_BYTES );

            pZipStream->write( pDataBuffer, nBytes );
        }

        //
        // clean up
        //
        pZipStream->flush();
        DWFCORE_FREE_OBJECT( pZipStream );
        DWFCORE_FREE_OBJECT( pTempInputStream );

        DUMP( L"Temporary binary file added to Zip" );

        //
        // finish (and delete) the temp files
        //
        DWFCORE_FREE_OBJECT( pTempBinFile );
        DWFCORE_FREE_OBJECT( pTempTextFile );

        DUMP( L"Temporary files destroyed" );

        oZipFile.close();

        DUMP( L"Zip file closed" );

        //
        // finally, let's reopen the zip, open each file and simultaneously
        // stream the contents to an in memory buffer
        //
        DWFCore::DWFZipFileDescriptor oUnzipFile( oZipFilename, DWFZipFileDescriptor::eUnzip );
        oUnzipFile.open();

        //
        // auto-resizing buffer
        //
        DWFCore::DWFBufferOutputStream oMemoryStream( DATA_BUFFER_BYTES );

            //
            // original text file
            //
        {
            // with a password, and salting.
            DWFCore::DWFInputStream* pUnzipStream = oUnzipFile.unzip( oTextFilename.name(), L"a password" );
            DWFCore::DWFMonitoredInputStream oMonitorStream( pUnzipStream, true );

            //
            // attach memory stream
            //
            oMonitorStream.attach( &oMemoryStream, false );

            //
            // read
            //
            while (oMonitorStream.available() > 0)
            {
                //
                // NOP read
                //
                oMonitorStream.read( pDataBuffer, DATA_BUFFER_BYTES );
            }

            //DWFCORE_FREE_OBJECT( pUnzipStream );

            DUMP( L"Original text file read from archive" );
        }

            //
            // original bin file
            //
        {
            // with a password, no salting.
            DWFCore::DWFInputStream* pUnzipStream = oUnzipFile.unzip( oBinFilename.name(), L"a password"  );
            DWFCore::DWFMonitoredInputStream oMonitorStream( pUnzipStream, true );

            //
            // attach memory stream
            //
            oMonitorStream.attach( &oMemoryStream, false );

            //
            // read
            //
            while (oMonitorStream.available() > 0)
            {
                //
                // NOP read
                //
                oMonitorStream.read( pDataBuffer, DATA_BUFFER_BYTES );
            }

            //DWFCORE_FREE_OBJECT( pUnzipStream );

            DUMP( L"Original bin file read from archive" );
        }

            //
            // copied text file
            //
        {
            DWFCore::DWFInputStream* pUnzipStream = oUnzipFile.unzip( zTextTempName );
            DWFCore::DWFMonitoredInputStream oMonitorStream( pUnzipStream, true );

            //
            // attach memory stream
            //
            oMonitorStream.attach( &oMemoryStream, false );

            //
            // read
            //
            while (oMonitorStream.available() > 0)
            {
                //
                // NOP read
                //
                oMonitorStream.read( pDataBuffer, DATA_BUFFER_BYTES );
            }

            //DWFCORE_FREE_OBJECT( pUnzipStream );

            DUMP( L"Copied text file read from archive" );
        }

            //
            // copied bin file
            //
        {
            DWFCore::DWFInputStream* pUnzipStream = oUnzipFile.unzip( zBinTempName );
            DWFCore::DWFMonitoredInputStream oMonitorStream( pUnzipStream, true );

            //
            // attach memory stream
            //
            oMonitorStream.attach( &oMemoryStream, false );

            //
            // read
            //
            while (oMonitorStream.available() > 0)
            {
                //
                // NOP read
                //
                oMonitorStream.read( pDataBuffer, DATA_BUFFER_BYTES );
            }

            //DWFCORE_FREE_OBJECT( pUnzipStream );

            DUMP( L"Copied binary file read from archive" );
        }

        //
        // close
        //
        oUnzipFile.close();

        DUMP( L"Zip archive closed" );

        //
        // final output
        //
        _DWFCORE_SWPRINTF( (wchar_t*)pDataBuffer, DATA_BUFFER_BYTES, L"Total buffered bytes: %lu", oMemoryStream.bytes() );
        DUMP( (wchar_t*)pDataBuffer );

        cout << "OK\n";

        DWFCORE_FREE_MEMORY(pDataBuffer);
 
    return 0;
}
