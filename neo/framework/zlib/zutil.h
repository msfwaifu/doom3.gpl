typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

extern const char *z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm,err) \
  return (strm->msg = (char*)ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

        /* Common defaults */

#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef F_OPEN
#  define F_OPEN(name, mode) fopen((name), (mode))
#endif

         /* functions */

#ifdef HAVE_STRERROR
   extern char *strerror OF((int));
#  define zstrerror(errnum) strerror(errnum)
#else
#  define zstrerror(errnum) ""
#endif

#define zmemcpy memcpy
#define zmemcmp memcmp
#define zmemzero(dest, len) memset(dest, 0, len)

/* Diagnostic functions */
#ifdef _ZIP_DEBUG_
   extern int z_verbose;
#  define Assert(cond,msg) assert(cond);
   //{if(!(cond)) Sys_Error(msg);}
#  define Trace(x) {if (z_verbose>=0) Sys_Error x ;}
#  define Tracev(x) {if (z_verbose>0) Sys_Error x ;}
#  define Tracevv(x) {if (z_verbose>1) Sys_Error x ;}
#  define Tracec(c,x) {if (z_verbose>0 && (c)) Sys_Error x ;}
#  define Tracecv(c,x) {if (z_verbose>1 && (c)) Sys_Error x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif


typedef uLong (*check_func) OF((uLong check, const Byte *buf, uInt len));
voidp zcalloc OF((voidp opaque, unsigned items, unsigned size));
void   zcfree  OF((voidp opaque, voidp ptr));

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidp)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}
