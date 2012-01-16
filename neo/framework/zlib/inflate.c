/* @(#) $Id: unzip.c,v 1.2 1999/09/07 20:51:25 zoid Exp $ */

#include "zutil.h"
#include "infblock.h"

typedef enum {
      imMETHOD,   /* waiting for method byte */
      imFLAG,     /* waiting for flag byte */
      imDICT4,    /* four dictionary check bytes to go */
      imDICT3,    /* three dictionary check bytes to go */
      imDICT2,    /* two dictionary check bytes to go */
      imDICT1,    /* one dictionary check byte to go */
      imDICT0,    /* waiting for inflateSetDictionary */
      imBLOCKS,   /* decompressing blocks */
      imCHECK4,   /* four check bytes to go */
      imCHECK3,   /* three check bytes to go */
      imCHECK2,   /* two check bytes to go */
      imCHECK1,   /* one check byte to go */
      imDONE,     /* finished check, done */
      imBAD}      /* got an error--stay here */
inflate_mode;

/* inflate private state */
struct internal_state {

  /* mode */
  inflate_mode  mode;   /* current inflate mode */

  /* mode dependent information */
  union {
    uInt method;        /* if FLAGS, method byte */
    struct {
      uLong was;                /* computed check value */
      uLong need;               /* stream check value */
    } check;            /* if CHECK, check values to compare */
    uInt marker;        /* if BAD, inflateSync's marker bytes count */
  } sub;        /* submode */

  /* mode independent information */
  int  nowrap;          /* flag for no wrapper */
  uInt wbits;           /* log2(window size)  (8..15, defaults to 15) */
  inflate_blocks_statef 
    *blocks;            /* current inflate_blocks state */

};


int inflateReset(z_streamp z)
{
  if (z == Z_NULL || z->state == Z_NULL)
    return Z_STREAM_ERROR;
  z->total_in = z->total_out = 0;
  z->msg = Z_NULL;
  z->state->mode = z->state->nowrap ? imBLOCKS : imMETHOD;
  inflate_blocks_reset(z->state->blocks, z, Z_NULL);
  Tracev(("inflate: reset\n"));
  return Z_OK;
}


int inflateEnd(z_streamp z)
{
  if (z == Z_NULL || z->state == Z_NULL || z->zfree == Z_NULL)
    return Z_STREAM_ERROR;
  if (z->state->blocks != Z_NULL)
    inflate_blocks_free(z->state->blocks, z);
  ZFREE(z, z->state);
  z->state = Z_NULL;
  Tracev(("inflate: end\n"));
  return Z_OK;
}



int inflateInit2_(z_streamp z, int w, const char *version, int stream_size)
{
  if (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
      stream_size != sizeof(z_stream))
      return Z_VERSION_ERROR;

  /* initialize state */
  if (z == Z_NULL)
    return Z_STREAM_ERROR;
  z->msg = Z_NULL;
  if (z->zalloc == Z_NULL)
  {
    z->zalloc = (void *(*)(void *, unsigned, unsigned))zcalloc;
    z->opaque = (voidp)0;
  }
  if (z->zfree == Z_NULL) z->zfree = (void (*)(void *, void *))zcfree;
  if ((z->state = (struct internal_state *)
       ZALLOC(z,1,sizeof(struct internal_state))) == Z_NULL)
    return Z_MEM_ERROR;
  z->state->blocks = Z_NULL;

  /* handle undocumented nowrap option (no zlib header or check) */
  z->state->nowrap = 0;
  if (w < 0)
  {
    w = - w;
    z->state->nowrap = 1;
  }

  /* set window size */
  if (w < 8 || w > 15)
  {
    inflateEnd(z);
    return Z_STREAM_ERROR;
  }
  z->state->wbits = (uInt)w;

  /* create inflate_blocks state */
  if ((z->state->blocks =
      inflate_blocks_new(z, z->state->nowrap ? Z_NULL : adler32, (uInt)1 << w))
      == Z_NULL)
  {
    inflateEnd(z);
    return Z_MEM_ERROR;
  }
  Tracev(("inflate: allocated\n"));

  /* reset state */
  inflateReset(z);
  return Z_OK;
}


int inflateInit_(z_streamp z, const char *version, int stream_size)
{
  return inflateInit2_(z, DEF_WBITS, version, stream_size);
}


#define iNEEDBYTE {if(z->avail_in==0)return r;r=f;}
#define iNEXTBYTE (z->avail_in--,z->total_in++,*z->next_in++)

int inflate(z_streamp z, int f)
{
  int r;
  uInt b;

  if (z == Z_NULL || z->state == Z_NULL || z->next_in == Z_NULL)
    return Z_STREAM_ERROR;
  f = f == Z_FINISH ? Z_BUF_ERROR : Z_OK;
  r = Z_BUF_ERROR;
  while (1) switch (z->state->mode)
  {
    case imMETHOD:
      iNEEDBYTE
      if (((z->state->sub.method = iNEXTBYTE) & 0xf) != Z_DEFLATED)
      {
        z->state->mode = imBAD;
        z->msg = (char*)"unknown compression method";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      if ((z->state->sub.method >> 4) + 8 > z->state->wbits)
      {
        z->state->mode = imBAD;
        z->msg = (char*)"invalid window size";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      z->state->mode = imFLAG;
    case imFLAG:
      iNEEDBYTE
      b = iNEXTBYTE;
      if (((z->state->sub.method << 8) + b) % 31)
      {
        z->state->mode = imBAD;
        z->msg = (char*)"incorrect header check";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      Tracev(("inflate: zlib header ok\n"));
      if (!(b & PRESET_DICT))
      {
        z->state->mode = imBLOCKS;
        break;
      }
      z->state->mode = imDICT4;
    case imDICT4:
      iNEEDBYTE
      z->state->sub.check.need = (uLong)iNEXTBYTE << 24;
      z->state->mode = imDICT3;
    case imDICT3:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE << 16;
      z->state->mode = imDICT2;
    case imDICT2:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE << 8;
      z->state->mode = imDICT1;
    case imDICT1:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE;
      z->adler = z->state->sub.check.need;
      z->state->mode = imDICT0;
      return Z_NEED_DICT;
    case imDICT0:
      z->state->mode = imBAD;
      z->msg = (char*)"need dictionary";
      z->state->sub.marker = 0;       /* can try inflateSync */
      return Z_STREAM_ERROR;
    case imBLOCKS:
      r = inflate_blocks(z->state->blocks, z, r);
      if (r == Z_DATA_ERROR)
      {
        z->state->mode = imBAD;
        z->state->sub.marker = 0;       /* can try inflateSync */
        break;
      }
      if (r == Z_OK)
        r = f;
      if (r != Z_STREAM_END)
        return r;
      r = f;
      inflate_blocks_reset(z->state->blocks, z, &z->state->sub.check.was);
      if (z->state->nowrap)
      {
        z->state->mode = imDONE;
        break;
      }
      z->state->mode = imCHECK4;
    case imCHECK4:
      iNEEDBYTE
      z->state->sub.check.need = (uLong)iNEXTBYTE << 24;
      z->state->mode = imCHECK3;
    case imCHECK3:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE << 16;
      z->state->mode = imCHECK2;
    case imCHECK2:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE << 8;
      z->state->mode = imCHECK1;
    case imCHECK1:
      iNEEDBYTE
      z->state->sub.check.need += (uLong)iNEXTBYTE;

      if (z->state->sub.check.was != z->state->sub.check.need)
      {
        z->state->mode = imBAD;
        z->msg = (char*)"incorrect data check";
        z->state->sub.marker = 5;       /* can't try inflateSync */
        break;
      }
      Tracev(("inflate: zlib check ok\n"));
      z->state->mode = imDONE;
    case imDONE:
      return Z_STREAM_END;
    case imBAD:
      return Z_DATA_ERROR;
    default:
      return Z_STREAM_ERROR;
  }
#ifdef NEED_DUMMY_RETURN
  return Z_STREAM_ERROR;  /* Some dumb compilers complain without this */
#endif
}


int inflateSetDictionary(z_streamp z, const Byte *dictionary, uInt dictLength)
{
  uInt length = dictLength;

  if (z == Z_NULL || z->state == Z_NULL || z->state->mode != imDICT0)
    return Z_STREAM_ERROR;

  if (adler32(1L, dictionary, dictLength) != z->adler) return Z_DATA_ERROR;
  z->adler = 1L;

  if (length >= ((uInt)1<<z->state->wbits))
  {
    length = (1<<z->state->wbits)-1;
    dictionary += dictLength - length;
  }
  inflate_set_dictionary(z->state->blocks, dictionary, length);
  z->state->mode = imBLOCKS;
  return Z_OK;
}


int inflateSync(z_streamp z)
{
  uInt n;       /* number of bytes to look at */
  Byte *p;     /* pointer to bytes */
  uInt m;       /* number of marker bytes found in a row */
  uLong r, w;   /* temporaries to save total_in and total_out */

  /* set up */
  if (z == Z_NULL || z->state == Z_NULL)
    return Z_STREAM_ERROR;
  if (z->state->mode != imBAD)
  {
    z->state->mode = imBAD;
    z->state->sub.marker = 0;
  }
  if ((n = z->avail_in) == 0)
    return Z_BUF_ERROR;
  p = z->next_in;
  m = z->state->sub.marker;

  /* search */
  while (n && m < 4)
  {
    static const Byte mark[4] = {0, 0, 0xff, 0xff};
    if (*p == mark[m])
      m++;
    else if (*p)
      m = 0;
    else
      m = 4 - m;
    p++, n--;
  }

  /* restore */
  z->total_in += p - z->next_in;
  z->next_in = p;
  z->avail_in = n;
  z->state->sub.marker = m;

  /* return no joy or set up to restart on a new block */
  if (m != 4)
    return Z_DATA_ERROR;
  r = z->total_in;  w = z->total_out;
  inflateReset(z);
  z->total_in = r;  z->total_out = w;
  z->state->mode = imBLOCKS;
  return Z_OK;
}


/* Returns true if inflate is currently at the end of a block generated
 * by Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
 * implementation to provide an additional safety check. PPP uses Z_SYNC_FLUSH
 * but removes the length bytes of the resulting empty stored block. When
 * decompressing, PPP checks that at the end of input packet, inflate is
 * waiting for these length bytes.
 */
int inflateSyncPoint(z_streamp z)
{
  if (z == Z_NULL || z->state == Z_NULL || z->state->blocks == Z_NULL)
    return Z_STREAM_ERROR;
  return inflate_blocks_sync_point(z->state->blocks);
}
