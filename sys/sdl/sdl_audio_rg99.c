#include <stdio.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include "../../src/core/sound.h"

static int sound = 1;
#define AUDIO_HZ 22050
#define AUDIO_BUFFER_LENGTH 3696  //(AUDIO_HZ / 16 + 1)
#define RING_BUFFER_LENGTH (AUDIO_BUFFER_LENGTH * 4)
static n16 audioBuffer[RING_BUFFER_LENGTH + AUDIO_BUFFER_LENGTH]; // * audioBuffer;
static volatile int audio_done;

int32_t rBuf, wBuf;

int readvolume()
{
	return 0;
}

void pcm_silence()
{
	memset(audioBuffer, 0x80, sizeof(audioBuffer));	
}

void setvolume(int involume)
{
}

int32_t sdl_buflen(void)
{
	if (wBuf >= rBuf) return wBuf - rBuf;
	return RING_BUFFER_LENGTH + (wBuf - rBuf);
}

static void audio_callback(void *blah, uint8_t  *stream, int len)
{
	int pos;
	uint16_t *buffer = (uint16_t *) stream;

	len /= 2;

	int32_t apubuflack = len - (sdl_buflen() & ~(1L));

	if (apubuflack > 0){
		rBuf -= (apubuflack < rBuf) ? apubuflack : apubuflack - RING_BUFFER_LENGTH;
	}

	if (rBuf + len <= RING_BUFFER_LENGTH) {
		memcpy(buffer, audioBuffer + rBuf, len * sizeof(uint16_t));
		rBuf += len;
	} else {
		pos = RING_BUFFER_LENGTH - rBuf;
		memcpy(buffer, audioBuffer + rBuf, pos * sizeof(uint16_t));
		memcpy(buffer + pos, audioBuffer, (len - pos) * sizeof(uint16_t));
		rBuf = len - pos;
	}
}


void pcm_init()
{
	int i;
	SDL_AudioSpec as;
	
	memset(audioBuffer, 0x80, sizeof(audioBuffer));	

	if (!sound) return;

	if (SDL_Init(SDL_INIT_AUDIO))
	{
		printf("SDL: Couldn't initialize SDL sou,d: %s\n", SDL_GetError());
		exit(1);
	}

	as.freq = AUDIO_HZ;
	as.format = AUDIO_U16MSB;
	as.channels = 2;
	as.samples = AUDIO_BUFFER_LENGTH / 2;
	as.callback = audio_callback;
	as.userdata = 0;
	if (SDL_OpenAudio(&as, 0) == -1)
		return;

	pcm.hz = AUDIO_HZ;
	pcm.stereo = 1;
	pcm.len = AUDIO_BUFFER_LENGTH;
	pcm.buf = audioBuffer;
	pcm.pos = 0;
	// memset(pcm.buf, 0, pcm.len);

	rBuf = 0;
	wBuf = 0;

	SDL_PauseAudio(0);
}


extern int speedup;

int pcm_submit()
{
	int len;
	static int trigger = 0;

	if (speedup) {
		if (!trigger) {
			pcm.buf = audioBuffer;
			SDL_PauseAudio(1);
			trigger = 1;
		}
		pcm.pos = 0;
		return 1;
	} else if (trigger) {
		trigger = 0;
		rBuf = 0;
		wBuf = 0;
		memset(audioBuffer, 0x80, sizeof(uint16_t) * AUDIO_BUFFER_LENGTH);
		SDL_PauseAudio(0);
	}

	if (!pcm.buf) return 0;

	while (sdl_buflen() > (AUDIO_BUFFER_LENGTH * 2)) SDL_Delay(1);

	if (wBuf + pcm.pos < RING_BUFFER_LENGTH) {
		wBuf += pcm.pos;
	} else {
		len = wBuf + pcm.pos - RING_BUFFER_LENGTH;
		memcpy(audioBuffer, &audioBuffer[RING_BUFFER_LENGTH], len);
		wBuf = len;
	}
	pcm.buf = &audioBuffer[wBuf];

	pcm.pos = 0;
	return 1;
}

void pcm_close()
{
	if (sound)
		SDL_CloseAudio();
}
