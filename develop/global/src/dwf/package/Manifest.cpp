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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Manifest.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/Version.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Constants.h"
#include "dwf/package/ContentManager.h"
using namespace DWFToolkit;



_DWFTK_API
DWFManifest::DWFManifest( DWFPackageReader* pPackageReader )
throw()
           : DWFManifestReader( pPackageReader )
           , _nVersion( 0.0 )
           , _zObjectID()
           , _oSectionsInOrder()
           , _oGlobalSectionsInOrder()
           , _oSectionsByName()
           , _oSectionsByType()
           , _oInterfaces()
           , _pContentManager( NULL )
           , _pPackageContentPresentations( NULL )
{
    _pContentManager = DWFCORE_ALLOC_OBJECT( DWFContentManager(pPackageReader) );
    _pContentManager->own( *this );
}

_DWFTK_API
DWFManifest::DWFManifest( const DWFString& zObjectID )
throw()
           : DWFPropertyContainer()
           , _nVersion( _DWF_FORMAT_MANIFEST_VERSION_CURRENT_FLOAT )
           , _zObjectID( zObjectID )
           , _oSectionsInOrder()
           , _oGlobalSectionsInOrder()
           , _oSectionsByName()
           , _oSectionsByType()
           , _oInterfaces()
           , _pContentManager( NULL )
           , _pPackageContentPresentations( NULL )
{
    _pContentManager = DWFCORE_ALLOC_OBJECT( DWFContentManager() );
    _pContentManager->own( *this );
}

_DWFTK_API
DWFManifest::~DWFManifest()
throw()
{
    tInterfaceIterator* piInterface = _oInterfaces.iterator();
    if (piInterface)
    {
        for(; piInterface->valid(); piInterface->next())
        {
            DWFCORE_FREE_OBJECT( piInterface->value() );
        }

        DWFCORE_FREE_OBJECT( piInterface );
    }

    DWFSection::tList::iterator iSection = _oGlobalSectionsInOrder.begin();
    for (; iSection != _oGlobalSectionsInOrder.end();
           iSection++)
    {
            //
            // delete if owned
            //
        DWFSection *pSection = *iSection;
        if (pSection->owner() == this)
        {
            DWFCORE_FREE_OBJECT( pSection );
        }
            //
            // unobserve if not owned
            //
        else
        {
            pSection->unobserve(*this);
        }
    }

    for (iSection = _oSectionsInOrder.begin();
         iSection != _oSectionsInOrder.end();
         iSection++)
    {
            //
            // delete if owned
            //
        DWFSection *pSection = *iSection;
        if (pSection->owner() == this)
        {
            DWFCORE_FREE_OBJECT( pSection );
        }
            //
            // unobserve if not owned
            //
        else
        {
            pSection->unobserve(*this);
        }
    }

    if (_pContentManager)
    {
            //
            // delete if owned
            //
        if (_pContentManager->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pContentManager );
        }
            //
            // unobserve if not owned
            //
        else
        {
            _pContentManager->unobserve( *this );
        }
    }

    if (_pPackageContentPresentations)
    {
        DWFCORE_FREE_OBJECT( _pPackageContentPresentations );
    }
}

_DWFTK_API
void
DWFManifest::attachContentManager( DWFContentManager* pContentManager,
                                   bool bTakeOwnership,
                                   bool bDeletePrevious )
throw( DWFException )
{
        //
        // It sometimes happens that attach gets called twice with the same manager.
        // Combined with the default of bDeletePrevious being false, we might delete
        // the one-and-only content manager. So, no matter what bDeletePrevious says
        // if we're being asked to set the same content manager again, don't delete.
        //
    if(pContentManager == _pContentManager)
    {
        bDeletePrevious = false;
    }

    if (bDeletePrevious)
    {
        DWFCORE_FREE_OBJECT( _pContentManager );
    }
    else
    {
        _pContentManager->unobserve(*this);
    }

    _pContentManager = pContentManager;

        //
        // If we're told to take ownership, then do so.
        //
    if (bTakeOwnership)
    {
        if (_pContentManager->owner() != this)
        {
            _pContentManager->own( *this );
        }
    }
        //
        // If we're not supposed to take ownership, then at least inform that we're observing.
        //
    else
    {
        _pContentManager->observe( *this );
    }
}

_DWFTK_API
double
DWFManifest::provideVersion( double nVersion )
throw()
{
    _nVersion = nVersion;

    return nVersion;
}

_DWFTK_API
const char*
DWFManifest::provideObjectID( const char* zObjectID )
throw()
{
    _zObjectID.assign( zObjectID );

    return zObjectID;
}

_DWFTK_API
DWFDependency*
DWFManifest::provideDependency( DWFDependency* pDependency )
throw()
{
    if (pDependency)
    {
        DWFCORE_FREE_OBJECT( pDependency );
    }

    return NULL;
}

_DWFTK_API
DWFInterface*
DWFManifest::provideInterface( DWFInterface* pInterface )
throw()
{
    if (pInterface)
    {
        const wchar_t* zID = pInterface->objectID();
        DWFInterface** ppKnown = _oInterfaces.find( zID );

        if (ppKnown == NULL)
        {
            _oInterfaces.insert( zID, pInterface );
        }
        else
        {
            DWFCORE_FREE_OBJECT( pInterface );
            pInterface = *ppKnown;
        }
    }

    return pInterface;
}

_DWFTK_API
DWFProperty*
DWFManifest::provideProperty( DWFProperty* pProperty )
throw()
{
    if (pProperty)
    {
        addProperty( pProperty, true );
    }

    return pProperty;
}

_DWFTK_API
DWFSection*
DWFManifest::provideSection( DWFSection* pSection )
throw()
{
    if (pSection)
    {
            //
            // preserve order
            //
        DWFGlobalSection* pGlobal = dynamic_cast<DWFGlobalSection*>(pSection);
        if (pGlobal)
        {
            _oGlobalSectionsInOrder.push_back( pSection );
        }
        else
        {
            _oSectionsInOrder.push_back( pSection );
        }

        //
        // key object by it's own name for fast look up
        //
        _oSectionsByName.insert( pSection->name(), pSection );
        _oSectionsByType.insert( DWFSection::tMultiMap::value_type(pSection->type(), pSection) );

            //
            // if unowned, claim ownership
            //
        if (pSection->owner() == NULL)
        {
            pSection->own( *this );
        }
            //
            // else, report that we are watching.
        else
        {
            pSection->observe( *this );
        }

        pSection->setContentManager( _pContentManager );
    }

    return pSection;
}


_DWFTK_API
DWFContent*
DWFManifest::provideContent( DWFContent* pContent )
throw( DWFException )
{
    pContent = _pContentManager->insertContent( pContent );

    return pContent;
}

_DWFTK_API
DWFPackageContentPresentations*
DWFManifest::provideContentPresentations( DWFPackageContentPresentations* pPackageContentPresentations )
throw()
{
	if(pPackageContentPresentations)
	{
		_pPackageContentPresentations = pPackageContentPresentations;
		_pPackageContentPresentations->load();
	}
    return pPackageContentPresentations;
}

_DWFTK_API
DWFManifest::tInterfaceIterator*
DWFManifest::getInterfaces()
throw()
{
    return _oInterfaces.iterator();
}

_DWFTK_API
DWFSection*
DWFManifest::findSectionByName( const DWFString& zName )
throw()
{
    DWFSection** ppSection = _oSectionsByName.find( (const wchar_t*)zName );
    return (ppSection ? *ppSection : NULL);
}

_DWFTK_API
DWFManifest::SectionIterator*
DWFManifest::findSectionsByType( const DWFString& zType )
throw()
{
    return DWFCORE_ALLOC_OBJECT( SectionIterator(_oSectionsByType.lower_bound((const wchar_t*)zType), _oSectionsByType.upper_bound((const wchar_t*)zType)) );
}

_DWFTK_API
DWFManifest::SectionIterator*
DWFManifest::getSections()
throw()
{
    return DWFCORE_ALLOC_OBJECT( SectionIterator(_oSectionsInOrder.begin(), _oSectionsInOrder.end()) );
}

_DWFTK_API
DWFManifest::SectionIterator*
DWFManifest::getGlobalSections()
throw()
{
    return DWFCORE_ALLOC_OBJECT( SectionIterator(_oGlobalSectionsInOrder.begin(), _oGlobalSectionsInOrder.end()) );
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFManifest::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    wchar_t     zTempBuffer[16] = {0};
    DWFString   zTempString;

    //
    // root element
    //
    rSerializer.startElement( DWFXML::kzElement_Manifest, DWFXML::kzNamespace_DWF );
    {
            //
            // attributes
            //
        {
            _DWFCORE_SWPRINTF( zTempBuffer, 16, /*NOXLATE*/L"%#2.2g", _nVersion );

            zTempString.assign( /*NOXLATE*/L"DWF-Manifest:" );
            zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

            rSerializer.addAttribute( /*NOXLATE*/L"dwf", zTempString, /*NOXLATE*/L"xmlns:" );
            rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );
            rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
        }

            //
            // interfaces
            //
        {
            if (_oInterfaces.size() > 0)
            {
                //
                // manually create the collection since there is no corresponding dwf object
                //
                rSerializer.startElement( DWFXML::kzElement_Interfaces, DWFXML::kzNamespace_DWF );

                tInterfaceIterator* piInterface = _oInterfaces.iterator();
                if (piInterface)
                {
                    for(; piInterface->valid(); piInterface->next())
                    {
                        //
                        // let each interface write itself into the XML document
                        //
                        piInterface->value()->serializeXML( rSerializer, nFlags );
                    }

                    DWFCORE_FREE_OBJECT( piInterface );
                }

                rSerializer.endElement();
            }
                //
                // the DWF specification says that there must be at least one interface
                //
            else
            {
                _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"There must be at least one document interface" );
            }
        }

            //
            // properties
            //
        {
            //
            // manually create the collection since there is no corresponding dwf object
            //
            rSerializer.startElement( DWFXML::kzElement_Properties, DWFXML::kzNamespace_DWF );

            DWFProperty::tMap::Iterator* piProperty = _oProperties.iterator();

            if (piProperty)
            {
                for (; piProperty->valid(); piProperty->next())
                {
                    //
                    // let each property write itself into the XML document
                    //
                    piProperty->value()->serializeXML( rSerializer, nFlags );
                }

                DWFCORE_FREE_OBJECT( piProperty );
            }

            rSerializer.endElement();
        }

        //
        // list of contents
        //
        if (_pContentManager != NULL)
        {
            _pContentManager->serializeXML( rSerializer, nFlags );
        }

        //
        // package level presentations
        //
        if (_pPackageContentPresentations && _pPackageContentPresentations->presentationCount() > 0)
        {
            _pPackageContentPresentations->serializeXML( rSerializer, nFlags );
        }

        //
        // manually create the collection since there is no corresponding dwf object
        //
        rSerializer.startElement( DWFXML::kzElement_Sections, DWFXML::kzNamespace_DWF );

            //
            // global sections
            //
        {
            DWFSection::tList::iterator iGlobal = _oGlobalSectionsInOrder.begin();

            for (; iGlobal != _oGlobalSectionsInOrder.end();
                   iGlobal++)
            {
                //
                // let each global section write itself into the XML document
                //
                (*iGlobal)->serializeXML( rSerializer, nFlags );
            }
        }

            //
            // sections
            //
        {
            DWFSection::tList::iterator iSection = _oSectionsInOrder.begin();

            for (; iSection != _oSectionsInOrder.end();
                   iSection++)
            {
                //
                // let each section write itself into the XML document
                //
                (*iSection)->serializeXML( rSerializer, nFlags );
            }
        }
        rSerializer.endElement();
    }
    rSerializer.endElement();
}

#endif


_DWFTK_API
void
DWFManifest::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFManifest::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    DWFSection* pSection = dynamic_cast<DWFSection*>(&rOwnable);
    if (pSection)
    {
        //
        // try removing it from the sections-by-name list
        //
        _oSectionsByName.erase( pSection->name() );

        //
        // try removing it from the sections-in-order list
        //
        DWFGlobalSection* pGlobal = dynamic_cast<DWFGlobalSection*>(pSection);

        DWFSection::tList* pSectionList = pGlobal ? &_oGlobalSectionsInOrder : &_oSectionsInOrder;
        DWFSection::tList::iterator iSection = pSectionList->begin();;
        for (; iSection != pSectionList->end(); iSection++)
        {
                //
                // remove if found
                //
            if ((*iSection) == pSection)
            {
                pSectionList->erase(iSection);
                break;
            }
        }

        //
        // try removing it from the sections-by-type list
        //
        DWFSection::tMultiMap::iterator pIter = _oSectionsByType.begin();
        for (; pIter != _oSectionsByType.end(); pIter++)
        {
                //
                // remove if found
                //
            if (pIter->second == pSection)
            {
                _oSectionsByType.erase(pIter);
                break;
            }
        }

        return;
    }

    DWFProperty* pProperty = dynamic_cast<DWFProperty*>(&rOwnable);
    if (pProperty)
    {
        _oProperties.erase( pProperty->category(), pProperty->name() );

        return;
    }

    DWFContentManager* pContentManager = dynamic_cast<DWFContentManager*>(&rOwnable);
    if (pContentManager)
    {
        _pContentManager = NULL;
    }
}

