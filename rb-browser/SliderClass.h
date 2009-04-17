/*
 * SliderClass.h
 */

#ifndef __SLIDERCLASS_H__
#define __SLIDERCLASS_H__

#include "ISO13522-MHEG-5.h"

void SliderClass_Preparation(SliderClass *);
void SliderClass_Activation(SliderClass *);
void SliderClass_Deactivation(SliderClass *);
void SliderClass_Destruction(SliderClass *);

void SliderClass_Clone(SliderClass *, Clone *, OctetString *);
void SliderClass_SetPosition(SliderClass *, SetPosition *, OctetString *);
void SliderClass_GetPosition(SliderClass *, GetPosition *, OctetString *);
void SliderClass_SetBoxSize(SliderClass *, SetBoxSize *, OctetString *);
void SliderClass_GetBoxSize(SliderClass *, GetBoxSize *, OctetString *);
void SliderClass_BringToFront(SliderClass *);
void SliderClass_SendToBack(SliderClass *);
void SliderClass_PutBefore(SliderClass *, PutBefore *, OctetString *);
void SliderClass_PutBehind(SliderClass *, PutBehind *, OctetString *);
void SliderClass_SetPaletteRef(SliderClass *, SetPaletteRef *, OctetString *);
void SliderClass_SetSliderParameters(SliderClass *, SetSliderParameters *, OctetString *);
void SliderClass_SetInteractionStatus(SliderClass *, SetInteractionStatus *, OctetString *);
void SliderClass_GetInteractionStatus(SliderClass *, GetInteractionStatus *, OctetString *);
void SliderClass_SetHighlightStatus(SliderClass *, SetHighlightStatus *, OctetString *);
void SliderClass_GetHighlightStatus(SliderClass *, GetHighlightStatus *, OctetString *);
void SliderClass_Step(SliderClass *, Step *, OctetString *);
void SliderClass_SetSliderValue(SliderClass *, SetSliderValue *, OctetString *);
void SliderClass_GetSliderValue(SliderClass *, GetSliderValue *, OctetString *);
void SliderClass_SetPortion(SliderClass *, SetPortion *, OctetString *);
void SliderClass_GetPortion(SliderClass *, GetPortion *, OctetString *);

void SliderClass_render(SliderClass *, MHEGDisplay *, XYPosition *, OriginalBoxSize *);

#endif	/* __SLIDERCLASS_H__ */

