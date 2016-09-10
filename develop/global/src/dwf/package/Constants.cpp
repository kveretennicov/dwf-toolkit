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


#include "dwf/package/Constants.h"


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFXML::kzNamespace_DWF                       = "dwf:";
const char* const DWFXML::kzNamespace_EPlot                     = "ePlot:";
const char* const DWFXML::kzNamespace_EModel                    = "eModel:";
const char* const DWFXML::kzNamespace_Data                      = "Data:";
const char* const DWFXML::kzNamespace_ECommon                   = "eCommon:";
const char* const DWFXML::kzNamespace_Signatures                = "Signatures:";

const char* const DWFXML::kzElement_TOC                         = "Toc";
const char* const DWFXML::kzElement_Node                        = "Node";
const char* const DWFXML::kzElement_Page                        = "Page";
const char* const DWFXML::kzElement_View                        = "View";
const char* const DWFXML::kzElement_Class                       = "Class";
const char* const DWFXML::kzElement_Group                       = "Group";
const char* const DWFXML::kzElement_Nodes                       = "Nodes";
const char* const DWFXML::kzElement_Paper                       = "Paper";
const char* const DWFXML::kzElement_Space                       = "Space";
const char* const DWFXML::kzElement_Units                       = "Units";
const char* const DWFXML::kzElement_Views                       = "Views";
const char* const DWFXML::kzElement_Entity                      = "Entity";
const char* const DWFXML::kzElement_Global                      = "Global";
const char* const DWFXML::kzElement_Groups                      = "Groups";
const char* const DWFXML::kzElement_Source                      = "Source";
const char* const DWFXML::kzElement_Object                      = "Object";
const char* const DWFXML::kzElement_Feature                     = "Feature";
const char* const DWFXML::kzElement_Objects                     = "Objects";
const char* const DWFXML::kzElement_Section                     = "Section";
const char* const DWFXML::kzElement_Classes                     = "Classes";
const char* const DWFXML::kzElement_Content                     = "Content";
const char* const DWFXML::kzElement_Contents                    = "Contents";
const char* const DWFXML::kzElement_Entities                    = "Entities";
const char* const DWFXML::kzElement_Features                    = "Features";
const char* const DWFXML::kzElement_Sections                    = "Sections";
const char* const DWFXML::kzElement_Manifest                    = "Manifest";
const char* const DWFXML::kzElement_Bookmark                    = "Bookmark";
const char* const DWFXML::kzElement_Instance                    = "Instance";
const char* const DWFXML::kzElement_Property                    = "Property";
const char* const DWFXML::kzElement_Resource                    = "Resource";
const char* const DWFXML::kzElement_Instances                   = "Instances";
const char* const DWFXML::kzElement_Interface                   = "Interface";
const char* const DWFXML::kzElement_Resources                   = "Resources";
const char* const DWFXML::kzElement_Interfaces                  = "Interfaces";
const char* const DWFXML::kzElement_Properties                  = "Properties";
const char* const DWFXML::kzElement_Dependency                  = "Dependency";
const char* const DWFXML::kzElement_Dependencies                = "Dependencies";
const char* const DWFXML::kzElement_FontResource                = "FontResource";
const char* const DWFXML::kzElement_Presentation                = "Presentation";
const char* const DWFXML::kzElement_Presentations               = "Presentations";
const char* const DWFXML::kzElement_ImageResource               = "ImageResource";
const char* const DWFXML::kzElement_ReferenceNode               = "ReferenceNode";
const char* const DWFXML::kzElement_ContentElement              = "ContentElement";
const char* const DWFXML::kzElement_SectionContent              = "SectionContent";
const char* const DWFXML::kzElement_GraphicResource             = "GraphicResource";
const char* const DWFXML::kzElement_ContentResource             = "ContentResource";
const char* const DWFXML::kzElement_ObjectDefinition            = "ObjectDefinition";
const char* const DWFXML::kzElement_SharedProperties            = "SharedProperties";
const char* const DWFXML::kzElement_PropertyReference           = "PropertyReference";
const char* const DWFXML::kzElement_PropertyReferences          = "PropertyReferences";
const char* const DWFXML::kzElement_ContentPresentation         = "ContentPresentation";
const char* const DWFXML::kzElement_PageObjectDefinition        = "PageObjectDefinition";
const char* const DWFXML::kzElement_SpaceObjectDefinition       = "SpaceObjectDefinition";
const char* const DWFXML::kzElement_GlobalObjectDefinition      = "GlobalObjectDefinition";
const char* const DWFXML::kzElement_ContentPresentationResource = "ContentPresentationResource";
const char* const DWFXML::kzElement_Camera                      = "Camera";
const char* const DWFXML::kzElement_Color                       = "Color";
const char* const DWFXML::kzElement_CuttingPlane                = "CuttingPlane";
const char* const DWFXML::kzElement_ModellingMatrix             = "ModellingMatrix";
const char* const DWFXML::kzElement_TextureMatrix               = "TextureMatrix";
const char* const DWFXML::kzElement_Visibility                  = "Visibility";
const char* const DWFXML::kzElement_InstanceVisibility          = "InstanceVisibility";
const char* const DWFXML::kzElement_InstanceTransparency        = "InstanceTransparency";
const char* const DWFXML::kzElement_GeometricVariation          = "GeometricVariation";
const char* const DWFXML::kzElement_ModelViewNode               = "ModelViewNode";
const char* const DWFXML::kzElement_InstanceAttributes          = "InstanceAttributes";
const char* const DWFXML::kzElement_ModelScene                  = "ModelScene";
const char* const DWFXML::kzElement_Channels                    = "Channels";
const char* const DWFXML::kzElement_Channel                     = "Channel";
const char* const DWFXML::kzElement_Planes                      = "Planes";
const char* const DWFXML::kzElement_Plane                       = "Plane";
const char* const DWFXML::kzElement_AttributeLock               = "AttributeLock";
const char* const DWFXML::kzElement_AttributeUnlock             = "AttributeUnlock";
const char* const DWFXML::kzElement_PropertyReferenceURI        = "PropertyReferenceURI";
const char* const DWFXML::kzElement_PropertyReferenceURIs       = "PropertyReferenceURIs";
const char* const DWFXML::kzElement_DisplayMode                 = "DisplayMode";
const char* const DWFXML::kzElement_UserAttributes              = "UserAttributes";
const char* const DWFXML::kzElement_SignatureResource           = "SignatureResource";
const char* const DWFXML::kzElement_CanonicalizationMethod      = "CanonicalizationMethod";
const char* const DWFXML::kzElement_DigestMethod                = "DigestMethod";
const char* const DWFXML::kzElement_DigestValue                 = "DigestValue";
const char* const DWFXML::kzElement_DSAKeyValue                 = "DSAKeyValue";
const char* const DWFXML::kzElement_DSAKeyValueP                = "P";
const char* const DWFXML::kzElement_DSAKeyValueQ                = "Q";
const char* const DWFXML::kzElement_DSAKeyValueG                = "G";
const char* const DWFXML::kzElement_DSAKeyValueY                = "Y";
const char* const DWFXML::kzElement_DSAKeyValueJ                = "J";
const char* const DWFXML::kzElement_DSAKeyValueSeed             = "Seed";
const char* const DWFXML::kzElement_DSAKeyValuePgenCounter      = "PgenCounter";
const char* const DWFXML::kzElement_KeyInfo                     = "KeyInfo";
const char* const DWFXML::kzElement_KeyName                     = "KeyName";
const char* const DWFXML::kzElement_KeyValue                    = "KeyValue";
const char* const DWFXML::kzElement_Reference                   = "Reference";
const char* const DWFXML::kzElement_RSAKeyValue                 = "RSAKeyValue";
const char* const DWFXML::kzElement_RSAKeyValueModulus          = "Modulus";
const char* const DWFXML::kzElement_RSAKeyValueExponent         = "Exponent";
const char* const DWFXML::kzElement_Signature                   = "Signature";
const char* const DWFXML::kzElement_SignatureMethod             = "SignatureMethod";
const char* const DWFXML::kzElement_SignatureValue              = "SignatureValue";
const char* const DWFXML::kzElement_SignedInfo                  = "SignedInfo";
const char* const DWFXML::kzElement_X509Certificate             = "X509Certificate";
const char* const DWFXML::kzElement_X509CRL                     = "X509CRL";
const char* const DWFXML::kzElement_X509Data                    = "X509Data";
const char* const DWFXML::kzElement_X509IssuerSerial            = "X509IssuerSerial";
const char* const DWFXML::kzElement_X509IssuerName              = "X509IssuerName";
const char* const DWFXML::kzElement_X509SerialNumber            = "X509SerialNumber";
const char* const DWFXML::kzElement_X509SKI                     = "X509SKI";
const char* const DWFXML::kzElement_X509SubjectName             = "X509SubjectName";
const char* const DWFXML::kzElement_CoordinateSystem            = "CoordinateSystem";
const char* const DWFXML::kzElement_CoordinateSystems           = "CoordinateSystems";
const char* const DWFXML::kzElement_Relationship                = "Relationship";
const char* const DWFXML::kzElement_Relationships               = "Relationships";

const char* const DWFXML::kzAttribute_ID                            = "id";
const char* const DWFXML::kzAttribute_URI                           = "uri";
const char* const DWFXML::kzAttribute_Name                          = "name";
const char* const DWFXML::kzAttribute_Type                          = "type";
const char* const DWFXML::kzAttribute_HRef                          = "href";
const char* const DWFXML::kzAttribute_Refs                          = "refs";
const char* const DWFXML::kzAttribute_MIME                          = "mime";
const char* const DWFXML::kzAttribute_Role                          = "role";
const char* const DWFXML::kzAttribute_Clip                          = "clip";
const char* const DWFXML::kzAttribute_Show                          = "show";
const char* const DWFXML::kzAttribute_Size                          = "size";
const char* const DWFXML::kzAttribute_Node                          = "node";
const char* const DWFXML::kzAttribute_Nodes                         = "nodes";
const char* const DWFXML::kzAttribute_Label                         = "label";
const char* const DWFXML::kzAttribute_Value                         = "value";
const char* const DWFXML::kzAttribute_Units                         = "units";
const char* const DWFXML::kzAttribute_Title                         = "title";
const char* const DWFXML::kzAttribute_Color                         = "color";
const char* const DWFXML::kzAttribute_SetID                         = "setId";
const char* const DWFXML::kzAttribute_Width                         = "width";
const char* const DWFXML::kzAttribute_Height                        = "height";
const char* const DWFXML::kzAttribute_Closed                        = "closed";
const char* const DWFXML::kzAttribute_Hidden                        = "hidden";
const char* const DWFXML::kzAttribute_ZOrder                        = "zOrder";
const char* const DWFXML::kzAttribute_Object                        = "object";
const char* const DWFXML::kzAttribute_Author                        = "author";
const char* const DWFXML::kzAttribute_Visible                       = "visible";
const char* const DWFXML::kzAttribute_Extents                       = "extents";
const char* const DWFXML::kzAttribute_Version                       = "version";
const char* const DWFXML::kzAttribute_Request                       = "request";
const char* const DWFXML::kzAttribute_Scanned                       = "scanned";
const char* const DWFXML::kzAttribute_SchemaID                      = "schemaId";
const char* const DWFXML::kzAttribute_Children                      = "children";
const char* const DWFXML::kzAttribute_Provider                      = "provider";
const char* const DWFXML::kzAttribute_ObjectID                      = "objectId";
const char* const DWFXML::kzAttribute_Category                      = "category";
const char* const DWFXML::kzAttribute_ContentID                     = "contentId";
const char* const DWFXML::kzAttribute_Transform                     = "transform";
const char* const DWFXML::kzAttribute_PlotOrder                     = "plotOrder";
const char* const DWFXML::kzAttribute_Privilege                     = "privilege";
const char* const DWFXML::kzAttribute_ContentIDs                    = "contentIds";
const char* const DWFXML::kzAttribute_InternalID                    = "internalId";
const char* const DWFXML::kzAttribute_ColorDepth                    = "colorDepth";
const char* const DWFXML::kzAttribute_Transparent                   = "transparent";
const char* const DWFXML::kzAttribute_Exclusivity                   = "exclusivity";
const char* const DWFXML::kzAttribute_Description                   = "description";
const char* const DWFXML::kzAttribute_Orientation                   = "orientation";
const char* const DWFXML::kzAttribute_LogfontName                   = "logfontName";
const char* const DWFXML::kzAttribute_ResourceURI                   = "resourceURI";
const char* const DWFXML::kzAttribute_InvertColors                  = "invertColors";
const char* const DWFXML::kzAttribute_CreationTime                  = "creationTime";
const char* const DWFXML::kzAttribute_CanonicalName                 = "canonicalName";
const char* const DWFXML::kzAttribute_CharacterCode                 = "characterCode";
const char* const DWFXML::kzAttribute_DefaultIconURI                = "defaultIconURI";
const char* const DWFXML::kzAttribute_ActiveIconURI                 = "activeIconURI";
const char* const DWFXML::kzAttribute_SchemaLocation                = "schemaLocation";
const char* const DWFXML::kzAttribute_ParentObjectID                = "parentObjectId";
const char* const DWFXML::kzAttribute_OriginalExtents               = "originalExtents";
const char* const DWFXML::kzAttribute_ExpandedIconURI               = "expandedIconURI";
const char* const DWFXML::kzAttribute_ModificationTime              = "modificationTime";
const char* const DWFXML::kzAttribute_PrimaryContentID              = "primaryContentId";
const char* const DWFXML::kzAttribute_ScannedResolution             = "scannedResolution";
const char* const DWFXML::kzAttribute_GeometricVariation            = "geometricVariation";
const char* const DWFXML::kzAttribute_EffectiveResolution           = "effectiveResolution";
const char* const DWFXML::kzAttribute_ClassRefs                     = "classRefs";
const char* const DWFXML::kzAttribute_EntityRef                     = "entityRef";
const char* const DWFXML::kzAttribute_EntityRefs                    = "entityRefs";
const char* const DWFXML::kzAttribute_FeatureRefs                   = "featureRefs";
const char* const DWFXML::kzAttribute_RenderableRef                 = "renderableRef";
const char* const DWFXML::kzAttribute_ContentElementRefs            = "contentElementRefs";
const char* const DWFXML::kzAttribute_PositionX                     = "positionX";
const char* const DWFXML::kzAttribute_PositionY                     = "positionY";
const char* const DWFXML::kzAttribute_PositionZ                     = "positionZ";
const char* const DWFXML::kzAttribute_TargetX                       = "targetX";
const char* const DWFXML::kzAttribute_TargetY                       = "targetY";
const char* const DWFXML::kzAttribute_TargetZ                       = "targetZ";
const char* const DWFXML::kzAttribute_UpVectorX                     = "upVectorX";
const char* const DWFXML::kzAttribute_UpVectorY                     = "upVectorY";
const char* const DWFXML::kzAttribute_UpVectorZ                     = "upVectorZ";
const char* const DWFXML::kzAttribute_FieldWidth                    = "fieldWidth";
const char* const DWFXML::kzAttribute_FieldHeight                   = "fieldHeight";
const char* const DWFXML::kzAttribute_ProjectionType                = "projectionType";
const char* const DWFXML::kzAttribute_ResetFlags                    = "resetFlags";
const char* const DWFXML::kzAttribute_Mask                          = "mask";
const char* const DWFXML::kzAttribute_Red                           = "red";
const char* const DWFXML::kzAttribute_Green                         = "green";
const char* const DWFXML::kzAttribute_Blue                          = "blue";
const char* const DWFXML::kzAttribute_Gloss                         = "gloss";
const char* const DWFXML::kzAttribute_Index                         = "index";
const char* const DWFXML::kzAttribute_Count                         = "count";
const char* const DWFXML::kzAttribute_A                             = "a";
const char* const DWFXML::kzAttribute_B                             = "b";
const char* const DWFXML::kzAttribute_C                             = "c";
const char* const DWFXML::kzAttribute_D                             = "d";
const char* const DWFXML::kzAttribute_Elements                      = "elements";
const char* const DWFXML::kzAttribute_State                         = "state";
const char* const DWFXML::kzAttribute_Mode                          = "mode";
const char* const DWFXML::kzAttribute_PropertyReferenceID           = "propertyReferenceID";
const char* const DWFXML::kzAttribute_SmoothTransition              = "smoothTransition";
const char* const DWFXML::kzAttribute_UseDefaultPropertyIfMissing   = "useDefaultPropertyIfMissing";
const char* const DWFXML::kzAttribute_Algorithm                     = "Algorithm";
const char* const DWFXML::kzAttribute_OriginX                       = "originX";
const char* const DWFXML::kzAttribute_OriginY                       = "originY";
const char* const DWFXML::kzAttribute_OriginZ                       = "originZ";
const char* const DWFXML::kzAttribute_Rotation                      = "rotation";
const char* const DWFXML::kzAttribute_TransitionType                = "TransitionType";
const char* const DWFXML::kzAttribute_Duration                      = "Duration";
const char* const DWFXML::kzAttribute_LeftRightAngle                = "LeftRightAngle";
const char* const DWFXML::kzAttribute_UpDownAngle                   = "UpDownAngle";
const char* const DWFXML::kzAttribute_CameraPosition                = "CameraPosition";
const char* const DWFXML::kzAttribute_CinematicType                 = "CinematicType";
const char* const DWFXML::kzAttribute_InOutDistance                 = "InOutDistance";
const char* const DWFXML::kzAttribute_LeftRightDistance             = "LeftRightDistance";
const char* const DWFXML::kzAttribute_UpDownDistance                = "UpDownDistance";
const char* const DWFXML::kzAttribute_LockCamera                    = "LockCamera";

const char* const DWFXML::kzDWFProperty_SourceProductVendor         = "SourceProductVendor";
const char* const DWFXML::kzDWFProperty_SourceProductName           = "SourceProductName";
const char* const DWFXML::kzDWFProperty_SourceProductVersion        = "SourceProductVersion";
const char* const DWFXML::kzDWFProperty_DWFProductVendor            = "DWFProductVendor";
const char* const DWFXML::kzDWFProperty_DWFProductVersion           = "DWFProductVersion";
const char* const DWFXML::kzDWFProperty_DWFToolkitVersion           = "DWFToolkitVersion";
const char* const DWFXML::kzDWFProperty_DWFFormatVersion            = "DWFFormatVersion";
const char* const DWFXML::kzDWFProperty_PasswordEncryptedDocument   = "_PasswordEncryptedDocument";

const wchar_t* const DWFXML::kzRole_Graphics2d                  = L"2d streaming graphics";
const wchar_t* const DWFXML::kzRole_Graphics3d                  = L"3d streaming graphics";   
const wchar_t* const DWFXML::kzRole_Descriptor                  = L"descriptor";
const wchar_t* const DWFXML::kzRole_Thumbnail                   = L"thumbnail";
const wchar_t* const DWFXML::kzRole_Graphics2dOverlay           = L"2d vector overlay";
const wchar_t* const DWFXML::kzRole_Graphics2dMarkup            = L"2d vector markup";
const wchar_t* const DWFXML::kzRole_RmlMarkup                   = L"RML markup";
const wchar_t* const DWFXML::kzRole_Preview                     = L"preview";
const wchar_t* const DWFXML::kzRole_OverlayPreview              = L"overlay preview";
const wchar_t* const DWFXML::kzRole_MarkupPreview               = L"markup preview";
const wchar_t* const DWFXML::kzRole_RasterOverlay               = L"raster overlay";
const wchar_t* const DWFXML::kzRole_RasterMarkup                = L"raster markup";
const wchar_t* const DWFXML::kzRole_Font                        = L"font";
const wchar_t* const DWFXML::kzRole_Metadata                    = L"metadata";
const wchar_t* const DWFXML::kzRole_PackageSignature            = L"package signature";
const wchar_t* const DWFXML::kzRole_SectionSignature            = L"section signature";
const wchar_t* const DWFXML::kzRole_FilesetSignature            = L"fileset signature";
const wchar_t* const DWFXML::kzRole_Signature                   = L"signature";
const wchar_t* const DWFXML::kzRole_ObjectDefinition            = L"object definition";
const wchar_t* const DWFXML::kzRole_MarkupObjectDefinition      = L"markup object definition";
const wchar_t* const DWFXML::kzRole_Texture                     = L"texture";
const wchar_t* const DWFXML::kzRole_ContentPresentation         = L"content presentation";
const wchar_t* const DWFXML::kzRole_MarkupContentPresentation   = L"markup content presentation";
const wchar_t* const DWFXML::kzRole_Animation                   = L"animation";
const wchar_t* const DWFXML::kzRole_Note                        = L"note";
const wchar_t* const DWFXML::kzRole_Graphics3dMarkup            = L"3D markup graphics";
const wchar_t* const DWFXML::kzRole_3dMarkup                    = L"3D markup";
const wchar_t* const DWFXML::kzRole_ContentDefinition           = L"content definition";
const wchar_t* const DWFXML::kzRole_Icon                        = L"icon";
const wchar_t* const DWFXML::kzRole_ObservationMesh             = L"observation mesh";
const wchar_t* const DWFXML::kzRole_GeographicCoordinateSystem  = L"geographic coordinate system";
const wchar_t* const DWFXML::kzRole_Graphics2dExtension         = L"2d graphics extension";
const wchar_t* const DWFXML::kzRole_Graphics2dDictionary        = L"2d graphics dictionary";
const wchar_t* const DWFXML::kzRole_RasterReference             = L"raster reference";
//DNT_End

#endif

