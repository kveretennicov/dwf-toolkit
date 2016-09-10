//
//  Copyright (c) 2004 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/SignatureSectionDescriptorReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//



#include "dwf/package/Constants.h"
#include "dwf/package/reader/SignatureSectionDescriptorReader.h"
using namespace DWFToolkit;



_DWFTK_API
DWFSignatureSectionDescriptorReader::DWFSignatureSectionDescriptorReader( DWFPackageReader* pPackageReader,
                                                                unsigned int      nProviderFlags )
throw()
                              : DWFSectionDescriptorReader( pPackageReader )
                              , _nProviderFlags( nProviderFlags )
                              , _nCurrentCollectionProvider( 0 )
                              , _pCurrentResource( NULL )
                              , _pFilter( NULL )
{
    ;
}

_DWFTK_API
DWFSignatureSectionDescriptorReader::~DWFSignatureSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFSignatureSectionDescriptorReader* 
DWFSignatureSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFSignatureSectionDescriptorReader::setFilter( DWFSignatureSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
const char*
DWFSignatureSectionDescriptorReader::provideName( const char* zName )
throw()
{
    return zName;
}

_DWFTK_API
const char*
DWFSignatureSectionDescriptorReader::provideObjectID( const char* zObjectID )
throw()
{
    return zObjectID;
}

_DWFTK_API
double
DWFSignatureSectionDescriptorReader::provideVersion( double nVersion )
throw()
{
    return nVersion;
}

_DWFTK_API
DWFProperty*
DWFSignatureSectionDescriptorReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFSignatureSectionDescriptorReader::provideResource( DWFResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFSignatureResource*
DWFSignatureSectionDescriptorReader::provideSignatureResource( DWFSignatureResource* pResource )
throw()
{
    return pResource;
}

void
DWFSignatureSectionDescriptorReader::_provideAttributes( const char** ppAttributeList )
throw()
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
            else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Signatures, ppAttributeList[iAttrib], 11) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][11];
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
                //
                // provide the name
                //
            else if ((_nProviderFlags & eProvideName)   &&
                     !(nFound & eProvideName)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
            {
                nFound |= eProvideName;

                _provideName( ppAttributeList[iAttrib+1] );
            }
                //
                // provide the object id
                //
            else if ((_nProviderFlags & eProvideObjectID)   &&
                     !(nFound & eProvideName)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
            {
                nFound |= eProvideObjectID;

                _provideObjectID( ppAttributeList[iAttrib+1] );
            }
        }
    }
}

_DWFTK_API
void
DWFSignatureSectionDescriptorReader::notifyStartElement( const char*   zName,
                                                     const char**  ppAttributeList )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyStartElement( zName, ppAttributeList );

        //
        // skip over any known prefixes in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, zName, 8) == 0)
    {
        zName+=8;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Signatures, zName, 11) == 0)
    {
        zName+=11;
    }

        //
        //
        //
    switch (_nElementDepth)
    {
            //
            // should be Signature:Section, if not, disable all further processing...
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Section) != 0)
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // *:Properties, *:Paper, *:Resources
            //
        case 1:
        {
            if ((_nProviderFlags & eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                _nCurrentCollectionProvider = eProvideProperties;
            }
            else if ((_nProviderFlags & eProvideResources) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resources) == 0))
            {
                _nCurrentCollectionProvider = eProvideResources;
            }
            else
            {
                _nCurrentCollectionProvider = eProvideNone;
            }

            break;
        }
            //
            // *:Property, *:Resource
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // create and provide new property object
                //
                _provideProperty( _pElementBuilder->buildProperty(ppAttributeList) );
            }
            else if (_nCurrentCollectionProvider == eProvideResources)
            {
                if (
                    (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) &&
                    (_nProviderFlags & eProvideUntypedResource)
                   )
                {
                    //
                    // create new resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureResource) == 0) &&
                         (_nProviderFlags & eProvideSignatureResource))
                {
                    //
                    // create new signature resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildSignatureResource( ppAttributeList, _pPackageReader );
                }
            }
            break;
        }
            //
            // *:Properties
            //
        case 3:
        {
            break;
        }
            //
            // *:Property
            //
        case 4:
        {
            if (_pCurrentResource &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // add new property object to the resource
                //
                _pCurrentResource->addProperty( _pElementBuilder->buildProperty(ppAttributeList), true );
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
DWFSignatureSectionDescriptorReader::notifyEndElement( const char* zName )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndElement( zName );


        //
        // skip over any known prefixes in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, zName, 8) == 0)
    {
        zName+=8;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Signatures, zName, 11) == 0)
    {
        zName+=11;
    }

        //
        // decrement the depth first
        // this lets us match up the switch statements for start and end
        //
    switch (--_nElementDepth)
    {
            //
            // *:Properties, *:Paper, *:Resources
            //
        case 1:
        {
            _nCurrentCollectionProvider = eProvideNone;
            break;
        }
            //
            // *:Property, *:Resource
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideResources) && _pCurrentResource)
            {
                if (
                    (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) &&
                    (_nProviderFlags & eProvideUntypedResource)
                   )
                {
                    //
                    // provide new resource object
                    //
                    _provideResource( _pCurrentResource );
                }

                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_SignatureResource) == 0) &&
                         (_nProviderFlags & eProvideSignatureResource))
                {
                    //
                    // provide new Signature resource object
                    //
                    _provideSignatureResource( (DWFSignatureResource*)_pCurrentResource );
                }

                _pCurrentResource = NULL;
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
DWFSignatureSectionDescriptorReader::notifyStartNamespace( const char* zPrefix,
                                                       const char* zURI )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyStartNamespace( zPrefix, zURI );
}

_DWFTK_API
void
DWFSignatureSectionDescriptorReader::notifyEndNamespace( const char* zPrefix )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndNamespace( zPrefix );
}


void
DWFSignatureSectionDescriptorReader::_provideName( const char* zName )
throw()
{
    provideName( _pFilter ? _pFilter->provideName(zName) : zName );
}

void
DWFSignatureSectionDescriptorReader::_provideObjectID( const char* zObjectID )
throw()
{
    provideObjectID( _pFilter ? _pFilter->provideObjectID(zObjectID) : zObjectID );
}

void
DWFSignatureSectionDescriptorReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFSignatureSectionDescriptorReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFSignatureSectionDescriptorReader::_provideResource( DWFResource* pResource )
throw()
{
    provideResource( _pFilter ? _pFilter->provideResource(pResource) : pResource );
}

void
DWFSignatureSectionDescriptorReader::_provideSignatureResource( DWFSignatureResource* pResource )
throw()
{
    provideSignatureResource( _pFilter ? _pFilter->provideSignatureResource(pResource) : pResource );
}

