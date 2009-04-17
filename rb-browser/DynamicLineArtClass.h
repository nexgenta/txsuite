/*
 * DynamicLineArtClass.h
 */

#ifndef __DYNAMICLINEARTCLASS_H__
#define __DYNAMICLINEARTCLASS_H__

#include "ISO13522-MHEG-5.h"

void DynamicLineArtClass_Preparation(DynamicLineArtClass *);
void DynamicLineArtClass_Activation(DynamicLineArtClass *);
void DynamicLineArtClass_Deactivation(DynamicLineArtClass *);
void DynamicLineArtClass_Destruction(DynamicLineArtClass *);

void DynamicLineArtClass_Clone(DynamicLineArtClass *, Clone *, OctetString *);
void DynamicLineArtClass_SetPosition(DynamicLineArtClass *, SetPosition *, OctetString *);
void DynamicLineArtClass_GetPosition(DynamicLineArtClass *, GetPosition *, OctetString *);
void DynamicLineArtClass_SetBoxSize(DynamicLineArtClass *, SetBoxSize *, OctetString *);
void DynamicLineArtClass_GetBoxSize(DynamicLineArtClass *, GetBoxSize *, OctetString *);
void DynamicLineArtClass_BringToFront(DynamicLineArtClass *);
void DynamicLineArtClass_SendToBack(DynamicLineArtClass *);
void DynamicLineArtClass_PutBefore(DynamicLineArtClass *, PutBefore *, OctetString *);
void DynamicLineArtClass_PutBehind(DynamicLineArtClass *, PutBehind *, OctetString *);
void DynamicLineArtClass_SetPaletteRef(DynamicLineArtClass *, SetPaletteRef *, OctetString *);
void DynamicLineArtClass_SetLineWidth(DynamicLineArtClass *, SetLineWidth *, OctetString *);
void DynamicLineArtClass_SetLineStyle(DynamicLineArtClass *, SetLineStyle *, OctetString *);
void DynamicLineArtClass_SetLineColour(DynamicLineArtClass *, SetLineColour *, OctetString *);
void DynamicLineArtClass_SetFillColour(DynamicLineArtClass *, SetFillColour *, OctetString *);
void DynamicLineArtClass_GetLineWidth(DynamicLineArtClass *, GetLineWidth *, OctetString *);
void DynamicLineArtClass_GetLineStyle(DynamicLineArtClass *, GetLineStyle *, OctetString *);
void DynamicLineArtClass_GetLineColour(DynamicLineArtClass *, GetLineColour *, OctetString *);
void DynamicLineArtClass_GetFillColour(DynamicLineArtClass *, GetFillColour *, OctetString *);
void DynamicLineArtClass_DrawArc(DynamicLineArtClass *, DrawArc *, OctetString *);
void DynamicLineArtClass_DrawSector(DynamicLineArtClass *, DrawSector *, OctetString *);
void DynamicLineArtClass_DrawLine(DynamicLineArtClass *, DrawLine *, OctetString *);
void DynamicLineArtClass_DrawOval(DynamicLineArtClass *, DrawOval *, OctetString *);
void DynamicLineArtClass_DrawPolygon(DynamicLineArtClass *, DrawPolygon *, OctetString *);
void DynamicLineArtClass_DrawPolyline(DynamicLineArtClass *, DrawPolyline *, OctetString *);
void DynamicLineArtClass_DrawRectangle(DynamicLineArtClass *, DrawRectangle *, OctetString *);
void DynamicLineArtClass_Clear(DynamicLineArtClass *);

void DynamicLineArtClass_render(DynamicLineArtClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __DYNAMICLINEARTCLASS_H__ */

