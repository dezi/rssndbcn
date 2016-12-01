/*
 * Simple HF sound generator using ALSA API and libasound.
 *
 * Compile:
 * $ cc -o generator generator.c -lasound -lm
 * 
 * Usage:
 * $ ./generator
 * 
 * Copyright (C) 2016 Dennis Zierahn <dezi@kappa-mm.de>
 */
 
#include <math.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"
#define PCM_CHANNELS 1
#define PCM_RATE 44100
#define PCM_DURATION 2

#define PI 3.14159265359

//
// 44100 = 2*2*3*3*5*5*7*7
// 48000 = 2*2*2*2*2*2*2*3*5*5*5
// GGT = 2*2*3*5*5 = 300
//
// 1000 ms @ 44100 = 44100 samples
//  100 ms @ 44100 =  4410 samples
//   10 ms @ 44100 =   441 samples
//    1 ms @ 44100 =    44 samples
//
// 343 m ~ 1.000 s
//   1 m ~ 0.003 s ~  3 ms ~ 132 samples
//	 5 m ~ 0.015 s ~ 15 ms ~ 660 samples
//

void generateTone(short *samples, int numSamples, int sampleRate, int freqOfTone, int volume)
{
	int inx;
	
	for (inx = 0; inx < numSamples; ++inx)
	{
		double val = sin(2 * PI * inx / (sampleRate / (double) freqOfTone));

		samples[ inx ] += (short) round(val * volume);
		
		//if (inx < 100) printf("s=%f=%d\n", val, samples[ inx ]);
	}
}

void generateHF(short *samples, int numSamples, int volume)
{
	int framepos;
	
	for (framepos = 0; framepos < numSamples; ++framepos)
	{
		samples[ framepos ] += ((framepos % 3) - 1) * volume;
	}
}

void generateFadeHF(short *samples, int numSamples, int volume)
{
	int framepos;
	int fade;
	
	for (framepos = 0; framepos < numSamples; ++framepos)
	{
		fade = volume;
		
		if (framepos < 10)
		{
			fade = fade * framepos / 10;
		}
		
		if ((numSamples - (framepos + 1)) < 10)
		{
			fade = fade * (numSamples - (framepos + 1)) / 10;
		}
		
		samples[ framepos ] += ((framepos % 3) - 1) * fade;
		//samples[ framepos ] += ((framepos % 2) == 0) ? -fade : +fade;
	}
}

int main(int argc, char **argv) 
{
	int channels = PCM_CHANNELS;
	int rate 	 = PCM_RATE;

	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;

	int perr;
	int tmp;
	
	if (perr = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(perr));
		
		return -1;
	}
	
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	if (perr = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(perr));
	}
	
	if (perr = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE) < 0)
	{ 
		printf("ERROR: Can't set format. %s\n", snd_strerror(perr));
	}
	
	if (perr = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0) 
	{
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(perr));
	}
	
	if (perr = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0)
	{
		printf("ERROR: Can't set rate. %s\n", snd_strerror(perr));
	}
	
	if (perr = snd_pcm_hw_params_set_period_size(pcm_handle, params, 1024, 0) < 0)
	{
		printf("ERROR: Can't set period. %s\n", snd_strerror(perr));
	}
	
	if (perr = snd_pcm_hw_params(pcm_handle, params) < 0)
	{
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(perr));
	}

	printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));
	printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

	snd_pcm_hw_params_get_channels(params, &tmp);
	printf("channels: %d\n", tmp);

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	printf("rate: %d hz\n", tmp);

	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	printf("period: %d frames\n", frames);

	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	printf("duration: %d seconds\n", PCM_DURATION);

	int samples = PCM_RATE * PCM_DURATION;
	samples -= samples % frames;
	
	printf("total: %d samples\n", samples);

	int framepos;
	int fade;
	int inx;
	
	short* samplebuffer = (short *) malloc(samples * 2);
	for (framepos = 0; framepos < samples; framepos++) samplebuffer[ framepos ] = 0;
	
	/*
	for (framepos = 0; framepos < samples; framepos++)
	{
		samplebuffer[ framepos ] = (short) random();
		samplebuffer[ framepos ] = ((framepos % 3) - 1) * 32000;
	}
	*/
	
	//generateHF(samplebuffer + 20000, 88);
	//generateHF(samplebuffer + 20000, 2000);
	
	//generateTone(samplebuffer + 20088, 44, PCM_RATE, 4000);
	
	//generateTone(samplebuffer + 20088, 44, PCM_RATE, 4000);
	
	//generateHF(samplebuffer, samples / 2);
	
	//generateTone(samplebuffer, samples / 4, PCM_RATE, 21430, 8000);

	//generateTone(samplebuffer, samples, PCM_RATE, 10, 8000);
	//generateHF(samplebuffer + samples / 2, 88, 8000);
	
	int taktung = 441 * 5;
	
	printf("taktung: %d samples\n", taktung);
	printf("maxbits: %d bits\n", samples / taktung);
	
	int bits = 10;
	
	for (framepos = 0; framepos + taktung < samples; framepos += taktung)
	{
		generateFadeHF(samplebuffer + framepos, 44, 16000);
		
		if (--bits == 0) break;
	}

	//generateHF(samplebuffer + samples / 4, samples / 2, 8000);

	if (0)
	{
		FILE *fd = fopen("./output.pcm", "w");
		fwrite(samplebuffer, samples, 2, fd);
		fclose(fd);
	}
	
	while (1)
	{
		for (framepos = 0; framepos + frames < samples; framepos += frames)
		{
			if (perr = snd_pcm_writei(pcm_handle, samplebuffer + framepos, frames) < 0) 
			{
				printf("ERROR: At playing. %s\n", snd_strerror(perr));
			}
		}
		
		printf("Played\n");
	}
	
	free(samplebuffer);

	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);

	return 0;
}

