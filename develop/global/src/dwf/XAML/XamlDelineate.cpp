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
#include "dwfcore/DWFXMLSerializer.h"

WT_Result WT_XAML_Delineate::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Delineate::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement());
	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

	pW2XSerializer->startElement(XamlXML::kpzDelineate_Element);
    pW2XSerializer->addAttribute( XamlXML::kpzDelineate_Attribute, delineate()==WD_True ? true : false );
	pW2XSerializer->endElement();

    //good to go
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Delineate::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pDelineate = rMap.find(XamlXML::kpzDelineate_Attribute);
	if(pDelineate != NULL && *pDelineate != NULL)
	{
		delineate() = atoi(*pDelineate)==1 ? WD_True : WD_False;
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;
	return WT_Result::Success;

}

//We may need this code if it's decided that WT_Delineate indicates edged polylines
//WT_Result WT_XAML_Delineate::provideFill(XamlDrawableAttributes::Fill*& rpFill)
//{
//    if (_pSerializeFile == NULL)
//    {
//        return WT_Result::Internal_Error;
//    }
//
//    if ( delineate() == WD_False )
//    {
//        // Delineate mode is off
//        return WT_Result::Success;
//    }
//
//    if (rpFill == NULL)
//    {
//        rpFill = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Fill() );
//        if (rpFill == NULL)
//        {
//            return WT_Result::Out_Of_Memory_Error;
//        }
//    }
//
//    XamlBrush::SolidColor* pSolidBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::SolidColor() );
//	pSolidBrush->set( _pSerializeFile->rendition().color().rgba() );
//
//	rpFill->brush() = pSolidBrush;
//    return WT_Result::Success;
//}
//
//WT_Result WT_XAML_Delineate::provideStroke(XamlDrawableAttributes::Stroke*& rpStroke)
//{
//    if (_pSerializeFile == NULL)
//    {
//        return WT_Result::Internal_Error;
//    }
//
//    if ( delineate() == WD_False )
//    {
//        // Delineate mode is off
//        return WT_Result::Success;
//    }
//
//    if (rpStroke == NULL)
//    {
//        rpStroke = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Stroke() );
//        if (rpStroke == NULL)
//        {
//            return WT_Result::Out_Of_Memory_Error;
//        }
//    }
//
//    XamlBrush::SolidColor* pSolidBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::SolidColor() );
//	pSolidBrush->set( _pSerializeFile->rendition().contrast_color().rgba() );
//
//	rpStroke->brush() = pSolidBrush;
//    return WT_Result::Success;
//}
//
//
//WT_Result WT_XAML_Delineate::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File& rFile ) const
//{
//    const_cast<WT_XAML_Delineate*>(this)->_pSerializeFile = &rFile;
//    return pConsumer->consumeFill( const_cast<WT_XAML_Delineate*>(this) );
//}
