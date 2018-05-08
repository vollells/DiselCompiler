/* diesel_rts.c */
#include <stdio.h>
// Compile with gcc -c diesel_rts.c -o diesel_rts.o -Wall -m64

#ifdef __cplusplus
extern "C" {
#endif

void myputchar(int ch) {
    putc(ch, stdout);
    fflush(stdout);
}

#ifdef __cplusplus
}
#endif
