//
//  Copyright (c) 2005-2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/ContentReader.cpp#2 $
//  $DateTime: 2011/10/11 01:26:00 $
//  $Author: caos $
//  $Change: 229047 $
//  $Revision: #2 $
//

#include "dwf/package/Class.h"
#include "dwf/package/Group.h"
#include "dwf/package/Entity.h"
#include "dwf/package/Object.h"
#include "dwf/package/Feature.h"
#include "dwf/package/Constants.h"
#include "dwf/package/PropertySet.h"
#include "dwf/package/reader/ContentReader.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentReader::DWFContentReader( unsigned int nProviderFlags )
throw()
                                    : DWFXMLCallback()
                                    , _oDefaultElementBuilder()
                                    , _pElementBuilder( NULL )
                                    , _nCurrentCollectionProvider( eProvideNone )
                                    , _nProviderFlags( nProviderFlags )
                                    , _pCurrentElement( NULL )
                                    , _pFilter( NULL )
                                    , _oPropertyContainer()
                                    , _oUnresolved()
{
    _pElementBuilder = &_oDefaultElementBuilder;
}

_DWFTK_API
DWFContentReader::~DWFContentReader()
throw()
{
}

////

_DWFTK_API
void
DWFContentReader::notifyStartElement( const char*   zName,
                                      const char**  ppAttributeList )
throw()
{
    //
    // skip over any "dwf:" in the element name
    //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }

    switch (_nElementDepth)
    {
        //
        // dwf:Content
        //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Content) == 0)
            {
                if (_nProviderFlags & eProvideAttributes)
                {
                    size_t iAttrib = 0;
                    unsigned char nFound = eProvideNone;
                    const char* pAttrib = NULL;

                    for(; ppAttributeList[iAttrib]; iAttrib += 2)
                    {
                        //
                        // skip over any "dwf:" in the attribute name
                        //
                        pAttrib = (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0) ?
                                   &ppAttributeList[iAttrib][4] :
                                   &ppAttributeList[iAttrib][0];
                            //
                            // provide the version
                            //
                        if ((_nProviderFlags & eProvideVersion) &&
                            !(nFound & eProvideVersion)         &&
                             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Version) == 0))
                        {
                            nFound |= eProvideVersion;

                            _provideVersion( ppAttributeList[iAttrib+1] );
                        }
                    }
                }
            }
                //
                // we have an invalid content here, turn off all subsequent
                // processing and indicate the error somewhere...
                //
            else
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }

        //
        // dwf:Classes, dwf:Features, dwf:Entities, dwf:Objects, dwf:Groups, dwf:SharedProperties
        //
        case 1:
        {
            if ((_nProviderFlags & eProvideClasses) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Classes) == 0))
            {
                _nCurrentCollectionProvider = eProvideClasses;
            }
            else if ((_nProviderFlags & eProvideFeatures) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Features) == 0))
            {
                _nCurrentCollectionProvider = eProvideFeatures;
            }
            else if ((_nProviderFlags & eProvideEntities) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Entities) == 0))
            {
                _nCurrentCollectionProvider = eProvideEntities;
            }
            else if ((_nProviderFlags & eProvideObjects) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Objects) == 0))
            {
                _nCurrentCollectionProvider = eProvideObjects;
            }
            else if ((_nProviderFlags & eProvideGroups) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Groups) == 0))
            {
                _nCurrentCollectionProvider = eProvideGroups;
            }
            else if ((_nProviderFlags & eProvideSharedProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SharedProperties) == 0))
            {
                _nCurrentCollectionProvider = eProvideSharedProperties;
            }
            else
            {
                _nCurrentCollectionProvider = eProvideNone;
            }

            break;
        }

        //
        // dwf:Class, dwf:Feature, dwf:Entity, dwf:Object (only ones with no parents), 
        // dwf:Group, dwf:PropertySet (for SharedProperties)
        //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideClasses) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Class) == 0))
            {
                //
                //  Create a new class content element
                //  Create the unresolved list first to prevent an unecessary copy.
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFClass* pClass = _pElementBuilder->buildClass( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pClass );
            }
            else if ((_nCurrentCollectionProvider == eProvideFeatures) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Feature) == 0))
            {
                //
                //  Create a new feature content element
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFFeature* pFeature = _pElementBuilder->buildFeature( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pFeature );
            }
            else if ((_nCurrentCollectionProvider == eProvideEntities) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Entity) == 0))
            {
                //
                //  Create a new entity content element
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFEntity* pEntity = _pElementBuilder->buildEntity( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pEntity );
            }
            else if ((_nCurrentCollectionProvider == eProvideObjects) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Object) == 0))
            {
                //
                //  Create a new object content element (look in default: for nested objects)
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFObject* pObject = _pElementBuilder->buildObject( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pObject );
            }
            else if ((_nCurrentCollectionProvider == eProvideGroups) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Group) == 0))
            {
                //
                //  Create new group content element
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFGroup* pGroup = _pElementBuilder->buildGroup( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pGroup );
            }
            else if ((_nCurrentCollectionProvider == eProvideSharedProperties) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                //
                // create a new "shared" property set (look in default: for nested property sets)
                //
                _oUnresolved.push( tUnresolvedList() );
                DWFPropertySet* pPropertySet = _pElementBuilder->buildPropertySet( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pPropertySet );
            }
            break;
        }

        default:
        {
            //
            //  Do we have a nested object i.e. child object?
            //
            if (_nCurrentCollectionProvider == eProvideObjects &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Object) == 0))
            {
                _oUnresolved.push( tUnresolvedList() );
                DWFObject* pObject = _pElementBuilder->buildObject( ppAttributeList, _oUnresolved.top() );
                _oPropertyContainer.push( pObject );
            }
            else
            {
                //
                //  This is a nested property set
                //
                if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0)
                {
                    _oUnresolved.push( tUnresolvedList() );
                    DWFPropertySet* pPropertySet = _pElementBuilder->buildPropertySet( ppAttributeList, _oUnresolved.top() );
                    _oPropertyContainer.push( pPropertySet );
                }

                //
                //  It's a property on a property container, i.e. a property set or a content element.
                //
                else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0)
                {
                    DWFProperty* pProperty = _pElementBuilder->buildProperty( ppAttributeList, !(_nProviderFlags & eProvideCustomizeAttributes) );
                    _oPropertyContainer.top()->addProperty( pProperty, true );
                } 
            }
        }
    };

    _nElementDepth++;
}

_DWFTK_API
void
DWFContentReader::notifyEndElement( const char* zName )
throw()
{
    //
    //  Skip over any "dwf:" in the element name
    //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }

    //
    //  Perform the decrement before testing so the element closing depth matches the opening depth
    //
    switch (--_nElementDepth)
    {
        case 0:
        {
            resolvePropertySets();

            //
            // Reset
            //
            _nCurrentCollectionProvider = eProvideNone;
            _nProviderFlags = eProvideAll;
            _pCurrentElement = NULL;
            _pFilter = NULL;
            while (!_oPropertyContainer.empty())
            {
                _oPropertyContainer.pop();
            }
            while (!_oUnresolved.empty())
            {
                _oUnresolved.pop();
            }
            break;
        }

        //
        // dwf:Classes, dwf:Features, dwf:Entities, dwf:Objects, dwf:Groups, dwf:SharedProperties
        //
        case 1:
        {
            if (_nCurrentCollectionProvider == eProvideClasses )
            {
                resolveClasses();
            }
            else if (_nCurrentCollectionProvider == eProvideFeatures )
            {
                resolveFeatures();
            }
            else if (_nCurrentCollectionProvider == eProvideEntities )
            {
                resolveEntities();
            }
            else if (_nCurrentCollectionProvider == eProvideObjects )
            {
                resolveObjects();
            }
            else if (_nCurrentCollectionProvider == eProvideGroups )
            {
                resolveGroups();
            }
            else if (_nCurrentCollectionProvider == eProvideSharedProperties )
            {
            }

            _nCurrentCollectionProvider = eProvideNone;

            break;
        }

        //
        //  dwf:Class, dwf:Feature, dwf:Entity, dwf:Object, dwf:Group, dwf:PropertySet (for SharedProperties)
        //
        case 2:
        {
            if (_nCurrentCollectionProvider == eProvideClasses )
            {
                _provideClass();
            }
            else if (_nCurrentCollectionProvider == eProvideFeatures )
            {
                _provideFeature();
            }
            else if (_nCurrentCollectionProvider == eProvideEntities )
            {
                _provideEntity();
            }
            else if (_nCurrentCollectionProvider == eProvideObjects )
            {
                _provideObject();
            }
            else if (_nCurrentCollectionProvider == eProvideGroups )
            {
                _provideGroup();
            }
            else if (_nCurrentCollectionProvider == eProvideSharedProperties )
            {
                _provideSharedProperty();
            }

            _pCurrentElement = NULL;

            break;
        }

        default:
        {
            //
            //  Do we have a nested object i.e. child object?
            //
            if (_nCurrentCollectionProvider == eProvideObjects &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Object) == 0))
            {
                _provideChildObject();
            }

            //
            //  Do we have a nested property set
            //
            else if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0)
            {
                _providePropertySet();
            }
        }
    };
}

/////

_DWFTK_API
const char*
DWFContentReader::provideVersion( const char* zVersion )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return zVersion;
}

_DWFTK_API
DWFClass*
DWFContentReader::provideClass( DWFClass* pClass,
                                const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pClass;
}

_DWFTK_API
DWFFeature*
DWFContentReader::provideFeature( DWFFeature* pFeature,
                                  const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pFeature;
}

_DWFTK_API
DWFEntity*
DWFContentReader::provideEntity( DWFEntity* pEntity,
                                 const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pEntity;
}

_DWFTK_API
DWFObject*
DWFContentReader::provideObject( DWFObject* pObject,
                                 const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pObject;
}

_DWFTK_API
DWFGroup*
DWFContentReader::provideGroup( DWFGroup* pGroup,
                                const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pGroup;
}

_DWFTK_API
DWFPropertySet*
DWFContentReader::provideSharedProperty( DWFPropertySet* pPropertySet,
                                         const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pPropertySet;
}


_DWFTK_API
DWFObject*
DWFContentReader::provideChildObject( DWFObject* pObject,
                                      DWFObject* /*pParent*/,
                                      const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pObject;
}

_DWFTK_API
DWFPropertySet*
DWFContentReader::providePropertySet( DWFPropertySet* pPropertySet,
                                      DWFPropertyContainer* /*pContainer*/,
                                      const tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    //
    //  These methods do real work in the derived class
    //
    return pPropertySet;
}


////

#define _DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(T, U)                                                  \
_DWFTK_API                                                                                                  \
void DWFContentReader::_provide##U()                                                                        \
    throw( DWFException )                                                                                   \
{                                                                                                           \
    if (_oPropertyContainer.empty() || _oUnresolved.empty())                                                \
    {                                                                                                       \
        _DWFCORE_THROW( DWFUnexpectedException,                                                             \
            /*NOXLATE*/L"The PropertyContainer stack and UnresolvedList stack should not be empty." );      \
    }                                                                                                       \
    DWF##T* p##T = dynamic_cast<DWF##T*>(_oPropertyContainer.top());                                        \
    if (p##T == NULL)                                                                                       \
    {                                                                                                       \
        _DWFCORE_THROW( DWFUnexpectedException,                                                             \
            /*NOXLATE*/L"Incorrect element type on property container stack." );                            \
    }                                                                                                       \
    if (_pFilter)                                                                                           \
    {                                                                                                       \
        p##T = _pFilter->provide##U( p##T, _oUnresolved.top() );                                            \
    }                                                                                                       \
    provide##U( p##T, _oUnresolved.top() );                                                                 \
    _oPropertyContainer.pop();                                                                              \
    _oUnresolved.pop();                                                                                     \
}

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(Class, Class)

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(Feature, Feature)

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(Entity, Entity)

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(Object, Object)

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(Group, Group)

_DWFTOOLKIT_DWFCONTENTREADER_DEFINE_PROVIDE_(PropertySet, SharedProperty)

_DWFTK_API
void
DWFContentReader::_provideChildObject()
throw( DWFException )
{
    if (_oPropertyContainer.empty() || _oUnresolved.empty())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The stacks of propertycontainers and unresolved references should not be empty." );
    }

    DWFObject* pObject = dynamic_cast<DWFObject*>(_oPropertyContainer.top());
    if (pObject == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Incorrect element type on property container stack." );
    }

    //
    //  Pop the object to get to the next one on the stack for parent information.
    //
    _oPropertyContainer.pop();

    DWFObject* pParent = NULL;
    if (!_oPropertyContainer.empty())
    {
        pParent = dynamic_cast<DWFObject*>(_oPropertyContainer.top());
        if (pParent == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The container at the top of the stack should be a DWFObject." );
        }
    }

    if (_pFilter)
    {
        pObject = _pFilter->provideChildObject( pObject, pParent, _oUnresolved.top() );
    }
    provideChildObject( pObject, pParent, _oUnresolved.top() );

    _oUnresolved.pop();
}



_DWFTK_API
void 
DWFContentReader::_providePropertySet()
    throw( DWFException )
{
    if (_oPropertyContainer.empty() || _oUnresolved.empty())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The stacks of propertycontainers and unresolved references should not be empty." );
    }

    DWFPropertySet* pPropertySet = dynamic_cast<DWFPropertySet*>(_oPropertyContainer.top());
    if (pPropertySet == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Incorrect element type on property container stack." );
    }

    //
    //  Pop the container to get to the next one on the stack for owner container information.
    //
    _oPropertyContainer.pop();

    if (_oPropertyContainer.empty())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The property container stack should have owner container for the current property set." );
    }
    DWFPropertyContainer* pContainer = _oPropertyContainer.top();

    if (_pFilter)
    {
        pPropertySet = _pFilter->providePropertySet( pPropertySet, pContainer, _oUnresolved.top() );
    }
    providePropertySet( pPropertySet, pContainer, _oUnresolved.top() );

    _oUnresolved.pop();
}

_DWFTK_API
void
DWFContentReader::_provideVersion( const char* zVersion )
throw( DWFException )
{
    provideVersion( _pFilter ? _pFilter->provideVersion(zVersion) : zVersion );
}


