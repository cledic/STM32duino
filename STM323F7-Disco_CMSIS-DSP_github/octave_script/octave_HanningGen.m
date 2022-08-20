%
%
close; clear; clc;
pkg load signal

% *********************************
array_name = "hanning_int16";
savedfile  = [array_name ".h"];
points = 1024;
% *********************************

h=hanning( points);
h=int16(h*2^15);
h=transpose(h);

% save the array as a .h file
myfile = fopen(savedfile,'wt');
fprintf( myfile, '/*\n');
fprintf( myfile, ' * Hanning window\n');
fprintf( myfile, ' * Num. points:     %d\n', points);
fprintf( myfile, ' */\n');
fprintf( myfile, '\n\n#define %s_SIZE\t%d\n',toupper(array_name),points);
fprintf( myfile, '\nint16_t %s[]={\n',array_name);
fprintf( myfile, '    %d, %d, %d, %d, %d, %d, %d, %d, \n', h);
fprintf( myfile, '};\n\n');
fclose(myfile);



