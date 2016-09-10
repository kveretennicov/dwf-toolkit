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

using namespace XamlBrush;

size_t 
Brush::PrintColor( wchar_t* buf, size_t bufSize, const WT_RGBA32& rColor )
{
    if (rColor.m_rgb.a != 0xFF)
    {
        return _DWFCORE_SWPRINTF(buf, bufSize, L"#%02X%02X%02X%02X", rColor.m_rgb.a, rColor.m_rgb.r, rColor.m_rgb.g, rColor.m_rgb.b );
    }
    else
    {
        return _DWFCORE_SWPRINTF(buf, bufSize, L"#%02X%02X%02X", rColor.m_rgb.r, rColor.m_rgb.g, rColor.m_rgb.b );
    }
}

WT_Result 
Brush::ReadColor( WT_RGBA32& rColor, const char* zBuf )
{
    size_t nLen = strlen(zBuf); //unsafe, but what choice do we have, we don't get lengths back from the XML parser

    if (nLen == 9)
    {
        int nProcessed = sscanf(&zBuf[1], "%08X", (unsigned int *)&rColor.m_whole );
        if (nProcessed == 1)
            return WT_Result::Success;
    }
    else if (nLen == 7)
    {
        int nProcessed = sscanf(&zBuf[1], "%06X",(unsigned int *) &rColor.m_whole );

        //
        // by default, the alpha is 255 : set it here otherwise lines will show up
        // as empty rectangles in the viewer
        //
        rColor.m_rgb.a = 255;

        if (nProcessed == 1)
            return WT_Result::Success;
    }

    return WT_Result::Internal_Error;
}


///////////////////////////////////////////////////////////////////
//SolidColorBrush
SolidColor::SolidColor()
: _oColor(0,0,0) 
{
}

SolidColor::SolidColor( const WT_RGBA32 &r )
{
    set( r );
}

bool SolidColor::operator==( const Brush& r) const
{
    if (r.type() != type())
    {
        return false;
    }

    return !!(static_cast<const SolidColor&>(r).color() == color());
}

SolidColor::~SolidColor() 
throw()
{ 
    _notifyDelete(); 
}

WT_Result
SolidColor::set( const WT_RGBA32 &r )
{
    _oColor = r;
    return WT_Result::Success;
}

const WT_RGBA32& 
SolidColor::color() const
{ 
    return _oColor; 
} 

const DWFString& 
SolidColor::colorString() const
{
    if (_szColorString.chars()==0)
    {
        wchar_t buf[16];
        PrintColor( buf, 16, _oColor );
        const_cast<SolidColor*>(this)->_szColorString = buf;
    }
    return _szColorString;
}

WT_Result 
SolidColor::materializeAttribute( WT_XAML_File &, const char *pAttribute )
{
    WT_RGBA32 oColor;
    WD_CHECK( ReadColor( oColor, pAttribute ) );
    return set( oColor );
}

WT_Result
SolidColor::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    pSerializer->startElement( XamlXML::kpzSolidColorBrush_Element );
    pSerializer->addAttribute( XamlXML::kpzColor_Attribute, _szColorString );
    pSerializer->endElement();

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////
//ImageBrush
Image::Image()
{
}

Image::~Image()
throw()
{ 
    _notifyDelete(); 
}

bool 
Image::operator==( const Brush& r) const
{
    WD_Assert ( false ) ; //implemented in a subclass
    return r.type() == type();
}

WT_Result
Image::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    WD_Assert ( false ) ; //implemented in a subclass
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//RadialGradientBrush
RadialGradient::RadialGradient()
{
}

RadialGradient::~RadialGradient()
throw()
{ 
    _notifyDelete(); 
}

bool 
RadialGradient::operator==( const Brush& r) const
{
    //FUTURE: implement
    return r.type() == type();
}

WT_Result 
RadialGradient::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    //FUTURE: implement verbose serialization
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//LinearGradientBrush
LinearGradient::LinearGradient()
: _oStartPt(0.,0.)
, _oEndPt(0.,0.)
, _oStartColor(0,0,0)
, _oEndColor(0,0,0)
{
}

LinearGradient::~LinearGradient()
throw()
{
    _notifyDelete(); 
}

bool 
LinearGradient::operator==( const Brush& r) const
{
    if (r.type() != type())
        return false;

    const LinearGradient& rBrush = static_cast<const LinearGradient&>(r);

    return  (_oStartColor == rBrush._oStartColor) &&
            (_oStartPt == rBrush._oStartPt) &&
            (_oEndColor == rBrush._oEndColor) &&
            (_oEndPt == rBrush._oEndPt);
}

WT_Result 
LinearGradient::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    wchar_t buf[64];

    //<LinearGradientBrush MappingMode="Absolute" StartPoint="0,0" EndPoint="300,300">
    pSerializer->startElement( XamlXML::kpzLinearGradientBrush_Element );
    pSerializer->addAttribute( XamlXML::kpzMappingMode_Attribute, XamlXML::kpzAbsolute_Value );
    
    //TODO: Do these need to be translated at all?  They're absolute, but do we have some sort of normalizer?
    pSerializer->addAttribute( XamlXML::kpzStartPoint_Attribute,const_cast<WT_Point2D &>(_oStartPt).toString(10));
    pSerializer->addAttribute( XamlXML::kpzEndPoint_Attribute, const_cast<WT_Point2D &>(_oEndPt).toString(10));

    //    <LinearGradientBrush.GradientStops>
    char szName[64];
    sprintf(szName, "%s.%s", XamlXML::kpzLinearGradientBrush_Element, XamlXML::kpzGradientStops_Element);
    pSerializer->startElement( szName );

    //        <GradientStop Color="#FFFF00" Offset="0" />
    pSerializer->startElement( XamlXML::kpzGradientStop_Element );
    PrintColor( buf, 64, _oStartColor );
    pSerializer->addAttribute( XamlXML::kpzColor_Attribute, buf );
    pSerializer->addAttribute( XamlXML::kpzOffset_Attribute, L"0" );
    pSerializer->endElement();

    //        <GradientStop Color="#0000FF" Offset="1" />
    pSerializer->startElement( XamlXML::kpzGradientStop_Element );
    PrintColor( buf, 64, _oEndColor );
    pSerializer->addAttribute( XamlXML::kpzColor_Attribute, buf );
    pSerializer->addAttribute( XamlXML::kpzOffset_Attribute, L"1" );
    pSerializer->endElement();

    //    </LinearGradientBrush.GradientStops>
    pSerializer->endElement();
    //</LinearGradientBrush> 
    pSerializer->endElement();

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////
//VisualBrush
Visual::Visual()
{
}

Visual::~Visual()
throw()
{
    _notifyDelete(); 
}

bool 
Visual::operator==(const Brush& other) const
{
    //
    //
    //
    if(other.type() == kVisual) {

        Visual *pCasted = (Visual *) &other;
        XamlResource *pRes = (XamlResource *)pCasted;
        return  *pRes == *this;
    }

    return false;
}

WT_Result
Visual::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    WD_Assert ( false ) ; //implemented in a subclass
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//XamlHatchBrush

//
// TEMPORARY - waiting for proper layouting capabilities
//
#define USER_HATCH_ADJUST_X (100.)
#define USER_HATCH_ADJUST_Y (100.)
#define USER_HATCH_ADJUST_WEIGHT (1.)

//
// ctor
//
XamlHatchBrush::XamlHatchBrush(
    WT_User_Hatch_Pattern::Hatch_Pattern *pPat)   // data from w2d
    : _pPat(pPat)
{
    WD_Assert(pPat);
    _pPat->increment();
}

//
// dtor
//
XamlHatchBrush::~XamlHatchBrush()
throw()
{
    _pPat->decrement();
    if (_pPat->count() == 0)
    {
        _pPat->destroy();
    }
}

//
// from XamlResource 
//
bool XamlHatchBrush::operator==(
    const XamlResource &other) const        // comparee
{
    if(other.resourceType() == XamlResource::eUserHatchBrushResource)
    {
       const XamlHatchBrush *pCasted = (const XamlHatchBrush *) &other;

       return (*_pPat == *pCasted->_pPat) != false;
    }
    return false;
}

//
// from XamlResource 
//
XamlResource *XamlHatchBrush::copy() const
{
    return DWFCORE_ALLOC_OBJECT( XamlHatchBrush(_pPat) );
}

//
// from XamlResource 
//
XamlResource::eResourceType XamlHatchBrush::resourceType() const
{
    return XamlResource::eUserHatchBrushResource;
}

//
// from XamlResource 
//
WT_Result XamlHatchBrush::serializeResource(
    const wchar_t*pKey,                             // key, != 0
    WT_XAML_File &rFile,                            // current file
    DWFXMLSerializer &rXml) const                   // rXml serializer
{
    DWFString s, strokes;
    wchar_t pBuf[ 128 ];

    //
    // the number of gaps and dashes should be even
    //
    if(!pKey || _pPat->data_size() % 2)
    {
        return WT_Result::Internal_Error;
    }

    //
    // start with a <VisualBrush> element keyed by rKey
    //
    rXml.startElement(XamlXML::kpzVisualBrush_Element);
    rXml.addAttribute(XamlXML::kpzXkey_Attribute, pKey);

	double total, height;
    int dash, gap;
    unsigned int weight = rFile.rendition().line_weight().weight_value();
    unsigned int i;
    WT_RGBA32 col = rFile.rendition().color().rgba();

    //
    // given the user data, craft an XAML stroke array string
    //
	for(
        i = 0, total = 0, height = USER_HATCH_ADJUST_Y * _pPat->spacing();
        i < _pPat->data_size() / 2;
        i++, total += dash + gap) {
	
		dash = (int)( _pPat->data()[ 2 * i ] * USER_HATCH_ADJUST_X );
		gap = (int)( _pPat->data()[ 2 * i + 1 ] * USER_HATCH_ADJUST_X );

		if(!dash)
        {
            dash = 1;
        }
		if(!gap)
        {
            gap = 1;
        }

		_DWFCORE_SWPRINTF(pBuf, 128, L"%d %d ", dash, gap);
		strokes.append(pBuf);
	}

    //
    // clip height, total and weight
    //
    if(height < 0)
    {
        height = -height;
    }
	if(!total)
    {
        total = 1.;
    }
    if(!weight)
    {
        weight = 1;
    }

    //
    // the unit tile is the rectangle [0,0]->[total, height]
    //
	_DWFCORE_SWPRINTF(pBuf, 128, L"0, 0, %ls, %ls", (const wchar_t*)DWFString::DoubleToString(total,3,3), (const wchar_t*)DWFString::DoubleToString(height,3,3));


    rXml.addAttribute(XamlXML::kpzViewbox_Attribute, pBuf);
	rXml.addAttribute(XamlXML::kpzViewport_Attribute, pBuf);

    //
    // let's rotate it to match the angle parameter from whip
    //
	_DWFCORE_SWPRINTF(pBuf, 128, L"%ls,%ls,%ls,%ls,0,0",
		(const wchar_t*)DWFString::DoubleToString(cos(_pPat->angle()),3,3), 
		(const wchar_t*)DWFString::DoubleToString(sin(_pPat->angle()),3,3),
		(const wchar_t*)DWFString::DoubleToString(-sin(_pPat->angle()),3,3),
		(const wchar_t*)DWFString::DoubleToString(cos(_pPat->angle()),3,3)
        );

	rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);

    //
    // all of these are defaulted
    // we need to use a canvas for the visual otherwise it does not
    // seem to render properly under IE
    //
	rXml.addAttribute(XamlXML::kpzTileMode_Attribute, XamlXML::kpzTile_Value);
	rXml.addAttribute(XamlXML::kpzViewboxUnits_Attribute, XamlXML::kpzAbsolute_Value);
	rXml.addAttribute(XamlXML::kpzViewportUnits_Attribute, XamlXML::kpzAbsolute_Value);

    s = XamlXML::kpzVisualBrush_Element;
    s.append(".");
    s.append(XamlXML::kpzVisual_Element);

    rXml.startElement(s);
	rXml.startElement(XamlXML::kpzCanvas_Element);
	rXml.startElement(XamlXML::kpzPath_Element);

    //
    // specify the hatch color
    //
    _DWFCORE_SWPRINTF(pBuf, 63,
        L"#%2.2x%2.2x%2.2x%2.2x", 
        col.m_rgb.a,
        col.m_rgb.r,
        col.m_rgb.g,
        col.m_rgb.b);

    rXml.addAttribute(XamlXML::kpzStroke_Attribute, pBuf);

    //
    // specify the hatch thickness
    //
    _DWFCORE_SWPRINTF(pBuf, 63, L"%ls", (const wchar_t*)DWFString::DoubleToString((weight * USER_HATCH_ADJUST_WEIGHT),3,3));
    rXml.addAttribute(XamlXML::kpzStrokeThickness_Attribute, pBuf);

    //
    // if we have some stroke data, specify it now and render
    // one line segment [0, total] in the unit tile
    //
	if(_pPat->data_size())
    {
		rXml.addAttribute(XamlXML::kpzStrokeDashArray_Attribute, strokes);
	}
    _DWFCORE_SWPRINTF(pBuf, 128, L"M 0,0 L %ls,0", (const wchar_t*)DWFString::DoubleToString(total,3,3));
    rXml.addAttribute(XamlXML::kpzData_Attribute, pBuf);

    rXml.endElement();
    rXml.endElement();
    rXml.endElement();
    rXml.endElement();

    return WT_Result::Success;
}

//
// from XamlBrush::Visual - not implemented and returning an error
//
WT_Result XamlHatchBrush::serializeElement(
    WT_XAML_File &,                                 // current file
    DWFCore::DWFXMLSerializer *) const              // XML utility, !=0
{
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//XamlFixedPatternBrush

//
// ctor
//
XamlFixedPatternBrush::XamlFixedPatternBrush(
    WT_Fill_Pattern::WT_Pattern_ID id,              // what pattern ?1
    double scale)                                   // pattern scale
    : _eId(id), _fScale(scale)
{
    //
    // Solid is not supported by XamlFixedPatternBrush
    //
    WD_Assert(_eId != WT_Fill_Pattern::Solid);
}

//
// dtor
//
XamlFixedPatternBrush::~XamlFixedPatternBrush()
throw()
{
}

//
// from XamlResource 
//
bool XamlFixedPatternBrush::operator==(
    const XamlResource &other) const        // comparee
{
    if(other.resourceType() == XamlResource::eFixedPatternBrushResource)
    {
       const XamlFixedPatternBrush *pCasted = (const XamlFixedPatternBrush *) &other;

       return (pCasted->_eId == _eId) && (pCasted->_fScale == _fScale);
    }
    return false;
}

//
// from XamlResource 
//
XamlResource *XamlFixedPatternBrush::copy() const
{
    return DWFCORE_ALLOC_OBJECT( XamlFixedPatternBrush(_eId, _fScale) );
}

//
// from XamlResource 
//
XamlResource::eResourceType XamlFixedPatternBrush::resourceType() const
{
    return XamlResource::eFixedPatternBrushResource;
}

//
// from XamlResource 
//
WT_Result XamlFixedPatternBrush::serializeResource(
    const wchar_t*pKey,                             // key, != 0
    WT_XAML_File &rFile,                            // current file
    DWFXMLSerializer &rXml) const                   // rXml serializer
{
	wchar_t pBuf[ 128 ];

    int adapted_scale = _fScale > 0 ? (int) (9 * _fScale) : 9;

    //
    // start with a <VisualBrush> element keyed by rKey
    //
    rXml.startElement(XamlXML::kpzVisualBrush_Element);
    rXml.addAttribute(XamlXML::kpzXkey_Attribute, pKey);

    WT_RGBA32 col = rFile.rendition().color().rgba();

    //
    // fake each pattern by a either a rotated line or a filled
    // square
    //
	switch(_eId) {
		case WT_Fill_Pattern::Crosshatch :
		case WT_Fill_Pattern::Horizontal_Bars :
		case WT_Fill_Pattern::Vertical_Bars :
		case WT_Fill_Pattern::Slant_Left :
		case WT_Fill_Pattern::Slant_Right :  
			{

				double angle = 0;

                adapted_scale *= 10;

				switch(_eId) {
					case WT_Fill_Pattern::Crosshatch :
					case WT_Fill_Pattern::Slant_Left :
						{
							angle = -TWO_PI / 8;
						}
						break;
					case WT_Fill_Pattern::Horizontal_Bars :
						{
							angle = TWO_PI / 4;
						}
						break;
					case WT_Fill_Pattern::Slant_Right :  
						{
							angle = TWO_PI / 8;
						}
						break;
					default:
						break;
				}

				_DWFCORE_SWPRINTF(pBuf, 128, L"%ls,%ls,%ls,%ls,0,0",
					(const wchar_t*)DWFString::DoubleToString(cos(angle),3,3), 
					(const wchar_t*)DWFString::DoubleToString(sin(angle),3,3),
					(const wchar_t*)DWFString::DoubleToString(-sin(angle),3,3),
					(const wchar_t*)DWFString::DoubleToString(cos(angle),3,3));

				rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);
				
				_DWFCORE_SWPRINTF(pBuf, 128, L"0,0,%d,%d", adapted_scale, adapted_scale);

				rXml.addAttribute(XamlXML::kpzViewbox_Attribute, pBuf);
				rXml.addAttribute(XamlXML::kpzViewport_Attribute, pBuf);
			}
			break;
		case WT_Fill_Pattern::Diamonds :
			{
				rXml.addAttribute(XamlXML::kpzViewbox_Attribute, "0,0,1,1");
				rXml.addAttribute(XamlXML::kpzViewport_Attribute, "0,0,8,8");

                _DWFCORE_SWPRINTF(pBuf, 128, L"%ls,%ls,%ls,%ls,0,0",
                    (const wchar_t*)DWFString::DoubleToString(.8 * adapted_scale,3,3),
                    (const wchar_t*)DWFString::DoubleToString(.707 * adapted_scale,3,3),
                    (const wchar_t*)DWFString::DoubleToString(-.8 * adapted_scale,3,3),
                    (const wchar_t*)DWFString::DoubleToString(.707 * adapted_scale,3,3));

                rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);
			}
			break;
		case WT_Fill_Pattern::Square_Dots :
			{
				rXml.addAttribute(XamlXML::kpzViewbox_Attribute, "0,0,1,1");
                rXml.addAttribute(XamlXML::kpzViewport_Attribute, "0,0,5,5");				

                _DWFCORE_SWPRINTF(pBuf, 128, L"%d,0,0,%d,0,0", adapted_scale, adapted_scale);
                rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);
			}
			break;
		default :
			{
				rXml.addAttribute(XamlXML::kpzViewbox_Attribute, "0,0,1,1");
				rXml.addAttribute(XamlXML::kpzViewport_Attribute, "0,0,8,8");				
				
                _DWFCORE_SWPRINTF(pBuf, 128, L"%d,0,0,%d,0,0", adapted_scale, adapted_scale);
                rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);
			}
			break;
	}
	
    //
    // all of these are defaulted
    //
	rXml.addAttribute(XamlXML::kpzTileMode_Attribute, XamlXML::kpzTile_Value);
	rXml.addAttribute(XamlXML::kpzViewboxUnits_Attribute, XamlXML::kpzAbsolute_Value);
	rXml.addAttribute(XamlXML::kpzViewportUnits_Attribute, XamlXML::kpzAbsolute_Value);

    DWFString s;

    s = XamlXML::kpzVisualBrush_Element;
    s.append(".");
    s.append(XamlXML::kpzVisual_Element);

    rXml.startElement(s);

	switch(_eId) {

		case WT_Fill_Pattern::Checkerboard :
		case WT_Fill_Pattern::Diamonds :
			{
                rXml.startElement(XamlXML::kpzPath_Element);

                _DWFCORE_SWPRINTF(pBuf, 63,
                    L"#%2.2x%2.2x%2.2x%2.2x", 
                    col.m_rgb.a,
                    col.m_rgb.r,
                    col.m_rgb.g,
                    col.m_rgb.b);

                rXml.addAttribute(XamlXML::kpzFill_Attribute, pBuf);
                rXml.addAttribute(XamlXML::kpzData_Attribute,
                    L"M0,0L.5,0 .5,.5 0,.5ZM.5,.5L1,.5 1,1 .5,1Z");

                rXml.endElement();
			}
			break;
		case WT_Fill_Pattern::Crosshatch :
			{
                rXml.startElement(XamlXML::kpzPath_Element);
                
                _DWFCORE_SWPRINTF(pBuf, 63,
                    L"#%2.2x%2.2x%2.2x%2.2x", 
                    col.m_rgb.a,
                    col.m_rgb.r,
                    col.m_rgb.g,
                    col.m_rgb.b);

                rXml.addAttribute(XamlXML::kpzStroke_Attribute, pBuf);
                rXml.addAttribute(XamlXML::kpzStrokeThickness_Attribute, L"2");

				_DWFCORE_SWPRINTF(pBuf, 128, L"M0,0L0,%dZM0,0L%d,0Z", 
                    adapted_scale,
                    adapted_scale
                    );
				rXml.addAttribute(XamlXML::kpzData_Attribute, pBuf);

                rXml.endElement();
			}
			break;
		case WT_Fill_Pattern::Horizontal_Bars :
		case WT_Fill_Pattern::Vertical_Bars :
		case WT_Fill_Pattern::Slant_Left :
		case WT_Fill_Pattern::Slant_Right :  
			{
                //
                // note : if the canvas is not there, the rotations is
                // incorrect - looks like an XPS renderer bug..
                //
                rXml.startElement(XamlXML::kpzCanvas_Element);
                rXml.startElement(XamlXML::kpzPath_Element);
                
                _DWFCORE_SWPRINTF(pBuf, 63,
                    L"#%2.2x%2.2x%2.2x%2.2x", 
                    col.m_rgb.a,
                    col.m_rgb.r,
                    col.m_rgb.g,
                    col.m_rgb.b);

                rXml.addAttribute(XamlXML::kpzStroke_Attribute, pBuf);
                rXml.addAttribute(XamlXML::kpzStrokeThickness_Attribute, L"1");

				_DWFCORE_SWPRINTF(pBuf, 128, L"M0,0L0,%dZ", adapted_scale);

				rXml.addAttribute(XamlXML::kpzData_Attribute, pBuf);

                rXml.endElement();
                rXml.endElement();
			}
			break;
		case WT_Fill_Pattern::Square_Dots :
			{
                rXml.startElement(XamlXML::kpzPath_Element);

                _DWFCORE_SWPRINTF(pBuf, 63,
                    L"#%2.2x%2.2x%2.2x%2.2x", 
                    col.m_rgb.a,
                    col.m_rgb.r,
                    col.m_rgb.g,
                    col.m_rgb.b);

                rXml.addAttribute(XamlXML::kpzFill_Attribute, pBuf);
                rXml.addAttribute(XamlXML::kpzData_Attribute,
                    L"M.1,.1L.9,.1 .9,.9 .1,.9Z");

                rXml.endElement();
			}
			break;
		case WT_Fill_Pattern::User_Defined :
			{
				//
                // according to the whip doc, this one is reserved and
                // not implemented (?) - stream out an empty canvas
                //
                rXml.startElement(XamlXML::kpzPath_Element);
                rXml.endElement();
			}
			break;
		default:
			break;
		}

	rXml.endElement();
	rXml.endElement();

    return WT_Result::Success;
}

//
// from XamlBrush::Visual - not implemented and returning an error
//
WT_Result XamlFixedPatternBrush::serializeElement(
    WT_XAML_File &,                                 // current file
    DWFCore::DWFXMLSerializer *) const              // XML utility, !=0
{
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//XamlUserPatternBrush

//
// ctor
//
XamlUserPatternBrush::XamlUserPatternBrush(
    WT_User_Fill_Pattern::Fill_Pattern *pPat, // what pattern ?
    double scale)                                   // pattern scale
    : _pPat(pPat), _fScale(scale)
{
    WD_Assert(_pPat);
    _pPat->increment();
}

//
// dtor
//
XamlUserPatternBrush::~XamlUserPatternBrush()
throw()
{
    _pPat->decrement();
    if (_pPat->count() == 0)
    {
        _pPat->destroy();
    }
}

//
// from XamlResource 
//
bool XamlUserPatternBrush::operator==(
    const XamlResource &other) const        // comparee
{
    if(other.resourceType() == XamlResource::eUserPatternBrushResource)
    {
       const XamlUserPatternBrush *pCasted = (const XamlUserPatternBrush *) &other;

       return (*_pPat == *pCasted->_pPat) != false;
    }
    return false;
}

//
// from XamlResource 
//
XamlResource *XamlUserPatternBrush::copy() const
{
    return DWFCORE_ALLOC_OBJECT( XamlUserPatternBrush(_pPat, _fScale) );
}

//
// from XamlResource 
//
XamlResource::eResourceType XamlUserPatternBrush::resourceType() const
{
    return XamlResource::eUserPatternBrushResource;
}

//
// from XamlResource 
//
WT_Result XamlUserPatternBrush::serializeResource(
    const wchar_t*pKey,                             // key, != 0
    WT_XAML_File &rFile,                            // current file
    DWFXMLSerializer& rXml) const                   // rXml serializer
{
    wchar_t pBuf[ 128 ];

//
// TODO : experiment with the scale and adjust it to come up with a similar
// rendering
//
#if 0
    int adapted_scale = _fScale > 0 ? (int) (9 * _fScale) : 9;
#endif

    //
    // start with a <VisualBrush> element keyed by rKey
    //
    rXml.startElement(XamlXML::kpzVisualBrush_Element);
    rXml.addAttribute(XamlXML::kpzXkey_Attribute, pKey);

    WT_RGBA32 col = rFile.rendition().color().rgba();

	_DWFCORE_SWPRINTF(pBuf, 128, L"0, 0, %d, %d",
        _pPat->columns() - 1,
        _pPat->rows() - 1
        );

	rXml.addAttribute(XamlXML::kpzViewbox_Attribute, pBuf);
	rXml.addAttribute(XamlXML::kpzViewport_Attribute, pBuf);

    //
    // all of these are defaulted
    //
	rXml.addAttribute(XamlXML::kpzTransform_Attribute, L"1,0,0,1,0,0");
    rXml.addAttribute(XamlXML::kpzTileMode_Attribute, XamlXML::kpzTile_Value);
    rXml.addAttribute(XamlXML::kpzViewboxUnits_Attribute, XamlXML::kpzAbsolute_Value);
	rXml.addAttribute(XamlXML::kpzViewportUnits_Attribute, XamlXML::kpzAbsolute_Value);

    DWFString s;

    s = XamlXML::kpzVisualBrush_Element;
    s.append(".");
    s.append(XamlXML::kpzVisual_Element);

    rXml.startElement(s);
    rXml.startElement(XamlXML::kpzCanvas_Element);

	int x, y, length;
	char bit_no, is_on, no_line, current;
	const WT_Byte *pRaw = _pPat->data();

    //
    // scanline the model and output line segments
    //
	for(y = 0; y < _pPat->rows(); y++) {

        s = "";

		bit_no = 0;
		no_line = current = 1;
		length = 0;
		for(x = 0; x < _pPat->columns(); x++)
        {

			is_on = (*pRaw & (1 << (7 - bit_no))) != 0;

			if(is_on ^ current)
            {

				_DWFCORE_SWPRINTF(pBuf, 128, L" %d", length);
				s.append(pBuf);

				current = is_on;
				length = 1;

			}
            else 
            {
                length++;
            }

			if(is_on)
            {
                no_line = 0;
            }

			bit_no++;
			
            if(bit_no == 8) 
            {
				pRaw++, bit_no = 0;
			}
		}

		_DWFCORE_SWPRINTF(pBuf, 128, L" %d", length);
		s.append(pBuf);

		if(current)
        {
			s.append(" 0");
		}

		if(no_line)
        {
            continue;
        }

        rXml.startElement(XamlXML::kpzPath_Element);

       _DWFCORE_SWPRINTF(pBuf, 128,
            L"#%2.2x%2.2x%2.2x%2.2x", 
            col.m_rgb.a,
            col.m_rgb.r,
            col.m_rgb.g,
            col.m_rgb.b);

        rXml.addAttribute(XamlXML::kpzStroke_Attribute, pBuf);
        //
        // TOSEE : hook to line weight ? - check out when testing
        //
        rXml.addAttribute(XamlXML::kpzStrokeThickness_Attribute, L"1");
        rXml.addAttribute(XamlXML::kpzStrokeDashArray_Attribute, s);

        s = "";

		_DWFCORE_SWPRINTF(pBuf, 128, L"M0,%dL%d,%d", y, _pPat->columns(), y);
		s.append(pBuf);

        rXml.addAttribute(XamlXML::kpzData_Attribute, s);
        rXml.endElement();
	}

    rXml.endElement();
	rXml.endElement();
	rXml.endElement();

    return WT_Result::Success;
}

//
// from XamlBrush::Visual - not implemented and returning an error
//
WT_Result XamlUserPatternBrush::serializeElement(
    WT_XAML_File &,                                 // current file
    DWFCore::DWFXMLSerializer *) const              // XML utility, !=0
{
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////
//XamlImageBrush

//
// ctor
//
XamlImageBrush::XamlImageBrush(
            const DWFString &name,                          // name
            const WT_Matrix2D &rXfo,                        // image xfo
            WT_Unsigned_Integer16 width,                    // width > 0
            WT_Unsigned_Integer16 height)                   // height > 0
: _zName(name), _oXfo(rXfo), _nDpi(-1),_nWidth(width), _nHeight(height)
{
    WD_Assert(width > 0 && height > 0);
}

XamlImageBrush::XamlImageBrush(
            const DWFString &name,                          // name
            const WT_Matrix2D &rXfo,                        // image xfo
            WT_Integer32 dpi,                               // Scanned Resoulution dpi                          
            WT_Unsigned_Integer16 width,                    // width > 0
            WT_Unsigned_Integer16 height)                   // height > 0
: _zName(name), _oXfo(rXfo), _nDpi(dpi),_nWidth(width), _nHeight(height)
{
    WD_Assert(width > 0 && height > 0);
}

//
// dtor
//
XamlImageBrush::~XamlImageBrush()
throw()
{
}

//
// from XamlResource 
//
bool XamlImageBrush::operator==(
    const XamlResource &other) const        // comparee
{
    if(other.resourceType() == XamlResource::eImageBrushResource)
    {
        const XamlImageBrush *pCasted = (const XamlImageBrush *) &other;
        if(pCasted->_zName == _zName &&
            pCasted->_oXfo == _oXfo &&
            pCasted->_nDpi == _nDpi &&
            pCasted->_nWidth == _nWidth &&
            pCasted->_nHeight == _nHeight)
        {
            return true;
        }
    }
    return false;
}

//
// from XamlResource 
//
XamlResource *XamlImageBrush::copy() const
{
    return new XamlImageBrush(_zName, _oXfo, _nDpi, _nWidth, _nHeight);
}

//
// from XamlResource 
//
XamlResource::eResourceType XamlImageBrush::resourceType() const
{
    return XamlResource::eImageBrushResource;
}

//
// from XamlResource 
//
WT_Result XamlImageBrush::serializeResource(
    const wchar_t*pKey,                             // key, != 0
    WT_XAML_File &,                                 // current file
    DWFXMLSerializer &rXml) const                   // rXml serializer
{
    DWFString s, strokes;

    //
    // start with an <ImageBrush> element keyed by rKey
    //
    rXml.startElement(XamlXML::kpzImageBrush_Element);
    rXml.addAttribute(XamlXML::kpzXkey_Attribute, pKey);
    rXml.addAttribute(XamlXML::kpzImageSource_Attribute, _zName);

    wchar_t pBuf[ 256 ];

    double dpiFactor = 1.0;
    if(_nDpi != -1)
        dpiFactor = 96./_nDpi;

   _DWFCORE_SWPRINTF(
    pBuf,
        256,
        L"%ls,%ls,%ls,%ls,%ls,%ls",
        (const wchar_t*)DWFString::DoubleToString((_oXfo(0,0)/ dpiFactor), 17),
        (const wchar_t*)DWFString::DoubleToString(_oXfo(1,0), 17),
        (const wchar_t*)DWFString::DoubleToString(_oXfo(0,1), 17),
        (const wchar_t*)DWFString::DoubleToString((_oXfo(1,1) / dpiFactor), 17),
        (const wchar_t*)DWFString::DoubleToString(_oXfo(2,0), 17),
        (const wchar_t*)DWFString::DoubleToString(_oXfo(2,1), 17)
    );

    rXml.addAttribute(XamlXML::kpzTransform_Attribute, pBuf);

    _DWFCORE_SWPRINTF(
    pBuf,
        256,
        L"0, 0, %ls, %ls",
        (const wchar_t*)DWFString::DoubleToString((_nWidth * dpiFactor), 17),
        (const wchar_t*)DWFString::DoubleToString((_nHeight * dpiFactor), 17)
    );

    rXml.addAttribute(XamlXML::kpzViewport_Attribute, pBuf);
    rXml.addAttribute(XamlXML::kpzViewbox_Attribute, pBuf);
    rXml.addAttribute(XamlXML::kpzViewportUnits_Attribute, XamlXML::kpzAbsolute_Value);
    rXml.addAttribute(XamlXML::kpzViewboxUnits_Attribute, XamlXML::kpzAbsolute_Value);
    rXml.addAttribute(XamlXML::kpzTileMode_Attribute, XamlXML::kpzTile_Value);

    rXml.endElement();

    return WT_Result::Success;
}

//
// from XamlBrush::Image - not implemented and returning an error
//
WT_Result XamlImageBrush::serializeElement(
    WT_XAML_File &,                                 // current file
    DWFCore::DWFXMLSerializer *) const              // XML utility, !=0
{
    return WT_Result::Internal_Error;
}
