/* ----------------------------- MNI Header -----------------------------------
@NAME       : mexec (CMEX)
@INPUT      : Name of file to execute along with argument list (to be
              passed to execvp).
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: 
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : 
@MODIFIED   : 
---------------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mex.h"

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

typedef enum { false=0, true=1 } boolean;

#define DEBUG


void RunShell (int nargout, Matrix *pargout [],
	       int nargin, Matrix *pargin []);


void mexFunction (int nargout, Matrix *pargout [],
		  int nargin, Matrix *pargin [])
{
   pid_t   kidpid;
   int	   statptr;		/* for wait() */
   boolean GrabOutput;		/* true if a second output arg is given */

#ifdef DEBUG
   printf ("Number of input arguments: %d\n", nargin);
   printf ("Number of output arguments: %d\n", nargout);
#endif

   if (nargin == 0)
   {
      mexErrMsgTxt ("No program name given");
   }

   if (nargout > 2)
   {
      mexErrMsgTxt ("Too many output arguments");
   }

   GrabOutput = (nargout == 2);


   kidpid = fork ();

   if (kidpid == 0)		/* now in child process? */
   {
      if (!GrabOutput || freopen ("temp.output", "w", stdout) != NULL)
      {
	 RunExtern (nargout, pargout, nargin, pargin);
      }
      else
      {
	 printf ("Error opening temporary file: %s\n", _sys_errlist [errno]);
	 exit (-errno);
      }
   }
   else
   {
      /*
       * Wait for child process to either terminate or be 
       * signalled to death.
       */
      do {
	 waitpid (kidpid, &statptr, 0);
      } while (WIFSTOPPED(statptr));

#ifdef DEBUG
      printf ("Parent sez: kid's pid was %d\n", kidpid);
      printf ("WIFSTOPPED:  %d\n", WIFSTOPPED (statptr));
      printf ("WIFEXITED:   %d,  WEXITSTATUS: %d\n", 
	      WIFEXITED (statptr), WEXITSTATUS(statptr));
      printf ("WIFSIGNALED: %d,  WTERMSIG: %d\n", 
	      WIFSIGNALED (statptr), WTERMSIG (statptr));
#endif
      
      if (WIFEXITED (statptr) && WEXITSTATUS (statptr) != 0)
      {
	 mexErrMsgTxt ("Error executing child process");
      }
      else if (WIFSIGNALED (statptr))
      {
	 mexErrMsgTxt ("Child process unexpectedly terminated");
      }

      pargout[0] = mxCreateFull (1,1,REAL);

      if (GrabOutput)
      {
	 FILE  *OutputFile;
	 struct stat statbuf;
	 char  *OutputStr;
	 
	 stat ("temp.output", &statbuf);
	 OutputStr = malloc (statbuf.st_size + 1);
	 OutputFile = fopen ("temp.output", "r");
	 fread (OutputStr, 1, statbuf.st_size, OutputFile);
	 OutputStr [statbuf.st_size] = '\0';
	 pargout[1] = mxCreateString (OutputStr);
	 fclose (OutputFile);
      }

   }
}    /* mexFunction/main */



void RunExtern (int nargout, Matrix *pargout [],
		int nargin, Matrix *pargin [])
{
   int	    m, n, len;
   char	  **argv;
   int	    arg;

   /*
    * Allocate room for all arguments to pass to external program
    * (including argv[0], the program name, and argv[argc] == NULL
    */
   argv = (char **) calloc (nargin+1, sizeof (char *));
   
   for (arg = 0; arg < nargin; arg++)
   {
      m = mxGetM (pargin [arg]); n = mxGetN (pargin [arg]);
      len = max (m, n);
      if ((min (m,n) != 1) || !mxIsString (pargin [arg]))
      {
	 mexErrMsgTxt 
	    ("Input arguments must be one-dimensional character strings");
      }
      argv [arg] = (char *) calloc (len+1, sizeof(char));
      mxGetString (pargin [arg], argv [arg], len+1);

   }

   if (execvp (argv[0], argv) == -1)
   {
      printf ("%s: %s\n", argv[0], _sys_errlist [errno]);
      exit (-errno);
   }

}
