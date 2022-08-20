%
%
close; clear; clc;
pkg load signal

% *********************************
array_name = "wave_xpico";
savedfile  = [array_name ".h"];
savedwave  = [array_name ".wav"];
points     = 20480;
frate      = 16000;
farray     = [440];
with_noise = 0;
% *********************************

m=0;
for ( fgen=farray)
    m = m + sGen(frate, fgen, points, 0);
end

m= m/length(farray);

% add some noise to waves
if ( with_noise != 0)
 sig_noise=randn(size(m));
 sig_pwr = sqrt(sum(m.^2))/length(m);
 sig_noise=sig_noise*(sig_pwr*with_noise);
 m=m+sig_noise;
endif

% FFT 
m_fft = fft(m);
abs_value=abs(m_fft(1:(points/2)));
bin_value=frate/points;
[pks idx]=findpeaks(abs_value);
fpeaks=bin_value*idx;
t=linspace(1,(points/2),(points/2));

% plot the wave on screen and save as .PNG
w=figure(1);
%
W = 14; H = 8;
set(w,'PaperUnits','inches')
set(w,'PaperOrientation','landscape');
set(w,'PaperSize',[W,H])
set(w,'PaperPosition',[0,0,W,H])

subplot(1,2,1);
m=int16(m*2^15);
plot(m);
mtitle="Custom wave at: ";
mtitle=[mtitle sprintf(" %dHz,",farray)];
mtitle=[mtitle sprintf("\nSampling rate: %dHz,",frate)];
mtitle=[mtitle sprintf(" Length: %d,",points)];
if ( with_noise != 0)
 mtitle = [mtitle ", w/ noise."];
endif
title(mtitle);
legend('Signal');

% plot the FFT o screen and save as .PNG
subplot(1,2,2);
t = frate*(0:(points/2)-1)/points;
plot( t,abs_value,t(idx),abs_value(idx),'.r');
axis([0 (max(farray)+100)]);
title(mtitle);
mlegend="Peak at:\n";
mlegend=[mlegend sprintf(" %dHz",fpeaks)];
legend(mlegend);
xlabel('f (Hz)')
% save as PNG
print(w,'-dpng','-color',[array_name '_wave' '.png']);

%
%m=int16(m*2^15);
% save the array as a .h file
myfile = fopen(savedfile,'wt');
fprintf( myfile, '/*\n');
fprintf( myfile, ' * Sampling rate:   %dHz\n', frate);
fprintf( myfile, ' * Num. points:     %d\n', points);
fprintf( myfile, ' * Freq. Generated: ');
for ( fgen=farray)
  fprintf( myfile, '%dHz ', fgen);
end
fprintf( myfile, '\n');
if ( with_noise != 0)
  fprintf( myfile, ' * Noise added.\n');
endif
fprintf( myfile, ' */\n\n');

fprintf( myfile, '#define %s_SAMPLING_RATE\t%d\n',toupper(array_name),frate);
fprintf( myfile, '#define %s_SIZE\t\t%d\n\n',toupper(array_name), points);

fprintf( myfile, 'const int16_t %s[]=\n{\n', array_name);
fprintf( myfile, '    %d, %d, %d, %d, %d, %d, %d, %d, \n', int16(m));
fprintf( myfile, '};\n\n');

fclose(myfile);

audiowrite(savedwave,[m(:)],frate,'BitsPerSample',32);
