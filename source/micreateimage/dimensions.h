Boolean CreateDims (int CDF, 
                    long  Frames, long Slices, long Height, long Width,
                    char *Orientation, 
                    int  *NumDims, int DimIDs[], char *DimNames[]);
Boolean CopyDimVar (int ParentCDF, int ChildCDF, 
	   	    int VarID,     char *VarName,
		    int DimID,     char *DimName,
		    int*NewVarID,  Boolean *CopyVals);
Boolean CreateDimVars (int ParentCDF, int ChildCDF,
                       int NumDim, int DimIDs[], char *DimNames[],
		       int *NumExclude, int Exclude[]);

