/*
 * VideoClass.h
 */

#ifndef __VIDEOCLASS_H__
#define __VIDEOCLASS_H__

#include "ISO13522-MHEG-5.h"

void VideoClass_Preparation(VideoClass *);
void VideoClass_Activation(VideoClass *);
void VideoClass_Deactivation(VideoClass *);
void VideoClass_Destruction(VideoClass *);

void VideoClass_SetPosition(VideoClass *, SetPosition *, OctetString *);
void VideoClass_GetPosition(VideoClass *, GetPosition *, OctetString *);
void VideoClass_SetBoxSize(VideoClass *, SetBoxSize *, OctetString *);
void VideoClass_GetBoxSize(VideoClass *, GetBoxSize *, OctetString *);
void VideoClass_BringToFront(VideoClass *);
void VideoClass_SendToBack(VideoClass *);
void VideoClass_PutBefore(VideoClass *, PutBefore *, OctetString *);
void VideoClass_PutBehind(VideoClass *, PutBehind *, OctetString *);
void VideoClass_SetVideoDecodeOffset(VideoClass *, SetVideoDecodeOffset *, OctetString *);
void VideoClass_GetVideoDecodeOffset(VideoClass *, GetVideoDecodeOffset *, OctetString *);
void VideoClass_ScaleVideo(VideoClass *, ScaleVideo *, OctetString *);

void VideoClass_render(VideoClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __VIDEOCLASS_H__ */


