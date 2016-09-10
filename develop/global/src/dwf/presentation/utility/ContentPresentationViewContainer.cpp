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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/utility/ContentPresentationViewContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/utility/ContentPresentationViewContainer.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationViewContainer::DWFContentPresentationViewContainer()
throw()
    : _oViews()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFContentPresentationViewContainer::~DWFContentPresentationViewContainer()
throw()
{
    //
    // delete all views we own
    //
    DWFContentPresentationView::tList::Iterator* piViews = getViews();

    if (piViews)
    {
        for (piViews->reset(); piViews->valid(); piViews->next())
        {
            DWFCORE_FREE_OBJECT(piViews->get());
        }
    }
    
    DWFCORE_FREE_OBJECT( piViews );
}

_DWFTK_API
void
DWFContentPresentationViewContainer::addView( DWFContentPresentationView* pView )
throw( DWFException )
{
    if (pView == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A non-null view must be provided" );
    }

    size_t tIndex = _oViews.size();
	size_t* tempPos = NULL;
    const DWFString& zID = pView->id();
	DWFString	tempID(zID);
    //
    // if we have an ID check for duplicate and replace it if it already exists
    //
    if (zID.chars())
    {	
		if (tIndex > 0)
		{
			tempPos = posMap.find(tempID);
			if (tempPos)
			{
				tIndex = *tempPos;
				DWFContentPresentationView* pExistingView = _oViews[tIndex];
				_oViews.eraseAt( tIndex );
				DWFCORE_FREE_OBJECT( pExistingView );
			}			 
		}	
	}

    pView->setParent( this );
    _oViews.insertAt( pView, tIndex );

	if (zID.chars() > 0)
	{
		posMap.insert(tempID, tIndex, true);
	}
	else
	{
		posMap.insert(tempID, tIndex, false);
	}
}

_DWFTK_API
DWFContentPresentationView::tList::Iterator*
DWFContentPresentationViewContainer::getViews( )
throw()
{
    return _oViews.iterator();
}

_DWFTK_API
void
DWFContentPresentationViewContainer::removeView( DWFContentPresentationView* pView,
                                                 bool                        bDelete )
throw( DWFException )
{
    if(pView != NULL)
    {
        pView->setParent(NULL);

        const DWFString& zID = pView->id();
        DWFString   tempID(zID);
		posMap.erase(tempID);

        _oViews.erase(pView);

        
        if (bDelete)
        {
            DWFCORE_FREE_OBJECT(pView);
        }    
    }
}

#ifndef DWFTK_READ_ONLY

void
DWFContentPresentationViewContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFContentPresentationView::tList::Iterator* piView = _pContainer->_oViews.iterator();

        if (piView && piView->valid())
        {
            DWFString zNamespace;

                //
                // namespace dictated by document and section type 
                //
            if (nFlags & DWFPackageWriter::eDescriptor)
            {
                zNamespace.assign( namespaceXML(nFlags) );
            }

            rSerializer.startElement( DWFXML::kzElement_Views, zNamespace );
            {
                for (; piView->valid(); piView->next())
                {            
                    piView->get()->serializeXML( rSerializer, nFlags );
                }
            }

            rSerializer.endElement();
        }

        if (piView)
        {
            DWFCORE_FREE_OBJECT( piView );
        }
    }
    else
    {
        DWFContentPresentationView::tList::Iterator* piView = _pContainer->_oViews.iterator();

        if (piView && piView->valid())
        {
            for (; piView->valid(); piView->next())
            {            
                piView->get()->serializeXML( rSerializer, nFlags );
            }
        }

        if (piView)
        {
            DWFCORE_FREE_OBJECT( piView );
        }
    }
}

#endif
