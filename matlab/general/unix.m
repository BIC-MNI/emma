function [stat,output] = unix(command)

[output, stat] = system(command);
