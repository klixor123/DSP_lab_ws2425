#pragma once




#define CODEC_SAMPLE_RATE hz48000
#define SAMPLE_FREQ 48000

#define BLOCK_SIZE  2048
#define CARRIER_FREQ 1000
#define FFT_SIZE 2048            //32, 64, 128, 256, 512, 1024, 2048, 4096


#define FG 20000            // Grenzfrequenz in Hz
#define FILTER_TAPS 51      // Anzahl der Filterkoeffizienten (ungerade Zahl bevorzugt)
#define NUM_TAPS_ARRAY_SIZE 50