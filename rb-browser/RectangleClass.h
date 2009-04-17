/*
 * RectangleClass.h
 */

#ifndef __RECTANGLECLASS_H__
#define __RECTANGLECLASS_H__

#include "ISO13522-MHEG-5.h"

void RectangleClass_Preparation(RectangleClass *);
void RectangleClass_Activation(RectangleClass *);
void RectangleClass_Deactivation(RectangleClass *);
void RectangleClass_Destruction(RectangleClass *);

void RectangleClass_Clone(RectangleClass *, Clone *, OctetString *);
void RectangleClass_SetPosition(RectangleClass *, SetPosition *, OctetString *);
void RectangleClass_GetPosition(RectangleClass *, GetPosition *, OctetString *);
void RectangleClass_SetBoxSize(RectangleClass *, SetBoxSize *, OctetString *);
void RectangleClass_GetBoxSize(RectangleClass *, GetBoxSize *, OctetString *);
void RectangleClass_BringToFront(RectangleClass *);
void RectangleClass_SendToBack(RectangleClass *);
void RectangleClass_PutBefore(RectangleClass *, PutBefore *, OctetString *);
void RectangleClass_PutBehind(RectangleClass *, PutBehind *, OctetString *);
void RectangleClass_SetPaletteRef(RectangleClass *, SetPaletteRef *, OctetString *);
void RectangleClass_SetLineWidth(RectangleClass *, SetLineWidth *, OctetString *);
void RectangleClass_SetLineStyle(RectangleClass *, SetLineStyle *, OctetString *);
void RectangleClass_SetLineColour(RectangleClass *, SetLineColour *, OctetString *);
void RectangleClass_SetFillColour(RectangleClass *, SetFillColour *, OctetString *);

void RectangleClass_render(RectangleClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __RECTANGLECLASS_H__ */

