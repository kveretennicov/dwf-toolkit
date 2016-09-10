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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/utility/ContentPresentationContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Constants.h"
#include "dwf/presentation/utility/ContentPresentationContainer.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationContainer::DWFContentPresentationContainer()
throw()
    : _oPresentations()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFContentPresentationContainer::~DWFContentPresentationContainer()
throw()
{
    //
    // delete all presentations we own
    //
    DWFContentPresentation::tList::Iterator* piPresentations = getPresentations();

    if (piPresentations)
    {
        for (piPresentations->reset(); piPresentations->valid(); piPresentations->next())
        {
            DWFContentPresentation* pPresentation = piPresentations->get();
                //
                // delete if owned
                //
            if (pPresentation->owner() == this)
            {
                DWFCORE_FREE_OBJECT( pPresentation );
            }
                //
                // remove from observation if not owned
                //
            else
            {
                pPresentation->unobserve(*this);
            }
        }
    }

    DWFCORE_FREE_OBJECT( piPresentations );
}

_DWFTK_API
void
DWFContentPresentationContainer::addPresentation( DWFContentPresentation* pPresentation, bool bOwnPresentation )
throw( DWFException )
{
    if (pPresentation == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A non-null presentation must be provided" );
    }

        //
        // claim ownership
        //
    if (bOwnPresentation)
    {
        pPresentation->own( *this );
    }
        //
        // inform that we're holding a pointer to this
        // presentation, and need to be told of its demise.
        //
    else
    {
        pPresentation->observe( *this );
    }
    size_t tIndex = _oPresentations.size();
	size_t* tempPos = NULL;
    const DWFString& zID = pPresentation->id();
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
				DWFContentPresentation* pExistingPresentation = _oPresentations[tIndex];
				_oPresentations.eraseAt( tIndex );
				DWFCORE_FREE_OBJECT( pExistingPresentation );
			}			 
		}
	}

	pPresentation->setParent( this );
	_oPresentations.insertAt( pPresentation, tIndex );

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
DWFContentPresentation::tList::Iterator*
DWFContentPresentationContainer::getPresentations( )
throw()
{
    return _oPresentations.iterator();
}

_DWFTK_API
DWFContentPresentation::tList::ConstIterator*
DWFContentPresentationContainer::getPresentations( ) const
throw()
{
    return _oPresentations.constIterator();
}

_DWFTK_API
void
DWFContentPresentationContainer::removePresentation( DWFContentPresentation* pPresentation,
                                                     bool                    bDeleteIfOwned )
throw( DWFException )
{
    const DWFString& zID = pPresentation->id();
    DWFString   tempID(zID);
	posMap.erase(tempID);

    _oPresentations.erase(pPresentation);
    
    if (bDeleteIfOwned && pPresentation->owner() == this)
    {
        DWFCORE_FREE_OBJECT(pPresentation);
        return;
    }
    else
    {
        if(pPresentation->owner() == this)
        {
            pPresentation->disown( *this, true );
        }
        else
        {
            pPresentation->unobserve( *this );
        }

        return;
    }
}

_DWFTK_API
void
DWFContentPresentationContainer::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFContentPresentationContainer::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    DWFContentPresentation* pPresentation = dynamic_cast<DWFContentPresentation*>(&rOwnable);

    if (pPresentation)
    {
        const DWFString& zID = pPresentation->id();
        DWFString   tempID(L" ");
        if (zID.chars())
        {
            tempID = zID;
        }
        posMap.erase(tempID);

        _oPresentations.erase( pPresentation );
    }
}

#ifndef DWFTK_READ_ONLY

void
DWFContentPresentationContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFContentPresentation::tList::Iterator* piContentPresentation = _pContainer->_oPresentations.iterator();

        if (piContentPresentation && piContentPresentation->valid())
        {
            DWFString zNamespace;

                //
                // namespace dictated by document and section type 
                //
            if (nFlags & DWFPackageWriter::eDescriptor)
            {
                zNamespace.assign( namespaceXML(nFlags) );
            }

            rSerializer.startElement( DWFXML::kzElement_Presentations, zNamespace );
            {
                if (piContentPresentation)
                {
                    for (; piContentPresentation->valid(); piContentPresentation->next())
                    {
                        piContentPresentation->get()->serializeXML( rSerializer, nFlags );
                    }
                }
            }

            rSerializer.endElement();
        }

        if (piContentPresentation)
        {
            DWFCORE_FREE_OBJECT( piContentPresentation );
        }
    }
    else
    {
        DWFContentPresentation::tList::Iterator* piContentPresentation = _pContainer->_oPresentations.iterator();

        if (piContentPresentation && piContentPresentation->valid())
        {
            for (; piContentPresentation->valid(); piContentPresentation->next())
            {
                piContentPresentation->get()->serializeXML( rSerializer, nFlags );
            }
        }

        if (piContentPresentation)
        {
            DWFCORE_FREE_OBJECT( piContentPresentation );
        }
    }
}

#endif

