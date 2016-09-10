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


#include "dwf/opc/Constants.h"


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start

const char* const OPCXML::kzSchemaID_CoreProperties = "6B241FB3-E174-4B01-9685-60381F320DA7";

const char* const OPCXML::kzNamespaceURI_ContentTypes  = "http://schemas.openxmlformats.org/package/2006/content-types";
const char* const OPCXML::kzNamespaceURI_CoreProperties  = "http://schemas.openxmlformats.org/package/2006/metadata/core-properties";
const char* const OPCXML::kzNamespaceURI_Relationsips  = "http://schemas.openxmlformats.org/package/2006/relationships";
const char* const OPCXML::kzNamespaceURI_DublinCore = "http://purl.org/dc/elements/1.1/";
const char* const OPCXML::kzNamespaceURI_DublinCoreTerms = "http://purl.org/dc/terms/";
const char* const OPCXML::kzNamespaceURI_XSI = "http://www.w3.org/2001/XMLSchema-instance";
const char* const OPCXML::kzNamespace_DublinCore = "dc:";
const char* const OPCXML::kzNamespace_DublinCoreTerms = "dcterms:";
const char* const OPCXML::kzNamespace_XSI = "xsi:";

const char* const OPCXML::kzElement_Relationships = "Relationships";
const char* const OPCXML::kzElement_Relationship = "Relationship";
const char* const OPCXML::kzElement_Types = "Types";
const char* const OPCXML::kzElement_Default = "Default";
const char* const OPCXML::kzElement_Override = "Override";
const char* const OPCXML::kzElement_CoreProperties = "coreProperties";

const char* const OPCXML::kzAttribute_TargetMode = "TargetMode";
const char* const OPCXML::kzAttribute_Target = "Target";
const char* const OPCXML::kzAttribute_Type = "Type";
const char* const OPCXML::kzAttribute_Id = "Id";
const char* const OPCXML::kzAttribute_ContentType = "ContentType";
const char* const OPCXML::kzAttribute_Extension = "Extension";
const char* const OPCXML::kzAttribute_PartName = "PartName";

const char* const OPCXML::kzAttributeVal_TargetMode_Internal = "Internal";
const char* const OPCXML::kzAttributeVal_TargetMode_External = "External";

const char* const OPCXML::kzRelationship_Thumbnail = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail";
const char* const OPCXML::kzRelationship_CoreProperties = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";

//
// Properties in the OPC Namespace
//
const char* const OPCXML::kzCoreProperty_Keywords = "keywords";
const char* const OPCXML::kzCoreProperty_ContentType = "contentType";
const char* const OPCXML::kzCoreProperty_Category = "category";
const char* const OPCXML::kzCoreProperty_Version = "version";
const char* const OPCXML::kzCoreProperty_Revision = "revision";
const char* const OPCXML::kzCoreProperty_LastModifiedBy = "lastModifiedBy";
const char* const OPCXML::kzCoreProperty_LastPrinted = "lastPrinted";
const char* const OPCXML::kzCoreProperty_ContentStatus = "contentStatus";
//
// Properties in the Dublin Core Namespace
//
const char* const OPCXML::kzCoreProperty_Creator = "creator";
const char* const OPCXML::kzCoreProperty_Identifier = "identifier";
const char* const OPCXML::kzCoreProperty_Title = "title";
const char* const OPCXML::kzCoreProperty_Subject = "subject";
const char* const OPCXML::kzCoreProperty_Description = "description";
const char* const OPCXML::kzCoreProperty_Language = "language";
//
// Properties in the Dublin Core Namespace - under terms
//
const char* const OPCXML::kzCoreProperty_Created = "created";
const char* const OPCXML::kzCoreProperty_Modified = "modified";

//DNT_End

#endif

