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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/PartContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/PartContainer.h"

using namespace DWFToolkit;


_DWFTK_API
OPCPartContainer::OPCPartContainer()
throw()
                : _oParts()
{;}

_DWFTK_API
OPCPartContainer::~OPCPartContainer()
throw()
{
    //
    //  Gather the parts that the container does not own, and unobserve
    //  the rest. 
    //
    //  The reason is that a number of the observed parts are actually
    //  descendents of parts owned by the container. If a top level part
    //  is deleted by the container, the descendents are also removed 
    //  from the parts list, but any iterator of the container would be 
    //  a static copy of the list it started with and hence it would
    //  return garbage.
    //
    DWFPointer<OPCPart::tCachingIterator> piOwnedParts( DWFCORE_ALLOC_OBJECT(OPCPart::tCachingIterator), false );

    DWFPointer<OPCPart::tList::Iterator> piParts( _oParts.iterator(), false );
    if (!piParts.isNull())
    {
        for(; piParts->valid(); piParts->next())
        {
            OPCPart* pPart = piParts->get();

            if (pPart)
            {
                if (pPart->owner() == this)
                {
                    piOwnedParts->add( pPart );
                }
                else
                {
                    pPart->unobserve( *this );
                }
            }
        }
    }

    for (; piOwnedParts->valid(); piOwnedParts->next())
    {
        OPCPart* pPart = piOwnedParts->get();
        DWFCORE_FREE_OBJECT( pPart );
    }
}

_DWFTK_API
void OPCPartContainer::addPart( OPCPart* pPart, bool bOwn )
throw()
{
    _oParts.push_back( pPart );
    if (bOwn)
    {
        pPart->own( *this );
    }
    else
    {
        pPart->observe( *this );
    }
}


_DWFTK_API
bool OPCPartContainer::insertPart( OPCPart* pPart, OPCPart* pAfterPart, bool bOwn )
throw()
{
    if (pAfterPart == NULL)
    {
        _oParts.insert( pPart, OPCPart::tList::eFront );
    }
    else
    {
        size_t iLocation = 0;
        if (_oParts.findFirst( pAfterPart, iLocation ))
        {
            _oParts.insertAt( pPart, iLocation+1 );
        }
        else
        {
            return false;
        }
    }

    if (bOwn)
    {
        pPart->own( *this );
    }
    else
    {
        pPart->observe( *this );
    }

    return true;
}


_DWFTK_API
bool OPCPartContainer::removePart( OPCPart* pPart, bool bOwnerDelete )
throw()
{
    if (_oParts.erase( pPart ))
    {
        if (pPart->owner() == this)
        {
            pPart->disown( *this, true );
            if (bOwnerDelete)
            {
                DWFCORE_FREE_OBJECT( pPart );
            }
        }
        else
        {
            pPart->unobserve( *this );
        }
        return true;
    }
    else
    {
        return false;
    }
}


_DWFTK_API
OPCPart::tIterator* OPCPartContainer::parts()
throw()
{
    return _oParts.iterator();
}

_DWFTK_API
OPCPart* OPCPartContainer::part( DWFString zURI )
throw()
{
    if (_oParts.size() > 0)
    {
        OPCPart::tIterator* piParts = _oParts.iterator();
        if (piParts != NULL)
        {
            while (piParts->valid())
            {
                OPCPart* pPart = piParts->get();
                if (pPart->uri() == zURI)
                {
                    //
                    //  Cleanup before returning
                    //
                    DWFCORE_FREE_OBJECT( piParts );
                    return pPart;
                }
            }
            DWFCORE_FREE_OBJECT( piParts );
        }
    }

    return NULL;
}

_DWFTK_API
void
OPCPartContainer::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    size_t n = 0;
    for (; n<_oParts.size(); n++)
    {
        DWFOwnable* pOwnable = (DWFOwnable*)(_oParts[n]);
        if (pOwnable == (&rOwnable))
        {
            _oParts.eraseAt( n );
            return;
        }
    }
}


