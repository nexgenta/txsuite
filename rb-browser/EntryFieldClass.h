/*
 * EntryFieldClass.h
 */

#ifndef __ENTRYFIELDCLASS_H__
#define __ENTRYFIELDCLASS_H__

#include "ISO13522-MHEG-5.h"

void EntryFieldClass_Preparation(EntryFieldClass *);
void EntryFieldClass_Activation(EntryFieldClass *);
void EntryFieldClass_Deactivation(EntryFieldClass *);
void EntryFieldClass_Destruction(EntryFieldClass *);

void EntryFieldClass_SetData(EntryFieldClass *, SetData *, OctetString *);
void EntryFieldClass_Clone(EntryFieldClass *, Clone *, OctetString *);
void EntryFieldClass_SetPosition(EntryFieldClass *, SetPosition *, OctetString *);
void EntryFieldClass_GetPosition(EntryFieldClass *, GetPosition *, OctetString *);
void EntryFieldClass_SetBoxSize(EntryFieldClass *, SetBoxSize *, OctetString *);
void EntryFieldClass_GetBoxSize(EntryFieldClass *, GetBoxSize *, OctetString *);
void EntryFieldClass_BringToFront(EntryFieldClass *);
void EntryFieldClass_SendToBack(EntryFieldClass *);
void EntryFieldClass_PutBefore(EntryFieldClass *, PutBefore *, OctetString *);
void EntryFieldClass_PutBehind(EntryFieldClass *, PutBehind *, OctetString *);
void EntryFieldClass_SetPaletteRef(EntryFieldClass *, SetPaletteRef *, OctetString *);
void EntryFieldClass_SetBackgroundColour(EntryFieldClass *, SetBackgroundColour *, OctetString *);
void EntryFieldClass_SetTextColour(EntryFieldClass *, SetTextColour *, OctetString *);
void EntryFieldClass_SetFontAttributes(EntryFieldClass *, SetFontAttributes *, OctetString *);
void EntryFieldClass_GetTextContent(EntryFieldClass *, GetTextContent *, OctetString *);
void EntryFieldClass_GetTextData(EntryFieldClass *, GetTextData *, OctetString *);
void EntryFieldClass_SetFontRef(EntryFieldClass *, SetFontRef *, OctetString *);
void EntryFieldClass_SetInteractionStatus(EntryFieldClass *, SetInteractionStatus *, OctetString *);
void EntryFieldClass_GetInteractionStatus(EntryFieldClass *, GetInteractionStatus *, OctetString *);
void EntryFieldClass_SetHighlightStatus(EntryFieldClass *, SetHighlightStatus *, OctetString *);
void EntryFieldClass_GetHighlightStatus(EntryFieldClass *, GetHighlightStatus *, OctetString *);
void EntryFieldClass_SetOverwriteMode(EntryFieldClass *, SetOverwriteMode *, OctetString *);
void EntryFieldClass_GetOverwriteMode(EntryFieldClass *, GetOverwriteMode *, OctetString *);
void EntryFieldClass_SetEntryPoint(EntryFieldClass *, SetEntryPoint *, OctetString *);
void EntryFieldClass_GetEntryPoint(EntryFieldClass *, GetEntryPoint *, OctetString *);

void EntryFieldClass_contentAvailable(EntryFieldClass *, OctetString *);

void EntryFieldClass_render(EntryFieldClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __ENTRYFIELDCLASS_H__ */

