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


#define TESTS   32
#define LOOP    665


int dmain()
{
    char    abc[] = "abcdefgh";
    char*   p = &abc[7];
    char    buf[64] = {0};


    DWFCore::DWFString::EncodeBase64( abc, 8, buf, 64, false );
    cout << buf << endl;

    (*p)+=16;

    DWFCore::DWFString::EncodeBase64( abc, 8, buf, 64, false );
    cout << buf << endl;
    return 0;
}

int main()
{
        bool bOK = true;

        for (unsigned int t = 0; t < TESTS; t++)
        {
            DWFCore::DWFUUID oUUID;
            DWFCore::DWFStringKeySkipList<bool>  oList;

            for (unsigned int i = 0; i < LOOP; i++)
            {

                DWFCore::DWFString zUUID = oUUID.next(false);
                wcout << "(" << t << ", " << i << ") generate UUID: " << (const wchar_t*)zUUID << "\n";
                if (false == oList.insert(zUUID, true))
                {
                    bOK = false;
                    wcout << "(" << t << ", " << i << ") DUPLICATE " << (const wchar_t*)zUUID << "\n";
                }
            }

            wcout << ".";
        }

        cout<<"\nOK\n";
   
    return 0;
}
