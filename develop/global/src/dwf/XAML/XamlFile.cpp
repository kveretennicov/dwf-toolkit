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
#include "dwfcore/MIME.h"
#include "XAML/win32/XamlFontUtilImpl.h"


#ifndef DWFTK_XML_PARSER_BUFFER_BYTES
#define DWFTK_XML_PARSER_BUFFER_BYTES   1200 //16384
#endif

const wchar_t* const WT_XAML_File::kpzName_Prefix = L"N";

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

WT_XAML_File::WT_XAML_File() throw(WT_Result)
: WT_File()
, _pXamlStreamOut(NULL)
, _pXamlDictionaryStreamOut(NULL)
, _pW2xStreamOut(NULL)
, _pXamlStreamIn(NULL)
, _pXamlDictionaryStreamIn(NULL)
, _pW2xStreamIn(NULL)
, _pXamlSerializer(NULL)
, _pXamlDictionarySerializer(NULL)
, _pW2xSerializer(NULL)
, _pOpcResourceSerializer(NULL)
, _pOpcResourceMaterializer(NULL)
, _pW2XParser(NULL)
, _pXamlParser(NULL)
, _oDesiredRendition()
, _pRendition(NULL)
, _oObjectList()
, _oNestedObjectStack()
, _oPageTransform()
, _oReversePageTransform()
, _oInnerPageTransform()
, _oPageSize(0.,0.)
, _oTranslation(0.,0.)
, _fYMirror(0.)
, _bValidLayout(false)
, _bTopLevelCanvasWritten(false)
, _oOverpostClip()
, _pDelayedPath(NULL)
, _oCurrentPoint(0.,0.)
, _bIsCurrentlyDumpingDelayedPath(false)
, _bIsUnitsTransformSet(false)
, _bIsRenditionSyncStarted(false)
, _bIsNonNestedCanvasElementStarted(false)
, _bIsObfuscateEmbeddedFonts(true)
, _bIsEndOfDwfSent(false)
, _bSerializingMacroDefinition(false)
, _nNameIndex(0)
, _pwcNameString(NULL)
, _zNamePrefix()
, _szDictionaryResourcePath()
, _pDictionary(NULL)
, _oUUIDProvider()
, _oFontRefNameMap()
, _oMacroMap()
, _oCachedTTFFontMap()
, _pMacroDefinitionFile(NULL)
, _oAvailableBuffers()
{
    _pRendition =  DWFCORE_ALLOC_OBJECT( WT_XAML_Rendition );
    if( !_pRendition )
        throw WT_Result::Out_Of_Memory_Error;   

    set_stream_open_action (&DWFXamlStreamOpen);
    set_stream_close_action (&DWFXamlStreamClose);
    set_stream_write_action(NULL);
    heuristics().set_target_version( toolkit_decimal_revision() );

    _zNamePrefix = generateUniquePrefix();
}

WT_XAML_File::~WT_XAML_File() throw()
{
    //clean up
    DWFCORE_FREE_OBJECT( _pXamlSerializer );
    DWFCORE_FREE_OBJECT( _pXamlDictionarySerializer );
    DWFCORE_FREE_OBJECT( _pW2xSerializer );
    if(_pW2XParser)
        DWFCORE_FREE_OBJECT( _pW2XParser );
    if(_pXamlParser)
        DWFCORE_FREE_OBJECT( _pXamlParser );
    if(_pwcNameString)
        DWFCORE_FREE_MEMORY(_pwcNameString);

    tMemoryBufferMap::iterator mbiter = _oAvailableBuffers.begin();
    for(; mbiter != _oAvailableBuffers.end(); mbiter++)
    {
        DWFCORE_FREE_OBJECT( mbiter->second );
    }

    tStringMap::iterator iter = _oFontRefNameMap.begin();
    for(; iter != _oFontRefNameMap.end(); iter++)
    {
        wchar_t *pFirst  = const_cast< wchar_t* >( iter->first  );
        wchar_t *pSecond = const_cast< wchar_t* >( iter->second );
        DWFCORE_FREE_MEMORY( pFirst );
        DWFCORE_FREE_MEMORY( pSecond );
    }

    tMacroMap::iterator iter2 = _oMacroMap.begin();
    for(; iter2 != _oMacroMap.end(); iter2++)
    {
        WT_XAML_Macro_Definition* pSecond = iter2->second;
        DWFCORE_FREE_OBJECT( pSecond );
    }

    tCachedTTFFontMap::iterator iter3 = _oCachedTTFFontMap.begin();
    for(; iter3 != _oCachedTTFFontMap.end(); iter3++)
    {
        WT_XAML_Font::CachedTTFFontData *pData = iter3->second;
        DWFCORE_FREE_OBJECT( pData );
    }
}


WT_Result WT_XAML_File::open()
{
    // The folowing commented out members are typicaly set in WT_File::open.
    // They canít be set by  WT_XAMLFile::open because they are private members
    // of WT_File. They are set correctly in the constructor, so this should
    // not be a problem.
    //m_number_of_opcodes_read = 0;
    //m_currently_dumping_delayed_drawable = WD_False;
    //m_delayed_drawable = NULL;
    //m_current_point = WT_Logical_Point(0,0);
    set_have_read_first_opcode(false);
    //m_read_count_state = Getting_Count_Byte;
    //m_read_ascii_integer32_state = Eating_Initial_Whitespace;
    //m_read_ascii_double_state = Eating_Initial_Whitespace;
    //m_read_ascii_logical_point_state = Getting_X_Coordinate;
    //m_read_ascii_logical_box_state = Getting_Min_Point;
    //m_read_ascii_color_state = Getting_First_Value;
    //m_read_hex_multiple_state = 0;
    //m_read_hex_state = Eating_Whitespace;

    //m_read_colors_state = 0;
    //m_read_ascii_value = 0;
    //m_read_ascii_sign = 1;

    WD_CHECK (stream_open_action()(*this));

    WT_Result retval = WT_Result::Success;
    if ((file_mode() == File_Write) ||
        (file_mode() == Block_Write))
    {       

        //TODO: take this "main units transform", and serialize out a WT_Units object
        //into the W2X stream only.  This is a transform used by clients to map BACK to
        //real world (model) coordinate space, not from drawing space to XAML space - that
        //should already be done.

        //XamlDrawableAttributes::RenderTransform *pXform = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::RenderTransform);
        //if (pXform == NULL)
        //{
        //    retval = WT_Result::Out_Of_Memory_Error;
        //}

        //WD_CHECK( retval );
        //pXform->matrix() = _oUnitsMainTransform;
        //WD_CHECK( pXform->serializeAttribute( *this, w2xSerializer() ) );
        //DWFCORE_FREE_OBJECT( pXform );

    }
    else
    {
        // We only care about the write case.
        retval = WT_Result::Success;
    }

    return retval;
}


WT_Result WT_XAML_File::close()
{
    WT_Result   result;
    WT_Result   first_error;

    //    FILE *    fp = (FILE *)stream_user_data();
    /*if(!fp)
    return WT_Result::Success;*/

    // Note: this routine should be written such that it can be called at
    // any time.
    // For example, it should be OK to call even after an error has occured,
    // or even if we previously closed the file.
    // This routine may be called from the WT_File destructor.

    //if (m_mode == File_Inactive)
    //    return WT_Result::Success;

    if ((file_mode() == File_Write) || (file_mode() == Block_Append) || (file_mode() == Block_Write))
    {
        first_error = dump_delayed_drawable();        
    }

    result = (m_stream_close_action)(*this);
    if (result != WT_Result::Success && first_error == WT_Result::Success)
        first_error = result;

    if (object() != WD_Null && !heuristics().deferred_delete())
    {
        WT_XAML_Class_Factory oFactory;
        oFactory.Destroy( object() );
        set_current_object( WD_Null );
    }

    result = WT_File::close();

    return first_error;
}

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_XAML_File::merge_or_delay_path(XamlPath const & current)
{
    if (!heuristics().allow_drawable_merging())
    {
        WD_CHECK (dump_delayed_drawable());
        return current.dump(*this);
    }

    if (delayed_path())
    {
        // Attempt to merge the two drawables.
        // If they are merged, they can be delayed further.
        // If not, then we'll dump the old drawable and delay the new one.
        if (delayed_path()->merge(current))
            return WT_Result::Success;
        else
            WD_CHECK (dump_delayed_drawable());
    }

    // Delay this new path by storing a copy of it.
    return current.delay(*this);
}
#else
WT_Result WT_XAML_File::merge_or_delay_path(XamlPath const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_XAML_File::dump_delayed_drawable()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (delayed_path() && !_bIsCurrentlyDumpingDelayedPath)
    {
        _bIsCurrentlyDumpingDelayedPath = true; // Prevents recursion
        WD_CHECK( delayed_path()->dump(*this) );
        _bIsCurrentlyDumpingDelayedPath = false;
        DWFCORE_FREE_OBJECT( _pDelayedPath );
        _pDelayedPath = NULL;
    }

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

WT_Result WT_XAML_File::definePageLayout(const WT_Matrix2D &rTransform, double fWidth, double fHeight, double fScale)
{
    if(fScale == 0. || fHeight == 0. || fWidth == 0.)
    {
        _bValidLayout = false;

        return WT_Result::Toolkit_Usage_Error;
    }

    //
    // if the top-level canvas has already been serialized,
    // return an error since we cannot change the layout
    // anymore
    //
    if(_bTopLevelCanvasWritten == true)
    {
        WD_Assert(false);
        return WT_Result::Toolkit_Usage_Error;
    }

    // Get rotation
    int rotation = 0;

    double x = rTransform(0, 0);
    double y = rTransform(0, 1);

    // We want to see which way the new x,y vector is pointing
    // If pointing 1,0, it's rotated 0 degrees
    // If pointing 0,1, it's rotated 90 degrees CCW
    // If pointing -1,0, it's 180
    // If pointing 0,-1, it's 270 CCW

    // This is a bit tricksy below. What's happening is simple:
    // If x > y and x > -y, then the vector is pointing mostly along
    // the x axis. This is equivalent to x > fabs(y) [which we don't
    // actually test for, as this is the default fRotation = 0.0f case].
    // Think of the Cartesian plane split into four diagonal quads by an X
    // through the origin; we want to find which quad the vector points in.
    // Similar tests can be made for the other quadrants.
    if ( y >= fabs( x ) )
    {
        rotation = 90;
    }
    else if ( -x >= fabs(y) )
    {
        rotation = 180;
    }
    else if ( -y >= fabs(x) )
    {
        rotation = 270;
    }

    //
    // 96 page units per inch
    //
    fScale *= 96.;

    _oPageSize.m_x = fWidth;
    _oPageSize.m_y = fHeight;

    _oPageTransform = rTransform;
    
    // Format transform
    if (rotation == 90 || rotation == 270)
    {
        _oPageTransform(0, 0) = fabs(_oPageTransform(0, 1));
        _oPageTransform(0, 1) = 0;
        _oPageTransform(1, 1) = fabs(_oPageTransform(1, 0));
        _oPageTransform(1, 0) = 0;
    }
    else if (rotation == 180)
    {
        _oPageTransform(0, 0) = -_oPageTransform(0, 0);
        _oPageTransform(1, 1) = -_oPageTransform(1, 1);
    }

    //
    // scale the paper transform
    //
    _oPageTransform *= fScale;

    //
    // eliminate the translation values by introducing an adjustment
    // offset in x and y
    //
    _oPageTransform(2,0) = _oPageTransform(2,1) = 0;

    double fTensor = _oPageTransform(1,0) * _oPageTransform(0,1) - _oPageTransform(0,0) * _oPageTransform(1,1);
    double fX = rTransform(2,0);
    double fY = rTransform(2,1);

    _oTranslation.m_x = (_oPageTransform(1,1) * fX - _oPageTransform(0,1) * fY) / fTensor;
    _oTranslation.m_y = (_oPageTransform(0,0) * fY - _oPageTransform(1,0) * fX) / fTensor;

    // Swap X & Y offset
    if (rotation == 90 || rotation == 270)
    {
        double temp = _oTranslation.m_x;
        _oTranslation.m_x = _oTranslation.m_y;
        _oTranslation.m_y = temp;
    }

    //
    // the mirroring in page space is (y = height - y) and uses
    // the reverse page transform
    //
    _oPageTransform.get_inverse(_oReversePageTransform);

    //
    // the inner transform is just a translation matrix inhibiting the translation
    // induced by the top-level 2D graphics resource page transform
    //
    _oInnerPageTransform(2,0) = _oTranslation.m_x;
    _oInnerPageTransform(2,1) = _oTranslation.m_y;

    _oTranslation.m_x *= fScale;
    _oTranslation.m_y *= fScale;

    // Set original height
    if (rotation == 90 || rotation == 270)
    {
        _fYMirror = fWidth;
    }
    else
    {
        _fYMirror = fHeight;
    }

    _bValidLayout = true;

    return WT_Result::Success;
}

WT_Result WT_XAML_File::writeTopLevelCanvas()
{
    //
    // we do not want to write graphics related content to
    // a file without the proper layout transform/dimensions
    //
    // if the top-level canvas has already been serialized,
    // return an error as well
    //
    if(isLayoutValid() == false ||
        isTopLevelCanvasWritten() == true)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    //
    // we're specifying a RenderTransform on our top-level <Canvas> element, using
    // the inner page transform.
    //
    // this must be completed by a topmost transform which specifies the scaled,
    // upper-left 3x3 submatrix of the graphics 2D resource 4x4 matrix (done by the
    // toolkit).
    //
    // all subsequent viewports will output a <Canvas> without a RenderTransform.
    //
    xamlSerializer()->startElement( XamlXML::kpzCanvas_Element );
    xamlSerializer()->addAttribute( XamlXML::kpzXmlns_Attribute, XamlXML::kpzXaml_Namespace );

    XamlDrawableAttributes::MatrixRef oInnerTransform;
    oInnerTransform = innerPageTransform();
    xamlSerializer()->addAttribute(XamlXML::kpzRenderTransform_Attribute, oInnerTransform);

    XamlDrawableAttributes::Resources oCanvasResources;
    oCanvasResources.sourceRef() = _szDictionaryResourcePath;
    WD_CHECK( oCanvasResources.serializeElement( *this, xamlSerializer() ) );

    //
    //  the canvas has now been written, switch the flag
    //
    _bTopLevelCanvasWritten = true;

    return WT_Result::Success;
}

WT_Result WT_XAML_File::flipPoint(WT_Point2D &r)
{
    WT_Point2D s;

    //
    // offset the vertex with the adjustment vector
    // computed above
    //
    r.m_x = r.m_x - _oTranslation.m_x;
    r.m_y = r.m_y - _oTranslation.m_y;

    _oPageTransform.transform(r, s);

    s.m_y = _fYMirror - s.m_y;

    _oReversePageTransform.transform(s, r);

    return WT_Result::Success;
}

WT_Result WT_XAML_File::unflipPoint(WT_Point2D &r)
{
    WT_Point2D s;

    _oPageTransform.transform(r, s);

    s.m_y = _fYMirror - s.m_y;

    _oReversePageTransform.transform(s, r);

    //
    // offset the vertex with the adjustment vector
    // computed above
    //
    r.m_x = r.m_x + _oTranslation.m_x;
    r.m_y = r.m_y + _oTranslation.m_y;

    return WT_Result::Success;
}

float WT_XAML_File::convertToPaperSpace(float length) const
{
    //
    // the lenght must be converted to page units (x96) for the
    // transformations to be coherent
    //
    WT_Point2D a(0, 0), b(length * 96., 0);
    WT_Point2D c, d;

    _oPageTransform.transform(a, c);
    _oPageTransform.transform(b, d);

    c.m_x = c.m_x - d.m_x;
    c.m_y = c.m_y - d.m_y;

    return (float)sqrt(c.m_x * c.m_x + c.m_y * c.m_y);
}

void WT_XAML_File::addToOverpostClip(const WT_Logical_Box &rBox)
{
    _oOverpostClip.push_back(rBox);
}

bool WT_XAML_File::isFittingInOverpostClip(const WT_Logical_Box &rBox)
{
    vector<WT_Logical_Box>::iterator itor = _oOverpostClip.begin();

    for(; itor != _oOverpostClip.end(); itor++)
    {
        WT_Logical_Box &rClip = *itor;

        if(rBox.m_min.m_x > rClip.m_max.m_x) continue;
        if(rBox.m_max.m_x < rClip.m_min.m_y) continue;
        if(rBox.m_min.m_y > rClip.m_max.m_y) continue;
        if(rBox.m_max.m_y < rClip.m_min.m_y) continue;

        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_XAML_File::get_next_object()
{
    WT_Result result = WT_Result::Success;

    if(w2xParser() == WD_Null)
        return WT_Result::Internal_Error;
    if(xamlParser() == WD_Null)
        return WT_Result::Internal_Error;

    bool bW2xComplete = w2xParser()->isParsingFinished();
    bool bXamlComplete = xamlParser()->isParsingFinished();;

    WT_XAML_Class_Factory oFactory; 

    while(1)
    {
        if (object() != WD_Null && !heuristics().deferred_delete())
        {
            oFactory.Destroy( object() );
            set_current_object( WD_Null );
        }

        if ( object_list().empty() )
        {
            if( !bW2xComplete && ( (result = w2xParser()->parseW2X() ) != WT_Result::Success) )
                return result;

            if( !bXamlComplete && ( (result = xamlParser()->parseXaml() )  != WT_Result::Success) )
                return result;

            if(object_list().size() == 0)
            {
                _nNameIndex++;
            }
        }        

        if(object_list().size())
        {
            //WD_Assert( object_list().front()->materialized() );

            WT_Object *pObject = object_list().front();
            set_current_object( pObject );
            object_list().pop_front();
            if ( nested_object_stack().size() > 0 &&
                nested_object_stack().top() == pObject )
            {
                nested_object_stack().pop();
            }
            break;
        }
        else
        {
            bW2xComplete = w2xParser()->isParsingFinished();
            bXamlComplete = xamlParser()->isParsingFinished();

            if (bW2xComplete && bXamlComplete)
            {
                if (!_bIsEndOfDwfSent)
                {
                    WT_XAML_Class_Factory oCF;
                    WT_End_Of_DWF *pEndOfDwf = oCF.Create_End_Of_DWF();
                    if ( pEndOfDwf == NULL )
                        return WT_Result::Out_Of_Memory_Error;

                    pEndOfDwf->materialized() = WD_True;
                    set_current_object( pEndOfDwf );
                    _bIsEndOfDwfSent = true;
                    return WT_Result::Success;
                }
                return WT_Result::Waiting_For_Data;
            }
        }

    } //while

    return result;
}

// XamlDrawableAttributes::Name::Provider
WT_Result
WT_XAML_File::provideName( XamlDrawableAttributes::Name *& rpName )
{
    if (rpName == NULL)
    {
        rpName = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Name() );
        if (rpName == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    rpName->name() = nameIndexString();

    return WT_Result::Success;
}

int WT_XAML_File::parseNameIndex( const char* pName )
{
    size_t nParseLen = DWFCORE_ASCII_STRING_LENGTH( pName );

    //The client code needs to setup the prefix string so that
    //its length matches the length of the prefix used in the 
    //refName Name attributes in the W2X and XAML, respectively.
    WD_Assert( nParseLen > _zNamePrefix.chars() );
    if ( nParseLen > _zNamePrefix.chars() )
    {
        return atoi( &pName[ _zNamePrefix.chars() ] );
    }

    //ERROR
    return -1;
}

DWFString
WT_XAML_File::generateUniquePrefix() const
{
    //
    // Generate a unique, non-squashed guid.
    //
    DWFCore::DWFUUID oUUID;
    DWFCore::DWFString zUUID = oUUID.next(false);

    //
    // Extract the characters from the guid into a local wchar buffer.
    //
    const wchar_t* zOrigGuidString = (const wchar_t*) zUUID;

    //
    // This will hold the modified string.
    //
    wchar_t zBuf[128]; // enough chars to hold any guid.
    wchar_t zBuf2[256]; // enough chars to hold any two guids

    //
    // Walk the incoming buffer, copying all alphanumeric characters over to the new string.
    // Dash characters are converting to underscores.
    //
    const wchar_t* pIn = zOrigGuidString;
    wchar_t* pOut = zBuf;
    int count = 0;
    while (*pIn != 0)
    {
        if ( (*pIn >= 'a' && *pIn <= 'z')
            || (*pIn >= 'A' && *pIn <= 'Z')
            || (*pIn >= '0' && *pIn <= '9'))
        {
            *pOut = *pIn;
            pOut++;
            count++;
        }
        pIn++;
    }
    *pOut++ = '_';
    *pOut = 0;

    //
    // Make a DWFString.
    // Ensure that it starts with a letter - numbers are not allowable.
    // Append the tweaked guid.
    // Return it.
    //
    if ( (zBuf[0] >= 'a' && zBuf[0] <= 'z') || (zBuf[0] >= 'A' && zBuf[0] <= 'Z') )
    {
        return zBuf;
    }

    DWFCORE_WIDE_STRING_COPY (zBuf2, kpzName_Prefix);
    DWFCORE_WIDE_STRING_CONCATENATE (zBuf2, zBuf );
    return zBuf2;
}

const wchar_t* WT_XAML_File::nameIndexString( )
{    
    size_t nAlloc = 32 + _zNamePrefix.chars();
    if(_pwcNameString == NULL)
    {
        _pwcNameString = DWFCORE_ALLOC_MEMORY( wchar_t, nAlloc);    
        DWFCORE_ZERO_MEMORY( (void *)_pwcNameString, sizeof(wchar_t) * nAlloc );
    }

    _DWFCORE_SWPRINTF(_pwcNameString, nAlloc, L"%ls%d", (const wchar_t*)_zNamePrefix, _nNameIndex);

    return _pwcNameString;
}

const DWFString& WT_XAML_File::nameIndexPrefix() const
{
    return _zNamePrefix;
}

DWFString& WT_XAML_File::nameIndexPrefix()
{
    return _zNamePrefix;
}

WT_Result WT_XAML_File::DWFXamlStreamOpen(WT_File & file)
{
    if (file.stream_user_data())
        return WT_Result::File_Already_Open_Error;

    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);

    if (rXamlFile.isWriteMode()) // write
    {
        if (rXamlFile.xamlStreamOut() == NULL ||
            rXamlFile.w2xStreamOut() == NULL ||
            rXamlFile.opcResourceSerializer() == NULL ||
            rXamlFile.opcResourceMaterializer() == NULL )
        {
            return WT_Result::Toolkit_Usage_Error;
        }

        if (rXamlFile.heuristics().target_version() < REVISION_WHEN_DWFX_FORMAT_BEGINS ||
            rXamlFile.heuristics().target_version() > rXamlFile.toolkit_decimal_revision() )
        {
            return WT_Result::Toolkit_Usage_Error;
        }

        //This block of code fixes the enormous values that are produced via the heuristic transform
        //We have the luxury of knowing the actual height and width, so we can properly translate the
        //drawing space back to the origin.
        WT_Transform oTransform( rXamlFile.heuristics().transform() );
        if ( oTransform.m_translate.m_x == 0 && oTransform.m_translate.m_y == 0 && rXamlFile.heuristics().apply_transform() )
        {
            WT_Logical_Point oTranslate;
            switch(oTransform.rotation())
            {
            case 90:
                oTranslate.m_y = 0x7FFFFFFF - WT_Integer32(floor(0.5 + rXamlFile._oPageSize.m_y));
                break;
            case 180:
                oTranslate.m_x = 0x7FFFFFFF - WT_Integer32(floor(0.5 + rXamlFile._oPageSize.m_x));
                oTranslate.m_y = 0x7FFFFFFF - WT_Integer32(floor(0.5 + rXamlFile._oPageSize.m_y));
                break;
            case 270:
                oTranslate.m_x = 0x7FFFFFFF - WT_Integer32(floor(0.5 + rXamlFile._oPageSize.m_x));
                break;
            default: break;
            }
            oTransform = WT_Transform( oTranslate, oTransform.m_x_scale, oTransform.m_y_scale, oTransform.rotation() );
            rXamlFile.heuristics().transform() = oTransform;
        }
        //TODO: affect the _oTranslate vector, if a huge translation was specified therein via the definePageLayout() method

        rXamlFile._pXamlSerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer(rXamlFile._oUUIDProvider) );
        if (rXamlFile.xamlSerializer() == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        rXamlFile.opcResourceSerializer()->getPartOutputStream(
            L"application/vnd.ms-package.xps-resourcedictionary+xml",
            rXamlFile._szDictionaryResourcePath,
            &rXamlFile._pXamlDictionaryStreamOut);

        rXamlFile._pXamlDictionarySerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer(rXamlFile._oUUIDProvider) );
        if (rXamlFile.xamlDictionarySerializer() == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        rXamlFile._pDictionary = DWFCORE_ALLOC_OBJECT( XamlDictionary(rXamlFile) );
        if(rXamlFile.dictionary() == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        rXamlFile._pW2xSerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer(rXamlFile._oUUIDProvider) );
        if (rXamlFile.w2xSerializer() == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        rXamlFile.xamlSerializer()->attach( *rXamlFile.xamlStreamOut() );
        rXamlFile.xamlDictionarySerializer()->attach( *rXamlFile.xamlDictionaryStreamOut() );
        rXamlFile.w2xSerializer()->attach( *rXamlFile.w2xStreamOut() );

        //start the dictionary
        rXamlFile.xamlDictionarySerializer()->startElement( XamlXML::kpzResourceDictionary_Element );
        rXamlFile.xamlDictionarySerializer()->addAttribute( XamlXML::kpzXmlns_Attribute, XamlXML::kpzXaml_Namespace );
        rXamlFile.xamlDictionarySerializer()->addAttribute( XamlXML::kpzXmlnsX_Attribute, XamlXML::kpzX_Namespace );

        //start the W2X file
        rXamlFile.w2xSerializer()->startElement( XamlXML::kpzW2X_Element );
        rXamlFile.w2xSerializer()->addAttribute( XamlXML::kpzVersion_Major_Attribute, (int)(rXamlFile.heuristics().target_version() / 100) );
        rXamlFile.w2xSerializer()->addAttribute( XamlXML::kpzVersion_Minor_Attribute, (int)(rXamlFile.heuristics().target_version() % 100) );
        rXamlFile.w2xSerializer()->addAttribute( XamlXML::kpzNamePrefix_Attribute, rXamlFile.nameIndexPrefix() );

        file.set_stream_user_data( &file );
    }
    else        //read   = no read at this time
    {
        if (rXamlFile.xamlStreamIn() == NULL ||
            rXamlFile.w2xStreamIn() == NULL ||
            rXamlFile.opcResourceMaterializer() == NULL )
        {
            return WT_Result::Toolkit_Usage_Error;
        }

        rXamlFile._pW2XParser = DWFCORE_ALLOC_OBJECT( WT_XAML_W2X_Parser(rXamlFile) );
        if( !rXamlFile._pW2XParser )
            throw WT_Result::Out_Of_Memory_Error;   

        rXamlFile._pXamlParser = DWFCORE_ALLOC_OBJECT( WT_XAML_Xaml_Parser(rXamlFile) );
        if( !rXamlFile._pXamlParser )
            throw WT_Result::Out_Of_Memory_Error;  

        rXamlFile._oCurrentPoint.m_x = 0.0;
        rXamlFile._oCurrentPoint.m_y = 0.0;
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_File::DWFXamlStreamClose (WT_File & file)
{

    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);

    //XAML
    if (rXamlFile._pXamlSerializer != NULL)
    {
        //Just to check if last viewport started Canvas element
        //needs to be ended.
        rXamlFile.serializeCanvasEndElement( false );

        //
        // in case our top level canvas (the one holding the inner
        // render-transform) was not serialized, do not close
        // the current element (this would crash the serializer)
        //
        if(rXamlFile.isTopLevelCanvasWritten() == true)
        {
            rXamlFile._pXamlSerializer->endElement();       
        }

        rXamlFile._pXamlSerializer->detach();
        DWFCORE_FREE_OBJECT(rXamlFile._pXamlSerializer);
    }

    if (rXamlFile._pXamlStreamOut != NULL)
    {
        rXamlFile._pXamlStreamOut->flush();
        DWFCORE_FREE_OBJECT( rXamlFile._pXamlStreamOut );
    }

    if (rXamlFile._pXamlStreamIn != NULL)
    {
        DWFCORE_FREE_OBJECT( rXamlFile._pXamlStreamIn );
    }

    //DICTIONARY
    if (rXamlFile._pXamlDictionarySerializer != NULL)
    {
        rXamlFile._pXamlDictionarySerializer->endElement();
        rXamlFile._pXamlDictionarySerializer->detach();
        DWFCORE_FREE_OBJECT(rXamlFile._pXamlDictionarySerializer);
        DWFCORE_FREE_OBJECT(rXamlFile._pDictionary);
    }

    if (rXamlFile._pXamlDictionaryStreamOut != NULL)
    {
        rXamlFile._pXamlDictionaryStreamOut->flush();
        DWFCORE_FREE_OBJECT(rXamlFile._pXamlDictionaryStreamOut);
    }        

    if (rXamlFile._pXamlDictionaryStreamIn != NULL)
    {
        DWFCORE_FREE_OBJECT(rXamlFile._pXamlDictionaryStreamIn);
    }        

    //W2X
    if (rXamlFile._pW2xSerializer != NULL)
    {
        WD_CHECK( rXamlFile.serializeRenditionSyncEndElement() );
        rXamlFile._pW2xSerializer->endElement();
        rXamlFile._pW2xSerializer->detach();
        DWFCORE_FREE_OBJECT(rXamlFile._pW2xSerializer);
    }

    if (rXamlFile._pW2xStreamOut != NULL)
    {
        rXamlFile._pW2xStreamOut->flush();
        DWFCORE_FREE_OBJECT(rXamlFile._pW2xStreamOut);
    }

    if (rXamlFile._pW2xStreamIn != NULL)
    {
        DWFCORE_FREE_OBJECT(rXamlFile._pW2xStreamIn);
    }

    //OTHER STUFF
    if (rXamlFile._pRendition != NULL)
    {
        DWFCORE_FREE_OBJECT( rXamlFile._pRendition );
    }

    file.set_stream_user_data( NULL );

    return WT_Result::Success;
}

WT_Result WT_XAML_File::serializeRenditionSyncStartElement()
{
    //nothing to do
    if(isRenditionSyncStarted())
        return WT_Result::Success;

    if (w2xSerializer() != NULL)
    {
        w2xSerializer()->startElement(XamlXML::kpzRenditionSync_Element);
        w2xSerializer()->addAttribute(XamlXML::kpzRefName_Attribute,nameIndexString());
        _bIsRenditionSyncStarted = true;

    }
    else 
        return WT_Result::Internal_Error;

    return WT_Result::Success;
}

WT_Result WT_XAML_File::serializeRenditionSyncEndElement()
{    
    //nothing to do
    if(!isRenditionSyncStarted())
        return WT_Result::Success;

    if (w2xSerializer() != NULL)
    {
        w2xSerializer()->endElement();
        _bIsRenditionSyncStarted = false;
    }
    else 
        return WT_Result::Internal_Error;

    return WT_Result::Success;


}

WT_Result WT_XAML_File::serializeCanvasStartElement( bool bNesting )
{
    if(xamlSerializer() == NULL)
        return WT_Result::Internal_Error;

    if( !bNesting )
    {
        if (_bIsNonNestedCanvasElementStarted)
        {
            serializeCanvasEndElement( bNesting );
        }

        _bIsNonNestedCanvasElementStarted = true;
    }

    xamlSerializer()->startElement(XamlXML::kpzCanvas_Element);

    return WT_Result::Success;
}

WT_Result WT_XAML_File::serializeCanvasEndElement( bool bNesting )
{    
    //nothing to do
    if(!bNesting && !_bIsNonNestedCanvasElementStarted )
        return WT_Result::Success;

    if (xamlSerializer() != NULL)
    {
        xamlSerializer()->endElement();
        if (!bNesting && _bIsNonNestedCanvasElementStarted)
        {
            _bIsNonNestedCanvasElementStarted = false;
        }
    }
    else 
        return WT_Result::Internal_Error;

    return WT_Result::Success;


}

#ifdef WIN32
#include "XAML\win32\XamlFontUtilImpl.h"
#else
//Other platform implementations
#endif

WT_Result WT_XAML_File::calculateDesiredFontStringSize( const WT_String& rString, double &rfWidth, double &rfHeight, double &rfBase, double &rfAscent ) const
{
	
#ifdef WIN32
	XamlFontUtilImpl oFontUtil;
    return oFontUtil.getFontStringWidth( *this, rString, rfWidth, rfHeight, rfBase, rfAscent );
#else
	return WT_Result::Success;
#endif
}


//////////////////////////////////////////////////////////////////////////////////
WT_Point2D 
WT_XAML_File::update_current_point(WT_Point2D const & new_point)
{
    WT_Point2D    delta;

    delta.m_x = new_point.m_x - _oCurrentPoint.m_x;
    delta.m_y = new_point.m_y - _oCurrentPoint.m_y;

    _oCurrentPoint = new_point;

    return delta;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Point2D 
WT_XAML_File::de_update_current_point(WT_Point2D const & delta)
{
    WT_Point2D    new_point;

    new_point.m_x = _oCurrentPoint.m_x + delta.m_x;
    new_point.m_y = _oCurrentPoint.m_y + delta.m_y;

    _oCurrentPoint = new_point;

    return new_point;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Logical_Point 
WT_XAML_File::update_current_point(WT_Logical_Point const & new_point)
{
    WT_Point2D oNew(new_point.m_x, new_point.m_y);
    WT_Point2D oTmp = update_current_point( oNew );
    WT_Logical_Point oReturn((WT_Integer32)(int)floor(0.5+oTmp.m_x), (WT_Integer32)(int)floor(0.5+oTmp.m_y));
    return oReturn;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Logical_Point
WT_XAML_File::de_update_current_point(WT_Logical_Point const & delta)
{
    WT_Point2D oDelta(delta.m_x, delta.m_y);
    WT_Point2D oTmp = de_update_current_point( oDelta );
    WT_Logical_Point oReturn((WT_Integer32)(int)floor(0.5+oTmp.m_x), (WT_Integer32)(int)floor(0.5+oTmp.m_y));
    return oReturn;
}

//////////////////////////////////////////////////////////////////////////////////
const wchar_t*
WT_XAML_File::resolveFontUri( const wchar_t* zFontName ) const
{
    tStringMap::const_iterator iter = _oFontRefNameMap.find( zFontName );
    if (iter != _oFontRefNameMap.end())
    {
        return iter->second;
    }
    else
    {
        //Font needs to be registered via registerFontUri() below
        return NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result
WT_XAML_File::registerFontUri( const wchar_t* zFontName, const wchar_t* zFontUri )
{
    size_t nStrlen = DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS( zFontName );
    wchar_t *pStoreName = DWFCORE_ALLOC_MEMORY( wchar_t, nStrlen+1 );
    if ( pStoreName == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }
    DWFCORE_WIDE_STRING_COPY( pStoreName, zFontName);

    nStrlen = DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS( zFontUri );
    wchar_t* pStoreUri = DWFCORE_ALLOC_MEMORY( wchar_t, nStrlen+1);
    if ( pStoreUri == NULL )
    {
        return WT_Result::Out_Of_Memory_Error;
    }
    DWFCORE_WIDE_STRING_COPY( pStoreUri, zFontUri);

    _oFontRefNameMap.insert( tStringMap::value_type( pStoreName, pStoreUri ) );

    return WT_Result::Success;
}


//////////////////////////////////////////////////////////////////////////////////
WT_Result
WT_XAML_File::registerMacro( const WT_XAML_Macro_Definition *pMacro )
{
    WT_Integer32 nIndex = pMacro->index();
    tMacroMap::iterator iter = _oMacroMap.find( nIndex );
    if (iter != _oMacroMap.end())
    {
        DWFCORE_FREE_OBJECT( iter->second );
        _oMacroMap.erase( iter );
    }

    WT_XAML_Macro_Definition *pCopy = DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Definition( *pMacro ) );
    if (pCopy == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    _oMacroMap.insert( tMacroMap::value_type( nIndex, pCopy ) );
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
bool
WT_XAML_File::findMacro( WT_Integer32 nIndex, WT_XAML_Macro_Definition *& rpMacro ) const
{
    tMacroMap::const_iterator iter = _oMacroMap.find( nIndex );
    if ( iter != _oMacroMap.end() )
    {
        rpMacro = iter->second;
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result
WT_XAML_File::compress_write(int count, void const * data)
{
    if (! serializingAsW2DContent() || w2dContentFile()==NULL )
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return w2dContentFile()->compress_write( count, data );
}


//////////////////////////////////////////////////////////////////////////////////
void
WT_XAML_File::set_materialized( WT_Object* pObject )
{
    pObject->materialized() = WD_True;
    object_list().update_next_incomplete( pObject );
}


//////////////////////////////////////////////////////////////////////////////////
///
///
///  WT_XAML_ObjectList
///
///
//////////////////////////////////////////////////////////////////////////////////

WT_XAML_File::WT_XAML_ObjectList::WT_XAML_ObjectList()
: _pIncompleteObjects (NULL)
, _pIncompleteAttributes (NULL)
, _nObjects(0)
, _bIsDwfHeaderSent(false)
{
    _oObjects.first = _oObjects.second = NULL;
    _oAttributes.first = _oAttributes.second = NULL;
}

WT_XAML_File::WT_XAML_ObjectList::~WT_XAML_ObjectList()
{
    // free up the pool of recycled allocated Nodes
    while ( !_oNodePool.empty())
    {
        DWFCORE_FREE_OBJECT(_oNodePool.top());
        _oNodePool.pop();
    }
}

size_t WT_XAML_File::WT_XAML_ObjectList::size() const
{
    return _nObjects;
}

bool WT_XAML_File::WT_XAML_ObjectList::empty() const
{
    return _nObjects==0;
}

void WT_XAML_File::WT_XAML_ObjectList::insert( WT_Object* pObj ) throw (DWFException)
{
    if (pObj)
    {
        Node **ppHead = NULL;
        Node **ppTail = NULL;
        Node **ppIncomplete = NULL;

        // Depending on the object type we're inserting,
        // we choose one of the two lists.
        if (pObj->object_type() == WT_Object::Attribute)
        {
            ppHead = &_oAttributes.first;
            ppTail = &_oAttributes.second;
            ppIncomplete = &_pIncompleteAttributes;
        }
        else
        {
            ppHead = &_oObjects.first;
            ppTail = &_oObjects.second;
            ppIncomplete = &_pIncompleteObjects;
        }

        // Allocate a new Node, or if we have an unused Node
        // in the pool, we recycle that and use it instead.
        Node *pNode = NULL;
        if (!_oNodePool.empty())
        {
            pNode = _oNodePool.top();
            _oNodePool.pop();
        }
        else
        {
            pNode = DWFCORE_ALLOC_OBJECT( Node );
        }

        if (pNode == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Could not allocate Node" );
        }

        // Initialize our new node, and bump the count
        _nObjects++;
        pNode->init( pObj, ppHead, ppTail );

        // Associate a WT_Object* with our Node*, so we can find the
        // Node later as needed.
        _nodemap.insert( pair<WT_Object*, Node*>(pObj, pNode) );

        // If we have incomplete items in the list
        if (*ppIncomplete != NULL )
        {
            // If the Node references a completed object, we add it just 
            // before the first incomplete entry in our list.  
            if ( pObj->materialized() )
            {
                pNode->add_before( *ppIncomplete );
            }
            else
            {
                // If the object is incomplete, we tack it on after the 
                // last item in the list.
                pNode->set_tail();
            }
        }
        else
        {
            // There are no incomplete items in the list, so
            // we just tack this one on at the end.
            pNode->set_tail();

            // If it's incomplete, let's remember it.
            if (!pObj->materialized() )
            {
                *ppIncomplete = pNode;
            }
        }

        // index by id
        _idmap.insert( pair<WT_Object::WT_ID, WT_Object*>(pObj->object_id(), pObj) );

        // index by type
        _typemap.insert( pair<WT_Object::WT_Type, WT_Object*>(pObj->object_type(), pObj) );
    }
}

void WT_XAML_File::WT_XAML_ObjectList::update_next_incomplete( WT_Object* pObj )
{
    std::map<WT_Object*, Node* >::iterator iter = _nodemap.find( pObj );
    if ( iter != _nodemap.end() )
    {
        Node *pNode = iter->second;
        Node **ppIncomplete = NULL;
        if (pObj->object_type() == WT_Object::Attribute)
        {
            ppIncomplete = &_pIncompleteAttributes;
        }
        else
        {
            ppIncomplete = &_pIncompleteObjects;
        }

        if (*ppIncomplete == pNode)
        {
            do 
            {
                pNode = pNode->next;

                if (pNode && !pNode->object->materialized())
                {
                    break;
                }
            } while( pNode );

            *ppIncomplete = pNode;
        }
    }
    else
    {
        WD_Assert( false );  //we're not tracking it!
    }
}

WT_XAML_File::WT_XAML_ObjectList::Node::Node()
: _head (NULL)
, _tail (NULL)
, prev  (NULL)
, next  (NULL)
, object(NULL)
{;}

void WT_XAML_File::WT_XAML_ObjectList::Node::init( WT_Object *pObject, Node **pHead, Node **pTail)
{
    _head = pHead;
    _tail = pTail;
    prev = next = NULL;
    object = pObject;

    if (*_head == NULL)
    {
        *_head = this;
    }
    if (*_tail == NULL)
    {
        *_tail = this;
    }

}

void WT_XAML_File::WT_XAML_ObjectList::Node::remove()
{
    if (prev != NULL)
    {
        WD_Assert( prev->next != next );
        prev->next = next;
    }

    if (next != NULL)
    {
        WD_Assert( next->prev != prev );
        next->prev = prev;
    }

    if (this == *_head)
    {
        *_head = next;
    }

    if (this == *_tail)
    {
        *_tail = prev;
    }

    WD_Assert( object->materialized() );
}

void WT_XAML_File::WT_XAML_ObjectList::Node::add_before( WT_XAML_File::WT_XAML_ObjectList::Node* pNode )
{
    WD_Assert( pNode != this );

    //hookup the links
    next = pNode;
    if (pNode != NULL)
    {
        prev = pNode->prev;
        pNode->prev = this;
        if (prev != NULL)
        {
            prev->next = this;
        }
    }

    //set head pointer if necessary
    if (pNode == *_head)
    {
        *_head = this;
    }
}

void WT_XAML_File::WT_XAML_ObjectList::Node::set_tail()
{
    if (this == *_tail)
        return;

    prev = *_tail;
    if (*_tail != NULL)
    {
        (*_tail)->next = this;
    }

    *_tail = this;
    if (*_head == NULL)
    {
        *_head = this;
    }
    next = NULL;
}

WT_Object*& WT_XAML_File::WT_XAML_ObjectList::front()
{
    if (_oAttributes.first != NULL && _bIsDwfHeaderSent)
    {
        return _oAttributes.first->object;
    }
    else
    {
        WD_Assert(_oObjects.first != NULL );
        return _oObjects.first->object;
    }
}

void WT_XAML_File::WT_XAML_ObjectList::pop_front()
{
    Node* pNode = NULL;
    Node* pIncomplete = NULL;

    if (_nObjects==0)
        return;

    if (_oAttributes.first != NULL && _bIsDwfHeaderSent)
    {
        pNode = _oAttributes.first;
        pIncomplete = _pIncompleteAttributes;
    }
    else
    {
        WD_Assert(_oObjects.first != NULL );
        pNode = _oObjects.first;
        pIncomplete = _pIncompleteObjects;

        if (!_bIsDwfHeaderSent)
        {
            WD_Assert (_oObjects.first->object->object_id() == WT_Object::DWF_Header_ID);
            _bIsDwfHeaderSent = true;
        }
    }

    _cull( pNode->object );
    if (pNode == pIncomplete)
    {
        update_next_incomplete( pNode->object );
    }

    _nodemap.erase( pNode->object );
    pNode->remove();
    _nObjects--;
    _oNodePool.push(pNode);
}

bool WT_XAML_File::WT_XAML_ObjectList::find_by_id( WT_Object::WT_ID eID,
                                                  std::multimap<WT_Object::WT_ID, WT_Object*>::iterator& rStart,
                                                  std::multimap<WT_Object::WT_ID, WT_Object*>::iterator& rEnd )
{
    //pair<std::multimap<WT_Object::WT_ID, WT_Object*>::iterator, std::multimap<WT_Object::WT_ID, WT_Object*>::iterator> r = _idmap.equal_range( eID );
    rStart = _idmap.lower_bound( eID );
    rEnd = _idmap.upper_bound( eID );
    return (rStart != rEnd);
}

bool WT_XAML_File::WT_XAML_ObjectList::find_by_type( WT_Object::WT_Type eType,
                                                    std::multimap<WT_Object::WT_Type, WT_Object*>::iterator& rStart,
                                                    std::multimap<WT_Object::WT_Type, WT_Object*>::iterator& rEnd )
{
    rStart = _typemap.lower_bound( eType );
    rEnd = _typemap.upper_bound( eType );
    return (rStart != rEnd);
}

// Return the first incomplete object by type
bool WT_XAML_File::WT_XAML_ObjectList::find_first_incomplete( WT_Object::WT_Type eType, WT_Object*& rpObject)
{
    Node* pIncomplete = NULL;
    if (eType==WT_Object::Attribute)
    {
        pIncomplete = _pIncompleteAttributes;
    }
    else
    {
        pIncomplete = _pIncompleteObjects;
    }

    if ( pIncomplete != NULL )
    {
        rpObject = pIncomplete->object;
        return true;
    }
    return false;
}

//Yes, this is a linear search, but it's always a VERY small list
bool WT_XAML_File::WT_XAML_ObjectList::find_first_incomplete( WT_Object::WT_Type eType, WT_Object::WT_ID eID, WT_Object*& rpObject)
{
    Node* pIncomplete = NULL;
    if (eType==WT_Object::Attribute)
    {
        pIncomplete = _pIncompleteAttributes;
    }
    else
    {
        pIncomplete = _pIncompleteObjects;
    }

    while (pIncomplete != NULL && pIncomplete->object != NULL)
    {
        if (pIncomplete->object->object_id() == eID)
        {
            rpObject = pIncomplete->object;
            return true;
        }
        pIncomplete = pIncomplete->next;
    }
    return false;
}

void WT_XAML_File::WT_XAML_ObjectList::_cull( WT_Object* pObj )
{
    // id
    std::multimap<WT_Object::WT_ID, WT_Object* >::iterator iID = _idmap.lower_bound( pObj->object_id() );

    if (iID != _idmap.end())
    {
        std::multimap<WT_Object::WT_ID, WT_Object*>::iterator jID = _idmap.upper_bound( pObj->object_id() );
        for (; iID != jID; iID++)
        {
            if (iID->second == pObj)
            {
                _idmap.erase( iID );
                break;
            }
        }
    }


    // type
    std::multimap<WT_Object::WT_Type, WT_Object*>::iterator iType = _typemap.lower_bound( pObj->object_type() );

    if (iType != _typemap.end())
    {
        std::multimap<WT_Object::WT_Type, WT_Object*>::iterator jType = _typemap.upper_bound( pObj->object_type() );
        for (; iType != jType; iType++)
        {
            if (iType->second == pObj)
            {
                _typemap.erase( iType );
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

WT_XAML_File::WT_XAML_Memory_File::WT_XAML_Memory_File( DWFInputStream *pSourceStream )
: pSourceInputStream( pSourceStream )
, pInputStream(NULL)
, pOutputStream(NULL)
{
    heuristics().set_target_version( REVISION_WHEN_MACRO_IS_SUPPORTED );

    if ( pSourceInputStream != NULL )
    {
        set_stream_user_data( pSourceInputStream );
        set_file_mode( WT_File::File_Read );
    }
    else
    {
        set_file_mode( WT_File::File_Write );
    }
}

WT_XAML_File::WT_XAML_Memory_File::~WT_XAML_Memory_File()
{
    if (pInputStream)
    {
        DWFCORE_FREE_OBJECT( pInputStream );
    }

    if (pOutputStream)
    {
        DWFCORE_FREE_OBJECT( pOutputStream );
    }
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_close (void)
{
    if (pOutputStream)
        pOutputStream->flush();
    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_end_seek (void)
{
    if ( file_mode() == WT_File::File_Read )
    {
        if (pInputStream == NULL)
            return WT_Result::Toolkit_Usage_Error;
        pInputStream->seek( SEEK_END, 0 );
    }
    else
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_open (void)
{
    switch ( file_mode() )
    {
    case WT_File::File_Write:
        pOutputStream = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream( 32768 ) ); //initially 32K
        if (pOutputStream == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        heuristics().set_allow_binary_data( WD_False );
        heuristics().set_allow_data_compression( WD_False );
        heuristics().set_allow_drawable_merging( WD_False );
        break;

    case WT_File::File_Read:

        if (pSourceInputStream == NULL)
        {
            return WT_Result::Toolkit_Usage_Error;
        }
        pInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream( pSourceInputStream, true ) );
        if (pInputStream == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        heuristics().set_deferred_delete( WD_True );

        break;

    default: 
        //Unknown mode
        WD_Assert( false );
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_read (int desired_bytes, int& bytes_read, void* buffer)
{
    if ( file_mode() == WT_File::File_Read )
    {
        if (pInputStream == NULL)
            return WT_Result::Toolkit_Usage_Error;

        bytes_read = (int)pInputStream->read( buffer, desired_bytes );
    }
    else
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_seek (int distance, int& amount_seeked)
{
    if ( file_mode() == WT_File::File_Read )
    {
        if (pInputStream == NULL)
            return WT_Result::Toolkit_Usage_Error;
        int nPrevious = pInputStream->seek( SEEK_CUR, distance );
        int nCurrent = pInputStream->seek( SEEK_CUR, 0 );
        amount_seeked = nCurrent - nPrevious;
    }
    else
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_tell (unsigned long *current_file_pointer_position)
{
    if ( file_mode() == WT_File::File_Read )
    {
        if (pInputStream == NULL)
            return WT_Result::Toolkit_Usage_Error;
        if (current_file_pointer_position == NULL)
            return WT_Result::Toolkit_Usage_Error;

        *current_file_pointer_position = pInputStream->seek( SEEK_CUR, 0 );
    }
    else
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_File::WT_XAML_Memory_File::process_stream_write(int size, void const* buffer)
{
    if ( file_mode() == WT_File::File_Write )
    {
        if (pOutputStream == NULL)
            return WT_Result::Toolkit_Usage_Error;

        pOutputStream->write( buffer, size );
    }
    else
    {
        WD_Assert( false );
        return WT_Result::Toolkit_Usage_Error;
    }

    return WT_Result::Success;
}

tMemoryBuffer* 
WT_XAML_File::getBuffer( size_t nMinLengthInBytes )
throw (DWFException)
{
    tMemoryBuffer *pBuffer = NULL;
    if (_oAvailableBuffers.size() > 0)
    {
        tMemoryBufferMap::iterator iter = _oAvailableBuffers.lower_bound( nMinLengthInBytes );
        if (iter != _oAvailableBuffers.end() )
        {
            pBuffer = iter->second;
            _oAvailableBuffers.erase( iter );
        }
    }

    if ( pBuffer == NULL )
    {
        size_t nBufferLength = max( nMinLengthInBytes, 32768 ); //minimum 32K buffer
        pBuffer = DWFCORE_ALLOC_OBJECT( tMemoryBuffer(nBufferLength) );
        if (pBuffer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Couldn't allocate tMemoryBuffer" );
        }
    }

    WD_Assert( pBuffer != NULL && pBuffer->buffer() && pBuffer->size() >= nMinLengthInBytes);

    if ( pBuffer != NULL )
    {
        pBuffer->init();
    }

    return pBuffer;
}

void
WT_XAML_File::releaseBuffer( tMemoryBuffer *pBuffer )
throw (DWFException)
{
    _oAvailableBuffers.insert( tMemoryBufferMap::value_type( pBuffer->size(), pBuffer ) );
}

tMemoryBuffer* 
WT_XAML_File::exchangeBuffer( tMemoryBuffer* pOldBuffer, size_t nMinLengthInBytes ) 
throw (DWFException)
{
    tMemoryBuffer* pNewBuffer = getBuffer( nMinLengthInBytes );
    pNewBuffer->copy(pOldBuffer);
    releaseBuffer( pOldBuffer );
    return pNewBuffer;
}
