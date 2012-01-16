#include "zutil.h"

voidp zcalloc (voidp opaque, unsigned items, unsigned size)
{
    if (opaque) items += size - size; /* make compiler happy */
    return (voidp)calloc(items, size);
}

void  zcfree (voidp opaque, voidp ptr)
{
    free(ptr);
    if (opaque) return; /* make compiler happy */
}
