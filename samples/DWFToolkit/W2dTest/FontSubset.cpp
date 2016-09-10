#include "Stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "fontsub.h"

typedef unsigned long (__cdecl * CREATEFONTPACKAGE)(
    CONST unsigned char * puchSrcBuffer, /* input TTF or TTC buffer */
    CONST unsigned long ulSrcBufferSize, /* size of input TTF or TTC buffer data */
    unsigned char ** ppuchFontPackageBuffer, /* output package buffer */
    unsigned long * pulFontPackageBufferSize, /* output package buffer size */
    unsigned long * pulBytesWritten,  /* output package buffer data length */
    CONST unsigned short usFlag, /* subset, compress, or both, TTF or TTC, Chars or Glyphs */
    CONST unsigned short usTTCIndex, /* TTC Index, only used if TTC bit set */
    CONST unsigned short usSubsetFormat, /* Old Subset, Subset or Delta */
    CONST unsigned short usSubsetLanguage, /* Language in Name table to keep */
    CONST unsigned short usSubsetPlatform, /* if ListType is Character, Platform of cmap to use for glyph list */
    CONST unsigned short usSubsetEncoding, /* if ListType is Character, Encoding of cmap to use for glyph list */
    CONST unsigned short *pusSubsetKeepList, /* List of Characters or Glyphs to keep */
    CONST unsigned short usSubsetListCount,  /* number of elements in list */
    CFP_ALLOCPROC lpfnAllocate,   /* call back function to allocate temp buffers and output buffers */
    CFP_REALLOCPROC lpfnReAllocate,   /* call back function to reallocate temp and output buffers */
    CFP_FREEPROC lpfnFree,  /* call back function to free buffer allocated with lpfnAllocate and lpfnReAllocate */
    void * lpvReserved);


int subsetFont(void* pFontBuffer,               /*in*/
    DWORD nFontBufferLen,                       /*in*/
    const unsigned short * keepList,            /*in*/
    unsigned short count,                       /*out*/
    unsigned char*& puchFontPackageBuffer,      /*out*/
    unsigned long&   ulFontPackageBufferSize,   /*out*/
    unsigned long&   ulBytesWritten)            /*out*/
{
    
    puchFontPackageBuffer   = NULL;
    ulFontPackageBufferSize = 0; 
    ulBytesWritten          = 0; 

    //Load the Font subset dll
    wchar_t dllPath[MAX_PATH];
    GetSystemDirectory(dllPath, MAX_PATH);
    wcscat(dllPath, L"\\FontSub.dll");
    HMODULE hFS = LoadLibrary(dllPath);

    int result = 0;

    if (hFS == NULL)
    {
        result = -3;
    }
    else
    {
        //Get the Address of the exported fn
        CREATEFONTPACKAGE lpDllEntryPoint = (CREATEFONTPACKAGE) GetProcAddress(hFS, "CreateFontPackage");

        if (lpDllEntryPoint == NULL)
        {
            result = -4;
        }
        else
        {
            DWORD ret;

            ret = (*lpDllEntryPoint)(
                (const unsigned char*)pFontBuffer,     /* input TTF or TTC buffer */
                nFontBufferLen,     /* size of input TTF or TTC buffer data */
                &puchFontPackageBuffer,     /* output package buffer */
                &ulFontPackageBufferSize, /* output package buffer size */
                &ulBytesWritten,   /* output package buffer data length */
                TTFCFP_FLAGS_SUBSET,  /* subset, compress, or both, TTF or TTC, Chars or Glyphs */
                0,       /* TTC Index, only used if TTC bit set */
                TTFCFP_SUBSET,    /* Old Subset, Subset or Delta */
                0,       /* Language in Name table to keep */
                TTFCFP_UNICODE_PLATFORMID,  /* if ListType is Character, Platform of cmap to use for glyph list */
                TTFCFP_DONT_CARE,   /* if ListType is Character, Encoding of cmap to use for glyph list */
                keepList,     /* List of Characters or Glyphs to keep */
                count,      /* number of elements in list */
                malloc,      /* call back function to allocate temp buffers and output buffers */
                realloc,     /* call back function to reallocate temp and output buffers */
                free,      /* call back function to free buffer allocated with lpfnAllocate and lpfnReAllocate */
                NULL);

            if (ret == 1006 && puchFontPackageBuffer==NULL)
            {
                ret = (*lpDllEntryPoint)(
                    (const unsigned char*)pFontBuffer,     /* input TTF or TTC buffer */
                    nFontBufferLen,     /* size of input TTF or TTC buffer data */
                    &puchFontPackageBuffer,     /* output package buffer */
                    &ulFontPackageBufferSize, /* output package buffer size */
                    &ulBytesWritten,   /* output package buffer data length */
                    TTFCFP_FLAGS_SUBSET,  /* subset, compress, or both, TTF or TTC, Chars or Glyphs */
                    0,       /* TTC Index, only used if TTC bit set */
                    TTFCFP_SUBSET,    /* Old Subset, Subset or Delta */
                    0,       /* Language in Name table to keep */
                    TTFCFP_MS_PLATFORMID,  /* if ListType is Character, Platform of cmap to use for glyph list */
                    TTFCFP_DONT_CARE,   /* if ListType is Character, Encoding of cmap to use for glyph list */
                    keepList,     /* List of Characters or Glyphs to keep */
                    count,      /* number of elements in list */
                    malloc,      /* call back function to allocate temp buffers and output buffers */
                    realloc,     /* call back function to reallocate temp and output buffers */
                    free,      /* call back function to free buffer allocated with lpfnAllocate and lpfnReAllocate */
                    NULL);
            }

            if (puchFontPackageBuffer == NULL)
            {
                WD_Assert( false );
                result = -5;
            }
            //else
            //{
            //    printf("Subset: %d glyphs, original size: %d bytes -> subset buffer size: %d (%d bytes actually written)\r\n",
            //        count, nFontBufferLen, ulFontPackageBufferSize, ulBytesWritten);
            //}
        }
    }

    return result;
}

/*
Return value Value Description 
NO_ERROR 0 No error. 
ERR_FORMAT 1006 Input data format error. 
ERR_GENERIC 1000   
ERR_MEM 1005 Error allocating memory. 
ERR_NO_GLYPHS 1009   
ERR_INVALID_BASE 1085   
ERR_INVALID_CMAP 1030   
ERR_INVALID_DELTA_FORMAT 1013 Trying to subset a format 1 or 2 font. 
ERR_INVALID_EBLC 1086   
ERR_INVALID_GLYF 1061   
ERR_INVALID_GDEF 1083   
ERR_INVALID_GPOS 1082   
ERR_INVALID_GSUB 1081   
ERR_INVALID_HDMX 1089   
ERR_INVALID_HEAD 1062   
ERR_INVALID_HHEA 1063   
ERR_INVALID_HHEA_OR_VHEA 1072   
ERR_INVALID_HMTX 1064   
ERR_INVALID_HMTX_OR_VMTX 1073   
ERR_INVALID_JSTF 1084   
ERR_INVALID_LTSH 1087   
ERR_INVALID_TTO 1080   
ERR_INVALID_VDMX 1088   
ERR_INVALID_LOCA 1065   
ERR_INVALID_MAXP 1066   
ERR_INVALID_MERGE_CHECKSUMS 1011 Trying to merge two fonts from different mother font. 
ERR_INVALID_MERGE_FORMATS 1010 Trying to merge fonts with the wrong dttf formats. 
ERR_INVALID_MERGE_NUMGLYPHS 1012 Trying to merge 2 fonts from different mother font. 
ERR_INVALID_NAME 1067   
ERR_INVALID_POST 1068   
ERR_INVALID_OS2 1069   
ERR_INVALID_VHEA 1070   
ERR_INVALID_VMTX 1071   
ERR_INVALID_TTC_INDEX 1015   
ERR_MEM 1005 Error allocating memory. 
ERR_MISSING_CMAP 1030   
ERR_MISSING_EBDT 1044   
ERR_MISSING_GLYF 1031   
ERR_MISSING_HEAD 1032   
ERR_MISSING_HHEA 1033   
ERR_MISSING_HMTX 1034   
ERR_MISSING_LOCA 1035   
ERR_MISSING_MAXP 1036   
ERR_MISSING_NAME 1037   
ERR_MISSING_POST 1038   
ERR_MISSING_OS2 1039   
ERR_MISSING_VHEA 1040   
ERR_MISSING_VMTX 1041   
ERR_MISSING_HHEA_OR_VHEA 1042   
ERR_MISSING_HMTX_OR_VMTX 1043   
ERR_NOT_TTC 1014   
ERR_PARAMETER0 1100 Calling function argument 0 is invalid. 
ERR_PARAMETER1 1101 Calling function argument 1 is invalid. 
ERR_PARAMETER2 1102 Calling function argument 2 is invalid. 
ERR_PARAMETER3 1103 Calling function argument 3 is invalid. 
ERR_PARAMETER4 1104 Calling function argument 4 is invalid. 
ERR_PARAMETER5 1105 Calling function argument 5 is invalid. 
ERR_PARAMETER6 1106 Calling function argument 6 is invalid. 
ERR_PARAMETER7 1107 Calling function argument 7 is invalid. 
ERR_PARAMETER8 1108 Calling function argument 8 is invalid. 
ERR_PARAMETER9 1109 Calling function argument 9 is invalid. 
ERR_PARAMETER10 1110 Calling function argument 10 is invalid. 
ERR_PARAMETER11 1111 Calling function argument 11 is invalid. 
ERR_PARAMETER12 1112 Calling function argument 12 is invalid. 
ERR_PARAMETER13 1113 Calling function argument 13 is invalid. 
ERR_PARAMETER14 1114 Calling function argument 14 is invalid. 
ERR_PARAMETER15 1115 Calling function argument 15 is invalid. 
ERR_PARAMETER16 1116 Calling function argument 16 is invalid. 
ERR_READCONTROL 1003 Read control structure does not match data. 
ERR_READOUTOFBOUNDS 1001 Trying to read from memory not allowed - data error? 
ERR_VERSION 1008 Major dttf.version of the input data is greater than the version this program can read. 
ERR_WOULD_GROW 1007 Action would cause data to grow. use original data. 
ERR_WRITECONTROL 1004 Write control structure does not match data. 
ERR_WRITEOUTOFBOUNDS 1002 Trying to write to memory not allowed - data error? 

*/