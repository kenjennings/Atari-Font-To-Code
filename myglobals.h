/* Dump font data as program text either for BASIC or assembly
 */

/* $Id: myglobals.h,v 1.3 2014/03/31 20:53:36 kjenning Exp $
 *
 */

#ifndef _MY_GLOBALS_INCL_
#define _MY_GLOBALS_INCL_

#include "globals.h"

/* Program globals...  mostly from command line. 
 */
extern int  gDoLines;      /* include line numbers or not. 1 == lines */
extern int  gLine;         /* start at line number */
extern int  gIncr;         /* increment line numbers */
extern int  gDoAsmOrBasiC; /* 0 == asm, 1 == BASIC, 2 == C */
extern int  gAsmOrg;       /* Starting assembly address.  default (0x9000). */
extern int  gDoHexOrDec;   /* 0 == hex, 1 == dec */
extern int  gDoBDW;        /* 0 == byte, 1 == dbyte, 2 == word */
extern int  gDoIndex;      /* 1 == include character index */
extern int  gDoDiff;       /* 1 == output only the characters that differ from ROM */

extern int  gROM;          /* 0 = $E000. 2 == $E200. 1 and 3 possible, but not so useful */
extern int  gPages[];      /* which 256 byte pages of input font to output */
extern int  gNumPages;     /* number of pages to use from gPages[] */

extern int  gPageComment;  /* Comment at the start of each page? 0 == off. 1 == on. */
extern int  gCharComment;  /* Comment labeling each character? 0 == off. 1 == on. */
extern int  gBitsComment;  /* Comment block for the bitmap? 0 == off. 1 == on */

extern int  gGlyphType;    /* Bitmap interpretation.  2 (2 color) or 4  (4 color ) */
extern int  gPixelSize;    /* Bitmap Field width (chars output) PER Bit (2) */
extern int  gPixelFill;    /* Number of times to output gPixel inside of PixelSize */

extern char gPixels[4];    /* Char to represent bits or pairs 0, 1, 2, 3 */

extern char * gFileName;   /* Required (not really) first argument */
extern int    gFileSize;   /* stat must show 512 or 1024. */

extern char * gStdin;      /* fallback if no filename provided */

extern unsigned char gCset[1024];  /* The character set read from the file */



char * myglobal_rcs_id( void );

int dumpMyArgs( void ); /* Debugging.  Output the Globals. */

int sanityCheck( void ); /* Verify choices are compatible.  All values are global */

GlobalArg ** get_GA( void ); /* Return the Global Args, so it can be passed to doArgs */

#endif /* _MY_GLOBALS_INCL_ */
