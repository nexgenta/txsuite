/*
 * carousel.c
 */

/*
 * Copyright (C) 2005, Simon Kilvington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "carousel.h"
#include "table.h"
#include "dsmcc.h"
#include "biop.h"
#include "utils.h"

void
load_carousel(struct carousel *car)
{
	unsigned char table[MAX_TABLE_LEN];
	bool done;

	/* no modules yet */
	car->nmodules = 0;
	car->modules = NULL;

	/* see what the next DSMCC table is */
	done = false;
	do
	{
		struct dsmccMessageHeader *dsmcc;
		if(!read_dsmcc_tables(car, table))
			fatal("Unable to read PID");
		dsmcc = (struct dsmccMessageHeader *) &table[8];
		if(dsmcc->protocolDiscriminator == DSMCC_PROTOCOL
		&& dsmcc->dsmccType == DSMCC_TYPE_DOWNLOAD)
		{
			if(ntohs(dsmcc->messageId) == DSMCC_MSGID_DII)
				process_dii(car, (struct DownloadInfoIndication *) dsmccMessage(dsmcc), ntohl(dsmcc->transactionId));
			else if(ntohs(dsmcc->messageId) == DSMCC_MSGID_DSI)
				process_dsi(car, (struct DownloadServerInitiate *) dsmccMessage(dsmcc));
			else if(ntohs(dsmcc->messageId) == DSMCC_MSGID_DDB)
				process_ddb(car, (struct DownloadDataBlock *) dsmccMessage(dsmcc), ntohl(dsmcc->transactionId), DDB_blockDataLength(dsmcc));
			else
				error("Unknown DSMCC messageId: 0x%x", ntohs(dsmcc->messageId));
		}
	}
	while(!done);

	return;
}

void
process_dii(struct carousel *car, struct DownloadInfoIndication *dii, uint32_t transactionId)
{
	unsigned int nmodules;
	unsigned int i;

	verbose("DownloadInfoIndication");
	vverbose("transactionId: %u", transactionId);
	vverbose("downloadId: %u", ntohl(dii->downloadId));

	nmodules = DII_numberOfModules(dii);
	vverbose("numberOfModules: %u", nmodules);

	for(i=0; i<nmodules; i++)
	{
		struct DIIModule *mod;
		mod = DII_module(dii, i);
		vverbose("Module %u", i);
		vverbose(" moduleId: %u", ntohs(mod->moduleId));
		vverbose(" moduleVersion: %u", mod->moduleVersion);
		vverbose(" moduleSize: %u", ntohl(mod->moduleSize));
		if(find_module(car, ntohs(mod->moduleId), mod->moduleVersion, ntohl(dii->downloadId)) == NULL)
			add_module(car, dii, mod);
	}

	return;
}

void
process_dsi(struct carousel *car, struct DownloadServerInitiate *dsi)
{
	uint16_t elementary_pid;

	verbose("DownloadServerInitiate");

	/* only download the DSI from the boot PID */
	if(car->current_pid != car->boot_pid
	|| car->got_dsi)
		return;

	/* TODO: check for updated version */
	car->got_dsi = true;

	elementary_pid = process_biop_service_gateway_info(car->service_id, &car->assoc, DSI_privateDataByte(dsi), ntohs(dsi->privateDataLength));

	/* make sure we are downloading data from the PID the DSI refers to */
	add_dsmcc_pid(car, elementary_pid);

	return;
}

void
process_ddb(struct carousel *car, struct DownloadDataBlock *ddb, uint32_t downloadId, uint32_t blockLength)
{
	unsigned char *block;
	struct module *mod;

	verbose("DownloadDataBlock");

	vverbose("downloadId: %u", downloadId);
	vverbose("moduleId: %u", ntohs(ddb->moduleId));
	vverbose("moduleVersion: %u", ddb->moduleVersion);
	vverbose("blockNumber: %u", ntohs(ddb->blockNumber));

	vverbose("blockLength: %u", blockLength);
	block = DDB_blockDataByte(ddb);
	vhexdump(block, blockLength);

	if((mod = find_module(car, ntohs(ddb->moduleId), ddb->moduleVersion, downloadId)) != NULL)
		download_block(car, mod, ntohs(ddb->blockNumber), block, blockLength);

	return;
}

