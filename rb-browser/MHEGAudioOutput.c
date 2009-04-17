/*
 * MHEGAudioOutput.c
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "MHEGAudioOutput.h"
#include "MHEGEngine.h"
#include "utils.h"

bool
MHEGAudioOutput_init(MHEGAudioOutput *a)
{
	int err;

	a->ctx = NULL;

	if((err = snd_pcm_open(&a->ctx, ALSA_AUDIO_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		error("Unable to open audio device '%s': %s", ALSA_AUDIO_DEVICE, snd_strerror(err));
		return false;
	}

	return true;
}

void
MHEGAudioOutput_fini(MHEGAudioOutput *a)
{
	if(a->ctx != NULL)
	{
		snd_pcm_close(a->ctx);
		a->ctx = NULL;
	}

	return;
}

bool
MHEGAudioOutput_setParams(MHEGAudioOutput *a, snd_pcm_format_t format, unsigned int rate, unsigned int channels)
{
	snd_pcm_hw_params_t *hw_params;
	int err;
	int dir;

	if(a->ctx == NULL)
		return false;

	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
	{
		error("No memory for audio parameters: %s", snd_strerror(err));
		return false;
	}

	if((err = snd_pcm_hw_params_any(a->ctx, hw_params)) < 0)
	{
		error("Unable to set audio parameters: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	/* interleaved samples */
	if((err = snd_pcm_hw_params_set_access(a->ctx, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		error("Unable to set audio access: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	if((err = snd_pcm_hw_params_set_format(a->ctx, hw_params, format)) < 0)
	{
		error("Unable to set audio format: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	if((err = snd_pcm_hw_params_set_rate_near(a->ctx, hw_params, &rate, &dir)) < 0)
	{
		error("Unable to set audio sample rate: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	if((err = snd_pcm_hw_params_set_channels(a->ctx, hw_params, channels)) < 0)
	{
		error("Unable to set audio channels: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	if((err = snd_pcm_hw_params(a->ctx, hw_params)) < 0)
	{
		error("Unable to set audio parameters: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return false;
	}

	snd_pcm_hw_params_free(hw_params);

	return true;
}

void
MHEGAudioOutput_addSamples(MHEGAudioOutput *a, uint16_t *samples, unsigned int nbytes)
{
	unsigned int nsamples;

	if(a->ctx == NULL)
		return;

	/* convert bytes to samples */
	nsamples = snd_pcm_bytes_to_frames(a->ctx, nbytes);

	/* interleaved samples */
	while(snd_pcm_writei(a->ctx, samples, nsamples) < 0)
	{
		snd_pcm_prepare(a->ctx);
		verbose("MHEGAudioOutput: buffer underrun");
	}

	return;
}

