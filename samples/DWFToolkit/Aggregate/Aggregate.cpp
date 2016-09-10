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

//#include <vld.h>
#include <iostream>
#include <set>

#include "dwfcore/File.h"
#include "dwfcore/String.h"
#include "dwfcore/MIME.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileInputStream.h"

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/writer/DWF6PackageWriter.h"
#include "dwf/package/reader/PackageReader.h"

using namespace DWFCore;
using namespace DWFToolkit;

#define _DO_CRTDBG_MEMORY_LEAK_CHECK_
#if defined(_DWFCORE_WIN32_SYSTEM) && defined(_DO_CRTDBG_MEMORY_LEAK_CHECK_)
    #include <crtdbg.h>
#endif

bool processArgs( int argc, 
                  char* argv[], 
                  std::vector<DWFString>& oInFiles, 
                  DWFString& zOutFile, 
                  DWFPackageWriter::teMergeContent& eMergeType );

/////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

#if defined(_DO_CRTDBG_MEMORY_LEAK_CHECK_) && defined(_DWFCORE_WIN32_SYSTEM)
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
    /*
    long foo = 1;
    _CrtSetBreakAlloc(foo);
    */
#endif

    if (argc < 3)
    {
        wcout << L"Usage: " << argv[0] << L"[-o output.dwf] [-m [p|s]] {files to aggregate}.dwf" << endl;
        wcout << "-o optionally specify output filename. If it is not provided then the name is Aggregated.dwf." << endl;
        wcout << "-m should be specified to merge the content in the aggregated package. The p implies that in the case of" << endl;
        wcout << "   conflicts, the primary content element will be kept (default for a merge). If s is specified, then " << endl;
        wcout << "   the elements being merged into the primary content have priority." << endl;
        wcout << endl;
        return 1;
    }


    try
    {
        std::vector<DWFString>      oInfiles(0);
        DWFString                   zOutfile( L"Aggregated.dwf" );
        DWFPackageWriter::teMergeContent eMergeType = DWFPackageWriter::eNoMerge;

        if (!processArgs(argc, argv, oInfiles, zOutfile, eMergeType))
        {
            wcout << L"Usage: " << argv[0] << L"[-o output.dwf] [-m [p|s]] {files to aggregate}.dwf" << endl;
            wcout << "-o optionally specify output filename. If it is not provided then the name is Aggregated.dwf." << endl;
            wcout << "-m should be specified to merge the content in the aggregated package. The p implies that in the case of" << endl;
            wcout << "   conflicts, the primary content element will be kept (default for a merge). If s is specified, then " << endl;
            wcout << "   the elements being merged into the primary content have priority." << endl;
            wcout << endl;
            return 1;
        }

        //
        //  Create the destination DWF
        //
        DWFFile oOutfile( zOutfile );
        DWF6PackageWriter oWriter( oOutfile );

        //  If we are going to aggregate sections from other packages then we need to initialize
        //  for aggregation
        oWriter.initializeForAggregation( eMergeType );

        //
        //  The source readers are going to be stored in here until the write is complete
        //
        DWFOrderedVector<DWFPackageReader*> oReaders;

        std::vector<DWFString>::iterator iFile = oInfiles.begin();
        for (; iFile != oInfiles.end(); ++iFile)
        {
            //
            //  Open each source file and grab the sections and contents and add them to the destination
            //
            DWFFile oInfile( *iFile );
            DWFPackageReader* pReader = DWFCORE_ALLOC_OBJECT( DWFPackageReader( oInfile ) );

            //
            //  Store the reader for later cleanup
            //
            oReaders.push_back( pReader );

            //
            //  Get the manifest
            //
            DWFManifest& rManifestIn = pReader->getManifest();

            //
            //  For each section determine content to get, track the content and copy sections
            //
            DWFManifest::SectionIterator* piSections = rManifestIn.getSections();
            if (piSections)
            {
                DWFSection* pSection = NULL;

                for (; piSections->valid(); piSections->next())
                {
                    pSection = piSections->get();
                    pSection->readDescriptor();

                    //
                    //  Add the section
                    //
                    oWriter.addSection( pSection );
                }

                DWFCORE_FREE_OBJECT( piSections );
            }
        }

        oWriter.write();

        DWFOrderedVector<DWFPackageReader*>::Iterator* piReader = oReaders.iterator();
        if (piReader)
        {
            for (; piReader->valid(); piReader->next())
            {
                DWFCORE_FREE_OBJECT( piReader->get() );
            }
            DWFCORE_FREE_OBJECT( piReader );
        }

    }

    catch (DWFException& ex)
    {
        wcout << ex.type() << L": " << ex.message() << endl;
        wcout << L"(function) " << ex.function() << endl;
        wcout << L"(file) " << ex.file() << endl;
        wcout << L"(line) " << ex.line() << endl;
    }

    return 0;
}

///////////////////////////////////////////////////////////

bool processArgs( int argc, 
                  char* argv[], 
                  std::vector<DWFString>& oInfiles, 
                  DWFString& zOutfile, 
                  DWFPackageWriter::teMergeContent& eMergeType )
{
    zOutfile.assign( L"Aggregated.dwf" );
    eMergeType = DWFPackageWriter::eMergePriorityPrimary;

    int i=1;
    while (i < argc)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'o')
            {
                if (argc == i+1)
                {
                    wcout << L"No output file specified with the -o option" << endl;
                    return false;
                }
                zOutfile.assign( argv[++i] );
            }

            if (argv[i][1] == 'm')
            {
                if (argc > i+1)
                {
                    //
                    // Compare whole word - not just character since this may be a file name otherwise
                    //
                    DWFString zMergeValue( argv[i+1] );
                    if (zMergeValue == L"P" || zMergeValue == L"p")
                    {
                        eMergeType = DWFPackageWriter::eMergePriorityPrimary;
                        ++i;
                    }
                    else if (zMergeValue == L"S" || zMergeValue == L"s")
                    {
                        eMergeType = DWFPackageWriter::eMergePrioritySecondary;
                        ++i;
                    }
                    else
                    {
                        eMergeType = DWFPackageWriter::eMergePriorityPrimary;
                    }
                }
                else
                {
                    eMergeType = DWFPackageWriter::eMergePriorityPrimary;
                }
            }

            if((argv[i][1] == 'h') ||
               (argv[i][1] == '?') ||
               (argv[i][1] == '-') && (argv[i][2] == 'h'))
            {
                return false;
            }
        }
        else
        {
            oInfiles.push_back( DWFString(argv[i]) );
        }

        i++;
    }

    if (oInfiles.empty())
    {
        wcout << L"No input files were provided for aggregation." << endl;
        return false;
    }

    return true;
}



