/* unzip.c -- IO on .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Read unzip.h for more info
*/


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "dwfcore/config.h"
#endif

#ifdef  DWFCORE_BUILD_ZLIB
#include "dwfcore/zlib/zlib.h"
#else
#include <zconf.h>
#include <zlib.h>
#endif

#include "dwfcore/zip/unzip.h"
#include "dwfcore/zip/password.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif

#if defined(_DWFCORE_MAC_SYSTEM)
//TODO - normalize this
#define _stricmp strcasecmp
#endif

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */



#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES) && \
                      !defined(CASESENSITIVITYDEFAULT_NO)
#define CASESENSITIVITYDEFAULT_NO
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif


const char unz_copyright[] =
   " unzip 0.15 Copyright 1998 Gilles Vollant ";

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct unz_file_info_internal_s
{
    uLong offset_curfile;/* relative offset of local header 4 bytes */
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
typedef struct
{
    char  *read_buffer;         /* internal buffer for compressed data */
    z_stream stream;            /* zLib stream structure for inflate */

    uLong pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong stream_initialised;   /* flag set if stream structure is initialised*/

    uLong offset_local_extrafield;/* offset of the local extra field */
    uInt  size_local_extrafield;/* size of the local extra field */
    uLong pos_local_extrafield;   /* position in the local extra field in read*/

    uLong crc32;                /* crc32 of all data uncompressed */
    uLong crc32_wait;           /* crc32 we must obtain after decompress all */
    uLong rest_read_compressed; /* number of byte to be decompressed */
    uLong rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    DWFInputStream* pStream;   /* io structore of the zipfile */
    uLong compression_method;   /* compression method (0==store) */
    uLong byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
} file_in_zip_read_info_s;


/* unz_s contain internal information about the zipfile
*/
typedef struct
{
    DWFInputStream* pStream;   /* io structore of the zipfile */
    bool bOwnStream;
    unz_global_info gi;         /* public global information */
    uLong byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    uLong num_file;             /* number of the current file in the zipfile*/
    uLong pos_in_central_dir;   /* pos of the current file in the central dir*/
    uLong current_file_ok;      /* flag about the usability of the current file*/
    uLong central_pos;          /* position of the beginning of the central dir*/

    uLong size_central_dir;     /* size of the central directory  */
    uLong offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info cur_file_info; /* public info about the current file in zip*/
    unz_file_info_internal cur_file_info_internal; /* private info about it*/
    file_in_zip_read_info_s* pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
    unzIndex *locator;
} unz_s;

DWFInputStream* ZEXPORT unzGetFilePointer(unzFile uf)
{
    return ((unz_s *)uf)->pStream;
}


/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/


local int unzlocal_getByte( DWFInputStream* pStream, int *pSize)
{
    unsigned char c;
    int err = (int)pStream->read(&c, 1);
    if (err==1)
    {
        *pSize = (int)c;
    }
    //else
    //{
        //if (ferror(fin))
        //    return UNZ_ERRNO;
        //else
        //    return UNZ_EOF;
    //}

    return UNZ_OK;
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local int unzlocal_getShort (DWFInputStream* pStream, uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(pStream,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pStream,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

local int unzlocal_getLong (DWFInputStream* pStream, uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = unzlocal_getByte(pStream,&i);
    x = (uLong)i;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pStream,&i);
    x += ((uLong)i)<<8;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pStream,&i);
    x += ((uLong)i)<<16;

    if (err==UNZ_OK)
        err = unzlocal_getByte(pStream,&i);
    x += ((uLong)i)<<24;

    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}


/* My own strcmpi / strcasecmp */
local int strcmpcasenosensitive_internal (const char* fileName1, const char* fileName2)
{
    for (;;)
    {
        char c1=*(fileName1++);
        char c2=*(fileName2++);
        if ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        if ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        if (c1=='\0')
            return ((c2=='\0') ? 0 : -1);
        if (c2=='\0')
            return 1;
        if (c1<c2)
            return -1;
        if (c1>c2)
            return 1;
    }
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#else
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
#define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal
#endif

/*
   Compare two filename (fileName1,fileName2).
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
                                                                or strcasecmp)
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
        (like 1 on Unix, 2 on Windows)

*/
int ZEXPORT unzStringFileNameCompare (
    const char* fileName1,
    const char* fileName2,
    int iCaseSensitivity)
{
    if (iCaseSensitivity==0)
        iCaseSensitivity=CASESENSITIVITYDEFAULTVALUE;

    if (iCaseSensitivity==1)
        return strcmp(fileName1,fileName2);

    return STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#define BUFREADCOMMENT (0x400)

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local uLong unzlocal_SearchCentralDir( DWFInputStream* pStream )
{
    unsigned char* buf;
    uLong uSizeFile;
    uLong uBackRead;
    uLong uMaxBack=0xffff; /* maximum size of global comment */
    uLong uPosFound=0;


    pStream->seek(SEEK_SET, 0);
    uSizeFile = (uLong)pStream->available();


    if (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
    if (buf==NULL)
        return 0;

    uBackRead = 4;
    while (uBackRead<uMaxBack)
    {
        uLong uReadSize,uReadPos ;
        int i;
        if (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);

        pStream->seek( SEEK_SET, uReadPos );

        if (pStream->read(buf,(uInt)uReadSize) != uReadSize)
        {
            break;
        }

        for (i=(int)uReadSize-3; (i--)>0;)
            if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+i;
                break;
            }

        if (uPosFound!=0)
            break;
    }
    TRYFREE(buf);
    return uPosFound;
}

local struct locatorEntry* newLocatorEntry(
    char* filename,
    unsigned long num_file,
    unsigned long pos_in_central_dir)
{
    struct locatorEntry *entry = (struct locatorEntry*) ALLOC(sizeof(struct locatorEntry));
    if (entry==NULL)
        return NULL;
    entry->filename = filename;
    entry->num_file = num_file;
    entry->pos_in_central_dir = pos_in_central_dir;
    return entry;
}

local int compareLocator(const void* locator1, const void* locator2)
{
    const char *fname1 = (*((struct locatorEntry**)locator1))->filename;
    const char *fname2 = (*((struct locatorEntry**)locator2))->filename;
    return DWFCORE_COMPARE_ASCII_STRINGS(fname1, fname2);
}

local unzFile unzLocal_OpenFile(const DWFString& zPath, unzIndex* locator)
{
    unz_s us;
    unz_s *s;
    uLong central_pos,uL;

    uLong number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    uLong number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */
    int err=UNZ_OK;

    if (unz_copyright[0]!=' ')
        return NULL;

    //
    // open the file - works on all platforms
    //
    DWFStreamFileDescriptor* pFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(zPath, "rb") );
    if (pFile == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file descriptor" );
    }

    try
    {
        pFile->open();
    }
    catch (...)
    {
        DWFCORE_FREE_OBJECT( pFile );
        throw;
    }

    //
    // create the stream
    //
    DWFFileInputStream* pStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
    if (pStream == NULL)
    {
        pFile->close();
        DWFCORE_FREE_OBJECT( pFile );
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file stream" );
    }

    pStream->attach( pFile, true );


    central_pos = unzlocal_SearchCentralDir( pStream );
    if (central_pos==0)
        err=UNZ_ERRNO;

    pStream->seek( SEEK_SET, central_pos );

    /* the signature, already checked */
    if (unzlocal_getLong(pStream,&uL)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of this disk */
    if (unzlocal_getShort(pStream,&number_disk)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of the disk with the start of the central directory */
    if (unzlocal_getShort(pStream,&number_disk_with_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir on this disk */
    if (unzlocal_getShort(pStream,&us.gi.number_entry)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir */
    if (unzlocal_getShort(pStream,&number_entry_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((number_entry_CD!=us.gi.number_entry) ||
        (number_disk_with_CD!=0) ||
        (number_disk!=0))
        err=UNZ_BADZIPFILE;

    /* size of the central directory */
    if (unzlocal_getLong(pStream,&us.size_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* offset of start of central directory with respect to the
          starting disk number */
    if (unzlocal_getLong(pStream,&us.offset_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* zipfile comment length */
    if (unzlocal_getShort(pStream,&us.gi.size_comment)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    if (err!=UNZ_OK)
    {
        DWFCORE_FREE_OBJECT( pStream );

        if (err == UNZ_BADZIPFILE)
        {
            _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"Bad zip file" );
        }

        return NULL;
    }

    us.pStream = pStream;
    us.bOwnStream = true;
    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;

    s=(unz_s*)ALLOC(sizeof(unz_s));
    *s=us;

    s->locator = locator;

    unzGoToFirstFile((unzFile)s);

    //Build an index of file names and their zip offsets.
    if (locator && locator->locatorArray == NULL)
    {
        s->locator->locatorArray = (struct locatorEntry**)ALLOC(us.gi.number_entry * sizeof(struct locatorEntry*));
        s->locator->locatorCount = 0;

        while (err == UNZ_OK)
        {
            unsigned long j=0, i=0;
            char *filename, szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
            unzGetCurrentFileInfo(s,NULL,
                                    szCurrentFileName,sizeof(szCurrentFileName)-1,
                                    NULL,0,NULL,0);
            //canonicalize the filename
            while(szCurrentFileName[i]=='/' || szCurrentFileName[i]=='\\')
                i++;
            for(;i<strlen(szCurrentFileName);i++)
            {
                if (szCurrentFileName[i]!='/')
                    szCurrentFileName[j++] = szCurrentFileName[i];
                else
                    szCurrentFileName[j++] = '\\';
            }
            szCurrentFileName[j] = 0;
            filename = (char*)ALLOC(strlen(szCurrentFileName)+1);
            strcpy(filename, szCurrentFileName);

            s->locator->locatorArray[s->locator->locatorCount++] = newLocatorEntry(
                filename,
                s->num_file,
                s->pos_in_central_dir);

            err = unzGoToNextFile((unzFile)s);
        }

        //Now that we've built the index, sort it
        qsort(s->locator->locatorArray, s->locator->locatorCount, sizeof(struct locatorEntry*), compareLocator);

        unzGoToFirstFile((unzFile)s);
    }

    return (unzFile)s;
}

local unzFile unzLocal_OpenStream(DWFInputStream& rStream, unzIndex *locator)
{
    unz_s us;
    unz_s *s;
    uLong central_pos,uL;

    uLong number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    uLong number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */
    int err=UNZ_OK;

    if (unz_copyright[0]!=' ')
        return NULL;

    DWFInputStream* pStream = &rStream;

    central_pos = unzlocal_SearchCentralDir( pStream );
    if (central_pos==0)
        err=UNZ_ERRNO;

    pStream->seek( SEEK_SET, central_pos );

    /* the signature, already checked */
    if (unzlocal_getLong(pStream,&uL)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of this disk */
    if (unzlocal_getShort(pStream,&number_disk)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* number of the disk with the start of the central directory */
    if (unzlocal_getShort(pStream,&number_disk_with_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir on this disk */
    if (unzlocal_getShort(pStream,&us.gi.number_entry)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* total number of entries in the central dir */
    if (unzlocal_getShort(pStream,&number_entry_CD)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((number_entry_CD!=us.gi.number_entry) ||
        (number_disk_with_CD!=0) ||
        (number_disk!=0))
        err=UNZ_BADZIPFILE;

    /* size of the central directory */
    if (unzlocal_getLong(pStream,&us.size_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* offset of start of central directory with respect to the
          starting disk number */
    if (unzlocal_getLong(pStream,&us.offset_central_dir)!=UNZ_OK)
        err=UNZ_ERRNO;

    /* zipfile comment length */
    if (unzlocal_getShort(pStream,&us.gi.size_comment)!=UNZ_OK)
        err=UNZ_ERRNO;

    if ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    if (err!=UNZ_OK)
    {
        if (err == UNZ_BADZIPFILE)
        {
            _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"Bad zip file" );
        }

        return NULL;
    }

    us.pStream = pStream;
    us.bOwnStream = false;

    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;

    s=(unz_s*)ALLOC(sizeof(unz_s));
    *s=us;

    s->locator = locator;

    unzGoToFirstFile((unzFile)s);

    //Build an index of file names and their zip offsets.
    if (locator && locator->locatorArray == NULL)
    {
        s->locator->locatorArray = (struct locatorEntry**)ALLOC(us.gi.number_entry * sizeof(struct locatorEntry*));
        s->locator->locatorCount = 0;

        while (err == UNZ_OK)
        {
            unsigned long j=0, i=0;
            char *filename, szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
            unzGetCurrentFileInfo(s,NULL,
                                    szCurrentFileName,sizeof(szCurrentFileName)-1,
                                    NULL,0,NULL,0);
            //canonicalize the filename
            while(szCurrentFileName[i]=='/' || szCurrentFileName[i]=='\\')
                i++;
            for(;i<strlen(szCurrentFileName);i++)
            {
                if (szCurrentFileName[i]!='/')
                    szCurrentFileName[j++] = szCurrentFileName[i];
                else
                    szCurrentFileName[j++] = '\\';
            }
            szCurrentFileName[j] = 0;
            filename = (char*)ALLOC(strlen(szCurrentFileName)+1);
            strcpy(filename, szCurrentFileName);

            s->locator->locatorArray[s->locator->locatorCount++] = newLocatorEntry(
                filename,
                s->num_file,
                s->pos_in_central_dir);

            err = unzGoToNextFile((unzFile)s);
        }

        //Now that we've built the index, sort it
        qsort(s->locator->locatorArray, s->locator->locatorCount, sizeof(struct locatorEntry*), compareLocator);

        unzGoToFirstFile((unzFile)s);
    }

    return (unzFile)s;
}

/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows NT computer "c:\\test\\zlib109.zip" or on an Unix computer
     "zlib/zlib109.zip".
     If the zipfile cannot be opened (file don't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/
unzFile ZEXPORT unzOpenFile ( const DWFString& zPath, unzIndex* pIndex )
{
    return unzLocal_OpenFile(zPath, pIndex);
}

unzFile ZEXPORT unzOpenStream ( DWFInputStream& rStream, unzIndex* pIndex )
{
    return unzLocal_OpenStream(rStream, pIndex);
}

/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
int ZEXPORT unzClose (unzFile file)
{
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    if (s->pfile_in_zip_read!=NULL)
        unzCloseCurrentFile(file);

    if (s->bOwnStream)
    {
        DWFCORE_FREE_OBJECT( s->pStream );
    }

    s->locator = NULL;
    TRYFREE(s);
    return UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
int ZEXPORT unzGetGlobalInfo (unzFile file, unz_global_info *pglobal_info)
{
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    *pglobal_info=s->gi;
    return UNZ_OK;
}


/*
   Translate date/time from Dos format to tm_unz (readable more easilty)
*/
local void unzlocal_DosDateToTmuDate (uLong ulDosDate, tm_unz* ptm)
{
    uLong uDate;
    uDate = (uLong)(ulDosDate>>16);
    ptm->tm_mday = (uInt)(uDate&0x1f) ;
    ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
}

/*
   Translate date/time from Dos format to tm_unz (readable more easilty)
*/
void ZEXPORT unzDosDateToTmuDate (uLong ulDosDate, tm_unz* ptm)
{

    unzlocal_DosDateToTmuDate(ulDosDate, ptm);
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local int unzlocal_GetCurrentFileInfoInternal OF((unzFile file,
                                                  unz_file_info *pfile_info,
                                                  unz_file_info_internal
                                                  *pfile_info_internal,
                                                  char *szFileName,
                                                  uLong fileNameBufferSize,
                                                  void *extraField,
                                                  uLong extraFieldBufferSize,
                                                  char *szComment,
                                                  uLong commentBufferSize));

local int unzlocal_GetCurrentFileInfoInternal (
    unzFile file,
    unz_file_info *pfile_info,
    unz_file_info_internal *pfile_info_internal,
    char *szFileName,
    uLong fileNameBufferSize,
    void *extraField,
    uLong extraFieldBufferSize,
    char *szComment,
    uLong commentBufferSize)
{
    unz_s* s;
    unz_file_info file_info;
    unz_file_info_internal file_info_internal;
    int err=UNZ_OK;
    uLong uMagic;
    long lSeek=0;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    s->pStream->seek(SEEK_SET, s->pos_in_central_dir+s->byte_before_the_zipfile);

    /* we check the magic */
    if (err==UNZ_OK)
        if (unzlocal_getLong(s->pStream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        else if (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;

    if (unzlocal_getShort(s->pStream,&file_info.version) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.version_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.compression_method) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unzlocal_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

    if (unzlocal_getLong(s->pStream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&file_info.compressed_size) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&file_info.uncompressed_size) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.size_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.size_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.size_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.disk_num_start) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&file_info.internal_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&file_info_internal.offset_curfile) != UNZ_OK)
        err=UNZ_ERRNO;

    lSeek+=file_info.size_filename;
    if ((err==UNZ_OK) && (szFileName!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_filename<fileNameBufferSize)
        {
            *(szFileName+file_info.size_filename)='\0';
            uSizeRead = file_info.size_filename;
        }
        else
            uSizeRead = fileNameBufferSize;

        if ((file_info.size_filename>0) && (fileNameBufferSize>0))
            if (s->pStream->read(szFileName,(uInt)uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek -= uSizeRead;
    }


    if ((err==UNZ_OK) && (extraField!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_file_extra<extraFieldBufferSize)
            uSizeRead = file_info.size_file_extra;
        else
            uSizeRead = extraFieldBufferSize;

        if (lSeek!=0)
        {
            s->pStream->seek( SEEK_CUR, lSeek );
            lSeek = 0;
        }
        if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
            if (s->pStream->read(extraField,(uInt)uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek += file_info.size_file_extra - uSizeRead;
    }
    else
        lSeek+=file_info.size_file_extra;

    file_info.file_header_offset = s->cur_file_info_internal.offset_curfile;
    file_info.file_data_offset = file_info.file_header_offset
                               + SIZEZIPLOCALHEADER
                               + file_info.size_filename
                               + file_info.size_file_extra;

    if ((err==UNZ_OK) && (szComment!=NULL))
    {
        uLong uSizeRead ;
        if (file_info.size_file_comment<commentBufferSize)
        {
            *(szComment+file_info.size_file_comment)='\0';
            uSizeRead = file_info.size_file_comment;
        }
        else
            uSizeRead = commentBufferSize;

        if (lSeek!=0)
        {
            s->pStream->seek( SEEK_CUR, lSeek );
            lSeek = 0;
        }
        if ((file_info.size_file_comment>0) && (commentBufferSize>0))
            if (s->pStream->read(szComment,(uInt)uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek+=file_info.size_file_comment - uSizeRead;
    }
    else
        lSeek+=file_info.size_file_comment;

    if ((err==UNZ_OK) && (pfile_info!=NULL))
        *pfile_info=file_info;

    if ((err==UNZ_OK) && (pfile_info_internal!=NULL))
        *pfile_info_internal=file_info_internal;

    return err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
int ZEXPORT unzGetCurrentFileInfo (
    unzFile file,
    unz_file_info *pfile_info,
    char *szFileName,
    uLong fileNameBufferSize,
    void *extraField,
    uLong extraFieldBufferSize,
    char *szComment,
    uLong commentBufferSize)
{
    return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,
                                                szFileName,fileNameBufferSize,
                                                extraField,extraFieldBufferSize,
                                                szComment,commentBufferSize);
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
int ZEXPORT unzGoToFirstFile (unzFile file)
{
    int err=UNZ_OK;
    unz_s* s;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    s->pos_in_central_dir=s->offset_central_dir;
    s->num_file=0;
    err=unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                             &s->cur_file_info_internal,
                                             NULL,0,NULL,0,NULL,0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}


/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
int ZEXPORT unzGoToNextFile (unzFile file)
{
    unz_s* s;
    int err;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;
    if (s->num_file+1==s->gi.number_entry)
        return UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
            s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
    s->num_file++;
    err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               NULL,0,NULL,0,NULL,0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

local int isLocator(const void* key, const void* locator)
{
    const char *fname1 = (const char*)key;
    const char *fname2 = (*((struct locatorEntry**)locator))->filename;
    return DWFCORE_COMPARE_ASCII_STRINGS(fname1, fname2);
}

/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzipStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
int ZEXPORT unzLocateFile (
    unzFile file,
    const DWFString& zFilename,
    int iCaseSensitivity)
{
    unz_s* s;
    int err;

    uLong num_fileSaved;
    uLong pos_in_central_dirSaved;
    char buf[UNZ_MAXFILENAMEINZIP+1];
    unsigned long j=0;
    unsigned long i=0;
    struct locatorEntry **fileLocator;

    if (file==NULL)
        return UNZ_PARAMERROR;

    //
    // get the utf-8 rep of the filename
    //
    ASCII_char_t szFileName[UNZ_MAXFILENAMEINZIP+1] = {0};
    zFilename.getUTF8( (ASCII_char_t*)&szFileName, UNZ_MAXFILENAMEINZIP );

    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    num_fileSaved = s->num_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;

    //canonicalize the filename - strip leading (back)slashes, change all slashes to backslashes
    while(szFileName[i]=='/' || szFileName[i]=='\\')
        i++;
    for(;i<strlen(szFileName);i++)
    {
        if (szFileName[i]!='/')
            buf[j++] = szFileName[i];
        else
            buf[j++] = '\\';
    }
    buf[j] = 0;

    fileLocator =
        (struct locatorEntry**)bsearch(buf, s->locator->locatorArray, s->locator->locatorCount, sizeof(struct locatorEntry*), isLocator);

    if (fileLocator != NULL && *fileLocator!=NULL)
    {
        s->pos_in_central_dir = (*fileLocator)->pos_in_central_dir;
        s->num_file = (*fileLocator)->num_file;
        s->current_file_ok = true;

        err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                                &s->cur_file_info_internal,
                                                NULL,0,NULL,0,NULL,0);
        return err;
    }

    //we should have found the file
    err = unzGoToFirstFile(file);
    while (err == UNZ_OK)
    {
        char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        unzGetCurrentFileInfo(file,NULL,
                                szCurrentFileName,sizeof(szCurrentFileName)-1,
                                NULL,0,
                                NULL,0);
        j=0;
        i=0;
        //canonicalize the filename
        while(szCurrentFileName[i]=='/' || szCurrentFileName[i]=='\\')
            i++;
        for(;i<strlen(szCurrentFileName);i++)
        {
            if (szCurrentFileName[i]!='/')
                szCurrentFileName[j++] = szCurrentFileName[i];
            else
                szCurrentFileName[j++] = '\\';
        }
        szCurrentFileName[j] = 0;

        if (unzStringFileNameCompare(szCurrentFileName,
                                        buf,iCaseSensitivity)==0)
            return UNZ_OK;
        err = unzGoToNextFile(file);
    }

    s->num_file = num_fileSaved ;
    s->pos_in_central_dir = pos_in_central_dirSaved ;
    return err;
}


/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
        directory about this file
  store in *piSizeVar the size of extra info in local header
        (filename and size of extra field data)
*/
local int unzlocal_CheckCurrentFileCoherencyHeader (
    unz_s* s,
    uInt* piSizeVar,
    uLong *poffset_local_extrafield,
    uInt  *psize_local_extrafield)
{
    uLong uMagic,uData,uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int err=UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    s->pStream->seek( SEEK_SET, s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile );


    if (err==UNZ_OK)
        if (unzlocal_getLong(s->pStream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        else if (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;

    if (unzlocal_getShort(s->pStream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    if (unzlocal_getShort(s->pStream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    if (unzlocal_getShort(s->pStream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    if ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(s->pStream,&uData) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    if (unzlocal_getLong(s->pStream,&uData) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(s->pStream,&uData) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    if (unzlocal_getLong(s->pStream,&uData) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) &&
                              ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;


    if (unzlocal_getShort(s->pStream,&size_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    else if ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
        err=UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    if (unzlocal_getShort(s->pStream,&size_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
int ZEXPORT unzOpenCurrentFile (unzFile file, const DWFString& zPassword, const DWFString& zFilename )
{
    char encryption_header[12];
    int err=UNZ_OK;
    int Store;
    uInt iSizeVar;
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    uLong offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */

    if (file==NULL)
        return UNZ_PARAMERROR;

    s=(unz_s*)file;
    if (!s->current_file_ok)
        return UNZ_PARAMERROR;

    if (s->pfile_in_zip_read != NULL)
        unzCloseCurrentFile(file);

    if (unzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar,
                &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
        return UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip_read_info_s*)
                                        ALLOC(sizeof(file_in_zip_read_info_s));
    if (pfile_in_zip_read_info==NULL)
        return UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_buffer=(char*)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;


    unsigned char dwf_extra[4] = {0};

    if (size_local_extrafield == 4)
    {
        off_t iCur = s->pStream->seek( SEEK_SET, offset_local_extrafield );
        s->pStream->read( dwf_extra, size_local_extrafield );
        s->pStream->seek( SEEK_SET, iCur );
    }

    if (pfile_in_zip_read_info->read_buffer==NULL)
    {
        TRYFREE(pfile_in_zip_read_info);
        return UNZ_INTERNALERROR;
    }

        //
        // Use password if provided
        //
    size_t nPasswordChars = zPassword.chars();
    if (nPasswordChars > 0)
    {
        char* pUTF8Password = NULL;
        zPassword.getUTF8( &pUTF8Password );

        if ((dwf_extra[3] & kzDWFSaltedPasswordMask)  &&
            (dwf_extra[1] == kzDWFLocalFileHeader[1]) &&
            (dwf_extra[2] == kzDWFLocalFileHeader[2]) &&
            (dwf_extra[0] == kzDWFLocalFileHeader[0]) && 
            (zFilename.chars() > 0))
        {
            char* pUTF8Filename = NULL;
            zFilename.getUTF8( &pUTF8Filename );

            dwf_salt_init_keys( pUTF8Filename, pUTF8Password, s->cur_file_info.key );
            
            DWFCORE_FREE_MEMORY( pUTF8Filename );
        }
        else
        {
            init_keys( pUTF8Password, s->cur_file_info.key );
        }

        DWFCORE_FREE_MEMORY( pUTF8Password );
    }
    else
    {
        init_keys( "", s->cur_file_info.key );
    }

    pfile_in_zip_read_info->stream_initialised=0;

    if ((s->cur_file_info.compression_method!=0) &&
        (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;
    Store = s->cur_file_info.compression_method==0;

    pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
    pfile_in_zip_read_info->crc32=0;
    pfile_in_zip_read_info->compression_method =
            s->cur_file_info.compression_method;
    pfile_in_zip_read_info->pStream=s->pStream;
    pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

    if (!Store)
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;

      err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
      if (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=1;
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_in_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_size ;
    pfile_in_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_size ;

    if (s->cur_file_info.flag & 0x01) /*file is encrypted*/
    {
        //Read the 12 byte encryption header, init the keys

        pfile_in_zip_read_info->pStream->seek( SEEK_SET, s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar );
        pfile_in_zip_read_info->pStream->read( encryption_header, 12 );

        decrypt_buffer(encryption_header, 12, s->cur_file_info.key);
        pfile_in_zip_read_info->rest_read_compressed -= 12;
        iSizeVar += 12;
    }

    pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
              iSizeVar;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;

    s->pfile_in_zip_read = pfile_in_zip_read_info;
    return UNZ_OK;
}


/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
int ZEXPORT unzReadCurrentFile (
    unzFile file,
    voidp buf,
    unsigned len)
{
    int err=UNZ_OK;
    uInt iRead = 0;
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;


    if ((pfile_in_zip_read_info->read_buffer == NULL))
        return UNZ_END_OF_LIST_OF_FILE;
    if (len==0)
        return 0;

    pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    if (len>pfile_in_zip_read_info->rest_read_uncompressed)
        pfile_in_zip_read_info->stream.avail_out =
          (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    while (pfile_in_zip_read_info->stream.avail_out>0)
    {
        if ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            if (uReadThis != 0)
            {
                pfile_in_zip_read_info->pStream->seek( SEEK_SET, pfile_in_zip_read_info->pos_in_zipfile +
                                                                                pfile_in_zip_read_info->byte_before_the_zipfile );

                pfile_in_zip_read_info->pStream->read( pfile_in_zip_read_info->read_buffer, uReadThis );

                if (s->cur_file_info.flag & 0x01) /*file is encrypted*/
                {
                    decrypt_buffer(pfile_in_zip_read_info->read_buffer,uReadThis, s->cur_file_info.key);
                }

                pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

                pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

                pfile_in_zip_read_info->stream.next_in =
                    (Bytef*)pfile_in_zip_read_info->read_buffer;
                pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
            }
        }

        if (pfile_in_zip_read_info->compression_method==0)
        {
            uInt uDoCopy,i ;
            if (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            else
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            for (i=0;i<uDoCopy;i++)
                *(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                pfile_in_zip_read_info->stream.next_out,
                                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.next_out += uDoCopy;
            pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        }
        else
        {
            uLong uTotalOutBefore,uTotalOutAfter;
            const Bytef *bufBefore;
            uLong uOutThis;
            int flush=Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_in_zip_read_info->stream,flush);

            if (err == Z_DATA_ERROR
                && uTotalOutBefore==0
                && s->cur_file_info.flag & 0x01) /*file is encrypted*/
            {
                return UNZ_BADPASSWORD;
            }

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)uOutThis;

            if (s->cur_file_info.flag & 0x01 /*file is encrypted*/
                     && err == Z_STREAM_END
                     && pfile_in_zip_read_info->stream.avail_out > 0
                     && pfile_in_zip_read_info->rest_read_uncompressed > 0
                     && pfile_in_zip_read_info->stream.avail_out != uOutThis)
            {
                return UNZ_BADPASSWORD;
            }


            if (err==Z_STREAM_END)
                return (iRead==0) ? UNZ_EOF : iRead;
            if (err!=Z_OK)
                break;
        }
    }

    if (err==Z_OK)
        return iRead;
    return err;
}


/*
  Give the current position in uncompressed data
*/
z_off_t ZEXPORT unztell (unzFile file)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
int ZEXPORT unzeof (unzFile file)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        return 1;
    else
        return 0;
}



/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
    more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/
int ZEXPORT unzGetLocalExtrafield (
    unzFile file,
    voidp buf,
    unsigned len)
{
    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    uInt read_now;
    uLong size_to_read;

    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;

    size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
                pfile_in_zip_read_info->pos_local_extrafield);

    if (buf==NULL)
        return (int)size_to_read;

    if (len>size_to_read)
        read_now = (uInt)size_to_read;
    else
        read_now = (uInt)len ;

    if (read_now==0)
        return 0;

    pfile_in_zip_read_info->pStream->seek( SEEK_SET, pfile_in_zip_read_info->offset_local_extrafield +
                                                                    pfile_in_zip_read_info->pos_local_extrafield );

    pfile_in_zip_read_info->pStream->read( buf, (uInt)size_to_read );

    return (int)read_now;
}

/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
int ZEXPORT unzCloseCurrentFile (unzFile file)
{
    int err=UNZ_OK;

    unz_s* s;
    file_in_zip_read_info_s* pfile_in_zip_read_info;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    if (pfile_in_zip_read_info==NULL)
        return UNZ_PARAMERROR;


    if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
    {
        if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err=UNZ_CRCERROR;
    }


    TRYFREE(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = NULL;
    if (pfile_in_zip_read_info->stream_initialised)
        inflateEnd(&pfile_in_zip_read_info->stream);

    pfile_in_zip_read_info->stream_initialised = 0;
    TRYFREE(pfile_in_zip_read_info);

    s->pfile_in_zip_read=NULL;

    return err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
int ZEXPORT unzGetGlobalComment (
    unzFile file,
    char *szComment,
    uLong uSizeBuf)
{
    unz_s* s;
    uLong uReadThis ;
    if (file==NULL)
        return UNZ_PARAMERROR;
    s=(unz_s*)file;

    uReadThis = uSizeBuf;
    if (uReadThis>s->gi.size_comment)
        uReadThis = s->gi.size_comment;

    s->pStream->seek( SEEK_SET, s->central_pos+22 );

    if (uReadThis>0)
    {
      *szComment='\0';

      if (s->pStream->read(szComment,(uInt)uReadThis)!=uReadThis)
        return UNZ_ERRNO;
    }

    if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment))
        *(szComment+s->gi.size_comment)='\0';
    return (int)uReadThis;
}

/*
int ZEXPORT unzIsEncrypted(const wchar_t *path)
{
    int result=0;
    unzFile file = unzLocal_OpenFile(path, false);
    unz_file_info file_info;

    if (file==NULL)
        return UNZ_PARAMERROR;

    result = unzGoToFirstFile(file);

    if (result==UNZ_OK)
    {
        result = unzGetCurrentFileInfo(file, &file_info, 0, 0, 0, 0, 0, 0);
        if (result==UNZ_OK)
            result = file_info.flag & 0x01;
    }

    unzClose(file);
    return result;
}
*/

int ZEXPORT unzIsEncrypted(unzFile file)
{
    int result=0;
    unz_file_info file_info;

    if (file==NULL)
        return UNZ_PARAMERROR;

    result = unzGoToFirstFile(file);

    if (result==UNZ_OK)
    {
        result = unzGetCurrentFileInfo(file, &file_info, 0, 0, 0, 0, 0, 0);
        if (result==UNZ_OK)
            result = file_info.flag & 0x01;
    }

    return result;
}
