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


#include "dwf/xps/Constants.h"


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const XPSXML::kzNamespaceURI_XPS = "http://schemas.microsoft.com/xps/2005/06";

const char* const XPSXML::kzElement_FixedDocumentSequence   = "FixedDocumentSequence";
const char* const XPSXML::kzElement_DocumentReference       = "DocumentReference";
const char* const XPSXML::kzElement_FixedDocument           = "FixedDocument";
const char* const XPSXML::kzElement_PageContent             = "PageContent";
const char* const XPSXML::kzElement_PageContentLinkTargets  = "PageContent.LinkTargets";
const char* const XPSXML::kzElement_LinkTarget              = "LinkTarget";
const char* const XPSXML::kzElement_FixedPage               = "FixedPage";
const char* const XPSXML::kzElement_Canvas                  = "Canvas";
const char* const XPSXML::kzElement_ImageBrush              = "ImageBrush";
const char* const XPSXML::kzElement_Path                    = "Path";
const char* const XPSXML::kzElement_Path_Fill               = "Path.Fill";

const char* const XPSXML::kzAttribute_Source                = "Source";
const char* const XPSXML::kzAttribute_Width                 = "Width";
const char* const XPSXML::kzAttribute_Height                = "Height";
const char* const XPSXML::kzAttribute_Name                  = "Name";
const char* const XPSXML::kzAttribute_Data                  = "Data";
const char* const XPSXML::kzAttribute_ImageSource           = "ImageSource";
const char* const XPSXML::kzAttribute_TileMode              = "TileMode";
const char* const XPSXML::kzAttribute_Viewbox               = "Viewbox";
const char* const XPSXML::kzAttribute_ViewboxUnits          = "ViewboxUnits";
const char* const XPSXML::kzAttribute_Viewport              = "Viewport";
const char* const XPSXML::kzAttribute_ViewportUnits         = "ViewportUnits";
const char* const XPSXML::kzAttribute_RenderTransform       = "RenderTransform";
const char* const XPSXML::kzAttribute_Fill                  = "Fill";
const char* const XPSXML::kzAttribute_Language              = "xml:lang";

const char* const XPSXML::kzRelationship_RequiredResource       = "http://schemas.microsoft.com/xps/2005/06/required-resource";
const char* const XPSXML::kzRelationship_FixedRepresentation    = "http://schemas.microsoft.com/xps/2005/06/fixedrepresentation";
const char* const XPSXML::kzRelationship_RestrictedFont         = "http://schemas.microsoft.com/xps/2005/06/restricted-font";
const char* const XPSXML::kzRelationship_DigitalSignatureDefinitions = "http://schemas.microsoft.com/xps/2005/06/signature-definitions";
const char* const XPSXML::kzRelationship_DiscardControl         = "http://schemas.microsoft.com/xps/2005/06/discard-control";
const char* const XPSXML::kzRelationship_DocumentStructure      = "http://schemas.microsoft.com/xps/2005/06/documentstructure";
const char* const XPSXML::kzRelationship_PrintTicket            = "http://schemas.microsoft.com/xps/2005/06/printticket";
const char* const XPSXML::kzRelationship_StoryFragments         = "http://schemas.microsoft.com/xps/2005/06/storyfragments";
//DNT_End

#endif

