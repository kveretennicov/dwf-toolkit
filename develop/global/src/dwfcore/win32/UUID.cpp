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

#include "dwfcore/Core.h"
#ifdef  _DWFCORE_WIN32_SYSTEM


#include "dwfcore/win32/UUID.h"
#include "dwfcore/Timer.h"
using namespace DWFCore;



DWFUUIDImpl_Win32::DWFUUIDImpl_Win32()
throw()
{
}

DWFUUIDImpl_Win32::DWFUUIDImpl_Win32( const DWFUUIDImpl_Win32& uuid )
throw()
                 : _oUUID( uuid._oUUID )
{
}

DWFUUIDImpl_Win32&
DWFUUIDImpl_Win32::operator=( const DWFUUIDImpl_Win32& uuid )
throw()
{
    _oUUID = uuid._oUUID;
    return *this;
}

DWFUUIDImpl_Win32::~DWFUUIDImpl_Win32()
throw()
{
    ;
}

DWFUUIDImpl*
DWFUUIDImpl_Win32::clone() const
throw( DWFException )
{
    DWFUUIDImpl_Win32* pNew = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_Win32(*this) );
    if (pNew == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate clone" );
    }

    return pNew;
}

const DWFString&
DWFUUIDImpl_Win32::uuid( bool bSquash )
throw( DWFException )
{
        //
        // this string will be empty upon initialization
        // in that case, generate a new UUID
        //
    if (_zUUID.bytes() == 0)
    {
            //
            // if this call fails, rather than replicating the generic implementation
            // we throw a exception indicating this feature is unavailable
            // it then becomes the job of the DWFUUID object to create an instance 
            // of the generic implementation
            // 
        if (::UuidCreate(&_oUUID) == RPC_S_UUID_NO_ADDRESS)
        {
            _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Failed to acquire UUID" );
        }
    }

    char str[64] = {0};

    if (bSquash)
    {
        DWFString::EncodeBase64( &_oUUID, sizeof(_oUUID), str, 64, false );
    }
    else
    {
        ::sprintf( str, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                        _oUUID.Data1, 
                        _oUUID.Data2, _oUUID.Data3,
                        _oUUID.Data4[0], _oUUID.Data4[1], _oUUID.Data4[2], _oUUID.Data4[3],
                        _oUUID.Data4[4], _oUUID.Data4[5], _oUUID.Data4[6], _oUUID.Data4[7] );
    }    

    _zUUID.assign( str );
    return _zUUID;
}

const DWFString&
DWFUUIDImpl_Win32::next( bool bSquash )
throw( DWFException )
{
        //
        // if the UUID already exists, increment the variant part
        //
    if (_zUUID.bytes() > 0)
    {
            //
            // incremement the timestamp portion
            // of the UUID only
            //
        if (_oUUID.Data1 < (uint32_t)0xffffffee)
        {
            _oUUID.Data1++;
        }
            //
            // don't allow anymore sequential incremements
            //
        else
        {
            //
            // clear the string to generate a new UUID
            //
            _zUUID.destroy();
        }
    }

    return uuid( bSquash );
}



#endif

