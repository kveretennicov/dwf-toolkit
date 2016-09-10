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

using namespace XamlDrawableAttributes;

const char* const kpzEmpty_String = "";

///////////////////////////////////////////////////////
BrushRef::BrushRef()
: _pBrush(0)
{
}

BrushRef::BrushRef( const XamlBrush::Brush* p)
{
    BrushRef::operator= (p);
}

const BrushRef& BrushRef::operator=( const BrushRef& rBrush )
{
    return BrushRef::operator= (rBrush._pBrush);
}

const BrushRef& BrushRef::operator= ( const XamlBrush::Brush* pBrush )
{
    if (_pBrush != NULL)
    {
        if (_pBrush->owner() != this)
        {
            _pBrush->unobserve( *this );
        }
        else
        {
            DWFCORE_FREE_OBJECT( _pBrush );
        }
    }
        
    _pBrush = const_cast<XamlBrush::Brush*>(pBrush);
    if (pBrush != NULL) 
    {
        _pBrush->own( *this );
    }
    return *this;
}

bool BrushRef::operator==(const BrushRef &r) const
{
    if (_pBrush && r._pBrush)
    {
        return *_pBrush == *r._pBrush;
    }

    return _pBrush == r._pBrush;
}

bool BrushRef::operator==(const XamlBrush::Brush* p) const
{
    if (_pBrush && p)
    {
        return *_pBrush == *p;
    }

    return _pBrush == p;
}

BrushRef::~BrushRef()
throw()
{
    if (_pBrush != NULL)
    {
        if (_pBrush->owner() != this)
        {
            _pBrush->unobserve( *this );
        }
        else
        {
            DWFCORE_FREE_OBJECT( _pBrush );
        }
    }
}

void 
BrushRef::notifyOwnableDeletion( DWFOwnable& rOwnable ) 
throw( DWFException )
{
    if (&rOwnable == _pBrush)
    {
        _pBrush = NULL;
    }
}    

///////////////////////////////////////////////////////
StringRef::StringRef()
{
}

StringRef::StringRef( const wchar_t* p)
{
    StringRef::operator= (p);
}

StringRef::StringRef( const char* p)
{
    StringRef::operator= (p);
}

const StringRef& StringRef::operator= ( const wchar_t* pString )
{
    _szString = pString;
    return *this;
}

const StringRef& StringRef::operator= ( const char* pString )
{
    _szString = pString;
    return *this;
}


bool StringRef::operator==(const StringRef &r) const
{
    return _szString == r._szString;
}

bool StringRef::operator==(const wchar_t* p) const
{
    return _szString == p;
}

StringRef::~StringRef()
{
}


///////////////////////////////////////////////////////
MatrixRef::MatrixRef()
{ 
}

MatrixRef::MatrixRef( const MatrixRef& r)
{
    MatrixRef::operator =(r);
}

const MatrixRef& 
MatrixRef::operator=( const WT_Matrix& rMatrix )  //converts to 3x3
{
    WT_Matrix2D xform2D;
    
    xform2D(0,0) = rMatrix(0,0);
    xform2D(0,1) = rMatrix(0,1);
    xform2D(1,0) = rMatrix(1,0);
    xform2D(1,1) = rMatrix(1,1);
    xform2D(2,0) = rMatrix(3,0);
    xform2D(2,1) = rMatrix(3,1);

    return MatrixRef::operator =( xform2D );
}

const MatrixRef& 
MatrixRef::operator=( const MatrixRef& r )
{
    return MatrixRef::operator =( r._oMatrix );
}

const MatrixRef& 
MatrixRef::operator=( const WT_Matrix2D& r )
{
    _oMatrix = r;

    wchar_t buf[128];
    _DWFCORE_SWPRINTF(buf, 128, L"%ls,%ls,%ls,%ls,%ls,%ls", 
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(0,0),10) ,
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(1,0),10),
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(0,1),10),
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(1,1),10),
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(2,0),10),
        (const wchar_t*)DWFString::DoubleToString(_oMatrix(2,1),10) );
    _szMatrixString = buf;

    return *this;
}

const MatrixRef& 
MatrixRef::operator=( const char* p )
{

    WT_Matrix2D oMatrix;
    int nProcessed = sscanf(p, "%lG,%lG,%lG,%lG,%lG,%lG", 
        &oMatrix(0,0), &oMatrix(1,0),
        &oMatrix(0,1), &oMatrix(1,1),
        &oMatrix(2,0), &oMatrix(2,1) );

    if (nProcessed == 6)
    {
        _oMatrix = oMatrix;
        _szMatrixString = p;
    }
    return *this;
}

bool 
MatrixRef::operator==(const MatrixRef& r) const
{
    return !!(_oMatrix == r._oMatrix); //convert bool
}

bool 
MatrixRef::operator==(const WT_Matrix2D& r) const
{
    return !!(_oMatrix == r); //convert bool
}

bool 
MatrixRef::operator!=(const WT_Matrix2D& r) const
{
    return !(_oMatrix == r); //you'd think the compiler would automatically figure this out. :(
}

MatrixRef::~MatrixRef()
{
}

///////////////////////////////////////////////////////

//
// optimization :
// see XamlDrawableAttributes.h for details - the buffer
// size is arbitrarily set to 64Kb
//
#define DATA_BUFFER_SIZE (64 * 1024)
size_t Data::_nBufferSize = DATA_BUFFER_SIZE;

Data::Data(const Data &r)
: _oGeometry()
{
    Data::operator=(r);
}

Data& Data::operator=(const Data &r)
{
    if (&r != this)
    {
        _oGeometry = r._oGeometry;
    }
    return *this;
}

bool Data::operator==(const Data &r) const
{
    return _oGeometry == r._oGeometry;
}

WT_Result
Data::materializeAttribute( WT_XAML_File & rFile, const char* pAttribute )
{
    return _oGeometry.materializeAttribute( rFile, pAttribute );
}

WT_Result 
Data::serializeAttribute( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pXmlSerializer ) const
{
    char buf[32];
    _DWFCORE_SPRINTF(buf, 32, " %s=\"", XamlXML::kpzData_Attribute);

    tMemoryBuffer *pBuffer = rFile.getBuffer( _nBufferSize );
    pBuffer->set(buf);

    WD_CHECK( _oGeometry.serializeAttributeValue(rFile, pBuffer) ); //populates the attribute buffer

    pBuffer->concatenate(/*NOXLATE*/"\"");
    _nBufferSize = pBuffer->size();

    size_t nLen = pBuffer->strlen();

    if (nLen > 10) //arbitrary, but greater than the attribute name at any rate
    {
        DWFBufferInputStream oInputStream( pBuffer->buffer(), nLen );
        pXmlSerializer->insertXMLStream( &oInputStream, false );
    }
    rFile.releaseBuffer( pBuffer );

    return WT_Result::Success;
}

WT_Result 
Data::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
Data::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Name::Name( const Name& r )
{
    Name::operator=(r);
}

Name& Name::operator=( const Name& r )
{
    _oString = r._oString;
    return *this;
}

bool Name::operator==(const Name &r) const
{
    return _oString == r._oString;
}

WT_Result
Name::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _oString = pAttribute;
    return WT_Result::Success;
}

WT_Result 
Name::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
	pSerializer->addAttribute( XamlXML::kpzName_Attribute, _oString );
    return WT_Result::Success;
}

WT_Result 
Name::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
 }

bool 
Name::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Fill::Fill()
: _oBrush()
{
}

Fill::Fill( const Fill& r )
{
    Fill::operator=(r);
}

Fill::~Fill()
{
}

Fill& Fill::operator=( const Fill& r )
{
    _oBrush = r._oBrush;
    return *this;
}

bool Fill::operator==(const Fill &r) const
{
    return _oBrush == r._oBrush;
}

WT_Result
Fill::materializeAttribute( WT_XAML_File &rFile, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    if (pAttribute[0] == '#')
    {
        XamlBrush::SolidColor *pSolid = new XamlBrush::SolidColor();
        if (pSolid == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        WD_CHECK( pSolid->materializeAttribute( rFile, pAttribute ) );

        _oBrush = pSolid;
    }
    else
    {
        //There can be Fill_Pattern,User_Fill_Pattern,User_Hatch_Pattern or Images. All of these
        //objects should have shells created in file's object_list. So find it and create appropriate
        //brush so XamlObjectFactory::_processPathObjects() can create right kind of objects
        std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
	    std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;
		XamlBrush::Brush *pSomeKindOfVisualBrush = NULL;

		if (rFile.object_list().find_by_id(WT_Object::Fill_Pattern_ID, iStart, iEnd))
		{
           
			WT_Fill_Pattern* pPat = static_cast< WT_Fill_Pattern* >( iStart->second );
            if(pPat)
            {
                pSomeKindOfVisualBrush = new XamlBrush::XamlFixedPatternBrush(pPat->pattern_id(),pPat->pattern_scale());
                if (pSomeKindOfVisualBrush == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
            }
                
           
        }
        else if( rFile.object_list().find_by_id(WT_Object::User_Hatch_Pattern_ID, iStart, iEnd) )
        {
        
		    WT_User_Hatch_Pattern* pPat = static_cast< WT_User_Hatch_Pattern* >( iStart->second );
            if(pPat && pPat->pattern_count())
            {
                pSomeKindOfVisualBrush = new XamlBrush::XamlHatchBrush(const_cast< WT_User_Hatch_Pattern::Hatch_Pattern * >
                                                            (pPat->pattern(0)));
                if (pSomeKindOfVisualBrush == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
            }                
                        
        }
        else if( rFile.object_list().find_by_id(WT_Object::User_Fill_Pattern_ID, iStart, iEnd) )
        {
            WT_User_Fill_Pattern* pPat = static_cast< WT_User_Fill_Pattern* >( iStart->second );
            if(pPat)
            {
                pSomeKindOfVisualBrush = new XamlBrush::XamlUserPatternBrush(
                                                const_cast< WT_User_Fill_Pattern::Fill_Pattern * >(pPat->fill_pattern()),
                                                pPat->pattern_scale());
                if (pSomeKindOfVisualBrush == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
            }    
        }
        else if( (!(rFile.object_list().find_by_id(WT_Object::Image_ID, iStart, iEnd)) ) &&
                  (!(rFile.object_list().find_by_id(WT_Object::PNG_Group4_Image_ID, iStart, iEnd))) )
        {
            pSomeKindOfVisualBrush = new XamlBrush::SolidColor();
            if (pSomeKindOfVisualBrush == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }
        
        //WD_Assert( false );
        //FUTURE: When we are purely XAML, we would need to implement visual 
		//and image brush resource cases here.  For DWFx (tk 7.4), our fill/hatch
		//and image information is contained in the W2X
        if(pSomeKindOfVisualBrush)
            _oBrush = pSomeKindOfVisualBrush;
    }
    return WT_Result::Success;
}

WT_Result 
Fill::serializeAttribute( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }
    else if (_oBrush->type() == XamlBrush::kSolidColor) //special case shorthand for solid color brushes
    {
        const XamlBrush::SolidColor *pSolidColor = 
            static_cast<const XamlBrush::SolidColor*>( static_cast<const XamlBrush::Brush *>( _oBrush ) );
        
        pSerializer->addAttribute( XamlXML::kpzFill_Attribute, pSolidColor->colorString() );
        
        return WT_Result::Success;
    }  
    else if (_oBrush->type() == XamlBrush::kImage)
    {
        //
        // down-cast to an Image
        //
        const XamlBrush::Image *pImage =
            static_cast<const XamlBrush::Image*>(
                static_cast<const XamlBrush::Brush *>( _oBrush )
                );
        //
        // - lookup the resource in the dictionary
        // - use the key to set Fill to "{StaticResource=xxx}"
        //
        DWFString buf = XamlXML::kpzStaticResource_Declaration;
        buf.append(rFile.dictionary()->lookup( *pImage ));
        buf.append(XamlXML::kpzStaticResource_DeclarationEnd);

        pSerializer->addAttribute( XamlXML::kpzFill_Attribute, buf );

        return WT_Result::Success;
    }
    else if (_oBrush->type() == XamlBrush::kVisual)
    {
        //
        // down-cast to a Visual
        //
        const XamlBrush::Visual *pVisual =
            static_cast<const XamlBrush::Visual*>(
                static_cast<const XamlBrush::Brush *>( _oBrush )
                );
        //
        // - lookup the resource in the dictionary
        // - use the key to set Fill to "{StaticResource=xxx}"
        //
        DWFString buf = XamlXML::kpzStaticResource_Declaration;
        buf.append(rFile.dictionary()->lookup( *pVisual ));
        buf.append(XamlXML::kpzStaticResource_DeclarationEnd);

        pSerializer->addAttribute( XamlXML::kpzFill_Attribute, buf );

        return WT_Result::Success;
    }
    
    // Non-solid brushes cannot be serialized as an attribute
    return WT_Result::Internal_Error;
}


WT_Result 
Fill::serializeElement( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }

    char szName[64];
    sprintf(szName, "%s.%s", XamlXML::kpzPath_Element, XamlXML::kpzFill_Attribute);
    pSerializer->startElement( szName );
    WT_Result res = _oBrush->serializeElement( rFile, pSerializer );
    pSerializer->endElement();
    return res;
}

bool 
Fill::validAsAttribute( void ) const
{
    if ( _oBrush == NULL ||
         _oBrush->type() == XamlBrush::kVisual ||   // <VisualBrush> is a resource
         _oBrush->type() == XamlBrush::kImage ||    // <ImageBrush> is a resource
         _oBrush->type() == XamlBrush::kSolidColor) // special case shorthand for solid color brushes
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////
RenderTransform::RenderTransform( const RenderTransform& r )
{
    RenderTransform::operator=(r);
}

RenderTransform& RenderTransform::operator=( const RenderTransform& r )
{
    _oMatrix = r._oMatrix;
    return *this;
}

bool RenderTransform::operator==(const RenderTransform &r) const
{
    return _oMatrix == r._oMatrix;
}

WT_Result
RenderTransform::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _oMatrix = pAttribute; //and we hope for the best
    return WT_Result::Success;
}

WT_Result 
RenderTransform::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_oMatrix != WT_Matrix2D::kIdentity)
    {
        pSerializer->addAttribute( XamlXML::kpzRenderTransform_Attribute, _oMatrix );
    }
    return WT_Result::Success;
}

WT_Result 
RenderTransform::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
RenderTransform::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Clip::Clip( const Clip& r )
{
    Clip::operator=(r);
}

Clip& Clip::operator=( const Clip& r )
{
    if (&r != this)
    {
        _oGeometry = r._oGeometry;
    }
    return *this;
}

bool Clip::operator==(const Clip &r) const
{
    return _oGeometry == r._oGeometry;
}

WT_Result
Clip::materializeAttribute( WT_XAML_File &rFile, const char* pAttribute )
{
    return _oGeometry.materializeAttribute( rFile, pAttribute );
}

WT_Result 
Clip::serializeAttribute( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pXmlSerializer ) const
{
    char buf[32];
    _DWFCORE_SPRINTF(buf, 32, " %s=\"", XamlXML::kpzClip_Attribute);

    tMemoryBuffer *pBuffer = rFile.getBuffer( 32768 );
    pBuffer->set(buf);

    WD_CHECK( _oGeometry.serializeAttributeValue(rFile, pBuffer) ); //populates the attribute buffer

    pBuffer->concatenate(/*NOXLATE*/"\"");

    size_t nLen = pBuffer->strlen();
    if (nLen > 10) //arbitrary, but greater than the attribute name at any rate
    {
        DWFBufferInputStream oInputStream( pBuffer->buffer(), nLen );
        pXmlSerializer->insertXMLStream( &oInputStream, false );
    }
    rFile.releaseBuffer( pBuffer );

    return WT_Result::Success;
}

WT_Result 
Clip::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
Clip::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Opacity::Opacity() 
: _fValue(1.0) 
{
}

Opacity::Opacity( const Opacity& r )
{
    Opacity::operator=(r);
}

Opacity& Opacity::operator=( const Opacity& r )
{
    _fValue= r._fValue;
    return *this;
}

bool Opacity::operator==(const Opacity &r) const
{
    return _fValue == r._fValue;
}

WT_Result
Opacity::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
Opacity::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pXmlSerializer ) const
{
    if (_fValue != 1.0) //default
    {
        pXmlSerializer->addAttribute(XamlXML::kpzOpacity_Attribute, _fValue);
    }
    return WT_Result::Success;
}

WT_Result 
Opacity::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
Opacity::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
OpacityMask::OpacityMask()
{
}

OpacityMask::OpacityMask( const OpacityMask& r )
{
    OpacityMask::operator=(r);
}

OpacityMask::~OpacityMask()
{
}

OpacityMask& OpacityMask::operator=( const OpacityMask& r )
{
    _oBrush = r._oBrush;
    return *this;
}

bool OpacityMask::operator==(const OpacityMask &r) const
{
    return _oBrush == r._oBrush;
}

WT_Result
OpacityMask::materializeAttribute( WT_XAML_File & rFile, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    if (pAttribute[0] == '#')
    {
        XamlBrush::SolidColor *pSolid = new XamlBrush::SolidColor();
        if (pSolid == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        WD_CHECK( pSolid->materializeAttribute( rFile, pAttribute ) );

        _oBrush = pSolid;
    }
    else
    {
        WD_Assert( false );
        //FUTURE: When we are purely XAML, we would need to implement visual 
		//and image brush resource cases here.  For DWFx (tk 7.4), opacity is
		//not utilized other than via the visibility rendition attribute.
    }
    return WT_Result::Success;
}

WT_Result 
OpacityMask::serializeAttribute( WT_XAML_File & /*rFile*/, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }
    else if (_oBrush->type() == XamlBrush::kSolidColor) //special case shorthand for solid color brushes
    {
        const XamlBrush::SolidColor *pSolidColor = 
            static_cast<const XamlBrush::SolidColor*>( static_cast<const XamlBrush::Brush*>( _oBrush ) );
        pSerializer->addAttribute( XamlXML::kpzOpacityMask_Attribute, pSolidColor->colorString() );
        return WT_Result::Success;
    }    
    
    // Non-solid brushes cannot be serialized as an attribute
    return WT_Result::Internal_Error;
}


WT_Result 
OpacityMask::serializeElement( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }
    
    DWFString zElementName = XamlXML::kpzPath_Element;
    zElementName.append( "." );
    zElementName.append( XamlXML::kpzOpacityMask_Attribute );

    pSerializer->startElement( zElementName );
    WT_Result res = _oBrush->serializeElement( rFile, pSerializer );
    pSerializer->endElement();
    return res;
}

bool 
OpacityMask::validAsAttribute( void ) const
{
    if ( _oBrush == NULL ||
         _oBrush->type() == XamlBrush::kSolidColor) //special case shorthand for solid color brushes
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////
Stroke::Stroke()
{
}

Stroke::Stroke( const Stroke& r )
{
    Stroke::operator=(r);
}

Stroke::~Stroke()
{
}

Stroke& Stroke::operator=( const Stroke& r )
{
    _oBrush = r._oBrush;
    return *this;
}

bool Stroke::operator==(const Stroke &r) const
{
    return _oBrush == r._oBrush;
}

WT_Result
Stroke::materializeAttribute( WT_XAML_File &rFile, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    else if (pAttribute[0] == '#')
    {
        XamlBrush::SolidColor *pSolid = new XamlBrush::SolidColor();
        if (pSolid == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        WD_CHECK( pSolid->materializeAttribute( rFile, pAttribute ) );

        _oBrush = pSolid;
    }
    else
    {
        WD_Assert( false );
        //FUTURE: When we are purely XAML, we would need to implement non-solid 
		//brushes for strokes.  For DWFx (tk 7.4), strokes are always solid, 
		//except gradient strokes, which are serialized as elements..
    }
    return WT_Result::Success;
}

WT_Result 
Stroke::serializeAttribute( WT_XAML_File & /*rFile*/, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }
    else if (_oBrush->type() == XamlBrush::kSolidColor) //special case shorthand for solid color brushes
    {
        const XamlBrush::SolidColor *pSolidColor = 
            static_cast<const XamlBrush::SolidColor*>( static_cast<const XamlBrush::Brush*>( _oBrush ) );
        pSerializer->addAttribute( XamlXML::kpzStroke_Attribute, pSolidColor->colorString() );
        return WT_Result::Success;
    }    
    
    // Non-solid brushes cannot be serialized as an attribute
    return WT_Result::Internal_Error;
}


WT_Result 
Stroke::serializeElement( WT_XAML_File &rFile, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _oBrush == NULL )
    {
        // no content
        return WT_Result::Success;
    }
    
    DWFString zElementName = XamlXML::kpzPath_Element;
    zElementName.append( "." );
    zElementName.append( XamlXML::kpzStroke_Attribute );

    pSerializer->startElement( zElementName );
    WT_Result res = _oBrush->serializeElement( rFile, pSerializer );
    pSerializer->endElement();
    return res;
}

bool 
Stroke::validAsAttribute( void ) const
{
    if ( _oBrush == NULL ||
         _oBrush->type() == XamlBrush::kSolidColor) //special case shorthand for solid color brushes
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////
StrokeDashArray::StrokeDashArray( const StrokeDashArray& r )
{
    StrokeDashArray::operator=(r);
}

StrokeDashArray& StrokeDashArray::operator=( const StrokeDashArray& r )
{
    _vfDashGap = r._vfDashGap;
    return *this;
}

bool StrokeDashArray::operator==(const StrokeDashArray &r) const
{
    return _vfDashGap == r._vfDashGap;
}

WT_Result
StrokeDashArray::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    
    char* pSavePtr;
    char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( const_cast<char*>( pAttribute ), /*NOXLATE*/" ", &pSavePtr );
    for (; pToken != NULL; pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr ))
    {
        float fDash = (float) DWFString::StringToDouble(pToken);
        pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
        if (pToken == NULL)
        {
            return WT_Result::Corrupt_File_Error;
        }
        float fGap = (float) DWFString::StringToDouble(pToken);

        _vfDashGap.push_back( tDashGapPair( fDash, fGap ) );
    }
    return WT_Result::Success;
}

WT_Result 
StrokeDashArray::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if ( _vfDashGap.size() > 0 )
    {
        float *faDashGap = DWFCORE_ALLOC_MEMORY( float, _vfDashGap.size()*2 );
        if (faDashGap == NULL)
            return WT_Result::Out_Of_Memory_Error;
            
        size_t i=0;
        for (;i<_vfDashGap.size();i++)
        {
            faDashGap[(i*2)] = _vfDashGap[i].first;
            faDashGap[(i*2)+1] = _vfDashGap[i].second;
        }
        pSerializer->addAttribute( XamlXML::kpzStrokeDashArray_Attribute, faDashGap, _vfDashGap.size()*2 );

		DWFCORE_FREE_MEMORY( faDashGap );
    }
    return WT_Result::Success;
}

WT_Result 
StrokeDashArray::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeDashArray::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeDashCap::StrokeDashCap()
: _eCapStyle( StrokeDashCap::Flat )
{ }


StrokeDashCap::StrokeDashCap( const StrokeDashCap& r )
{
    StrokeDashCap::operator=(r);
}

StrokeDashCap& StrokeDashCap::operator=( const StrokeDashCap& r )
{
    _eCapStyle = r._eCapStyle; 
    return *this;
}

bool StrokeDashCap::operator==(const StrokeDashCap &r) const
{
    return _eCapStyle == r._eCapStyle;
}

WT_Result
StrokeDashCap::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _eCapStyle = Flat;
    if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzSquare_Value) == 0)
    {
        _eCapStyle = Square;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzRound_Value ) == 0)
    {
        _eCapStyle = Round;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzTriangle_Value ) == 0)
    {
        _eCapStyle = Triangle;
    }

    return WT_Result::Success;
}

WT_Result 
StrokeDashCap::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    switch( _eCapStyle )
    {
    case Flat:
        return WT_Result::Success; // default

    case Square:
        pSerializer->addAttribute( XamlXML::kpzStrokeDashCap_Attribute, XamlXML::kpzSquare_Value );
        break;
    case Round:
        pSerializer->addAttribute( XamlXML::kpzStrokeDashCap_Attribute, XamlXML::kpzRound_Value );
        break;
    case Triangle:
        pSerializer->addAttribute( XamlXML::kpzStrokeDashCap_Attribute, XamlXML::kpzTriangle_Value );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

WT_Result 
StrokeDashCap::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeDashCap::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeDashOffset::StrokeDashOffset()
: _fValue(0.0)
{ }

StrokeDashOffset::StrokeDashOffset( const StrokeDashOffset& r )
{
    StrokeDashOffset::operator=(r);
}

StrokeDashOffset& StrokeDashOffset::operator=( const StrokeDashOffset& r )
{
    _fValue = r._fValue;
    return *this;
}

bool StrokeDashOffset::operator==(const StrokeDashOffset &r) const
{
    return _fValue == r._fValue;
}

WT_Result
StrokeDashOffset::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
StrokeDashOffset::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_fValue != 0.0) //if not default
    {
        pSerializer->addAttribute( XamlXML::kpzStrokeDashOffset_Attribute, _fValue );
    }
    return WT_Result::Success;
}

WT_Result 
StrokeDashOffset::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeDashOffset::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeEndLineCap::StrokeEndLineCap()
: _eCapStyle( StrokeEndLineCap::Flat )
{ }


StrokeEndLineCap::StrokeEndLineCap( const StrokeEndLineCap& r )
{
    StrokeEndLineCap::operator=(r);
}

StrokeEndLineCap& StrokeEndLineCap::operator=( const StrokeEndLineCap& r )
{
    _eCapStyle = r._eCapStyle; 
    return *this;
}

bool StrokeEndLineCap::operator==(const StrokeEndLineCap &r) const
{
    return _eCapStyle == r._eCapStyle;
}

WT_Result
StrokeEndLineCap::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _eCapStyle = Flat;
    if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzSquare_Value) == 0)
    {
        _eCapStyle = Square;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzRound_Value ) == 0)
    {
        _eCapStyle = Round;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzTriangle_Value ) == 0)
    {
        _eCapStyle = Triangle;
    }

    return WT_Result::Success;
}

WT_Result 
StrokeEndLineCap::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    switch( _eCapStyle )
    {
    case Flat:
        return WT_Result::Success; // default

    case Square:
        pSerializer->addAttribute( XamlXML::kpzStrokeEndLineCap_Attribute, XamlXML::kpzSquare_Value );
        break;
    case Round:
        pSerializer->addAttribute( XamlXML::kpzStrokeEndLineCap_Attribute, XamlXML::kpzRound_Value );
        break;
    case Triangle:
        pSerializer->addAttribute( XamlXML::kpzStrokeEndLineCap_Attribute, XamlXML::kpzTriangle_Value );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

WT_Result 
StrokeEndLineCap::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeEndLineCap::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeStartLineCap::StrokeStartLineCap()
: _eCapStyle( StrokeStartLineCap::Flat )
{ }


StrokeStartLineCap::StrokeStartLineCap( const StrokeStartLineCap& r )
{
    StrokeStartLineCap::operator=(r);
}

StrokeStartLineCap& StrokeStartLineCap::operator=( const StrokeStartLineCap& r )
{
    _eCapStyle = r._eCapStyle; 
    return *this;
}

bool StrokeStartLineCap::operator==(const StrokeStartLineCap &r) const
{
    return _eCapStyle == r._eCapStyle;
}

WT_Result
StrokeStartLineCap::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _eCapStyle = Flat;
    if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzSquare_Value) == 0)
    {
        _eCapStyle = Square;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzRound_Value ) == 0)
    {
        _eCapStyle = Round;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzTriangle_Value ) == 0)
    {
        _eCapStyle = Triangle;
    }

    return WT_Result::Success;
}

WT_Result 
StrokeStartLineCap::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    switch( _eCapStyle )
    {
    case Flat:
        return WT_Result::Success; // default

    case Square:
        pSerializer->addAttribute( XamlXML::kpzStrokeStartLineCap_Attribute, XamlXML::kpzSquare_Value );
        break;
    case Round:
        pSerializer->addAttribute( XamlXML::kpzStrokeStartLineCap_Attribute, XamlXML::kpzRound_Value );
        break;
    case Triangle:
        pSerializer->addAttribute( XamlXML::kpzStrokeStartLineCap_Attribute, XamlXML::kpzTriangle_Value );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

WT_Result 
StrokeStartLineCap::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeStartLineCap::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeLineJoin::StrokeLineJoin()
: _eJoinStyle( StrokeLineJoin::None )
{ }


StrokeLineJoin::StrokeLineJoin( const StrokeLineJoin& r )
{
    StrokeLineJoin::operator=(r);
}

StrokeLineJoin& StrokeLineJoin::operator=( const StrokeLineJoin& r )
{
    _eJoinStyle = r._eJoinStyle; 
    return *this;
}

bool StrokeLineJoin::operator==(const StrokeLineJoin &r) const
{
    return _eJoinStyle == r._eJoinStyle;
}

WT_Result
StrokeLineJoin::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _eJoinStyle = None;
    if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzMiter_Value) == 0)
    {
        _eJoinStyle = Miter;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzBevel_Value ) == 0)
    {
        _eJoinStyle = Bevel;
    }
    else if (DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzRound_Value ) == 0)
    {
        _eJoinStyle = Round;
    }

    return WT_Result::Success;
}

WT_Result 
StrokeLineJoin::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    switch( _eJoinStyle )
    {
    case None:
        return WT_Result::Success; // default
    case Miter:
        pSerializer->addAttribute( XamlXML::kpzStrokeLineJoin_Attribute, XamlXML::kpzMiter_Value );
        break;
    case Bevel:
        pSerializer->addAttribute( XamlXML::kpzStrokeLineJoin_Attribute, XamlXML::kpzBevel_Value );
        break;
    case Round:
        pSerializer->addAttribute( XamlXML::kpzStrokeLineJoin_Attribute, XamlXML::kpzRound_Value );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

WT_Result 
StrokeLineJoin::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeLineJoin::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeMiterLimit::StrokeMiterLimit()
: _fValue(1.0)
{ }

StrokeMiterLimit::StrokeMiterLimit( const StrokeMiterLimit& r )
{
    StrokeMiterLimit::operator=(r);
}

StrokeMiterLimit& StrokeMiterLimit::operator=( const StrokeMiterLimit& r )
{
    _fValue = r._fValue;
    return *this;
}

bool StrokeMiterLimit::operator==(const StrokeMiterLimit &r) const
{
    return _fValue == r._fValue;
}

WT_Result
StrokeMiterLimit::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
StrokeMiterLimit::serializeAttribute( WT_XAML_File & , DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_fValue > 1.0) //if not default and if valid
		pSerializer->addAttribute( XamlXML::kpzStrokeMiterLimit_Attribute, _fValue );
   
    return WT_Result::Success;
}

WT_Result 
StrokeMiterLimit::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeMiterLimit::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
StrokeThickness::StrokeThickness()
: _nWeight(1)
{ 
}

StrokeThickness::StrokeThickness( const StrokeThickness& r )
{
    StrokeThickness::operator=(r);
}

StrokeThickness& StrokeThickness::operator=( const StrokeThickness& r )
{
    _nWeight = r._nWeight; 
    return *this;
}

bool StrokeThickness::operator==(const StrokeThickness &r) const
{
    return _nWeight == r._nWeight; 
}

WT_Result
StrokeThickness::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _nWeight = atoi( pAttribute );

    return WT_Result::Success;
}

WT_Result 
StrokeThickness::serializeAttribute( WT_XAML_File &/*rFile*/, DWFCore::DWFXMLSerializer *pXmlSerializer ) const
{
    if (_nWeight != 1) // default
    {
        pXmlSerializer->addAttribute( XamlXML::kpzStrokeThickness_Attribute, (int)_nWeight);
    }
    return WT_Result::Success;   
}

WT_Result 
StrokeThickness::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Internal_Error;
}

bool 
StrokeThickness::validAsAttribute( void ) const
{
    return true;
}


///////////////////////////////////////////////////////
NavigateUri::NavigateUri( const NavigateUri& r )
{
    NavigateUri::operator=(r);
}

NavigateUri& NavigateUri::operator=( const NavigateUri& r )
{
    _oString = r._oString;
    return *this;
}

bool NavigateUri::operator==(const NavigateUri &r) const
{
    return _oString == r._oString;
}

WT_Result
NavigateUri::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _oString = pAttribute;
    return WT_Result::Success;
}

WT_Result 
NavigateUri::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    //>>HACK BEGIN
    // This is very wrong.  At this writing, Microsoft's XPS viewers cannot handle arguments in the
    // URLs attached to XAML objects.  To compensate for this gross malfeasance, we have no choice but
    // to whack the arguments in the URL.  When they finally get their act together, this hack should
    // be removed.  J. Klug  3/19/2007
    const DWFString& rString = _oString;
    off_t nOffset = rString.find('?');
    if (nOffset >=0)
    {
        pSerializer->addAttribute( XamlXML::kpzNavigateUri_Attribute, rString.substring(0, nOffset) );
        return WT_Result::Success;
    }
    //<<HACK END

    pSerializer->addAttribute( XamlXML::kpzNavigateUri_Attribute, _oString );
    return WT_Result::Success;
}

WT_Result 
NavigateUri::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    DWFString zElementNavigateUri = XamlXML::kpzPath_Element;
    zElementNavigateUri.append( "." );
    zElementNavigateUri.append( XamlXML::kpzNavigateUri_Attribute );

    pSerializer->startElement( zElementNavigateUri );
    pSerializer->addCData( _oString );
    pSerializer->endElement();

    return WT_Result::Success;
 }

bool 
NavigateUri::validAsAttribute( void ) const
{
    return true;
}


///////////////////////////////////////////////////////
IndexArrayRef::IndexArrayRef()
: _nCount(0)
, _pnArray(NULL)
{ }

IndexArrayRef::IndexArrayRef( int* pnArray, size_t nCount ) throw(DWFException)
: _nCount(nCount)
, _pnArray(NULL)
{
    if (nCount != 0)
    {
        _pnArray = DWFCORE_ALLOC_MEMORY( int, nCount );
        if (_pnArray == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Out of memory" );
        }
        DWFCORE_COPY_MEMORY( _pnArray, pnArray, sizeof( _pnArray ) );
    }
}

const IndexArrayRef& 
IndexArrayRef::operator=( vector<int> oArray ) 
throw(DWFException)
{
    if (_nCount != 0 && _pnArray != NULL)
    {
        _nCount = 0;
        DWFCORE_FREE_MEMORY( _pnArray );
    }

    if (oArray.size() != 0)
    {
        _pnArray = DWFCORE_ALLOC_MEMORY( int, oArray.size() );
        if (_pnArray == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Out of memory" );
        }
        for (size_t i=0; i< oArray.size(); i++)
        {
            _pnArray[i] = oArray[i];
        }
    }
    return *this;
}

const IndexArrayRef& 
IndexArrayRef::operator=( const IndexArrayRef& rSrc )
throw(DWFException)
{
    if (_nCount != 0 && _pnArray != NULL)
    {
        _nCount = 0;
        DWFCORE_FREE_MEMORY( _pnArray );
    }

    if (rSrc._nCount != 0)
    {
        _pnArray = DWFCORE_ALLOC_MEMORY( int, rSrc._nCount );
        if (_pnArray == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Out of memory" );
        }
        DWFCORE_COPY_MEMORY( _pnArray, rSrc._pnArray, sizeof( _pnArray ) );
    }
    return *this;
}

bool 
IndexArrayRef::operator==(const IndexArrayRef& r) const
{
    if (r._nCount != _nCount)
        return false;

    return 0 == DWFCORE_COMPARE_MEMORY( _pnArray, r._pnArray, sizeof( _pnArray ) );
}

IndexArrayRef::~IndexArrayRef()
{
    if (_nCount && _pnArray != NULL)
    {
        DWFCORE_FREE_MEMORY( _pnArray );
    }
}

const int 
IndexArrayRef::operator[]( unsigned int nIndex ) const 
throw(DWFException)
{ 
    if (nIndex >= _nCount)
    {
        _DWFCORE_THROW( DWFOverflowException, L"Overflowed index" );
    }

    return _pnArray[nIndex]; 
}

///////////////////////////////////////////////////////
BidiLevel::BidiLevel( const BidiLevel& r)
{
    BidiLevel::operator=( r );
}

BidiLevel& 
BidiLevel::operator=( const BidiLevel& r)
{
    _nLevel = r._nLevel;
    return *this;
}

bool 
BidiLevel::operator==(const BidiLevel& r) const
{
    return _nLevel == r._nLevel;
}

WT_Result
BidiLevel::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _nLevel = (short)atoi( pAttribute );
    return WT_Result::Success;
}

WT_Result 
BidiLevel::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_nLevel > 0)
    {
        pSerializer->addAttribute( XamlXML::kpzBidiLevel_Attribute, _nLevel );
    }
    return WT_Result::Success;
}

WT_Result 
BidiLevel::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
BidiLevel::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
CaretStops::CaretStops( const CaretStops& r)
{
    CaretStops::operator=( r );
}

CaretStops& 
CaretStops::operator=( const CaretStops& r)
{
    _oStops = r._oStops;
    return *this;
}

bool 
CaretStops::operator==(const CaretStops& r) const
{
    return _oStops == r._oStops;
}

WT_Result
CaretStops::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    vector<int> oArray;
    char* pSavePtr; 
    char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( const_cast<char*>( pAttribute ), /*NOXLATE*/" ", &pSavePtr );
    for (; pToken != NULL; pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr ))
    {
        int nStop = (int) atoi(pToken);
        oArray.push_back( nStop );
    }

    _oStops = oArray;

    return WT_Result::Success;
}

WT_Result
CaretStops::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_oStops != NULL && _oStops.count()>0)
    {
        pSerializer->addAttribute( XamlXML::kpzCaretStops_Attribute, (const int*) _oStops, _oStops.count() );
    }
    return WT_Result::Success;    
}

WT_Result
CaretStops::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool
CaretStops::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
DeviceFontName::DeviceFontName( const DeviceFontName& r )
{
    DeviceFontName::operator=(r);
}

DeviceFontName& DeviceFontName::operator=( const DeviceFontName& r )
{
    _oString = r._oString;
    return *this;
}

bool DeviceFontName::operator==(const DeviceFontName &r) const
{
    return _oString == r._oString;
}

WT_Result
DeviceFontName::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _oString = pAttribute;
    return WT_Result::Success;
}

WT_Result 
DeviceFontName::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (((const DWFCore::DWFString&)_oString).chars() > 0)
    {
    	pSerializer->addAttribute( XamlXML::kpzDeviceFontName_Attribute, _oString );
    }
    return WT_Result::Success;
}

WT_Result 
DeviceFontName::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
 }

bool 
DeviceFontName::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
FontRenderingEmSize::FontRenderingEmSize()
: _fValue(0.0)
{ }

FontRenderingEmSize::FontRenderingEmSize( const FontRenderingEmSize& r )
{
    FontRenderingEmSize::operator=(r);
}

FontRenderingEmSize& FontRenderingEmSize::operator=( const FontRenderingEmSize& r )
{
    _fValue = r._fValue;
    return *this;
}

bool FontRenderingEmSize::operator==(const FontRenderingEmSize &r) const
{
    return _fValue == r._fValue;
}

WT_Result
FontRenderingEmSize::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
FontRenderingEmSize::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    //Required, always serialize
    pSerializer->addAttribute( XamlXML::kpzFontRenderingEmSize_Attribute, _fValue );
    return WT_Result::Success;
}

WT_Result 
FontRenderingEmSize::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
FontRenderingEmSize::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
FontUri::FontUri( const FontUri& r )
{
    FontUri::operator=(r);
}

FontUri& FontUri::operator=( const FontUri& r )
{
    _oString = r._oString;
    return *this;
}

bool FontUri::operator==(const FontUri &r) const
{
    return _oString == r._oString;
}

WT_Result
FontUri::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _oString = pAttribute;
    return WT_Result::Success;
}

WT_Result 
FontUri::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    //Required, always serialize
	pSerializer->addAttribute( XamlXML::kpzFontUri_Attribute, _oString );
    return WT_Result::Success;
}

WT_Result 
FontUri::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    DWFString zElementFontUri = XamlXML::kpzPath_Element;
    zElementFontUri.append( "." );
    zElementFontUri.append( XamlXML::kpzFontUri_Attribute );

    pSerializer->startElement( zElementFontUri );
    pSerializer->addCData( _oString );
    pSerializer->endElement();

    return WT_Result::Success;
 }

bool 
FontUri::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
OriginX::OriginX()
: _fValue(0.0)
{ }

OriginX::OriginX( const OriginX& r )
{
    OriginX::operator=(r);
}

OriginX& OriginX::operator=( const OriginX& r )
{
    _fValue = r._fValue;
    return *this;
}

bool OriginX::operator==(const OriginX &r) const
{
    return _fValue == r._fValue;
}

WT_Result
OriginX::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
OriginX::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    //Required, always serialize
    pSerializer->addAttribute( XamlXML::kpzOriginX_Attribute, _fValue );
    return WT_Result::Success;
}

WT_Result 
OriginX::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
OriginX::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
OriginY::OriginY()
: _fValue(0.0)
{ }

OriginY::OriginY( const OriginY& r )
{
    OriginY::operator=(r);
}

OriginY& OriginY::operator=( const OriginY& r )
{
    _fValue = r._fValue;
    return *this;
}

bool OriginY::operator==(const OriginY &r) const
{
    return _fValue == r._fValue;
}

WT_Result
OriginY::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _fValue = (float) DWFString::StringToDouble( pAttribute );
    return WT_Result::Success;
}

WT_Result 
OriginY::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    //Required, always serialize
    pSerializer->addAttribute( XamlXML::kpzOriginY_Attribute, _fValue );
    return WT_Result::Success;
}

WT_Result 
OriginY::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
OriginY::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
IsSideways::IsSideways()
: _bValue(false)
{ }

IsSideways::IsSideways( const IsSideways& r )
{
    IsSideways::operator=(r);
}

IsSideways& IsSideways::operator=( const IsSideways& r )
{
    _bValue = r._bValue;
    return *this;
}

bool IsSideways::operator==(const IsSideways &r) const
{
    return _bValue == r._bValue;
}

WT_Result
IsSideways::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _bValue = false;
    if ( DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzTrue_Value ) == 0 )
    {
        _bValue = true;
    }

    return WT_Result::Success;
}

WT_Result 
IsSideways::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (_bValue) 
    {
        pSerializer->addAttribute( XamlXML::kpzIsSideways_Attribute, XamlXML::kpzTrue_Value );
    }
    return WT_Result::Success;
}

WT_Result 
IsSideways::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
IsSideways::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Indices::Indices( const Indices& r )
{
    Indices::operator=(r);
}

Indices& Indices::operator=( const Indices& r )
{
    _oString = r._oString;
    return *this;
}

bool Indices::operator==(const Indices &r) const
{
    return _oString == r._oString;
}

WT_Result
Indices::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    _oString = pAttribute;
    return WT_Result::Success;
}

WT_Result 
Indices::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (((const DWFCore::DWFString&)_oString).chars() > 0)
    {
	    pSerializer->addAttribute( XamlXML::kpzIndices_Attribute, _oString );
    }
    return WT_Result::Success;
}

WT_Result 
Indices::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
 }

bool 
Indices::validAsAttribute( void ) const
{
    return true;
}

///////////////////////////////////////////////////////
Resources::Resources( const Resources& r )
{
    Resources::operator=(r);
}

Resources& Resources::operator=( const Resources& r )
{
    _oString = r._oString;
    return *this;
}

bool Resources::operator==(const Resources &r) const
{
    return _oString == r._oString;
}

WT_Result
Resources::materializeAttribute( WT_XAML_File &/*rFile*/, const char* /*pAttribute*/ )
{
    //Resources are never serialized as attributes
    return WT_Result::Internal_Error;
}

WT_Result 
Resources::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *) const
{
    return WT_Result::Toolkit_Usage_Error;
}

WT_Result 
Resources::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer  ) const
{
    if (((const DWFCore::DWFString&)_oString).chars() > 0)
    {
        DWFString zElementName = XamlXML::kpzCanvas_Element;
        zElementName.append( "." );
        zElementName.append( XamlXML::kpzResources_Element );

        pSerializer->startElement( zElementName );
        pSerializer->startElement( XamlXML::kpzResourceDictionary_Element );
        pSerializer->addAttribute( XamlXML::kpzSource_Attribute, _oString );
        pSerializer->endElement();
        pSerializer->endElement();
    }
    return WT_Result::Success;
}

bool 
Resources::validAsAttribute( void ) const
{
    return false;
}

///////////////////////////////////////////////////////
UnicodeString::UnicodeString( const UnicodeString& r )
{
    UnicodeString::operator=(r);
}

UnicodeString& UnicodeString::operator=( const UnicodeString& r )
{
    _oString = r._oString;
    return *this;
}

bool UnicodeString::operator==(const UnicodeString &r) const
{
    return _oString == r._oString;
}

WT_Result
UnicodeString::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    if (pAttribute[0] != '{')
    {
        _oString = pAttribute;
    }
    else
    {
        if (pAttribute[1] == '}')
        {
            _oString = &pAttribute[2];
        }
        else
        {
            WD_Assert( false );
            //Invalid string format.  Begins with "{" but doesn't have the proper prefix "{}"
            return WT_Result::Corrupt_File_Error;
        }
    }

    return WT_Result::Success;
}

WT_Result 
UnicodeString::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    if (((const DWFCore::DWFString&)_oString).chars() > 0)
    {
        const wchar_t* zStr = _oString;
        if (zStr[0] == '{' && zStr[1] != '}')
        {
            DWFString oStr( ((const DWFString&) _oString).chars() + 4 );
            oStr.assign(L"{}");
            oStr.append( (const DWFString&) _oString );
    	    pSerializer->addAttribute( XamlXML::kpzUnicodeString_Attribute, oStr );
        }
        else
        {
    	    pSerializer->addAttribute( XamlXML::kpzUnicodeString_Attribute, _oString );
        }
    }
    return WT_Result::Success;
}

WT_Result 
UnicodeString::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
 }

bool 
UnicodeString::validAsAttribute( void ) const
{
    return true;
}


///////////////////////////////////////////////////////
StyleSimulations::StyleSimulations()
: _eSimulationStyle( StyleSimulations::None )
{ }


StyleSimulations::StyleSimulations( const StyleSimulations& r )
{
    StyleSimulations::operator=(r);
}

StyleSimulations& StyleSimulations::operator=( const StyleSimulations& r )
{
    _eSimulationStyle = r._eSimulationStyle; 
    return *this;
}

bool StyleSimulations::operator==(const StyleSimulations &r) const
{
    return _eSimulationStyle == r._eSimulationStyle;
}

WT_Result
StyleSimulations::materializeAttribute( WT_XAML_File &/*rFile*/, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }

    _eSimulationStyle = None;
    if ( DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzItalicSimulation_Value ) == 0 )
    {
        _eSimulationStyle = Italic;
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzBoldSimulation_Value ) == 0 )
    {
        _eSimulationStyle = Bold;
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( pAttribute, XamlXML::kpzBoldItalicSimulation_Value ) == 0 )
    {
        _eSimulationStyle = BoldItalic;
    }

    return WT_Result::Success;
}

WT_Result 
StyleSimulations::serializeAttribute( WT_XAML_File &, DWFCore::DWFXMLSerializer *pSerializer ) const
{
    switch( _eSimulationStyle )
    {
    case None:
        return WT_Result::Success; // default
    case Italic:
        pSerializer->addAttribute( XamlXML::kpzStyleSimulations_Attribute, XamlXML::kpzItalicSimulation_Value );
        break;
    case Bold:
        pSerializer->addAttribute( XamlXML::kpzStyleSimulations_Attribute, XamlXML::kpzBoldSimulation_Value );
        break;
    case BoldItalic:
        pSerializer->addAttribute( XamlXML::kpzStyleSimulations_Attribute, XamlXML::kpzBoldItalicSimulation_Value );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return WT_Result::Success;
}

WT_Result 
StyleSimulations::serializeElement( WT_XAML_File &, DWFCore::DWFXMLSerializer * ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

bool 
StyleSimulations::validAsAttribute( void ) const
{
    return true;
}
