/*
 * StreamComponent.c
 */

#include "MHEGEngine.h"
#include "StreamComponent.h"
#include "AudioClass.h"
#include "VideoClass.h"
#include "RTGraphicsClass.h"
#include "utils.h"

void
StreamComponent_registerStreamClass(StreamComponent *s, StreamClass *owner)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		s->u.audio.inst.owner = owner;
		break;

	case StreamComponent_video:
		s->u.video.inst.owner = owner;
		break;

	case StreamComponent_rtgraphics:
		s->u.rtgraphics.inst.owner = owner;
		break;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		break;
	}

	return;
}

RootClass *
StreamComponent_rootClass(StreamComponent *s)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		return &s->u.audio.rootClass;

	case StreamComponent_video:
		return &s->u.video.rootClass;

	case StreamComponent_rtgraphics:
		return &s->u.rtgraphics.rootClass;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		return NULL;
	}
}

bool
StreamComponent_isInitiallyActive(StreamComponent *s)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		return s->u.audio.initially_active;

	case StreamComponent_video:
		return s->u.video.initially_active;

	case StreamComponent_rtgraphics:
		return s->u.rtgraphics.initially_active;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		return false;
	}
}

void
StreamComponent_Activation(StreamComponent *s)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		AudioClass_Activation(&s->u.audio);
		break;

	case StreamComponent_video:
		VideoClass_Activation(&s->u.video);
		break;

	case StreamComponent_rtgraphics:
		RTGraphicsClass_Activation(&s->u.rtgraphics);
		break;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		break;
	}

	return;
}

void
StreamComponent_Destruction(StreamComponent *s)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		AudioClass_Destruction(&s->u.audio);
		break;

	case StreamComponent_video:
		VideoClass_Destruction(&s->u.video);
		break;

	case StreamComponent_rtgraphics:
		RTGraphicsClass_Destruction(&s->u.rtgraphics);
		break;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		break;
	}

	return;
}

void
StreamComponent_play(StreamComponent *s, MHEGStreamPlayer *player)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		MHEGStreamPlayer_setAudioStream(player, &s->u.audio);
		break;

	case StreamComponent_video:
		MHEGStreamPlayer_setVideoStream(player, &s->u.video);
		break;

	case StreamComponent_rtgraphics:
		error("RTGraphics streams not supported");
		break;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		break;
	}

	return;
}

void
StreamComponent_stop(StreamComponent *s, MHEGStreamPlayer *player)
{
	switch(s->choice)
	{
	case StreamComponent_audio:
		MHEGStreamPlayer_setAudioStream(player, NULL);
		break;

	case StreamComponent_video:
		MHEGStreamPlayer_setVideoStream(player, NULL);
		break;

	case StreamComponent_rtgraphics:
		error("RTGraphics streams not supported");
		break;

	default:
		error("Unknown StreamComponent type: %d", s->choice);
		break;
	}

	return;
}

