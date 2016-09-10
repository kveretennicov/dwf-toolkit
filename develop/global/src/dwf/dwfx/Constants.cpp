//
//  Copyright (c) 2006 by Autodesk, Inc.
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


#include "dwf/dwfx/Constants.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;

//
// Holds the singleton - will be deleted when this module unloads
//
DWFPointer<DWFXXML>             g_apDWFXXML( NULL, false );
DWFPointer<DWFXXML::_tList>     DWFXXML::_kapRoleMap( NULL, false );
DWFPointer<DWFXXML::_tList>     DWFXXML::_kapRelationshipMap( NULL, false );

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFXXML::kzSchemaID_DWFProperties = "6B241FB6-E174-4B01-9685-60381F320DA7";

const char* const DWFXXML::kzNamespaceURI_DWFX = "http://schemas.dwf.autodesk.com/dwfx/2006/11";

const char* const DWFXXML::kzElement_DWFDocumentSequence = "DWFDocumentSequence";
const char* const DWFXXML::kzElement_ManifestReference = "ManifestReference";
const char* const DWFXXML::kzElement_DWFProperties = "DWFProperties";

const char* const DWFXXML::kzAttribute_Source = "Source";

const char* const DWFXXML::kzPrefix_ResourceInternalID = "dwfresource_";

const wchar_t* const DWFXXML::kzRelationship_Document                             = L"http://schemas.autodesk.com/dwfx/2007/relationships/document";
const wchar_t* const DWFXXML::kzRelationship_Content                              = L"http://schemas.autodesk.com/dwfx/2007/relationships/content";
const wchar_t* const DWFXXML::kzRelationship_DocumentPresentations                = L"http://schemas.autodesk.com/dwfx/2007/relationships/documentpresentations";
const wchar_t* const DWFXXML::kzRelationship_Section                              = L"http://schemas.autodesk.com/dwfx/2007/relationships/section";
const wchar_t* const DWFXXML::kzRelationship_DocumentSequence                     = L"http://schemas.autodesk.com/dwfx/2007/relationships/documentsequence";
const wchar_t* const DWFXXML::kzRelationship_RequiredResource                     = L"http://schemas.autodesk.com/dwfx/2007/relationships/requiredresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics2dResource                   = L"http://schemas.autodesk.com/dwfx/2007/relationships/graphics2dresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics3dResource                   = L"http://schemas.autodesk.com/dwfx/2007/relationships/3dstreaminggraphicsresource";
const wchar_t* const DWFXXML::kzRelationship_DescriptorResource                   = L"http://schemas.autodesk.com/dwfx/2007/relationships/descriptorresource";
const wchar_t* const DWFXXML::kzRelationship_ThumbnailResource                    = L"http://schemas.autodesk.com/dwfx/2007/relationships/thumbnailresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics2dOverlayResource            = L"http://schemas.autodesk.com/dwfx/2007/relationships/2dvectoroverlayresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics2dMarkupResource             = L"http://schemas.autodesk.com/dwfx/2007/relationships/2dvectormarkupresource";
const wchar_t* const DWFXXML::kzRelationship_RmlMarkupResource                    = L"http://schemas.autodesk.com/dwfx/2007/relationships/RMLmarkupresource";
const wchar_t* const DWFXXML::kzRelationship_PreviewResource                      = L"http://schemas.autodesk.com/dwfx/2007/relationships/previewresource";
const wchar_t* const DWFXXML::kzRelationship_OverlayPreviewResource               = L"http://schemas.autodesk.com/dwfx/2007/relationships/overlaypreviewresource";
const wchar_t* const DWFXXML::kzRelationship_MarkupPreviewResource                = L"http://schemas.autodesk.com/dwfx/2007/relationships/markuppreviewresource";
const wchar_t* const DWFXXML::kzRelationship_RasterOverlayResource                = L"http://schemas.autodesk.com/dwfx/2007/relationships/rasteroverlayresource";
const wchar_t* const DWFXXML::kzRelationship_RasterMarkupResource                 = L"http://schemas.autodesk.com/dwfx/2007/relationships/rastermarkupresource";
const wchar_t* const DWFXXML::kzRelationship_FontResource                         = L"http://schemas.autodesk.com/dwfx/2007/relationships/fontresource";
const wchar_t* const DWFXXML::kzRelationship_MetadataResource                     = L"http://schemas.autodesk.com/dwfx/2007/relationships/metadataresource";
const wchar_t* const DWFXXML::kzRelationship_ObjectDefinitionResource             = L"http://schemas.autodesk.com/dwfx/2007/relationships/objectdefinitionresource";
const wchar_t* const DWFXXML::kzRelationship_MarkupObjectDefinitionResource       = L"http://schemas.autodesk.com/dwfx/2007/relationships/markupobjectdefinitionresource";
const wchar_t* const DWFXXML::kzRelationship_TextureResource                      = L"http://schemas.autodesk.com/dwfx/2007/relationships/textureresource";
const wchar_t* const DWFXXML::kzRelationship_ContentPresentationResource          = L"http://schemas.autodesk.com/dwfx/2007/relationships/contentpresentationresource";
const wchar_t* const DWFXXML::kzRelationship_MarkupContentPresentationResource    = L"http://schemas.autodesk.com/dwfx/2007/relationships/markupcontentpresentationresource";
const wchar_t* const DWFXXML::kzRelationship_AnimationResource                    = L"http://schemas.autodesk.com/dwfx/2007/relationships/animationresource";
const wchar_t* const DWFXXML::kzRelationship_NoteResource                         = L"http://schemas.autodesk.com/dwfx/2007/relationships/noteresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics3dMarkupResource             = L"http://schemas.autodesk.com/dwfx/2007/relationships/3Dmarkupgraphicsresource";
const wchar_t* const DWFXXML::kzRelationship_ContentDefinitionResource            = L"http://schemas.autodesk.com/dwfx/2007/relationships/contentdefinitionresource";
const wchar_t* const DWFXXML::kzRelationship_IconResource                         = L"http://schemas.autodesk.com/dwfx/2007/relationships/iconresource";
const wchar_t* const DWFXXML::kzRelationship_ObservationMeshResource              = L"http://schemas.autodesk.com/dwfx/2007/relationships/observationmeshresource";
const wchar_t* const DWFXXML::kzRelationship_GeographicCoordinateSystemResource   = L"http://schemas.autodesk.com/dwfx/2007/relationships/geographiccoordinatesystemresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics2dExtensionResource          = L"http://schemas.autodesk.com/dwfx/2007/relationships/graphics2dextensionresource";
const wchar_t* const DWFXXML::kzRelationship_Graphics2dDictionaryResource         = L"http://schemas.autodesk.com/dwfx/2007/relationships/graphics2ddictionaryresource";
const wchar_t* const DWFXXML::kzRelationship_RasterReferenceResource              = L"http://schemas.autodesk.com/dwfx/2007/relationships/rasterreferenceresource";
const wchar_t* const DWFXXML::kzRelationship_DWFProperties                        = L"http://schemas.autodesk.com/dwfx/2007/relationships/dwfproperties";
const wchar_t* const DWFXXML::kzRelationship_CustomProperties                     = L"http://schemas.autodesk.com/dwfx/2007/relationships/customproperties";
//DNT_End

#endif

DWFXXML::DWFXXML()
throw()
{
    ;
}

_DWFTK_API
const wchar_t* const
DWFXXML::GetRelationship( const wchar_t* const zRole )
throw( DWFException )
{
    //
    //  If an empty role is passed in, then there will be no corresponding relationship.
    //
    if (zRole == NULL)
    {
        return NULL;
    }

        //
        // create the map on first access
        //
    if (g_apDWFXXML.isNull())
    {
        _Build();
    }

    const wchar_t** pzRelationship = _kapRoleMap->find( zRole );
    
    return (pzRelationship ? *pzRelationship : NULL);
}

_DWFTK_API
const wchar_t* const
DWFXXML::GetRole( const wchar_t* const zRelationship )
throw( DWFException )
{
    //
    //  If an empty relationship string is passed in, there is no corresponding role.
    //
    if (zRelationship == NULL)
    {
        return NULL;
    }

        //
        // create the map on first access
        //
    if (g_apDWFXXML.isNull())
    {
        _Build();
    }

    const wchar_t** pzRole = _kapRelationshipMap->find( zRelationship );
    
    return (pzRole ? *pzRole : NULL);
}

void
DWFXXML::_Build()
throw( DWFException )
{
    g_apDWFXXML = DWFCORE_ALLOC_OBJECT( DWFXXML );
    if (g_apDWFXXML.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
    }

    _kapRoleMap = DWFCORE_ALLOC_OBJECT( DWFXXML::_tList );
    if (_kapRoleMap.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
    }

    _kapRelationshipMap = DWFCORE_ALLOC_OBJECT( DWFXXML::_tList );
    if (_kapRelationshipMap.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
    }

    //
    // Role -> Relationship
    //
    _kapRoleMap->insert( kzRelationship_Graphics2dResource,                 DWFXML::kzRole_Graphics2d );
    _kapRoleMap->insert( kzRelationship_Graphics3dResource,                 DWFXML::kzRole_Graphics3d );
    _kapRoleMap->insert( kzRelationship_DescriptorResource,                 DWFXML::kzRole_Descriptor );
    _kapRoleMap->insert( kzRelationship_ThumbnailResource,                  DWFXML::kzRole_Thumbnail );
    _kapRoleMap->insert( kzRelationship_Graphics2dOverlayResource,          DWFXML::kzRole_Graphics2dOverlay );
    _kapRoleMap->insert( kzRelationship_Graphics2dMarkupResource,           DWFXML::kzRole_Graphics2dMarkup );
    _kapRoleMap->insert( kzRelationship_RmlMarkupResource,                  DWFXML::kzRole_RmlMarkup );
    _kapRoleMap->insert( kzRelationship_PreviewResource,                    DWFXML::kzRole_Preview );
    _kapRoleMap->insert( kzRelationship_OverlayPreviewResource,             DWFXML::kzRole_OverlayPreview );
    _kapRoleMap->insert( kzRelationship_MarkupPreviewResource,              DWFXML::kzRole_MarkupPreview );
    _kapRoleMap->insert( kzRelationship_RasterOverlayResource,              DWFXML::kzRole_RasterOverlay );
    _kapRoleMap->insert( kzRelationship_RasterMarkupResource,               DWFXML::kzRole_RasterMarkup );
    _kapRoleMap->insert( kzRelationship_FontResource,                       DWFXML::kzRole_Font );
    _kapRoleMap->insert( kzRelationship_MetadataResource,                   DWFXML::kzRole_Metadata );
    _kapRoleMap->insert( kzRelationship_ObjectDefinitionResource,           DWFXML::kzRole_ObjectDefinition );
    _kapRoleMap->insert( kzRelationship_MarkupObjectDefinitionResource,     DWFXML::kzRole_MarkupObjectDefinition );
    _kapRoleMap->insert( kzRelationship_TextureResource,                    DWFXML::kzRole_Texture );
    _kapRoleMap->insert( kzRelationship_ContentPresentationResource,        DWFXML::kzRole_ContentPresentation );
    _kapRoleMap->insert( kzRelationship_MarkupContentPresentationResource,  DWFXML::kzRole_MarkupContentPresentation );
    _kapRoleMap->insert( kzRelationship_AnimationResource,                  DWFXML::kzRole_Animation );
    _kapRoleMap->insert( kzRelationship_NoteResource,                       DWFXML::kzRole_Note );
    _kapRoleMap->insert( kzRelationship_Graphics3dMarkupResource,           DWFXML::kzRole_Graphics3dMarkup );
    _kapRoleMap->insert( kzRelationship_ContentDefinitionResource,          DWFXML::kzRole_ContentDefinition );
    _kapRoleMap->insert( kzRelationship_IconResource,                       DWFXML::kzRole_Icon );
    _kapRoleMap->insert( kzRelationship_ObservationMeshResource,            DWFXML::kzRole_ObservationMesh );
    _kapRoleMap->insert( kzRelationship_GeographicCoordinateSystemResource, DWFXML::kzRole_GeographicCoordinateSystem );
    _kapRoleMap->insert( kzRelationship_Graphics2dExtensionResource,        DWFXML::kzRole_Graphics2dExtension );
    _kapRoleMap->insert( kzRelationship_Graphics2dDictionaryResource,       DWFXML::kzRole_Graphics2dDictionary );
    _kapRoleMap->insert( kzRelationship_RasterReferenceResource,            DWFXML::kzRole_RasterReference );

    //
    // Relationship -> Role
    //
    _kapRoleMap->insert( DWFXML::kzRole_Graphics2d,                   kzRelationship_Graphics2dResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics3d,                   kzRelationship_Graphics3dResource );
    _kapRoleMap->insert( DWFXML::kzRole_Descriptor,                   kzRelationship_DescriptorResource );
    _kapRoleMap->insert( DWFXML::kzRole_Thumbnail,                    kzRelationship_ThumbnailResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics2dOverlay,            kzRelationship_Graphics2dOverlayResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics2dMarkup,             kzRelationship_Graphics2dMarkupResource );
    _kapRoleMap->insert( DWFXML::kzRole_RmlMarkup,                    kzRelationship_RmlMarkupResource );
    _kapRoleMap->insert( DWFXML::kzRole_Preview,                      kzRelationship_PreviewResource );
    _kapRoleMap->insert( DWFXML::kzRole_OverlayPreview,               kzRelationship_OverlayPreviewResource );
    _kapRoleMap->insert( DWFXML::kzRole_MarkupPreview,                kzRelationship_MarkupPreviewResource );
    _kapRoleMap->insert( DWFXML::kzRole_RasterOverlay,                kzRelationship_RasterOverlayResource );
    _kapRoleMap->insert( DWFXML::kzRole_RasterMarkup,                 kzRelationship_RasterMarkupResource );
    _kapRoleMap->insert( DWFXML::kzRole_Font,                         kzRelationship_FontResource );
    _kapRoleMap->insert( DWFXML::kzRole_Metadata,                     kzRelationship_MetadataResource );
    _kapRoleMap->insert( DWFXML::kzRole_ObjectDefinition,             kzRelationship_ObjectDefinitionResource );
    _kapRoleMap->insert( DWFXML::kzRole_MarkupObjectDefinition,       kzRelationship_MarkupObjectDefinitionResource );
    _kapRoleMap->insert( DWFXML::kzRole_Texture,                      kzRelationship_TextureResource );
    _kapRoleMap->insert( DWFXML::kzRole_ContentPresentation,          kzRelationship_ContentPresentationResource );
    _kapRoleMap->insert( DWFXML::kzRole_MarkupContentPresentation,    kzRelationship_MarkupContentPresentationResource );
    _kapRoleMap->insert( DWFXML::kzRole_Animation,                    kzRelationship_AnimationResource );
    _kapRoleMap->insert( DWFXML::kzRole_Note,                         kzRelationship_NoteResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics3dMarkup,             kzRelationship_Graphics3dMarkupResource );
    _kapRoleMap->insert( DWFXML::kzRole_ContentDefinition,            kzRelationship_ContentDefinitionResource );
    _kapRoleMap->insert( DWFXML::kzRole_Icon,                         kzRelationship_IconResource );
    _kapRoleMap->insert( DWFXML::kzRole_ObservationMesh,              kzRelationship_ObservationMeshResource );
    _kapRoleMap->insert( DWFXML::kzRole_GeographicCoordinateSystem,   kzRelationship_GeographicCoordinateSystemResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics2dExtension,          kzRelationship_Graphics2dExtensionResource );
    _kapRoleMap->insert( DWFXML::kzRole_Graphics2dDictionary,         kzRelationship_Graphics2dDictionaryResource );
    _kapRoleMap->insert( DWFXML::kzRole_RasterReference,              kzRelationship_RasterReferenceResource );
}
