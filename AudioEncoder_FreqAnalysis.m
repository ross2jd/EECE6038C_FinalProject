close all
Fs = 1/20e-6;               % Sampling frequency
T = 1/Fs;                     % Sample time
L = length(data(:,1));             % Length of signal
t = (0:L-1)*T;                % Time vector
plot(t,data(:,1))
title('Raw Audio Signal')
xlabel('time (seconds)')
NFFT = 2^nextpow2(L); % Next power of 2 from length of y
Y = fft((data(:,1)),NFFT)/L;
DC_comp = floor(Y(1));
Y(1) = 0;
f = Fs/2*linspace(0,1,NFFT/2+1);

% Plot single-sided amplitude spectrum.
figure;
plot(f,2*abs(Y(1:NFFT/2+1))) 
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')

ADC_DCremoved = data(:,1) - DC_comp;
figure;
plot(t,ADC_DCremoved)
title('Audio Signal w/ DC Bias removed')
xlabel('time (seconds)')

fileID = fopen('formattedData.txt', 'w');
fprintf(fileID, '%g\n',ADC_DCremoved);