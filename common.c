/* Yes! This is about the most primitive utility one can imgine. However, for
 * some reason, nobody seemed to care to implement it. So I did. It depends
 * only on the C standard run-time library, of course.
 *
 * The utility is useful for inverting monochromatic bitmap images, for
 * implementing on-time pad (OTP) encryption schemes, for combining split
 * cryptographic keys back into a single key, for calculating parity of
 * striped data, and many more possible applications.
 *
 * Version 2016.225
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

int main(int argc, char **argv) {
   FILE *other= 0;
   char const *error= 0, *oname;
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
      if (argind + 1 != argc) {
         bad_argument_count:
         error=
            "Exactly one argument is required: The name of a binary file to"
            " bitwise " OPNAME " with binary data read from the standard input"
            " stream. (The binary result of the operation will be written to"
            " the standard output stream)."
         ;
         goto complain;
      }
      if (!(other= fopen(oname= argv[argind], "rb"))) {
         (void)fprintf(
            stderr, error= "Could not open '%s' for reading!\n", oname
         );
         goto cleanup;
      }
   } else {
      goto bad_argument_count;
   }
   {
      int c;
      while ((c= getchar()) != EOF) {
         int co;
         if ((co= getc(other)) == EOF) {
            if (ferror(other)) {
               (void)fprintf(
                  stderr, error= "Error reading from '%s'!\n", oname
               );
               goto cleanup;
            }
            (void)fprintf(
                  stderr
               ,  error=
                  "Ran out of " OPNAME "-data: '%s' is shorter than"
                  " standard input stream!\n"
               , oname
            );
            goto cleanup;
         }
         assert((unsigned)c <= UCHAR_MAX);
         assert((unsigned)co <= UCHAR_MAX);
         c OP_EQ co;
         if (putchar(c) != c) {
            error= "Error writing to standard output stream!";
            goto complain;
         }
      }
      if (ferror(stdin)) {
         error= "Error reading from standard input stream!";
         goto complain;
      }
   }
   if (fflush(0)) {
      error= "Write error!";
      complain:
      (void)fprintf(stderr, "%s\n", error);
   }
   cleanup:
   if (other) {
      FILE *fh= other; other= 0;
      if (fclose(fh)) {
         (void)fprintf(stderr, error= "I/O error for '%s'!\n", oname);
         goto cleanup;
      }
   }
   return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
