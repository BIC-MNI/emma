/* ----------------------------- MNI Header -----------------------------------
@NAME       : createnan.c
@DESCRIPTION: Supplies the CreateNaN() function, an unreliable and non-
              portable way to get a 'nan' value in a C program (bummer!).
@CREATED    : Feb 1995, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: createnan.c,v 1.2 1997-10-20 17:58:25 greg Exp $
              $Name:  $
---------------------------------------------------------------------------- */


/* 
 * CreateNaN
 * 
 * Returns a double-precision not-a-number (which is defined by IEEE
 * 754).  This method is known to work with Sun4's running SunOS using
 * cc or gcc and SGI's running IRIX 4 or IRIX 5, using cc or gcc.
 * It does *NOT* work under Linux - a floating point exception occurs.
 */
double CreateNaN (void)
{
   double d = 0.0;
   
   return (d/d);
}
