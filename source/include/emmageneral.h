#ifndef _EMMAGENERAL
#define _EMMAGENERAL

/*
 * define a few useful constants and macros
 */

typedef unsigned char Boolean;

#define TRUE 1
#define FALSE 0

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))
#define abs(A)    ((A) < 0 ? (A*(-1)) : (A))


#endif
