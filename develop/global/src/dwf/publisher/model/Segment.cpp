//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#ifndef DWFTK_READ_ONLY


#include "dwfcore/Timer.h"
#include "dwfcore/Pointer.h"
using namespace DWFCore;

#include "dwf/publisher/model/Segment.h"
#include "dwf/publisher/model/StyleSegment.h"
#include "dwf/publisher/model/IncludeSegment.h"
#include "dwf/publisher/impl/PublishedDefinedObject.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
using namespace DWFToolkit;

#include <iostream>
using namespace std;

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFSegment::kz_User_Options_Node                         = "node";
const char* const DWFSegment::kz_User_Options_Edges                        = "edges";
const char* const DWFSegment::kz_User_Options_Hidden                       = "hidden";
const char* const DWFSegment::kz_User_Options_Transparent                  = "transparent";

const char* const DWFSegment::kz_User_Options_Hidden_Node                  = "node,hidden";
const char* const DWFSegment::kz_User_Options_Transparent_Node             = "node,transparent";

const char* const DWFSegment::kz_User_Options_Node_and_Edges               = "node,edges";
const char* const DWFSegment::kz_User_Options_Hidden_Node_and_Edges        = "node,edges,hidden";
const char* const DWFSegment::kz_User_Options_Transparent_Node_and_Edges   = "node,edges,transparent";
//DNT_End

#endif


unsigned int DWFSegment::_knNextKey = 0;


_DWFTK_API
DWFSegment::DWFSegment( DWFSegmentHandlerBuilder&    rSegmentBuilder,
                        DWFGeometryHandlerBuilder&   rGeometryBuilder,
                        DWFFeatureHandlerBuilder&    rFeatureBuilder,
                        DWFAttributeHandlerBuilder&  rAttributeBuilder,
                        DWFPublishedObject::Factory& rObjectFactory,
                        DWFPublishedObject*          pPublishedParent,
                        bool                         bUsingOldObjectModel )
throw()
          : _rSegmentBuilder( rSegmentBuilder )
          , _rGeometryBuilder( rGeometryBuilder )
          , _rFeatureBuilder( rFeatureBuilder )
          , _rAttributeBuilder( rAttributeBuilder )
          , _rObjectFactory( rObjectFactory )
          , _pPublishedObject( NULL )
          , _pPublishedParent( pPublishedParent )
          , _pLibrary( NULL )
          , _bOpen( false )
          , _bUsingOldObjectModel( bUsingOldObjectModel )
          , _eContentIncludeFlag( eNoContentIncludeCalls )
          , _nKey( 0 )
{
    //
    // use sequential numbers for segment keying
    // rather than unique ticks or guids
    // this greatly improves standard LZH compression
    // further, since we name the segments in the W3D stream
    // with the same strings, we get even better compression
    // if the W3D stream itself is written with compression OFF
    //
    _nKey = DWFCore::AtomicIncrement( (int*)&_knNextKey );
}

_DWFTK_API
DWFSegment::DWFSegment( const DWFSegment& rSegment )
throw( DWFException )
          : _rSegmentBuilder( rSegment._rSegmentBuilder )
          , _rGeometryBuilder( rSegment._rGeometryBuilder )
          , _rFeatureBuilder( rSegment._rFeatureBuilder )
          , _rAttributeBuilder( rSegment._rAttributeBuilder )
          , _rObjectFactory( rSegment._rObjectFactory )
          , _pPublishedObject( rSegment._pPublishedObject )
          , _pPublishedParent( rSegment._pPublishedParent )
          , _pLibrary( NULL )
          , _bOpen( rSegment._bOpen )
          , _bUsingOldObjectModel( rSegment._bUsingOldObjectModel )
          , _eContentIncludeFlag( rSegment._eContentIncludeFlag )
          , _nKey( rSegment._nKey )
{
    if (rSegment._pLibrary)
    {
        _pLibrary = DWFCORE_ALLOC_OBJECT( DWFString(*(rSegment._pLibrary)) );
    }
}

_DWFTK_API
DWFSegment&
DWFSegment::operator=( const DWFSegment& rSegment )
throw( DWFException )
{
    _rSegmentBuilder = rSegment._rSegmentBuilder;
    _rGeometryBuilder = rSegment._rGeometryBuilder;
    _rFeatureBuilder = rSegment._rFeatureBuilder;
    _rAttributeBuilder = rSegment._rAttributeBuilder;
    _rObjectFactory = rSegment._rObjectFactory;
    _pPublishedObject = rSegment._pPublishedObject;
    _pPublishedParent = rSegment._pPublishedParent;
    _bOpen = rSegment._bOpen;
    _bUsingOldObjectModel = rSegment._bUsingOldObjectModel;
    _eContentIncludeFlag = rSegment._eContentIncludeFlag;
    _nKey = rSegment._nKey;

    if (rSegment._pLibrary)
    {
        _pLibrary = DWFCORE_ALLOC_OBJECT( DWFString(*(rSegment._pLibrary)) );
    }
    else
    {
        _pLibrary = NULL;
    }

    return *this;
}

_DWFTK_API
DWFSegment::~DWFSegment()
throw()
{
    if (_pLibrary)
    {
        DWFCORE_FREE_OBJECT( _pLibrary );
    }
}

_DWFTK_API
DWFSegment::tKey
DWFSegment::key() const
throw()
{
    return _nKey;
}

_DWFTK_API
DWFSegment
DWFSegment::openSegment()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return DWFSegment( _rSegmentBuilder,
                       _rGeometryBuilder,
                       _rFeatureBuilder,
                       _rAttributeBuilder,
                       _rObjectFactory,
                       ((_pPublishedObject) ? _pPublishedObject  : _pPublishedParent),
                       _bUsingOldObjectModel );
}

_DWFTK_API
void
DWFSegment::style( DWFStyleSegment& rSegment )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    TK_Referenced_Segment& rHandler = _rSegmentBuilder.getStyleSegmentHandler();

    //
    //
    //
    const DWFString& zName = rSegment.name();

    if (zName.chars() > 0)
    {
        char* pUTF8Name = NULL;
        zName.getUTF8( &pUTF8Name );

        rHandler.SetSegment( pUTF8Name );
        rHandler.serialize();

        DWFCORE_FREE_MEMORY( pUTF8Name );
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Style segments must be named" );
    }
}

_DWFTK_API
void
DWFSegment::include( DWFIncludeSegment& rSegment,
                     bool bGenerateMetadata )
throw( DWFException )
{
    //
    //  This will throw an exception if the call is not allowed, or it will set the call flag
    //
    _testContentIncludeIsAllowed( eIncludeCall );

    _serializeIncludeSegment( rSegment._pPublishedObject );

    if (bGenerateMetadata)
    {
        _updatePublishedObjects( rSegment._pPublishedObject, NULL );
    }
}

_DWFTK_API
void
DWFSegment::include( DWFIncludeSegment& rSegment,
                     const DWFString& rInstanceName )
throw( DWFException )
{
    //
    //  This will throw an exception if the call is not allowed, or it will set the call flag
    //
    _testContentIncludeIsAllowed( eIncludeCall );

    _serializeIncludeSegment( rSegment._pPublishedObject );

    _updatePublishedObjects( rSegment._pPublishedObject, &rInstanceName );
}

_DWFTK_API
void
DWFSegment::include( DWFSegment::tKey nSegmentKey,
                     bool bGenerateMetadata )
throw( DWFException )
{
    //
    //  This will throw an exception if the call is not allowed, or it will set the call flag
    //
    _testContentIncludeIsAllowed( eIncludeCall );

    //
    // locate the desired object and ensure it refers to an include segment
    //
    DWFPublishedObject& rObject = _rObjectFactory.findPublishedObject( nSegmentKey );
    if (rObject.isReferenced() == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The key provided doesn't refer to an IncludeSegment" );
    }

    _serializeIncludeSegment( &rObject );

    if (bGenerateMetadata)
    {
        _updatePublishedObjects( &rObject, NULL );
    }
}

_DWFTK_API
void
DWFSegment::include( DWFSegment::tKey nSegmentKey,
                     const DWFString& rInstanceName )
throw( DWFException )
{
    //
    //  This will throw an exception if the call is not allowed, or it will set the call flag
    //
    _testContentIncludeIsAllowed( eIncludeCall );

    //
    // locate the desired object and ensure it refers to an include segment
    //
    DWFPublishedObject& rObject = _rObjectFactory.findPublishedObject( nSegmentKey );
    if (rObject.isReferenced() == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The key provided doesn't refer to an IncludeSegment" );
    }

    _serializeIncludeSegment( &rObject );

    _updatePublishedObjects( &rObject, &rInstanceName );
}

_DWFTK_API
void 
DWFSegment::hideFromDefaultModel()
throw( DWFException )
{
    DWFPublishedObject* pObject = _pPublishedObject ? _pPublishedObject : _pPublishedParent;

    if (pObject != NULL &&
        pObject->setHideFromDefaultModel( _nKey ))
    {
        return;
    }
    else
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Only named segments, and unnamed segments with includes maybe hidden from the default model." );
    }
}

_DWFTK_API
void
DWFSegment::_serializeIncludeSegment( DWFPublishedObject* pIncludeObject )
throw( DWFException )
{
    //
    // get the segment path from the object
    //
    DWFString zIncludePath( DWFIncludeSegment::kz_Include_Library );

    pIncludeObject->path( zIncludePath );

    //
    //
    //
    size_t nChars = zIncludePath.chars();

    TK_Referenced_Segment& rHandler = _rSegmentBuilder.getIncludeSegmentHandler();

    if (nChars > 0)
    {
        char* pUTF8Name = NULL;
        zIncludePath.getUTF8( &pUTF8Name );

        rHandler.SetSegment( pUTF8Name );
        rHandler.serialize();

        DWFCORE_FREE_MEMORY( pUTF8Name );
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Include segments must be named" );
    }
}


_DWFTK_API
void
DWFSegment::_updatePublishedObjects( DWFPublishedObject* pIncludeObject,
                                     const DWFString* pzInstanceName )
throw( DWFException )
{
    if (pIncludeObject == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The include object pointer should not be null" );
    }

    //
    // add the included segment as a reference
    //
    // named segments will acquire the geomtry of the include
    // segment but not their structure.  this is because the
    // nav tree will end up in a odd looking state with a
    // parent & child essentially being the same segment.
    //
    if (_pPublishedObject == NULL)
    {
        if (_pPublishedParent)
        {
            _pPublishedParent->addReference( pIncludeObject, _nKey, pzInstanceName, false );
        }
            //
            // nothing to attach this reference to
            // so we need to create one...
            //
        else
        {
            //
            // create a published object using this segment key with
            // the name of the inclusion segment
            //
            if (pzInstanceName)
            {
                _pPublishedObject = _rObjectFactory.makePublishedObject( _nKey, *pzInstanceName );
            }
            else
            {
                _pPublishedObject = _rObjectFactory.makePublishedObject( _nKey, pIncludeObject->name() );
            }

            //
            // still need to capture the referenced properties
            //
            _pPublishedObject->addReference( pIncludeObject, _nKey, NULL, true );
        }

        //
        // mark this as an interesting/selectable segment this is required as
        //  with no published object (i.e. the segment was not named) there is no
        // user_options attribute to mark this segment as scene selectable
        //
        TK_User_Options& rOptionsHandler = getUserOptionsHandler();
        rOptionsHandler.SetOptions( kz_User_Options_Node );
        rOptionsHandler.serialize();
    }

        //
        // still need to capture the referenced properties
        //
    else
    {
        _pPublishedObject->addReference( pIncludeObject, _nKey, pzInstanceName, true );
    }
}


_DWFTK_API
void
DWFSegment::open( const DWFString* zName,
                  bool             bPublishedEdges )
throw( DWFException )
{
    if (_bOpen)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment already open" );
    }

        //
        // open named segment
        //
    if (zName)
    {
        open( *zName, bPublishedEdges );
    }
    else
    {
        //
        // write to stream and request this segment be tagged
        //
        const void *keyAsPtr = reinterpret_cast<const void*>((uint64_t)_nKey);
        _rSegmentBuilder.getOpenSegmentHandler().serialize( keyAsPtr );

        //
        // must be set before handlers are used below
        //
        _bOpen = true;

            //
            // this segment will contain published edges
            // so add a reference to a known style segment
            //
        if (bPublishedEdges)
        {
            TK_Referenced_Segment& rEdges = _rSegmentBuilder.getStyleSegmentHandler();
            rEdges.SetSegment( DWFStyleSegment::kz_StyleSegment_PublishedEdges );
            rEdges.serialize();

            //
            // mark this as a published edge segment
            //
            TK_User_Options& rOptionsHandler = getUserOptionsHandler();
            rOptionsHandler.SetOptions( kz_User_Options_Edges );
            rOptionsHandler.serialize();
        }
    }
}

_DWFTK_API
void
DWFSegment::open( const DWFString& zName,
                  bool             bPublishedEdges )
throw( DWFException )
{
    if (_bOpen)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment already open" );
    }

    TK_Open_Segment& rHandler = _rSegmentBuilder.getOpenSegmentHandler();

        //
        // this segment will contain published edges
        // so add a reference to a known style segment
        //
    if (bPublishedEdges)
    {
        TK_Referenced_Segment& rEdges = _rSegmentBuilder.getStyleSegmentHandler();
        rEdges.SetSegment( DWFStyleSegment::kz_StyleSegment_PublishedEdges );
        rEdges.serialize();
    }

    //
    // create a published object keyed to this segment
    // hold onto the new object so we can set properties on it
    //
    _pPublishedObject = _rObjectFactory.makePublishedObject( _nKey, zName );

        //
        // tell the published object who his parent is
        //
    if (_pPublishedParent)
    {
        _pPublishedObject->setParent( _pPublishedParent );
    }

    //
    // "name" segment with the key. The publisher uses this "name" later
    // to retrieve the index of the segment, which is difference from the
    // key.
    //
    char zSegmentID[12] = {0};
    ::sprintf( zSegmentID, /*NOXLATE*/"%lu", _nKey );

        //
        // special case
        //
    if (_pLibrary)
    {
        DWFString zSegmentPath( *_pLibrary );
                  zSegmentPath.append( zSegmentID );

        //
        // abbreviated buffer since this will be a 7-bit ASCII string
        //
        size_t nChars = zSegmentPath.chars() + 1;
        DWFPointer<ASCII_char_t> apBuffer( DWFCORE_ALLOC_MEMORY(ASCII_char_t, nChars), true );
        zSegmentPath.getUTF8( apBuffer, nChars );

        rHandler.SetSegment( (const char*)apBuffer );
    }
    else
    {
        rHandler.SetSegment( zSegmentID );
    }

    //
    // write to stream and request this segment be tagged
    //
    rHandler.serialize( (const void*)(uintptr_t)_nKey );

    //
    // mark as open as soon as the opcode is written
    //
    _bOpen = true;

    //
    // mark this as an interesting/selectable segment
    //
    /*
    const char* zOptions = NULL;

    if (bPublishedEdges)
    {
        zOptions = (eInitialState == eDefault) ? kz_User_Options_Node_and_Edges :
                   (eInitialState == eHidden)  ? kz_User_Options_Hidden_Node_and_Edges :
                                                 kz_User_Options_Transparent_Node_and_Edges;
    }
    else
    {
        zOptions = (eInitialState == eDefault) ? kz_User_Options_Node :
                   (eInitialState == eHidden)  ? kz_User_Options_Hidden_Node :
                                                 kz_User_Options_Transparent_Node;
    }
    */

    TK_User_Options& rOptionsHandler = getUserOptionsHandler();
    rOptionsHandler.SetOptions( bPublishedEdges ? kz_User_Options_Node_and_Edges : kz_User_Options_Node );
    rOptionsHandler.serialize();

}

_DWFTK_API
void
DWFSegment::close()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    _rSegmentBuilder.getCloseSegmentHandler().serialize();
}

_DWFTK_API
void
DWFSegment::setVisibility( bool bVisible )
    throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    //
    //  Set the visibility for all geometry types on this segment
    //
    TK_Visibility& rVisibility = getVisibilityHandler();
    rVisibility.SetGeometry(~TKO_Geo_Extended_Mask);
    rVisibility.SetValue(bVisible);
    rVisibility.serialize();
    return;
}

_DWFTK_API
void
DWFSegment::setContentEntity( DWFEntity*        /*pEntity*/, 
                              const DWFString*  /*pzInstanceName*/ )
throw( DWFException )
{
    //
    //  TODO: Implement or remove before final release of 7.2.0
    //
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not implemented yet." );

    ////
    ////  This will throw an exception if the call is not allowed, or it will set the call flag
    ////
    //_testContentIncludeIsAllowed( eSetEntityCall );

    //if (_pPublishedObject == NULL)
    //{
    //    if (_pPublishedParent)
    //    {
    //        _pPublishedParent->setContentEntity( pEntity, _nKey, pzInstanceName );
    //    }

    //    //
    //    //  If this is provided to a root level unnamed segment
    //    //
    //    else
    //    {
    //        //
    //        //  Create a published object using this segment key with the name of the entity
    //        //  unless a name is provided via pzInstanceName
    //        //
    //        _pPublishedObject = _rObjectFactory.makePublishedObject( _nKey, 
    //                                                                 (pzInstanceName ? *pzInstanceName : pEntity->getLabel()) );
    //        _pPublishedObject->setContentEntity( pEntity, _nKey, pzInstanceName );
    //    }

    //    //
    //    // mark this as an interesting/selectable segment this is required as
    //    //  with no published object (i.e. the segment was not named) there is no
    //    // user_options attribute to mark this segment as scene selectable
    //    //
    //    TK_User_Options& rOptionsHandler = getUserOptionsHandler();
    //    rOptionsHandler.SetOptions( kz_User_Options_Node );
    //    rOptionsHandler.serialize();
    //}
    //else
    //{
    //    _pPublishedParent->setContentEntity( pEntity, _nKey, pzInstanceName );
    //}
}

_DWFTK_API
void
DWFSegment::setContentObject( DWFObject* /*pObject*/ )
throw( DWFException )
{
    //
    //  TODO: Implement or remove before final release of 7.2.0
    //
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not implemented yet." );

    ////
    ////  This will throw an exception if the call is not allowed, or it will set the call flag
    ////
    //_testContentIncludeIsAllowed( eSetObjectCall );

    ////   If the object has a parent, then 
    ////        - if the parent segment has an object, it better be this object's parent.
    ////        - if the parent segment has an entity, then this object's parent must use that entity
    ////          in which case the parent segment's content element should be replaced with this
    ////          object's parent.
    ////        - if the parent segment has an include segment the include's entity must be defined
    ////          and must correspond to this object's parent's entity.
    ////        - if the parent has none of the above then do we set the object's parent as the 
    ////          parent segment's object? It could get set later - but the above restrictions
    ////          require the same end result.
    ////        - if there is no parent segment - OK - might be recreating a fragment of the 
    ////          whole model
    ////    If the objet has no parent, then having a parent segment causes problems because the
    ////          object should not be parent - exception?
    ////
    ////    This precludes you from dealing with object heirarchies where intermediate objects are
    ////    dropped - can't skip generations.

    //if (_pPublishedObject == NULL)
    //{
    //    if (_pPublishedParent)
    //    {
    //        _testSetObjectIsAllowed( pObject );
    //        _pPublishedParent->setContentObject( pObject, _nKey );
    //    }
    //    else
    //    {
    //        //
    //        //  Create a published object using this segment key with the name of the entity
    //        //  unless a name is provided via pzInstanceName
    //        //
    //        _pPublishedObject = _rObjectFactory.makePublishedObject( _nKey, pObject->getLabel() );
    //        _pPublishedObject->setContentObject( pObject, _nKey );
    //    }

    //    //
    //    // mark this as an interesting/selectable segment this is required as
    //    //  with no published object (i.e. the segment was not named) there is no
    //    // user_options attribute to mark this segment as scene selectable
    //    //
    //    TK_User_Options& rOptionsHandler = getUserOptionsHandler();
    //    rOptionsHandler.SetOptions( kz_User_Options_Node );
    //    rOptionsHandler.serialize();
    //}
    //else
    //{
    //    _pPublishedParent->setContentObject( pObject, _nKey );
    //}
}

_DWFTK_API
void 
DWFSegment::setUsingOldObjectModel( bool bUse )
throw()
{
    _bUsingOldObjectModel = bUse;
}

_DWFTK_API
void
DWFSegment::_testContentIncludeIsAllowed( _teContentIncludeCalls eCallType )
throw( DWFException )
{
    //
    //  The content include calls can only occur on open segments.
    //
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    if (!_bUsingOldObjectModel)
    {
        switch (_eContentIncludeFlag)
        {
        case eNoContentIncludeCalls:
            {
                _eContentIncludeFlag = eCallType;
                break;
            }
        case eIncludeCall:
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"This segment already has an include. Only one include per segment is allowed." );
                break;
            }
        case eSetEntityCall:
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"setEntity already called on this segment. The calls include, setEntity and setObject are mutually exclusive." );
                break;
            }
        case eSetObjectCall:
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"setObject already called on this segment. The calls include, setEntity and setObject are mutually exclusive." );
                break;
            }
        default:
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Call type is unknown." );
                break;
            }
        };
    }
}

_DWFTK_API
const DWFProperty* const
DWFSegment::findProperty( const DWFString& zName,
                          const DWFString& zCategory )
const
throw()
{
        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        return _pPublishedObject->findProperty( zName, zCategory );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        return _pPublishedParent->findProperty( zName, zCategory );
    }

    return NULL;
}

_DWFTK_API
DWFProperty::tMap::Iterator*
DWFSegment::getProperties( const DWFString& zCategory )
throw()
{
        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        return _pPublishedObject->getProperties( zCategory );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        return _pPublishedParent->getProperties( zCategory );
    }
    else
    {
        return DWFPropertyContainer::getProperties( zCategory );
    }
}

_DWFTK_API
void
DWFSegment::addProperty( DWFProperty* pProperty )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->addProperty( pProperty, true );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->addProperty( pProperty, true );
    }
        //
        // this means that there are no named segments above this one
        // as such, there is nothing to add this property to
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot add property to non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::addProperty( const DWFProperty& rProperty )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->addProperty( const_cast<DWFProperty*>(&rProperty), false );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->addProperty( const_cast<DWFProperty*>(&rProperty), false );
    }
        //
        // this means that there are no named segments above this one
        // as such, there is nothing to add this property to
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot add property to non-rooted segment" );
    }
}

_DWFTK_API
DWFProperty*
DWFSegment::addProperty( const DWFString& zName,
                         const DWFString& zValue,
                         const DWFString& zCategory,
                         const DWFString& zType,
                         const DWFString& zUnits )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    DWFProperty* pProperty = DWFCORE_ALLOC_OBJECT( DWFProperty( zName, zValue, zCategory, zType, zUnits ) );

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->addProperty( pProperty, true );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->addProperty( pProperty, true );
    }
        //
        // this means that there are no named segments above this one
        // as such, there is nothing to add this property to
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot add property to non-rooted segment" );
    }

    return pProperty;
}


_DWFTK_API
void
DWFSegment::addPropertyContainer( DWFPropertyContainer* pContainer )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->addPropertyContainer( pContainer );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->addPropertyContainer( pContainer );
    }
        //
        // this means that there are no named segments above this one
        // as such, there is nothing to add this property to
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot add property to non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::referencePropertyContainer( const DWFPropertyContainer& rContainer )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->referencePropertyContainer( rContainer );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->referencePropertyContainer( rContainer );
    }
        //
        // this means that there are no named segments above this one
        // as such, there is nothing to add this property to
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot add property to non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::removeOwnedPropertyContainers( DWFPropertyContainer::tList& rContainerList,
                                           bool                         bMakeReferences )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->removeOwnedPropertyContainers( rContainerList, bMakeReferences );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->removeOwnedPropertyContainers( rContainerList, bMakeReferences );
    }
        //
        // this means that there are no named segments above this one
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot remove property from non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::removeReferencedPropertyContainers( DWFPropertyContainer::tList& rContainerList )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->removeReferencedPropertyContainers( rContainerList );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->removeReferencedPropertyContainers( rContainerList );
    }
        //
        // this means that there are no named segments above this one
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot remove property from non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::removeAllPropertyContainers( DWFPropertyContainer::tList& rContainerList )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->removeAllPropertyContainers( rContainerList );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->removeAllPropertyContainers( rContainerList );
    }
        //
        // this means that there are no named segments above this one
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot remove property from non-rooted segment" );
    }
}

_DWFTK_API
void
DWFSegment::copyProperties( DWFPropertyContainer& rContainer,
                            bool                  bRemoveOwnership,
                            bool                  bMakeReferences )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

        //
        // pass along to the published object
        // and ignore the default implementation
        //
    if (_pPublishedObject)
    {
        _pPublishedObject->copyProperties( rContainer, bRemoveOwnership, bMakeReferences );
    }
        //
        // this case for unnamed segments
        //
    else if (_pPublishedParent)
    {
        _pPublishedParent->copyProperties( rContainer, bRemoveOwnership, bMakeReferences );
    }
        //
        // this means that there are no named segments above this one
        //
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot copy property to non-rooted segment" );
    }
}

        //
        //
        // GeometryHandlerBuilder Interface
        //
        //

_DWFTK_API
TK_Area_Light&
DWFSegment::getAreaLightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getAreaLightHandler();
}

_DWFTK_API
TK_Circle&
DWFSegment::getCircleHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCircleHandler();
}

_DWFTK_API
TK_Circle&
DWFSegment::getCircularArcHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCircularArcHandler();
}

_DWFTK_API
TK_Circle&
DWFSegment::getCircularChordHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCircularChordHandler();
}

_DWFTK_API
TK_Clip_Rectangle&
DWFSegment::getClipRectangleHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getClipRectangleHandler();
}

_DWFTK_API
TK_Clip_Region&
DWFSegment::getClipRegionHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getClipRegionHandler();
}

_DWFTK_API
TK_Circle&
DWFSegment::getCircularWedgeHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCircularWedgeHandler();
}

_DWFTK_API
TK_Cutting_Plane&
DWFSegment::getCuttingPlaneHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCuttingPlaneHandler();
}

_DWFTK_API
TK_Cylinder&
DWFSegment::getCylinderHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getCylinderHandler();
}

_DWFTK_API
TK_Point&
DWFSegment::getDistantLightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getDistantLightHandler();
}

_DWFTK_API
TK_Ellipse&
DWFSegment::getEllipseHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getEllipseHandler();
}

_DWFTK_API
TK_Ellipse&
DWFSegment::getEllipticalArcHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getEllipticalArcHandler();
}

_DWFTK_API
TK_Glyph_Definition&
DWFSegment::getGlyphDefinitionHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getGlyphDefinitionHandler();
}

_DWFTK_API
TK_Grid&
DWFSegment::getGridHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getGridHandler();
}

_DWFTK_API
TK_Image&
DWFSegment::getImageHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getImageHandler();
}

_DWFTK_API
TK_Line&
DWFSegment::getLineHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getLineHandler();
}

_DWFTK_API
TK_Point&
DWFSegment::getLocalLightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getLocalLightHandler();
}

_DWFTK_API
TK_Point&
DWFSegment::getMarkerHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getMarkerHandler();
}

_DWFTK_API
TK_Mesh&
DWFSegment::getMeshHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getMeshHandler();
}

_DWFTK_API
TK_NURBS_Curve&
DWFSegment::getNURBSCurveHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getNURBSCurveHandler();
}


_DWFTK_API
TK_NURBS_Surface&
DWFSegment::getNURBSSurfaceHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getNURBSSurfaceHandler();
}

_DWFTK_API
TK_PolyCylinder&
DWFSegment::getPolyCylinderHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getPolyCylinderHandler();
}

_DWFTK_API
TK_Polypoint&
DWFSegment::getPolygonHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getPolygonHandler();
}

_DWFTK_API
TK_Polypoint&
DWFSegment::getPolylineHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getPolylineHandler();
}

_DWFTK_API
TK_Shell&
DWFSegment::getShellHandler( bool bTriStripsOnly,
                             bool bDisableOptimization )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getShellHandler( bTriStripsOnly, bDisableOptimization );
}

_DWFTK_API
TK_Sphere&
DWFSegment::getSphereHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getSphereHandler();
}

_DWFTK_API
TK_Spot_Light&
DWFSegment::getSpotLightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getSpotLightHandler();
}

_DWFTK_API
TK_Text&
DWFSegment::getTextHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getTextHandler();
}

_DWFTK_API
TK_Text&
DWFSegment::getTextWithEncodingHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rGeometryBuilder.getTextWithEncodingHandler();
}

        //
        //
        // FeatureHandlerBuilder Interface
        //
        //

_DWFTK_API
TK_Texture&
DWFSegment::getTextureHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rFeatureBuilder.getTextureHandler();
}

_DWFTK_API
TK_Matrix&
DWFSegment::getTextureMatrixHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rFeatureBuilder.getTextureMatrixHandler();
}
        //
        //
        // AttributeHandlerBuilder Interface
        //
        //

_DWFTK_API
TK_Camera&
DWFSegment::getCameraHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getCameraHandler();
}

_DWFTK_API
TK_Color&
DWFSegment::getColorHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorHandler();
}

_DWFTK_API
TK_Color_Map&
DWFSegment::getColorMapHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorMapHandler();
}

_DWFTK_API
TK_Color_RGB&
DWFSegment::getColorRGBHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorRGBHandler();
}

_DWFTK_API
TK_Named&
DWFSegment::getEdgePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getEdgePatternHandler();
}

_DWFTK_API
TK_Size&
DWFSegment::getEdgeWeightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getEdgeWeightHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getFacePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getFacePatternHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getHandednessHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getHandednessHandler();
}

_DWFTK_API
TK_Heuristics&
DWFSegment::getHeuristicsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getHeuristicsHandler();
}

_DWFTK_API
TK_Named&
DWFSegment::getLinePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLinePatternHandler();
}

_DWFTK_API
TK_Line_Style&
DWFSegment::getLineStyleHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLineStyleHandler();
}

_DWFTK_API
TK_Size&
DWFSegment::getLineWeightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLineWeightHandler();
}

_DWFTK_API
TK_Size&
DWFSegment::getMarkerSizeHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getMarkerSizeHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getMarkerSymbolHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getMarkerSymbolHandler();
}

_DWFTK_API
TK_Matrix&
DWFSegment::getModellingMatrixHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getModellingMatrixHandler();
}

_DWFTK_API
TK_Rendering_Options&
DWFSegment::getRenderingOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getRenderingOptionsHandler();
}

_DWFTK_API
TK_Selectability&
DWFSegment::getSelectabilityHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getSelectabilityHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getTextAlignmentHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextAlignmentHandler();
}

_DWFTK_API
TK_Text_Font&
DWFSegment::getTextFontHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextFontHandler();
}

_DWFTK_API
TK_Point&
DWFSegment::getTextPathHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextPathHandler();
}

_DWFTK_API
TK_Size&
DWFSegment::getTextSpacingHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextSpacingHandler();
}

_DWFTK_API
TK_User_Options&
DWFSegment::getUserOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getUserOptionsHandler();
}

_DWFTK_API
TK_Unicode_Options&
DWFSegment::getUnicodeOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getUnicodeOptionsHandler();
}

_DWFTK_API
TK_Visibility&
DWFSegment::getVisibilityHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getVisibilityHandler();
}

_DWFTK_API
TK_Window&
DWFSegment::getWindowHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getWindowFrameHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowFrameHandler();
}

_DWFTK_API
TK_Enumerated&
DWFSegment::getWindowPatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowPatternHandler();
}


_DWFTK_API
void
DWFSegment::openLocalLightAttributes()
    throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    _rAttributeBuilder.openLocalLightAttributes();
}

_DWFTK_API
void
DWFSegment::closeLocalLightAttributes()
    throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    _rAttributeBuilder.closeLocalLightAttributes();
}

#endif

