//
//  Copyright (c) 2007 by Autodesk, Inc.
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

#include "dwfcore/String.h"

#include "dwf/opc/Constants.h"
#include "dwf/opc/Package.h"
#include "dwf/opc/ZipFileReader.h"

using namespace DWFCore;
using namespace DWFToolkit;

#define _DO_CRTDBG_MEMORY_LEAK_CHECK_
#if defined(_DWFCORE_WIN32_SYSTEM) && defined(_DO_CRTDBG_MEMORY_LEAK_CHECK_)
    #include <crtdbg.h>
#endif

set<DWFString> oVisitedPartsURIs;
set<DWFString> oVisitedRelationshipTypes;

void traverseAllPartsFromRelationship(OPCRelationship* pRel, OPCPhysicalLayerReader* pPhysReader);

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
    long foo = 0;
    _CrtSetBreakAlloc(foo);
    */
#endif

    if (argc < 2)
    {
        wcout << L"Usage: " << argv[0] << L" {file to examine}" << endl;
        return 1;
    }

    try
    {
        DWFFile oInFile( argv[1] );

        //
        // Create some kind of OPCPhysicalLayerReader for loading the package.
        //
        // OPCZipFileReader is the only Physical Layer Reader we have at this time.
        // It takes a Zip file descriptor as an argument.
        //
        DWFZipFileDescriptor* pZipDescr = DWFCORE_ALLOC_OBJECT(DWFZipFileDescriptor(oInFile, DWFZipFileDescriptor::eUnzip));
        OPCPhysicalLayerReader* pPhysReader = DWFCORE_ALLOC_OBJECT(OPCZipFileReader(pZipDescr));

        //
        // Open the package file.
        //
        pPhysReader->open();

        //
        // Create the OPCPackage object.
        //
        OPCPackage* pPackage = DWFCORE_ALLOC_OBJECT(OPCPackage);

        //
        // Load the top-level relationships ("/_rels/.rels")
        //
        pPackage->readRelationships(pPhysReader);

        wcout << L"Parts found:" << endl;

        //
        // Iterate over the top-level relationships
        //
        OPCRelationship::tIterator* pIRel = pPackage->relationships();
        if(pIRel)
        {
            for( ; pIRel->valid(); pIRel->next() )
            {
                //
                // Traverse from the part (URI) listed in this relationship.
                //
                traverseAllPartsFromRelationship( pIRel->get(), pPhysReader );
            }

            DWFCORE_FREE_OBJECT( pIRel );
        }

        //
        // Report all the Relationship Types seen.
        //
        wcout << endl << endl << L"Relationship types found:" << endl;
        set<DWFString>::iterator it = oVisitedRelationshipTypes.begin();
        for(; it != oVisitedRelationshipTypes.end(); it++)
        {
            wcout << (const wchar_t*)(*it) << endl;
        }

        //
        // Close the package file.
        //
        pPhysReader->close();

        //
        // Clean up
        //
        DWFCORE_FREE_OBJECT( pPackage );
        DWFCORE_FREE_OBJECT( pPhysReader );
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


//
// TODO - Consider moving this or something like it to OPCPackage.
//
OPCPart*
getPart(const DWFString &zPartURI, OPCPhysicalLayerReader* pPhysReader, bool bInitializeStream, bool bLoadRelationships)
    throw( DWFException )
{

    //
    // Create the new part
    //
    OPCPart* pOPCPart = DWFCORE_ALLOC_OBJECT( OPCPart );
    if (pOPCPart == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate part" );
    }

    //
    // TODO: API CHANGE NEEDED - It's painful to have to seperately init the path and name
    //
    off_t pos = zPartURI.findLast(L'/');
    if(pos != -1)
    {
        pOPCPart->setPath(zPartURI.substring(0, pos));
    }
    pOPCPart->setName(zPartURI.substring(pos+1));

    if(bInitializeStream)
    {
        pOPCPart->setInputStream(pPhysReader->read(zPartURI), 0, true);
    }

    if(bLoadRelationships)
    {
        const DWFString& zRelsURI = pOPCPart->relationshipUri();
        if(zRelsURI.chars() > 0)
        {
            DWFInputStream *pRelStream = pPhysReader->read(zRelsURI);
            if(pRelStream)
            {
                pOPCPart->loadRelationships(pRelStream);
                DWFCORE_FREE_OBJECT(pRelStream);
            }
        }
    }

    return pOPCPart;
}

void traverseAllRelationshipsFromPart(OPCPart* pPart, OPCPhysicalLayerReader* pPhysReader)
{
    //
    // Report seeing this URI.
    //
    wcout << (const wchar_t*)(pPart->uri()) << endl;

    //
    // Iterate over the relationships
    //
    OPCRelationship::tIterator* pIRel = pPart->relationships();
    if(pIRel)
    {
        for( ; pIRel->valid(); pIRel->next() )
        {
            //
            // Recurse on the part (URI) listed in this relationship.
            //
            traverseAllPartsFromRelationship( pIRel->get(), pPhysReader );
        }

        DWFCORE_FREE_OBJECT( pIRel );
    }
}

void
traverseAllPartsFromRelationship(OPCRelationship* pRel, OPCPhysicalLayerReader* pPhysReader)
{
    //
    // Track that we'd seen this type of relationship.
    //
    oVisitedRelationshipTypes.insert(pRel->relationshipType());

        //
        // Have we visited a part with this URI already?
        //
        // If so, bail.
        //
    if(1 == oVisitedPartsURIs.count(pRel->targetURI()))
    {
        return;
    }

    //
    // Remember that we saw a part with this URI.
    //
    oVisitedPartsURIs.insert(pRel->targetURI());

    //
    // Cons up a part for the "thing" with this URI, and load it's relationships.
    //
    OPCPart* pPart = getPart(pRel->targetURI(), pPhysReader, false, true);

    //
    // Walk all the part's relationships - eventually recursing back to us from each of them.
    //
    traverseAllRelationshipsFromPart(pPart, pPhysReader);

    //
    // Done with the part.
    //
    DWFCORE_FREE_OBJECT(pPart);
}


