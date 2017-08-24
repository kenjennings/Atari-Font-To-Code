/* Dump font data as program text either for BASIC or assembly
 */

/* $Id: myglobals.c,v 1.6 2014/04/04 17:00:07 kjenning Exp $
 *
 */

#include "globals.h"


#define CKP( STR ) fprintf(stderr, "Checkpoint in %s at %d: ",__FILE__,__LINE__); fprintf(stderr, STR); fputc( '\n', stderr ); fflush(NULL);


static char * rcs_id = "$Id: myglobals.c,v 1.6 2014/04/04 17:00:07 kjenning Exp $";

char * myglobal_rcs_id( void ) /* report file version for myglobals.o */
{
	return rcs_id;
}


/* Program globals...  mostly from command line. 
 */
int gDoDebug      = 0;      /* Turn on debug to stderr. 0 == off.  1 == on*/
int gDoLines      = 1;      /* include line numbers or not. 1 == lines */
int gLine         = 1000;   /* start at line number */
int gIncr         = 10;     /* increment line numbers */
int gDoAsmOrBasiC = 0;      /* 0 == asm, 1 == BASIC, 2 == C */
int gAsmOrg       = 0x9000; /* Starting assembly address.  default ($9000). */
int gDoHexOrDec   = 0;      /* 0 == hex, 1 == dec */
int gDoBDW        = 0;      /* 0 == byte, 1 == dbyte, 2 == word */
int gDoIndex      = 0;      /* 1 == include character index */
int gDoDiff       = 0;      /* 1 == output only the characters that differ from ROM */
int gROM          = 0;      /* 0 = $E000. 2 == $E200. 1 and 3 possible, but not so useful */
int gPages[]      = { 0, 1, 2, 3 }; /* which 256 byte pages of input font to output */
int gNumPages     = 0;      /* number of pages to use from gPages[] */
int gPageComment  = 1;      /* Comment at the start of each page? 0 == off. 1 == on. */
int gCharComment  = 0;      /* Comment labeling each character? 0 == off. 1 == on. */
int gBitsComment  = 0;      /* Comment block for the bitmap? 0 == off. 1 == on */
int gGlyphType    = 2;      /* Bitmap interpretation.  2 (2 color) or 4  (4 color ) */
int gPixelSize    = 2;      /* Bitmap Field width (chars output) PER Bit (2) */
int gPixelFill    = 1;      /* Number of times to output gPixel inside of PixelSize */

char gPixels[4]   = { '.', '#', 'X', 'O' }; /* Chars to represent bits or pairs 0, 1, 2, 3 */

char * gFileName  = NULL; /* Required (but not really) first argument */
int    gFileSize  = 0;    /* stat must show 512 or 1024. */

char * gStdin = "stdin";  /* fallback if no filename provided */

unsigned char gCset[1024];  /* The character set read from the file */



static int fDoPages( GlobalArg * arg,    /* This matching arg */
					 char      * name,   /* The actual option text matched */
					 char      * value );/* The actual argument */
		 

/* Tedium.  Documentation arrays must be declared. */
static char * gdFile[] = { "filename",
						   "Providing filename 'stdin' (default) will use stdio instead.",
						   NULL };
static char * gdLines[] = { "",
						    "Include line numbers. The C format always overrides this to turn off line numbers. (default for BASIC and ASM).",
						    NULL };
static char * gdNoLines[] = { "",
						      "Do not include line numbers. (default for C)",
						      NULL };	
static char * gdStart[] = { "num",
						    "Starting line number less than 32000 (default 1000).",
						    NULL };	
static char * gdInc[] = { "num",
						  "Line number increment. (default 10).",
						  NULL };	
static char * gdAsm[] = { "",
						  "Output for Assembly. (default)",
						  NULL };	
static char * gdBasic[] = { "",
						    "Output for BASIC. (DATA format only)",
						    NULL };							
static char * gdCC[] = { "",
					     "Output for C. (unsigned char array)",
					     NULL };	
static char * gdAsmOrg[] = { "num",
							 "Starting address for Assembly output. (default $9000)",
							 NULL };								
static char * gdDec[] = { "",
					      "Output data using decimal format.",
					      NULL };	
static char * gdHex[] = { "",
					      "Output data using hex values. ($00 or $0000) (default)",
					      NULL };	
static char * gdByte[] = { "",
					       "Output bytes as single byte values. (default)",
					       NULL };	
static char * gdDbyte[] = { "",
					        "Outputs pairs of bytes as word values low-endian. ($1234 == $12 $34)",
					        NULL };	
static char * gdWord[] = { "",
					       "Outputs pairs of bytes as word values high-endian. ($1234 == $34 $12)",
					       NULL };	
static char * gdIndex[] = { "",
						    "Outputs the character index before the eight character bytes.  See --diff for "
							"how this option benefits the BASIC format.  This is of limited use in the C format "
							"and corrupts the Assembly format output.",
						    NULL };
static char * gdNoIndex[] = { "",
						      "Do not output the character index. (default)",
						      NULL };						  
static char * gdDiff[] = { "",
					       "Outputs the character bytes only if they differ from the ROM character set. "
						   "Where gaps occur the Assembly format will insert a new origination address directive "
						   "declaring the next character bytes in the correct location. "
						   "The options --basic --index --diff used together permit optimizing the data for BASIC. Where "
						   "assembly language can easily declare the bytes assigned at specific memory addresses, "
						   "BASIC must read DATA statements which themselves occupy memory in order to load "
						   "character bit-map values into memory. The output of these options produces the minimum "
						   "amount of data for use by a program such as the example below, which should be compatible with "
						   "TurboBasic XL, and OSS BASIC XL. (The hex values and memory move to copy the character set "
						   "in lines 100 to 120 are not compatible with Atari BASIC.):\n"
						   "100 Rem Copy Rom (Eset) To RAM (Cset)\n"
						   "110 Eset = $e000: Cset = $9000\n"
						   "120 Move Eset, Cset, $0400\n"
						   "130 Rem Read the character index\n"
						   "140 Read Cindex\n"
						   "150 Rem Exit if index is end of data\n"
						   "160 If Cindex = -1 Then Goto 240\n"
						   "170 Rem Determine character location\n"
						   "180 Char = Cset + (Cindex * 8 )\n"
						   "190 Rem Read/Store 8 bytes\n"
						   "200   For Loop = Char to Char + 7\n"
						   "210   Read Cdata: Poke Loop, Cdata\n"
						   "220   Next Loop\n"
						   "230 Goto 140: Rem Read next index\n"
						   "240 Rem Program Continues Here...",
					       NULL };
static char * gdNoDiff[] = { "",
						     "Do not compare. Output all the characters. (default)",
						     NULL };	
static char * gdRom[] = { "num",
						  "Specifies the ROM page to start at (0 to 3). "
						  "For 1024 byte fonts, page must be 0. "
						  "For 512 byte fonts page 0 or 2 may be used. (default 0)",
						  NULL };
static char * gdPages[] = { "nums",
						    "Specifies which 256 byte page(s) of the input font to "
						    "output and the order they are output. The defaults "
						    "are \"0,1,2,3\" for 1024 byte fonts, and \"0,1\" for 512 "
						    "byte fonts.\n",
						    NULL };						   
static char * gdPCom[] = { "",
						   "At the start of each page (32 character block) output a descriptive comment.\n"
						   "Example for Page 3 in C format:\n"
						   "/* Page 0xE3.  Chars 96 to 127 -- lowercase */",
						   NULL };
static char * gdNoPCom[] = { "",
						     "Do not output the page comment. (default)",
						     NULL };						 
static char * gdCCom[] = { "",
						   "Before each block of character bytes output the character information.\n"
						   "Example for a control character in C format with Hex data:\n"
						   "/* 0x7b ctrl-; */ 0x00,0x18,0x3c,0x7e,0x7e,0x18,0x3c,0x00,",
						   NULL };
static char * gdNoCCom[] = { "",
						     "Do not output the character information comment. (default)",
						     NULL };						 
static char * gdBCom[] = { "",
						   "After the block of character bytes output a series of lines of text "
						   "representing the character bitmap as it would be rendered according to the "
						   "\"glyphtype\" value using the \"pixel\" character values to represent the "
						   "font pixels, according to the \"pixelsize\".\n"
						   "Example shown for control character 0x7b as glyphtype 2 in C format for hex data:\n"
						   "/* 0x00:  . . . . . . . .  */\n"
						   "/* 0x18:  . . . # # . . .  */\n"
						   "/* 0x3c:  . . # # # # . .  */\n"
						   "/* 0x7e:  . # # # # # # .  */\n"
						   "/* 0x7e:  . # # # # # # .  */\n"
						   "/* 0x18:  . . . # # . . .  */\n"
						   "/* 0x3c:  . . # # # # . .  */\n"
						   "/* 0x00:  . . . . . . . .  */",
						   NULL };
static char * gdNoBCom[] = { "",
						     "Do not output the character bitmap comments. (default)",
						     NULL };
static char * gdGlType[] = { "num",
						     "Specify the glyph rendering type when the bitmap comment is produced. The value is the "
						     "corresponding ANTIC text mode: 2, 3, 4. The values 5 through 7 are also valid, but map to "
						     "one of these other values: 5 is the same as 4, 6 and 7 are the same as 2.  Antic 2 is two-color "
						     "(bitmap), and Antic 4 is four-color (bit pairs).  Antic 3 is a two-color bitmap, but occupies 10 " 
						     "scan lines using 8 bytes of data by shifting the first two data rows of the lowercase characters to the "
						     "last scan lines, and filling in 0 data for all other \"extra\" scan lines. (default is 2)",
						     NULL };
static char * gdPxSize[] = { "num",
						     "Specifies how many characters wide a \"pixel\" is when the bitmap comment is produced. "
						     "Valid values are 1 through 4. (default 2)",
						     NULL };
static char * gdPxFill[] = { "num",
						     "Specifies how many times the character representing a \"pixel\" will be written out within "
						     "the pixel field (specified by \"pixelsize\") when the bitmap comment is produced.  If the size "
						     " of \"pixelfill\" is less than \"pixelsize\" then the difference will be made up by blank "
						     "spaces. Valid values are 1 through 4. (default 1)",
						     NULL };
static char * gdPixel0[] = { "char(s)",
						     "Specifies the character to use to represent pixel value 0 when the bitmap comment is produced. "
						     "Note that this and all other \"pixel\" values can be a string of multiple characters "
							 "determined by \"pixelsize\" and \"pixelfill\" which can "
						     "be used to simulate the on screen aspect ratios more accurately. "
						     "(default \".\")",
						     NULL };
static char * gdPixel1[] = { "char(s)",
						     "Specifies the character to use to represent pixel value 1 when the bitmap comment is produced. "
						     "(default \"#\")",
						     NULL };
static char * gdPixel2[] = { "char(s)",
						     "Specifies the character to use to represent pixel value 2 when the bitmap comment is produced. "
						     "(default \"O\")",
						     NULL };
static char * gdPixel3[] = { "char(s)",
						     "Specifies the character to use to represent pixel value 3 when the bitmap comment is produced. "
						     "(default \"X\")",
						     NULL };

						 
/* Though individual declaration of each struct is tedious, this
 * allows the remaining code to use a direct name instead of
 * a subscript in the GlobalArgs array, and so, subscript references
 * do not need to be revised every time a new command is inserted
 * in the array.
 */
static GlobalArg gaFile    = { "file",          gdFile,    1, NULL, GA_STRING, 0, 0,     (void *)&gFileName,     0, "stdin",   NULL,       NULL                };

static GlobalArg gaNoDebug = { "nodebug",       NULL,      0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoDebug,      0, "0",       NULL,       NULL                };
static GlobalArg gaDebug   = { "debug",         NULL,      0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoDebug,      1, "1",       &gaNoDebug, NULL                }; 
 
static GlobalArg gaLines   = { "lines",         gdLines,   0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoLines,      0, "1",       NULL,       NULL                };
static GlobalArg gaNoLines = { "nolines",       gdNoLines, 0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoLines,      1, "0",       &gaLines,   NULL                }; 

static GlobalArg gaStart   = { "start",         gdStart,   1, NULL, GA_INT,    1, 32000, (void *)&gLine,         0, "1000",    NULL,       NULL                };

static GlobalArg gaInc     = { "inc",           gdInc,     1, NULL, GA_INT,    1, 1000,  (void *)&gIncr,         0, "10",      NULL,       NULL                };

static GlobalArg gaAsm     = { "asm",           gdAsm,     0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoAsmOrBasiC, 0, "0",       NULL,       NULL                };
static GlobalArg gaBasic   = { "basic",         gdBasic,   0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoAsmOrBasiC, 1, "1",       &gaAsm,     NULL                };
static GlobalArg gaCC      = { "cc",            gdCC,      0, NULL, GA_IFLAG,  2, 2,     (void *)&gDoAsmOrBasiC, 1, "2",       &gaAsm,     NULL                };

static GlobalArg gaAsmOrg  = { "org",           gdAsmOrg,  1, NULL, GA_INT,    1, 64000, (void *)&gAsmOrg,       0, "0x9000",  NULL,       NULL                };

static GlobalArg gaHex     = { "hex",           gdHex,     0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoHexOrDec,   0, "0",       NULL,       NULL                };
static GlobalArg gaDec     = { "dec",           gdDec,     0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoHexOrDec,   1, "1",       &gaHex,     NULL                };

static GlobalArg gaByte    = { "byte",          gdByte,    0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoBDW,        0, "0",       NULL,       NULL                };
static GlobalArg gaDbyte   = { "dbyte",         gdDbyte,   0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoBDW,        1, "1",       &gaByte,    NULL                };
static GlobalArg gaWord    = { "word",          gdWord,    0, NULL, GA_IFLAG,  2, 2,     (void *)&gDoBDW,        1, "2",       &gaByte,    NULL                };

static GlobalArg gaNoIndex = { "noindex",       gdNoIndex, 0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoIndex,      0, "0",       NULL,       NULL                };
static GlobalArg gaIndex   = { "index",         gdIndex,   0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoIndex,      1, "1",       &gaNoIndex, NULL                };

static GlobalArg gaNoDiff  = { "nodiff",        gdNoDiff,  0, NULL, GA_IFLAG,  0, 0,     (void *)&gDoDiff,       0, "0",       NULL,       NULL                };
static GlobalArg gaDiff    = { "diff",          gdDiff,    0, NULL, GA_IFLAG,  1, 1,     (void *)&gDoDiff,       1, "1",       &gaNoDiff,  NULL                };

static GlobalArg gaROM     = { "rom",           gdRom,     1, NULL, GA_INT,    0, 3,     (void *)&gROM,          0, "0",       NULL,       NULL                };

static GlobalArg gaPages   = { "pages",         gdPages,   1, NULL, GA_OTHER,  0, 0,     (void *)&gPages,        0, "0,1,2,3", NULL,       (int (*)())fDoPages };

static GlobalArg gaNoPCom  = { "nopagecomment", gdNoPCom,  0, NULL, GA_IFLAG,  0, 0,     (void *)&gPageComment,  0, "0",       NULL,       NULL                }; 
static GlobalArg gaPCom    = { "pagecomment",   gdPCom,    0, NULL, GA_IFLAG,  1, 1,     (void *)&gPageComment,  1, "1",       &gaNoPCom,  NULL                };

static GlobalArg gaNoCCom  = { "nocharcomment", gdNoCCom,  0, NULL, GA_IFLAG,  0, 0,     (void *)&gCharComment,  0, "0",       NULL,       NULL                }; 
static GlobalArg gaCCom    = { "charcomment",   gdCCom,    0, NULL, GA_IFLAG,  1, 1,     (void *)&gCharComment,  1, "1",       &gaNoCCom,  NULL                };

static GlobalArg gaNoBCom  = { "nobitscomment", gdNoBCom,  0, NULL, GA_IFLAG,  0, 0,     (void *)&gBitsComment,  0, "0",       NULL,       NULL                }; 
static GlobalArg gaBCom    = { "bitscomment",   gdBCom,    0, NULL, GA_IFLAG,  1, 1,     (void *)&gBitsComment,  1, "1",       &gaNoBCom,  NULL                };

static GlobalArg gaGlType  = { "glyphtype",     gdGlType,  1, NULL, GA_INT,    1, 16,    (void *)&gGlyphType,    0, "2",       NULL,       NULL                };
static GlobalArg gaPxSize  = { "pixelsize",     gdPxSize,  1, NULL, GA_INT,    1, 4,     (void *)&gPixelSize,    0, "2",       NULL,       NULL                };
static GlobalArg gaPxFill  = { "pixelfill",     gdPxFill,  1, NULL, GA_INT,    1, 4,     (void *)&gPixelFill,    0, "1",       NULL,       NULL                };

static GlobalArg gaPixel0  = { "pixel0",        gdPixel0,  1, NULL, GA_CHAR,   0, 0,     (void *)&gPixels[0],    0, ".",       NULL,       NULL                };
static GlobalArg gaPixel1  = { "pixel1",        gdPixel1,  1, NULL, GA_CHAR,   0, 0,     (void *)&gPixels[1],    0, "#",       NULL,       NULL                };
static GlobalArg gaPixel2  = { "pixel2",        gdPixel2,  1, NULL, GA_CHAR,   0, 0,     (void *)&gPixels[2],    0, "O",       NULL,       NULL                };
static GlobalArg gaPixel3  = { "pixel3",        gdPixel3,  1, NULL, GA_CHAR,   0, 0,     (void *)&gPixels[3],    0, "X",       NULL,       NULL                };


/* Control structure for command line entries.
 */
static GlobalArg * GlobalArgs[] = 
{
	&gaFile,
	&gaNoDebug,
	&gaDebug,
	&gaLines,
	&gaNoLines,
	&gaStart,
	&gaInc,
	&gaAsm,
	&gaBasic,
	&gaCC,
	&gaAsmOrg,
	&gaHex,
	&gaDec,
	&gaByte,
	&gaDbyte,
	&gaWord,
	&gaNoIndex,
	&gaIndex,
	&gaNoDiff,
	&gaDiff,
	&gaROM,
	&gaPages,
	&gaNoPCom,
	&gaPCom,
	&gaNoCCom,
	&gaCCom,
	&gaNoBCom,
	&gaBCom,
	&gaGlType,
	&gaPxSize,
	&gaPxFill,
	&gaPixel0,
	&gaPixel1,
	&gaPixel2,
	&gaPixel3,
	NULL
};



int dumpMyArgs( void ) /* Debugging.  Output the Globals. */
{
	if ( !gDoDebug )
		return 0;
		
	dumpArgs(); /* Generic Globals -- dump array of structures */

	fprintf(stderr," gdump: gDoDebug      = %d\n",    gDoDebug     ); /* debug to stderr. 1 == yes */
	fprintf(stderr," gdump: gDoLines      = %d\n",    gDoLines     ); /* include line numbers or not. 1 == lines */
	fprintf(stderr," gdump: gLine         = %d\n",    gLine        ); /* start at line number */
	fprintf(stderr," gdump: gIncr         = %d\n",    gIncr        ); /* increment line numbers */
	fprintf(stderr," gdump: gDoAsmOrBasiC = %d\n",    gDoAsmOrBasiC); /* 0 == asm, 1 == BASIC, 2 == C */
	fprintf(stderr," gdump: gAsmOrg       = $%04x\n", gAsmOrg      ); /* Starting assembly address.  default ($9000). */
	fprintf(stderr," gdump: gDoHexOrDec   = %d\n",    gDoHexOrDec  ); /* 0 == hex, 1 == dec */
	fprintf(stderr," gdump: gDoBDW        = %d\n",    gDoBDW       ); /* 0 == byte, 1 == dbyte, 2 == word */
	fprintf(stderr," gdump: gDoIndex      = %d\n",    gDoIndex     ); /* 1 == include character index */
	fprintf(stderr," gdump: gDoDiff       = %d\n",    gDoDiff      ); /* 1 == output only the characters that differ from ROM */
	fprintf(stderr," gdump: gROM          = %d\n",    gROM         ); /* 0 = $E000. 2 == $E200. 1 and 3 possible, but not so useful */
	fprintf(stderr," gdump: gPages[]      = { %d, %d, %d, %d }\n", gPages[0],gPages[1],gPages[2],gPages[3]); /* which 256 byte pages to output */
	fprintf(stderr," gdump: gNumPages     = %d\n",    gNumPages    );
	fprintf(stderr," gdump: gPageComment  = %d\n",    gPageComment );
	fprintf(stderr," gdump: gCharComment  = %d\n",    gCharComment );
	fprintf(stderr," gdump: gBitsComment  = %d\n",    gBitsComment );
	fprintf(stderr," gdump: gGlyphType    = %d\n",    gGlyphType );	
	fprintf(stderr," gdump: gPixelSize    = %d\n",    gPixelSize );
	fprintf(stderr," gdump: gPixelFill    = %d\n",    gPixelFill );
	fprintf(stderr," gdump: gPixels[]     = { '%c', '%c', '%c', '%c' }\n", gPixels[0],gPixels[1],gPixels[2],gPixels[3]); /* which 256 byte pages to output */
			
	fprintf(stderr," gdump: * gFileName  = %p: \"%s\"\n",gFileName, gFileName ? gFileName : "NULL") ; /* Required (not really) first argument */
	fprintf(stderr," gdump:   gFileSize  = %d\n", gFileSize     );    /* stat must show 512 or 1024. */

	fprintf(stderr," gdump: * gStdin     = %p: \"%s\"\n", gStdin, gStdin ? gStdin : "NULL") ; 
	fflush(NULL);
	
	return 0;
}



/* Nasty, hackingness.  Break apart --pages argument into 
 * up to 4 separate numbers, and stuff them in that order into
 * the pages global array.
 * strtok() provides even dumber brokenness, so this is being 
 * worked by hand here.
 */
static int fDoPages( GlobalArg * arg,    /* This matching arg */
					 char      * name,   /* The actual option text matched */
					 char      * value ) /* The actual argument */
{
	int    tempPage;
	int    theEnd    = 0;
	char * tempVal;
	char * testHere  = NULL;
	char * testEnd   = NULL;
	
	if ( !arg || !name || !value )
		return -1;

	/* Do not mangle original string.  Use a copy. */ 
	if ( !(tempVal = strdup( value ) ) )
		return -1;
		
	testHere = tempVal;
	testEnd  = testHere;

	gNumPages = 0;
	
	while ( *testHere && !theEnd )
	{
		while ( isdigit( *testEnd ) )
			testEnd++;
		
		if ( !*testEnd )  /* End of string? */ 
			theEnd++;     /* No more fields follow */
		
		*testEnd = '\0';  /* Force it to be the end of a (shorter) string */
		
		if ( testHere == testEnd ) /* a zero length string?  User shenanigans! */
		{
			if ( !theEnd )
			{
				testHere++; /* Start next string after the end-of-string */
				testEnd++;  /* for the next search */
				continue;
			}
		}
				
		tempPage = atoi( testHere );
		
		if ( ( gNumPages > 3 )  || /* Too many values have been given  or */
		     ( ( tempPage < 0 ) || /* Page number given is out of range...*/
			   ( tempPage > 3 ) ) )
		{
			free(tempVal);
			return -1;
		}
		
		gPages[ gNumPages++ ] = tempPage ;

		/* try for the next field */
		if ( !theEnd )
			testEnd++; /* Skip over the current end of string */

		testHere = testEnd;
	}
	
	arg->isSet = 1;  /* and turn off 'pages' default override. */

	free( tempVal );
	
	return 0;
}



/* Sanity check the pages and sizes. 
 * The number of pages being output cannot be more than the 
 * number of pages available.
 *		 
 * So, If the starting ROM page is not zero, then the number of pages,  
 * and the max page number change.
 * --pages 3,2,1,0 is perfectly legal... but
 * --diff 2 means a 512 byte font and pages 3 and 2 are now illegal.  
 * Also, if the FileSize is 512 bytes a similar situation occurs where
 * only pages 0 and 1 are valid.
 *
 * Note that --pages are always relative to whatever gRom states is 
 * the first page of the font.
 *
 * Rather than error out on every silly combination, the pages list will 
 * be truncated if too long and then the page list will be checked.
 * 
 * Also Sanity check glyph/pixel settings.
 * Glyph type may only be 2 or 4.
 * Pixel fill must be <= Pixel size.
 */
 int sanityCheck( void ) /* All values are global */
 {
 	int loop    = 0;
	int maxPage = 0;

	if ( gFileSize == 512 ) /* 1/2 K font is only pages 0 and 1. */
		maxPage = 1;
	else 
		maxPage = 3 - gROM;

	if ( gNumPages > ( maxPage + 1 ) ) /* Page list can't have more entries than pages */ 
		gNumPages = maxPage + 1;
		
	while ( loop < gNumPages ) /* Check page list for out of range pages */
	{
		if ( gPages[ loop ] > maxPage )
		{
			fprintf( stderr,
					 "Error: Page %d is too large for font size and ROM base.\n",
					 gPages[ loop ] );
			return -1;
		}
		
		loop++;
	}
	
	/* GlyphType only supports 2 or 4 colors per character bitmap... 
	 * Though this value could also be thought of as Antic mode 2 or 4.  
	 * Perhaps in the future this will support Antic Mode 3 descenders.
	 * Note that Modes 6 and 7 are effectively the same as Mode 2 (2 colors
	 * per glyph bitmap), and Mode 5 is the same as Mode 4 (4 colors  
	 */
	if ( ( gGlyphType < 2 ) || ( gGlyphType > 7 ) ) 
	{
		fprintf( stderr,
				 "Error: GlyphType cannot be %d.  It may range from 2 through 7.\n",
				 gGlyphType );
		return -1;
	}

	/* can't fill more pixels than allowed by the field size */
	
	if ( gPixelFill > gPixelSize )
	{
		fprintf( stderr,
				 "Error: PixelFill (%d) can't be greater than PixelSize (%d) field width.\n",
				 gPixelFill,
				 gPixelSize );
		return -1;
	}
	
	return 0;
}



/* Return the Global Args, so it can be passed to doArgs */

GlobalArg ** get_GA( void )
{
	return GlobalArgs;
}
