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


#include "stdafx.h"

#include <iomanip>

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;

#ifdef _DWFCORE_WIN32_SYSTEM

    #if _USE_VLD
        #define _USE_VLD_FOR_MEMORY_LEAK_TEST
        #include <vld.h>
    #else
        #define _USE_CRTDBG_FOR_MEMORY_LEAK_TEST
        #include <crtdbg.h>
    #endif

#endif

DWFString do_children( DWFObjectDefinition* pDef, DWFDefinedObjectInstance* pInst )
{
    DWFString zOut;

        zOut.append( L"Object [" );
        zOut.append( pInst->object() );
        zOut.append( L"] Node [" );
        zOut.append( pInst->node() );
        zOut.append( L"]\n" );

        DWFProperty* pProp = NULL;
        DWFPropertyContainer* pInstProps = pDef->getInstanceProperties( *pInst );
                    
        DWFProperty::tMap::Iterator* piProp = pInstProps->getProperties();

        if (piProp)
        {
            for (;piProp->valid(); piProp->next())
            {
                pProp = piProp->value();

                zOut.append( pProp->name() );
                zOut.append( L", " );
                zOut.append( pProp->value() );
                zOut.append( L"    [" );
                zOut.append( pProp->category() );
                zOut.append( L"]\n" );

            }

            DWFCORE_FREE_OBJECT( piProp );
        }

        DWFCORE_FREE_OBJECT( pInstProps );

        DWFDefinedObjectInstance::tMap::Iterator* piChildren = pInst->resolvedChildren();
        
        if (piChildren)
        {
            for (;piChildren->valid(); piChildren->next())
            {
                zOut.append( do_children( pDef, piChildren->value() ) );
            }
            DWFCORE_FREE_OBJECT( piChildren );
        }

    return zOut;
}

int main(int argc, char* argv[])
{
#if defined(_DEBUG) && defined(_USE_CRTDBG_FOR_MEMORY_LEAK_TEST)
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

        if (tInfo.eType != DWFPackageReader::eDWFPackage)
        {
            cout << "File is not a DWF package ";
            if (tInfo.eType == DWFPackageReader::eW2DStream)
            {
                cout << "[W2D Stream]";
            }
            else if (tInfo.eType == DWFPackageReader::eDWFStream)
            {
                cout << "[DWF Stream (<6.0)]";
            } 
            else if (tInfo.eType == DWFPackageReader::eZIPFile)
            {
                cout << "[ZIP Archive]";
            }
            else
            {
                cout << "[Unknown]";
            }
            cout << endl;
            exit( 0 );
        }

        cout << setprecision(2) << "DWF Package version [" << (float)(tInfo.nVersion)/100.0f << "]" << endl;

        DWFManifest& rManifest = oReader.getManifest();

        //
        // create a text file
        //
        DWFFile oTextFilename( L"ObjectDefinitionDump.txt" );
        DWFStreamFileDescriptor oTextFile( oTextFilename, L"w+" );
        oTextFile.open();

        //
        // create a stream to write to the files
        //
        DWFFileOutputStream oFilestream;

        //
        // first, attach the descriptor for the text file to the stream
        // and write the text buffer to the file
        //
        oFilestream.attach( &oTextFile, false );


        DWFSection* pSection = NULL;
        DWFManifest::SectionIterator* piSections = rManifest.getSections();
        
        if (piSections)
        {
            for (; piSections->valid(); piSections->next())
            {
                pSection = piSections->get();

                DWFResourceContainer::ResourceIterator* piObjDefs = pSection->findResourcesByRole( DWFXML::kzRole_ObjectDefinition );
                if (piObjDefs && piObjDefs->valid())
                {
                    DWFObjectDefinition* pDef = pSection->getObjectDefinition();
                    if (pDef)
                    {
                        DWFString zOut;

                        DWFDefinedObjectInstance::tList oRootInstances;
                        pDef->getRootInstances( oRootInstances );

                        DWFDefinedObjectInstance* pInst = NULL;
                        DWFDefinedObjectInstance::tList::const_iterator iInst = oRootInstances.begin();
                        for (; iInst != oRootInstances.end(); iInst++)
                        {
                            pInst = *iInst;
                            zOut = do_children( pDef, pInst );

//                            wcout << (const wchar_t*)zOut;

                            oFilestream.write( (const wchar_t*)zOut, zOut.bytes() );
                        }

                        DWFCORE_FREE_OBJECT( pDef );
                    }

                    DWFCORE_FREE_OBJECT( piObjDefs );
                }
            }
            DWFCORE_FREE_OBJECT( piSections );
        }

        oFilestream.detach();

        char dummy[10]={0};
        wcout << L"Press any key to purge string table...\n";
        ::scanf( dummy );
        DWFStringTable::Purge();
        wcout << L"Press any key to continue...\n";
        ::scanf( dummy );

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


