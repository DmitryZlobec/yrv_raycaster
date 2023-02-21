/* memcpy (the standard C function)
   This function is in the public domain.  */

/*

@deftypefn Supplemental void* memcpy (void *@var{out}, const void *@var{in}, @
  size_t @var{length})

Copies @var{length} bytes from memory region @var{in} to region
@var{out}.  Returns a pointer to @var{out}.

@end deftypefn

*/

#include "coremark.h"
void bcopy (const void*, void*, size_t);

void *
memcpy (void *out, const void *in, size_t length)
{
    bcopy(in, out, length);
    return out;
}
