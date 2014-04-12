/* simple malloc()/free() implementation
   Based on the snes-sdk implementation which has in turn been adapted
   from http://www.flipcode.com/archives/Simple_Malloc_Free_Functions.shtml */

#include "string.h"

#define USED 1

struct unit {
  unsigned int size;
};

struct msys_t {
  struct unit* free;
  struct unit* heap;
};

static struct msys_t msys;

static struct unit* __compact(struct unit *p, unsigned int nsize)
{
       unsigned int bsize, psize;
       struct unit *best;
       void *vp;

       best = p;
       bsize = 0;

       while (psize = p->size, psize) {
              if (psize & USED) {
                  if (bsize != 0) {
                      best->size = bsize;
                      if(bsize >= nsize) {
                          return best;
                      }
                  }
                  bsize = 0;
                  vp = /*(void *)*/p;
                  best = p = /*(struct unit *)*/(vp + (psize & ~USED));
              }
              else {
                  bsize += psize;
                  vp = /*(void *)*/p;
                  p = /*(struct unit *)*/(vp + psize);
              }
       }

       if(bsize != 0) {
           best->size = bsize;
           if(bsize >= nsize) {
               return best;
           }
       }

       return 0;
}

void free(void *ptr)
{
     if(ptr) {
         struct unit *p;

         p = /*(struct unit *)*/(/*(void*)*/ptr - sizeof(struct unit));
         p->size &= ~USED;
     }
}

void *malloc(unsigned int size)
{
     unsigned int fsize;
     struct unit *p;
     void *vp;

     if(size == 0) return 0;

     size  += 3 + sizeof(struct unit);
     size >>= 2;
     size <<= 2;

     if(msys.free == 0 || size > msys.free->size) {
         msys.free = __compact(msys.heap, size);
         if(msys.free == 0) return 0;
     }

     p = msys.free;
     fsize = msys.free->size;

     if(fsize >= size + sizeof(struct unit)) {
         vp = /*(void *)*/p;
         msys.free = /*(struct unit *)*/(vp + size);
         msys.free->size = fsize - size;
     }
     else {
         msys.free = 0;
         size = fsize;
     }

     p->size = size | USED;

     vp = /*(void *)*/p;
     return /*(void *)*/(vp + sizeof(struct unit));
}

void __malloc_init(void *heap, unsigned int len)
{
     len >>= 2;
     len <<= 2;
     msys.free = msys.heap = /*(struct unit *)*/ heap;
     msys.free->size = msys.heap->size = len - sizeof(struct unit);
}

void compact(void)
{
     msys.free = __compact(msys.heap, 0xffff);
}

void *realloc(void *ptr, unsigned int size)
{
    void *p; p = malloc(size);
    memcpy(p, ptr, size); /* this is suboptimal, but not erroneous */
    free(ptr);
    return p;
}
