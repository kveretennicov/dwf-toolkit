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


#include "dwf/package/Constants.h"
#include "dwf/presentation/reader/ContentPresentationReader.h"
using namespace DWFToolkit;



_DWFTK_API
DWFContentPresentationReader::DWFContentPresentationReader( DWFPackageReader* pPackageReader,
                                                            unsigned char     nProviderFlags )
throw()
                             : _pPackageReader( pPackageReader )
                             , _oDefaultElementBuilder()
                             , _pElementBuilder( &_oDefaultElementBuilder )
                             , _nProviderFlags( nProviderFlags )
                             , _nCurrentCollectionProvider( 0 )
                             , _pReaderFilter( NULL )
                             , _pCurrentPresentation( NULL )
                             , _pCurrentPresentationView( NULL )
                             , _pCurrentPropertyReference( NULL )
                             , _pCurrentNodeContainer( NULL )
                             , _pCurrentNode( NULL )
                             , _nCurrentCuttingPlanesCount( 0 )
                             , _pCurrentCuttingPlanes( NULL )
                             , _pCurrentCuttingPlanesInteriorPointer( NULL )
                             , _pCurrentHandler( NULL )
{
    ;
}

_DWFTK_API
DWFContentPresentationReader::~DWFContentPresentationReader()
throw()
{
    ;
}

_DWFTK_API
DWFContentPresentationReader*
DWFContentPresentationReader::filter() const
throw()
{
    return _pReaderFilter;
}

_DWFTK_API
void
DWFContentPresentationReader::setFilter( DWFContentPresentationReader* pFilter )
throw()
{
    _pReaderFilter = pFilter;
}
    
_DWFTK_API
double
DWFContentPresentationReader::provideVersion( double nDecimalVersion )
throw()
{
    return nDecimalVersion;
}

_DWFTK_API
DWFContentPresentation*
DWFContentPresentationReader::providePresentation( DWFContentPresentation* pPresentation )
throw()
{
    return pPresentation;
}

_DWFTK_API
DWFContentPresentationView*
DWFContentPresentationReader::provideView( DWFContentPresentationView* pView )
throw()
{
    return pView;
}

_DWFTK_API
DWFPropertyReference*
DWFContentPresentationReader::providePropertyReference( DWFPropertyReference* pPropertyReference )
throw()
{
    return pPropertyReference;
}

_DWFTK_API
DWFContentPresentationNode*
DWFContentPresentationReader::provideNode( DWFContentPresentationNode* pNode )
throw()
{
    return pNode;
}

_DWFTK_API
DWFContentPresentationReferenceNode*
DWFContentPresentationReader::provideReferenceNode( DWFContentPresentationReferenceNode* pReferenceNode )
throw()
{
    return pReferenceNode;
}

_DWFTK_API
DWFContentPresentationModelViewNode*
DWFContentPresentationReader::provideModelViewNode( DWFContentPresentationModelViewNode* pModelViewNode )
throw()
{
    return pModelViewNode;
}

_DWFTK_API
void
DWFContentPresentationReader::notifyStartElement( const char*   zName,
                                                  const char**  ppAttributeList )
throw()
{

    switch (_nElementDepth)
    {
            //
            // Expected values:
            //  Content presentation resource
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentation) == 0)               
            {
                    //
                    // provide the version
                    //
                if (_nProviderFlags & DWFContentPresentationReader::eProvideVersion)
                {
                    const char* pVersion = "0";
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Version, pVersion);
                    _provideVersion( DWFString::StringToDouble(pVersion) );
                }
            }
            break;
        }

            //
            // Expected values:
            //  Presentations
            //
        case 1:
        {
            if ((_nProviderFlags & eProvidePresentations) &&
                DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Presentations) == 0)
            {
                ;
            }
            break;
        }

            //
            //  Presentation
            //
        case 2:
        {
            if ((_nProviderFlags & eProvidePresentations)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Presentation) == 0)
            {

                //
                // Create a presentation object.
                //
                _pCurrentPresentation = _pElementBuilder->buildContentPresentation( ppAttributeList );

            }
            break;
        }

            //
            // Expected values:
            //  Views
            //
        case 3:
        {
            if ((_nProviderFlags & eProvideViews)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Views) == 0)
            {
                // nothing to do
            }
            break;
        }

            //
            // Expected values:
            //  View
            //
        case 4:
        {
            if ((_nProviderFlags & eProvideViews)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_View) == 0)
            {
                if(_pCurrentPresentation)
                {
                    _pCurrentPresentationView = _pElementBuilder->buildContentPresentationView( ppAttributeList );
                    if (_pCurrentPresentationView)
                    {
                        //
                        // establish this view as the current recipient of child nodes
                        //
                        _pCurrentNodeContainer = dynamic_cast<DWFContentPresentationNodeContainer*>( _pCurrentPresentationView );
                        _oNodeContainerStack.push_back(_pCurrentNodeContainer);
                    }
                }
            }
            break;
        }

            //
            // Below level 4, things can be a jumble.
            //
            // Expected values:
            //  PropertyReferences - from the view
            //    - PropertyReference
            //  Nodes
            //    - Node
            //    - ReferenceNode
            //      - PropertyReferenceURIs
            //        - PropertyReferenceURI
            //    - ModelViewNode
            //      - a ton of stuff
            //
        default:
        {
            if ((_nProviderFlags & eProvidePropertyReferences)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PropertyReferences) == 0)
            {
                // todo: sanity check?
            }
            else
            if ((_nProviderFlags & eProvidePropertyReferences)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PropertyReference) == 0)
            {
                _pCurrentPropertyReference = _pElementBuilder->buildPropertyReference( ppAttributeList );
            }
            else
            if ((_nProviderFlags & eProvideNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Nodes) == 0)
            {
                // nothing to do
            }
            else
            if ((_nProviderFlags & eProvideNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Node) == 0)
            {
                if( _pCurrentNodeContainer )
                {
                    //
                    // Create a new Node object, and add it to the active node container.
                    //
                    _pCurrentNode = _pElementBuilder->buildContentPresentationNode( ppAttributeList );

                    // establish this node as the current recipient of child nodes
                    _pCurrentNodeContainer = dynamic_cast<DWFContentPresentationNodeContainer*>( _pCurrentNode );
                    _oNodeContainerStack.push_back(_pCurrentNodeContainer);
                }
                // else, what?
            }
            else
            if ((_nProviderFlags & eProvideRefereceNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ReferenceNode) == 0)
            {
                //
                // Remember, ReferenceNode is derived from Node. Anything
                // that it does, we must also do.
                //

                DWFContentPresentationReferenceNode *pRefNode = _pElementBuilder->buildContentPresentationReferenceNode( ppAttributeList );

                _pCurrentNode = pRefNode;

                // establish this node as the current recipient of child nodes
                _pCurrentNodeContainer = dynamic_cast<DWFContentPresentationNodeContainer*>( _pCurrentNode );
                _oNodeContainerStack.push_back(_pCurrentNodeContainer);
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PropertyReferenceURIs) == 0)
            {
                // nothing to do
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PropertyReferenceURI) == 0)
            {
                DWFContentPresentationReferenceNode *pRefNode = dynamic_cast<DWFContentPresentationReferenceNode *>(_pCurrentNode);
                if(pRefNode != NULL)
                {
                    const char* pId = NULL;
                    const char* pURI = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_PropertyReferenceID, pId);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_URI, pURI);

                    if((pId != NULL) && (pURI != NULL))
                    {
                        pRefNode->setPropertyReferenceURI( pId, pURI);
                    }
                }
            }
            else
            if ((_nProviderFlags & eProvideModelViewNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModelViewNode) == 0)
            {
                //
                // Remember, ModelViewNode is derived from ReferenceNode.
                // Anything that it does, we must also do.
                //

                //
                // Create a new Presentation Model View Node object.
                //
                DWFContentPresentationModelViewNode *pModelViewNode = _pElementBuilder->buildContentPresentationModelViewNode( ppAttributeList );

                //
                // Add it to the active node container.
                //
                _pCurrentNode = pModelViewNode;

                // establish this node as the current recipient of child nodes
                _pCurrentNodeContainer = dynamic_cast<DWFContentPresentationNodeContainer*>( _pCurrentNode );
                _oNodeContainerStack.push_back(_pCurrentNodeContainer);
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModelScene) == 0)
            {
                // no attrs
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Camera) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pPositionX = NULL;
                    const char* pPositionY = "";
                    const char* pPositionZ = "";
                    const char* pTargetX = "";
                    const char* pTargetY = "";
                    const char* pTargetZ = "";
                    const char* pUpVectorX = "";
                    const char* pUpVectorY = "";
                    const char* pUpVectorZ = "";
                    const char* pFieldWidth = "";
                    const char* pFieldHeight = "";
                    const char* pProjectionType = "";
                    const char* pSmoothTransition = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_PositionX, pPositionX);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_PositionY, pPositionY);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_PositionZ, pPositionZ);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_TargetX, pTargetX);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_TargetY, pTargetY);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_TargetZ, pTargetZ);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_UpVectorX, pUpVectorX);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_UpVectorY, pUpVectorY);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_UpVectorZ, pUpVectorZ);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_FieldWidth, pFieldWidth);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_FieldHeight, pFieldHeight);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_ProjectionType, pProjectionType);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_SmoothTransition, pSmoothTransition);

                    if(pPositionX != NULL)
                    {
                        W3DCamera::teProjection eProjectionType = W3DCamera::eOrthographic;
                        if(DWFCORE_COMPARE_ASCII_STRINGS(pProjectionType, /*NOXLATE*/"Perspective") == 0)
                        {
                            eProjectionType = W3DCamera::ePerspective;
                        }
                        W3DCamera newCam(
                            (float) DWFString::StringToDouble(pPositionX),
                            (float) DWFString::StringToDouble(pPositionY),
                            (float) DWFString::StringToDouble(pPositionZ),
                            (float) DWFString::StringToDouble(pTargetX),
                            (float) DWFString::StringToDouble(pTargetY),
                            (float) DWFString::StringToDouble(pTargetZ),
                            (float) DWFString::StringToDouble(pUpVectorX),
                            (float) DWFString::StringToDouble(pUpVectorY),
                            (float) DWFString::StringToDouble(pUpVectorZ),
                            (float) DWFString::StringToDouble(pFieldWidth),
                            (float) DWFString::StringToDouble(pFieldHeight),
                            eProjectionType
                        );
                        bool bSmoothTransition = false;
                        if(DWFCORE_COMPARE_ASCII_STRINGS(pSmoothTransition, /*NOXLATE*/"true") == 0)
                        {
                            bSmoothTransition = true;
                        }
                        pModelViewNode->setCamera(newCam, bSmoothTransition);
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceAttributes) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pId = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_ID, pId);

                    pModelViewNode->setCurrentInstanceID(pId);
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModellingMatrix) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pElements = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Elements, pElements);

                    if(pElements != NULL)
                    {
                        float anTransform[16];
                        int i;
                        char* pElementsWithConstRemoved = (char*)pElements;
                        char* pSavePtr;
                        char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pElementsWithConstRemoved, /*NOXLATE*/" ", &pSavePtr );
                        for(i=0; i<16; i++)
                        {
                            anTransform[i] = (float) DWFString::StringToDouble(pToken);
                            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                        }
                        TK_Matrix& rMatrix = pModelViewNode->getModellingMatrixHandler();
                        _pCurrentHandler = dynamic_cast<DWFModelSceneChangeHandler*>(&rMatrix);
                        rMatrix.SetMatrix( anTransform );
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_TextureMatrix) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pElements = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Elements, pElements);

                    if(pElements != NULL)
                    {
                        float anTransform[16];
                        int i;
                        char* pElementsWithConstRemoved = (char*)pElements;
                        char* pSavePtr;
                        char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pElementsWithConstRemoved, /*NOXLATE*/" ", &pSavePtr );
                        for(i=0; i<16; i++)
                        {
                            anTransform[i] = (float) DWFString::StringToDouble(pToken);
                            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                        }
                        TK_Matrix& rMatrix = pModelViewNode->getTextureMatrixHandler();
                        _pCurrentHandler = dynamic_cast<DWFModelSceneChangeHandler*>(&rMatrix);
                        rMatrix.SetMatrix( anTransform );
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Visibility) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pMask = NULL;
                    const char* pValue = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Mask, pMask);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Value, pValue);

                    if( (pMask!=NULL) && (pValue!=NULL) )
                    {
                        TK_Visibility &visHandler = pModelViewNode->getVisibilityHandler();
                        _pCurrentHandler = dynamic_cast<DWFModelSceneChangeHandler*>(&visHandler);
                        visHandler.SetGeometry(::atoi(pMask));
                        visHandler.SetValue(::atoi(pValue));
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceVisibility) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pState = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_State, pState);

                    bool on = (DWFCORE_COMPARE_ASCII_STRINGS(pState, /*NOXLATE*/"on") == 0);
                    pModelViewNode->setVisibility(on);
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceTransparency) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pState = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_State, pState);

                    bool on = (DWFCORE_COMPARE_ASCII_STRINGS(pState, /*NOXLATE*/"on") == 0);
                    pModelViewNode->setTransparency(on);
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GeometricVariation) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pIndex = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Index, pIndex);

                    if(pIndex != NULL)
                    {
                        pModelViewNode->setGeometricVariationIndex(::atoi(pIndex));
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Color) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pMask = NULL;
                    const char* pGloss = NULL;
                    const char* pIndex = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Mask, pMask);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Gloss, pGloss);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Index, pIndex);

                    TK_Color& rColorHandler = pModelViewNode->getColorHandler();
                    _pCurrentHandler = dynamic_cast<DWFModelSceneChangeHandler*>(&rColorHandler);

                    if(pMask != NULL)
                    {
                        rColorHandler.SetGeometry(::atoi(pMask));
                    }

                    if(pGloss != NULL)
                    {
                        rColorHandler.SetGloss((float) DWFString::StringToDouble(pGloss));
                    }

                    if(pIndex != NULL)
                    {
                        rColorHandler.SetIndex((float) DWFString::StringToDouble(pIndex));
                    }

                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Channels) == 0)
            {
                // no attrs
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Channel) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {

                    const char* pType = NULL;

                    if (_pCurrentHandler != NULL)
                    {
                        TK_Color& rColorHandler = dynamic_cast<TK_Color&>(*_pCurrentHandler);
                        _getAttribute(ppAttributeList, DWFXML::kzAttribute_Type, pType);

                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"environment") == 0)
                        {
                            const char* pMapName = NULL;

                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Name, pMapName);
                            if(pMapName != NULL)
                            {
                                rColorHandler.SetEnvironmentName(pMapName);
                            }
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"bump") == 0)
                        {
                            const char* pMapName = NULL;

                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Name, pMapName);
                            if(pMapName != NULL)
                            {
                                rColorHandler.SetBumpName(pMapName);
                            }
                        }
                        else
                        {
                            //
                            // First, look for R,G,B
                            //
                            const char* pRed   = NULL;
                            const char* pGreen = NULL;
                            const char* pBlue  = NULL;

                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Red, pRed);
                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Green, pGreen);
                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Blue, pBlue);

                            if( (pRed != NULL) && (pGreen != NULL) && (pBlue != NULL) )
                            {
                                float red   = (float) DWFString::StringToDouble(pRed);
                                float green = (float) DWFString::StringToDouble(pGreen);
                                float blue  = (float) DWFString::StringToDouble(pBlue);

                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"diffuse") == 0)
                                {
                                    rColorHandler.SetDiffuse(red, green, blue);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"specular") == 0)
                                {
                                    rColorHandler.SetSpecular(red, green, blue);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"mirror") == 0)
                                {
                                    rColorHandler.SetMirror(red, green, blue);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"transmission") == 0)
                                {
                                    rColorHandler.SetTransmission(red, green, blue);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"emission") == 0)
                                {
                                    rColorHandler.SetEmission(red, green, blue);
                                }
                            }


                            //
                            // Then look for color name.
                            //
                            const char* pName = NULL;
                            _getAttribute(ppAttributeList, DWFXML::kzAttribute_Name, pName);
                            if( pName != NULL )
                            {
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"diffuse") == 0)
                                {
                                    rColorHandler.SetDiffuseName(pName);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"specular") == 0)
                                {
                                    rColorHandler.SetSpecularName(pName);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"mirror") == 0)
                                {
                                    rColorHandler.SetMirrorName(pName);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"transmission") == 0)
                                {
                                    rColorHandler.SetTransmissionName(pName);
                                }
                                else
                                if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"emission") == 0)
                                {
                                    rColorHandler.SetEmissionName(pName);
                                }
                            }
                        }
                    }                
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CuttingPlane) == 0)
            {
                //
                // Note that the grammer of cutting planes is such that, unlike everything else,
                // we must wait until the end of kzElement_CuttingPlane is seen before we can
                // make the calls on the model-scene api.
                //

                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pCount = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Count, pCount);

                    if(_pCurrentCuttingPlanes != NULL)
                    {
                        // Major boo boo.
                        delete [] _pCurrentCuttingPlanes;
                    }

                    int count = ::atoi(pCount);
                    if(count > 0)
                    {
                        _nCurrentCuttingPlanesCount = count;
                        _pCurrentCuttingPlanes = new float[count*4];
                        _pCurrentCuttingPlanesInteriorPointer = _pCurrentCuttingPlanes;
                    }

                    TK_Cutting_Plane& rCuttingPlaneHandler = pModelViewNode->getCuttingPlaneHandler();
                    _pCurrentHandler = dynamic_cast<DWFModelSceneChangeHandler*>(&rCuttingPlaneHandler);
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Planes) == 0)
            {
                // no attrs
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Plane) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pA = NULL;
                    const char* pB = "";
                    const char* pC = "";
                    const char* pD = "";

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_A, pA);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_B, pB);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_C, pC);
                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_D, pD);

                    if( pA != NULL )
                    {
                        float A = (float) DWFString::StringToDouble(pA);
                        float B = (float) DWFString::StringToDouble(pB);
                        float C = (float) DWFString::StringToDouble(pC);
                        float D = (float) DWFString::StringToDouble(pD);

                        *_pCurrentCuttingPlanesInteriorPointer = A;
                        _pCurrentCuttingPlanesInteriorPointer++;

                        *_pCurrentCuttingPlanesInteriorPointer = B;
                        _pCurrentCuttingPlanesInteriorPointer++;

                        *_pCurrentCuttingPlanesInteriorPointer = C;
                        _pCurrentCuttingPlanesInteriorPointer++;

                        *_pCurrentCuttingPlanesInteriorPointer = D;
                        _pCurrentCuttingPlanesInteriorPointer++;
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_AttributeLock) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pType = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Type, pType);

                    if(pType != NULL)
                    {
                        // ugh
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"Color") == 0)
                        {
                            pModelViewNode->lockAttribute(DWFModelScene::eColor);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"CuttingPlane") == 0)
                        {
                            pModelViewNode->lockAttribute(DWFModelScene::eCuttingPlane);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"ModellingMatrix") == 0)
                        {
                            pModelViewNode->lockAttribute(DWFModelScene::eModellingMatrix);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"TextureMatrix") == 0)
                        {
                            pModelViewNode->lockAttribute(DWFModelScene::eTextureMatrix);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"Visibility") == 0)
                        {
                            pModelViewNode->lockAttribute(DWFModelScene::eVisibility);
                        }
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_AttributeUnlock) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pType = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Type, pType);

                    if(pType != NULL)
                    {
                        // ugh
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"Color") == 0)
                        {
                            pModelViewNode->unlockAttribute(DWFModelScene::eColor);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"CuttingPlane") == 0)
                        {
                            pModelViewNode->unlockAttribute(DWFModelScene::eCuttingPlane);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"ModellingMatrix") == 0)
                        {
                            pModelViewNode->unlockAttribute(DWFModelScene::eModellingMatrix);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"TextureMatrix") == 0)
                        {
                            pModelViewNode->unlockAttribute(DWFModelScene::eTextureMatrix);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pType, /*NOXLATE*/"Visibility") == 0)
                        {
                            pModelViewNode->unlockAttribute(DWFModelScene::eVisibility);
                        }
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_DisplayMode) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    const char* pMode = NULL;

                    _getAttribute(ppAttributeList, DWFXML::kzAttribute_Mode, pMode);

                    if(pMode != NULL)
                    {
                        // ugh
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pMode, /*NOXLATE*/"Shaded") == 0)
                        {
                            pModelViewNode->setDisplayMode(DWFModelScene::eShaded);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pMode, /*NOXLATE*/"Edges") == 0)
                        {
                            pModelViewNode->setDisplayMode(DWFModelScene::eEdges);
                        }
                        else
                        if (DWFCORE_COMPARE_ASCII_STRINGS(pMode, /*NOXLATE*/"ShadedWithEdges") == 0)
                        {
                            pModelViewNode->setDisplayMode(DWFModelScene::eShaded | DWFModelScene::eEdges);
                        }
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_UserAttributes) == 0)
            {
                if (_pCurrentHandler != NULL)
                {
                    size_t iAttrib = 0;
                    for(; ppAttributeList[iAttrib]; iAttrib += 2)
                    {
                        const char* pAttrib = &ppAttributeList[iAttrib][0];
                        const char* pAttrValue = ppAttributeList[iAttrib+1];
                        const char* pNamespaceEnd = strchr(pAttrib, ':');
                        size_t nNamespaceLength = pNamespaceEnd - pAttrib + 1;
                        char* pNamespace = new char[nNamespaceLength + 1];
                        strncpy( pNamespace, pAttrib, nNamespaceLength );
                        pNamespace[nNamespaceLength] = 0;
                        _pCurrentHandler->addUserAttribute( pNamespaceEnd + 1, pAttrValue, pNamespace );
                        delete [] pNamespace;
                    }
                }
            }

            break;
        }
    }


    _nElementDepth++;

}

_DWFTK_API
void
DWFContentPresentationReader::notifyEndElement( const char* zName )
throw()
{
    //
    // decrement the depth first
    // this lets us match up the switch statements for start and end
    //
    --_nElementDepth;

    switch (_nElementDepth)
    {
            //
            // Expected values:
            //  Content presentation resource
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentationResource) == 0)
            {
                ;
            }
            break;
        }

            //
            // Expected values:
            //  Presentations
            //
        case 1:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Presentations) == 0)
            {
                ;
            }
            break;
        }

            //
            // Expected values:
            //  Presentation
            //
        case 2:
        {
            if ((_nProviderFlags & eProvidePresentations)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Presentation) == 0)
            {
                _providePresentation( _pCurrentPresentation );
                _pCurrentPresentation = NULL;
            }
            break;
        }

            //
            // Expected values:
            //  Views
            //
        case 3:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Views) == 0)
            {
                ;
            }
            break;
        }

            //
            // Expected values:
            //  View
            //
        case 4:
        {
            if ((_nProviderFlags & eProvideViews)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_View) == 0)
            {
                if (_pCurrentPresentationView)
                {
                    _pCurrentPresentationView = _provideView( _pCurrentPresentationView );
                    
                    _pCurrentPresentation->addView(_pCurrentPresentationView);
                }
                //
                // When the View ends, re-establish any pre-existing node container.
                //
                _oNodeContainerStack.pop_back();
                if(_oNodeContainerStack.size() > 0)
                {
                    _pCurrentNodeContainer = _oNodeContainerStack.back();
                    _pCurrentNode = dynamic_cast<DWFContentPresentationNode*>(_pCurrentNodeContainer);
                }
                else
                {
                    _pCurrentNodeContainer = NULL;
                }
            }
            break;
        }

            //
            // Below level 4, things can be a jumble.
            //
            //  PropertyReferences - from the view
            //    - PropertyReference
            // Expected values:
            //  Nodes
            //  Node
            //  ReferenceNode
            //  ModelViewNode
            //
        default:
        {
            if ((_nProviderFlags & eProvidePropertyReferences)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_PropertyReference) == 0)
            {
                if (_pCurrentPropertyReference)
                {
                    _pCurrentPropertyReference = _providePropertyReference( _pCurrentPropertyReference );

                    if (_pCurrentPresentationView)
                    {
                        _pCurrentPresentationView->addPropertyReference( _pCurrentPropertyReference );
                    }

                    _pCurrentPropertyReference = NULL;
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Nodes) == 0)
            {
            }
            else
            if ((_nProviderFlags & eProvideNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Node) == 0)
            {
                //
                // When the Node ends, re-establish any pre-existing node container.
                //
                _oNodeContainerStack.pop_back();
                if(_oNodeContainerStack.size() > 0)
                {
                    _pCurrentNodeContainer = _oNodeContainerStack.back();

                    if (_pCurrentNode)
                    {
                        _pCurrentNode = _provideNode( _pCurrentNode );
                        
                        _pCurrentNodeContainer->addNode(_pCurrentNode);
                    }

                    _pCurrentNode = dynamic_cast<DWFContentPresentationNode*>(_pCurrentNodeContainer);
                }
                else
                {
                    _pCurrentNodeContainer = NULL;
                }
            }
            else
            if ((_nProviderFlags & eProvideRefereceNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ReferenceNode) == 0)
            {

                //
                // When the Node ends, re-establish any pre-existing node container.
                //
                _oNodeContainerStack.pop_back();
                if(_oNodeContainerStack.size() > 0)
                {
                    _pCurrentNodeContainer = _oNodeContainerStack.back();

                    //
                    // Remember, ReferenceNode is derived from Node
                    //
                    if (_pCurrentNode)
                    {
                        DWFContentPresentationReferenceNode* pReferenceNode = dynamic_cast<DWFContentPresentationReferenceNode*>(_pCurrentNode);
                        if (pReferenceNode)
                        {
                            _pCurrentNode = _provideReferenceNode( pReferenceNode );
                        }
                        
                        _pCurrentNodeContainer->addNode(_pCurrentNode);
                    }

                    _pCurrentNode = dynamic_cast<DWFContentPresentationNode*>(_pCurrentNodeContainer);
                }
                else
                {
                    _pCurrentNodeContainer = NULL;
                }
            }
            else
            if ((_nProviderFlags & eProvideModelViewNodes)
                && DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModelViewNode) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    pModelViewNode->close();
                }

                //
                // When the Node ends, re-establish any pre-existing node container.
                //
                _oNodeContainerStack.pop_back();
                if(_oNodeContainerStack.size() > 0)
                {
                    _pCurrentNodeContainer = _oNodeContainerStack.back();

                    //
                    // Remember, ModelViewNode is derived from ReferenceNode
                    //
                    if (_pCurrentNode)
                    {
                        DWFContentPresentationModelViewNode* pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode*>(_pCurrentNode);
                        if (pModelViewNode)
                        {
                            _pCurrentNode = _provideModelViewNode( pModelViewNode );
                        }
                        
                        _pCurrentNodeContainer->addNode(_pCurrentNode);
                    }
                    
                    _pCurrentNode = dynamic_cast<DWFContentPresentationNode*>(_pCurrentNodeContainer);
                }
                else
                {
                    _pCurrentNodeContainer = NULL;
                }
            }
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModelScene) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Camera) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceAttributes) == 0)
            {
            }
            else
            if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ModellingMatrix) == 0) ||
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_TextureMatrix)   == 0))
            {
                if (_pCurrentHandler != NULL)
                {
                    TK_Matrix* pMatrixHandler = dynamic_cast<TK_Matrix*>(_pCurrentHandler);
                    if (pMatrixHandler)
                    {
                        pMatrixHandler->serialize();
                    }
                }
                _pCurrentHandler = NULL;
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Visibility) == 0)
            {
                if (_pCurrentHandler != NULL)
                {
                    TK_Visibility* pVisibilityHandler = dynamic_cast<TK_Visibility*>(_pCurrentHandler);
                    if (pVisibilityHandler)
                    {
                        pVisibilityHandler->serialize();
                    }
                }
                _pCurrentHandler = NULL;
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceVisibility) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_InstanceTransparency) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GeometricVariation) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Color) == 0)
            {
                if (_pCurrentHandler != NULL)
                {
                    TK_Color* pColorHandler = dynamic_cast<TK_Color*>(_pCurrentHandler);
                    if (pColorHandler)
                    {
                        pColorHandler->serialize();
                    }
                }
                _pCurrentHandler = NULL;
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Channels) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Channel) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CuttingPlane) == 0)
            {
                DWFContentPresentationModelViewNode *pModelViewNode = dynamic_cast<DWFContentPresentationModelViewNode *>(_pCurrentNode);
                if(pModelViewNode != NULL)
                {
                    if (_pCurrentHandler)
                    {
                        TK_Cutting_Plane* pCuttingPlane = dynamic_cast<TK_Cutting_Plane*>(_pCurrentHandler);
                        if (pCuttingPlane)
                        {
                            pCuttingPlane->SetPlanes(_nCurrentCuttingPlanesCount, _pCurrentCuttingPlanes);
                            pCuttingPlane->serialize();
                        }
                        delete [] _pCurrentCuttingPlanes;
                        _pCurrentCuttingPlanes = NULL;
                        _nCurrentCuttingPlanesCount = 0;
                    }
                }
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Planes) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Plane) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_AttributeLock) == 0)
            {
            }
            else
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_AttributeUnlock) == 0)
            {
            }
            break;
        }
    }
}

_DWFTK_API
void
DWFContentPresentationReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                    const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFContentPresentationReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFContentPresentationReader::notifyCharacterData( const char* /*zCData*/, 
                                                   int         /*nLength*/ ) 
throw()
{
}

void 
DWFContentPresentationReader::_provideVersion( double nVersion)
throw()
{
    provideVersion( _pReaderFilter ? _pReaderFilter->provideVersion(nVersion) : nVersion );
}

void 
DWFContentPresentationReader::_providePresentation( DWFContentPresentation* pPresentation )
throw()
{
    providePresentation( _pReaderFilter ? _pReaderFilter->providePresentation(pPresentation) : pPresentation );
}

DWFContentPresentationView*
DWFContentPresentationReader::_provideView( DWFContentPresentationView* pView )
throw()
{
    return provideView( _pReaderFilter ? _pReaderFilter->provideView(pView) : pView );
}

DWFPropertyReference*
DWFContentPresentationReader::_providePropertyReference( DWFPropertyReference* pPropertyReference )
throw()
{
    return providePropertyReference( _pReaderFilter ? _pReaderFilter->providePropertyReference(pPropertyReference) : pPropertyReference );
}

DWFContentPresentationNode*
DWFContentPresentationReader::_provideNode( DWFContentPresentationNode* pNode )
throw()
{
    return provideNode( _pReaderFilter ? _pReaderFilter->provideNode(pNode) : pNode );
}

DWFContentPresentationReferenceNode*
DWFContentPresentationReader::_provideReferenceNode( DWFContentPresentationReferenceNode* pReferenceNode )
throw()
{
    return provideReferenceNode( _pReaderFilter ? _pReaderFilter->provideReferenceNode(pReferenceNode) : pReferenceNode );
}

DWFContentPresentationModelViewNode*
DWFContentPresentationReader::_provideModelViewNode( DWFContentPresentationModelViewNode* pModelViewNode )
throw()
{
    return provideModelViewNode( _pReaderFilter ? _pReaderFilter->provideModelViewNode(pModelViewNode) : pModelViewNode );
}

void
DWFContentPresentationReader::_getAttribute( const char**  ppAttributeList, const char * pAttrName, const char * &rpAttrValue )
{
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];
            //
            // If it matches, grab it.
            //
        if(DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, pAttrName) == 0)
        {
            rpAttrValue = ppAttributeList[iAttrib+1];
            break;
        }
    }
}
