/*
 * AudioClass.h
 */

#ifndef __AUDIOCLASS_H__
#define __AUDIOCLASS_H__

void AudioClass_Preparation(AudioClass *);
void AudioClass_Activation(AudioClass *);
void AudioClass_Deactivation(AudioClass *);
void AudioClass_Destruction(AudioClass *);

void AudioClass_SetVolume(AudioClass *, SetVolume *, OctetString *);
void AudioClass_GetVolume(AudioClass *, GetVolume *, OctetString *);

#endif	/* __AUDIOCLASS_H__ */
