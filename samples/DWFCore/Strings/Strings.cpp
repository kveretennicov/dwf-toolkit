//
//  Copyright (c) 2006 by Autodesk, Inc.
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

#include "stdafx.h"


using namespace std;
using namespace DWFCore;



int main()
{
        unsigned long nLoops = 10;

        DWFCore::DWFFile name( L"Strings.txt" );
        DWFCore::DWFStreamFileDescriptor fd( name, L"w+" );
        DWFCore::DWFFileOutputStream file;

        fd.open();
        file.attach( &fd, false );


        unsigned long i = 0;
        for (; i < nLoops; i++)
        {
            char* pUTF8 = NULL;
            size_t nUTF8Bytes = 0;


            DWFCore::DWFString  zTestMeStack;
            DWFCore::DWFString* pTestMeHeap = DWFCORE_ALLOC_OBJECT( DWFString );
            if (pTestMeHeap == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate DWFString on heap" );
            }

            zTestMeStack.assign( "small " );
            zTestMeStack.append( "smaller " );
            zTestMeStack.append( L"wide " );

            //
            // Test != and == operator corner cases.
            //
            DWFCore::DWFString  zEmpty1;
            DWFCore::DWFString  zEmpty2;
            if(zEmpty1 != zTestMeStack) {;}
            if(zTestMeStack!= zEmpty1) {;}
            if(zEmpty1 != zEmpty2) {;}
            if(zEmpty1 != L"ABC") {;}
            if(zEmpty1 != L"") {;}
            if(zEmpty1 == zTestMeStack) {;}
            if(zTestMeStack == zEmpty1) {;}
            if(zEmpty1 == zEmpty2) {;}
            if(zEmpty1 == L"ABC") {;}
            if(zEmpty1 == L"") {;}


            // added to test Find(). doesn't write to the file. you'll need the debugger to verify.
            off_t offset;
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 0, false);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 0, true);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 1, false);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 1, true);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 18, false);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 18, true);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 19, false);
            offset = DWFCore::DWFString::Find(zTestMeStack, L's', 19, true);
            offset = DWFCore::DWFString::Find(zTestMeStack, L'X', 0, false);
            offset = DWFCore::DWFString::Find(zTestMeStack, L'X', 0, true);
            offset = DWFCore::DWFString::Find(zEmpty1, L"ABC", 0, true);
            offset = DWFCore::DWFString::Find(zTestMeStack, L"", 0, true);
            offset = DWFCore::DWFString::Find(zEmpty1, L"", 0, true);
            offset = DWFCore::DWFString::Find(zEmpty1, L'X', 0, true);
            offset = DWFCore::DWFString::Find(L"", L"", 0, true);
            offset = DWFCore::DWFString::Find(L"", L'X', 0, true);
            offset = zEmpty1.find( L'X', 0, false);
            offset = zEmpty1.find( L"ABC", 0, false);
            offset = zEmpty1.find( L"", 0, false);
            offset = zEmpty1.find( zTestMeStack, 0, false);
            offset = zEmpty1.find( zEmpty2, 0, false);
            offset = zTestMeStack.find( L"", 0, false);
            offset = zTestMeStack.find( zEmpty1, 0, false);

            if(offset)
            {
            }

            file.write( L"[1] ", 4*sizeof(wchar_t) );
            file.write( (const wchar_t*)zTestMeStack, zTestMeStack.bytes() );
            file.write( L"\n", sizeof(wchar_t) );

            pTestMeHeap->assign( zTestMeStack );
            nUTF8Bytes = pTestMeHeap->getUTF8( &pUTF8 );

            if (pUTF8)
            {
                file.write( L"[2] ", 4*sizeof(wchar_t) );
                file.write( (const wchar_t*)*pTestMeHeap, pTestMeHeap->bytes() );
                file.write( L"\n", sizeof(wchar_t) );

                file.write( L"[3] ", 4*sizeof(wchar_t) );
                file.write( pUTF8, nUTF8Bytes );
                file.write( L"\n", sizeof(wchar_t) );

                DWFCORE_FREE_MEMORY( pUTF8 );
            }

            pTestMeHeap->destroy();

            if ((pTestMeHeap->bytes() == 0) &&
                (pTestMeHeap->chars() == 0) &&
                ((const wchar_t*)*pTestMeHeap == NULL))
            {
                file.write( L"[4] (is empty)\n", 15*sizeof(wchar_t) );
            }

            *pTestMeHeap = L"日本語";
            nUTF8Bytes = pTestMeHeap->getUTF8( &pUTF8 );

            if (pUTF8)
            {
                file.write( L"[5] ", 4*sizeof(wchar_t) );
                file.write( (const wchar_t*)*pTestMeHeap, pTestMeHeap->bytes() );
                file.write( L"\n", 2 );

                file.write( L"[6] ", 4*sizeof(wchar_t) );
                file.write( pUTF8, nUTF8Bytes );
                file.write( L"\n", sizeof(wchar_t) );
            }

            DWFCORE_FREE_MEMORY( pUTF8 );

            zTestMeStack = *pTestMeHeap;

            if (zTestMeStack == *pTestMeHeap)
            {
                file.write( L"[7] ", 4*sizeof(wchar_t) );
                file.write( (const wchar_t*)zTestMeStack, zTestMeStack.bytes() );
                file.write( L" == ", 4*sizeof(wchar_t) );
                file.write( (const wchar_t*)*pTestMeHeap, pTestMeHeap->bytes() );
                file.write( L"\n", sizeof(wchar_t) );
            }

            DWFCORE_FREE_OBJECT( pTestMeHeap );

            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );
            zTestMeStack.append( zTestMeStack );

            nUTF8Bytes = zTestMeStack.getUTF8( &pUTF8 );

            if (pUTF8)
            {
                file.write( L"[8] ", 4*sizeof(wchar_t) );
                file.write( (const wchar_t*)zTestMeStack, zTestMeStack.bytes() );
                file.write( L"\n", sizeof(wchar_t) );

                file.write( L"[9] ", 4*sizeof(wchar_t) );
                file.write( pUTF8, nUTF8Bytes );
                file.write( L"\n", sizeof(wchar_t) );
            }

            DWFCORE_FREE_MEMORY( pUTF8 );
        }

        DWFCore::DWFString s1(L"This is my string.");
        DWFCore::DWFString s2 = s1.substring(0);
        DWFCore::DWFString s3 = s1.substring(5);
        DWFCore::DWFString s4 = s1.substring(5, 0);
        DWFCore::DWFString s5 = s1.substring(5, 2);
        DWFCore::DWFString s6 = s1.substring(5, 100);

        cout << "OK\n";

    return 0;
}
