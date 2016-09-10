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



    //
    // Win32 only (this will compile for Win64 too)
    //
#ifdef  _DWFCORE_WIN32_SYSTEM

    //
    // Define default global allocation/deallocation operators
    //
#if defined( DWFCORE_ENABLE_MEMORY_BASE ) && defined( DWFCORE_USE_DEFAULT_MEMORY_BASE_IMPL )

void* DWFCore::DWFCoreMemory::operator new( size_t s )
{   
    return ::operator new( s );
}

void* DWFCore::DWFCoreMemory::operator new( size_t s, void* p )
{   
    p = ::operator new( s );
    return p;
}

void DWFCore::DWFCoreMemory::operator delete( void* p )
{ 
    ::operator delete( p ); 
}

void DWFCore::DWFCoreMemory::operator delete( void* p, void* )
{ 
    ::operator delete( p ); 
}

#endif

    //
    // Win9x specific support functions
    //
#ifdef DWFCORE_WIN32_INCLUDE_WIN9X_CODE

_DWFCORE_API
bool
DWFCore::IsWindows9x()
{ 
    OSVERSIONINFO tInfo;
    tInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    
    ::GetVersionEx( &tInfo );

    return (tInfo.dwPlatformId & VER_PLATFORM_WIN32_WINDOWS);
}
#endif


#endif


