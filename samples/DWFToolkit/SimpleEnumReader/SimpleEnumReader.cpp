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

// SimpleEnumReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _DWFCORE_WIN32_SYSTEM

    #if _USE_VLD
        #define _USE_VLD_FOR_MEMORY_LEAK_TEST
        #include <vld.h>
    #else
        #define _USE_CRTDBG_FOR_MEMORY_LEAK_TEST
        #include <crtdbg.h>
    #endif

#endif

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;


void dump_bookmarks( const DWFBookmark* pRoot )
{
    static size_t l = 0;
    size_t i;

    for (i=0;i<l;i++)
    {
        wcout << "  ";
    }

    if (pRoot->name())
    {
        wcout << (const wchar_t*)(pRoot->name()) << endl;
    }

    const DWFBookmark::tList& rChildren = pRoot->getChildBookmarks();

    for (i=0; i < rChildren.size(); i++)
    {
        l++;
        dump_bookmarks( rChildren[i] );
        l--;
    }
}

int main(int argc, char* argv[])
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
    long foo = 0;
    _CrtSetBreakAlloc(foo);
#endif

    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" file.dwf" << endl;
        return ( 0 );
    }

    try
    {
        DWFFile oDWF( argv[1] );
        DWFPackageReader oReader( oDWF );

        DWFPackageReader::tPackageInfo tInfo;
        oReader.getPackageInfo( tInfo );

        wchar_t zBuffer[256] = {0};

        if (tInfo.eType != DWFPackageReader::eDWFPackage &&
            tInfo.eType != DWFPackageReader::eDWFXPackage)
        {
            _DWFCORE_SWPRINTF( zBuffer, 256, L"File is not a DWF package [%s]",
                        (tInfo.eType == DWFPackageReader::eW2DStream) ? L"W2D Stream" :
                        (tInfo.eType == DWFPackageReader::eDWFStream) ? L"DWF Stream (<6.0)" :
                        (tInfo.eType == DWFPackageReader::eZIPFile) ? L"ZIP Archive" : L"Unknown" );

            wcout << zBuffer << endl;
            exit( 0 );
        }

        _DWFCORE_SWPRINTF( zBuffer, 256, L"DWF Package version [%0.2f]", (float)(tInfo.nVersion)/100.0f );
        wcout << zBuffer << endl;

        wcout << L"Reading the manifest..." << endl;

        DWFManifest& rManifest = oReader.getManifest();

        _DWFCORE_SWPRINTF( zBuffer, 256, L"\tVersion: %0.2g", rManifest.version() );
        wcout << zBuffer << endl;
        wcout << L"\tObject ID: " << (const wchar_t*)rManifest.objectID() << endl;

        DWFManifest::tInterfaceIterator* piInterfaces = rManifest.getInterfaces();

        if (piInterfaces)
        {
            for (;piInterfaces->valid(); piInterfaces->next())
            {
                wcout << L"\tInterface found: " << (const wchar_t*)(piInterfaces->value()->name()) << endl;
            }

            DWFCORE_FREE_OBJECT( piInterfaces );
        }

        DWFProperty* pProperty = NULL;
        DWFProperty::tMap::Iterator* piProperties = rManifest.getProperties();

        if (piProperties)
        {
            for (;piProperties->valid(); piProperties->next())
            {
                pProperty = piProperties->value();
                wcout << L"\tProperty found: " << (const wchar_t*)(pProperty->name());

                if (pProperty->value())
                {
                    wcout << L" [" << (const wchar_t*)(pProperty->value()) << L"]";
                }

                if (pProperty->category())
                {
                    wcout << L" [" << (const wchar_t*)(pProperty->category()) << L"]";
                }

                wcout << endl;
            }

            DWFCORE_FREE_OBJECT( piProperties );
        }

        DWFSection* pSection = NULL;
        DWFManifest::SectionIterator* piSections = rManifest.getSections();

        if (piSections)
        {
            for (;piSections->valid(); piSections->next())
            {
                pSection = piSections->get();

                //
                // piSections->key() will also return the section name...
                //
                wcout << L"\tSection found: " << (const wchar_t*)(pSection->name());

                if (pSection->type())
                {
                    wcout << L" [" << (const wchar_t*)(pSection->type()) << L"]";
                }

                if (pSection->title())
                {
                    wcout << L" [" << (const wchar_t*)(pSection->title()) << L"]";
                }

                wcout << endl;
            }

            wcout << "Reading section descriptors..." << endl << endl;

            for (piSections->reset(); piSections->valid(); piSections->next())
            {
                pSection = piSections->get();

                pSection->readDescriptor();

                DWFGlobalSection* pGlobal = dynamic_cast<DWFGlobalSection*>(pSection);

                if (pGlobal == NULL)
                {
                    wcout << L"\t[OK] (" << pSection->order() << L") v" << pSection->version();

                    if (pSection->name())
                    {
                        wcout << L" [" << (const wchar_t*)(pSection->name()) << L"]";
                    }

                    if (pSection->objectID())
                    {
                        wcout << L" [" << (const wchar_t*)(pSection->objectID()) << L"]";
                    }

                    wcout << L" [" << pSection->order() << L"]";

                    wcout << endl;
                }
                else
                {
                    wcout << L"\t[OK] (Global Section)" << endl;

                    const DWFBookmark* pRoot = pGlobal->bookmark();
                    if (pRoot)
                    {
                        wcout << L"\tDumping bookmarks..." << endl;
                        dump_bookmarks( pRoot );
                    }
                }
            }

            DWFCORE_FREE_OBJECT( piSections );
        }

        wcout << L"OK\n";
    }
    catch (DWFException& ex)
    {
        wcout << ex.type() << endl;
        wcout << ex.message() << endl;
        wcout << ex.function() << endl;
        wcout << ex.file() << endl;
        wcout << ex.line() << endl;
    }

    return 0;
}


