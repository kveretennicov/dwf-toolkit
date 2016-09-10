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


#include "dwfcore/Core.h"
#include "dwfcore/Timer.h"
#include "dwfcore/String.h"
#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/ZipFileDescriptor.h"
#include "dwfcore/UnzippingInputStream.h"
#include "dwfcore/ZippingOutputStream.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
#include "dwfcore/TempFile.h"
#include "dwfcore/MonitoredInputStream.h"

#ifdef  _DWFCORE_WIN32_SYSTEM
#include <crtdbg.h>
#endif

using namespace DWFCore;


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

        //
        // create a text file
        //
        DWFCore::DWFFile oTextFilename( L"Serializer.xml" );
        DWFCore::DWFStreamFileDescriptor oTextFile( oTextFilename, L"w+" );
        oTextFile.open();

        //
        // create a stream to write to the file
        //
        DWFCore::DWFFileOutputStream oFilestream;


        //
        // attach the descriptor for the text file to the stream
        //
        oFilestream.attach( &oTextFile, false );

        //
        // Create an XMLSerializer, and attach it to the output stream.
        //
        DWFUUID oUUIDGen;
        DWFCore::DWFXMLSerializer oXMLSerializer(oUUIDGen);
        oXMLSerializer.attach(oFilestream);

        //
        // Add an element and an attribute on that element.
        //
        oXMLSerializer.startElement(L"Woof");
        oXMLSerializer.addAttribute(L"textAttrName", L"textAttrValue");

        DWFString woof(200);
        woof.assign(L"Woof");
        woof.append(L"-woof");
        woof.bytes();
        woof.assign("");
        DWFString arf(woof);
        arf;

        //
        // Test the "insert" functionality.
        //
        DWFString zStringToEmbed(L"<meow> <squeak> Squeak Data </squeak> </meow>");
        char *pBuffer;
        size_t nBytes = zStringToEmbed.getUTF8(&pBuffer);
        {
            DWFBufferInputStream oBuffIS(pBuffer, nBytes);
            oXMLSerializer.insertXMLStream(&oBuffIS);
        }
        DWFCORE_FREE_MEMORY(pBuffer);

        //
        // Add another element, an attribute, and close.
        //
        oXMLSerializer.startElement(L"Arf");
        oXMLSerializer.addAttribute(L"integerAttribute", 1);
        int intArray[5] = {1, 2, 3, 4, 5};
        oXMLSerializer.addAttribute(L"manyIntsAttribute", intArray, 5);
        double doubleArray[5] = {0.5, 0.75, 1.0, 1.25, 1.5};
        oXMLSerializer.addAttribute(L"doubleAttribute", 2.5);
        oXMLSerializer.addAttribute(L"manyDoublesAttribute", doubleArray, 5);
        oXMLSerializer.endElement();

        //
        // Close the first element.
        //
        oXMLSerializer.endElement();

        //
        // Finalize the output.
        //
        oXMLSerializer.detach();

        //
        // close the file
        //
        oTextFile.close();

    return 0;
}
