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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/DWFDocumentSequence.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/DWFDocumentSequence.h"
#include "dwf/dwfx/Constants.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFXDWFDocumentSequence::kzName = "DWFDocumentSequence.dwfseq";
//DNT_End

#endif



_DWFTK_API
DWFXDWFDocumentSequence::DWFXDWFDocumentSequence()
throw()
                       : _oDWFDocuments()
{
    setName( DWFXDWFDocumentSequence::kzName );
}

_DWFTK_API
DWFXDWFDocumentSequence::~DWFXDWFDocumentSequence()
throw()
{
    DWFXDWFDocument::tIterator* piParts = _oDWFDocuments.iterator();
    if (piParts)
    {
        for (; piParts->valid(); piParts->next())
        {
            DWFXDWFDocument* pPart = piParts->get();
            if (pPart->owner() == this)
            {
                DWFCORE_FREE_OBJECT( pPart );
            }
            else
            {
                pPart->unobserve( *this );
            }
        }
        DWFCORE_FREE_OBJECT( piParts );
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
bool
DWFXDWFDocumentSequence::addDWFDocument( DWFXDWFDocument* pDWFDocument,
                                         bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oDWFDocuments.findFirst( pDWFDocument, iPosition ))
    {
        //
        //  If the part is already in the sequence we cannot add it again
        //
        return false;
    }
    else
    {
        _oDWFDocuments.push_back( pDWFDocument );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pDWFDocument->own( *this );
        }
        else
        {
            pDWFDocument->observe( *this );
        }

        addRelationship( pDWFDocument, DWFXXML::kzRelationship_Document );

        return true;
    }
}

_DWFTK_API
bool
DWFXDWFDocumentSequence::insertDWFDocument( DWFXDWFDocument* pDWFDocument,
                                             DWFXDWFDocument* pAfterDocument,
                                             bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oDWFDocuments.findFirst( pDWFDocument, iPosition ))
    {
        //
        //  If the document is already in the sequence we cannot add it again
        //
        return false;
    }
    else
    {
        iPosition = 0;

        if (pAfterDocument != NULL)
        {
            if (_oDWFDocuments.findFirst( pAfterDocument, iPosition ))
            {
                iPosition++;
            }
            else
            {
                //
                // If pAfterDocument is not NULL, it has to be in the list
                //
                return false;
            }
        }

        //
        //  Now add the document to the container in the sequence
        //
        _oDWFDocuments.insertAt( pDWFDocument, iPosition );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pDWFDocument->own( *this );
        }
        else
        {
            pDWFDocument->observe( *this );
        }

        addRelationship( pDWFDocument, DWFXXML::kzRelationship_Document );
        return true;
    }
}

_DWFTK_API
bool
DWFXDWFDocumentSequence::removeDWFDocument( DWFXDWFDocument* pDWFDocument )
throw()
{
    if (_oDWFDocuments.erase( pDWFDocument ))
    {
        if (pDWFDocument->owner() == this)
        {
            pDWFDocument->disown( *this, true );
        }
        else
        {
            pDWFDocument->unobserve( *this );
        }

        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pDWFDocument );

        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
void
DWFXDWFDocumentSequence::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    DWFXDWFDocument* pPart = dynamic_cast<DWFXDWFDocument*>(&rOwnable);
    if (pPart)
    {
        _oDWFDocuments.erase( pPart );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXDWFDocumentSequence::serializeXML( DWFXMLSerializer& rSerializer )
throw()
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( DWFXXML::kzElement_DWFDocumentSequence );
    rSerializer.addAttribute( /*NOXLATE*/L"xmlns", DWFXXML::kzNamespaceURI_DWFX );

    DWFXDWFDocument::tIterator* piParts = _oDWFDocuments.iterator();
    if (piParts != NULL)
    {
        while (piParts->valid())
        {
            DWFXDWFDocument* pPart = piParts->get();

            if (pPart != NULL)
            {
                rSerializer.startElement( DWFXXML::kzElement_ManifestReference );
                rSerializer.addAttribute( DWFXXML::kzAttribute_Source, pPart->uri() );
                rSerializer.endElement();
            }

            piParts->next();
        }
        DWFCORE_FREE_OBJECT( piParts );
    }

    rSerializer.endElement();
}

#endif

_DWFTK_API
DWFString
DWFXDWFDocumentSequence::provideDocumentURI( const DWFString& zDocumentURI )
throw()
{
    _oDWFDocumentURIs.push_back( zDocumentURI );
    return zDocumentURI;
}

