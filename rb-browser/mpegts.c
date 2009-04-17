/*
 * mpegts.c
 */

/*
 * MPEG2 Transport Stream demuxer
 * based on ffmpeg/libavformat code
 * changed to avoid any seeking on the input
 */

/*
 * MPEG2 transport stream (aka DVB) demux
 * Copyright (c) 2002-2003 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ffmpeg/avformat.h>

#include "mpegts.h"
#include "utils.h"
#include "MHEGEngine.h"

#define TS_PACKET_SIZE	188
/* expands as necessary */
#define INIT_FRAME_BUFF_SIZE	(128 * 1024)

/* TS stream handling */
enum MpegTSState
{
	MPEGTS_SKIP = 0,	/* must be 0, => don't start getting data until we have set is_start flag */
	MPEGTS_HEADER,
	MPEGTS_PESHEADER_FILL,
	MPEGTS_PAYLOAD,
};

/* enough for PES header + length */
#define PES_START_SIZE		9
#define MAX_PES_HEADER_SIZE	(9 + 255)

struct PESContext
{
	int pid;
	MpegTSContext *ts;
	enum MpegTSState state;
	/* used to get the format */
	int data_index;
	int total_size;
	int pes_header_size;
	int64_t pts, dts;
	uint8_t header[MAX_PES_HEADER_SIZE];
	/* frame we are currently building for this stream */
	int64_t frame_pts;
	int64_t frame_dts;
	uint8_t *frame_data;
	unsigned int frame_size;	/* number of bytes of valid data in frame_data */
	unsigned int alloc_size;	/* number of bytes malloc'ed to frame_data */
};

typedef struct PESContext PESContext;

struct MpegTSContext
{
	FILE *ts_stream;	/* transport stream we are reading from */
	int apid;		/* audio PID we want, -1 => no audio */
	int vpid;		/* video PID we want, -1 => no video */
	AVPacket *pkt;		/* packet containing av data */
	int stop_parse;		/* stop parsing loop */
	PESContext apes;	/* audio PES we are demuxing */
	PESContext vpes;	/* video PES we are demuxing */
	int is_start;		/* is the current packet the first of a frame */
	int last_cc;		/* last Continuity Check value we saw (<0 => none seen yet) */
};

static int read_packet(FILE *, uint8_t *);
static void handle_packet(MpegTSContext *, const uint8_t *);
static int init_pes_stream(MpegTSContext *, PESContext *, int);
static void free_pes_stream(PESContext *);
static PESContext *find_pes_stream(MpegTSContext *, int);
static void mpegts_push_data(PESContext *, const uint8_t *, int, int);
static int64_t get_pts(const uint8_t *);

/* my interface */

/*
 * demux the given audio and video PIDs from the transport stream
 * if apid or vpid is -1, ignore it
 */

MpegTSContext *
mpegts_open(FILE *ts, int apid, int vpid)
{
	MpegTSContext *ctx;

	if((ctx = av_mallocz(sizeof(MpegTSContext))) == NULL)
		return NULL;

	ctx->ts_stream = ts;
	ctx->apid = apid;
	ctx->vpid = vpid;

	if(init_pes_stream(ctx, &ctx->apes, apid) < 0)
	{
		av_free(ctx);
		return NULL;
	}
	if(init_pes_stream(ctx, &ctx->vpes, vpid) < 0)
	{
		free_pes_stream(&ctx->apes);
		av_free(ctx);
		return NULL;
	}

	ctx->is_start = 0;

	/* last continuity check value (-1 => CC always passes) */
	ctx->last_cc = -1;

	return ctx;
}

int
mpegts_demux_frame(MpegTSContext *ctx, AVPacket *frame)
{
	AVPacket packet;
	PESContext *pes;
	uint8_t *frame_data;

	do
	{
		if(mpegts_demux_packet(ctx, &packet) < 0)
		{
			av_free_packet(&packet);
			return -1;
		}
		/* find the stream */
		if((pes = find_pes_stream(ctx, packet.stream_index)) == NULL)
			fatal("mpegts_demux_frame: unexpected PID %d", packet.stream_index);
		/* is it the first packet of the next frame */
		if(ctx->is_start == 0)
		{
			/* not a new frame, add data to the exisiting one */
			if((frame_data = av_fast_realloc(pes->frame_data, &pes->alloc_size, pes->frame_size + packet.size)) == NULL)
			{
				av_free_packet(&packet);
				return -1;
			}
			pes->frame_data = frame_data;
			memcpy(pes->frame_data + pes->frame_size, packet.data, packet.size);
			pes->frame_size += packet.size;
			av_free_packet(&packet);
		}
	}
	while(ctx->is_start == 0);

	/*
	 * pes->frame_data contains the last frame
	 * copy it into the output packet
	 * packet contains the first packet of the next frame
	 * copy it into the PES context for the stream
	 */
	if(av_new_packet(frame, pes->frame_size) != 0)
	{
		av_free_packet(&packet);
		return -1;
	}
	memcpy(frame->data, pes->frame_data, pes->frame_size);

	frame->stream_index = pes->pid;
	frame->pts = pes->frame_pts;
	frame->dts = pes->frame_dts;

	/* copy the first packet of the next frame into PES context */
	if((frame_data = av_fast_realloc(pes->frame_data, &pes->alloc_size, packet.size)) == NULL)
	{
		av_free_packet(&packet);
		av_free_packet(frame);
		return -1;
	}
	pes->frame_data = frame_data;
	pes->frame_size = packet.size;
	memcpy(pes->frame_data, packet.data, packet.size);

	/* remember the new frame's PTS (or calc from the previous one) */
	if(packet.pts == AV_NOPTS_VALUE && pes->frame_pts != AV_NOPTS_VALUE)
		pes->frame_pts += 3600;
	else
		pes->frame_pts = packet.pts;
	if(packet.dts == AV_NOPTS_VALUE && pes->frame_dts != AV_NOPTS_VALUE)
		pes->frame_dts += 3600;
	else
		pes->frame_dts = packet.dts;

	av_free_packet(&packet);

	return 0;
}

int
mpegts_demux_packet(MpegTSContext *ctx, AVPacket *pkt)
{
	uint8_t packet[TS_PACKET_SIZE];
	int ret;

	ctx->pkt = pkt;

	ctx->stop_parse = 0;
	do
	{
		if((ret = read_packet(ctx->ts_stream, packet)) != 0)
			return ret;
		handle_packet(ctx, packet);
	}
	while(ctx->stop_parse == 0);

	return 0;
}

void
mpegts_close(MpegTSContext *ctx)
{
	free_pes_stream(&ctx->apes);
	free_pes_stream(&ctx->vpes);

	av_free(ctx);

	return;
}

/* internal functions */

/* return -1 if error or EOF. Return 0 if OK. */
#define TS_SYNC_BYTE	0x47

static int
read_packet(FILE *ts, uint8_t *buf)
{
	size_t nread;
	unsigned int resync;

	/* find the next sync byte */
	resync = 0;
	nread = 0;
	*buf = 0;
	do
	{
		/* read the whole of the next packet */
		while(nread != TS_PACKET_SIZE && !feof(ts))
			nread += fread(buf + nread, 1, TS_PACKET_SIZE - nread, ts);
		if(nread > 0 && *buf != TS_SYNC_BYTE && !feof(ts))
		{
			resync ++;
			memmove(buf, buf + 1, nread - 1);
			buf[nread - 1] = 0;
			nread --;
		}
	}
	while(nread < TS_PACKET_SIZE && *buf != TS_SYNC_BYTE && !feof(ts));

	if(feof(ts))
		return -1;

	if(resync > 0)
		verbose("MPEG TS demux: lost sync; skipped %u bytes", resync);

	return 0;
}

/* handle one TS packet */
static void
handle_packet(MpegTSContext *ctx, const uint8_t *packet)
{
	PESContext *pes;
	int cc, cc_ok;
	int pid, afc;
	const uint8_t *p, *p_end;

	pid = ((packet[1] & 0x1f) << 8) | packet[2];

	if((pes = find_pes_stream(ctx, pid)) == NULL)
	{
		verbose("MPEG TS demux: ignoring unexpected PID %d", pid);
		return;
	}

	ctx->is_start = packet[1] & 0x40;

	/* continuity check */
	cc = (packet[3] & 0xf);
	cc_ok = (ctx->last_cc < 0) || (((ctx->last_cc + 1) & 0x0f) == cc);
	ctx->last_cc = cc;
#if 0
	/* skip until we find the next start packet */
	if(!cc_ok && !ctx->is_start)
		pes->state = MPEGTS_SKIP;
#endif

	/* skip adaptation field */
	afc = (packet[3] >> 4) & 3;
	p = packet + 4;
	if(afc == 0)		/* reserved value */
		return;
	if(afc == 2)		/* adaptation field only */
		return;
	if(afc == 3)		/* skip adapation field */
		p += p[0] + 1;

	/* if past the end of packet, ignore */
	p_end = packet + TS_PACKET_SIZE;
	if(p >= p_end)
		return;

	mpegts_push_data(pes, p, p_end - p, ctx->is_start);

	return;
}

static int
init_pes_stream(MpegTSContext *ctx, PESContext *pes, int pid)
{
	bzero(pes, sizeof(PESContext));

	pes->ts = ctx;
	pes->pid = pid;

	pes->alloc_size = INIT_FRAME_BUFF_SIZE;
	if((pes->frame_data = av_malloc(pes->alloc_size)) == NULL)
		return -1;

	pes->frame_pts = AV_NOPTS_VALUE;
	pes->frame_dts = AV_NOPTS_VALUE;

	return 0;
}

static void
free_pes_stream(PESContext *pes)
{
	if(pes->frame_data)
		av_free(pes->frame_data);

	return;
}

static PESContext *
find_pes_stream(MpegTSContext *ctx, int pid)
{
	if(pid == ctx->apid)
		return &ctx->apes;
	else if(pid == ctx->vpid)
		return &ctx->vpes;
	else
		return NULL;
}

/* return non zero if a packet could be constructed */
static void
mpegts_push_data(PESContext *pes, const uint8_t *buf, int buf_size, int is_start)
{
	MpegTSContext *ts = pes->ts;
	const uint8_t *p;
	int len, code;

	if(is_start)
	{
		pes->state = MPEGTS_HEADER;
		pes->data_index = 0;
	}
	p = buf;
	while(buf_size > 0)
	{
		switch(pes->state)
		{
		case MPEGTS_HEADER:
			len = PES_START_SIZE - pes->data_index;
			if(len > buf_size)
				len = buf_size;
			memcpy(pes->header + pes->data_index, p, len);
			pes->data_index += len;
			p += len;
			buf_size -= len;
			if(pes->data_index == PES_START_SIZE)
			{
				/* we got all the PES or section header. We can now decide */
				if(pes->header[0] == 0x00
				&& pes->header[1] == 0x00
				&& pes->header[2] == 0x01)
				{
					/* it must be an mpeg2 PES stream */
					code = pes->header[3] | 0x100;
					if(!((code >= 0x1c0 && code <= 0x1df)
					|| (code >= 0x1e0 && code <= 0x1ef)
					|| (code == 0x1bd)))
						goto skip;
					pes->state = MPEGTS_PESHEADER_FILL;
					pes->total_size = (pes->header[4] << 8) | pes->header[5];
					/* NOTE: a zero total size means the PES size is unbounded */
					if(pes->total_size)
						pes->total_size += 6;
					pes->pes_header_size = pes->header[8] + 9;
				}
				else
				{
					/* otherwise, it should be a table */
					/* skip packet */
				skip:
					pes->state = MPEGTS_SKIP;
					continue;
				}
			}
			break;

		case MPEGTS_PESHEADER_FILL:
			/* PES packing parsing */
			len = pes->pes_header_size - pes->data_index;
			if(len > buf_size)
				len = buf_size;
			memcpy(pes->header + pes->data_index, p, len);
			pes->data_index += len;
			p += len;
			buf_size -= len;
			if(pes->data_index == pes->pes_header_size)
			{
				const uint8_t *r;
				unsigned int flags;
				flags = pes->header[7];
				r = pes->header + 9;
				pes->pts = AV_NOPTS_VALUE;
				pes->dts = AV_NOPTS_VALUE;
				if((flags & 0xc0) == 0x80)
				{
					pes->pts = get_pts(r);
					r += 5;
				}
				else if((flags & 0xc0) == 0xc0)
				{
					pes->pts = get_pts(r);
					r += 5;
					pes->dts = get_pts(r);
					r += 5;
				}
				/* we got the full header. We parse it and get the payload */
				pes->state = MPEGTS_PAYLOAD;
			}
			break;

		case MPEGTS_PAYLOAD:
			if(pes->total_size)
			{
				len = pes->total_size - pes->data_index;
				if(len > buf_size)
					len = buf_size;
			}
			else
			{
				len = buf_size;
			}
			if(len > 0)
			{
				AVPacket *pkt = ts->pkt;
				if(av_new_packet(pkt, len) == 0)
				{
					memcpy(pkt->data, p, len);
					pkt->stream_index = pes->pid;
					pkt->pts = pes->pts;
					pkt->dts = pes->dts;
					/* reset pts values */
					pes->pts = AV_NOPTS_VALUE;
					pes->dts = AV_NOPTS_VALUE;
					ts->stop_parse = 1;
					return;
				}
			}
			buf_size = 0;
			break;

		case MPEGTS_SKIP:
			buf_size = 0;
			break;
		}
	}

	return;
}

static int64_t
get_pts(const uint8_t *p)
{
	int64_t pts;
	int val;

	pts = (int64_t)((p[0] >> 1) & 0x07) << 30;
	val = (p[1] << 8) | p[2];
	pts |= (int64_t)(val >> 1) << 15;
	val = (p[3] << 8) | p[4];
	pts |= (int64_t)(val >> 1);

	return pts;
}

