/*
 * TokenGroupClass.h
 */

#ifndef __TOKENGROUPCLASS_H__
#define __TOKENGROUPCLASS_H__

#include "ISO13522-MHEG-5.h"

void TokenGroupClass_Preparation(TokenGroupClass *);
void TokenGroupClass_Activation(TokenGroupClass *);
void TokenGroupClass_Deactivation(TokenGroupClass *);
void TokenGroupClass_Destruction(TokenGroupClass *);
void TokenGroupClass_TransferToken(TokenGroupClass *, unsigned int);

void TokenGroupClass_Clone(TokenGroupClass *, Clone *, OctetString *);
void TokenGroupClass_Move(TokenGroupClass *, Move *, OctetString *);
void TokenGroupClass_MoveTo(TokenGroupClass *, MoveTo *, OctetString *);
void TokenGroupClass_GetTokenPosition(TokenGroupClass *, GetTokenPosition *, OctetString *);
void TokenGroupClass_CallActionSlot(TokenGroupClass *, CallActionSlot *, OctetString *);

#endif	/* __TOKENGROUPCLASS_H__ */

