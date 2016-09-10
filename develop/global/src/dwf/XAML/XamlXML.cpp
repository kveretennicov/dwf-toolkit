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

void XamlXML::populateAttributeMap (const char** ppAttributeList, tAttributeMap& rMap)
{
    rMap.clear();
    bool bResult;

    for(;;) //infinite until we break
    {
        const char* pName = *ppAttributeList;
        if (pName == NULL)
            break;
        ppAttributeList++;

        const char* pValue = *ppAttributeList;
        if (pValue == NULL)
            break;
        ppAttributeList++;
        
        bResult = rMap.insert( pName, pValue );

        WD_Assert(bResult);
    }
}


const char* const XamlXML::kpzAbsolute_Value                = "Absolute"; //LinearGradientBrush
const char* const XamlXML::kpzAdapt_Patterns_Attribute      = "Adapt_Patterns"; //W2X Line_Style Adapt_Patterns attribute
const char* const XamlXML::kpzAddress_Attribute             = "Address";
const char* const XamlXML::kpzAngle_Attribute               = "Angle";
const char* const XamlXML::kpzArea_Attribute                = "Area";
const char* const XamlXML::kpzBevel_Value                   = "Bevel";
const char* const XamlXML::kpzBidiLevel_Attribute           = "BidiLevel";
const char* const XamlXML::kpzBoldItalicSimulation_Value    = "BoldItalicSimulation";
const char* const XamlXML::kpzBold_Attribute                = "Bold";
const char* const XamlXML::kpzBoldSimulation_Value          = "BoldSimulation";
const char* const XamlXML::kpzBounds_Attribute              = "Bounds"; //Text
const char* const XamlXML::kpzCannonical_Attribute          = "Cannonical";//Font_Extension
const char* const XamlXML::kpzCanvas_Element                = "Canvas";
const char* const XamlXML::kpzCaretStops_Attribute          = "CaretStops";
const char* const XamlXML::kpzCenterX_Attribute             = "CenterX"; //W2X Ellipse Center attribute
const char* const XamlXML::kpzCenterY_Attribute             = "CenterY"; //W2X Ellipse Center attribute
const char* const XamlXML::kpzCharset_Attribute             = "Charset"; //Font
const char* const XamlXML::kpzClip_Attribute                = "Clip";
const char* const XamlXML::kpzColorMap_Flag_Attribute       = "ColorMap_Flag"; //W2X Pen_Pattern color_map flag
const char* const XamlXML::kpzColor_Attribute               = "Color"; //this is for the SolidColorBrush
const char* const XamlXML::kpzColumns_Attribute             = "Columns"; //User_Fill_Pattern columns
const char* const XamlXML::kpzComplete_Attribute            = "Complete"; 
const char* const XamlXML::kpzContours_Attribute            = "Contours";
const char* const XamlXML::kpzCount_Attribute               = "Count"; //W2X_Gouraud_Polytriangle
const char* const XamlXML::kpzDash_End_Cap_Attribute        = "Dash_End_Cap";
const char* const XamlXML::kpzDash_Start_Cap_Attribute      = "Dash_Start_Cap"; //W2X Line_Style Dash_Start_Cap attribute
const char* const XamlXML::kpzData_Attribute                = "Data";
const char* const XamlXML::kpzData_Size_Attribute           = "Data_Size";
const char* const XamlXML::kpzDelineate_Attribute           = "Delineate";
const char* const XamlXML::kpzDescription_Attribute         = "Description"; //Embed
const char* const XamlXML::kpzDeviceFontName_Attribute      = "DeviceFontName";
const char* const XamlXML::kpzDpi_Attribute                 = "Dpi"; //Image's scanned resolution
const char* const XamlXML::kpzEnd_Attribute                 = "End"; //Ellipse
const char* const XamlXML::kpzEndPoint_Attribute            = "EndPoint"; //LinearGradientBrush
const char* const XamlXML::kpzExtent_Attribute              = "Extent";
const char* const XamlXML::kpzFalse_Value                   = "false";
const char* const XamlXML::kpzFamily_Attribute              = "Family"; //Font
const char* const XamlXML::kpzFilename_Attribute            = "Filename"; //Embed
const char* const XamlXML::kpzFill_Attribute                = "Fill";
const char* const XamlXML::kpzFlags_Attribute               = "Flags"; //Font
const char* const XamlXML::kpzFlat_Value                    = "Flat";
const char* const XamlXML::kpzFontRenderingEmSize_Attribute = "FontRenderingEmSize";
const char* const XamlXML::kpzFontUri_Attribute             = "FontUri";
const char* const XamlXML::kpzFormat_Attribute              = "Format";
const char* const XamlXML::kpzFriendly_Name_Attribute       = "Friendly_Name";
const char* const XamlXML::kpzGlyphs_Element                = "Glyphs";
const char* const XamlXML::kpzGradientStop_Element          = "GradientStop"; //LinearGradientBrush
const char* const XamlXML::kpzGradientStops_Element         = "GradientStops"; //LinearGradientBrush
const char* const XamlXML::kpzHeight_Attribute              = "Height"; //Font
const char* const XamlXML::kpzHatch_Pattern_Element         = "Pattern"; //User_Hatch_Pattern->pattern
const char* const XamlXML::kpzId_Attribute                  = "Id"; //W2X Pen_Pattern id
const char* const XamlXML::kpzImageBrush_Element            = "ImageBrush";
const char* const XamlXML::kpzImageSource_Attribute         = "ImageSource";
const char* const XamlXML::kpzIndex_Attribute               = "Index"; //W2X_Color_Attribute,W2XURL_Index,W2X_Macro_Index
const char* const XamlXML::kpzIndices_Attribute             = "Indices";
const char* const XamlXML::kpzIsSideways_Attribute          = "IsSideways";
const char* const XamlXML::kpzItalic_Attribute              = "Italic";
const char* const XamlXML::kpzItalicSimulation_Value        = "ItalicSimulation";
const char* const XamlXML::kpzLabel_Attribute               = "Label";
const char* const XamlXML::kpzLanguage_Attribute            = "lang";
const char* const XamlXML::kpzLinearGradientBrush_Element   = "LinearGradientBrush";
const char* const XamlXML::kpzLine_End_Cap_Attribute        = "Line_End_Cap";
const char* const XamlXML::kpzLine_Join_Attribute           = "Line_Join";
const char* const XamlXML::kpzLine_Start_Cap_Attribute      = "Line_Start_Cap";
const char* const XamlXML::kpzLogfont_Attribute             = "Logfont";//Font_Extension
const char* const XamlXML::kpzMIME_Attribute                = "MIME"; //Embed
const char* const XamlXML::kpzMappingMode_Attribute         = "MappingMode"; //LinearGradientBrush
const char* const XamlXML::kpzMatrixTransform_Element       = "MatrixTransform"; //this is for the RenderTransform
const char* const XamlXML::kpzMatrix_Attribute              = "Matrix"; //this is for the RenderTransform
const char* const XamlXML::kpzMerge_Value                   = "Merge"; //MergeControl
const char* const XamlXML::kpzMiter_Angle_Attribute         = "Miter_Angle"; //W2X Line_Style Miter_Angle attribute
const char* const XamlXML::kpzMiter_Length_Attribute        = "Miter_Length"; //W2X Line_Style Miter_Length attribute
const char* const XamlXML::kpzMiter_Value                   = "Miter";
const char* const XamlXML::kpzMode_Attribute                = "Mode";
const char* const XamlXML::kpzName_Attribute                = "Name";
const char* const XamlXML::kpzNamePrefix_Attribute          = "NamePrefix";
const char* const XamlXML::kpzNavigateUri_Attribute         = "FixedPage.NavigateUri"; //an exception to our naming convention
const char* const XamlXML::kpzNumber_Attribute              = "Number";
const char* const XamlXML::kpzObjectStream_Attribute        = "ObjectStream";
const char* const XamlXML::kpzOblique_Attribute             = "Oblique"; //Font
const char* const XamlXML::kpzOffset_Attribute              = "Offset"; //LinearGradientBrush
const char* const XamlXML::kpzOpacityMask_Attribute         = "OpacityMask";
const char* const XamlXML::kpzOpacity_Attribute             = "Opacity";
const char* const XamlXML::kpzOpaque_Value                  = "Opaque"; //MergeControl
const char* const XamlXML::kpzOriginX_Attribute             = "OriginX";
const char* const XamlXML::kpzOriginY_Attribute             = "OriginY";
const char* const XamlXML::kpzOverscore_Attribute           = "Overscore"; //Text
const char* const XamlXML::kpzPNG_Attribute                 = "PNG";
const char* const XamlXML::kpzPath_Element                  = "Path";
const char* const XamlXML::kpzPitch_Attribute               = "Pitch"; //Font
const char* const XamlXML::kpzPoints_Attribute              = "Points"; //Macro_Draw
const char* const XamlXML::kpzPrivilege_Attribute           = "Privilege";
const char* const XamlXML::kpzRadialGradientBrush_Element   = "RadialGradientBrush";
const char* const XamlXML::kpzRefName_Attribute             = "refName";
const char* const XamlXML::kpzRef_Attribute                 = "Ref";
const char* const XamlXML::kpzRenderTransform_Attribute     = "RenderTransform";
const char* const XamlXML::kpzRender_Attribute              = "Render";
const char* const XamlXML::kpzRenditionSync_Element         = "RenditionSync";
const char* const XamlXML::kpzRequest_Attribute             = "Request";
const char* const XamlXML::kpzResources_Element             = "Resources";
const char* const XamlXML::kpzResourceDictionary_Element    = "ResourceDictionary";
const char* const XamlXML::kpzRows_Attribute                = "Rows"; //User_Fill_Pattern rows
const char* const XamlXML::kpzRotation_Attribute            = "Rotation"; //Font
const char* const XamlXML::kpzRound_Value                   = "Round";
const char* const XamlXML::kpzScale_Attribute               = "Scale"; //Line_Style Pattern_Scale Macro_Scale
const char* const XamlXML::kpzSkew_Attribute                = "Skew"; //User_Hatch_Pattern->skew
const char* const XamlXML::kpzScreening_Percentage_Attribute= "Screening_Percentage"; //W2X Pen_Pattern Screening_percentage for face patterns
const char* const XamlXML::kpzSolidColorBrush_Element       = "SolidColorBrush";
const char* const XamlXML::kpzSource_Attribute              = "Source";
const char* const XamlXML::kpzSpacing_Attribute             = "Spacing"; //Font
const char* const XamlXML::kpzSquare_Value                  = "Square";
const char* const XamlXML::kpzStart_Attribute               = "Start"; //Ellipse
const char* const XamlXML::kpzStartPoint_Attribute          = "StartPoint"; //LinearGradientBrush
const char* const XamlXML::kpzStaticResource_Declaration    = "{StaticResource ";
const char* const XamlXML::kpzStaticResource_DeclarationEnd = "}";
const char* const XamlXML::kpzStrokeDashArray_Attribute     = "StrokeDashArray";
const char* const XamlXML::kpzStrokeDashCap_Attribute       = "StrokeDashCap";
const char* const XamlXML::kpzStrokeDashOffset_Attribute    = "StrokeDashOffset";
const char* const XamlXML::kpzStrokeEndLineCap_Attribute    = "StrokeEndLineCap";
const char* const XamlXML::kpzStrokeLineJoin_Attribute      = "StrokeLineJoin";
const char* const XamlXML::kpzStrokeMiterLimit_Attribute    = "StrokeMiterLimit";
const char* const XamlXML::kpzStrokeStartLineCap_Attribute  = "StrokeStartLineCap";
const char* const XamlXML::kpzStrokeThickness_Attribute     = "StrokeThickness";
const char* const XamlXML::kpzStroke_Attribute              = "Stroke";
const char* const XamlXML::kpzStyle_Attribute               = "Style"; //MergeControl
const char* const XamlXML::kpzStyleSimulations_Attribute    = "StyleSimulations";
const char* const XamlXML::kpzTileMode_Attribute            = "TileMode";
const char* const XamlXML::kpzTile_Value                    = "Tile";
const char* const XamlXML::kpzTransform_Attribute           = "Transform";
const char* const XamlXML::kpzTransparent_Value             = "Transparent"; //MergeControl
const char* const XamlXML::kpzTriangle_Value                = "Triangle";
const char* const XamlXML::kpzTrue_Value                    = "true";
const char* const XamlXML::kpzType_Attribute                = "Type";
const char* const XamlXML::kpzURL_Attribute                 = "URL"; //Embed
const char* const XamlXML::kpzURL_Item_Element              = "Item"; //W2X URL_Item element
const char* const XamlXML::kpzUnderline_Attribute           = "Underline";
const char* const XamlXML::kpzUnderscore_Attribute          = "Underscore"; //Text
const char* const XamlXML::kpzUnicodeString_Attribute       = "UnicodeString";
const char* const XamlXML::kpzValue_Attribute               = "Value"; //TextVAlign, TextHAlign
const char* const XamlXML::kpzVersion_Major_Attribute       = "VersionMajor"; //W2X
const char* const XamlXML::kpzVersion_Minor_Attribute       = "VersionMinor"; //W2X
const char* const XamlXML::kpzViewboxUnits_Attribute        = "ViewboxUnits";
const char* const XamlXML::kpzViewbox_Attribute             = "Viewbox";
const char* const XamlXML::kpzViewportUnits_Attribute       = "ViewportUnits";
const char* const XamlXML::kpzViewport_Attribute            = "Viewport";
const char* const XamlXML::kpzVisibility_Attribute          = "Visible";
const char* const XamlXML::kpzVisualBrush_Element           = "VisualBrush";
const char* const XamlXML::kpzVisual_Element                = "Visual";
const char* const XamlXML::kpzW2X_Element                   = "W2X";
const char* const XamlXML::kpzWeight_Attribute              = "Weight"; //W2X Line_Weight attribute
const char* const XamlXML::kpzWidthScale_Attribute          = "WidthScale"; //Font
const char* const XamlXML::kpzWidth_Attribute               = "Width";
const char* const XamlXML::kpzXaml_Namespace                = "http://schemas.microsoft.com/xps/2005/06";
const char* const XamlXML::kpzX_Namespace                   = "http://schemas.microsoft.com/xps/2005/06/resourcedictionary-key";
const char* const XamlXML::kpzXkey_Attribute                = "x:Key";
const char* const XamlXML::kpzXmlns_Attribute               = "xmlns";
const char* const XamlXML::kpzXmlnsX_Attribute              = "xmlns:x";
const char* const XamlXML::kpzX_Attribute                   = "X";
const char* const XamlXML::kpzY_Attribute                   = "Y";

#define STRINGIZE(a) #a
#define CALLBACK_MACRO(class_name, class_lower) \
	const char* const XamlXML::kpz##class_name##_Element = STRINGIZE(class_name);
CALLBACK_LIST
#undef CALLBACK_MACRO
#undef STRINGISE
