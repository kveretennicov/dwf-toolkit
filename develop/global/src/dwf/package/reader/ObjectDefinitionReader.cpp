//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
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



#include "dwf/package/Constants.h"
#include "dwf/package/reader/ObjectDefinitionReader.h"
using namespace DWFToolkit;



_DWFTK_API
DWFObjectDefinitionReader::DWFObjectDefinitionReader( DWFPackageReader* pPackageReader,
                                                      unsigned char     nProviderFlags )
throw()
                 : _pPackageReader( pPackageReader )
                 , _oDefaultElementBuilder()
                 , _pElementBuilder( &_oDefaultElementBuilder )
                 , _nProviderFlags( nProviderFlags )
                 , _nCurrentCollectionProvider( 0 )
                 , _nInstanceSequence( 0L )
                 , _pCurrentObject( NULL )
                 , _pCurrentRefs( NULL )
                 , _pCurrentProperties( NULL )
                 , _pReaderFilter( NULL )
{
    ;
}

_DWFTK_API
DWFObjectDefinitionReader::~DWFObjectDefinitionReader()
throw()
{
    ;
}

_DWFTK_API
DWFObjectDefinitionReader*
DWFObjectDefinitionReader::filter() const
throw()
{
    return _pReaderFilter;
}

_DWFTK_API
void
DWFObjectDefinitionReader::setFilter( DWFObjectDefinitionReader* pFilter )
throw()
{
    _pReaderFilter = pFilter;
}
    
_DWFTK_API
double
DWFObjectDefinitionReader::provideVersion( double nDecimalVersion )
throw()
{
    return nDecimalVersion;
}

_DWFTK_API
void
DWFObjectDefinitionReader::provideProperties( const DWFString&      /*zID*/,
                                              tStringVector*    /*pPropertyRefs*/,
                                              DWFProperty::tList*   /*pPropertyList*/ )
throw()
{
    ;
}

_DWFTK_API
DWFDefinedObject*
DWFObjectDefinitionReader::provideObject( DWFDefinedObject* pObject )
throw()
{
    return pObject;
}

_DWFTK_API
DWFDefinedObjectInstance*
DWFObjectDefinitionReader::provideInstance( DWFDefinedObjectInstance* pInstance )
throw()
{
    return pInstance;
}

_DWFTK_API
void
DWFObjectDefinitionReader::notifyStartElement( const char*   zName,
                                               const char**  ppAttributeList )
throw()
{
        //
        // skip over any acceptable prefixes in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    }
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, zName, 8) == 0)
    {
        zName+=8;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, zName, 6) == 0)
    {
        zName+=6;
    }
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, zName, 7) == 0)
    {
        zName+=7;
    }

        //
        //
        //
    switch (_nElementDepth)
    {
            //
            // PageObjectDefinition, SpaceObjectDefinition
            //
        case 0:
        {
            if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PageObjectDefinition) == 0) ||
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SpaceObjectDefinition) == 0) ||
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ObjectDefinition) == 0) ||
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GlobalObjectDefinition) == 0))
            {
                    //
                    // evaluate attribute list only if asked
                    //
                if (_nProviderFlags & eProvideAttributes)
                {
                    size_t iAttrib = 0;
                    unsigned char nFound = eProvideNone;
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
                        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
                        {
                            pAttrib = &ppAttributeList[iAttrib][7];
                        }
                        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
                        {
                            pAttrib = &ppAttributeList[iAttrib][6];
                        }
                        else
                        {
                            pAttrib = &ppAttributeList[iAttrib][0];
                        }

                            //
                            // provide the version
                            //
                        if ((_nProviderFlags & eProvideVersion) &&
                            !(nFound & eProvideVersion)         &&
                             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Version) == 0))
                        {
                            nFound |= eProvideVersion;

                            _provideVersion( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
                        }
                    }
                }
            }
                //
                // 
                //
            else
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // Properties, Objects, Instances
            //
        case 1:
        {
            if ((_nProviderFlags & eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                size_t iAttrib = 0;
                const char* pAttrib = NULL;

                for(; ppAttributeList[iAttrib]; iAttrib += 2)
                {
                        //
                        // skip over any acceptable prefixes in the attribute name
                        //
                    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][4];
                    } 
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][8];
                    } 
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][7];
                    }
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][6];
                    }
                    else
                    {
                        pAttrib = ppAttributeList[iAttrib];
                    }

                    if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0)
                    {
                        _zCurrentID.assign( ppAttributeList[iAttrib+1] );
                    }
                    else if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Refs) == 0)
                    {
                        char* pRefs = (char*)ppAttributeList[iAttrib+1];
                        char* pSavePtr;
                        char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pRefs, /*NOXLATE*/" ", &pSavePtr );

                        while (pToken)
                        {
                            if (_pCurrentRefs == NULL)
                            {
                                _pCurrentRefs = DWFCORE_ALLOC_OBJECT( tStringVector );
                            }
                            if (_pCurrentRefs)
                            {
                                _pCurrentRefs->push_back( pToken );
                            }

                            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                        }
                    }
                }

                _nCurrentCollectionProvider = eProvideProperties;
            }
            else if ((_nProviderFlags & eProvideObjects) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Objects) == 0))
            {
                _nCurrentCollectionProvider = eProvideObjects;
            }
            else if ((_nProviderFlags & eProvideInstances) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Instances) == 0))
            {
                _nCurrentCollectionProvider = eProvideInstances;
            }
            else
            {
                _nCurrentCollectionProvider = eProvideNone;
            }

            break;
        }
            //
            // Property, Object, Instance
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                    //
                    // create new collection if needed
                    // we map the properties by category
                    //
                if (_pCurrentProperties == NULL)
                {
                    _pCurrentProperties = DWFCORE_ALLOC_OBJECT( DWFProperty::tList );
                }

                    //
                    // add new property to collection
                    //
                if (_pCurrentProperties)
                {
                    DWFProperty* pProperty = _pElementBuilder->buildProperty( ppAttributeList, !(_nProviderFlags & eProvideCustomizeAttributes) );
                    if (pProperty)
                    {
                        _pCurrentProperties->push_back( pProperty );
                    }
                }
            }
            else if ((_nCurrentCollectionProvider == eProvideObjects) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Object) == 0))
            {
                //
                // create object
                //
                _pCurrentObject = _pElementBuilder->buildDefinedObject( ppAttributeList );
            }
            else if ((_nCurrentCollectionProvider == eProvideInstances) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Instance) == 0))
            {
                //
                // create instance
                //
                _pCurrentObject = _pElementBuilder->buildDefinedObjectInstance( ppAttributeList, ++_nInstanceSequence );
            }

            break;
        }
            //
            // Object|Instance - Properties
            //
        case 3:
        {
            if (_pCurrentObject &&
               (_nProviderFlags & eProvideProperties) &&
               (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                size_t iAttrib = 0;
                const char* pAttrib = NULL;

                for(; ppAttributeList[iAttrib]; iAttrib += 2)
                {
                        //
                        // skip over any acceptable prefixes in the attribute name
                        //
                    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][4];
                    } 
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][8];
                    } 
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][7];
                    }
                    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
                    {
                        pAttrib = &ppAttributeList[iAttrib][6];
                    }
                    else
                    {
                        pAttrib = ppAttributeList[iAttrib];
                    }

                    //if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0)
                    //{
                    //    _zCurrentID.assign( ppAttributeList[iAttrib+1] );
                    //}
                    //else 
                    if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Refs) == 0)
                    {
                        char* pRefs = (char*)ppAttributeList[iAttrib+1];
                        char* pSavePtr;
                        char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pRefs, /*NOXLATE*/" ", &pSavePtr );

                        while (pToken)
                        {
                            //
                            // add a new property ref to the current element
                            //
                            _pCurrentObject->addPropertyReference( pToken );

                            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                        }
                    }
                }
            }

            break;
        }
            //
            // Property
            //
        case 4:
        {
            if (_pCurrentObject && (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // add a new property to the current element
                //
                _pCurrentObject->addProperty( _pElementBuilder->buildProperty(ppAttributeList, !(_nProviderFlags & eProvideCustomizeAttributes) ), true ); 
            }
            break;
        }
        default:
        {
            ;
        }
    };

    _nElementDepth++;
}

_DWFTK_API
void
DWFObjectDefinitionReader::notifyEndElement( const char* /*zName*/ )
throw()
{
        //
        // decrement the depth first
        // this lets us match up the switch statements for start and end
        //
    switch (--_nElementDepth)
    {
            //
            // Properties, Objects, Instances
            //
        case 1:
        {
                //
                // provide the collection of properties
                //
            if (_nCurrentCollectionProvider == eProvideProperties)
            {
                _provideProperties( _zCurrentID, _pCurrentRefs, _pCurrentProperties );
                
                //
                // the providee owns these pointers now
                //
                _zCurrentID.destroy();
                _pCurrentRefs = NULL;
                _pCurrentProperties = NULL;
            }

            _nCurrentCollectionProvider = eProvideNone;
            break;
        }
            //
            // Property, Object, Instance
            //
        case 2:
        {
            if (_nCurrentCollectionProvider == eProvideProperties)
            {
                ;
            }
            else if ((_nCurrentCollectionProvider == eProvideObjects) && _pCurrentObject)
            {
                //
                // provide the new interface object
                //
                _provideObject( (DWFDefinedObject*)_pCurrentObject );
            
                _pCurrentObject = NULL;
            }
            else if ((_nCurrentCollectionProvider == eProvideInstances) && _pCurrentObject)
            {
                //
                // provide the new section object
                //
                _provideInstance( (DWFDefinedObjectInstance*)_pCurrentObject );
            
                _pCurrentObject = NULL;
            }

            break;
        }
        default:
        {
            ;
        }
    };
}

_DWFTK_API
void
DWFObjectDefinitionReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                 const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFObjectDefinitionReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFObjectDefinitionReader::notifyCharacterData( const char* /*zCData*/, 
                                                int         /*nLength*/ ) 
throw()
{
}

void 
DWFObjectDefinitionReader::_provideVersion( double nVersion)
throw()
{
    provideVersion( _pReaderFilter ? _pReaderFilter->provideVersion(nVersion) : nVersion );
}

void 
DWFObjectDefinitionReader::_provideProperties( const DWFString&    zID,
                                               tStringVector*  pPropertyRefs,
                                               DWFProperty::tList* pPropertyList )
throw()
{
    if (_pReaderFilter != NULL)
    {
        _pReaderFilter->provideProperties( zID, pPropertyRefs, pPropertyList );
    }
        
    provideProperties( zID, pPropertyRefs, pPropertyList );
}

void 
DWFObjectDefinitionReader::_provideObject( DWFDefinedObject* pObject )
throw()
{
    provideObject( _pReaderFilter ? _pReaderFilter->provideObject(pObject) : pObject );
}

void 
DWFObjectDefinitionReader::_provideInstance( DWFDefinedObjectInstance* pInstance )
throw()
{
    provideInstance( _pReaderFilter ? _pReaderFilter->provideInstance(pInstance) : pInstance );
}
