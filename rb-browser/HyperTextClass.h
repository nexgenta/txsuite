/*
 * HyperTextClass.h
 */

#ifndef __HYPERTEXTCLASS_H__
#define __HYPERTEXTCLASS_H__

#include "ISO13522-MHEG-5.h"

void HyperTextClass_Preparation(HyperTextClass *);
void HyperTextClass_Activation(HyperTextClass *);
void HyperTextClass_Deactivation(HyperTextClass *);
void HyperTextClass_Destruction(HyperTextClass *);

void HyperTextClass_SetData(HyperTextClass *, SetData *, OctetString *);
void HyperTextClass_Clone(HyperTextClass *, Clone *, OctetString *);
void HyperTextClass_SetPosition(HyperTextClass *, SetPosition *, OctetString *);
void HyperTextClass_GetPosition(HyperTextClass *, GetPosition *, OctetString *);
void HyperTextClass_SetBoxSize(HyperTextClass *, SetBoxSize *, OctetString *);
void HyperTextClass_GetBoxSize(HyperTextClass *, GetBoxSize *, OctetString *);
void HyperTextClass_BringToFront(HyperTextClass *);
void HyperTextClass_SendToBack(HyperTextClass *);
void HyperTextClass_PutBefore(HyperTextClass *, PutBefore *, OctetString *);
void HyperTextClass_PutBehind(HyperTextClass *, PutBehind *, OctetString *);
void HyperTextClass_SetPaletteRef(HyperTextClass *, SetPaletteRef *, OctetString *);
void HyperTextClass_SetBackgroundColour(HyperTextClass *, SetBackgroundColour *, OctetString *);
void HyperTextClass_SetTextColour(HyperTextClass *, SetTextColour *, OctetString *);
void HyperTextClass_SetFontAttributes(HyperTextClass *, SetFontAttributes *, OctetString *);
void HyperTextClass_GetFocusPosition(HyperTextClass *, GetFocusPosition *, OctetString *);
void HyperTextClass_SetFocusPosition(HyperTextClass *, SetFocusPosition *, OctetString *);
void HyperTextClass_GetTextContent(HyperTextClass *, GetTextContent *, OctetString *);
void HyperTextClass_GetTextData(HyperTextClass *, GetTextData *, OctetString *);
void HyperTextClass_SetFontRef(HyperTextClass *, SetFontRef *, OctetString *);
void HyperTextClass_SetInteractionStatus(HyperTextClass *, SetInteractionStatus *, OctetString *);
void HyperTextClass_GetInteractionStatus(HyperTextClass *, GetInteractionStatus *, OctetString *);
void HyperTextClass_SetHighlightStatus(HyperTextClass *, SetHighlightStatus *, OctetString *);
void HyperTextClass_GetHighlightStatus(HyperTextClass *, GetHighlightStatus *, OctetString *);
void HyperTextClass_GetLastAnchorFired(HyperTextClass *, GetLastAnchorFired *, OctetString *);

void HyperTextClass_contentAvailable(HyperTextClass *, OctetString *);

void HyperTextClass_render(HyperTextClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __HYPERTEXTCLASS_H__ */

