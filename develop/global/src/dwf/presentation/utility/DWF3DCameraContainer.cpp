//
//  Copyright (c) 2007 by Autodesk, Inc.
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


#include "dwf/presentation/utility/DWF3DCameraContainer.h"
#include "dwf/presentation/DWF3DCamera.h"
using namespace DWFToolkit;

_DWFTK_API
DWF3DCameraContainer::DWF3DCameraContainer()
throw()
{
}

_DWFTK_API
DWF3DCameraContainer::~DWF3DCameraContainer()
throw()
{
    //
    // delete all cameras we own
    //
    DWFOrderedVector<DWF3DCamera*>::Iterator* piCameras = getCameras();

    if (piCameras)
    {
        for (piCameras->reset(); piCameras->valid(); piCameras->next())
        {
            DWFCORE_FREE_OBJECT(piCameras->get());
        }
    }

    DWFCORE_FREE_OBJECT( piCameras );
}

_DWFTK_API
void
DWF3DCameraContainer::addCamera( DWF3DCamera& camera )
throw( DWFException )
{
    DWF3DCamera* pNewCamera = DWFCORE_ALLOC_OBJECT(DWF3DCamera(camera));
    if (pNewCamera != NULL)
    {
        size_t tIndex = _oCameras.size();
        _oCameras.insertAt(pNewCamera, tIndex);
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new camera." );
    }
}

_DWFTK_API
void 
DWF3DCameraContainer::insertAt( DWF3DCamera* p3DCamera, size_t index )
throw( DWFException )
{
    if (p3DCamera == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current camera" );
    }

    if (index>_oCameras.size())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The insertion index is larger than the vector size" );
    }

    DWF3DCamera* pNewCamera = DWFCORE_ALLOC_OBJECT(DWF3DCamera(*p3DCamera));
    
    if (pNewCamera != NULL)
    {
        _oCameras.insertAt(pNewCamera, index); 
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new camera." );
    }
}

_DWFTK_API
DWFOrderedVector<DWF3DCamera*>::Iterator*
DWF3DCameraContainer::getCameras( )
throw()
{
    return _oCameras.iterator();
}

_DWFTK_API
void
DWF3DCameraContainer::removeCameraAt( size_t index )
throw( DWFException )
{
    if( _oCameras.size() < index )
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The index is out of range." );
    }

    DWF3DCamera * pCamera = _oCameras[index];
    _oCameras.eraseAt(index);
    
    DWFCORE_FREE_OBJECT(pCamera);
}
