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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/utility/PropertyReferenceContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/presentation/utility/PropertyReferenceContainer.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFPropertyReferenceContainer::DWFPropertyReferenceContainer()
throw()
    : _oPropertyReferences()
{

#ifndef DWFTK_READ_ONLY
    _oSerializer.is( this );
#endif

}

_DWFTK_API
DWFPropertyReferenceContainer::~DWFPropertyReferenceContainer()
throw()
{
    //
    // delete all property references we own
    //
    DWFPropertyReference::tList::Iterator* piRefs = getPropertyReferences();

    if (piRefs)
    {
        for (piRefs->reset(); piRefs->valid(); piRefs->next())
        {
            DWFCORE_FREE_OBJECT(piRefs->get());
        }
    }
    
    DWFCORE_FREE_OBJECT( piRefs );
}

_DWFTK_API
void
DWFPropertyReferenceContainer::addPropertyReference( DWFPropertyReference* pPropertyReference )
throw( DWFException )
{
    if (pPropertyReference == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A non-null property reference must be provided" );
    }

    size_t tIndex = _oPropertyReferences.size();
    const DWFString& zID = pPropertyReference->id();
    //
    // if we have an ID check for duplicate and replace it if it already exists
    //
    if (zID.chars())
    {
        DWFPropertyReference::tList::Iterator* piPropertyReferences = getPropertyReferences();
        for( tIndex = 0; piPropertyReferences && piPropertyReferences->valid(); piPropertyReferences->next(), tIndex++)
        {
            DWFPropertyReference* pExistingPropertyReference = piPropertyReferences->get();
            if (pExistingPropertyReference->id() == zID)
            {
                _oPropertyReferences.eraseAt( tIndex );
                DWFCORE_FREE_OBJECT( pExistingPropertyReference );
                break;
            }
        }

        DWFCORE_FREE_OBJECT( piPropertyReferences );
    }

    _oPropertyReferences.insertAt(pPropertyReference, tIndex);
}

_DWFTK_API
DWFPropertyReference::tList::Iterator*
DWFPropertyReferenceContainer::getPropertyReferences()
throw()
{
    return _oPropertyReferences.iterator();
}

_DWFTK_API
void
DWFPropertyReferenceContainer::removePropertyReference( DWFPropertyReference* pPropertyReference,
                                                        bool                  bDelete )
throw( DWFException )
{
    _oPropertyReferences.erase(pPropertyReference);
    
    if (bDelete)
    {
        DWFCORE_FREE_OBJECT(pPropertyReference);
    }    
}


#ifndef DWFTK_READ_ONLY

void
DWFPropertyReferenceContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFPropertyReference::tList::Iterator* piPropertyReferences = _pContainer->_oPropertyReferences.iterator();

        if (piPropertyReferences && piPropertyReferences->valid())
        {
            DWFString zNamespace;

                //
                // namespace dictated by document and section type 
                //
            if (nFlags & DWFPackageWriter::eDescriptor)
            {
                zNamespace.assign( namespaceXML(nFlags) );
            }

            rSerializer.startElement( DWFXML::kzElement_PropertyReferences, zNamespace );
            {
                if (piPropertyReferences)
                {
                    for (; piPropertyReferences->valid(); piPropertyReferences->next())
                    {            
                        piPropertyReferences->get()->serializeXML( rSerializer, nFlags );
                    }
                }
            }

            rSerializer.endElement();
        }

        if (piPropertyReferences)
        {
            DWFCORE_FREE_OBJECT( piPropertyReferences );
        }
    }
    else
    {
        DWFPropertyReference::tList::Iterator* piPropertyReferences = _pContainer->_oPropertyReferences.iterator();

        if (piPropertyReferences && piPropertyReferences->valid())
        {
            for (; piPropertyReferences->valid(); piPropertyReferences->next())
            {            
                piPropertyReferences->get()->serializeXML( rSerializer, nFlags );
            }
        }

        if (piPropertyReferences)
        {
            DWFCORE_FREE_OBJECT( piPropertyReferences );
        }
    }
}

#endif

