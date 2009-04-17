/*
 * si.h
 */

#ifndef __SI_H__
#define __SI_H__

#include "der_decode.h"

int si_get_index(OctetString *);
OctetString *si_get_url(int);

bool si_tune_index(int);

unsigned int si_get_network_id(OctetString *);
unsigned int si_get_transport_id(OctetString *);
unsigned int si_get_service_id(OctetString *);

void si_free(void);

#endif	/* __SI_H__ */
