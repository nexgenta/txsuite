/*
 * MHEGAudioOutput.h
 */

#include <stdbool.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

#ifndef __MHEGAUDIOOUTPUT_H__
#define __MHEGAUDIOOUTPUT_H__

typedef struct
{
	snd_pcm_t *ctx;
} MHEGAudioOutput;

/* default ALSA device */
#define ALSA_AUDIO_DEVICE	"plughw"

bool MHEGAudioOutput_init(MHEGAudioOutput *);
void MHEGAudioOutput_fini(MHEGAudioOutput *);

bool MHEGAudioOutput_setParams(MHEGAudioOutput *, snd_pcm_format_t, unsigned int, unsigned int);

void MHEGAudioOutput_addSamples(MHEGAudioOutput *, uint16_t *, unsigned int);

#endif	/* __MHEGAUDIOOUTPUT_H__ */
