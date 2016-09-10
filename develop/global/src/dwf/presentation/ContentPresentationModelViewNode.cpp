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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationModelViewNode.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/ContentPresentationModelViewNode.h"
#include "dwf/w3dtk/W3DCamera.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;



_DWFTK_API
DWFContentPresentationModelViewNode::DWFContentPresentationModelViewNode( const DWFString& zLabel,
                                                                          const DWFString& zID )
throw()
                                   : DWFContentPresentationReferenceNode( zLabel, zID )
                                   , DWFModelScene()
#ifndef DWFTK_READ_ONLY
                                   , _oAddedNamespaces()
#endif
                                   , _nResetFlags( 0 )
                                   , _bInstancePreviouslyOpen( false )
                                   , _zPreviousInstanceID()
{
    ;
}

_DWFTK_API
DWFContentPresentationModelViewNode::~DWFContentPresentationModelViewNode()
throw()
{
}

_DWFTK_API
void
DWFContentPresentationModelViewNode::getModelSceneChanges( DWFModelScene& rScene )
throw(DWFException)
{
    //
    // notify start of scene changes
    //
    rScene.open();

    size_t iSceneChange = 0;
    for (; iSceneChange < _oSerializableSceneAPI.size();
           iSceneChange++)
    {
            //
            // first check for handlers that are customized for the DWFModelScene
            // such that they can pass through the notify() method of the specialized
            // class but are not really W3DTK handlers that can play against the scene graph
            //
        _W3DXMLPseudoHandler* pHandler = dynamic_cast<_W3DXMLPseudoHandler*>(_oSerializableSceneAPI[iSceneChange]);
        if (pHandler)
        {
                //
                // modify the scene camera
                //
            _W3DCamera* pCamera = dynamic_cast<_W3DCamera*>(pHandler);
            if (pCamera)
            {
                rScene.setCamera( *pCamera, pCamera->smoothTransition() );
            }
            else
            {
                    //
                    // change the DWFInstance under consideration for the next rendition
                    //
                _W3DInstance* pInstance = dynamic_cast<_W3DInstance*>(pHandler);
                if (pInstance)
                {
                    const DWFString& zID = pInstance->id();
                    if (zID.bytes() > 0)
                    {
                        rScene.setCurrentInstanceID( zID );
                    }
                }
                else
                {
                        //
                        // lock or unlock an attribute in the rendition
                        //
                    _W3DAttributeLock* pLock = dynamic_cast<_W3DAttributeLock*>(pHandler);
                    if (pLock)
                    {
                        (pLock->locked() ? rScene.lockAttribute(pLock->attribute())
                                         : rScene.unlockAttribute(pLock->attribute()) );
                    }
                    else
                    {
                            //
                            // instance specific visualization options...
                            //
                        _W3DInstanceOptions* pOptions = dynamic_cast<_W3DInstanceOptions*>(pHandler);
                        if (pOptions)
                        {
                                //
                                // instance visibility
                                //
                            if (pOptions->visible() > -1)
                            {
                                rScene.setVisibility( (pOptions->visible() == 1) ? true : false );
                            }
                                //
                                // instance transparency
                                //
                            if (pOptions->transparent() > -1)
                            {
                                rScene.setTransparency( (pOptions->transparent() == 1) ? true : false );
                            }
                                //
                                // instance geometric variation
                                //
                            if (pOptions->variation() > -1)
                            {
                                rScene.setGeometricVariationIndex( pOptions->variation() );
                            }
                        }
                        else
                        {
                                //
                                // instance specific visualization options...
                                //
                            _W3DDisplayMode* pDisplayMode = dynamic_cast<_W3DDisplayMode*>(pHandler);
                            if (pDisplayMode)
                            {
                                rScene.setDisplayMode( pDisplayMode->mode() );
                            }
                                //
                                // this shouldn't ever happen which is why an exception is getting thrown here instead of
                                // "just doing nothing" since this most certainly isn't what is expected we want to actually
                                // catch the problem at run (debug) time rather than let things pass silently
                                //
                            else
                            {
                                _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"An unknown specialized handler class made it's way onto the change stack." );
                            }
                        }

                    }
                }
            }
        }
            //
            // these handlers can be passed directly to the observer interface
            //
        else
        {
            BBaseOpcodeHandler* pOpcodeHandler = dynamic_cast<BBaseOpcodeHandler*>(_oSerializableSceneAPI[iSceneChange]);
            if (pOpcodeHandler)
            {
                rScene.notify( pOpcodeHandler );
            }
                //
                // this shouldn't ever happen which is why an exception is getting thrown here instead of
                // "just doing nothing" since this most certainly isn't what is expected we want to actually
                // catch the problem at run (debug) time rather than let things pass silently
                //
            else
            {
                _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"An unknown handler class made it's way onto the change stack." );
            }
        }
    } 

    //
    // we're done, finalize the scene
    //
    rScene.close();
}

_DWFTK_API
unsigned int
DWFContentPresentationModelViewNode::getResetFlags() const
throw( DWFException )
{
    return _nResetFlags;
}

_DWFTK_API
void
DWFContentPresentationModelViewNode::notify( BBaseOpcodeHandler* pHandler,
                                             const void*         /*pTag*/ )
throw( DWFException )
{
        //
        // we should only be getting DWFModelScene "special" handlers back through here
        //
    DWFModelSceneChangeHandler* pSceneChange = dynamic_cast<DWFModelSceneChangeHandler*>(pHandler);
    if (pSceneChange)
    {
        _preProcessHandler( pSceneChange );

        //
        // TODO: MAKE COPIES TO AVOID DOUBLE DELETES
        //
        _oSerializableSceneAPI.push_back( pSceneChange );
    }
    else
    {
        _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"Bad handler observed - this handler cannot modify the scene." );
    }
}

_DWFTK_API
void
DWFContentPresentationModelViewNode::insertSceneChanges( DWFContentPresentationModelViewNode& rNode )
throw( DWFException )
{
    // accumulate reset flags
    setResetFlags(getResetFlags()|rNode.getResetFlags());

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

    size_t iSceneChange = 0;
    for (; iSceneChange < rNode._oSerializableSceneAPI.size();
           iSceneChange++)
    {
        DWFModelSceneChangeHandler* pHandler = rNode._oSerializableSceneAPI[iSceneChange];
        DWFModelSceneChangeHandler* pHandlerClone = pHandler->clone();
        _preProcessHandler( pHandlerClone );
        _oSerializableSceneAPI.push_back( pHandlerClone );

            //
            //
            //
        _W3DInstance* pInstance = dynamic_cast<_W3DInstance*>(pHandlerClone);
        if (pInstance)
        {
            _bOpenInstance = !_bOpenInstance;
        }
    }
}

_DWFTK_API
void
DWFContentPresentationModelViewNode::setResetFlags( unsigned int nFlags )
throw(DWFException)
{
    _nResetFlags = nFlags;
}

_DWFTK_API
void
DWFContentPresentationModelViewNode::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationReferenceNode::parseAttributeList( ppAttributeList );

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

            //
            // Extract the reset flags field.
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ResetFlags) == 0))
        {
            nFound |= 0x01;
            int resetFlags = ::atoi(ppAttributeList[iAttrib+1]);
            setResetFlags(resetFlags);
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
const DWFXMLNamespace& 
DWFContentPresentationModelViewNode::addNamespace( const DWFString& zNamespace, 
                                              const DWFString& zXMLNS )
throw( DWFException )
{
    _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.find( zNamespace );
    
    if (iNamespace != _oAddedNamespaces.end())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"This namespace has already been defined." );
    }

    _oAddedNamespaces[zNamespace] = DWFXMLNamespace( zNamespace, zXMLNS );

    return _oAddedNamespaces[zNamespace];
}

void
DWFContentPresentationModelViewNode::serializeAttributes( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
throw( DWFException )
{
    //
    // attributes
    //
    if (_nResetFlags != 0)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_ResetFlags, (int)_nResetFlags );
    }

}

_DWFTK_API
void
DWFContentPresentationModelViewNode::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
            //
            // if the derived class has already open the element
            // we can only serialize the base attributes here
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_ModelViewNode );

                //
                // other namespaces added for extension
                //
            _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.begin();
            for (; iNamespace != _oAddedNamespaces.end(); iNamespace++)
            {
                rSerializer.addAttribute( iNamespace->second.prefix(), iNamespace->second.xmlns(), /*NOXLATE*/L"xmlns:" );
            }

            //
            // let base class know not to start the element
            //
            nFlags |= DWFXMLSerializer::eElementOpen;

            serializeAttributes(rSerializer, nFlags);

            //
            // base attributes
            //
            DWFContentPresentationReferenceNode::serializeXML( rSerializer, nFlags );

            //
            // clear this bit
            //
            nFlags &= ~DWFXMLSerializer::eElementOpen;
        }
            //
            // element already started by derived class
            //
        else
        {
                //
                // other namespaces added for extension
                //
            _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.begin();
            for (; iNamespace != _oAddedNamespaces.end(); iNamespace++)
            {
                rSerializer.addAttribute( iNamespace->second.prefix(), iNamespace->second.xmlns(), /*NOXLATE*/L"xmlns:" );
            }

            serializeAttributes(rSerializer, nFlags);

            //
            // base attributes
            //
            DWFContentPresentationReferenceNode::serializeXML( rSerializer, nFlags );
        }

        //
        // the scene
        //
        DWFModelScene::serializeXML( rSerializer, nFlags );

            //
            // close element if this bit is not set
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
    else
    {
            DWFContentPresentationReferenceNode::serializeXML( rSerializer, nFlags );
    }
}

#endif

void
DWFContentPresentationModelViewNode::_preProcessHandler (DWFModelSceneChangeHandler* pSceneChange)
throw( DWFException )
{
    _W3DInstance* pInstanceHandler = dynamic_cast<_W3DInstance*>(pSceneChange);
    if (pInstanceHandler)
    {
        if (!_bOpenInstance)
        {
            //
            // this signals the start of a new instance
            //
            _zOpenInstanceID = pInstanceHandler->id();
        }
    }
    else
    {
        if (pSceneChange->changeType() == DWFModelSceneChangeHandler::eSceneLevel)
        {
            if (_bOpenInstance)
            {
                //
                // this is a scene level change, so close the open instance and remember it
                //
                _bInstancePreviouslyOpen = true;
                _zPreviousInstanceID = _zOpenInstanceID;

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
        else
        {
            //
            // if an instance attribute was previously open and if we had closed it to
            // serialize a global scene change, then open it back up
            //
            if (_bInstancePreviouslyOpen)
            {
                _bInstancePreviouslyOpen = false;
                _bOpenInstance = true;

                _W3DInstance* pHandler = DWFCORE_ALLOC_OBJECT( _W3DInstance );
                if (pHandler == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate handler" );
                }
            }

            if (!_bOpenInstance)
            {
                _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"There is no currently instance to receive this change" );
            }
        }
    }
}
