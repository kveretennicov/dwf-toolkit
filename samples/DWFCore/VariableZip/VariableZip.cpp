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

//
// VariableZip.cpp : Defines the entry point for the console application.
//
//  The sample shows how to create a zip archive and add files to it with
//  varying modes of compression, and also how to read the archive.
//


#include <iostream>

#include "dwfcore/Core.h"
#include "dwfcore/String.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/ZipFileDescriptor.h"
using namespace std;
using namespace DWFCore;


#ifdef _DWFCORE_WIN32_SYSTEM

    #if _USE_VLD
        #define _USE_VLD_FOR_MEMORY_LEAK_TEST
        #include <vld.h>
    #else
        #define _USE_CRTDBG_FOR_MEMORY_LEAK_TEST
        #include <crtdbg.h>
    #endif

#endif

#define DUMP( s )       wcout << s << endl

#define DATA_BUFFER_BYTES   4096


//////////////////////////////////////////////////////////////////////////
//
//  Forward Declarations
//
//////////////////////////////////////////////////////////////////////////

wostream& operator<<( wostream& os, 
                      const DWFString& zStr );

void CreateTextFile( const DWFString& zFileName,
                     const DWFString& zData );

void AddToZip( DWFZipFileDescriptor& rZipFile, 
               const DWFString& zOriginalFileName,
               const DWFString& zZipFileName = L"",
               bool bBinary = true,
               DWFZipFileDescriptor::teFileMode eMode = DWFZipFileDescriptor::eZip );

void ReadFromZip( DWFZipFileDescriptor& rZipFile,
                  const DWFString& zFileName );

//////////////////////////////////////////////////////////////////////////
//
//  Main
//
//////////////////////////////////////////////////////////////////////////

int main()
{
#if defined(_USE_CRTDBG_FOR_MEMORY_LEAK_TEST)
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

        ///////////////////////////////////////////////////
        //
        //  Create a text file for adding to the archive
        //
        ///////////////////////////////////////////////////


        DWFCore::DWFString zDataString( L"Autodesk, Inc. is engaged in the development and marketing of design and drafting software and multimedia tools, primarily for the business and professional environment. For the 3 months ended 4/30/04, net revenues rose 41% to $297.9M. Net income totalled $42.5M, up from $7.5M. Revenues reflect increased upgrade revenues and higher subscription bookings. Net income also reflects improved gross margins." );
        CreateTextFile( "Files.txt", zDataString );


        ///////////////////////////////////////////////////
        //
        //  Create a Zip archive 
        //
        ///////////////////////////////////////////////////

        //
        // let's create a ZIP file and archive the text file and the jpg on disk
        //
        DWFFile oZipFilename( L"FilesArchive.zip" );
        DWFZipFileDescriptor oZipFile( oZipFilename, DWFZipFileDescriptor::eZip );

        //
        //  Open the archive first
        //
        oZipFile.open();
        DUMP( L"Created Zip file" );

        AddToZip( oZipFile, "Files.txt", "", false );
        AddToZip( oZipFile, "Files.txt", "FilesZipNone.txt", false, DWFZipFileDescriptor::eZipNone );

        AddToZip( oZipFile, "pic1.jpg" );
        AddToZip( oZipFile, "pic1.jpg", "pic1ZipNone.jpg", true, DWFZipFileDescriptor::eZipNone );
        AddToZip( oZipFile, "pic1.jpg", "pic1ZipSmallest.jpg", true, DWFZipFileDescriptor::eZipSmallest );
        AddToZip( oZipFile, "pic1.jpg", "pic1ZipFastest.jpg", true, DWFZipFileDescriptor::eZipFastest );

        //
        //  Done with the archive.
        //
        oZipFile.close();
        DUMP( L"Zip file closed" );


        ////////////////////////////
        //
        //  Reopen Zip for Reading
        //
        ////////////////////////////
        wcout << L"\n";

        //
        // finally, let's reopen the zip, open each file and simultaneously
        // stream the contents to an in memory buffer
        //
        DWFZipFileDescriptor oUnzipFile( oZipFilename, DWFZipFileDescriptor::eUnzip );
        
        //
        //  Open the zip file for unzipping.
        //
        oUnzipFile.open();
        DUMP( L"Opened Zip file" );

        ReadFromZip( oUnzipFile, "Files.txt" );
        ReadFromZip( oUnzipFile, "FilesZipNone.txt" );

        ReadFromZip( oUnzipFile, "pic1.jpg" );
        ReadFromZip( oUnzipFile, "pic1ZipNone.jpg" );
        ReadFromZip( oUnzipFile, "pic1ZipSmallest.jpg" );
        ReadFromZip( oUnzipFile, "pic1ZipFastest.jpg" );

        //
        // close
        //
        oUnzipFile.close();

        DUMP( L"Zip archive closed" );

        wcout << L"\nOK\n";
 
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  End of Main
//
//////////////////////////////////////////////////////////////////////////

void AddToZip( DWFZipFileDescriptor& rZipFile, 
               const DWFString& zOriginalFileName,
               const DWFString& zZipFileName,
               bool bBinary,
               DWFZipFileDescriptor::teFileMode eMode )
{
    //
    //  The stream through which the data will be archive in the zip package
    //
    DWFOutputStream* pZipStream = NULL;

    //
    //  If zZipFileName is empty, then just save the original file to the zip package
    //  otherwise use the name provided in zZipFileName.
    //

    //
    //  Open the stream with the corresponding name.
    //  The zipping mode is set when opening the stream
    //
    pZipStream = rZipFile.zip( zZipFileName.chars()==0 ? zOriginalFileName : zZipFileName,
                               eMode );

    //
    //  We need a stream to read in the data from the file on disk
    //  Open the descriptor first with the appropriate format and then attach
    //  it to a file input stream.
    //

    DWFFile oDiskFile( zOriginalFileName );
    DWFCore::DWFStreamFileDescriptor oInDescr( oDiskFile, bBinary ? L"rb" : L"r" );
    oInDescr.open();

    DWFCore::DWFFileInputStream oInStream;
    oInStream.attach( &oInDescr, false );

    //
    //  Read the data from the input stream and write it to the zipping output stream
    //
    size_t nBytes = 0;
    unsigned char oDataBuffer[DATA_BUFFER_BYTES] = {0};
    while (oInStream.available() > 0)
    {
        nBytes = oInStream.read( oDataBuffer, DATA_BUFFER_BYTES );
        pZipStream->write( oDataBuffer, nBytes );
    }
    pZipStream->flush();

    //
    //  Cleanup
    //
    DWFCORE_FREE_OBJECT( pZipStream );
    oInStream.detach();
    oInDescr.close();

    wcout << "Archived " << zOriginalFileName << " as " 
          << ((zZipFileName.chars()>0) ? zZipFileName : zOriginalFileName)
          << endl;
}

void ReadFromZip( DWFZipFileDescriptor& rZipFile, 
                  const DWFString& zFileName )
{
    //
    //  The stream through which the data will be read from the archive
    //
    DWFInputStream* pUnzipStream = rZipFile.unzip( zFileName );
    if (pUnzipStream == NULL)
    {
        wcout << "The file " << zFileName << " was not found in the archive" << endl;
    }

    //
    //  Prepare a file output stream to write out the archived files, to files
    //  with names containing the "Out." prefix.
    //
    DWFString oFilename( "Out." );
    oFilename.append( zFileName );
    DWFFile oFile( oFilename );
    DWFStreamFileDescriptor oOutDescr( oFile, L"wb+" );
    oOutDescr.open();

    DWFFileOutputStream oOutStream;
    oOutStream.attach( &oOutDescr, false );

    //
    //  Write out the data
    //
    size_t nBytes = 0;
    unsigned char oDataBuffer[DATA_BUFFER_BYTES] = {0};
    while (pUnzipStream->available() > 0)
    {
        nBytes = pUnzipStream->read( oDataBuffer, DATA_BUFFER_BYTES );
        oOutStream.write( oDataBuffer, nBytes );
    }
    oOutStream.flush();

    //
    //  Cleanup
    //
    DWFCORE_FREE_OBJECT( pUnzipStream );
    oOutStream.detach();
    oOutDescr.close();

    wcout << "Extracted " << zFileName << " to " << oFilename << endl;
}


wostream& operator<<( wostream& os, 
                      const DWFString& zStr )
{
    os << (const wchar_t*)(zStr);
    return os;
}

void CreateTextFile( const DWFString& zFileName,
                     const DWFString& zData )
{
    //
    // create a text file
    //
    DWFCore::DWFFile oTextFilename( zFileName );
    DWFCore::DWFStreamFileDescriptor oTextFile( oTextFilename, L"w+" );
    oTextFile.open();
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
        oFilestream.write( (const wchar_t*)zData, zData.bytes() );
        oFilestream.detach();

        //
        // close the file
        //
        oTextFile.close();
        DUMP( L"Text file created." );
    }
}

