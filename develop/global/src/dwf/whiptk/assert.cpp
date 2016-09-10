//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/assert.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"

#if defined WD_WIN32_SYSTEM

#include <windows.h>

void WD_Fail_Assertion (
    WD_stack char const *    assertion,
    WD_stack char const *    file,
    WD_stack int             line) {
    WD_stack char            buf[512];

    wsprintfA (buf, "\nThe assertion '%s' failed in %s at line %d.\nThe last Win32 error code was %d.\n",
              assertion, file, line, GetLastError ());
    OutputDebugStringA (buf);

    strcat (buf, "\n\nClick OK to ignore and continue, or CANCEL to debug.");

    WD_stack       int             status;
    status = MessageBoxA (NULL, buf, "WHIP! Toolkit Assertion Failed", MB_ICONSTOP|MB_OKCANCEL|MB_TASKMODAL);

    if (status == IDCANCEL)
    {
        #ifdef USE_INTEL_ASSEMBLER
            __asm int 3   //fire up the debugger
        #else
            DebugBreak();
        #endif
    }
    return;

}

#pragma warning (default: 4702)

#elif defined WD_MAC_SYSTEM

#include <stdio.h>
#include <signal.h>

void WD_Fail_Assertion (
    WD_stack char const *    assertion,
    WD_stack char const *    file,
    WD_stack int             line)
{
    fprintf (stderr, "\nThe assertion '%s' failed in %s at line %d.\n", assertion, file, line);
    fprintf (stderr, "WHIP! Toolkit Assertion Failed");
    raise (SIGUSR1);  // gdb: "handle SIGUSR1 stop"
}



#elif defined WD_LINUX_SYSTEM

#include <stdio.h>
#include <signal.h>

void WD_Fail_Assertion (
    WD_stack char const *    assertion,
    WD_stack char const *    file,
    WD_stack int             line)
{
    fprintf (stderr, "\nThe assertion '%s' failed in %s at line %d.\n", assertion, file, line);
    fprintf (stderr, "WHIP! Toolkit Assertion Failed");
    raise (SIGUSR1);  // gdb: "handle SIGUSR1 stop"
}

#else
    #error      "Fatal WHIP! toolkit build error -- Unknown system type"
#endif
