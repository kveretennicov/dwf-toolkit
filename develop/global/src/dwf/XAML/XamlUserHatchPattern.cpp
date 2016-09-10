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

//
// sets the pattern definition # to use while rendering to XAML
//
WT_Result WT_XAML_User_Hatch_Pattern::set_pattern_number(
    unsigned int index)
{
    if(index >= this->pattern_count())
    {
        return WT_Result::Internal_Error;
    }

    _pattern_index = index;

    return WT_Result::Success;
}

//
// from XamlDrawableAttributes::PathAttributeProvider
//
WT_Result WT_XAML_User_Hatch_Pattern::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File& rFile ) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_User_Hatch_Pattern*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_User_Hatch_Pattern*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result WT_XAML_User_Hatch_Pattern::provideFill(
    XamlDrawableAttributes::Fill *& rpFill)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    if ( _pSerializeFile->rendition().fill().fill() == WD_False )
    {
        // Fill mode is off
        return WT_Result::Success;
    }

    //
    // user fill pattern seems to take precedence over the hatch pattern
    //
    if( _pSerializeFile->rendition().user_fill_pattern().pattern_number() != -1)
    {
        return WT_Result::Success;
    }

    //
    // - if the fill is null (what case would it be ?), alloc it
    // - alloc our XAML translator and set it in the fill
    //
    if(!rpFill)
    {
        rpFill = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::Fill);
        if(!rpFill)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    if(pattern_count() > 0)
    {
        rpFill->brush() =
            DWFCORE_ALLOC_OBJECT(XamlBrush::XamlHatchBrush(const_cast<WT_User_Hatch_Pattern::Hatch_Pattern*>( pattern(_pattern_index) ) ) );
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_User_Hatch_Pattern::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_User_Hatch_Pattern::serialize( *rFile.w2dContentFile() );
    }

	WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    //Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement());

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
	pW2XSerializer->startElement(XamlXML::kpzUser_Hatch_Pattern_Element);

    //pattern_number
    if(pattern_number())
        pW2XSerializer->addAttribute(XamlXML::kpzId_Attribute,(int)pattern_number());

    //xsize = width
    if(xsize())
        pW2XSerializer->addAttribute(XamlXML::kpzWidth_Attribute,(int)xsize());

    //ysize = height
    if(ysize())
        pW2XSerializer->addAttribute(XamlXML::kpzHeight_Attribute,(int)ysize());

    //pattern_count
    if(pattern_count())
    {
        pW2XSerializer->addAttribute(XamlXML::kpzCount_Attribute,(int)pattern_count());
        for(int i=0;i<(int)pattern_count();i++)
        {
            Hatch_Pattern* hatch_pattern = (Hatch_Pattern*)pattern(i);
            if(hatch_pattern)
            {
               pW2XSerializer->startElement(XamlXML::kpzHatch_Pattern_Element);

               //x
               if(hatch_pattern->x())
                    pW2XSerializer->addAttribute(XamlXML::kpzX_Attribute,hatch_pattern->x());

               //y
               if(hatch_pattern->y())
                    pW2XSerializer->addAttribute(XamlXML::kpzY_Attribute,hatch_pattern->y());

               //angle
               if(hatch_pattern->angle())
                    pW2XSerializer->addAttribute(XamlXML::kpzAngle_Attribute,(double)hatch_pattern->angle());

               //spacing
               if(hatch_pattern->spacing())
                    pW2XSerializer->addAttribute(XamlXML::kpzSpacing_Attribute,hatch_pattern->spacing());

               if(hatch_pattern->data_size() > 0)
               {
                   //skew
                   if(hatch_pattern->skew())
                        pW2XSerializer->addAttribute(XamlXML::kpzSkew_Attribute,hatch_pattern->skew());

                   //data_size
                   WT_Unsigned_Integer32 size = hatch_pattern->data_size();
                   if(size)
                   {
                       pW2XSerializer->addAttribute(XamlXML::kpzData_Size_Attribute,(int)size);
                       //data
                       DWFString zData;               
                       for(int i=0;i<(int)size;i++)
                       {
                            wchar_t pBuf[128];
                            _DWFCORE_SWPRINTF(pBuf,128,L"%ls ",(const wchar_t*)DWFString::DoubleToString((hatch_pattern->data()[i]),10));
                            zData.append(pBuf);
                       }
                       pW2XSerializer->addAttribute(XamlXML::kpzData_Attribute,zData);
                   }
               }
               pW2XSerializer->endElement();
            }
        }
    }
	pW2XSerializer->endElement();

    return WT_Result::Success;
}
WT_Result
WT_XAML_User_Hatch_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
 	if(!rMap.size())
		return WT_Result::Internal_Error;

	//pattern_number
    const char** ppValue = rMap.find( XamlXML::kpzId_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
        //parent element found
        pattern_number() = (WT_Integer16)atoi(*ppValue);
        //xsize = width
        ppValue = rMap.find( XamlXML::kpzWidth_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            xsize() = (WT_Unsigned_Integer16)atoi(*ppValue);
        }
        else
            xsize() = 0;

        //ysize
        ppValue = rMap.find( XamlXML::kpzHeight_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            ysize() = (WT_Unsigned_Integer16)atoi(*ppValue);
        }
        else
            ysize() = 0;

        //Count
        ppValue = rMap.find( XamlXML::kpzCount_Attribute );
        if (!ppValue)
	        materialized() = WD_True;
    }
    else
    {
        //Child element describing each hatch_pattern found
        //Pattern stuff
        WT_User_Hatch_Pattern::Hatch_Pattern * hatch_pattern = WD_Null;
        // temp holding variables
        double x = 0.0,y = 0.0,angle = 0.0,spacing = 0.0,skew = 0.0;
        WT_Unsigned_Integer32 data_size = 0;
        double* data = WD_Null;
        //x
        ppValue = rMap.find( XamlXML::kpzX_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            x = DWFString::StringToDouble(*ppValue);            
        }
        //y
        ppValue = rMap.find( XamlXML::kpzY_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            y = DWFString::StringToDouble(*ppValue); 
           
        }
        //angle
        ppValue = rMap.find( XamlXML::kpzAngle_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            angle = DWFString::StringToDouble(*ppValue);  
        }
        //spacing
        ppValue = rMap.find( XamlXML::kpzSpacing_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            spacing = DWFString::StringToDouble(*ppValue);            
        }
        //skew
        ppValue = rMap.find( XamlXML::kpzSkew_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
            skew = DWFString::StringToDouble(*ppValue);  
        }
        //data_size
        ppValue = rMap.find( XamlXML::kpzData_Size_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
	    {
           data_size = (WT_Unsigned_Integer32)atoi(*ppValue);
        }
        //data
        if(data_size)
        {
            ppValue = rMap.find( XamlXML::kpzData_Attribute );
            if (ppValue!=NULL && *ppValue!=NULL)
	        {
                //parse tokens to get double values for data array
                data = DWFCORE_ALLOC_MEMORY(double,data_size);
                if(data == NULL)
                    return WT_Result::Out_Of_Memory_Error;

               int iIndex = 0;
               char* pSavePtr;
               char* token = DWFCORE_ASCII_STRING_TOKENIZE( (char *)(*ppValue), /*NOXLATE*/" ", &pSavePtr );
               while( token != NULL )
               {
                   data[iIndex++] = DWFString::StringToDouble(token);
                   /* Get next token: */
                   token = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
               }
            }
            else
            {
                return WT_Result::Corrupt_File_Error;
            }
            
        }

        // construct the hatch pattern 
        // We don't increment/decrement the ref count here
        hatch_pattern = WT_User_Hatch_Pattern::Hatch_Pattern::Construct(
                                                         x,
                                                         y,
                                                         angle,
                                                         spacing,
                                                         skew,
                                                         data_size,
                                                         data);
        // add the new object to the collection
        add_pattern(*hatch_pattern);
        hatch_pattern = WD_Null;
            
        // clean up 
        if (data)
            DWFCORE_FREE_MEMORY(data);

         materialized() = WD_True;
    }
    
	return WT_Result::Success;
}

