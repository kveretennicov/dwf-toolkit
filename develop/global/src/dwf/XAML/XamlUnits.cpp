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

#include "XAML/pch.h"


WT_XAML_Units::~WT_XAML_Units(void)
{
}


#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_XAML_Units::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Units::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );
    WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if(!pW2XSerializer)
    {
		return WT_Result::Internal_Error;
    }

    pW2XSerializer->startElement(XamlXML::kpzUnits_Element);
    
    pW2XSerializer->addAttribute(
        XamlXML::kpzRefName_Attribute,
        rFile.nameIndexString());
   
    pW2XSerializer->addAttribute(
        XamlXML::kpzLabel_Attribute,
        units().ascii());

    wchar_t pBuf[ 1024 ];
    WT_Matrix rMatrix = application_to_dwf_transform();

    if (file.heuristics().apply_transform())
    {
        rMatrix *= file.heuristics().transform();
    }

    if (file.heuristics().transform().rotation() &&
        file.heuristics().apply_transform_to_units_matrix())
    {
        WT_Matrix temp;

        rMatrix.rotate(temp, file.heuristics().transform().rotation());

        rMatrix = temp;
    }

    _DWFCORE_SWPRINTF(pBuf, 1024, L"%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls", 
        (const wchar_t*)DWFString::DoubleToString(rMatrix(0,0),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(0,1),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(0,2),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(0,3),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(1,0),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(1,1),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(1,2),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(1,3),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(2,0),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(2,1),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(2,2),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(2,3),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(3,0),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(3,1),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(3,2),10),
        (const wchar_t*)DWFString::DoubleToString(rMatrix(3,3),10)
        );

    pW2XSerializer->addAttribute(
        XamlXML::kpzTransform_Attribute,
        pBuf);

    pW2XSerializer->endElement();

    return WT_Result::Success;
}
#else
WT_Result WT_Units::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result
WT_XAML_Units::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

    const char** ppXfo = rMap.find(XamlXML::kpzTransform_Attribute);
    if(ppXfo != NULL && *ppXfo != NULL)
    {
        WT_Matrix rMatrix;

        int nProcessed = sscanf( *ppXfo,
            "%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG",
            &rMatrix(0,0),
            &rMatrix(0,1),
            &rMatrix(0,2),
            &rMatrix(0,3),
            &rMatrix(1,0),
            &rMatrix(1,1),
            &rMatrix(1,2),
            &rMatrix(1,3),
            &rMatrix(2,0),
            &rMatrix(2,1),
            &rMatrix(2,2),
            &rMatrix(2,3),
            &rMatrix(3,0),
            &rMatrix(3,1),
            &rMatrix(3,2),
            &rMatrix(3,3));

            if (nProcessed == 16)
            {
                set_application_to_dwf_transform(rMatrix);
            }
    }

    const char** ppLabel = rMap.find(XamlXML::kpzLabel_Attribute);
    if(ppLabel != NULL && *ppLabel != NULL)
    {
        set_units(*ppLabel);
    }

    materialized() = WD_True;
	return WT_Result::Success;
}
