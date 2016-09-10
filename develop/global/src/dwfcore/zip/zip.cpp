/* zip.c -- IO on .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Read zip.h for more info
*/


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

#include "dwfcore/zip/zip.h"
#ifdef  DWFCORE_BUILD_ZLIB
#include "dwfcore/zlib/zutil.h"
#endif
#include "dwfcore/zip/password.h"
#include <time.h>
#include <sys/timeb.h>

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


#include "dwfcore/StreamFileDescriptor.h"
using namespace DWFCore;


#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x14) /* platform depedent */
#endif

#ifndef Z_MAXFILENAMEINZIP
#define Z_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

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


const char zip_copyright[] =
   " zip 0.15 Copyright 1998 Gilles Vollant ";


#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define EXTHEADERMAGIC      (0x08074b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

local linkedlist_datablock_internal* allocate_new_datablock()
{
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(sizeof(linkedlist_datablock_internal));
    if (ldi!=NULL)
    {
        ldi->next_datablock = NULL ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    return ldi;
}

local void free_datablock(linkedlist_datablock_internal* ldi)
{
    while (ldi!=NULL)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        TRYFREE(ldi);
        ldi = ldinext;
    }
}

local void init_linkedlist(linkedlist_data* ll)
{
    ll->first_block = ll->last_block = NULL;
}

//local void free_linkedlist( linkedlist_data* ll)
//{
//    free_datablock(ll->first_block);
//    ll->first_block = ll->last_block = NULL;
//}

local int add_data_in_datablock( linkedlist_data* ll, const void* buf, uLong len, void** where)
{
    linkedlist_datablock_internal* ldi;
    const unsigned char* from_copy;

    if (ll==NULL)
        return ZIP_INTERNALERROR;

    if (ll->last_block == NULL)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        if (ll->first_block == NULL)
            return ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (unsigned char*)buf;

    while (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned char* to_copy;

        if (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            if (ldi->next_datablock == NULL)
                return ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        if (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        else
            copy_this = (uInt)len;

        *where = to_copy = &(ldi->data[ldi->filled_in_this_block]);

        for (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    return ZIP_OK;
}


//local int write_datablock(FILE * fout, linkedlist_data* ll)
//{
//    linkedlist_datablock_internal* ldi;
//    ldi = ll->first_block;
//    while (ldi!=NULL)
//    {
//        if (ldi->filled_in_this_block > 0)
//            if (fwrite(ldi->data,(uInt)ldi->filled_in_this_block,1,fout)!=1)
//                return ZIP_ERRNO;
//        ldi = ldi->next_datablock;
//    }
//    return ZIP_OK;
//}

/****************************************************************************/

/* ===========================================================================
   Outputs a long in LSB order to the given file
   nbByte == 1, 2 or 4 (byte, short or long)
*/

local int ziplocal_putValue OF((DWFFileDescriptor* file, uLong x, int nbByte));
local int ziplocal_putValue (DWFFileDescriptor* file, uLong x, int nbByte)
{
    unsigned char buf[4];
    int n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned char)(x & 0xff);
        x >>= 8;
    }
    //if (fwrite(buf,nbByte,1,file)!=1)
    if (file->write(buf, (size_t)nbByte) != (size_t)nbByte)
        return ZIP_ERRNO;
    else
        return ZIP_OK;
}

local void ziplocal_putValue_inmemory OF((void* dest, uLong x, int nbByte));
local void ziplocal_putValue_inmemory (void* dest, uLong x, int nbByte)
{
    unsigned char* buf=(unsigned char*)dest;
    int n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned char)(x & 0xff);
        x >>= 8;
    }
}
/****************************************************************************/


local uLong ziplocal_TmzDateToDosDate(tm_zip* ptm)
{
    uLong year = (uLong)ptm->tm_year;
    if (year>1980)
        year-=1980;
    else if (year>80)
        year-=80;
    return
      (uLong) (((ptm->tm_mday) + (32 * (ptm->tm_mon+1)) + (512 * year)) << 16) |
        ((ptm->tm_sec/2) + (32* ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

zipFile _DWFCORE_API dwfOpen ( const DWFString& zPath, unsigned int nMajorRevision, unsigned int nMinorRevision )
{
    char buf[16] = {0};
    ::sprintf( buf, "(DWF V%02d.%02d)", nMajorRevision, nMinorRevision );

    return zipOpen( zPath, buf, ::strlen(buf) );
}

zipFile ZEXPORT zipOpenStream ( DWFFileDescriptor* pDescriptor, const void* pHeader, size_t nHeaderBytes )
{
    zip_internal ziinit;

    ziinit.filezip = pDescriptor;
    ziinit.own_descriptor = false;

    if (ziinit.filezip == NULL)
        return NULL;

    return zipOpenArchive( ziinit, pHeader, nHeaderBytes );
}

zipFile ZEXPORT zipOpen ( const DWFString& zPath, const void* pHeader, size_t nHeaderBytes )
{
    zip_internal ziinit;

    ziinit.filezip = NULL;
    //
    // open the file - works on all platforms
    //
    ziinit.filezip = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(zPath, "wb") );

    if (ziinit.filezip == NULL)
        return NULL;
    else
        ziinit.own_descriptor = true;

    return zipOpenArchive( ziinit, pHeader, nHeaderBytes );
}

zipFile ZEXPORT zipOpenArchive ( zip_internal& ziinit, const void* pHeader, size_t nHeaderBytes )
{
    zip_internal* zi;

    ziinit.filezip->open();

        //
        // custom header added
        //
    if ((pHeader != NULL) && (nHeaderBytes > 0))
    {
        if (nHeaderBytes != ziinit.filezip->write(pHeader, nHeaderBytes))
        {
            ziinit.filezip->close();
            DWFCORE_FREE_OBJECT( ziinit.filezip );
            ziinit.filezip = NULL;
            return NULL;
        }

        ziinit.begin_pos = (uLong)nHeaderBytes;
    }
    else
    {
        ziinit.begin_pos = (uLong)0;
    }

    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.number_entry = 0;
    init_linkedlist(&(ziinit.central_dir));


    zi = (zip_internal*)ALLOC(sizeof(zip_internal));
    if (zi==NULL)
    {
            ziinit.filezip->close();
            DWFCORE_FREE_OBJECT( ziinit.filezip );
            ziinit.filezip = NULL;
            return NULL;
    }

    *zi = ziinit;
    return (zipFile)zi;
}

int ZEXPORT zipOpenNewFileInZip (
    zipFile file,
    const DWFString& filename,
    const zip_fileinfo* zipfi,
    const void* extrafield_local,
    uInt size_extrafield_local,
    const void* extrafield_global,
    uInt size_extrafield_global,
    const DWFString& comment,
    int method,
    int level,
    const DWFString& password)
{
    char encryption_header[12];
    zip_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    int err = ZIP_OK;

    if (file == NULL)
        return ZIP_PARAMERROR;
    if ((method!=0) && (method!=Z_DEFLATED))
        return ZIP_PARAMERROR;

    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        if (err != ZIP_OK)
            return err;
    }

    char filebuf[4096] = {0};
    if (filename.chars()==0)
    {
        filebuf[0] = '-';
        filebuf[1] = 0;
        size_filename = 1;
    }
    else
    {
        size_filename = (uInt)filename.getUTF8( filebuf, 4096 );
    }

    char commentbuf[1024] = {0};
    if (comment.chars() == 0)
    {
        size_comment = 0;
    }
    else
    {
        size_comment = (uInt)comment.getUTF8( commentbuf, 1024 );
    }

    char pwdbuf[1024] = {0};
    size_t pwdbuf_len = 0;
    if (password.chars() > 0)
    {
        pwdbuf_len = (uInt)password.getUTF8( pwdbuf, 1024 );
    }

    if (zipfi == NULL)
    {
        //use 'now' as the default time
        tm_zip tmzdate;
        time_t now;
        time(&now);
        tmzdate.tm_hour = localtime(&now)->tm_hour;
        tmzdate.tm_mday = localtime(&now)->tm_mday;
        tmzdate.tm_min = localtime(&now)->tm_min;
        tmzdate.tm_mon = localtime(&now)->tm_mon;
        tmzdate.tm_sec = localtime(&now)->tm_sec;
        tmzdate.tm_year = localtime(&now)->tm_year;
        zi->ci.dosDate = ziplocal_TmzDateToDosDate(&tmzdate);
    }
    else
    {
        if (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        else zi->ci.dosDate = ziplocal_TmzDateToDosDate((tm_zip*)&zipfi->tmz_date);
    }

    zi->ci.flag = 0;
    if ((level==8) || (level==9))
      zi->ci.flag |= 2;
    if ((level==2))
      zi->ci.flag |= 4;
    if ((level==1))
      zi->ci.flag |= 6;
    if (pwdbuf_len)
      zi->ci.flag |= 9;

    zi->ci.crc32 = 0;
    zi->ci.method = method;
    zi->ci.stream_initialised = 0;
    zi->ci.pos_in_buffered_data = 0;
    //zi->filezip->flush();
    //zi->ci.pos_local_header = ftell(zi->filezip);
    zi->ci.pos_local_header = zi->filezip->seek( SEEK_CUR, 0 );
    zi->ci.size_centralheader = SIZECENTRALHEADER + size_filename +
                                      size_extrafield_global + size_comment;
    zi->ci.central_header = (char*)ALLOC((uInt)zi->ci.size_centralheader);
    zi->ci.header_data_block = zi->ci.central_header; //for now

    ziplocal_putValue_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    ziplocal_putValue_inmemory(zi->ci.central_header+4,(uLong)VERSIONMADEBY,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+6,(uLong)20,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    ziplocal_putValue_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    ziplocal_putValue_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    ziplocal_putValue_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    ziplocal_putValue_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    if (zipfi==NULL)
        ziplocal_putValue_inmemory(zi->ci.central_header+36,(uLong)0,2);
    else
        ziplocal_putValue_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    if (zipfi==NULL)
        ziplocal_putValue_inmemory(zi->ci.central_header+38,(uLong)0x20/*FILE_ATTRIBUTE_ARCHIVE*/,4);
    else
        ziplocal_putValue_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    ziplocal_putValue_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header,4);

    for (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filebuf+i);

    for (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((const char*)extrafield_global)+i);

    for (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(commentbuf+i);
    if (zi->ci.central_header == NULL)
        return ZIP_INTERNALERROR;

    /* write the local header */
    err = ziplocal_putValue(zi->filezip,(uLong)LOCALHEADERMAGIC,4);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)20,2);/* version needed to extract */
    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.flag,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.method,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.dosDate,4);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); /* crc 32, unknown */
    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); /* compressed size, unknown */
    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); /* uncompressed size, unknown */

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)size_filename,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)size_extrafield_local,2);

    if ((err==ZIP_OK) && (size_filename>0))
        if (zi->filezip->write(filebuf, (uInt)size_filename) != (uInt)size_filename)
                err = ZIP_ERRNO;

    if ((err==ZIP_OK) && (size_extrafield_local>0))
        if (zi->filezip->write(extrafield_local, (uInt)size_extrafield_local) != (uInt)size_extrafield_local)
                err = ZIP_ERRNO;

    //zi->filezip->flush();
    //zi->ci.pos_file_data = ftell(zi->filezip);
    zi->ci.pos_file_data = zi->filezip->seek( SEEK_CUR, 0 );

    if (pwdbuf_len)
    {
        if ((size_extrafield_local == 4) &&
            (((const char*)extrafield_local)[3] & kzDWFSaltedPasswordMask) &&
            (((const char*)extrafield_local)[0] & kzDWFLocalFileHeader[0]) &&
            (((const char*)extrafield_local)[1] & kzDWFLocalFileHeader[1]) &&
            (((const char*)extrafield_local)[2] & kzDWFLocalFileHeader[2]))
        {
            char* pUTF8Filename = NULL;
            filename.getUTF8( &pUTF8Filename );

            dwf_salt_init_keys( pUTF8Filename, pwdbuf, zi->ci.key );

            DWFCORE_FREE_MEMORY( pUTF8Filename );
        }
        else
        {
            init_keys( pwdbuf, zi->ci.key );
        }
        encrypt_header((long)zi->ci.dosDate << 16, encryption_header, zi->ci.key);
        if (zi->filezip->write(encryption_header, 12)!=12)
            err = ZIP_ERRNO;
    }

    zi->ci.stream.avail_in = (uInt)0;
    zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.stream.next_out = zi->ci.buffered_data;
    zi->ci.stream.total_in = 0;
    zi->ci.stream.total_out = 0;

    if ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED))
    {
        zi->ci.stream.zalloc = (alloc_func)0;
        zi->ci.stream.zfree = (free_func)0;
        zi->ci.stream.opaque = (voidpf)0;

        err = deflateInit2(&zi->ci.stream, level,
               Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

        if (err==Z_OK)
            zi->ci.stream_initialised = 1;
    }

    if (pwdbuf_len)
        zi->ci.stream.total_out = 12;

    if (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    return err;
}

int ZEXPORT zipWriteInFileInZip (
    zipFile file,
    const void* buf,
    unsigned len)
{
    zip_internal* zi;
    int err=ZIP_OK;

    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 0)
        return ZIP_PARAMERROR;

    zi->ci.stream.next_in = (Bytef*)buf;
    zi->ci.stream.avail_in = len;
    zi->ci.crc32 = crc32(zi->ci.crc32,(const Bytef*)buf,len);

    while ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
    {
        if (zi->ci.stream.avail_out == 0)
        {
            if (zi->ci.flag & 1)
                encrypt_buffer((char*)zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data, zi->ci.key);
            //if (fwrite(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data,1,zi->filezip)!=1)
            if (zi->filezip->write(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data)!=(uInt)zi->ci.pos_in_buffered_data)
                err = ZIP_ERRNO;
            zi->ci.pos_in_buffered_data = 0;
            zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
            zi->ci.stream.next_out = zi->ci.buffered_data;
        }

        if (zi->ci.method == Z_DEFLATED)
        {
            uLong uTotalOutBefore = zi->ci.stream.total_out;
            err=deflate(&zi->ci.stream,  Z_NO_FLUSH);
            zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;

        }
        else
        {
            uInt copy_this,i;
            if (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                copy_this = zi->ci.stream.avail_in;
            else
                copy_this = zi->ci.stream.avail_out;
            for (i=0;i<copy_this;i++)
                *(((char*)zi->ci.stream.next_out)+i) =
                    *(((const char*)zi->ci.stream.next_in)+i);
            {
                zi->ci.stream.avail_in -= copy_this;
                zi->ci.stream.avail_out-= copy_this;
                zi->ci.stream.next_in+= copy_this;
                zi->ci.stream.next_out+= copy_this;
                zi->ci.stream.total_in+= copy_this;
                zi->ci.stream.total_out+= copy_this;
                zi->ci.pos_in_buffered_data += copy_this;
            }
        }
    }

    return 0;
}

int ZEXPORT zipCloseFileInZip (zipFile file)
{
    zip_internal* zi;
    int err=ZIP_OK;
    void *where = NULL;

    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 0)
        return ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    if (zi->ci.method == Z_DEFLATED)
    {
        while (err==ZIP_OK)
        {
            uLong uTotalOutBefore;
            if (zi->ci.stream.avail_out == 0)
            {
                if (zi->ci.flag & 1)
                    encrypt_buffer((char*)zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data, zi->ci.key);
                //if (fwrite(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data,1,zi->filezip)!=1)
                if (zi->filezip->write(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data)!=(uInt)zi->ci.pos_in_buffered_data)
                    err = ZIP_ERRNO;
                zi->ci.pos_in_buffered_data = 0;
                zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                zi->ci.stream.next_out = zi->ci.buffered_data;
            }
            uTotalOutBefore = zi->ci.stream.total_out;
            err=deflate(&zi->ci.stream,  Z_FINISH);
            zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
        }
    }

    if (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    if ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
    {
        if (zi->ci.flag & 1)
            encrypt_buffer((char*)zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data, zi->ci.key);
        if (zi->filezip->write(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data) != (uInt)zi->ci.pos_in_buffered_data)
//        if (fwrite(zi->ci.buffered_data,(uInt)zi->ci.pos_in_buffered_data,1,zi->filezip)!=1)
            err = ZIP_ERRNO;
    }

    if ((zi->ci.method == Z_DEFLATED) && (err==ZIP_OK))
    {
        err=deflateEnd(&zi->ci.stream);
        zi->ci.stream_initialised = 0;
    }

    //Data descriptor
    if (zi->ci.flag & 8)
    {
        if (zi->ci.flag & 1)
        {
            if (err==ZIP_OK)
                err = ziplocal_putValue(zi->filezip,(uLong)EXTHEADERMAGIC,4);
        }
        if (err==ZIP_OK)
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.crc32,4); /* crc 32, now known */
        if (err==ZIP_OK) /* compressed size, now known */
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.stream.total_out,4);
        if (err==ZIP_OK) /* uncompressed size, now known */
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.stream.total_in,4);
    }

    ziplocal_putValue_inmemory(zi->ci.central_header+16,(uLong)zi->ci.crc32,4); /*crc*/
    ziplocal_putValue_inmemory(zi->ci.central_header+20,
                                (uLong)zi->ci.stream.total_out,4); /*compressed size*/
    ziplocal_putValue_inmemory(zi->ci.central_header+24,
                                (uLong)zi->ci.stream.total_in,4); /*uncompressed size*/

    if (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir,zi->ci.central_header,
                                       (uLong)zi->ci.size_centralheader, &where);

    free(zi->ci.central_header);
    zi->ci.header_data_block = (char*)where;

    if (err==ZIP_OK)
    {
        long cur_pos_inzip = zi->filezip->seek( SEEK_CUR, 0 );
        //if (fseek(zi->filezip, zi->ci.pos_local_header + 14,SEEK_SET)!=0)
        zi->filezip->seek( SEEK_SET, zi->ci.pos_local_header + 14 );
            //err = ZIP_ERRNO;

        if (err==ZIP_OK)
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.crc32,4); /* crc 32, now known */

        if (err==ZIP_OK) /* compressed size, now known */
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.stream.total_out,4);

        if (err==ZIP_OK) /* uncompressed size, now known */
            err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.stream.total_in,4);

        zi->filezip->seek( SEEK_SET, cur_pos_inzip );

        //if (fseek(zi->filezip,
        //          cur_pos_inzip,SEEK_SET)!=0)
        //    err = ZIP_ERRNO;
    }

    zi->number_entry ++;
    zi->in_opened_file_inzip = 0;

    return err;
}

int ZEXPORT zipClose (zipFile file, const char* global_comment)
{
    zip_internal* zi;
    int err = 0;
    uLong size_centraldir = 0;
    uLong centraldir_pos_inzip ;
    uInt size_global_comment;
    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

    if (global_comment==NULL)
        size_global_comment = 0;
    else
        size_global_comment = (uInt)strlen(global_comment);


    //centraldir_pos_inzip = ftell(zi->filezip);
    centraldir_pos_inzip = zi->filezip->seek( SEEK_CUR, 0 );
    if (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block ;
        while (ldi!=NULL)
        {
            if ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
                //if (fwrite(ldi->data,(uInt)ldi->filled_in_this_block,1,zi->filezip) !=1 )
                if (zi->filezip->write(ldi->data,(uInt)ldi->filled_in_this_block) != (uInt)ldi->filled_in_this_block)
                    err = ZIP_ERRNO;

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_datablock(zi->central_dir.first_block);

    if (err==ZIP_OK) /* Magic End */
        err = ziplocal_putValue(zi->filezip,(uLong)ENDHEADERMAGIC,4);

    if (err==ZIP_OK) /* number of this disk */
        err = ziplocal_putValue(zi->filezip,(uLong)0,2);

    if (err==ZIP_OK) /* number of the disk with the start of the central directory */
        err = ziplocal_putValue(zi->filezip,(uLong)0,2);

    if (err==ZIP_OK) /* total number of entries in the central dir on this disk */
        err = ziplocal_putValue(zi->filezip,(uLong)zi->number_entry,2);

    if (err==ZIP_OK) /* total number of entries in the central dir */
        err = ziplocal_putValue(zi->filezip,(uLong)zi->number_entry,2);

    if (err==ZIP_OK) /* size of the central directory */
        err = ziplocal_putValue(zi->filezip,(uLong)size_centraldir,4);

    if (err==ZIP_OK) /* offset of start of central directory with respect to the
                            starting disk number */
        err = ziplocal_putValue(zi->filezip,(uLong)centraldir_pos_inzip ,4);

    if (err==ZIP_OK) /* zipfile comment length */
        err = ziplocal_putValue(zi->filezip,(uLong)size_global_comment,2);

    if ((err==ZIP_OK) && (size_global_comment>0))
        //if (fwrite(global_comment,(uInt)size_global_comment,1,zi->filezip) !=1 )
        if (zi->filezip->write(global_comment,(uInt)size_global_comment) != (uInt)size_global_comment)
                err = ZIP_ERRNO;

    //zi->filezip->flush();
    zi->filezip->close();
    if (zi->own_descriptor)
    {
        DWFCORE_FREE_OBJECT( zi->filezip );
    }
    TRYFREE(zi);

    return err;
}

int ZEXPORT zipBlankBuffer (void* buffer, uInt size_blank)
{
    memset(buffer, ' ', size_blank);
    return 0;
}

int ZEXPORT zipBlankFileBytes (zipFile file, uLong position, uInt size_blank)
{
    DWFFileDescriptor* pFile = ((zip_internal*)file)->filezip;
    off_t pos = pFile->seek( SEEK_SET, position );

    char* buf = (char*)ALLOC(size_blank);
    if (buf==NULL)
        return -2;
    memset(buf, ' ', size_blank);
    pFile->write( buf, size_blank );
    TRYFREE(buf);

    pFile->seek( SEEK_SET, pos );
    return 0;
}

