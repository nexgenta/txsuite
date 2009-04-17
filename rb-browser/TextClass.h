/*
 * TextClass.h
 */

#ifndef __TEXTCLASS_H__
#define __TEXTCLASS_H__

#include "ISO13522-MHEG-5.h"

void TextClass_Preparation(TextClass *);
void TextClass_Activation(TextClass *);
void TextClass_Deactivation(TextClass *);
void TextClass_Destruction(TextClass *);

void TextClass_SetData(TextClass *, SetData *, OctetString *);
void TextClass_Clone(TextClass *, Clone *, OctetString *);
void TextClass_SetPosition(TextClass *, SetPosition *, OctetString *);
void TextClass_GetPosition(TextClass *, GetPosition *, OctetString *);
void TextClass_SetBoxSize(TextClass *, SetBoxSize *, OctetString *);
void TextClass_GetBoxSize(TextClass *, GetBoxSize *, OctetString *);
void TextClass_BringToFront(TextClass *);
void TextClass_SendToBack(TextClass *);
void TextClass_PutBefore(TextClass *, PutBefore *, OctetString *);
void TextClass_PutBehind(TextClass *, PutBehind *, OctetString *);
void TextClass_SetPaletteRef(TextClass *, SetPaletteRef *, OctetString *);
void TextClass_SetBackgroundColour(TextClass *, SetBackgroundColour *, OctetString *);
void TextClass_SetTextColour(TextClass *, SetTextColour *, OctetString *);
void TextClass_SetFontAttributes(TextClass *, SetFontAttributes *, OctetString *);
void TextClass_GetTextContent(TextClass *, GetTextContent *, OctetString *);
void TextClass_GetTextData(TextClass *, GetTextData *, OctetString *);
void TextClass_SetFontRef(TextClass *, SetFontRef *, OctetString *);

void TextClass_contentAvailable(TextClass *, OctetString *);

void TextClass_render(TextClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __TEXTCLASS_H__ */

