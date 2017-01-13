/* Detect sparse sections consisting of runs of at least
 * MINIMUM_ZEROS_FOR_SPARSENESS zero bytes within the file read from standard
 * input, and write the decimal starting offsets and lengths of the sections
 * to standard output.
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

static char const *finish_range(unsigned long offset, unsigned long zeroes) {
   if (zeroes >= MINIMUM_ZEROES_FOR_SPARSENESS) {
      if (printf("%lu + %lu = %lu\n", offset - zeroes, zeroes, offset) <= 0) {
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
            " for sparse sections (runs of " ZRUN_SIZE " or more zero bytes)"
            " will be read from the standard input stream."
         ;
         goto complain;
      }
   }
   {
      int c;
      unsigned long offset, zeroes;
      offset= zeroes= 0;
      while ((c= getchar()) != EOF) {
         if (c) {
            if (zeroes) {
               if (error= finish_range(offset, zeroes)) goto complain;
               zeroes= 0;
            }
         } else {
            ++zeroes;
         }
         ++offset;
      }
      if (ferror(stdin)) {
         error= "Error reading from standard input stream!";
         goto complain;
      }
      if (error= finish_range(offset, zeroes)) goto complain;
   }
   if (fflush(0)) {
      error= "Write error!";
      complain:
      (void)fprintf(stderr, "%s\n", error);
   }
   cleanup:
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
