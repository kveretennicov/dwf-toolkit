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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/GlobalSectionDescriptorReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//




#include "dwf/package/Constants.h"
#include "dwf/package/reader/GlobalSectionDescriptorReader.h"
using namespace DWFToolkit;



_DWFTK_API
DWFGlobalSectionDescriptorReader::DWFGlobalSectionDescriptorReader( DWFPackageReader* pPackageReader,
                                                                    unsigned int      nProviderFlags )
throw()
                               : DWFSectionDescriptorReader( pPackageReader )
                               , _nProviderFlags( nProviderFlags )
                               , _nCurrentCollectionProvider( 0 )
                               , _pCurrentResource( NULL )
                               , _pFilter( NULL )
                               , _oBookmarks()
{
    ;
}

_DWFTK_API
DWFGlobalSectionDescriptorReader::~DWFGlobalSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFGlobalSectionDescriptorReader* 
DWFGlobalSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFGlobalSectionDescriptorReader::setFilter( DWFGlobalSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
const char*
DWFGlobalSectionDescriptorReader::provideName( const char* zName )
throw()
{
    return zName;
}

_DWFTK_API
double
DWFGlobalSectionDescriptorReader::provideVersion( double nVersion )
throw()
{
    return nVersion;
}

_DWFTK_API
DWFProperty*
DWFGlobalSectionDescriptorReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFGlobalSectionDescriptorReader::provideResource( DWFResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFGlobalSectionDescriptorReader::provideFontResource( DWFFontResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFGlobalSectionDescriptorReader::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFGlobalSectionDescriptorReader::provideImageResource( DWFImageResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFBookmark*
DWFGlobalSectionDescriptorReader::provideBookmark( DWFBookmark* pBookmark )
throw()
{
    return pBookmark;
}

void
DWFGlobalSectionDescriptorReader::_provideAttributes( const char** ppAttributeList )
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
        }
    }
}

_DWFTK_API
void
DWFGlobalSectionDescriptorReader::notifyStartElement( const char*   zName,
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
            // should be *:Global, if not, disable all further processing...
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Global) != 0)
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // *:Properties, *:Bookmark, *:Resources
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
            else if ((_nProviderFlags & eProvideBookmarks) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Bookmark) == 0))
            {
                _nCurrentCollectionProvider = eProvideBookmarks;

                //
                // the first bookmark is the root of the tree/stack/whatever
                //
                _oBookmarks.push_back( _pElementBuilder->buildBookmark(ppAttributeList) );
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

        //
        // handle subdepth bookmarks here
        //
    if ((_nProviderFlags & eProvideBookmarks)   &&
        (_nElementDepth > 1)                    &&
        (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Bookmark) == 0))
    {
        //
        // create a new bookmark 
        //
        DWFBookmark* pBookmark = _pElementBuilder->buildBookmark( ppAttributeList );

        if (pBookmark)
        {
            //
            // get the parent bookmark of this level
            //
            DWFBookmark* pParent = _oBookmarks[_nElementDepth - 2];

            //
            // add as a child
            //
            pParent->addChildBookmark( pBookmark );

            //
            //
            //
            if (_oBookmarks.size() >= _nElementDepth)
            {
                //
                // remove all parents at and below this level
                //
                _oBookmarks.erase(_oBookmarks.end()-(_oBookmarks.size()-_nElementDepth)-1, _oBookmarks.end());
            }

            //
            // push into the vector as the current parent for this level
            //
            _oBookmarks.push_back( pBookmark );
        }
    }

    //
    //
    //
    _nElementDepth++;
}

_DWFTK_API
void
DWFGlobalSectionDescriptorReader::notifyEndElement( const char* zName )
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
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, zName, 6) == 0)
    {
        zName+=6;
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
            // *:Properties, *:Bookmark, *:Resources
            //
        case 1:
        {
            _nCurrentCollectionProvider = eProvideNone;

                //
                // provide root bookmark - the first in the array
                // which carries with it, the whole tree
                //
            if (_oBookmarks.size() > 0)
            {
                _provideBookmark( _oBookmarks[0] );
            }

            //
            //
            //
            _oBookmarks.clear();

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
DWFGlobalSectionDescriptorReader::notifyStartNamespace( const char* zPrefix,
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
DWFGlobalSectionDescriptorReader::notifyEndNamespace( const char* zPrefix )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndNamespace( zPrefix );
}

void
DWFGlobalSectionDescriptorReader::_provideName( const char* zName )
throw()
{
    provideName( _pFilter ? _pFilter->provideName(zName) : zName );
}

void
DWFGlobalSectionDescriptorReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFGlobalSectionDescriptorReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFGlobalSectionDescriptorReader::_provideResource( DWFResource* pResource )
throw()
{
    provideResource( _pFilter ? _pFilter->provideResource(pResource) : pResource );
}

void
DWFGlobalSectionDescriptorReader::_provideFontResource( DWFFontResource* pResource )
throw()
{
    provideFontResource( _pFilter ? _pFilter->provideFontResource(pResource) : pResource );
}

void
DWFGlobalSectionDescriptorReader::_provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    provideGraphicResource( _pFilter ? _pFilter->provideGraphicResource(pResource) : pResource );
}

void
DWFGlobalSectionDescriptorReader::_provideImageResource( DWFImageResource* pResource )
throw()
{
    provideImageResource( _pFilter ? _pFilter->provideImageResource(pResource) : pResource );
}

void
DWFGlobalSectionDescriptorReader::_provideBookmark( DWFBookmark* pBookmark )
throw()
{
    provideBookmark( _pFilter ? _pFilter->provideBookmark(pBookmark) : pBookmark );
}




