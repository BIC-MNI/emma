
if (strcmp(get(0,'TerminalProtocol'),'x'))
  !xmosaic http://www.mni.mcgill.ca/users/wolforth/matlab_demos/ &
else
  disp ('Unable to run XMosaic since this is not an X display.');
end
  
