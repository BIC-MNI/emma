Boolean GetArgs (int *pargc, char *argv[], 
                 long *NumFrames, long *NumSlices, long *Height, long *Width,
                 nc_type *Type, Boolean *Signed);
Boolean SetTypeAndVR (char *type_str, nc_type *Type, Boolean *signed_type,
                      double valid_range[]);

