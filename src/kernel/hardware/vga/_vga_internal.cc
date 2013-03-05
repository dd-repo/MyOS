#include <string.h> 
#include <stdio.h> 

#define FP_SEG(x) ((x & 0xFFFF0000) >> 16)
#define FP_OFF(x) (x & 0xFFFF)

#include <hardware/io.h>

#define pokeb(S,O,V)        *(uint8_t *)(16uL * (S) + (O)) = (V)
#define pokew(S,O,V)        *(uint16_t *)(16uL * (S) + (O)) = (V)
#define _vmemwr(DS,DO,S,N)  memcpy((char *)((uint64_t)(DS) * 16 + (DO)), S, N)

#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_AC_READ     0x3C1
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9
#define VGA_MISC_READ       0x3CC
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF

#define VGA_CRTC_INDEX      0x3D4       /* 0x3B4 */
#define VGA_CRTC_DATA       0x3D5       /* 0x3B5 */
#define VGA_INSTAT_READ     0x3DA

#define VGA_NUM_SEQ_REGS    5
#define VGA_NUM_CRTC_REGS   25
#define VGA_NUM_GC_REGS     9
#define VGA_NUM_AC_REGS     21
#define VGA_NUM_REGS        (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)


static uint8_t g_90x60_text[] =
{
/* MISC */
    0xE7,
/* SEQ */
    0x03, 0x01, 0x03, 0x00, 0x02,
/* CRTC */
    0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
    0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3,
    0xFF,
/* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
/* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

/*****************************************************************************
8X8 AND 8X16 FONTS
*****************************************************************************/
static uint8_t g_8x8_font[2048] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,
    0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,
    0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x92, 0x10, 0x7C,
    0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,
    0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,
    0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,
    0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,
    0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,
    0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
    0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,
    0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,
    0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,
    0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,
    0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,
    0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00, 
    0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00, 
    0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0x86, 0xFC, 
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 
    0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,
    0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00, 
    0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00, 
    0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00, 
    0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00, 
    0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00, 
    0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
    0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 
    0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00, 
    0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00,
    0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00,
    0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 
    0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
    0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 
    0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
    0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
    0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00,
    0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00, 
    0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00, 
    0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00, 
    0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00, 
    0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00, 
    0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00, 
    0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00,
    0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
    0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00, 
    0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00, 
    0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 
    0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30, 
    0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 
    0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 
    0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
    0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00, 
    0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00, 
    0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00, 
    0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00, 
    0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00, 
    0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00, 
    0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00, 
    0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00,
    0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00, 
    0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00, 
    0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00, 
    0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00, 
    0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00, 
    0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
    0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00, 
    0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00, 
    0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00, 
    0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00, 
    0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00,
    0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00, 
    0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00, 
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
    0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00, 
    0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00, 
    0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00,
    0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,
    0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
    0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
    0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 
    0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
    0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 
    0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00, 
    0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00,
    0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00, 
    0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 
    0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00, 
    0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 
    0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
    0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00, 
    0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 
    0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 
    0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0, 
    0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E, 
    0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00, 
    0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00,
    0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00, 
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00, 
    0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00, 
    0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 
    0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
    0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00, 
    0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 
    0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00, 
    0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00,
    0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x0C, 0x06, 0x7C, 
    0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x1C, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 
    0x7E, 0x81, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00,
    0xCC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 
    0xE0, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 
    0x30, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0xC0, 0x78, 0x0C, 0x38,
    0x7E, 0x81, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00,
    0xCC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0xE0, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 
    0xCC, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0x7C, 0x82, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00, 
    0xE0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0xC6, 0x10, 0x7C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00, 
    0x30, 0x30, 0x00, 0x78, 0xCC, 0xFC, 0xCC, 0x00, 
    0x1C, 0x00, 0xFC, 0x60, 0x78, 0x60, 0xFC, 0x00,
    0x00, 0x00, 0x7F, 0x0C, 0x7F, 0xCC, 0x7F, 0x00,
    0x3E, 0x6C, 0xCC, 0xFE, 0xCC, 0xCC, 0xCE, 0x00, 
    0x78, 0x84, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 
    0x00, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 
    0x00, 0xE0, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 
    0x78, 0x84, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
    0x00, 0xE0, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
    0x00, 0xCC, 0x00, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
    0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00, 
    0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 
    0x18, 0x18, 0x7E, 0xC0, 0xC0, 0x7E, 0x18, 0x18, 
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0xE6, 0xFC, 0x00, 
    0xCC, 0xCC, 0x78, 0x30, 0xFC, 0x30, 0xFC, 0x30, 
    0xF8, 0xCC, 0xCC, 0xFA, 0xC6, 0xCF, 0xC6, 0xC3, 
    0x0E, 0x1B, 0x18, 0x3C, 0x18, 0x18, 0xD8, 0x70, 
    0x1C, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
    0x00, 0x1C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 
    0x00, 0x1C, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
    0x00, 0xF8, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0x00, 
    0xFC, 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00, 
    0x3C, 0x6C, 0x6C, 0x3E, 0x00, 0x7E, 0x00, 0x00, 
    0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C, 0x00, 0x00,
    0x18, 0x00, 0x18, 0x18, 0x30, 0x66, 0x3C, 0x00, 
    0x00, 0x00, 0x00, 0xFC, 0xC0, 0xC0, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xFC, 0x0C, 0x0C, 0x00, 0x00, 
    0xC6, 0xCC, 0xD8, 0x36, 0x6B, 0xC2, 0x84, 0x0F, 
    0xC3, 0xC6, 0xCC, 0xDB, 0x37, 0x6D, 0xCF, 0x03,
    0x18, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x18, 0x00, 
    0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00,
    0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00, 
    0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 
    0xDB, 0xF6, 0xDB, 0x6F, 0xDB, 0x7E, 0xD7, 0xED, 
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0xF8, 0x18, 0x18, 0x18,
    0x18, 0x18, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18,
    0x36, 0x36, 0x36, 0x36, 0xF6, 0x36, 0x36, 0x36,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0x36, 0x36, 0x36, 
    0x00, 0x00, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18, 
    0x36, 0x36, 0xF6, 0x06, 0xF6, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
    0x00, 0x00, 0xFE, 0x06, 0xF6, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0xF6, 0x06, 0xFE, 0x00, 0x00, 0x00, 
    0x36, 0x36, 0x36, 0x36, 0xFE, 0x00, 0x00, 0x00,
    0x18, 0x18, 0xF8, 0x18, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0x18, 0x18, 0x18, 
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, 0x00, 0x00, 
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x18, 0x18, 0x18, 
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x18, 0x18, 0x18, 
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 
    0x36, 0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0x37, 0x30, 0x3F, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x3F, 0x30, 0x37, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0xF7, 0x00, 0xFF, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xFF, 0x00, 0xF7, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x36, 0x36, 0xF7, 0x00, 0xF7, 0x36, 0x36, 0x36, 
    0x18, 0x18, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 
    0x36, 0x36, 0x36, 0x36, 0xFF, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x18, 0x18, 0x18, 
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0x36, 0x36, 0x3F, 0x00, 0x00, 0x00, 
    0x18, 0x18, 0x1F, 0x18, 0x1F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x36, 0x36, 0x36, 
    0x36, 0x36, 0x36, 0x36, 0xFF, 0x36, 0x36, 0x36,
    0x18, 0x18, 0xFF, 0x18, 0xFF, 0x18, 0x18, 0x18, 
    0x18, 0x18, 0x18, 0x18, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x18, 0x18, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x76, 0xDC, 0xC8, 0xDC, 0x76, 0x00,
    0x00, 0x78, 0xCC, 0xF8, 0xCC, 0xF8, 0xC0, 0xC0,
    0x00, 0xFC, 0xCC, 0xC0, 0xC0, 0xC0, 0xC0, 0x00,
    0x00, 0x00, 0xFE, 0x6C, 0x6C, 0x6C, 0x6C, 0x00,
    0xFC, 0xCC, 0x60, 0x30, 0x60, 0xCC, 0xFC, 0x00,
    0x00, 0x00, 0x7E, 0xD8, 0xD8, 0xD8, 0x70, 0x00,
    0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x60, 0xC0,
    0x00, 0x76, 0xDC, 0x18, 0x18, 0x18, 0x18, 0x00,
    0xFC, 0x30, 0x78, 0xCC, 0xCC, 0x78, 0x30, 0xFC,
    0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x6C, 0x38, 0x00,
    0x38, 0x6C, 0xC6, 0xC6, 0x6C, 0x6C, 0xEE, 0x00,
    0x1C, 0x30, 0x18, 0x7C, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0x00, 0x7E, 0xDB, 0xDB, 0x7E, 0x00, 0x00,
    0x06, 0x0C, 0x7E, 0xDB, 0xDB, 0x7E, 0x60, 0xC0,
    0x38, 0x60, 0xC0, 0xF8, 0xC0, 0x60, 0x38, 0x00,
    0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
    0x00, 0x7E, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00,
    0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x7E, 0x00,
    0x60, 0x30, 0x18, 0x30, 0x60, 0x00, 0xFC, 0x00,
    0x18, 0x30, 0x60, 0x30, 0x18, 0x00, 0xFC, 0x00,
    0x0E, 0x1B, 0x1B, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0xD8, 0xD8, 0x70,
    0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x76, 0xDC, 0x00, 0x76, 0xDC, 0x00, 0x00,
    0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x0F, 0x0C, 0x0C, 0x0C, 0xEC, 0x6C, 0x3C, 0x1C,
    0x58, 0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00,
    0x70, 0x98, 0x30, 0x60, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*****************************************************************************
*****************************************************************************/
static void write_regs(uint8_t *regs)
{
    uint32_t i;

/* write MISCELLANEOUS reg */
    outb(VGA_MISC_WRITE, *regs);
    regs++;
/* write SEQUENCER regs */
    for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
    {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs);
        regs++;
    }
/* unlock CRTC registers */
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;
/* write CRTC regs */
    for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
    {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs);
        regs++;
    }
/* write GRAPHICS CONTROLLER regs */
    for(i = 0; i < VGA_NUM_GC_REGS; i++)
    {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs);
        regs++;
    }
/* write ATTRIBUTE CONTROLLER regs */
    for(i = 0; i < VGA_NUM_AC_REGS; i++)
    {
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs);
        regs++;
    }
/* lock 16-color palette and unblank display */
    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}
/*****************************************************************************
*****************************************************************************/
static void set_plane(uint32_t p)
{
    uint8_t pmask;

    p &= 3;
    pmask = 1 << p;
/* set read plane */
    outb(VGA_GC_INDEX, 4);
    outb(VGA_GC_DATA, p);
/* set write plane */
    outb(VGA_SEQ_INDEX, 2);
    outb(VGA_SEQ_DATA, pmask);
}
/*****************************************************************************
VGA framebuffer is at A000:0000, B000:0000, or B800:0000
depending on bits in GC 6
*****************************************************************************/
static uint32_t get_fb_seg(void)
{
    uint32_t seg;

    outb(VGA_GC_INDEX, 6);
    seg = inb(VGA_GC_DATA);
    seg >>= 2;
    seg &= 3;
    switch(seg)
    {
        case 0:
        case 1:
        seg = 0xA000;
        break;
        case 2:
        seg = 0xB000;
        break;
        case 3:
        seg = 0xB800;
        break;
    }
    return seg;
}
/*****************************************************************************
*****************************************************************************/
static void vmemwr(uint32_t dst_off, uint8_t *src, uint32_t count)
{
    _vmemwr(get_fb_seg(), dst_off, src, count);
}


/*****************************************************************************
write font to plane P4 (assuming planes are named P1, P2, P4, P8)
*****************************************************************************/
static void write_font(uint8_t *buf, uint32_t font_height)
{
    uint8_t seq2, seq4, gc4, gc5, gc6;
    uint32_t i;

/* save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
    outb(VGA_SEQ_INDEX, 2);
    seq2 = inb(VGA_SEQ_DATA);

    outb(VGA_SEQ_INDEX, 4);
    seq4 = inb(VGA_SEQ_DATA);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
    outb(VGA_SEQ_DATA, seq4 | 0x04);

    outb(VGA_GC_INDEX, 4);
    gc4 = inb(VGA_GC_DATA);

    outb(VGA_GC_INDEX, 5);
    gc5 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
    outb(VGA_GC_DATA, gc5 & ~0x10);

    outb(VGA_GC_INDEX, 6);
    gc6 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
    outb(VGA_GC_DATA, gc6 & ~0x02);
/* write font to plane P4 */
    set_plane(2);
/* write font 0 */
    for(i = 0; i < 256; i++)
    {
        vmemwr(16384u * 0 + i * 32, buf, font_height);
        buf += font_height;
    }
/* restore registers */
    outb(VGA_SEQ_INDEX, 2);
    outb(VGA_SEQ_DATA, seq2);
    outb(VGA_SEQ_INDEX, 4);
    outb(VGA_SEQ_DATA, seq4);
    outb(VGA_GC_INDEX, 4);
    outb(VGA_GC_DATA, gc4);
    outb(VGA_GC_INDEX, 5);
    outb(VGA_GC_DATA, gc5);
    outb(VGA_GC_INDEX, 6);
    outb(VGA_GC_DATA, gc6);
}


void _vga_internal_set_text_mode()
{
    uint32_t rows, cols, ht, i;

    write_regs(g_90x60_text);
    cols = 90;
    rows = 60;
    ht = 8;

    write_font(g_8x8_font, 8);

/* tell the BIOS what we've done, so BIOS text output works OK */
    pokew(0x40, 0x4A, cols);    /* columns on screen */
    pokew(0x40, 0x4C, cols * rows * 2); /* framebuffer size */
    pokew(0x40, 0x50, 0);       /* cursor pos'n */
    pokeb(0x40, 0x60, ht - 1);  /* cursor shape */
    pokeb(0x40, 0x61, ht - 2);
    pokeb(0x40, 0x84, rows - 1);    /* rows on screen - 1 */
    pokeb(0x40, 0x85, ht);      /* char height */
/* set white-on-black attributes for all text */
    for(i = 0; i < cols * rows; i++)
        pokeb(0xB800, i * 2 + 1, 7);
}