/*	$OpenBSD$	*/

/*
 * Copyright (c) 2019 Reyk Floeter <reyk@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/queue.h>

#ifndef HPACK_H
#define HPACK_H

struct hpack_table;

enum hpack_header_index {
	HPACK_NO_INDEX = 0,
	HPACK_NEVER_INDEX,
	HPACK_INDEX,
};

struct hpack_header {
	char				*hdr_name;
	char				*hdr_value;
	enum hpack_header_index		 hdr_index;
	TAILQ_ENTRY(hpack_header)	 hdr_entry;
};
TAILQ_HEAD(hpack_headerblock, hpack_header);

int	 hpack_init(void);

struct hpack_table
	*hpack_table_new(size_t);
void	 hpack_table_free(struct hpack_table *);
size_t	 hpack_table_size(struct hpack_table *);
int	 hpack_table_setsize(long, struct hpack_table *);

struct hpack_headerblock
	*hpack_decode(unsigned char *, size_t, struct hpack_table *);
unsigned char
	*hpack_encode(struct hpack_headerblock *, size_t *,
	    struct hpack_table *);

struct hpack_header
	*hpack_header_new(void);
struct hpack_header
	*hpack_header_add(struct hpack_headerblock *,
	    const char *, const char *, enum hpack_header_index);
void	 hpack_header_free(struct hpack_header *);
struct hpack_headerblock
	*hpack_headerblock_new(void);
void	 hpack_headerblock_free(struct hpack_headerblock *);

unsigned char
	*hpack_huffman_decode(unsigned char *, size_t, size_t *);
char	*hpack_huffman_decode_str(unsigned char *, size_t);
unsigned char
	*hpack_huffman_encode(unsigned char *, size_t, size_t *);

#ifdef HPACK_INTERNAL

#ifndef DEBUG
#define DPRINTF(x...)		do{} while(0)
#else
#define DPRINTF			warnx
#endif

/* from sys/param.h */
#define MAX(a,b)		(((a)>(b))?(a):(b))

#define HPACK_HUFFMAN_BUFSZ	256
#define HPACK_MAX_TABLE_SIZE	4096

struct hpack_huffman_node {
	struct hpack_huffman_node	*hpn_zero;
	struct hpack_huffman_node	*hpn_one;
	int				 hpn_sym;
};

struct hpack {
	struct hpack_huffman_node	*hpack_huffman;
};

struct hpack_table {
	struct hpack_headerblock	*htb_dynamic;
	long				 htb_dynamic_size;
	long				 htb_dynamic_entries;

	long				 htb_table_size;
	long				 htb_max_table_size;

	struct hpack_headerblock	*htb_headers;
	struct hpack_header		*htb_next;
};

/* Simple internal buffer API */
struct hbuf {
	unsigned char		*data;		/* data pointer */
	size_t			 size;		/* data size */
	size_t			 rpos;		/* read position */
	size_t			 wpos;		/* write position */
	size_t			 wbsz;		/* realloc buf size */
};

/* Masks, flags, and prefixes of the field types */
#define HPACK_M_INDEX			0x80	/* 7-bit prefix */
#define HPACK_F_INDEX			0x80	/* index flag */
#define HPACK_M_LITERAL_INDEX		0xc0	/* 6-bit prefix */
#define HPACK_F_LITERAL_INDEX		0x40	/* literal index flag */
#define HPACK_M_LITERAL_NO_INDEX	0xf0	/* 4-bit prefix */
#define HPACK_F_LITERAL_NO_INDEX	0x00	/* no index flag */
#define HPACK_M_LITERAL_NEVER_INDEX	0xf0	/* 4-bit prefix */
#define HPACK_F_LITERAL_NEVER_INDEX	0x10	/* never index flag */
#define HPACK_M_TABLE_SIZE_UPDATE	0xe0	/* 5-bit prefix */
#define HPACK_F_TABLE_SIZE_UPDATE	0x20	/* dynamic table size flag */
#define HPACK_M_LITERAL			0x80	/* 7-bit index */
#define HPACK_F_LITERAL			0x00	/* literal encoding */
#define HPACK_F_LITERAL_HUFFMAN		0x80	/* huffman encoding */

/*
 * Appendix A.  Static Table Definition
 */
struct hpack_index {
	long			 hpi_id;	/* Index */
	const char		*hpi_name;	/* Header Name */
	const char		*hpi_value;	/* Value */
};
#define HPACK_STATIC_SIZE (sizeof(static_table) / sizeof(static_table[0]))
static struct hpack_index static_table[] = {
	{ 1,	":authority",			NULL },			\
	{ 2,	":method",			"GET" },		\
	{ 3,	":method",			"POST" },		\
	{ 4,	":path",			"/" },			\
	{ 5,	":path",			"/index.html"},		\
	{ 6,	":scheme",			"http" },		\
	{ 7,	":scheme",			"https" },		\
	{ 8,	":status",			"200" },		\
	{ 9,	":status",			"204" },		\
	{ 10,	":status",			"206" },		\
	{ 11,	":status",			"304" },		\
	{ 12,	":status",			"400" },		\
	{ 13,	":status",			"404" },		\
	{ 14,	":status",			"500" },		\
	{ 15,	"accept-charset",		NULL },			\
	{ 16,	"accept-encoding",		"gzip, deflate" },	\
	{ 17,	"accept-language",		NULL },			\
	{ 18,	"accept-ranges",		NULL },			\
	{ 19,	"accept",			NULL },			\
	{ 20,	"access-control-allow-origin",	NULL },			\
	{ 21,	"age",				NULL },			\
	{ 22,	"allow",			NULL },			\
	{ 23,	"authorization",		NULL },			\
	{ 24,	"cache-control",		NULL },			\
	{ 25,	"content-disposition",		NULL },			\
	{ 26,	"content-encoding",		NULL },			\
	{ 27,	"content-language",		NULL },			\
	{ 28,	"content-length",		NULL },			\
	{ 29,	"content-location",		NULL },			\
	{ 30,	"content-range",		NULL },			\
	{ 31,	"content-type",			NULL },			\
	{ 32,	"cookie",			NULL },			\
	{ 33,	"date",				NULL },			\
	{ 34,	"etag",				NULL },			\
	{ 35,	"expect",			NULL },			\
	{ 36,	"expires",			NULL },			\
	{ 37,	"from",				NULL },			\
	{ 38,	"host",				NULL },			\
	{ 39,	"if-match",			NULL },			\
	{ 40,	"if-modified-since",		NULL },			\
	{ 41,	"if-none-match",		NULL },			\
	{ 42,	"if-range",			NULL },			\
	{ 43,	"if-unmodified-since",		NULL },			\
	{ 44,	"last-modified",		NULL },			\
	{ 45,	"link",				NULL },			\
	{ 46,	"location",			NULL },			\
	{ 47,	"max-forwards",			NULL },			\
	{ 48,	"proxy-authenticate",		NULL },			\
	{ 49,	"proxy-authorization",		NULL },			\
	{ 50,	"range",			NULL },			\
	{ 51,	"referer",			NULL },			\
	{ 52,	"refresh",			NULL },			\
	{ 53,	"retry-after",			NULL },			\
	{ 54,	"server",			NULL },			\
	{ 55,	"set-cookie",			NULL },			\
	{ 56,	"strict-transport-security",	NULL },			\
	{ 57,	"transfer-encoding",		NULL },			\
	{ 58,	"user-agent",			NULL },			\
	{ 59,	"vary",				NULL },			\
	{ 60,	"via",				NULL },			\
	{ 61,	"www-authenticate",		NULL },			\
};

/*
 * Appendix B.  Huffman Code
 */
struct hpack_huffman {
	/* (comment) */			/* ASCII symbol */
	/* (comment) */			/* code as bits aligned to MSB */
	unsigned int	hph_code;	/* code as hex aligned to LSB */
	unsigned int	hph_length;	/* len in bits */
};
#define HPACK_HUFFMAN_SIZE (sizeof(huffman_table) / sizeof(huffman_table[0]))
static struct hpack_huffman huffman_table[] = {
	{ /*     (  0) |11111111|11000 */                        0x1ff8, 13 },
	{ /*     (  1) |11111111|11111111|1011000 */           0x7fffd8, 23 },
	{ /*     (  2) |11111111|11111111|11111110|0010 */    0xfffffe2, 28 },
	{ /*     (  3) |11111111|11111111|11111110|0011 */    0xfffffe3, 28 },
	{ /*     (  4) |11111111|11111111|11111110|0100 */    0xfffffe4, 28 },
	{ /*     (  5) |11111111|11111111|11111110|0101 */    0xfffffe5, 28 },
	{ /*     (  6) |11111111|11111111|11111110|0110 */    0xfffffe6, 28 },
	{ /*     (  7) |11111111|11111111|11111110|0111 */    0xfffffe7, 28 },
	{ /*     (  8) |11111111|11111111|11111110|1000 */    0xfffffe8, 28 },
	{ /*     (  9) |11111111|11111111|11101010 */          0xffffea, 24 },
	{ /*     ( 10) |11111111|11111111|11111111|111100 */ 0x3ffffffc, 30 },
	{ /*     ( 11) |11111111|11111111|11111110|1001 */    0xfffffe9, 28 },
	{ /*     ( 12) |11111111|11111111|11111110|1010 */    0xfffffea, 28 },
	{ /*     ( 13) |11111111|11111111|11111111|111101 */ 0x3ffffffd, 30 },
	{ /*     ( 14) |11111111|11111111|11111110|1011 */    0xfffffeb, 28 },
	{ /*     ( 15) |11111111|11111111|11111110|1100 */    0xfffffec, 28 },
	{ /*     ( 16) |11111111|11111111|11111110|1101 */    0xfffffed, 28 },
	{ /*     ( 17) |11111111|11111111|11111110|1110 */    0xfffffee, 28 },
	{ /*     ( 18) |11111111|11111111|11111110|1111 */    0xfffffef, 28 },
	{ /*     ( 19) |11111111|11111111|11111111|0000 */    0xffffff0, 28 },
	{ /*     ( 20) |11111111|11111111|11111111|0001 */    0xffffff1, 28 },
	{ /*     ( 21) |11111111|11111111|11111111|0010 */    0xffffff2, 28 },
	{ /*     ( 22) |11111111|11111111|11111111|111110 */ 0x3ffffffe, 30 },
	{ /*     ( 23) |11111111|11111111|11111111|0011 */    0xffffff3, 28 },
	{ /*     ( 24) |11111111|11111111|11111111|0100 */    0xffffff4, 28 },
	{ /*     ( 25) |11111111|11111111|11111111|0101 */    0xffffff5, 28 },
	{ /*     ( 26) |11111111|11111111|11111111|0110 */    0xffffff6, 28 },
	{ /*     ( 27) |11111111|11111111|11111111|0111 */    0xffffff7, 28 },
	{ /*     ( 28) |11111111|11111111|11111111|1000 */    0xffffff8, 28 },
	{ /*     ( 29) |11111111|11111111|11111111|1001 */    0xffffff9, 28 },
	{ /*     ( 30) |11111111|11111111|11111111|1010 */    0xffffffa, 28 },
	{ /*     ( 31) |11111111|11111111|11111111|1011 */    0xffffffb, 28 },
	{ /* ' ' ( 32) |010100 */                                  0x14,  6 },
	{ /* '!' ( 33) |11111110|00 */                            0x3f8, 10 },
	{ /* '"' ( 34) |11111110|01 */                            0x3f9, 10 },
	{ /* '#' ( 35) |11111111|1010 */                          0xffa, 12 },
	{ /* '$' ( 36) |11111111|11001 */                        0x1ff9, 13 },
	{ /* '%' ( 37) |010101 */                                  0x15,  6 },
	{ /* '&' ( 38) |11111000 */                                0xf8,  8 },
	{ /* ''' ( 39) |11111111|010 */                           0x7fa, 11 },
	{ /* '(' ( 40) |11111110|10 */                            0x3fa, 10 },
	{ /* ')' ( 41) |11111110|11 */                            0x3fb, 10 },
	{ /* '*' ( 42) |11111001 */                                0xf9,  8 },
	{ /* '+' ( 43) |11111111|011 */                           0x7fb, 11 },
	{ /* ',' ( 44) |11111010 */                                0xfa,  8 },
	{ /* '-' ( 45) |010110 */                                  0x16,  6 },
	{ /* '.' ( 46) |010111 */                                  0x17,  6 },
	{ /* '/' ( 47) |011000 */                                  0x18,  6 },
	{ /* '0' ( 48) |00000 */                                    0x0,  5 },
	{ /* '1' ( 49) |00001 */                                    0x1,  5 },
	{ /* '2' ( 50) |00010 */                                    0x2,  5 },
	{ /* '3' ( 51) |011001 */                                  0x19,  6 },
	{ /* '4' ( 52) |011010 */                                  0x1a,  6 },
	{ /* '5' ( 53) |011011 */                                  0x1b,  6 },
	{ /* '6' ( 54) |011100 */                                  0x1c,  6 },
	{ /* '7' ( 55) |011101 */                                  0x1d,  6 },
	{ /* '8' ( 56) |011110 */                                  0x1e,  6 },
	{ /* '9' ( 57) |011111 */                                  0x1f,  6 },
	{ /* ':' ( 58) |1011100 */                                 0x5c,  7 },
	{ /* ';' ( 59) |11111011 */                                0xfb,  8 },
	{ /* '<' ( 60) |11111111|1111100 */                      0x7ffc, 15 },
	{ /* '=' ( 61) |100000 */                                  0x20,  6 },
	{ /* '>' ( 62) |11111111|1011 */                          0xffb, 12 },
	{ /* '?' ( 63) |11111111|00 */                            0x3fc, 10 },
	{ /* '@' ( 64) |11111111|11010 */                        0x1ffa, 13 },
	{ /* 'A' ( 65) |100001 */                                  0x21,  6 },
	{ /* 'B' ( 66) |1011101 */                                 0x5d,  7 },
	{ /* 'C' ( 67) |1011110 */                                 0x5e,  7 },
	{ /* 'D' ( 68) |1011111 */                                 0x5f,  7 },
	{ /* 'E' ( 69) |1100000 */                                 0x60,  7 },
	{ /* 'F' ( 70) |1100001 */                                 0x61,  7 },
	{ /* 'G' ( 71) |1100010 */                                 0x62,  7 },
	{ /* 'H' ( 72) |1100011 */                                 0x63,  7 },
	{ /* 'I' ( 73) |1100100 */                                 0x64,  7 },
	{ /* 'J' ( 74) |1100101 */                                 0x65,  7 },
	{ /* 'K' ( 75) |1100110 */                                 0x66,  7 },
	{ /* 'L' ( 76) |1100111 */                                 0x67,  7 },
	{ /* 'M' ( 77) |1101000 */                                 0x68,  7 },
	{ /* 'N' ( 78) |1101001 */                                 0x69,  7 },
	{ /* 'O' ( 79) |1101010 */                                 0x6a,  7 },
	{ /* 'P' ( 80) |1101011 */                                 0x6b,  7 },
	{ /* 'Q' ( 81) |1101100 */                                 0x6c,  7 },
	{ /* 'R' ( 82) |1101101 */                                 0x6d,  7 },
	{ /* 'S' ( 83) |1101110 */                                 0x6e,  7 },
	{ /* 'T' ( 84) |1101111 */                                 0x6f,  7 },
	{ /* 'U' ( 85) |1110000 */                                 0x70,  7 },
	{ /* 'V' ( 86) |1110001 */                                 0x71,  7 },
	{ /* 'W' ( 87) |1110010 */                                 0x72,  7 },
	{ /* 'X' ( 88) |11111100 */                                0xfc,  8 },
	{ /* 'Y' ( 89) |1110011 */                                 0x73,  7 },
	{ /* 'Z' ( 90) |11111101 */                                0xfd,  8 },
	{ /* '[' ( 91) |11111111|11011 */                        0x1ffb, 13 },
	{ /* '\' ( 92) |11111111|11111110|000 */                0x7fff0, 19 },
	{ /* '}' ( 93) |11111111|11100 */                        0x1ffc, 13 },
	{ /* '^' ( 94) |11111111|111100 */                       0x3ffc, 14 },
	{ /* '_' ( 95) |100010 */                                  0x22,  6 },
	{ /* '`' ( 96) |11111111|1111101 */                      0x7ffd, 15 },
	{ /* 'a' ( 97) |00011 */                                    0x3,  5 },
	{ /* 'b' ( 98) |100011 */                                  0x23,  6 },
	{ /* 'c' ( 99) |00100 */                                    0x4,  5 },
	{ /* 'd' (100) |100100 */                                  0x24,  6 },
	{ /* 'e' (101) |00101 */                                    0x5,  5 },
	{ /* 'f' (102) |100101 */                                  0x25,  6 },
	{ /* 'g' (103) |100110 */                                  0x26,  6 },
	{ /* 'h' (104) |100111 */                                  0x27,  6 },
	{ /* 'i' (105) |00110 */                                    0x6,  5 },
	{ /* 'j' (106) |1110100 */                                 0x74,  7 },
	{ /* 'k' (107) |1110101 */                                 0x75,  7 },
	{ /* 'l' (108) |101000 */                                  0x28,  6 },
	{ /* 'm' (109) |101001 */                                  0x29,  6 },
	{ /* 'n' (110) |101010 */                                  0x2a,  6 },
	{ /* 'o' (111) |00111 */                                    0x7,  5 },
	{ /* 'p' (112) |101011 */                                  0x2b,  6 },
	{ /* 'q' (113) |1110110 */                                 0x76,  7 },
	{ /* 'r' (114) |101100 */                                  0x2c,  6 },
	{ /* 's' (115) |01000 */                                    0x8,  5 },
	{ /* 't' (116) |01001 */                                    0x9,  5 },
	{ /* 'u' (117) |101101 */                                  0x2d,  6 },
	{ /* 'v' (118) |1110111 */                                 0x77,  7 },
	{ /* 'w' (119) |1111000 */                                 0x78,  7 },
	{ /* 'x' (120) |1111001 */                                 0x79,  7 },
	{ /* 'y' (121) |1111010 */                                 0x7a,  7 },
	{ /* 'z' (122) |1111011 */                                 0x7b,  7 },
	{ /* '{' (123) |11111111|1111110 */                      0x7ffe, 15 },
	{ /* '|' (124) |11111111|100 */                           0x7fc, 11 },
	{ /* '}' (125) |11111111|111101 */                       0x3ffd, 14 },
	{ /* '~' (126) |11111111|11101 */                        0x1ffd, 13 },
	{ /*     (127) |11111111|11111111|11111111|1100 */    0xffffffc, 28 },
	{ /*     (128) |11111111|11111110|0110 */               0xfffe6, 20 },
	{ /*     (129) |11111111|11111111|010010 */            0x3fffd2, 22 },
	{ /*     (130) |11111111|11111110|0111 */               0xfffe7, 20 },
	{ /*     (131) |11111111|11111110|1000 */               0xfffe8, 20 },
	{ /*     (132) |11111111|11111111|010011 */            0x3fffd3, 22 },
	{ /*     (133) |11111111|11111111|010100 */            0x3fffd4, 22 },
	{ /*     (134) |11111111|11111111|010101 */            0x3fffd5, 22 },
	{ /*     (135) |11111111|11111111|1011001 */           0x7fffd9, 23 },
	{ /*     (136) |11111111|11111111|010110 */            0x3fffd6, 22 },
	{ /*     (137) |11111111|11111111|1011010 */           0x7fffda, 23 },
	{ /*     (138) |11111111|11111111|1011011 */           0x7fffdb, 23 },
	{ /*     (139) |11111111|11111111|1011100 */           0x7fffdc, 23 },
	{ /*     (140) |11111111|11111111|1011101 */           0x7fffdd, 23 },
	{ /*     (141) |11111111|11111111|1011110 */           0x7fffde, 23 },
	{ /*     (142) |11111111|11111111|11101011 */          0xffffeb, 24 },
	{ /*     (143) |11111111|11111111|1011111 */           0x7fffdf, 23 },
	{ /*     (144) |11111111|11111111|11101100 */          0xffffec, 24 },
	{ /*     (145) |11111111|11111111|11101101 */          0xffffed, 24 },
	{ /*     (146) |11111111|11111111|010111 */            0x3fffd7, 22 },
	{ /*     (147) |11111111|11111111|1100000 */           0x7fffe0, 23 },
	{ /*     (148) |11111111|11111111|11101110 */          0xffffee, 24 },
	{ /*     (149) |11111111|11111111|1100001 */           0x7fffe1, 23 },
	{ /*     (150) |11111111|11111111|1100010 */           0x7fffe2, 23 },
	{ /*     (151) |11111111|11111111|1100011 */           0x7fffe3, 23 },
	{ /*     (152) |11111111|11111111|1100100 */           0x7fffe4, 23 },
	{ /*     (153) |11111111|11111110|11100 */             0x1fffdc, 21 },
	{ /*     (154) |11111111|11111111|011000 */            0x3fffd8, 22 },
	{ /*     (155) |11111111|11111111|1100101 */           0x7fffe5, 23 },
	{ /*     (156) |11111111|11111111|011001 */            0x3fffd9, 22 },
	{ /*     (157) |11111111|11111111|1100110 */           0x7fffe6, 23 },
	{ /*     (158) |11111111|11111111|1100111 */           0x7fffe7, 23 },
	{ /*     (159) |11111111|11111111|11101111 */          0xffffef, 24 },
	{ /*     (160) |11111111|11111111|011010 */            0x3fffda, 22 },
	{ /*     (161) |11111111|11111110|11101 */             0x1fffdd, 21 },
	{ /*     (162) |11111111|11111110|1001 */               0xfffe9, 20 },
	{ /*     (163) |11111111|11111111|011011 */            0x3fffdb, 22 },
	{ /*     (164) |11111111|11111111|011100 */            0x3fffdc, 22 },
	{ /*     (165) |11111111|11111111|1101000 */           0x7fffe8, 23 },
	{ /*     (166) |11111111|11111111|1101001 */           0x7fffe9, 23 },
	{ /*     (167) |11111111|11111110|11110 */             0x1fffde, 21 },
	{ /*     (168) |11111111|11111111|1101010 */           0x7fffea, 23 },
	{ /*     (169) |11111111|11111111|011101 */            0x3fffdd, 22 },
	{ /*     (170) |11111111|11111111|011110 */            0x3fffde, 22 },
	{ /*     (171) |11111111|11111111|11110000 */          0xfffff0, 24 },
	{ /*     (172) |11111111|11111110|11111 */             0x1fffdf, 21 },
	{ /*     (173) |11111111|11111111|011111 */            0x3fffdf, 22 },
	{ /*     (174) |11111111|11111111|1101011 */           0x7fffeb, 23 },
	{ /*     (175) |11111111|11111111|1101100 */           0x7fffec, 23 },
	{ /*     (176) |11111111|11111111|00000 */             0x1fffe0, 21 },
	{ /*     (177) |11111111|11111111|00001 */             0x1fffe1, 21 },
	{ /*     (178) |11111111|11111111|100000 */            0x3fffe0, 22 },
	{ /*     (179) |11111111|11111111|00010 */             0x1fffe2, 21 },
	{ /*     (180) |11111111|11111111|1101101 */           0x7fffed, 23 },
	{ /*     (181) |11111111|11111111|100001 */            0x3fffe1, 22 },
	{ /*     (182) |11111111|11111111|1101110 */           0x7fffee, 23 },
	{ /*     (183) |11111111|11111111|1101111 */           0x7fffef, 23 },
	{ /*     (184) |11111111|11111110|1010 */               0xfffea, 20 },
	{ /*     (185) |11111111|11111111|100010 */            0x3fffe2, 22 },
	{ /*     (186) |11111111|11111111|100011 */            0x3fffe3, 22 },
	{ /*     (187) |11111111|11111111|100100 */            0x3fffe4, 22 },
	{ /*     (188) |11111111|11111111|1110000 */           0x7ffff0, 23 },
	{ /*     (189) |11111111|11111111|100101 */            0x3fffe5, 22 },
	{ /*     (190) |11111111|11111111|100110 */            0x3fffe6, 22 },
	{ /*     (191) |11111111|11111111|1110001 */           0x7ffff1, 23 },
	{ /*     (192) |11111111|11111111|11111000|00 */      0x3ffffe0, 26 },
	{ /*     (193) |11111111|11111111|11111000|01 */      0x3ffffe1, 26 },
	{ /*     (194) |11111111|11111110|1011 */               0xfffeb, 20 },
	{ /*     (195) |11111111|11111110|001 */                0x7fff1, 19 },
	{ /*     (196) |11111111|11111111|100111 */            0x3fffe7, 22 },
	{ /*     (197) |11111111|11111111|1110010 */           0x7ffff2, 23 },
	{ /*     (198) |11111111|11111111|101000 */            0x3fffe8, 22 },
	{ /*     (199) |11111111|11111111|11110110|0 */       0x1ffffec, 25 },
	{ /*     (200) |11111111|11111111|11111000|10 */      0x3ffffe2, 26 },
	{ /*     (201) |11111111|11111111|11111000|11 */      0x3ffffe3, 26 },
	{ /*     (202) |11111111|11111111|11111001|00 */      0x3ffffe4, 26 },
	{ /*     (203) |11111111|11111111|11111011|110 */     0x7ffffde, 27 },
	{ /*     (204) |11111111|11111111|11111011|111 */     0x7ffffdf, 27 },
	{ /*     (205) |11111111|11111111|11111001|01 */      0x3ffffe5, 26 },
	{ /*     (206) |11111111|11111111|11110001 */          0xfffff1, 24 },
	{ /*     (207) |11111111|11111111|11110110|1 */       0x1ffffed, 25 },
	{ /*     (208) |11111111|11111110|010 */                0x7fff2, 19 },
	{ /*     (209) |11111111|11111111|00011 */             0x1fffe3, 21 },
	{ /*     (210) |11111111|11111111|11111001|10 */      0x3ffffe6, 26 },
	{ /*     (211) |11111111|11111111|11111100|000 */     0x7ffffe0, 27 },
	{ /*     (212) |11111111|11111111|11111100|001 */     0x7ffffe1, 27 },
	{ /*     (213) |11111111|11111111|11111001|11 */      0x3ffffe7, 26 },
	{ /*     (214) |11111111|11111111|11111100|010 */     0x7ffffe2, 27 },
	{ /*     (215) |11111111|11111111|11110010 */          0xfffff2, 24 },
	{ /*     (216) |11111111|11111111|00100 */             0x1fffe4, 21 },
	{ /*     (217) |11111111|11111111|00101 */             0x1fffe5, 21 },
	{ /*     (218) |11111111|11111111|11111010|00 */      0x3ffffe8, 26 },
	{ /*     (219) |11111111|11111111|11111010|01 */      0x3ffffe9, 26 },
	{ /*     (220) |11111111|11111111|11111111|1101 */    0xffffffd, 28 },
	{ /*     (221) |11111111|11111111|11111100|011 */     0x7ffffe3, 27 },
	{ /*     (222) |11111111|11111111|11111100|100 */     0x7ffffe4, 27 },
	{ /*     (223) |11111111|11111111|11111100|101 */     0x7ffffe5, 27 },
	{ /*     (224) |11111111|11111110|1100 */               0xfffec, 20 },
	{ /*     (225) |11111111|11111111|11110011 */          0xfffff3, 24 },
	{ /*     (226) |11111111|11111110|1101 */               0xfffed, 20 },
	{ /*     (227) |11111111|11111111|00110 */             0x1fffe6, 21 },
	{ /*     (228) |11111111|11111111|101001 */            0x3fffe9, 22 },
	{ /*     (229) |11111111|11111111|00111 */             0x1fffe7, 21 },
	{ /*     (230) |11111111|11111111|01000 */             0x1fffe8, 21 },
	{ /*     (231) |11111111|11111111|1110011 */           0x7ffff3, 23 },
	{ /*     (232) |11111111|11111111|101010 */            0x3fffea, 22 },
	{ /*     (233) |11111111|11111111|101011 */            0x3fffeb, 22 },
	{ /*     (234) |11111111|11111111|11110111|0 */       0x1ffffee, 25 },
	{ /*     (235) |11111111|11111111|11110111|1 */       0x1ffffef, 25 },
	{ /*     (236) |11111111|11111111|11110100 */          0xfffff4, 24 },
	{ /*     (237) |11111111|11111111|11110101 */          0xfffff5, 24 },
	{ /*     (238) |11111111|11111111|11111010|10 */      0x3ffffea, 26 },
	{ /*     (239) |11111111|11111111|1110100 */           0x7ffff4, 23 },
	{ /*     (240) |11111111|11111111|11111010|11 */      0x3ffffeb, 26 },
	{ /*     (241) |11111111|11111111|11111100|110 */     0x7ffffe6, 27 },
	{ /*     (242) |11111111|11111111|11111011|00 */      0x3ffffec, 26 },
	{ /*     (243) |11111111|11111111|11111011|01 */      0x3ffffed, 26 },
	{ /*     (244) |11111111|11111111|11111100|111 */     0x7ffffe7, 27 },
	{ /*     (245) |11111111|11111111|11111101|000 */     0x7ffffe8, 27 },
	{ /*     (246) |11111111|11111111|11111101|001 */     0x7ffffe9, 27 },
	{ /*     (247) |11111111|11111111|11111101|010 */     0x7ffffea, 27 },
	{ /*     (248) |11111111|11111111|11111101|011 */     0x7ffffeb, 27 },
	{ /*     (249) |11111111|11111111|11111111|1110 */    0xffffffe, 28 },
	{ /*     (250) |11111111|11111111|11111101|100 */     0x7ffffec, 27 },
	{ /*     (251) |11111111|11111111|11111101|101 */     0x7ffffed, 27 },
	{ /*     (252) |11111111|11111111|11111101|110 */     0x7ffffee, 27 },
	{ /*     (253) |11111111|11111111|11111101|111 */     0x7ffffef, 27 },
	{ /*     (254) |11111111|11111111|11111110|000 */     0x7fffff0, 27 },
	{ /*     (255) |11111111|11111111|11111011|10 */      0x3ffffee, 26 },
	{ /* EOS (256) |11111111|11111111|11111111|111111 */ 0x3fffffff, 30 },
};

#endif /* HPACK_INTERNAL */
#endif /* HPACK_H */
