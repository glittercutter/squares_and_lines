/*
===========================================================================
Copyright © 2010 Sebastien Raymond <glittercutter@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
===========================================================================
*/
// net_z.c - 

#include "net.h"

#define ZCHUNK_SIZE 16384
#define ZPACKET_SIZE 512


/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

int zdeflate(char *source, unsigned char *out, int *size)
{
	int ret, flush;
    unsigned have;
    z_stream strm;
	char *tmp_source = source;
    unsigned char in[ZCHUNK_SIZE];
	int overflow = 0;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_BEST_COMPRESSION);
	if (ret != Z_OK)
        return ret;

    /* compress until end of buffer */
    do {
		assert(overflow == 0);
// 		tmp_source += overflow * ZCHUNK_SIZE;
        strm.avail_in = strlen(tmp_source);
		strncpy((char*)in, tmp_source, ZCHUNK_SIZE);
        if (!strm.avail_in) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = strlen(tmp_source) <= ((overflow + 1) * ZCHUNK_SIZE) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

		assert(flush == Z_FINISH);

        /* run deflate() on input until output buffer not full, finish
           compression if all of tmp_source has been read in */
        do {
            strm.avail_out = ZPACKET_SIZE;
            strm.next_out = out + (overflow * ZPACKET_SIZE);
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = ZPACKET_SIZE - strm.avail_out;

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */
		if (flush != Z_FINISH) {
			overflow++;
		}
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
	*size = have;
    return Z_OK;

}


int zinflate(char *source, char *out, int size)
{
    int ret;
    unsigned have;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = size;
		if (strm.avail_in == 0)
            break;
        strm.next_in = (void*)source;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = ZCHUNK_SIZE;
            strm.next_out = (void*)out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = ZCHUNK_SIZE - strm.avail_out;
//             if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
//                 (void)inflateEnd(&strm);
//                 return Z_ERRNO;
//             }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

