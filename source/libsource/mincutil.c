#include <stdlib.h>
#include "gpw.h"
#include "minc.h"
#include "mincutil.h"
#include "mierrors.h"


extern   char *ErrMsg;     /* should be defined in your main program */
extern   Boolean debug;


/* ----------------------------- MNI Header -----------------------------------
@NAME       : OpenFile
@INPUT      : Filename - name of the NetCDF/MINC file to open
@OUTPUT     : *CDF - handle of the opened file
@RETURNS    : ERR_NONE if file successfully opened
              ERR_IN_MINC if any error opening file
              sets ErrMsg on error
@DESCRIPTION: Opens a NetCDF/MINC file using ncopen.
@METHOD     : 
@GLOBALS    : debug, ErrMsg
@CALLS      : standard NetCDF, mex functions.
@CREATED    : 93-5-31, adapted from code in micopyvardefs.c, Greg Ward
@MODIFIED   : 93-6-4, modified debug/error handling and added Mode parameter
@COMMENTS   : N.B. this is just a copy of the same function from
              mireadvar... need to work on that modularity thing, eh?
---------------------------------------------------------------------------- */
int OpenFile (char *Filename, int *CDF, int Mode)
{
   ncopts = 0;         /* don't abort or print messages */

   *CDF = ncopen (Filename, Mode);

   if (*CDF == MI_ERROR)
   {
      sprintf (ErrMsg, "Error opening output (MINC) file %s", Filename);
      return (ERR_IN_MINC);
   }  
   return (ERR_NONE);
}     /* OpenFile */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetVarInfo
@INPUT      : CDF - handle for a NetCDF file
              vName - string containing the name of the variable in question
@OUTPUT     : *vInfo - a struct which contains the CDF and variable id's,
                number of dimensions and attributes, and an array of 
                DimInfoRec's which tells everything about the various
                dimensions associated with the variable.         
@RETURNS    : ERR_NONE if all went well
              ERR_NO_VAR if specified variable not found
@DESCRIPTION: Gets gobs of information about a NetCDF variable and its 
                associate dimensions.
@METHOD     : 
@GLOBALS    : debug, ErrMsg
@CALLS      : standard NetCDF, library functions
@CREATED    : 93-5-31, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int GetVarInfo (int CDF, char vName[], VarInfoRec *vInfo)
{
   int         DimIDs [MAX_NC_DIMS];
   int         dim;
   DimInfoRec  *Dims;      /* purely for convenience! */

   vInfo->CDF = CDF;
   vInfo->Name = vName;
   vInfo->ID = ncvarid (CDF, vName);

   /* 
    * Abort if there was an error finding the variable
    */
       
   if (vInfo->ID == MI_ERROR)
   {
      sprintf (ErrMsg, "Unknown variable: %s", vName);
      return (ERR_NO_VAR);
   }     /* if ID == MI_ERROR */

   /*
    * Get most of the info about the variable...
    */

   ncvarinq (CDF, vInfo->ID, NULL, &vInfo->DataType, 
             &vInfo->NumDims, DimIDs, &vInfo->NumAtts);

   if (debug)
   {
      printf ("Variable %s has %d dimensions, %d attributes\n",
                 vInfo->Name, vInfo->NumDims, vInfo->NumAtts);
   }

   /*
    * Now loop through all the dimensions, getting info about them
    */

   Dims = (DimInfoRec *) calloc (vInfo->NumDims, sizeof (DimInfoRec));
   for (dim = 0; dim < vInfo->NumDims; dim++)
   {
      ncdiminq (CDF, DimIDs [dim], Dims [dim].Name, &(Dims [dim].Size));
      if (debug)
      {
         printf ("  Dim %d: %s, size %ld\n", 
					  dim, Dims[dim].Name, Dims [dim].Size);
      }
   }     /* for dim */  
   vInfo->Dims = Dims;
	return (ERR_NONE);
}     /* GetVarInfo */




/* ----------------------------- MNI Header -----------------------------------
@NAME       : GetImageInfo
@INPUT      : CDF - handle for a NetCDF file
              vName - string containing the name of the variable in question

@OUTPUT :     *Image - struct containing much useful and relevant info
              about the MIimage variable in the given NetCDF/MINC
              file.  The fields of ImageInfoRec are described in
              mincutil.h.  Note that for any dimensions that do not
              exist (particulary time, but this function will handle
              any missing dimension without complaint), the dimension
              number (FrameDim, SliceDim, etc.) is set to -1, and the
              dimension size (Frames, Slices, etc.) is 0.  This is
              the way any code that uses an ImageInfoRec (eg. to read
              or write images) should check for existence of certain
              dimensions, particularly time.
@RETURNS    : ERR_NONE if all went well
              ERR_NO_VAR if any of the required variables (currently
              MIimage, MIimagemax, and MIimagemin) were not found
              in the file.
@DESCRIPTION: Gets gobs of information about a MINC image variable.  See
              ImageInfoRec in myminc.h for details.
@METHOD     : 
@GLOBALS    : debug, ErrMsg
@CALLS      : standard MINC, NetCDF, library functions
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 93-6-4, modified debug/error handling
@COMMENTS   : Based on GetVarInfo from mireadvar.c, and on Gabe Leger's
              open_minc_file (from mincread.c).
              Note -- assumes that MIimagemax and MIimagemin correctly
              point to the max and min variables for MIimage.  Should
              probably be fixed to "follow" the image-max/min pointers
              in the actual MIimage variable...?
---------------------------------------------------------------------------- */
int GetImageInfo (int CDF, ImageInfoRec *Image)
{
   int      DimIDs [MAX_NC_DIMS];
   int      dim;
   char     CurDimName [MAX_NC_NAME];
   long     CurDimSize;    /* temp storage -- copied into one of
                              Frames, Slices, Width, or Height */

   Image->CDF = CDF;
   Image->ID = ncvarid (CDF, MIimage);
   Image->MaxID = ncvarid (CDF, MIimagemax);
   Image->MinID = ncvarid (CDF, MIimagemin);

   /* 
    * Abort if there was an error finding any of the variables
    */

   if ((Image->ID == MI_ERROR) || 
       (Image->MaxID == MI_ERROR) ||
       (Image->MinID == MI_ERROR))
   {
      sprintf (ErrMsg,
					"Error in MINC file: could not find all required variables");
      return (ERR_NO_VAR);
   }     /* if ID == MI_ERROR */

   /*
    * Get most of the info about the variable (with dimension ID's
    * going to a temporary local array; relevant dimension data will
    * be copied to the struct momentarily!)
    */

   ncvarinq (CDF, Image->ID, NULL, &Image->DataType, 
             &Image->NumDims, DimIDs, &Image->NumAtts);

   if (debug)
   {
      printf ("Image variable has %d dimensions, %d attributes\n",
				  Image->NumDims, Image->NumAtts);
   }

   /*
    * Now loop through all the dimensions, getting info about them
    * and determining from that FrameDim, Frames, SliceDim, Slices, etc.
    */

   Image->FrameDim = Image->SliceDim = Image->HeightDim = Image->WidthDim = -1;
   Image->Frames = Image->Slices = Image->Height = Image->Width = 0;

   for (dim = 0; dim < Image->NumDims; dim++)
   {
      ncdiminq (CDF, DimIDs [dim], CurDimName, &CurDimSize);

      /* 
       * Assign the {..}Dim members of Image based entirely on the name
       * of the dimension.  Thus, the dimensions *could* be in any order
       * you like, this just takes it as it is.
       */

      if (strcmp (MItime, CurDimName) == 0)     /* time dimension = frames */
      {
         Image->FrameDim = dim;
         Image->Frames = CurDimSize;
      }
      else if (strcmp (MIzspace, CurDimName) == 0) /* z dimension = slices */
      {
         Image->SliceDim = dim;
         Image->Slices = CurDimSize;
      }
      else if (strcmp (MIyspace, CurDimName) == 0) /* y dimension = height */
      {
         Image->HeightDim = dim;
         Image->Height = CurDimSize;
      }
      else if (strcmp (MIxspace, CurDimName) == 0) /* x dimension = width */
      {
         Image->WidthDim = dim;
         Image->Width = CurDimSize;
      }

      if (debug)
      {
         printf ("  Dim %d: %s, size %ld\n",dim,CurDimName,CurDimSize);
      }
   }     /* for dim */  
  
   Image->ImageSize = Image->Width * Image->Height;

   if (debug)
   {
      printf("Image var has %ld frames, %ld slices; each image is %ld x %ld\n",
				 Image->Frames, Image->Slices, Image->Height, Image->Width);
   }
   return (ERR_NONE);

}     /* GetImageInfo */




/* ----------------------------- MNI Header -----------------------------------
@NAME       : OpenImage
@INPUT      : Filename - name of the MINC file to open
@OUTPUT     : *Image - struct containing lots of relevant data about the
              MIimage variable and associated dimensions/variables
@RETURNS    : ERR_NONE if all went well
              ERR_IN_MINC if error opening MINC file (from OpenFile)
              ERR_NO_VAR if error reading variables (from GetImageInfo)
              ErrMsg will be set by OpenFile or GetImageInfo
@DESCRIPTION: Open a MINC file and read relevant data about the image variable.
@METHOD     : 
@GLOBALS    : 
@CALLS      : OpenFile, GetImageInfo, miicv{...} functions
@CREATED    : 93-6-3, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
int OpenImage (char Filename[], ImageInfoRec *Image, int mode)
{
   int   CDF;
   int   Result;        /* of various function calls */

   Result = OpenFile (Filename, &CDF, mode);
   if (Result != ERR_NONE)
   {
      return (Result);
   }

   Result = GetImageInfo (CDF, Image);
   if (Result != ERR_NONE)
   {
      return (Result);
   }

   Image->ICV = miicv_create ();
   (void) miicv_setint (Image->ICV, MI_ICV_TYPE, NC_DOUBLE);
   (void) miicv_setint (Image->ICV, MI_ICV_DO_NORM, TRUE);
   (void) miicv_setint (Image->ICV, MI_ICV_USER_NORM, TRUE);
   (void) miicv_attach (Image->ICV, Image->CDF, Image->ID);

   return (ERR_NONE);
}     /* OpenImage */



/* ----------------------------- MNI Header -----------------------------------
@NAME       : CloseImage
@INPUT      : *Image - pointer to struct describing image variable
@OUTPUT     : (none)
@RETURNS    : (none)
@DESCRIPTION: Undoes OpenImage: free icv, close MINC file.
@METHOD     : 
@GLOBALS    : (none)
@CALLS      : standard MINC, NetCDF functions
@CREATED    : 93-6-7, Greg Ward
@MODIFIED   : 
---------------------------------------------------------------------------- */
void CloseImage (ImageInfoRec *Image)
{
   miicv_free (Image->ICV);
   ncclose (Image->CDF);
}
