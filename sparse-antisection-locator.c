/* Detect non-sparse sections (not containing any runs of at least
 * MINIMUM_ZEROS_FOR_SPARSENESS zero bytes) within the file read from standard
 * input, and write the decimal starting offsets and lengths of the sections
 * as well as a Fletcher-16 checksum of the section contents to standard
 * output.
 *
 * Version 2017.14
 *
 * Copyright (c) 2017 Guenther Brunthaler. All rights reserved.
 *
 * This source file is free software.
 * Distribution is permitted under the terms of the GPLv3. */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

/* Let it be larger than any normal native simple variable. */
#define MINIMUM_ZEROES_FOR_SPARSENESS 64

#define N2STR0(x) #x
#define N2STR(x) N2STR0(x)
#define ZRUN_SIZE N2STR(MINIMUM_ZEROES_FOR_SPARSENESS)

struct state {
   unsigned long start, offset, zeroes;
   unsigned sum_low, sum_high;
};

static char const *finish_range(struct state *s) {
   if (s->zeroes >= MINIMUM_ZEROES_FOR_SPARSENESS) {
      /* Delayed zeroes are part of a sparse section. Report
       * attributes of the non-sparse section preceding it. */
      if (
         printf(
               "%lu + %lu = %lu CHK %u\n"
            ,  s->start, s->offset - s->zeroes - s->start
            ,  s->offset - s->zeroes, s->sum_high << 8 | s->sum_low
         ) <= 0
      ) {
         return "Error writing to standard output stream!";
      }
      s->zeroes= 0;
      s->sum_low= s->sum_high= 0; /* Start a new checksum. */
      s->start= s->offset;
   } else {
      /* Too few delayed zeroes for sparseness. Include them
       * in the checksum as normal data. */
      assert(s->zeroes >= 1);
      do {
         /* <sum_low> won't change by adding zero values. */
         if ((s->sum_high+= s->sum_low) >= 255) s->sum_high-= 255;
         assert(s->sum_high < 255);
      } while (--s->zeroes);
   }
   assert(s->zeroes == 0);
   return 0;
}

int main(int argc, char **argv) {
   char const *error= 0;
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
      if (argind != argc) {
         bad_argument_count:
         error=
            "No argument must be specified: The binary file to be examined"
            " for non-sparse sections (sparse sections are runs of"
            " " ZRUN_SIZE " or more zero bytes) will be read from the standard"
            " input stream. Offsets, lengths and a Fletcher-16 checksum of"
            " the section contents will be written to the standard output"
            " stream."
         ;
         goto complain;
      }
   }
   {
      int c;
      struct state s;
      s.start= s.offset= s.zeroes= s.sum_low= s.sum_high= 0;
      while ((c= getchar()) != EOF) {
         if (c) {
            /* Not a sparse byte. */
            if (s.zeroes) {
               /* Process delayed zeroes. */
               if (error= finish_range(&s)) goto complain;
            }
            /* Add the nonzero byte just read to the running sum. */
            if ((s.sum_low+= c) >= 255) s.sum_low-= 255;
            assert(s.sum_low < 255);
            if ((s.sum_high+= s.sum_low) >= 255) s.sum_high-= 255;
            assert(s.sum_high < 255);
         } else {
            ++s.zeroes;
         }
         ++s.offset;
      }
      if (ferror(stdin)) {
         error= "Error reading from standard input stream!";
         goto complain;
      }
      if (s.zeroes) if (error= finish_range(&s)) goto complain;
      assert(s.zeroes == 0);
      if (s.start != s.offset) if (error= finish_range(&s)) goto complain;
      assert(s.start == s.offset);
   }
   if (fflush(0)) {
      error= "Write error!";
      complain:
      (void)fprintf(stderr, "%s\n", error);
   }
   cleanup:
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
