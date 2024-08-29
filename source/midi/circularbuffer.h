/*
	circularbuffer.h
*/
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

typedef union {
	unsigned long rw;
	struct {
		unsigned short rdp;
		unsigned short wrp;
	};
} CCRPOS_t;

typedef struct {
	CCRPOS_t pos;
	unsigned short siz;
	unsigned char *buf;
} CCRBUF_t;

int putccrbuf(CCRBUF_t *ccr, unsigned char dt);
int getccrbuf(CCRBUF_t *ccr);
int getccrcnt(CCRBUF_t *ccr);
int getccrspc(CCRBUF_t *ccr);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	// CIRCULARBUFFER_H
