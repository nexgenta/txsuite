/*
 * BitmapClass.h
 */

#ifndef __BITMAPCLASS_H__
#define __BITMAPCLASS_H__

#include "ISO13522-MHEG-5.h"

void BitmapClass_Preparation(BitmapClass *);
void BitmapClass_Activation(BitmapClass *);
void BitmapClass_Deactivation(BitmapClass *);
void BitmapClass_Destruction(BitmapClass *);

void BitmapClass_SetData(BitmapClass *, SetData *, OctetString *);
void BitmapClass_Clone(BitmapClass *, Clone *, OctetString *);
void BitmapClass_SetPosition(BitmapClass *, SetPosition *, OctetString *);
void BitmapClass_GetPosition(BitmapClass *, GetPosition *, OctetString *);
void BitmapClass_SetBoxSize(BitmapClass *, SetBoxSize *, OctetString *);
void BitmapClass_GetBoxSize(BitmapClass *, GetBoxSize *, OctetString *);
void BitmapClass_BringToFront(BitmapClass *);
void BitmapClass_SendToBack(BitmapClass *);
void BitmapClass_PutBefore(BitmapClass *, PutBefore *, OctetString *);
void BitmapClass_PutBehind(BitmapClass *, PutBehind *, OctetString *);
void BitmapClass_SetPaletteRef(BitmapClass *, SetPaletteRef *, OctetString *);
void BitmapClass_SetBitmapDecodeOffset(BitmapClass *, SetBitmapDecodeOffset *, OctetString *);
void BitmapClass_GetBitmapDecodeOffset(BitmapClass *, GetBitmapDecodeOffset *, OctetString *);
void BitmapClass_ScaleBitmap(BitmapClass *, ScaleBitmap *, OctetString *);
void BitmapClass_SetTransparency(BitmapClass *, SetTransparency *, OctetString *);

void BitmapClass_contentAvailable(BitmapClass *, OctetString *);

void BitmapClass_render(BitmapClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __BITMAPCLASS_H__ */

