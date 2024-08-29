/*
	circularbuffer.c
*/

#include "circularbuffer.h"

int putccrbuf(CCRBUF_t *ccr, unsigned char dt)
{
	int result = -1;
	CCRPOS_t pos = {ccr->pos.rw};
	unsigned short wrp = pos.wrp;
	unsigned short nextwrp = wrp + 1;

	if (nextwrp >= ccr->siz)
	{
		nextwrp = 0;
	}
	if (pos.rdp != nextwrp)
	{
		(ccr->buf)[wrp] = dt;
		ccr->pos.wrp = nextwrp;
		result = dt;
	}

	return result;
}

int getccrbuf(CCRBUF_t *ccr)
{
	int result = -1;
	CCRPOS_t pos = {ccr->pos.rw};
	unsigned short rdp = pos.rdp;
	unsigned short nextrdp = rdp + 1;

	if (nextrdp >= ccr->siz)
	{
		nextrdp = 0;
	}
	if (rdp != pos.wrp)
	{
		result = (ccr->buf)[rdp];
		ccr->pos.rdp = nextrdp;
	}

	return result;
}

int getccrcnt(CCRBUF_t *ccr)
{
#if 1
	CCRPOS_t pos = {ccr->pos.rw};
	int result = pos.rdp > pos.wrp ? ccr->siz : 0;

	result = result + pos.wrp - pos.rdp;
#else
	int result = 0;
	CCRPOS_t pos = {ccr->pos.rw};

	if (pos.rdp < pos.wrp)
	{
		result = pos.wrp - pos.rdp;
	}
	else if (pos.rdp > pos.wrp)
	{
		result = ccr->siz - pos.rdp + pos.wrp;
	}
#endif

	return result;
}

int getccrspc(CCRBUF_t *ccr)
{
	CCRPOS_t pos = {ccr->pos.rw};
	int result = pos.rdp > pos.wrp ? 0 : ccr->siz;

	result = result + pos.rdp - pos.wrp - 1;

	return result;
}
