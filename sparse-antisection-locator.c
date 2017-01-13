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
   unsigned long offset, zeroes;
   unsigned sum_low, sum_high;
};

static char const *finish_range(struct state *s) {
   if (s->zeroes >= MINIMUM_ZEROES_FOR_SPARSENESS) {
      if (
         printf(
            "%lu + %lu = %lu\n", s->offset - s->zeroes, s->zeroes, s->offset
         ) <= 0
      ) {
         return "Error writing to standard output stream!";
      }
   }
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
      s.offset= s.zeroes= s.sum_low= s.sum_high= 0;
      while ((c= getchar()) != EOF) {
         if (c) {
            if (s.zeroes) {
               if (error= finish_range(&s)) goto complain;
               s.zeroes= 0;
            }
         } else {
            ++s.zeroes;
         }
         ++s.offset;
      }
      if (ferror(stdin)) {
         error= "Error reading from standard input stream!";
         goto complain;
      }
      if (error= finish_range(&s)) goto complain;
   }
   if (fflush(0)) {
      error= "Write error!";
      complain:
      (void)fprintf(stderr, "%s\n", error);
   }
   cleanup:
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
