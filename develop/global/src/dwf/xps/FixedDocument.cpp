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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/xps/FixedDocument.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/xps/FixedDocument.h"
#include "dwf/xps/Constants.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const XPSFixedDocument::kzName = "FixedDocument.fdoc";
//DNT_End

#endif


_DWFTK_API
XPSFixedDocument::XPSFixedDocument()
throw()
    : _oFixedPages()
{
    setName( XPSFixedDocument::kzName );
}


_DWFTK_API
XPSFixedDocument::~XPSFixedDocument()
throw()
{
    DWFPointer<XPSFixedPage::tIterator> piPages( _oFixedPages.iterator(), false );
    if (!piPages.isNull())
    {
        for (; piPages->valid(); piPages->next())
        {
            XPSFixedPage* pPage = piPages->get();
            if (pPage->owner() == this)
            {
                pPage->disown( *this, true );
                DWFCORE_FREE_OBJECT( pPage );
            }
            else
            {
                pPage->unobserve( *this );
            }
        }
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
bool
XPSFixedDocument::addFixedPage( XPSFixedPage* pFixedPage,
                                bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oFixedPages.findFirst( pFixedPage, iPosition ))
    {
        //
        //  If the page is already in the sequence we cannot add it again
        //
        return false;
    }
    else
    {
        _oFixedPages.push_back( pFixedPage );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pFixedPage->own( *this );
        }
        else
        {
            pFixedPage->observe( *this );
        }

        return true;
    }
}

_DWFTK_API
bool 
XPSFixedDocument::insertFixedPage( XPSFixedPage* pFixedPage, 
                                   XPSFixedPage* pAfterPage,
                                   bool bOwn )
throw()
{
    size_t iPosition = 0;
    if (_oFixedPages.findFirst( pFixedPage, iPosition ))
    {
        //
        //  If the page is already in the sequence we cannot add it again
        //
        return false;
    }
    else
    {
        iPosition = 0;

        if (pAfterPage != NULL)
        {
            if (_oFixedPages.findFirst( pAfterPage, iPosition ))
            {
                iPosition++;
            }
            else
            {
                //
                // If pAfterPage is not NULL, it has to be in the list
                //
                return false;
            }
        }

        //
        //  Now add the page to the container in the document
        //
        _oFixedPages.insertAt( pFixedPage, iPosition );

        //
        //  Manage the ownership as requested
        //
        if (bOwn)
        {
            pFixedPage->own( *this );
        }
        else
        {
            pFixedPage->observe( *this );
        }

        return true;
    }
}

_DWFTK_API
bool
XPSFixedDocument::removeFixedPage( XPSFixedPage* pFixedPage )
throw()
{
    if (_oFixedPages.erase( pFixedPage ))
    {
        if (pFixedPage->owner() == this)
        {
            pFixedPage->disown( *this, true );
        }
        else
        {
            pFixedPage->unobserve( *this );
        }

        return true;
    }
    else
    {
        return false;
    }
}


_DWFTK_API
XPSFixedPage* const
XPSFixedDocument::fixedPage( const DWFString& zURI ) const
throw()
{
    if (_oFixedPages.size() > 0)
    {
        DWFPointer<XPSFixedPage::tConstIterator> piPages( _oFixedPages.constIterator(), false );
        if (!piPages.isNull())
        {
            while (piPages->valid())
            {
                XPSFixedPage* pPage = piPages->get();
                if (pPage->uri() == zURI)
                {
                    return pPage;
                }
                piPages->next();
            }
        }
    }

    return NULL;
}

_DWFTK_API
void
XPSFixedDocument::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    //
    //  If an ownable is getting destroyed we just need to remove
    //  any references to it.
    //

    size_t n = 0;
    for (; n<_oFixedPages.size(); n++)
    {
        DWFOwnable* pOwnable = (DWFOwnable*)(_oFixedPages[n]);
        if (pOwnable == (&rOwnable))
        {
            _oFixedPages.eraseAt( n );
            return;
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void XPSFixedDocument::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();
    rSerializer.startElement( XPSXML::kzElement_FixedDocument );
    rSerializer.addAttribute( /*NOXLATE*/L"xmlns", XPSXML::kzNamespaceURI_XPS );

    DWFPointer<XPSFixedPage::tIterator> piPages( _oFixedPages.iterator(), false );
    if (!piPages.isNull())
    {
        while (piPages->valid())
        {
            XPSFixedPage* pPage = piPages->get();

            if (pPage != NULL)
            {
                rSerializer.startElement( XPSXML::kzElement_PageContent );
                rSerializer.addAttribute( XPSXML::kzAttribute_Source, pPage->uri() );
                if (pPage->width() > 0)
                {
                    rSerializer.addAttribute( XPSXML::kzAttribute_Width, pPage->width() );
                }
                if (pPage->height() > 0)
                {
                    rSerializer.addAttribute( XPSXML::kzAttribute_Height, pPage->height() );
                }
                rSerializer.endElement();
            }

            piPages->next();
        }
    }

    rSerializer.endElement();
}

#endif


