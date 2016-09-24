//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
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

#ifdef _MSC_VER
// Earlier MSC compilers did not have <cstdint>.
#include <stddef.h>
typedef intptr_t std_intptr_t;
#else
#include <cstdint>
typedef std::intptr_t std_intptr_t;
#endif

#include "dwfcore/Timer.h"
using namespace DWFCore;

#include "dwf/package/Constants.h"
#include "dwf/package/DefinedObject.h"
#include "dwfcore/DWFXMLSerializer.h"
using namespace DWFToolkit;


_DWFTK_API
DWFDefinedObject::DWFDefinedObject( const DWFString& zID )
throw()
                : _zID( zID )
                , _oPropertyRefs()
                , _oInstances()
                , _oChildren()
{
    ;
}

_DWFTK_API
DWFDefinedObject::~DWFDefinedObject()
throw()
{
    ;
}

_DWFTK_API
const DWFString&
DWFDefinedObject::id() const
throw()
{
    return _zID;
}

_DWFTK_API
void
DWFDefinedObject::identify( const DWFString& zID )
throw()
{
    _zID = zID;
}

_DWFTK_API
DWFIterator<DWFString>*
DWFDefinedObject::getPropertyRefs()
throw()
{
    return DWFCORE_ALLOC_OBJECT( _tStringVectorIterator(_oPropertyRefs) );
}

_DWFTK_API
void
DWFDefinedObject::addPropertyReference( const char* zReferenceID )
throw( DWFException )
{
    if (zReferenceID)
    {
        _oPropertyRefs.push_back( zReferenceID );
    }
}

_DWFTK_API
DWFDefinedObjectInstance*
DWFDefinedObject::instance( const DWFString& zNode )
throw( DWFException )
{
    if (zNode.bytes() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Instance node must be specified" );
    }

    DWFDefinedObjectInstance* pInst = DWFCORE_ALLOC_OBJECT( DWFDefinedObjectInstance(*this, zNode) );

    if (pInst == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate instance" );
    }

    // If using SkipLists:
    // _oInstances.insert( pInst->node(), pInst );
    // else if using STL Maps:
    _oInstances[pInst->node()] = pInst;

    return pInst;
}

_DWFTK_API
DWFDefinedObjectInstance*
DWFDefinedObject::getInstance( const DWFString& zNode )
throw( DWFException )
{
    if (zNode.bytes() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Instance node must be specified" );
    }

    // If using SkipLists:
    //DWFDefinedObject** ppInstance = _oInstances.find( zNode );
    //return (ppInstance ? dynamic_cast<DWFDefinedObjectInstance*>(*ppInstance) : NULL);
    // else if using STL Maps:
	DWFDefinedObject::tMap::iterator i = _oInstances.find( zNode );
	if (i != _oInstances.end())
	{
		return dynamic_cast<DWFDefinedObjectInstance*>(i->second);
	}
	else
	{
		return NULL;
	}

}

_DWFTK_API
void
DWFDefinedObject::removeInstance( const DWFString& zNode )
throw( DWFException )
{
    _oInstances.erase(zNode);
}

_DWFTK_API
void
DWFDefinedObject::addChild( DWFDefinedObject* pChild, uint32_t nKey )
throw()
{
    if (pChild)
    {
        _oChildren.insert( _tGroupedChildMap::value_type(nKey, pChild) );
    }
}

_DWFTK_API
void
DWFDefinedObject::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

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
            // set the ID
            //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x01;

            _zID.assign( ppAttributeList[iAttrib+1] );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFDefinedObject::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
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

    rSerializer.startElement( DWFXML::kzElement_Object, zNamespace );
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );

        DWFString zChildren;
        DWFDefinedObject* pChild = NULL;
        _tGroupedChildMap::const_iterator iChild = _oChildren.begin();

        for (; iChild != _oChildren.end(); iChild++)
        {
            pChild = iChild->second;

            if (pChild->id().bytes() == 0)
            {
                pChild->identify( rSerializer.nextUUID(true) );
            }

            zChildren.append( pChild->id() );
            zChildren.append( /*NOXLATE*/L" " );
        }

        if (zChildren.bytes() > 0)
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Children, zChildren );
        }

        //
        // properties
        //
        DWFPropertyContainer::getSerializable().serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}

#endif

///
///
///


_DWFTK_API
DWFDefinedObjectInstance::DWFDefinedObjectInstance( unsigned long nSequence )
throw()
                        : DWFDefinedObject()
                        , _nSequence( nSequence )
                        , _zNode()
                        , _zObject()
                        , _oChildIDs()
                        , _oChildren()
                        , _pResolvedObject( NULL )
                        , _pResolvedParent( NULL )
                        , _oResolvedChildren()
{
    ;
}

_DWFTK_API
DWFDefinedObjectInstance::DWFDefinedObjectInstance( DWFDefinedObject& rObject,
                                                    const DWFString&  zNode )
throw()
                        : DWFDefinedObject( /*NOXLATE*/L"" )
                        , _nSequence( 0 )
                        , _zNode( zNode )
                        , _zObject()
                        , _oChildIDs()
                        , _oChildren()
                        , _pResolvedObject( &rObject )
                        , _pResolvedParent( NULL )
                        , _oResolvedChildren()
{
    ;
}

_DWFTK_API
DWFDefinedObjectInstance::~DWFDefinedObjectInstance()
throw()
{
    ;
}

_DWFTK_API
const DWFString&
DWFDefinedObjectInstance::node() const
throw()
{
    return _zNode;
}

_DWFTK_API
const DWFString&
DWFDefinedObjectInstance::object() const
throw()
{
    return _zObject;
}

_DWFTK_API
unsigned long
DWFDefinedObjectInstance::sequence() const
throw()
{
    return _nSequence;
}

_DWFTK_API
DWFIterator<DWFString>*
DWFDefinedObjectInstance::children()
throw()
{
    return DWFCORE_ALLOC_OBJECT( _tStringVectorIterator(_oChildIDs) );
}

_DWFTK_API
void
DWFDefinedObjectInstance::addChild( DWFDefinedObjectInstance* pChild, uint32_t nKey )
throw()
{
    if (pChild)
    {
        _oChildren.insert( _tGroupedChildMap::value_type(nKey, pChild));
    }
}

_DWFTK_API
void
DWFDefinedObjectInstance::resolveObject( DWFDefinedObject* pObject )
throw()
{
    _pResolvedObject = pObject;
}

_DWFTK_API
void
DWFDefinedObjectInstance::resolveChild( DWFDefinedObjectInstance* pInstance )
throw()
{
    if (pInstance)
    {
        // If using SkipLists:
        //_oResolvedChildren.insert( pInstance->id(), pInstance );
        // else if using STL Maps:
        _oResolvedChildren[pInstance->id()] = pInstance;
    }
}

_DWFTK_API
void
DWFDefinedObjectInstance::resolveParent( DWFDefinedObjectInstance* pInstance )
throw()
{
    _pResolvedParent = pInstance;
}

_DWFTK_API
DWFDefinedObject*
DWFDefinedObjectInstance::resolvedObject() const
throw()
{
    return _pResolvedObject;
}

_DWFTK_API
DWFDefinedObjectInstance*
DWFDefinedObjectInstance::resolvedParent() const
throw()
{
    return _pResolvedParent;
}

_DWFTK_API
DWFDefinedObjectInstance::tMap::Iterator*
DWFDefinedObjectInstance::resolvedChildren()
throw()
{
    // If using SkipLists:
    // return _oResolvedChildren.iterator();
    // else if using STL Maps:
    return _oResolvedChildren.getIterator();
}

_DWFTK_API
void
DWFDefinedObjectInstance::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    //
    // base parse
    //
    DWFDefinedObject::parseAttributeList( ppAttributeList );


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
            // set the ID of the instanced Object
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Object) == 0))
        {
            nFound |= 0x01;

            _zObject.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the node reference
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Nodes) == 0))
        {
            nFound |= 0x02;

            _zNode.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the children
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Children) == 0))
        {
            nFound |= 0x04;

            char* pChildren = (char*)ppAttributeList[iAttrib+1];
            char* pSavePtr;
            char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pChildren, /*NOXLATE*/" ", &pSavePtr );

            while (pToken)
            {
                _oChildIDs.push_back( pToken );

                pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            }
        }
    }

        //
        // no ID was set - we will need to generate and assign one
        //
    if (_zID.bytes() == 0)
    {
        //
        // convert the instance pointer address as a string, and set to the instance id.
        //
        // Note: on 64-bit MSC++ (and any other 64-bit compilers with 32-bit longs)
        // this truncates the pointer value to 32 bit.
        long nId = static_cast<long>(reinterpret_cast<std_intptr_t>(this));
        int nBufferSize = 32;
        DWFPointer<wchar_t> zBuffer( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
        int nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zBuffer, nBufferSize, L"%d", nId );
        _zID.assign( zBuffer, nBytes );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFDefinedObjectInstance::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
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

    rSerializer.startElement( DWFXML::kzElement_Instance, zNamespace );
    {
        if (_zID.bytes() == 0)
        {
            _zID.assign( rSerializer.nextUUID(true) );
        }

        rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );

        if ((_zObject.bytes() == 0) && _pResolvedObject)
        {
            _zObject.assign( _pResolvedObject->id() );
        }

        rSerializer.addAttribute( DWFXML::kzAttribute_Object, _zObject );
        rSerializer.addAttribute( DWFXML::kzAttribute_Nodes, _zNode );

        DWFString zChildren;
        DWFDefinedObjectInstance* pChild = NULL;
        _tGroupedChildMap::const_iterator iChild = _oChildren.begin();

        for (; iChild != _oChildren.end(); iChild++)
        {
            pChild = iChild->second;

            if (pChild->id().bytes() == 0)
            {
                pChild->identify( rSerializer.nextUUID(true) );
            }

            zChildren.append( pChild->id() );
            zChildren.append( /*NOXLATE*/L" " );
        }

        if (zChildren.bytes() > 0)
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Children, zChildren );
        }

        //
        // properties
        //
        DWFPropertyContainer::getSerializable().serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}

#endif
