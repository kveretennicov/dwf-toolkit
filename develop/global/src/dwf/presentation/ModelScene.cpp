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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ModelScene.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/presentation/ModelScene.h"
#include "dwf/package/Constants.h"
#include "dwf/package/Instance.h"
using namespace DWFToolkit;

_DWFTK_API
DWFModelScene::DWFModelScene()
throw()
             : _oSerializableSceneAPI()
             , _bOpenInstance( false )
             , _zOpenInstanceID()
             , _bInstancePreviouslyOpen( false )
             , _zPreviousInstanceID()
{
}

_DWFTK_API
DWFModelScene::~DWFModelScene()
throw()
{
    while (_oSerializableSceneAPI.size() > 0)
    {
        DWFCORE_FREE_OBJECT( _oSerializableSceneAPI.back() );
        _oSerializableSceneAPI.pop_back();
    }
}

_DWFTK_API
void
DWFModelScene::setCurrentInstance( const DWFInstance& rInstance )
throw( DWFException )
{
    this->setCurrentInstanceID( rInstance.id() );
}

_DWFTK_API
void
DWFModelScene::setCurrentInstanceID( const DWFString& zID )
throw( DWFException )
{
        //
        // emit "close instance" scene change
        //
    if (_bOpenInstance)
    {
        _W3DInstance* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstance );
        if (pHandler == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
        }

        pHandler->setObserver( this );
        pHandler->serialize();
        _bOpenInstance = false;
    }

    _W3DInstance* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstance(zID) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    pHandler->serialize();
    _bOpenInstance = true;
}

_DWFTK_API
void
DWFModelScene::lockAttribute( teAttributeType eType )
throw(DWFException)
{
    _W3DAttributeLock* pHandler = DWFCORE_ALLOC_OBJECT( _W3DAttributeLock(eType) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->lock();
    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
void
DWFModelScene::unlockAttribute( teAttributeType eType )
throw(DWFException)
{
    _W3DAttributeLock* pHandler = DWFCORE_ALLOC_OBJECT( _W3DAttributeLock(eType) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->unlock();
    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
void
DWFModelScene::setCamera( const W3DCamera& rCamera, bool bSmoothTransition )
throw( DWFException )
{
    _W3DCamera* pHandler = DWFCORE_ALLOC_OBJECT( _W3DCamera(rCamera, bSmoothTransition) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
TK_Color&
DWFModelScene::getColorHandler()
throw( DWFException )
{
    _W3DColor* pHandler = DWFCORE_ALLOC_OBJECT( _W3DColor );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    return dynamic_cast<TK_Color&>(*pHandler);
}

_DWFTK_API
TK_Cutting_Plane&
DWFModelScene::getCuttingPlaneHandler()
throw( DWFException )
{
    _W3DCuttingPlanes* pHandler = DWFCORE_ALLOC_OBJECT( _W3DCuttingPlanes );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    return dynamic_cast<TK_Cutting_Plane&>(*pHandler);
}

_DWFTK_API
TK_Matrix&
DWFModelScene::getModellingMatrixHandler()
throw( DWFException )
{
    _W3DTransform* pHandler = DWFCORE_ALLOC_OBJECT( _W3DTransform(TKE_Modelling_Matrix) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    return dynamic_cast<TK_Matrix&>(*pHandler);
}

_DWFTK_API
TK_Matrix&
DWFModelScene::getTextureMatrixHandler()
throw( DWFException )
{
    _W3DTransform* pHandler = DWFCORE_ALLOC_OBJECT( _W3DTransform(TKE_Texture_Matrix) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    return dynamic_cast<TK_Matrix&>(*pHandler);
}

_DWFTK_API
TK_Visibility&
DWFModelScene::getVisibilityHandler()
throw( DWFException )
{
    _W3DVisibility* pHandler = DWFCORE_ALLOC_OBJECT( _W3DVisibility );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setObserver( this );
    return dynamic_cast<TK_Visibility&>(*pHandler);
}

_DWFTK_API
void
DWFModelScene::setGeometricVariationIndex( unsigned int nIndex )
throw( DWFException )
{
    _W3DInstanceOptions* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstanceOptions );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->vary( nIndex );
    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
void
DWFModelScene::setVisibility( bool bVisibility )
throw( DWFException )
{
    _W3DInstanceOptions* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstanceOptions );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    (bVisibility == true) ? pHandler->show() : pHandler->hide();

    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
void
DWFModelScene::setTransparency( bool bTransparency )
throw( DWFException )
{
    _W3DInstanceOptions* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstanceOptions );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    (bTransparency == true) ? pHandler->ghost() : pHandler->unghost();

    pHandler->setObserver( this );
    pHandler->serialize();
}

_DWFTK_API
void
DWFModelScene::setDisplayMode( unsigned int nDisplayMode )
throw( DWFException )
{
    _W3DDisplayMode* pHandler = DWFCORE_ALLOC_OBJECT( _W3DDisplayMode(nDisplayMode) );
    if (pHandler == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
    }

    pHandler->setMode(nDisplayMode);
    pHandler->setObserver( this );
    pHandler->serialize();
}


void
DWFModelScene::close()
throw( DWFException )
{
        //
        // emit "close instance" scene change
        //
    if (_bOpenInstance)
    {
        _W3DInstance* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstance );
        if (pHandler == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
        }

        pHandler->setObserver( this );
        pHandler->serialize();
        _bOpenInstance = false;
    }
}

_DWFTK_API
DWFModelSceneChangeHandler::~DWFModelSceneChangeHandler()
    throw()
{
    UserAttribute::tList::Iterator* piAttributes = getUserAttributes();

    for(; piAttributes && piAttributes->valid(); piAttributes->next())
    {
        DWFCORE_FREE_OBJECT( piAttributes->get() );
    }
    
    DWFCORE_FREE_OBJECT( piAttributes );
}

_DWFTK_API
void
DWFModelSceneChangeHandler::addUserAttribute( const DWFString& zName,
                                              const DWFString& zValue,
                                              const DWFString& zNamespace )
throw( DWFException )
{
    if (zNamespace.chars() == 0
        || zName.chars() == 0
        || zValue.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The namespace, attribute and value must all be non null strings" );
    }

    UserAttribute* pAttribute = DWFCORE_ALLOC_OBJECT( UserAttribute( zName, zValue, zNamespace ) );
    _oUserAttributes.insert( pAttribute );
}

#ifndef DWFTK_READ_ONLY
_DWFTK_API
void 
DWFModelScene::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    //
    // always start new element
    //
    rSerializer.startElement( DWFXML::kzElement_ModelScene );

        //
        // write out the scene changes
        //
    size_t iSceneChange = 0;
    for (; iSceneChange < _oSerializableSceneAPI.size();
           iSceneChange++)
    {
        _oSerializableSceneAPI[iSceneChange]->serializeXML( rSerializer, nFlags );
    }

    //
    // close scene
    //
    rSerializer.endElement();
}

_DWFTK_API
void
DWFModelSceneChangeHandler::serializeXML( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
    throw( DWFException )
{
    if (_oUserAttributes.empty())
    {
        return;
    }

    rSerializer.startElement( DWFXML::kzElement_UserAttributes );

    //
    // attributes
    //
    UserAttribute::tList::Iterator* piAttributes = getUserAttributes();

    for(; piAttributes && piAttributes->valid(); piAttributes->next())
    {
        UserAttribute* pAttribute = piAttributes->get();
        rSerializer.addAttribute( pAttribute->name(), pAttribute->value(), pAttribute->nameSpace() );
    }

    DWFCORE_FREE_OBJECT( piAttributes );

    rSerializer.endElement();
}

_DWFTK_API
void 
DWFModelScene::_W3DColor::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Color );
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Mask, GetGeometry() );

        int nChannelMask = GetChannels();

        if (nChannelMask & (1<<TKO_Channel_Gloss))
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Gloss, GetGloss() );
        }

        if (nChannelMask & (1<<TKO_Channel_Index))
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Index, GetIndex() );
        }

        rSerializer.startElement( DWFXML::kzElement_Channels);
        {
            if (nChannelMask & (1<<TKO_Channel_Diffuse))
            {
                rSerializer.startElement( DWFXML::kzElement_Channel);
                {
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"diffuse" );

                    //
                    // Serialize either the name or the rgb, but not both.
                    //
                    char const *pName = GetDiffuseName();
                    if(pName != NULL)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_Name, pName );
                    }
                    else
                    {
                        _serializeChannels( rSerializer, GetDiffuse() );
                    }
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Specular))
            {
                rSerializer.startElement( DWFXML::kzElement_Channel );
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"specular" );                        

                    //
                    // Serialize either the name or the rgb, but not both.
                    //
                    char const *pName = GetSpecularName();
                    if(pName != NULL)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_Name, pName );
                    }
                    else
                    {
                        _serializeChannels( rSerializer, GetSpecular() );
                    }
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Mirror))
            {
                rSerializer.startElement( DWFXML::kzElement_Channel);
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"mirror" );                        

                    //
                    // Serialize either the name or the rgb, but not both.
                    //
                    char const *pName = GetMirrorName();
                    if(pName != NULL)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_Name, pName );
                    }
                    else
                    {
                        _serializeChannels( rSerializer, GetMirror() );
                    }
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Transmission))
            {
                rSerializer.startElement(DWFXML::kzElement_Channel);
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"transmission" );                        

                    //
                    // Serialize either the name or the rgb, but not both.
                    //
                    char const *pName = GetTransmissionName();
                    if(pName != NULL)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_Name, pName );
                    }
                    else
                    {
                        _serializeChannels( rSerializer, GetTransmission() );
                    }
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Emission))
            {
                rSerializer.startElement(DWFXML::kzElement_Channel);
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"emission" );                        

                    //
                    // Serialize either the name or the rgb, but not both.
                    //
                    char const *pName = GetEmissionName();
                    if(pName != NULL)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_Name, pName );
                    }
                    else
                    {
                        _serializeChannels( rSerializer, GetEmission() );
                    }
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Environment))
            {
                rSerializer.startElement(DWFXML::kzElement_Channel);
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"environment" );                        
                    rSerializer.addAttribute( DWFXML::kzAttribute_Name, /*NOXLATE*/GetEnvironmentName() );                       
                }
                rSerializer.endElement();
            }

            if (nChannelMask & (1<<TKO_Channel_Bump))
            {
                rSerializer.startElement(DWFXML::kzElement_Channel);
                {   
                    rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"bump" );                        
                    rSerializer.addAttribute( DWFXML::kzAttribute_Name, /*NOXLATE*/GetBumpName() );                       
                }
                rSerializer.endElement();
            }
        }
        rSerializer.endElement();

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}

inline
void
DWFModelScene::_W3DColor::_serializeChannels( DWFXMLSerializer& rSerializer, const float* anChannels )
{
    rSerializer.addAttribute( DWFXML::kzAttribute_Red, anChannels[0] );

    rSerializer.addAttribute( DWFXML::kzAttribute_Green, anChannels[1] );

    rSerializer.addAttribute( DWFXML::kzAttribute_Blue, anChannels[2] );
}

///
///
_DWFTK_API
void 
DWFModelScene::_W3DCuttingPlanes::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_CuttingPlane);
    {
        int nPlaneCount = GetCount();
        rSerializer.addAttribute( DWFXML::kzAttribute_Count, nPlaneCount );

        rSerializer.startElement( DWFXML::kzElement_Planes);
        {
            const float* anPlanes = GetPlanes();
            for (int i = 0; i < nPlaneCount; i++)
            {
                rSerializer.startElement( DWFXML::kzElement_Plane);
                {
                    rSerializer.addAttribute( DWFXML::kzAttribute_A, anPlanes[i*4] );

                    rSerializer.addAttribute( DWFXML::kzAttribute_B, anPlanes[i*4+1] );

                    rSerializer.addAttribute( DWFXML::kzAttribute_C, anPlanes[i*4+2] );

                    rSerializer.addAttribute( DWFXML::kzAttribute_D, anPlanes[i*4+3] );
                }
                rSerializer.endElement();
            }
        }
        rSerializer.endElement();

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();            
}

///
///
_DWFTK_API
void 
DWFModelScene::_W3DTransform::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( (m_opcode == TKE_Modelling_Matrix? DWFXML::kzElement_ModellingMatrix : DWFXML::kzElement_TextureMatrix) );
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Elements, GetMatrix(), 16 );

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement(); 
}

///
///
_DWFTK_API
void 
DWFModelScene::_W3DVisibility::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Visibility);
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Mask, GetGeometry() );

        rSerializer.addAttribute( DWFXML::kzAttribute_Value, GetValue() );

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();            
}

///
///
_DWFTK_API
void 
DWFModelScene::_W3DCamera::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Camera );
    {
        float anValues[3];
        getPosition(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_PositionX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_PositionY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_PositionZ, anValues[2] );

        getTarget(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_TargetX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_TargetY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_TargetZ, anValues[2] );

        getUpVector(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorZ, anValues[2] );

        getField(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_FieldWidth, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_FieldHeight, anValues[1] );

        DWFString zTempString = getProjection() == W3DCamera::eOrthographic ? /*NOXLATE*/"Orthographic" : /*NOXLATE*/"Perspective";
        rSerializer.addAttribute( DWFXML::kzAttribute_ProjectionType, zTempString );

        rSerializer.addAttribute( DWFXML::kzAttribute_SmoothTransition, _bSmoothTransition ? /*NOXLATE*/L"true" : /*NOXLATE*/L"false" );

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}


///
///
_DWFTK_API
void 
DWFModelScene::_W3DAttributeLock::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( (_bLock ? DWFXML::kzElement_AttributeLock : DWFXML::kzElement_AttributeUnlock) );
    {
        switch (_eAttribute)
        {
        case DWFModelScene::eColor:
            rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"Color" ); ;
            break;
        case DWFModelScene::eCuttingPlane:
            rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"CuttingPlane" ); ;
            break;
        case DWFModelScene::eModellingMatrix:
            rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"ModellingMatrix" ); ;
            break;
        case DWFModelScene::eTextureMatrix:
            rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"TextureMatrix" ); ;
            break;
        case DWFModelScene::eVisibility:
            rSerializer.addAttribute( DWFXML::kzAttribute_Type, /*NOXLATE*/"Visibility" ); ;
            break;

        case eCamera:
        case eInstanceVisibility:
        case eInstanceTransparency:
        case eGeometricVariation:
        case eDisplayMode:
        default:
            //
            //  Do nothing
            //
            break;
        }

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}

///
///
_DWFTK_API
void 
DWFModelScene::_W3DInstance::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // no instance ID means we need to close the open instance
        //
    if (_zID.bytes() == 0)
    {

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
        rSerializer.endElement();
    }
        //
        // start a new instance
        //
    else
    {
        rSerializer.startElement( DWFXML::kzElement_InstanceAttributes );
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );
        }
    }
}
///
///
_DWFTK_API
void 
DWFModelScene::_W3DInstanceOptions::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_nVisible > -1)
    {
        rSerializer.startElement( DWFXML::kzElement_InstanceVisibility );
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_State, ((_nVisible == 1) ? /*NOXLATE*/"on" : /*NOXLATE*/"off") );
        }
        rSerializer.endElement();
    }

    if (_nTransparent > -1)
    {
        rSerializer.startElement( DWFXML::kzElement_InstanceTransparency );
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_State, ((_nTransparent == 1) ? /*NOXLATE*/"on" : /*NOXLATE*/"off") );
        }
        rSerializer.endElement();
    }

    if (_nVariation > -1)
    {
        rSerializer.startElement( DWFXML::kzElement_GeometricVariation );
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Index, _nVariation );
        }
        rSerializer.endElement();
    }

    //
    // serialize the base class element
    // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
    //
    DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
}

_DWFTK_API
void 
DWFModelScene::_W3DDisplayMode::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_DisplayMode );
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Mode, _nDisplayMode == eShaded ? /*NOXLATE*/"Shaded"
            : _nDisplayMode == eEdges ? /*NOXLATE*/"Edges" : /*NOXLATE*/"ShadedWithEdges"); ;

        //
        // serialize the base class element
        // DWFModelSceneChangeHandler serializes elements only, so we do it at the end
        //
        DWFModelSceneChangeHandler::serializeXML( rSerializer, nFlags );
    }
    rSerializer.endElement();
}

#endif
