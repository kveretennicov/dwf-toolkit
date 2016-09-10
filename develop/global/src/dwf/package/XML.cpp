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


#include "dwf/package/XML.h"

#include "dwf/package/Bookmark.h"
#include "dwf/package/DefinedObject.h"
#include "dwf/package/Dependency.h"
#include "dwf/package/FontResource.h"
#include "dwf/package/GraphicResource.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/presentation/ContentPresentationResource.h"
#include "dwf/package/SignatureResource.h"
#include "dwf/package/Interface.h"
#include "dwf/package/Paper.h"
#include "dwf/package/Property.h"
#include "dwf/package/Source.h"
#include "dwf/package/Units.h"
#include "dwf/package/reader/PackageReader.h"
#include "dwf/package/Content.h"
#include "dwf/package/CoordinateSystem.h"
#include "dwf/presentation/PackageContentPresentations.h"
#include "dwf/package/Constants.h"

using namespace DWFToolkit;



//////////

template<class T>
T* _build( T*& pT, const char** ppAttributeList, DWFPackageReader* pPackageReader )
throw( DWFException )
{
    pT = DWFCORE_ALLOC_OBJECT( T(pPackageReader) );

    if (pT == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pT->parseAttributeList( ppAttributeList );
    return pT;
}

template<class T>
T* _build( T*& pT, const char** ppAttributeList )
throw( DWFException )
{
    pT = DWFCORE_ALLOC_OBJECT( T );

    if (pT == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pT->parseAttributeList( ppAttributeList );
    return pT;
}

//////////

DWFXMLElementBuilder::DWFXMLElementBuilder()
throw()
{
    ;
}

DWFXMLElementBuilder::~DWFXMLElementBuilder()
throw()
{
    ;
}

DWFResource*
DWFXMLElementBuilder::buildResource( const char**      ppAttributeList,
                                     DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFFontResource*
DWFXMLElementBuilder::buildFontResource( const char**      ppAttributeList,
                                         DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFFontResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFImageResource*
DWFXMLElementBuilder::buildImageResource( const char**      ppAttributeList,
                                          DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFImageResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFGraphicResource*
DWFXMLElementBuilder::buildGraphicResource( const char**      ppAttributeList,
                                            DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFGraphicResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFSectionContentResource*
DWFXMLElementBuilder::buildSectionContentResource( const char**      ppAttributeList,
                                                   DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFSectionContentResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFContentPresentationResource*
DWFXMLElementBuilder::buildContentPresentationResource( const char**      ppAttributeList,
                                                        DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFContentPresentationResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFSignatureResource*
DWFXMLElementBuilder::buildSignatureResource( const char**      ppAttributeList,
                                              DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFSignatureResource* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFBookmark*
DWFXMLElementBuilder::buildBookmark( const char** ppAttributeList )
throw( DWFException )
{
    DWFBookmark* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFDefinedObject*
DWFXMLElementBuilder::buildDefinedObject( const char** ppAttributeList )
throw( DWFException )
{
    DWFDefinedObject* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFDefinedObjectInstance*
DWFXMLElementBuilder::buildDefinedObjectInstance( const char**  ppAttributeList,
                                                  unsigned long nSequence )
throw( DWFException )
{
    DWFDefinedObjectInstance* pElement = DWFCORE_ALLOC_OBJECT( DWFDefinedObjectInstance(nSequence) );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList );
    return pElement;
}

DWFDependency*
DWFXMLElementBuilder::buildDependency( const char** ppAttributeList )
throw( DWFException )
{
    DWFDependency* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFInterface*
DWFXMLElementBuilder::buildInterface( const char** ppAttributeList )
throw( DWFException )
{
    DWFInterface* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFPaper*
DWFXMLElementBuilder::buildPaper( const char** ppAttributeList )
throw( DWFException )
{
    DWFPaper* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFProperty*
DWFXMLElementBuilder::buildProperty( const char** ppAttributeList, bool bIgnoreCustomizeAttribute )
throw( DWFException )
{
	if(bIgnoreCustomizeAttribute)
	{
		DWFProperty* pElement = DWFCORE_ALLOC_OBJECT( DWFProperty() );

		if (pElement == NULL)
		{
			_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFProperty object" );
		}

		pElement->parseAttributeList( ppAttributeList, bIgnoreCustomizeAttribute );
		return pElement;
	}

    DWFProperty* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFSource*
DWFXMLElementBuilder::buildSource( const char** ppAttributeList )
throw( DWFException )
{
    DWFSource* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFUnits*
DWFXMLElementBuilder::buildUnits( const char** ppAttributeList )
throw( DWFException )
{
    DWFUnits* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContent*
DWFXMLElementBuilder::buildContent( const char** ppAttributeList, DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFContent* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFClass*
DWFXMLElementBuilder::buildClass( const char** ppAttributeList,
                                  DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFClass* pElement = DWFCORE_ALLOC_OBJECT( DWFClass );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFFeature*
DWFXMLElementBuilder::buildFeature( const char** ppAttributeList,
                                    DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFFeature* pElement = DWFCORE_ALLOC_OBJECT( DWFFeature );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFEntity*
DWFXMLElementBuilder::buildEntity( const char** ppAttributeList,
                                   DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFEntity* pElement = DWFCORE_ALLOC_OBJECT( DWFEntity );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFObject*
DWFXMLElementBuilder::buildObject( const char** ppAttributeList,
                                   DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFObject* pElement = DWFCORE_ALLOC_OBJECT( DWFObject );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFGroup*
DWFXMLElementBuilder::buildGroup( const char** ppAttributeList,
                                  DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFGroup* pElement = DWFCORE_ALLOC_OBJECT( DWFGroup );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFPropertySet*
DWFXMLElementBuilder::buildPropertySet( const char** ppAttributeList,
                                        DWFXMLBuildable::tUnresolvedList& rUnresolved )
throw( DWFException )
{
    DWFPropertySet* pElement = DWFCORE_ALLOC_OBJECT( DWFPropertySet );

    if (pElement == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXMLBuildable object" );
    }

    pElement->parseAttributeList( ppAttributeList, rUnresolved );
    return pElement;
}

DWFInstance*
DWFXMLElementBuilder::buildInstance( const char** ppAttributeList )
throw( DWFException )
{
    DWFInstance* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContentPresentation*
DWFXMLElementBuilder::buildContentPresentation( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentation* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContentPresentationView*
DWFXMLElementBuilder::buildContentPresentationView( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationView* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFPropertyReference*
DWFXMLElementBuilder::buildPropertyReference( const char** ppAttributeList )
throw( DWFException )
{
    DWFPropertyReference* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContentPresentationNode*
DWFXMLElementBuilder::buildContentPresentationNode( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationNode* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContentPresentationReferenceNode*
DWFXMLElementBuilder::buildContentPresentationReferenceNode( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationReferenceNode* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFContentPresentationModelViewNode*
DWFXMLElementBuilder::buildContentPresentationModelViewNode( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationModelViewNode* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFCoordinateSystem*
DWFXMLElementBuilder::buildCoordinateSystem( const char** ppAttributeList )
throw( DWFException )
{
    DWFCoordinateSystem* pElement = _build( pElement, ppAttributeList );
    return pElement;
}

DWFPackageContentPresentations*
DWFXMLElementBuilder::buildPackageContentPresentations( const char** ppAttributeList,
                                                        DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFPackageContentPresentations* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

DWFResourceRelationship*
DWFXMLElementBuilder::buildRelationship( const char** ppAttributeList,
                                         DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFResourceRelationship* pElement = _build( pElement, ppAttributeList, pPackageReader );
    return pElement;
}

/////

_DWFTK_API
DWFXMLNamespace::DWFXMLNamespace( const DWFString& zNamespace, const DWFString& zXMLNS )
throw( DWFException )
            : DWFXMLNamespaceBase( zNamespace, zXMLNS )
{
        //
        // known DWF namespaces are prohibited
        //
    if ((_zPrefix == DWFXML::kzNamespace_DWF)           ||
        (_zPrefix == DWFXML::kzNamespace_EPlot)         ||
        (_zPrefix == DWFXML::kzNamespace_EModel)        ||
        (_zPrefix == DWFXML::kzNamespace_Data)          ||
        (_zPrefix == DWFXML::kzNamespace_ECommon))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Cannot provide a known DWF namespace to this method." );
    }
}

/////

#ifndef DWFTK_READ_ONLY

_DWFTK_API
DWFString
DWFXMLSerializable::namespaceXML( unsigned int nFlags ) const
throw()
{
    return (nFlags & DWFPackageWriter::eEPlot)      ? DWFXML::kzNamespace_EPlot         :
           (nFlags & DWFPackageWriter::eEModel)     ? DWFXML::kzNamespace_EModel        :
           (nFlags & DWFPackageWriter::eData)       ? DWFXML::kzNamespace_Data          :
           (nFlags & DWFPackageWriter::eSignatures) ? DWFXML::kzNamespace_Signatures    :
           (_zDefaultNamespace.bytes() > 0 )        ? _zDefaultNamespace                :
                                                      DWFXML::kzNamespace_DWF;
}

#endif


