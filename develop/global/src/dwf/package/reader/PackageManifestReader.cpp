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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/PackageManifestReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//




#include "dwf/package/Constants.h"
#include "dwf/package/SectionBuilder.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/reader/PackageManifestReader.h"
#include "dwf/presentation/PackageContentPresentations.h"
using namespace DWFToolkit;



_DWFTK_API
DWFManifestReader::DWFManifestReader( DWFPackageReader* pPackageReader,
                                      unsigned char     nProviderFlags )
throw()
                 : _pPackageReader( pPackageReader )
                 , _oDefaultElementBuilder()
                 , _pElementBuilder( &_oDefaultElementBuilder )
                 , _nProviderFlags( nProviderFlags )
                 , _nCurrentCollectionProvider( 0 )
                 , _pCurrentElement( NULL )
                 , _pFilter( NULL )
{
    ;
}

_DWFTK_API
DWFManifestReader::~DWFManifestReader()
throw()
{
    ;
}

_DWFTK_API
DWFManifestReader*
DWFManifestReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFManifestReader::setFilter( DWFManifestReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
double
DWFManifestReader::provideVersion( double nDecimalVersion )
throw( DWFException )
{
    return nDecimalVersion;
}

_DWFTK_API
const char*
DWFManifestReader::provideObjectID( const char* zObjectID )
throw( DWFException )
{
    return zObjectID;
}

_DWFTK_API
DWFDependency*
DWFManifestReader::provideDependency( DWFDependency* pDependency )
throw( DWFException )
{
    return pDependency;
}

_DWFTK_API
DWFInterface*
DWFManifestReader::provideInterface( DWFInterface* pInterface )
throw( DWFException )
{
    return pInterface;
}

_DWFTK_API
DWFProperty*
DWFManifestReader::provideProperty( DWFProperty* pProperty )
throw( DWFException )
{
    return pProperty;
}

_DWFTK_API
DWFSection*
DWFManifestReader::provideSection( DWFSection* pSection )
throw( DWFException )
{
    return pSection;
}

_DWFTK_API
DWFContent*
DWFManifestReader::provideContent( DWFContent* pContent )
throw( DWFException )
{
    return pContent;
}

_DWFTK_API
DWFPackageContentPresentations*
DWFManifestReader::provideContentPresentations( DWFPackageContentPresentations* pPackageContentPresentations )
throw( DWFException )
{
    return pPackageContentPresentations;
}

_DWFTK_API
void
DWFManifestReader::notifyStartElement( const char*   zName,
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

        //
        //
        //
    switch (_nElementDepth)
    {
            //
            // dwf:Manifest
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Manifest) == 0)
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

                            _provideVersion( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
                        }
                            //
                            // provide the object id
                            //
                        else if ((_nProviderFlags & eProvideObjectID)   &&
                                 !(nFound & eProvideObjectID)           &&
                                 (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
                        {
                            nFound |= eProvideObjectID;

                            _provideObjectID( ppAttributeList[iAttrib+1] );
                        }
                    }
                }
            }
                //
                // we have an invalid DWF here, turn off all subsequent
                // processing and indicate the error somewhere...
                //
            else
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // dwf:Dependencies, dwf:Interfaces, dwf:Properties, dwf:Sections, dwf:Contents
            //
        case 1:
        {
            if ((_nProviderFlags & eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                _nCurrentCollectionProvider = eProvideProperties;
            }
            else if ((_nProviderFlags & eProvideInterfaces) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Interfaces) == 0))
            {
                _nCurrentCollectionProvider = eProvideInterfaces;
            }
            else if ((_nProviderFlags & eProvideSections) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Sections) == 0))
            {
                _nCurrentCollectionProvider = eProvideSections;
            }
            else if ((_nProviderFlags & eProvideDependencies) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Dependencies) == 0))
            {
                _nCurrentCollectionProvider = eProvideDependencies;
            }
            else if ((_nProviderFlags & eProvideContents) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Contents) == 0))
            {
                _nCurrentCollectionProvider = eProvideContents;
            }
            else if ((_nProviderFlags & eProvideContentPresentations) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Presentations) == 0))
            {
                _nCurrentCollectionProvider = eProvideContentPresentations;
                //
                // create a new package content presentations object
                //
                _pCurrentElement = _pElementBuilder->buildPackageContentPresentations( ppAttributeList, _pPackageReader );
            }
            else
            {
                _nCurrentCollectionProvider = eProvideNone;
            }

            break;
        }
            //
            // dwf:Dependency, dwf:Interface, dwf:Property, dwf:Section, dwf:Content
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // create new property object
                //
                _pCurrentElement = _pElementBuilder->buildProperty( ppAttributeList );
            }
            else if ((_nCurrentCollectionProvider == eProvideInterfaces) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Interface) == 0))
            {
                //
                // create new interface object
                //
                _pCurrentElement = _pElementBuilder->buildInterface( ppAttributeList );
            }
            else if ((_nCurrentCollectionProvider == eProvideSections) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Section) == 0))
            {
                //
                // create new section using the DWFSectionBuilder interface on the package reader
                //
                _pCurrentElement = _pPackageReader->getSectionBuilder().buildSection( ppAttributeList, _pPackageReader );
            }
            else if ((_nCurrentCollectionProvider == eProvideDependencies) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Dependency) == 0))
            {
                //
                // create new dependency object
                //
                _pCurrentElement = _pElementBuilder->buildDependency( ppAttributeList );
            }
            else if ((_nCurrentCollectionProvider == eProvideContents) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Content) == 0))
            {
                //
                // create a new content
                //
                _pCurrentElement = _pElementBuilder->buildContent( ppAttributeList, _pPackageReader );
            }
            break;
        }
            //
            // Section - dwf:Source, dwf:Toc
            // Dependency -
            //
        case 3:
        {
                //
                // add the source element to the current section
                //
            if ((_nCurrentCollectionProvider == eProvideSections) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Source) == 0))
            {
                DWFSource* pSource = _pElementBuilder->buildSource( ppAttributeList );
                DWFSection* pSection = dynamic_cast<DWFSection*>(_pCurrentElement);
                pSection->addSource( *pSource );

                DWFCORE_FREE_OBJECT( pSource );
            }
            break;
        }
            //
            // Section TOC - dwf:Resource
            // Dependency -
            //
        case 4:
        {
                //
                // add the resource element to the current section
                //
            if ((_nCurrentCollectionProvider == eProvideSections) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0))
            {
                DWFResource* pResource = _pElementBuilder->buildResource(ppAttributeList, _pPackageReader);
                ((DWFSection*)_pCurrentElement)->addResource( pResource, true );
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
DWFManifestReader::notifyEndElement( const char* /*zName*/ )
throw()
{
        //
        // decrement the depth first
        // this lets us match up the switch statements for start and end
        //
    switch (--_nElementDepth)
    {
            //
            // dwf:Dependencies, dwf:Interfaces, dwf:Properties, dwf:Contents, dwf:Sections, dwf:Presentations
            //
        case 1:
        {
            if (_nCurrentCollectionProvider == eProvideContentPresentations)
            {
                _provideContentPresentations( dynamic_cast<DWFPackageContentPresentations*>(_pCurrentElement) );
            }

            _nCurrentCollectionProvider = eProvideNone;
            break;
        }
            //
            // dwf:Dependency, dwf:Interface, dwf:Property, dwf:Content, dwf:Section
            //
        case 2:
        {
            if (_nCurrentCollectionProvider == eProvideProperties)
            {
                //
                // provide the new property object
                //
                _provideProperty( dynamic_cast<DWFProperty*>(_pCurrentElement) );
            }
            else if (_nCurrentCollectionProvider == eProvideInterfaces)
            {
                //
                // provide the new interface object
                //
                _provideInterface( dynamic_cast<DWFInterface*>(_pCurrentElement) );
            }
            else if (_nCurrentCollectionProvider == eProvideSections)
            {
                //
                // provide the new [typed if possible] section object
                //
                _provideSection( dynamic_cast<DWFSection*>(_pCurrentElement) );
            }
            else if (_nCurrentCollectionProvider == eProvideDependencies)
            {
                //
                // provide the new dependency object
                //
                _provideDependency( dynamic_cast<DWFDependency*>(_pCurrentElement) );
            }
            else if (_nCurrentCollectionProvider == eProvideContents)
            {
                //
                // provide the new content manager object
                //
                _provideContent( dynamic_cast<DWFContent*>(_pCurrentElement) );
            }

            _pCurrentElement = NULL;
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
DWFManifestReader::notifyStartNamespace( const char* /*zPrefix*/,
                                         const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFManifestReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFManifestReader::notifyCharacterData( const char* /*zCData*/,
                                        int         /*nLength*/ )
throw()
{
}

void
DWFManifestReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFManifestReader::_provideObjectID( const char* zObjectID )
throw()
{
    provideObjectID( _pFilter ? _pFilter->provideObjectID(zObjectID) : zObjectID );
}

void
DWFManifestReader::_provideDependency( DWFDependency* pDependency )
throw()
{
    provideDependency( _pFilter ? _pFilter->provideDependency(pDependency) : pDependency );
}

void
DWFManifestReader::_provideInterface( DWFInterface* pInterface )
throw()
{
    provideInterface( _pFilter ? _pFilter->provideInterface(pInterface) : pInterface );
}

void
DWFManifestReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFManifestReader::_provideSection( DWFSection* pSection )
throw()
{
    provideSection( _pFilter ? _pFilter->provideSection(pSection) : pSection );
}

void 
DWFManifestReader::_provideContent( DWFContent* pContent )
throw()
{
    provideContent( _pFilter ? _pFilter->provideContent(pContent) : pContent );
}

void 
DWFManifestReader::_provideContentPresentations( DWFPackageContentPresentations* pPackageContentPresentations )
throw()
{
    provideContentPresentations( _pFilter ? _pFilter->provideContentPresentations(pPackageContentPresentations) : pPackageContentPresentations );
}

