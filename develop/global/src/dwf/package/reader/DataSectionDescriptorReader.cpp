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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/DataSectionDescriptorReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//



#include "dwf/package/Constants.h"
#include "dwf/package/reader/DataSectionDescriptorReader.h"
#include "dwf/package/SectionContentResource.h"
using namespace DWFToolkit;



_DWFTK_API
DWFDataSectionDescriptorReader::DWFDataSectionDescriptorReader( DWFPackageReader* pPackageReader,
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
DWFDataSectionDescriptorReader::~DWFDataSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFDataSectionDescriptorReader* 
DWFDataSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFDataSectionDescriptorReader::setFilter( DWFDataSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
const char*
DWFDataSectionDescriptorReader::provideName( const char* zName )
throw()
{
    return zName;
}

_DWFTK_API
const char*
DWFDataSectionDescriptorReader::provideObjectID( const char* zObjectID )
throw()
{
    return zObjectID;
}

_DWFTK_API
double
DWFDataSectionDescriptorReader::provideVersion( double nVersion )
throw()
{
    return nVersion;
}

_DWFTK_API
double
DWFDataSectionDescriptorReader::providePlotOrder( double nPlotOrder )
throw()
{
    return nPlotOrder;
}

_DWFTK_API
DWFProperty*
DWFDataSectionDescriptorReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFDataSectionDescriptorReader::provideResource( DWFResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFDataSectionDescriptorReader::provideFontResource( DWFFontResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFDataSectionDescriptorReader::provideImageResource( DWFImageResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFContentPresentationResource*
DWFDataSectionDescriptorReader::provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    return pResource;
}

void
DWFDataSectionDescriptorReader::_provideAttributes( const char** ppAttributeList )
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
            else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Data, ppAttributeList[iAttrib], 5) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][5];
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
                // provide the plot order
                //
            else if ((_nProviderFlags & eProvidePlotOrder)   &&
                     !(nFound & eProvidePlotOrder)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_PlotOrder) == 0))
            {
                nFound |= eProvidePlotOrder;

                _providePlotOrder( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
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
DWFDataSectionDescriptorReader::notifyStartElement( const char*   zName,
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
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Data, zName, 5) == 0)
    {
        zName+=5;
    }

        //
        //
        //
    switch (_nElementDepth)
    {
            //
            // should be Data:Section, if not, disable all further processing...
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
                //
                //  kzElement_ContentResource should eventually be removed - this is to handle develop time test files.
                //
                if (((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) ||
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentResource) == 0)) &&
                    (_nProviderFlags & eProvideUntypedResource))
                {
                    //
                    // create new resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildResource( ppAttributeList, _pPackageReader );

                    //
                    //  Since 7.3:
                    //  As of 7.3 we are adding the capability to modify the section content resource when it is
                    //  read in and then republish it. To allow this plain resources that were published with the
                    //  role kzRole_ContentDefinition must be reinstantiated as a section content resource.
                    //
                    if (_pCurrentResource->role() == DWFXML::kzRole_ContentDefinition)
                    {
                        //
                        //  Create a section content resource object
                        //
                        DWFCORE_FREE_OBJECT( _pCurrentResource );
                        _pCurrentResource = _pElementBuilder->buildSectionContentResource( ppAttributeList, _pPackageReader );
                    }   
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_FontResource) == 0) &&
                         (_nProviderFlags & eProvideFontResource))
                {
                    //
                    // create new font resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildFontResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ImageResource) == 0) &&
                         (_nProviderFlags & eProvideImageResource))
                {
                    //
                    // create new image resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildImageResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentationResource) == 0) &&
                         (_nProviderFlags & eProvideContentPresentationResource))
                {
                    //
                    // create new resource object for content metadata
                    //
                    _pCurrentResource = _pElementBuilder->buildContentPresentationResource( ppAttributeList, _pPackageReader );
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
            else if (_pCurrentResource &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Relationship) == 0))
            {
                //
                // add new relationship object to the resource
                //
                _pCurrentResource->addRelationship( _pElementBuilder->buildRelationship( ppAttributeList, _pPackageReader ) );
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
DWFDataSectionDescriptorReader::notifyEndElement( const char* zName )
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
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_Data, zName, 5) == 0)
    {
        zName+=5;
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
                if (((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) ||
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentResource) == 0)) &&
                    (_nProviderFlags & eProvideUntypedResource))
                {
                    //
                    // provide new resource object
                    //
                    _provideResource( _pCurrentResource );
                }

                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_FontResource) == 0) &&
                         (_nProviderFlags & eProvideFontResource))
                {
                    //
                    // provide new font resource object
                    //
                    _provideFontResource( (DWFFontResource*)_pCurrentResource );
                }

                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ImageResource) == 0) &&
                         (_nProviderFlags & eProvideImageResource))
                {
                    //
                    // provide new image resource object
                    //
                    _provideImageResource( (DWFImageResource*)_pCurrentResource );
                }


                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentationResource) == 0) &&
                         (_nProviderFlags & eProvideContentPresentationResource))
                {
                    //
                    // provide new content resource object
                    //
                    _provideContentPresentationResource( (DWFContentPresentationResource*)_pCurrentResource );
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
DWFDataSectionDescriptorReader::notifyStartNamespace( const char* zPrefix,
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
DWFDataSectionDescriptorReader::notifyEndNamespace( const char* zPrefix )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndNamespace( zPrefix );
}


void
DWFDataSectionDescriptorReader::_provideName( const char* zName )
throw()
{
    provideName( _pFilter ? _pFilter->provideName(zName) : zName );
}

void
DWFDataSectionDescriptorReader::_provideObjectID( const char* zObjectID )
throw()
{
    provideObjectID( _pFilter ? _pFilter->provideObjectID(zObjectID) : zObjectID );
}

void
DWFDataSectionDescriptorReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFDataSectionDescriptorReader::_providePlotOrder( double nPlotOrder )
throw()
{
    providePlotOrder( _pFilter ? _pFilter->providePlotOrder(nPlotOrder) : nPlotOrder );
}

void
DWFDataSectionDescriptorReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFDataSectionDescriptorReader::_provideResource( DWFResource* pResource )
throw()
{
    provideResource( _pFilter ? _pFilter->provideResource(pResource) : pResource );
}

void
DWFDataSectionDescriptorReader::_provideFontResource( DWFFontResource* pResource )
throw()
{
    provideFontResource( _pFilter ? _pFilter->provideFontResource(pResource) : pResource );
}

void
DWFDataSectionDescriptorReader::_provideImageResource( DWFImageResource* pResource )
throw()
{
    provideImageResource( _pFilter ? _pFilter->provideImageResource(pResource) : pResource );
}


void
DWFDataSectionDescriptorReader::_provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    if (_pFilter)
    {
        pResource = _pFilter->provideContentPresentationResource( pResource );
    }
    provideContentPresentationResource( pResource );
}
