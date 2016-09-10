//
//  Copyright (c) 2003-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/package/FontResource.cpp 7     8/11/05 7:56p Kuk $
//


#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/package/FontResource.h"
#include "dwf/package/Constants.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;


_DWFTK_API
DWFFontResource::DWFFontResource( DWFPackageReader* pPackageReader )
throw()
               : DWFResource( pPackageReader )
               , _nRequest( 0 )
               , _ePrivilege( eNoEmbedding )
               , _eCharacterCode( eUnicode )
{
    ;
}

_DWFTK_API
DWFFontResource::DWFFontResource( int                nRequest,
                                  tePrivilege        ePrivilege,
                                  teCharacterCode    eCharacterCode,
                                  const DWFString&   zCanonicalName,
                                  const DWFString&   zLogfontName)
throw()
               : DWFResource( /*NOXLATE*/L"", DWFXML::kzRole_Font, DWFMIME::kzMIMEType_FONT )
               , _nRequest( nRequest )
               , _ePrivilege( ePrivilege )
               , _eCharacterCode( eCharacterCode )
               , _zLogfontName ( zLogfontName )
               , _zCanonicalName( zCanonicalName )
{
    ;
}

_DWFTK_API
DWFFontResource::~DWFFontResource()
throw()
{
    ;
}

_DWFTK_API
void
DWFFontResource::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    //
    // parse with base resource first
    //
    DWFResource::parseAttributeList( ppAttributeList );

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
            //
            // skip over any acceptable prefixes in the element name
            //
        if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][4];
        }
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][8];
        }
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][6];
        }
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][7];
        }
        else
        {
            pAttrib = &ppAttributeList[iAttrib][0];
        }

            //
            // set the canonical name
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_CanonicalName) == 0))
        {
            nFound |= 0x01;

            _zCanonicalName.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the logfont name
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_LogfontName) == 0))
        {
            nFound |= 0x02;

            _zLogfontName.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the request
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Request) == 0))
        {
            nFound |= 0x04;

            _nRequest = ::atoi( ppAttributeList[iAttrib+1] );
        }
            //
            // set the privilege
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Privilege) == 0))
        {
            nFound |= 0x08;

            if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"previewPrint") == 0)
            {
                _ePrivilege = ePreviewPrint;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"editable") == 0)
            {
                _ePrivilege = eEditable;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"installable") == 0)
            {
                _ePrivilege = eInstallable;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"noEmbedding") == 0)
            {
                _ePrivilege = eNoEmbedding;
            }
        }
            //
            // set the character code
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_CharacterCode) == 0))
        {
            nFound |= 0x10;

            if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"unicode") == 0)
            {
                _eCharacterCode = eUnicode;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"symbol") == 0)
            {
                _eCharacterCode = eSymbol;
            }
            else if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"glyphIdx") == 0)
            {
                _eCharacterCode = eGlyphIndex;
            }
        }
    }
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFFontResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    DWFString zNamespace;

        //
        // namespace dictated by document and section type
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        zNamespace.assign( namespaceXML(nFlags) );
    }

        //
        // serialize in detail for descriptor
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        rSerializer.startElement( DWFXML::kzElement_FontResource, zNamespace );

        //
        // let base class know not to start the element
        //
        nFlags |= DWFXMLSerializer::eElementOpen;

        //
        // base attributes
        //
        DWFResource::serializeXML( rSerializer, nFlags );

            //
            // attributes
            //
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Request, _nRequest );

            rSerializer.addAttribute( DWFXML::kzAttribute_Privilege,
                                        (_ePrivilege == ePreviewPrint ? /*NOXLATE*/L"previewPrint" :
                                         _ePrivilege == eEditable ? /*NOXLATE*/L"editable" :
                                         _ePrivilege == eInstallable ? /*NOXLATE*/L"installable" : /*NOXLATE*/L"noEmbedding") );

            rSerializer.addAttribute( DWFXML::kzAttribute_CharacterCode,
                                        (_eCharacterCode == eUnicode ? /*NOXLATE*/L"unicode" :
                                         _eCharacterCode == eSymbol ? /*NOXLATE*/L"symbol" : /*NOXLATE*/L"glyphIdx") );

            rSerializer.addAttribute( DWFXML::kzAttribute_LogfontName, _zLogfontName );
            rSerializer.addAttribute( DWFXML::kzAttribute_CanonicalName, _zCanonicalName );
        }

        //
        // properties - as the most derived resource, we must invoke the
        // property serialization ourselves or we will end up with properties
        // in the middle of our resource element
        //
        DWFXMLSerializable& rSerializable = DWFPropertyContainer::getSerializable();
        rSerializable.serializeXML( rSerializer, nFlags );

            //
            // Serialize relationships.
            //
        if(!_oRelationships.empty())
        {
            DWFResourceRelationship::tList::Iterator *pIter = _oRelationships.iterator();
            if (pIter)
            {
                rSerializer.startElement( DWFXML::kzElement_Relationships, zNamespace );

                for (; pIter->valid(); pIter->next())
                {
                    DWFResourceRelationship *pRelationship = pIter->get();
                    if(pRelationship != NULL)
                    {
                        rSerializer.startElement( DWFXML::kzElement_Relationship, zNamespace );
                        rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, pRelationship->resourceID() );
                        rSerializer.addAttribute( DWFXML::kzAttribute_Type, pRelationship->type());
                        rSerializer.endElement();
                    }
                }
                DWFCORE_FREE_OBJECT( pIter );

                rSerializer.endElement();
            }
        }

        rSerializer.endElement();
    }
        //
        // otherwise defer to the base class
        //
    else
    {
        DWFResource::serializeXML( rSerializer, nFlags );
    }
}

#endif

