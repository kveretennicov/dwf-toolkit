//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
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

#include "dwfcore/Owner.h"
using namespace DWFCore;


_DWFCORE_API
DWFOwner::DWFOwner()
throw()
{
    ;
}

_DWFCORE_API
DWFOwner::~DWFOwner()
throw()
{
    ;
}

_DWFCORE_API
void
DWFOwner::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
throw( DWFException )
{
    ;
}

_DWFCORE_API
void
DWFOwner::notifyOwnableDeletion( DWFOwnable& /*rOwnable*/ )
throw( DWFException )
{
    ;
}

///
///
///


_DWFCORE_API
DWFOwnable::DWFOwnable()
throw()
          : _pOwner( NULL )
{
    ;
}

_DWFCORE_API
DWFOwnable::~DWFOwnable()
throw()
{
    ;
}

_DWFCORE_API
void
DWFOwnable::own( DWFOwner& rOwner )
throw( DWFException )
{
    if (_pOwner == &rOwner)
    {
        return;
    }

    if (_pOwner)
    {
        _pOwner->notifyOwnerChanged( *this );
    }

    _pOwner = &rOwner;
    _oOwnerObservers.insert( &rOwner );
}

_DWFCORE_API
bool
DWFOwnable::disown( DWFOwner& rOwner, bool bForget )
throw( DWFException )
{
    if (_pOwner)
    {
            //
            // can only disown what we own
            //
        if (_pOwner == &rOwner)
        {
                //
                // remove owner from observers
                //
            if (bForget)
            {
                _oOwnerObservers.erase( _pOwner );
            }

            //
            //
            //
            _pOwner->notifyOwnerChanged( *this );
            _pOwner = NULL;

            return true;
        }
    }

    return false;
}

_DWFCORE_API
DWFOwner*
DWFOwnable::owner()
throw( DWFException )
{
    return _pOwner;
}

_DWFCORE_API
void
DWFOwnable::observe( DWFOwner& rOwner )
throw( DWFException )
{
    if (_pOwner == &rOwner)
    {
        // can't observe something that you already own.
        return;
    }

    _oOwnerObservers.insert( &rOwner );
}

_DWFCORE_API
bool
DWFOwnable::unobserve( DWFOwner& rOwner )
throw( DWFException )
{
        //
        // Ensure that observe and own aren't being confused.
        // We'd better not be the owner if we're just observing.
        //
    if (_pOwner == &rOwner)
    {
        return false;
    }

    //
    // remove observer from observers
    //
    _oOwnerObservers.erase( &rOwner );

    return true;
}

_DWFCORE_API
void
DWFOwnable::_notifyDelete()
throw()
{
    if (_oOwnerObservers.size() > 0)
    {
        DWFOwner* pOwner = NULL;
        std::set<DWFOwner*>::iterator piOwner = _oOwnerObservers.begin();

        for (; piOwner != _oOwnerObservers.end(); )
        {
            pOwner = *piOwner;

            //
            // Increment the iterator here, before calling notifyOwnableDeletion,
            // as that might cause pOwner to disown/unobserve us, which messes
            // up the iterator. (Which causes a crash when calling piOwner->next().)
            //
            ++piOwner;

                //
                // owner should be the only one deleting this object
                // so there is no need to notify him
                //
            if (pOwner != _pOwner)
            {
                pOwner->notifyOwnableDeletion( *this );
            }
        }

    }
}
