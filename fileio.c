/* 
 * File:   fileio.c
 * Author: Jordan Ross
 *
 * Created on March 5, 2014, 7:23 PM
 */

#include <stdlib.h>     // malloc...
#include <ctype.h>      // toupper...
#include <string.h>     // memcpy...
#include "fileio.h"
#include "CONU2.h"

// global definitions
MEDIA *D;           // mounting information for default storage devices
char FError;        // error reporting mailbox

//----------------------------------------------------------------------
// Master Boot Record key fields offsets
#define FO_MBR          0L   // master boot record sector LBA
#define FO_FIRST_P    0x1BE  // offset of first partition table
#define FO_FIRST_TYPE 0x1C2  // offset of first partition type
#define FO_FIRST_SECT 0x1C6  // first sector of first partition offset
#define FO_FIRST_SIZE 0x1CA  // number of sectors in partition
#define FO_SIGN       0x1FE  // MBR signature location (55,AA)

#define FAT_EOF       0xffff // last cluster in a file
#define FAT_MCLST     0xfff8 // max cluster value in a fat structure

// Partition Boot Record key fields offsets
#define BR_SXC      0xd      // (byte) number of secotrs per cluster
#define BR_RES      0xe      // (word) number of reserved sectors for the boot record
#define BR_FAT_SIZE 0x16     // (word) FAT size in number of sectors
#define BR_FAT_CPY  0x10     // (byte) number of FAT copies
#define BR_MAX_ROOT 0x11     // (odd word) max number of entries in root dir

// directory entry management
#define DIR_ESIZE   32      // size of a directory entry (bytes)
#define DIR_NAME    0       // offset of file name
#define DIR_EXT     8       // offset of file extension
#define DIR_ATTRIB  11      // offset of attribute( 00ARSHDV) (byte)
#define DIR_TIME    22      // offset of last use time  (word)
#define DIR_DATE    24      // offset of last use date  (word)
#define DIR_CLST    26      // offset of first cluster in FAT (word)
#define DIR_SIZE    28      // offset of file size (dword)
#define DIR_DEL     0xE5    // marker of a deleted entry
#define DIR_EMPTY   0       // marker of last entry in directory

//-----------------------------------------------------------------------------
// mount initializes a MEDUA structure for FILEIO access
//
MEDIA * mount(void)
{
    LBA psize;  // number of sectors in partition
    LBA firsts; // first sector inside the first partition
    int i;
    unsigned char *buffer;

    // 0. init the I/Os
    InitSD();

    // 1. check if the card is in the slot
    if(!DetectSD())
    {
        FError = FE_NOT_PRESENT;
        return NULL;
    }

    // 2. initialize the card
    if (InitMedia())
    {
        FError = FE_CANNOT_INIT;
        return NULL;
    }

    // 3. allocate space for the MEDIA structure
    D = (MEDIA *) malloc(sizeof(MEDIA));
    if (D == NULL)
    {
        FError = FE_MALLOC_FAILED;
        return NULL;
    }

    // 4. allocate space for a temp sector buffer
    buffer = (unsigned char *) malloc(512);
    if (buffer == NULL)
    {
        // report an error
        FError = FE_MALLOC_FAILED;
        free(D);
        return NULL;
    }

    // 5. get the Master Boot Record
    if (!ReadSECTOR(0, buffer))
    {
        FError = FE_CANNOT_READ_MBR;
        free(D); free(buffer);
        return NULL;
    }

    // 6. check if the MBR sector is valid, verify the signature word
    if ((buffer[FO_SIGN] != 0x55) || (buffer[FO_SIGN + 1] != 0xAA))
    {
        FError = FE_INVALID_MBR;
        free(D); free(buffer);
        return NULL;
    }


    // 7. read the number of sectors in partition
    psize = ReadL(buffer, FO_FIRST_SIZE);

    // 8. check if the partition type is acceptable
    i = buffer[FO_FIRST_TYPE];
    switch(i)
    {
        case 0x04:
        case 0x06:
        case 0x0E:
            // valid FAT16 options
            break;
        default:
            FError = FE_PARTITION_TYPE;
            free(D); free(buffer);
            return NULL;
    }

    // 9. get the first partition first sector -> Boot Record
    firsts = ReadL(buffer, FO_FIRST_SECT);

    // 10. get the sector loaded (boot record)
    if (!ReadSECTOR(firsts, buffer))
    {
        free(D); free(buffer);
        return NULL;
    }

    // 11. check if  the boot record is valid, verify the signature word
    if ((buffer[FO_SIGN] != 0x55) || (buffer[FO_SIGN + 1] != 0xAA))
    {
        FError = FE_INVALID_BR;
        free(D); free(buffer);
        return NULL;
    }


    // 12. determine the size of a cluster
    D->sxc = buffer[BR_SXC];

    // 13. determine FAT, root and data LBAs
    // FAT = first sector in partition
    // (boot record) + reserved records
    D->fat = firsts + ReadW(buffer, BR_RES);
    D->fatsize = ReadW(buffer, BR_FAT_SIZE);
    D->fatcopy = buffer[BR_FAT_CPY];

    // 14. R00T = FAT + (sectors per FAT * copies of FAT)
    D->root = D->fat + (D->fatsize * D->fatcopy);

    // 15. MAX ROOT is the maximum number of entries in the root directory
    D->maxroot = ReadOddW(buffer, BR_MAX_ROOT);

    // 16. DATA = ROOT + (MAXIMUM ROOT * 32 / 512)
    D->data = D->root + (D->maxroot >> 4);  // assuming maxroot % 16 == 0!!!

    // 17. max clusters in this partition = (tot sectors - sys sectors)/sxc
    D->maxcls = (psize - (D->data - firsts)) / D->sxc;

    // 18. free up the temporary buffer
    free(buffer);
    return D;
} // mount

//-----------------------------------------------------------------------------
// unmount  releases the space allocated for the MEDIA structure
//
void unmount()
{
    free(D);
    D = NULL;
}

MFILE *fopenM(const char *filename, const char *mode)
{
    char c;
    int i, r, e;
    unsigned char *b;
    MFILE *fp;

    // 1. check if a storage device is mounted
    if (D == NULL)
    {
        putsU2("Error:: Media not mounted");
        FError = FE_MEDIA_NOT_MNTD;
        return NULL;
    }

    // 2. allocate a buffer for the file
    b = (unsigned char*)malloc(512);
    if (b == NULL)
    {
        putsU2("Error:: Malloc failed");
        FError = FE_MALLOC_FAILED;
        return NULL;
    }

    // 3. allocate a MFILE structure on the heap
    fp = (MFILE *) malloc(sizeof(MFILE));
    if (fp == NULL) // report an error
    {
        putsU2("Error:: Malloc failed for mfile");
        FError = FE_MALLOC_FAILED;
        free(b);
        return NULL;
    }

    // 4. set pointers to the MEDIA structure and buffer
    fp->mda = D;
    fp->buffer = b;

    // 5. format the filename into name
    for (i=0; i<8; i++)
    {
        // read a char and convert to upper case
        c = toupper(*filename++);
        // extension or short name no-extension
        if ((c == '.') || (c == '\0'))
            break;
        else
            fp->name[i] = c;
    }
    // if short fill the rest up to 8 with spaces
    while(i<8) fp->name[i++] = ' ';

    // 6. if there is an extension
    if (c != '\0')
    {
        for (i = 8; i < 11; i++)
        {
            // read a char and convert to upper case
            c = toupper(*filename++);
            if (c == '.')
                c = toupper(*filename++);
            if (c == '\0')
                break;
            else
                fp->name[i] = c;
        }
        // if short fill the rest up to 8 with spaces
        while(i<8) fp->name[i++] = ' ';
    }

    // 7. copy the file mode character (r, w)
    if ((*mode == 'r') || (*mode == 'w'))
        fp->mode = *mode;
    else
    {
        putsU2("Error:: Invalid mode");
        FError = FE_INVALID_MODE;
        goto ExitOpen;
    }

    // 8. Search for the file in current directory
    if ((r = FindDIR(fp)) == FAIL)
    {
        putsU2("Error:: Find failed!");
        FError = FE_FIND_ERROR;
        goto ExitOpen;
    }

    // 9. init all counters to the beginning of the file
    fp->seek = 0;   // first byte in file
    fp->sec = 0;    // first sector in the cluster
    fp->pos = 0;    // first byte in sector/cluster

    // 10. depending on the mode (read or write)
    if (fp->mode == 'r')
    {
        // 10.1 'r' open for reading
        if (r == NOT_FOUND)
        {
            putsU2("Error:: File not found");
            FError = FE_FILE_NOT_FOUND;
            goto ExitOpen;
        }
        else
        {
            // 10.2 set the current cluster pointer on the first file cluster
            fp->ccls = fp->cluster;
            // 10.3 read a sector of data from the file
            if (!ReadDATA(fp))
            {
                goto ExitOpen;
            }
            // 10.4 determine how much data is really inside the buffer
            if (fp->size-fp->seek < 512)
                fp->top = fp->size - fp->seek;
            else
                fp->top = 512;
        }
    }
    else // 11. open for 'write'
    {
        if (r == NOT_FOUND) // we only want a file that does not exisit
        {
            // 11.1 allocate a first cluster to it
            fp->ccls = 0;                           // indicate brand new file
            if ( NewFAT( fp) == FAIL)
            { // must be media full
                putsU2("Error:: Media full");
                FError = FE_MEDIA_FULL;
                goto ExitOpen;
            }
            fp->cluster = fp->ccls;

            // 11.2 create a new entry
            // search again, for an empty entry this time
            if ( (r = NewDIR( fp)) == FAIL)            // report any error
            {
                putsU2("Error:: Fail of new dir");
                FError = FE_IDE_ERROR;
                goto ExitOpen;
            }
            // 11.3 new entry not found
            if ( r == NOT_FOUND)
            {
                putsU2("Error:: Dir full");
                FError = FE_DIR_FULL;
                goto ExitOpen;
            }
            else // 11.4 new entry identified fp->entry filled
            {
                // 11.4.1
                fp->size = 0;

                // 11.4.2    determine offset in DIR sector
                e = (fp->entry & 0xf) * DIR_ESIZE;    // 16 entry per sector

                // 11.4.3 set initial file size to 0
                fp->buffer[ e + DIR_SIZE]  = 0;
                fp->buffer[ e + DIR_SIZE+1]= 0;
                fp->buffer[ e + DIR_SIZE+2]= 0;
                fp->buffer[ e + DIR_SIZE+3]= 0;

                // 11.4.4 set date and time
                fp->date = 0x34FE; // July 30th, 2006
                fp->buffer[ e + DIR_DATE]  = fp->date;
                fp->buffer[ e + DIR_DATE+1]= fp->date>>8;
                fp->buffer[ e + DIR_TIME]  = fp->time;
                fp->buffer[ e + DIR_TIME+1]= fp->time>>8;

                // 11.4.5 set first cluster
                fp->buffer[ e + DIR_CLST]  = fp->cluster;
                fp->buffer[ e + DIR_CLST+1]= (fp->cluster>>8);

                // 11.4.6 set name
                for ( i = 0; i<DIR_ATTRIB; i++)
                    fp->buffer[ e + i] = fp->name[i];

                // 11.4.7 set attrib
                fp->buffer[ e + DIR_ATTRIB] = ATT_ARC;

                // 11.4.8  update the directory sector;
                if ( !WriteDIR( fp, fp->entry))
                {
                    putsU2("Error:: Write failed");
                    FError = FE_IDE_ERROR;
                    goto ExitOpen;
                }
            } // new entry
        } // not found

        else // file exist already, report error
        {
            //putsU2("Error:: file already exists"); -- ignore because we are
            // expecting this error now.
            FError = FE_FILE_OVERWRITE;
            goto ExitOpen;
        }
    }

    return fp;

    // 13. Exit with error
ExitOpen:
    free(fp->buffer);
    free(fp);
    return NULL;
}

unsigned ReadDATA(MFILE *fp)
{
    LBA l;
    // calculate lba of cluster/sector
    l = fp->mda->data + (LBA)(fp->ccls-2) * fp->mda->sxc + fp->sec;
    return (ReadSECTOR(l, fp->buffer));
}

//---------------------------------------------------------------------
//
unsigned WriteDATA( MFILE *fp)
{
    LBA l;

    // calculate lba of cluster/sector
    l = fp->mda->data + (LBA)(fp->ccls-2) * fp->mda->sxc + fp->sec;

    return ( WriteSECTOR( l, fp->buffer));

}

// loads current entry sector in file buffer
// returns  FAIL/TRUE
unsigned ReadDIR(MFILE *fp, unsigned e)
{
    LBA l;
    // load the root sector containing the DIR entry "e"
    l = fp->mda->root + (e >> 4);

    return (ReadSECTOR(l, fp->buffer));
}

//----------------------------------------------------------------------
//
unsigned WriteDIR( MFILE *fp, unsigned entry)
// write current entry sector
// returns      FAIL/TRUE
{
    LBA l = fp->mda->root + (entry >> 4);
    // write the root sector
    return ( WriteSECTOR( l, fp->buffer)) ;
} // writeDIR

// fp file structure
// return found/not_found/fail
unsigned FindDIR(MFILE *fp)
{
    unsigned eCount;    // current entry counter
    unsigned e;         // current entry offset in buffer
    int i, a, c, d;
    MEDIA *mda = fp->mda;

    // 1. start from the first entry
    eCount = 0;
    // load the first sector of root
    if (!ReadDIR(fp, eCount))
        return FAIL;

    // 2. loop until you reach the end or find the file
    while (1)
    {
        // 2.0 determine the offset in current buffer
        e = (eCount & 0xf) * DIR_ESIZE;
        // 2.1 read the first char of the file name
        a = fp->buffer[e + DIR_NAME];
        // 2.2 terminate if it is empty (end of the list)
        if (a == DIR_EMPTY)
        {
            return NOT_FOUND;
        }
        // 2.3 skip erased entries if looking for a match
        if (a != DIR_DEL)
        {
            // 2.3.1 if not Volume or DIR compare the names
            a = fp->buffer[e + DIR_ATTRIB];
            if ( !(a & (ATT_DIR | ATT_HIDE)))
            {
                // compare file name and extension
                for (i=DIR_NAME; i<DIR_ATTRIB; i++)
                {
                    if ((fp->buffer[e + i]) != (fp->name[i]))
                        break;  // difference found
                }
                if (i == DIR_ATTRIB)
                {
                    // entry found, fill the mfile structure
                    fp->entry = eCount; // sotre entry index
                    fp->time = ReadW(fp->buffer, e + DIR_TIME);
                    fp->date = ReadW(fp->buffer, e + DIR_DATE);
                    fp->size = ReadL(fp->buffer, e + DIR_SIZE);
                    fp->cluster = ReadL(fp->buffer, e + DIR_CLST);
                    return FOUND;
                }
            }
        }
        // 2.4 get the next entry
        eCount++;
        if ((eCount & 0xf) == 0)
        {
            if (!ReadDIR(fp, eCount))
                return FAIL;
        }
        // 2.5 exit the loop if reached the end or error
        if (eCount >= mda->maxroot)
        {
            putsU2("Error:: Last entry reached");
            return NOT_FOUND;   // last entry reached
        }
    }
}

// fp pointer to MFILE struct
// dest pointer to destination buffer
// size number of bytes to transfer
// returns number of bytes actually transferred
unsigned freadM(void * dest, unsigned size, MFILE *fp)
{
    MEDIA *mda = fp->mda;
    unsigned count = size;
    unsigned len;

    // 1. check if fp points to a valid open file structure
    if ((fp->mode != 'r'))
    {
        // invalid file or not open in read mode
        putsU2("Error: Invalid file");
        FError = FE_INVALID_FILE;
        return 0;
    }

    // 2. loop to transfer the data
    while (count > 0)
    {
        // 2.1 check if EOF reached
        if (fp->seek >= fp->size)
        {
            //putsU2("Error: EOF");
            FError = FE_EOF; // reached the end
            break;
        }

        // 2.2 load a new sector if necessary
        if (fp->pos == fp->top)
        {
            fp->pos = 0;
            fp->sec++;
            // 2.2.1 get a new cluster if necessary
            if (fp->sec == mda->sxc)
            {
                fp->sec = 0;
                if (!NextFAT(fp,1))
                    break;
            }
            // 2.2.2 load a sector of data
            if (!ReadDATA(fp))
                break;
            // 2.2.3 determine how much data is inside the buffer
            if (fp->size - fp->seek < 512)
                fp->top = fp->size - fp->seek;
            else
                fp->top = 512;
        }
        // 2.3 copy as many bytes as possible in a single chunk
        // take as much as fits in the current sector
        if (fp->pos+count < fp->top)
            len = count;    // fits all in current sector
        else
            len = fp->top - fp->pos;    // take a first chunk, there is more

        memcpy(dest, fp->buffer + fp->pos, len);

        // 2.4 update all counters and pointers
        count -= len;   // compute what is left
        dest += len;    // advance destination pointer
        fp->pos += len; // advance the pointer in sector
        fp->seek += len;    // advance the seek pointer
    }

    // 3. return number of bytes actually transformed
    return size-count;
}

//-----------------------------------------------------------------------------
// Write data to a file
//
unsigned fwriteM(void *src, unsigned count, MFILE *fp)
{
    MEDIA *mda = fp->mda;
    unsigned len, size = count;

    // 1. check if the file is open
    if (fp->mode != 'w')
    {
        // file not valid or not open for writing
        putsU2("Invalid file!");
        FError = FE_INVALID_FILE;
        return FAIL;
    }
    // 2. loop writing count bytes
    while (count > 0)
    {
        // 2.1 copy as many bytes at a time as possible
        if (fp->pos+count < 512)
            len = count;
        else
            len = 512 - fp->pos;

        memcpy(fp->buffer + fp->pos, src, len);

        // 2.2 update all pointers and counters
        fp->pos += len;     // advance buffer position
        fp->seek += len;    // count the added bytes
        count -= len;       // update the counter
        src += len;         // advance the source pointer

        // 2.3 update the file size too
        if (fp->seek > fp->size)
            fp->size = fp->seek;

        // 2.4 if buffer full, write current buffer to current sector
        if (fp->pos == 512)
        {
            // 2.4.1 write buffer full of data
            if (!WriteDATA(fp))
                return FAIL;

            // 2.4.2 advance ot next sector in cluster
            fp->pos = 0;
            fp->sec++;

            // 2.4.3 get a new cluster if necessary
            if (fp->sec == mda->sxc)
            {
                fp->sec = 0;
                if (NewFAT(fp) == FAIL)
                    return FAIL;
            }
        }
    }

    // 3. number of bytes actually written
    return size-count;
}

unsigned ReadFAT( MFILE *fp, unsigned ccls)
// mda      disk structure
// ccls     current cluster
// return   next cluster value,
//          0xffff if failed or last
{
    unsigned p, c;
    LBA l;

    // get address of current cluster in fat
    p = ccls;
    // cluster = 0xabcd
    // packed as:     0   |   1    |   2   |  3    |
    // word p       0   1 |  2   3 | 4   5 | 6   7 |..
    //              cd  ab|  cd  ab| cd  ab| cd  ab|

    // load the fat sector containing the cluster
    l = fp->mda->fat + (p >> 8 );   // 256 clusters per sector
    if ( !ReadSECTOR( l, fp->buffer))
        return 0xffff;  // failed

    // get the next cluster value
    c = ReadOddW( fp->buffer, ((p & 0xFF)<<1));

    return c;

}

// fp file structure
// n number of links in FAT cluster chain to jump through
//  n == 1, next cluster in the chain
unsigned NextFAT(MFILE * fp, unsigned n)
{
    unsigned c;
    MEDIA *mda = fp->mda;

    // loop n times
    do {
        // get the next cluster link from FAT
        c = ReadFAT(fp, fp->ccls);

        // compare against max value of a cluster in FATxx
        // return if eof
        if (c >= FAT_MCLST) // check against EOF
        {
            putsU2("Error: FAT EOF");
            FError = FE_FAT_EOF;
            return FAIL;    // seeking beyond EOF
        }

        // check if cluster value is valid
        if (c >= mda->maxcls)
        {
            putsU2("Error: Invalid cluster");
            FError = FE_INVALID_CLUSTER;
            return FAIL;
        }
    } while (--n > 0);

    // update the MFILE structure
    fp->ccls = c;
    return TRUE;
}

//----------------------------------------------------------------------
// Close a File
//
unsigned fcloseM(MFILE *fp)
{
    unsigned e, r;            // offset of directory entry in current buffer

    r = FAIL;

    // 1. check if it was open for write
    if ( fp->mode == 'w')
    {
        // 1.1 if the current buffer contains data, flush it
        if ( fp->pos >0)
        {
            if ( !WriteDATA( fp))
            {
                putsU2("Error: Write data failed");
                goto ExitClose;
            }
        }

        // 1.2      finally update the dir entry,
        // 1.2.1    retrive the dir sector
        if ( !ReadDIR( fp, fp->entry))
        {
            putsU2("Error: Read directory failed");
            goto ExitClose;
        }

        // 1.2.2    determine position in DIR sector
        e = (fp->entry & 0xf) * DIR_ESIZE;    // 16 entry per sector

        // 1.2.3 update file size
        fp->buffer[ e + DIR_SIZE]  = fp->size;
        fp->buffer[ e + DIR_SIZE+1]= fp->size>>8;
        fp->buffer[ e + DIR_SIZE+2]= fp->size>>16;
        fp->buffer[ e + DIR_SIZE+3]= fp->size>>24;

        // 1.2.4    update the directory sector;
        if ( !WriteDIR( fp, fp->entry))
        {
            putsU2("Error: Write dir failed");
            goto ExitClose;
        }
    } // write

    // 2. exit with success
    r = TRUE;

ExitClose:
    // 3. invalidate the file structure
    fp->chk = fp->entry + fp->name[0];     // set checksum wrong!

    // 4. free up the buffer and the MFILE struct
    free( fp->buffer);
    free( fp);
    //puts( "free b");
    //puts( "free fp");

    return( r);

} // fcloseM

//----------------------------------------------------------------------
// Find a New entry in root directory or an empty entry
//
unsigned NewDIR( MFILE *fp)
// fp       file structure
// return   found/fail,  fp->entry filled
{
    unsigned eCount;            // current entry counter
    unsigned e;                 // current entry offset in buffer
    int a;
    MEDIA *mda = fp->mda;

    // 1. start from the first entry
    eCount = 0;
    // load the first sector of root
    if ( !ReadDIR( fp, eCount))
        return FAIL;

    // 2. loop until you reach the end or find the file
    while ( 1)
    {
    // 2.0 determine the offset in current buffer
        e = (eCount&0xf) * DIR_ESIZE;

    // 2.1 read the first char of the file name
        a = fp->buffer[ e + DIR_NAME];

    // 2.2 terminate if it is empty (end of the list)or deleted
        if (( a == DIR_EMPTY) ||( a == DIR_DEL))
        {
            fp->entry = eCount;
            return FOUND;
        } // empty or deleted entry found


    // 2.3 get the next entry
        eCount++;
        if ( (eCount & 0xf) == 0)
        { // load a new sector from the root
            if ( !ReadDIR( fp, eCount))
                return FAIL;
        }

    // 2.4 exit the loop if reached the end or error
        if ( eCount > mda->maxroot)
            return NOT_FOUND;               // last entry reached
    }// while

    return FAIL;
} // newDIR

//----------------------------------------------------------------------
// Allocate New Cluster
//
unsigned NewFAT( MFILE * fp)
// fp       file structure
// fp->ccls     ==0 if first cluster to be allocated
//              !=0 if additional cluster
// return   TRUE/FAIL
//  fp->ccls new cluster number
{
    unsigned i, c = fp->ccls;

    // sequentially scan through the FAT looking for an empty cluster
    do {
        c++;    // check next cluster in FAT
        // check if reached last cluster in FAT, re-start from top
        if ( c >= fp->mda->maxcls)
            c = 0;

        // check if full circle done, media full
        if ( c == fp->ccls)
        {
            FError = FE_MEDIA_FULL;
            return FAIL;
        }

        // look at its value
        i = ReadFAT( fp, c);

    } while ( i!=0);    // scanning for an empty cluster


    // mark the new cluster as taken, and last in chain
    WriteFAT( fp, c, FAT_EOF);

    // if not first cluster, link current cluster to the new one
    if ( fp->ccls >0)
        WriteFAT( fp, fp->ccls, c);

    // update the MFILE structure
    fp->ccls = c;

    return TRUE;
} // newFAT

//----------------------------------------------------------------------
// Write a cluster link in all FAT copies
//
unsigned WriteFAT( MFILE *fp, unsigned cls, unsigned v)
// mda      disk structure
// cls      current cluster
// v        next value
// return   TRUE if successful, or FAIL
{
    int i;
    unsigned p;
    LBA l;

    // get address of current cluster in fat
    p = cls * 2; // always even
    // cluster = 0xabcd
    // packed as:     0   |   1    |   2   |  3    |
    // word p       0   1 |  2   3 | 4   5 | 6   7 |..
    //              cd  ab|  cd  ab| cd  ab| cd  ab|

    // load the fat sector containing the cluster
    l = fp->mda->fat + (p >> 9 );
    p &= 0x1fe;
    if ( !ReadSECTOR( l, fp->buffer))
        return FAIL;

    // get the next cluster value
    fp->buffer[ p] = v;       // lsb
    fp->buffer[ p+1] = (v>>8);// msb

    // update all FAT copies
    for ( i=0; i<fp->mda->fatcopy; i++, l += fp->mda->fatsize)
       if ( !WriteSECTOR( l, fp->buffer))
           return FAIL;

    return TRUE;

}

//-----------------------------------------------------------------------------
// fseekM
//
// Advances the file pointer by count positions from CUR_POS
// simple implementation of a seek function
// returns 0 if successful
unsigned fseekM(MFILE *fp, unsigned count)
{
    char buffer[16];    // a small buffer
    unsigned d, r;

    while (count)
    {
        d = (count >= 16) ? 16 : count;
        r = freadM(buffer, d, fp);
        count -= r;
        if (r != d)
            break;  // reach end of file or error
    }

    return count;
}

//----------------------------------------------------------------------
// Scans the current disk and compiles a list of files with given extension
//
unsigned listTYPE( char *list, int max, const char *ext )
// list     array of file names max * 8
// max      number of entries
// ext      file extension we are searching for
// return   number of files found
{
    unsigned eCount;            // current entry counter
    unsigned eOffs;             // current entry offset in buffer
    unsigned x, a, r;
    MFILE *fp;
    unsigned char *b;

    x = 0;
    r = 0;

    // 1.  check if storage device is mounted
    if ( D == NULL)       // unmounted
    {
        putsU2("Error:: Media not mounted");
        FError = FE_MEDIA_NOT_MNTD;
        return 0;
    }

    // 2. allocate a buffer for the file
    b = (unsigned char*)malloc( 512);
    if ( b == NULL)
    {
        putsU2("Error:: Malloc failed");
        FError = FE_MALLOC_FAILED;
        return 0;
    }

    // 3. allocate a MFILE structure on the heap
    fp = (MFILE *) malloc( sizeof( MFILE));
    if ( fp == NULL)            // report an error
    {
        putsU2("Error:: Malloc failed");
        FError = FE_MALLOC_FAILED;
        free( b);
        return 0;
    }

    // 4. set pointers to the MEDIA structure and buffer
    fp->mda = D;
    fp->buffer = b;


    // 5. start from the first entry and load the first sector from root
    eCount = 0;
    eOffs = 0;
    if ( !ReadDIR( fp, eCount))
    {    //report error
        putsU2("Error:: Find Error");
        FError = FE_FIND_ERROR;
        goto ListExit;
    }
    ReadDIR( fp, eCount);

//    putrs( "Compiling PlayList\r");

    // 6. loop until you reach the end of the root directory
    while ( TRUE)
    {
    // RAMHexDump( e, 1);

    // 6.1 read the first char of the file name
        a =  fp->buffer[ eOffs + DIR_NAME];

    // 6.2 terminate if it is empty (end of the list)
        if ( a == DIR_EMPTY)
            break;
    // 6.3
        if ( a != DIR_DEL)
        {
        // if not hidden, print the file name and size
            a = fp->buffer[ eOffs + DIR_ATTRIB];
            if ( !( a & (ATT_HIDE|ATT_DIR)))
            {   //  check the file extension
                if ( memcmp( &fp->buffer[ eOffs + DIR_EXT], ext, 3) == 0)
                {
                    // copy file name in playlist
                    memcpy( &list[x*8], &fp->buffer[ eOffs+DIR_NAME], 8);

                    // check if maximum number of entries reached
                    if ( ++x >= max)
                        break;
                }

                // EOL
                //pcr();

            } // WAV file
        } // not deleted or hidden

    // 6.4 get the next entry
    //     exit the loop if reached the end or error
        eCount++;
        if ( eCount > fp->mda->maxroot)
            break;              // last entry reached

        eOffs += 32;
        if ( eOffs >= 512)
        {
            eOffs = 0;
            if ( !ReadDIR( fp, eCount))
            {
                putsU2("Error:: Find error");
                FError = FE_FIND_ERROR;
                goto ListExit;
            }
        }
    }// while

    // 7. return the number of files found
    r = x;

ListExit:
    // 8. free buffers and return
    free( fp->buffer);
    free( fp);
    return r;

} // listTYPE
