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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/xps/FixedDocumentSequence.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/xps/FixedDocumentSequence.h"
#include "dwf/xps/Constants.h"

using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const XPSFixedDocumentSequence::kzName = "FixedDocumentSequence.fdseq";
//DNT_End

#endif


_DWFTK_API
XPSFixedDocumentSequence::XPSFixedDocumentSequence()
throw()
                        : _oFixedDocuments()
{
    setName( XPSFixedDocumentSequence::kzName );
}

_DWFTK_API
XPSFixedDocumentSequence::~XPSFixedDocumentSequence()
throw()
{
    XPSFixedDocument::tIterator* piDocs = _oFixedDocuments.iterator();
    if (piDocs)
    {
        for (; piDocs->valid(); piDocs->next())
        {
            XPSFixedDocument* pDoc = piDocs->get();
            if (pDoc->owner() == this)
            {
                pDoc->disown( *this, true );
                DWFCORE_FREE_OBJECT( pDoc );
            }
            else
            {
                pDoc->unobserve( *this );
            }
        }
        DWFCORE_FREE_OBJECT( piDocs );
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
bool
XPSFixedDocumentSequence::addFixedDocument( XPSFixedDocument* pFixedDocument,
                                            bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oFixedDocuments.findFirst( pFixedDocument, iPosition ))
    {
        //
        //  If the document is already in the sequence we cannot add it again
        //
        return false;
    }
    else
    {
        _oFixedDocuments.push_back( pFixedDocument );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pFixedDocument->own( *this );
        }
        else
        {
            pFixedDocument->observe( *this );
        }

        return true;
    }
}


_DWFTK_API
bool
XPSFixedDocumentSequence::insertFixedDocument( XPSFixedDocument* pFixedDocument, 
                                               XPSFixedDocument* pAfterDocument,
                                               bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oFixedDocuments.findFirst( pFixedDocument, iPosition ))
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
            if (_oFixedDocuments.findFirst( pAfterDocument, iPosition ))
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
        _oFixedDocuments.insertAt( pFixedDocument, iPosition );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pFixedDocument->own( *this );
        }
        else
        {
            pFixedDocument->observe( *this );
        }

        return true;
    }
}


_DWFTK_API
bool
XPSFixedDocumentSequence::removeFixedDocument( XPSFixedDocument* pFixedDocument )
throw()
{
    if (_oFixedDocuments.erase( pFixedDocument ))
    {
        if (pFixedDocument->owner() == this)
        {
            pFixedDocument->disown( *this, true );
        }
        else
        {
            pFixedDocument->unobserve( *this );
        }

        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pFixedDocument );

        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
XPSFixedDocument* const
XPSFixedDocumentSequence::fixedDocument( const DWFString& zURI ) const
throw()
{
    if (_oFixedDocuments.size() > 0)
    {
        DWFPointer<XPSFixedDocument::tConstIterator> piDocs( _oFixedDocuments.constIterator(), false );
        if (!piDocs.isNull())
        {
            while (piDocs->valid())
            {
                XPSFixedDocument* pDoc = piDocs->get();
                if (pDoc->uri() == zURI)
                {
                    return pDoc;
                }
                piDocs->valid();
            }
        }
    }

    return NULL;
}

_DWFTK_API
void
XPSFixedDocumentSequence::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    //
    //  If an ownable is getting destroyed we just need to remove
    //  any references to it.
    //

    XPSFixedDocument* pDoc = dynamic_cast<XPSFixedDocument*>(&rOwnable);
    if (pDoc)
    {
        _oFixedDocuments.erase( pDoc );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void XPSFixedDocumentSequence::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( XPSXML::kzElement_FixedDocumentSequence );
    rSerializer.addAttribute( /*NOXLATE*/"xmlns", XPSXML::kzNamespaceURI_XPS );

    XPSFixedDocument::tIterator* piDocs = _oFixedDocuments.iterator();
    if (piDocs != NULL)
    {
        while (piDocs->valid())
        {
            XPSFixedDocument* pDoc = piDocs->get();

            if (pDoc != NULL)
            {
                rSerializer.startElement( XPSXML::kzElement_DocumentReference );
                rSerializer.addAttribute( XPSXML::kzAttribute_Source, pDoc->uri() );
                rSerializer.endElement();
            }

            piDocs->next();
        }
        DWFCORE_FREE_OBJECT( piDocs );
    }

    rSerializer.endElement();
}

#endif

