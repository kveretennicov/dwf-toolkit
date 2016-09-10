//
//  Copyright (c) 2004-2006 by Autodesk, Inc.  All rights reserved.
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

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Constants.h"
#include "dwf/package/utility/DefinedObjectContainer.h"
using namespace DWFToolkit;



_DWFTK_API
DWFDefinedObjectContainer::DWFDefinedObjectContainer()
throw()
    : _oKeyedObjects()
    , _oObjectProperties()
    , _oGroupedObjects()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFDefinedObjectContainer::~DWFDefinedObjectContainer()
throw()
{
    _tGroupedObjectMap::iterator iObject = _oGroupedObjects.begin();
    for(; iObject != _oGroupedObjects.end(); iObject++)
    {
        DWFCORE_FREE_OBJECT( iObject->second );
    }
}

_DWFTK_API
void
DWFDefinedObjectContainer::addObject( DWFDefinedObject* pObject, uint32_t nKey )
throw()
{
    if (pObject)
    {
        _oGroupedObjects.insert( _tGroupedObjectMap::value_type(nKey, pObject) );

            //
            // if the object has an ID, then stuff it in a map for look up
            //
        if (pObject->id().bytes() > 0)
        {
            // If using Skiplists:
            //_oKeyedObjects.insert( pObject->id(), pObject );
            // else if using DWF STL Maps:
			_oKeyedObjects[pObject->id()] = pObject;
        }
    }
}

_DWFTK_API
void
DWFDefinedObjectContainer::removeObject( DWFDefinedObject* pObject )
throw()
{
    if (pObject)
    {
        _tGroupedObjectMap::iterator iObject = _oGroupedObjects.begin();
        for(; iObject != _oGroupedObjects.end(); iObject++)
        {
            if( iObject->second == pObject )
            {
                _oGroupedObjects.erase( iObject );
                break;
            }
        }

            //
            // if the object has an ID, the erase it from that map too.
            //
        if (pObject->id().bytes() > 0)
        {
            _oKeyedObjects.erase(pObject->id());
        }
    }
}

_DWFTK_API
DWFDefinedObject*
DWFDefinedObjectContainer::findObject( const DWFString& zID )
throw( DWFException )
{
    // If using Skiplists:
    //DWFDefinedObject** ppObject = _oKeyedObjects.find( zID );
    //return (ppObject ? *ppObject : NULL);
    // else if using DWF STL Maps:
    DWFDefinedObject* pObject = NULL;
	DWFDefinedObject::tMap::iterator i = _oKeyedObjects.find( zID );
	if (i != _oKeyedObjects.end())
	{
		pObject = i->second;
	}
    return pObject;
}

_DWFTK_API
bool
DWFDefinedObjectContainer::empty() const
throw()
{
    return _oGroupedObjects.empty();
}

_DWFTK_API
DWFPropertyContainer&
DWFDefinedObjectContainer::getObjectProperties()
throw()
{
    return _oObjectProperties;
}


#ifndef DWFTK_READ_ONLY

void
DWFDefinedObjectContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    _tGroupedObjectMap::iterator iObject = _pContainer->_oGroupedObjects.begin();

    if (iObject != _pContainer->_oGroupedObjects.end())
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
        // Object Properties
        //
        _pContainer->_oObjectProperties.getSerializable().serializeXML( rSerializer, nFlags );

        //
        // Objects
        //

        DWFDefinedObject* pObject = NULL;

        rSerializer.startElement( DWFXML::kzElement_Objects, zNamespace );
        {
            for (; iObject != _pContainer->_oGroupedObjects.end(); iObject++)
            {
                pObject = iObject->second;

                    //
                    // 
                    //
                if (pObject->id().bytes() == 0)
                {
                    pObject->identify( rSerializer.nextUUID(true) );
                }

                pObject->serializeXML( rSerializer, nFlags );
            }
        }
        rSerializer.endElement();
    }
}

#endif

///
///
///

_DWFTK_API
DWFDefinedObjectInstanceContainer::DWFDefinedObjectInstanceContainer()
throw()
    : _oGroupedInstances()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFDefinedObjectInstanceContainer::~DWFDefinedObjectInstanceContainer()
throw()
{
    _tGroupedInstanceMap::iterator iInstance = _oGroupedInstances.begin();
    for(; iInstance != _oGroupedInstances.end(); iInstance++)
    {
        DWFCORE_FREE_OBJECT( iInstance->second );
    }
}

_DWFTK_API
void
DWFDefinedObjectInstanceContainer::addInstance( DWFDefinedObjectInstance* pInstance, uint32_t nKey )
throw()
{
    if (pInstance)
    {
        _oGroupedInstances.insert( _tGroupedInstanceMap::value_type(nKey, pInstance) );
    }
}

_DWFTK_API
void
DWFDefinedObjectInstanceContainer::removeInstance( DWFDefinedObjectInstance* pInstance )
throw()
{
    if (pInstance)
    {
        _tGroupedInstanceMap::iterator iInstance = _oGroupedInstances.begin();
        for(; iInstance != _oGroupedInstances.end(); iInstance++)
        {
            if( iInstance->second == pInstance )
            {
                _oGroupedInstances.erase( iInstance );
                break;
            }
        }
    }
}

_DWFTK_API
bool
DWFDefinedObjectInstanceContainer::empty() const
throw()
{
    return _oGroupedInstances.empty();
}

#ifndef DWFTK_READ_ONLY

void
DWFDefinedObjectInstanceContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    _tGroupedInstanceMap::const_iterator iInstance  = _pContainer->_oGroupedInstances.begin();

    if (iInstance != _pContainer->_oGroupedInstances.end())
    {
        DWFString zNamespace;

            //
            // namespace dictated by document and section type 
            //
        if (nFlags & DWFPackageWriter::eDescriptor)
        {
            zNamespace.assign( namespaceXML(nFlags) );
        }

        rSerializer.startElement( DWFXML::kzElement_Instances, zNamespace );
        {
            for (; iInstance != _pContainer->_oGroupedInstances.end(); iInstance++)
            {
                ((DWFDefinedObjectInstance*)(iInstance->second))->serializeXML( rSerializer, nFlags );
            }
        }
        rSerializer.endElement();
    }
}

#endif

