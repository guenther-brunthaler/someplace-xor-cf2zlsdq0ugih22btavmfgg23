/* This utility does mostly the same as "truncate -s <bytes> <file>" except
 * that it will never shrink the file
 *
 * Version 2017.8
 *
 * Copyright (c) 2016 Guenther Brunthaler. All rights reserved.
 *
 * This source file is free software.
 * Distribution is permitted under the terms of the GPLv3. */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

typedef struct { unsigned full_longs; long last_rem; } seekoff_t;

static int starts_with(char const *whole, char const *prefix) {
   int i;
   char c;
   for (i= 0; c= prefix[i]; ++i) {
      if (c != whole[i]) return 0;
   }
   return 1;
}

static int seekoff_add(seekoff_t *dst, seekoff_t const *src) {
   {
      unsigned old= dst->full_longs;
      if ((dst->full_longs+= src->full_longs) < old) goto fail;
   }
   {
      long last;
      if ((last= dst->last_rem + src->last_rem) < 0 || last == LONG_MAX) {
         last-= LONG_MAX;
         assert(last < LONG_MAX);
         if (++dst->full_longs == 0) fail: return 0;
      }
      dst->last_rem= last;
   }
   return 1;
}

static int times2(seekoff_t *off) {
   return seekoff_add(off, off);
}

static int times5(seekoff_t *off) {
   seekoff_t copy= *off;
   return times2(off) && times2(off) && seekoff_add(off, &copy);
}

static int atoseekoff(seekoff_t *off, char const *s) {
   int hex;
   seekoff_t limb;
   if (hex= starts_with(s, "0x") || starts_with(s, "0X")) s+= 2;
   if (!*s) fail: return 0;
   off->full_longs= 0u;
   off->last_rem= 0l;
   limb.full_longs= 0u;
   do {
      unsigned digit;
      {
         static const char digits[]= "0123456789abcdefABCDEF";
         char const *p;
         if (!(p= strchr(digits, *s))) goto fail;
         assert(p >= digits);
         assert(p <= digits + strlen(digits));
         digit= (unsigned)(int)(p - digits);
         assert(digits + digit == p);
      }
      if (digit > 16) digit-= 6;
      if (digit >= 10 && !hex) goto fail;
      if (hex) {
         /* off*= 16 / 2; */
         if (!times2(off) || !times2(off) || !times2(off)) goto fail;
      } else {
         /* off*= 10 / 2; */
         if (!times5(off)) goto fail;
      }
      if (!times2(off)) goto fail;
      limb.last_rem= (int)digit;
      if (!seekoff_add(off, &limb)) goto fail;
   } while (*++s);
   return 1;
}

static int seekto(FILE *fh, seekoff_t const *off) {
   if (fseek(fh, off->last_rem, SEEK_SET)) fail: return 0;
   {
      unsigned n= off->full_longs;
      while (n--) if (fseek(fh, LONG_MAX, SEEK_CUR)) goto fail;
   }
   return 1;
}

int main(int argc, char **argv) {
   FILE *file= 0;
   char const *error= 0, *fname;
   seekoff_t off;
   if (argc >= 2) {
      int argind;
      char const *arg;
      if (strlen(arg= argv[argind= 1]) >= 2 && *arg == '-') {
         if (strcmp(arg, "--")) {
            (void)fprintf(stderr, error= "Unknown option %s!\n", arg);
            goto cleanup;
         }
         ++argind;
      }
      if (argind + 2 != argc) {
         bad_argument_count:
         error=
            "Exactly two arguments are required: The name of a binary file"
            " and a byte count. If the file is shorter than the byte count,"
            " it will be padded to that size by appending binary zeroes. The"
            " file will be created if it does not already exist."
         ;
         goto complain;
      }
      if (!(file= fopen(fname= argv[argind++], "w+b"))) {
         (void)fprintf(
            stderr, error= "Could not open '%s' for writing!\n", fname
         );
         goto cleanup;
      }
      if (!(atoseekoff(&off, argv[argind]))) {
         (void)fprintf(
            stderr, error= "Invalid byte count '%s'!\n", argv[argind]
         );
         goto cleanup;
      }
   } else {
      goto bad_argument_count;
   }
   (void)seekto;
   (void)printf(
         "off fulls: %u (%#x) + rest %ld (%#lx)\n"
      ,  off.full_longs, off.full_longs
      ,  off.last_rem, off.last_rem
   );
   if (fflush(0)) {
      error= "Write error!";
      complain:
      (void)fprintf(stderr, "%s\n", error);
   }
   cleanup:
   if (file) {
      FILE *fh= file; file= 0;
      if (fclose(fh)) {
         (void)fprintf(stderr, error= "I/O error for '%s'!\n", fname);
         goto cleanup;
      }
   }
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
