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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/EModelSectionDescriptorReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//



#include "dwf/package/Constants.h"
#include "dwf/package/reader/EModelSectionDescriptorReader.h"
#include "dwf/package/SectionContentResource.h"
using namespace DWFToolkit;



_DWFTK_API
DWFEModelSectionDescriptorReader::DWFEModelSectionDescriptorReader( DWFPackageReader* pPackageReader,
                                                                    unsigned int      nProviderFlags )
throw()
                               : DWFSectionDescriptorReader( pPackageReader )
                               , _nProviderFlags( nProviderFlags )
                               , _nCurrentCollectionProvider( 0 )
                               , _pCurrentResource( NULL )
                               , _pCoordinateSystem( NULL )
                               , _pFilter( NULL )
{
    ;
}

_DWFTK_API
DWFEModelSectionDescriptorReader::~DWFEModelSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFEModelSectionDescriptorReader* 
DWFEModelSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFEModelSectionDescriptorReader::setFilter( DWFEModelSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
const char*
DWFEModelSectionDescriptorReader::provideName( const char* zName )
throw()
{
    return zName;
}

_DWFTK_API
double
DWFEModelSectionDescriptorReader::provideVersion( double nVersion )
throw()
{
    return nVersion;
}

_DWFTK_API
double
DWFEModelSectionDescriptorReader::providePlotOrder( double nPlotOrder )
throw()
{
    return nPlotOrder;
}

_DWFTK_API
DWFUnits*
DWFEModelSectionDescriptorReader::provideUnits( DWFUnits* pUnits )
throw()
{
    return pUnits;
}

_DWFTK_API
DWFProperty*
DWFEModelSectionDescriptorReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFEModelSectionDescriptorReader::provideResource( DWFResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFEModelSectionDescriptorReader::provideFontResource( DWFFontResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFEModelSectionDescriptorReader::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFEModelSectionDescriptorReader::provideImageResource( DWFImageResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFContentPresentationResource*
DWFEModelSectionDescriptorReader::provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    return pResource;
}

void
DWFEModelSectionDescriptorReader::_provideAttributes( const char** ppAttributeList )
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
            else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][7];
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
                     !(nFound & eProvidePlotOrder)            &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_PlotOrder) == 0))
            {
                nFound |= eProvidePlotOrder;

                _providePlotOrder( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
            }
        }
    }
}

_DWFTK_API
void
DWFEModelSectionDescriptorReader::notifyStartElement( const char*   zName,
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
            // should be EModel:Space, if not, disable all further processing...
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Space) != 0)
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // *:Properties, *:Units, *:Resources
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
            else if ((_nProviderFlags & eProvideUnits) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Units) == 0))
            {
                //
                // create and provide new paper object
                //
                _provideUnits( _pElementBuilder->buildUnits(ppAttributeList) );

                _nCurrentCollectionProvider = eProvideNone;
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
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GraphicResource) == 0) &&
                         (_nProviderFlags & eProvideGraphicResource))
                {
                    //
                    // create new graphic resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildGraphicResource( ppAttributeList, _pPackageReader );
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
                    // create new resource object for content presentation
                    //
                    _pCurrentResource = _pElementBuilder->buildContentPresentationResource( ppAttributeList, _pPackageReader );
                }
            }
            break;
        }
            //
            // *:Properties, *:CoordinateSystems
            //
        case 3:
        {
            break;
        }
            //
            // *:Property, *:CoordinateSystem
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
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CoordinateSystem) == 0))
            {
                DWFGraphicResource* pGraphicResource = dynamic_cast<DWFGraphicResource*>(_pCurrentResource);
                if (pGraphicResource)
                {
                    //
                    // add new coordinate system object to the resource
                    //
                    _pCoordinateSystem = _pElementBuilder->buildCoordinateSystem( ppAttributeList );
                    pGraphicResource->addCoordinateSystem( _pCoordinateSystem );
                }
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
            //
            // *:Properties
            //
        case 5:
        {
            break;
        }
            //
            // *:Property
            //
        case 6:
        {
            if (_pCoordinateSystem &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // add new property object to the resource
                //
                _pCoordinateSystem->addProperty( _pElementBuilder->buildProperty(ppAttributeList), true );
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
DWFEModelSectionDescriptorReader::notifyEndElement( const char* zName )
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
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, zName, 7) == 0)
    {
        zName+=7;
    }

        //
        // decrement the depth first
        // this lets us match up the switch statements for start and end
        //
    switch (--_nElementDepth)
    {
            //
            // *:Properties, *:Units, *:Resources
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
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GraphicResource) == 0) &&
                         (_nProviderFlags & eProvideGraphicResource))
                {
                    //
                    // provide new graphic resource object
                    //
                    _provideGraphicResource( (DWFGraphicResource*)_pCurrentResource );
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
DWFEModelSectionDescriptorReader::notifyStartNamespace( const char* zPrefix,
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
DWFEModelSectionDescriptorReader::notifyEndNamespace( const char* zPrefix )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndNamespace( zPrefix );
}

void
DWFEModelSectionDescriptorReader::_provideName( const char* zName )
throw()
{
    provideName( _pFilter ? _pFilter->provideName(zName) : zName );
}

void
DWFEModelSectionDescriptorReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFEModelSectionDescriptorReader::_providePlotOrder( double nPlotOrder )
throw()
{
    providePlotOrder( _pFilter ? _pFilter->providePlotOrder(nPlotOrder) : nPlotOrder );
}

void
DWFEModelSectionDescriptorReader::_provideUnits( DWFUnits* pUnits )
throw()
{
    provideUnits( _pFilter ? _pFilter->provideUnits(pUnits) : pUnits );
}

void
DWFEModelSectionDescriptorReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFEModelSectionDescriptorReader::_provideResource( DWFResource* pResource )
throw()
{
    provideResource( _pFilter ? _pFilter->provideResource(pResource) : pResource );
}

void
DWFEModelSectionDescriptorReader::_provideFontResource( DWFFontResource* pResource )
throw()
{
    provideFontResource( _pFilter ? _pFilter->provideFontResource(pResource) : pResource );
}

void
DWFEModelSectionDescriptorReader::_provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    provideGraphicResource( _pFilter ? _pFilter->provideGraphicResource(pResource) : pResource );
}

void
DWFEModelSectionDescriptorReader::_provideImageResource( DWFImageResource* pResource )
throw()
{
    provideImageResource( _pFilter ? _pFilter->provideImageResource(pResource) : pResource );
}

void
DWFEModelSectionDescriptorReader::_provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    if (_pFilter)
    {
        pResource = _pFilter->provideContentPresentationResource( pResource );
    }
    provideContentPresentationResource( pResource );
}
