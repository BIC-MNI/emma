#ifndef _MEX_UTILS_H
#define _MEX_UTILS_H

/* 
 * Error codes returned by functions in this library
 */

#define mexARGS_TOO_BIG -1
#define mexARGS_INVALID -2

/*
 * Function prototypes.  Details in mexutils.c
 */

int   ParseOptions   (Matrix *OptVector, int MaxOptions, Boolean *debug);
char *ParseStringArg (Matrix *Mstr, char *Cstr []);
int   ParseIntArg    (Matrix *Mvector, int MaxSize, long Cvector[]);

#endif
