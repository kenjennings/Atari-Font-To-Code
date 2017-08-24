/* Dump font data as program text either for BASIC or assembly or C
 */

/* $Id: io.c,v 1.7 2014/03/31 20:53:36 kjenning Exp $
 *
 * $Log: io.c,v $
 * Revision 1.7  2014/03/31 20:53:36  kjenning
 * Added Assembly origin command line argument.
 * Cleaned up assemby output.
 * Probably made the code even cruftier.
 * Rearranged some of the help text.
 *
 * Revision 1.6  2014/03/26 20:44:11  kjenning
 * Slight cleanup of bloaty cruftiness.   A little modularity
 * inflicted on the I/O section.  Probably fixed the C
 * output bug that resulted in a trailing comma at the end
 * of data when character comments were used.
 *
 * Revision 1.5  2014/03/18 19:47:11  kjenning
 * Bells and Whistles implemented and some sanity
 * testing completed.  This seems to be working
 * well enough, so here's a check-in baseline.
 *
 * Revision 1.4  2014/03/13 20:41:56  kjenning
 * Interim check-in after myglobals and documentation
 * text formatting were revised and tested.   Still
 * more features to add per the configs.
 *
 * Revision 1.3  2013/11/07 21:38:45  kjenning
 * Added command line controls for outputting
 * decorative comments with the data.  The
 * actual code to do the decorating is not
 * included here. Wait for a future version.
 *
 * Revision 1.2  2013/10/21 20:48:50  kjenning
 * Temporary Checkin.
 * Most functionality had been working.
 * Next unecessary revision of the globals due
 * to begin after this version.
 *
 * Revision 1.1  2013/10/17 20:36:08  kjenning
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "io.h"
#include "myglobals.h"

static char * pageTitles[] = 
{ 
	"Page 0xE0.  Chars 0 to 31 -- Symbols, numbers",
	"Page 0xE1.  Chars 32 to 63 -- Uppercase",
	"Page 0xE2.  Chars 64 to 95 -- graphics control characters",
	"Page 0xE3.  Chars 96 to 127 -- lowercase",
	NULL
};

static char * charTitles[] =
{
/* Page 0xE0.  Chars 0 to 31 -- Symbols, numbers */
	/* 0x00 */ "SPACE ",
	/* 0x01 */ "  !   ",
	/* 0x02 */ "  \"   ",  
	/* 0x03 */ "  #   ",
	/* 0x04 */ "  $   ",
	/* 0x05 */ "  %   ",
	/* 0x06 */ "  &   ",
	/* 0x07 */ "  '   ",
	/* 0x08 */ "  (   ",
	/* 0x09 */ "  )   ",
	/* 0x0a */ "  *   ",
	/* 0x0b */ "  +   ",
	/* 0x0c */ "  ,   ",
	/* 0x0d */ "  -   ",
	/* 0x0e */ "  .   ",
	/* 0x0f */ "  /   ",
	/* 0x10 */ "  0   ",
	/* 0x11 */ "  1   ",
	/* 0x12 */ "  2   ",
	/* 0x13 */ "  3   ",
	/* 0x14 */ "  4   ",
	/* 0x15 */ "  5   ",
	/* 0x16 */ "  6   ",
	/* 0x17 */ "  7   ",
	/* 0x18 */ "  8   ",
	/* 0x19 */ "  9   ",
	/* 0x1a */ "  :   ",
	/* 0x1b */ "  ;   ",
	/* 0x1c */ "  <   ",
	/* 0x1d */ "  =   ",
	/* 0x1e */ "  >   ",
	/* 0x1f */ "  ?   ",
/* Page 0xE1.  Chars 32 to 63 -- Uppercase */
	/* 0x20 */ "  @   ",
	/* 0x21 */ "  A   ",
	/* 0x22 */ "  B   ",
	/* 0x23 */ "  C   ",
	/* 0x24 */ "  D   ",
	/* 0x25 */ "  E   ",
	/* 0x26 */ "  F   ",
	/* 0x27 */ "  G   ",
	/* 0x28 */ "  H   ",
	/* 0x29 */ "  I   ",
	/* 0x2a */ "  J   ",
	/* 0x2b */ "  K   ",
	/* 0x2c */ "  L   ",
	/* 0x2d */ "  M   ",
	/* 0x2e */ "  N   ",
	/* 0x2f */ "  O   ",
	/* 0x30 */ "  P   ",
	/* 0x31 */ "  Q   ",
	/* 0x32 */ "  R   ",
	/* 0x33 */ "  S   ",
	/* 0x34 */ "  T   ",
	/* 0x35 */ "  U   ",
	/* 0x36 */ "  V   ",
	/* 0x37 */ "  W   ",
	/* 0x38 */ "  X   ",
	/* 0x39 */ "  Y   ",
	/* 0x3a */ "  Z   ",
	/* 0x3b */ "  [   ",
	/* 0x3c */ "  \\   ",
	/* 0x3d */ "  ]   ",
	/* 0x3e */ "  ^   ",
	/* 0x3f */ "  _   ",
/* Page 0xE2.  Chars 64 to 95 -- graphics control characters */
	/* 0x40 */ "ctrl-,",
	/* 0x41 */ "ctrl-A",
	/* 0x42 */ "ctrl-B",
	/* 0x43 */ "ctrl-C",
	/* 0x44 */ "ctrl-D",
	/* 0x45 */ "ctrl-E",
	/* 0x46 */ "ctrl-F",
	/* 0x47 */ "ctrl-G",
	/* 0x48 */ "ctrl-H",
	/* 0x49 */ "ctrl-I",
	/* 0x4a */ "ctrl-J",
	/* 0x4b */ "ctrl-K",
	/* 0x4c */ "ctrl-L",
	/* 0x4d */ "ctrl-M",
	/* 0x4e */ "ctrl-N",
	/* 0x4f */ "ctrl-O",
	/* 0x50 */ "ctrl-P",
	/* 0x51 */ "ctrl-Q",
	/* 0x52 */ "ctrl-R",
	/* 0x53 */ "ctrl-S",
	/* 0x54 */ "ctrl-T",
	/* 0x55 */ "ctrl-U",
	/* 0x56 */ "ctrl-V",
	/* 0x57 */ "ctrl-W",
	/* 0x58 */ "ctrl-X",
	/* 0x59 */ "ctrl-Y",
	/* 0x5a */ "ctrl-Z",
	/* 0x5b */ "ESCAPE",
	/* 0x5c */ "UP    ",
	/* 0x5d */ "DOWN  ",
	/* 0x5e */ "LEFT  ",
	/* 0x5f */ "RIGHT ",
/* Page 0xE3.  Chars 96 to 127 -- lowercase */
	/* 0x60 */ "ctrl-.",
	/* 0x61 */ "  a   ",
	/* 0x62 */ "  b   ",
	/* 0x63 */ "  c   ",
	/* 0x64 */ "  d   ",
	/* 0x65 */ "  e   ",
	/* 0x66 */ "  f   ",
	/* 0x67 */ "  g   ",
	/* 0x68 */ "  h   ",
	/* 0x69 */ "  i   ",
	/* 0x6a */ "  j   ",
	/* 0x6b */ "  k   ",
	/* 0x6c */ "  l   ",
	/* 0x6d */ "  m   ",
	/* 0x6e */ "  n   ",
	/* 0x6f */ "  o   ",
	/* 0x70 */ "  p   ",
	/* 0x71 */ "  q   ",
	/* 0x72 */ "  r   ",
	/* 0x73 */ "  s   ",
	/* 0x74 */ "  t   ",
	/* 0x75 */ "  u   ",
	/* 0x76 */ "  v   ",
	/* 0x77 */ "  w   ",
	/* 0x78 */ "  x   ",
	/* 0x79 */ "  y   ",
	/* 0x7a */ "  z   ",
	/* 0x7b */ "ctrl-;",
	/* 0x7c */ "  |   ",
	/* 0x7d */ "CLEAR ",
	/* 0x7e */ "DELETE",
	/* 0x7f */ "TAB   ",
	NULL
};



/* Controlling C output... */
static int firstCCA = 0;    /* flag that C/Asm format did/did not output the array start declarations */
static int dataCC   = 0;    /* flag that C data was/was not output... so, a comma must be added for more data */



extern unsigned char * getE000( void ); /* Get the standard font for comparison */


#define CKP( STR ) fprintf(stderr, "Checkpoint in %s at %d: ",__FILE__,__LINE__); fprintf(stderr, STR); fputc( '\n', stderr ); fflush(NULL);


static char * rcs_id = "$Id: io.c,v 1.7 2014/03/31 20:53:36 kjenning Exp $";

char * io_rcs_id( void ) /* report file version for io.o */
{
	return rcs_id;
}



/* Read the data.
 * If this is a regular file, check file size first. 
 * If this is stdin, then fill up a buffer up to the max size of a font.
 */ 
int readFile( void )
{
	int ch         = 0;
	int expectSize = 0;
	FILE * myFile  = NULL;
	struct stat st;
	
	/* Filename defined and is not "stdin" ? */
	if ( gFileName && strcmp( gFileName, gStdin ) )
	{
		if ( stat( gFileName, &st ) )
		{
			fprintf( stderr,
					 "Error: unable to stat(\"%s\")\n",
					 gFileName );
			return -1;
		}
		
		/* In a future iteration we might allow reading a binary 
		 * segment load file containing load addresses.  But, for 
		 * now this only allows half-K or 1K raw memory in a file.
		 */
		expectSize = st.st_size;
		
		if ( ( expectSize != 512 ) && ( expectSize != 1024 ) )
		{
			fprintf( stderr,
					 "Error: file \"%s\" size of %d bytes is not 512 or 1024.\n",
					 gFileName,
					 expectSize );
			return -1;
		}
		
		myFile = fopen( gFileName, "r" );
		
		if ( !myFile )
		{
			fprintf( stderr,
					 "Error: unable to fopen(\"%s\", \"r\");\n",
					 gFileName );
			return -1;
		}
	}
	else /* reading stdin */
		myFile = stdin;
		
	while ( ( ( ch = fgetc( myFile ) ) != EOF ) && ( gFileSize < 1024 ) )
		gCset[gFileSize++] = ch;
		
	if ( gFileSize < 512 ) /* If this is too small for any purpose, then error */
	{
		fprintf( stderr,
				 "Error: only read %d bytes from file \"%s\"",
				 gFileSize,
				 gFileName );
		
		if ( myFile != stdin )
			fclose( myFile );

		return -1;
	}

	if ( expectSize && ( expectSize != gFileSize ) ) /* Not what was expected from file */
	{
		fprintf( stderr,
				 "Warning: read %d bytes, but expected %d, from  file \"%s\".",
				 gFileSize,
				 expectSize,
				 gFileName );
	}
	
	if ( ( gFileSize > 512 ) && ( gFileSize < 1024 ) ) /* Incomplete 1K font, so reduce to usable data */
	{
		fprintf( stderr,
				 "Warning: read %d bytes, not 512 or 1024, from  file \"%s\". Using 512.",
				 gFileSize,
				 gFileName );

		gFileSize = 512;
	}
	
	if ( myFile != stdin )
		fclose( myFile );

	return 0;
}
	


/* if this is C format and the last item output was data without	 
 * the comma then end that line now before outputting more data.
 */					
static void doDataCC( void )
{
	if ( ( gDoAsmOrBasiC == 2) && dataCC )
	{
		fprintf( stdout, ",\n" );
		dataCC = 0;
	}
}


/* Output a line number if it is needed. 
 * Yes, everything this function needs is global. 
 */
static void doLine( void )
{
	doDataCC(); /* Check if prior C data needs end of line */
	
	if ( gDoLines && ( gDoAsmOrBasiC != 2 ) ) /* Output Line number except for C */
	{
		fprintf( stdout, "%d ", gLine );

		gLine += gIncr;
	}
}



/* Force data declaration for C
 * if it is needed.
 */
static void doFirstCCA( void )
{
	if ( firstCCA )
		return;

	if ( gDoAsmOrBasiC == 0 ) /* Last chance.  Output ORG for Assembly if needed */
	{
		doLine(); /* Output Line number if needed ... and end prior text if needed */
		fprintf( stdout, "CSET=$%04X\n", gAsmOrg);
		firstCCA++;
	}
		
	if ( gDoAsmOrBasiC == 2 ) /* Last chance.  Output declaration for C if needed */
	{
		doLine(); /* Output Line number if needed ... and end prior text if needed */
		fprintf( stdout, "unsigned %s cset[] =\n{\n", ( gDoBDW == 0 ) ? "char" : "int" );
		firstCCA++;
	}
	
	
}



/* Does more data follow the current character?
 * This is used to circumvent the end of file bug
 * where C leaves a trailing comma after the last 
 * data when comments follow the data.  Yes, this 
 * is a redundant copy of the code in outputData.
 */
static int lookAheadCC( unsigned char * E0,        /* ROM Character Set */
						int             romStart,  /* Page * 256. */
						unsigned char * cSet,      /* User's Character Set */
						int             dataStart, /* Input page * 256. */
						int             index,     /* The per page byte counter 0 to 256-1 */
						int             thisPage)  /* Which input page are we outputting */
{
	int theEnd  = 0;
	int newLine = 0; /* By default there is NOT a following character */ 
	
	if ( !E0   || !cSet || 
	    ( romStart  < 0 ) || ( romStart  > 1023 ) ||
		( dataStart < 0 ) || ( dataStart > 1023 ) ||
		( thisPage  < 0 ) || ( thisPage  > 3    ) ||
		( index     < 0 ) || ( index     > 255  ) ) /* Invalid arguments? */
		return 0; /* Nothing follows */
	
	index += 8; /* Values provided point to current character. Start at the next character's bytes */

	if ( index >= 256 ) /* Go to next page?  Or is it the end? */
	{
		index = 0;
		romStart += 256;
		thisPage++;
					
		if ( thisPage == gNumPages ) /* Output all that was requested? */
			theEnd++;
		else
			dataStart = gPages[thisPage] * 256;
	}		

	while ( !theEnd && !newLine )
	{	
	    /* Not in Diff mode means this character is a good character, so testing can stop */
		if ( !gDoDiff )  
		{	
			newLine = 1;
			continue;
		}
		
		/* In Diff mode means a newLine is based on ROM vs file difference */		
		if ( memcmp( E0    + romStart  + index,
					 gCset + dataStart + index,
					 8 ) )
		{
			newLine = 1; /* It does not match so testing can stop */
			continue;
		}
		
		index += 8; /* Step to next character's bytes */

		if ( index >= 256 ) /* Go to next page?  Or is it the end? */
		{
			index = 0;
			romStart += 256;
			thisPage++;
					
			if ( thisPage == gNumPages ) /* Output all that was requested? */
				theEnd++;
			else
				dataStart = gPages[thisPage] * 256;
		}
	}
	
	return newLine;
}



/* Output a byte (or word) as hex or decimal in a format based on the 
 * type or program source being output.  A value in decimal may require 
 * padding to 3 digits. However, the valid sizes for hex are 2 or 4.  
 * To allow the caller not to make a decision any odd value is turned 
 * even for hex output.
 */
static void decOrHex( unsigned int size,
					  unsigned int val )
{
	if ( !gDoHexOrDec   &&                /* Size check is valid for Hex output */
	    ( ( size <  1 ) || ( size > 5 ) )  ) /* Maybe 5 could be valid for dec. */
		return;

	if ( gDoHexOrDec ) /* Decimal */
	{
		if ( size == 3 ) /* special case output padded */
			fprintf(stdout, "%03d", val );
		else             /* otherwise, as is without padding */
			fprintf(stdout, "%d", val );
	}
	else
	{
		size = size & 0x06; /* 3 for dec is 2 for hex */
		
		if ( gDoAsmOrBasiC == 2 ) /* C is different */
			fprintf(stdout, "0x%0*x", size, val );
		else                      /* asm and BASIC. */
			fprintf(stdout, "$%0*X", size, val );
	}
}


/* Output one byte representing the bitmap at text patterns.  
 * Monochrome and 4 color (bit pairs) are supported.  
 */
static void charPicture( unsigned char val )
{
	int    bits     = 0;    /* masked, shifted bits to the low bits position. */
	int    pixS     = 0;    /* from gPixelSize */
	int    pixF     = 0 ;   /* from gPixelFill */
	int  * mask     = NULL;
	int  * shift    = NULL;
	int    mask2[]  = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00 };
	int    shift2[] = { 7, 6, 5, 4, 3, 2, 1, 0, -1 };
	int    mask4[]  = { 0xC0, 0x30, 0x0C, 0x03, 0x00 };
	int    shift4[] = { 6, 4, 2, 0, -1 };

	
	if ( ( gGlyphType == 4 ) || 
		 ( gGlyphType == 5 ) ) /* Color bits */
	{
		mask  = mask4;
		shift = shift4;
	}
	else /* Monochrome bitmap */
	{
		mask  = mask2;
		shift = shift2;
	}
				
	while ( *shift >= 0 ) /* Negative shift means we have output all the bits */
	{
		bits = ( val & ( *mask ) ) >> *shift; /* Extract bits, shift down to low bits position */
			
		pixS = gPixelSize; /* The number of characters in a "pixel" */
		pixF = gPixelFill; /* The number of times the "graphics" character is duplicated in the "pixel". */
						
		if ( pixS < pixF ) /* The Fill count can't be bigger than the Size */
			pixS = pixF;
							
		while ( pixF ) /* Output pixel up to fill size */
		{
			fprintf(stdout, "%c", gPixels[ bits ] ) ;
			pixS--;
			pixF--;
		}
						
		while ( pixS-- ) /* If pixel width continues, pad spaces */
			fputc( ' ', stdout );
						 				
		mask++;
		shift++;
	} /* Output next bit(s) in this byte */
}					



/* Write the font data.
 * Most of the ugliness occurs here.
 */
int outputData( void ) /* Shamefully, everything is in global vars. */
{
	int theEnd    = 0;    /* Flag out of data */
	int newLine   = 1;    /* Flag to generate start of line */
	int romPage   = gROM; /* Start with this ROM page if doing Diff */
	int index     = 0;    /* The per page byte counter 0 to 256-1 */
	int romStart  = 0;    /* Page * 256. */
	int thisPage  = 0;    /* Which input page are we outputting */
	int dataStart = 0;    /* Input page * 256. */
	int loop      = 0;    /* 0 to 7 loop for the individual character. */
	int dataOut   = 0;    /* Flag if any char has been output */
	int hiB       = 0;    /* hi byte when pairing */
	int loB       = 0;    /* lo byte when pairing */

	char * dtype[]    = { "  .BYTE", "  .DBYTE", "  .WORD", NULL };
	char * sComment[] = { "; ", "REM ", "/* ", NULL };
	char * eComment[] = { "", "", " */", NULL };
	int    pageOut[]  = { 0, 0, 0, 0, -1 };
	int    order1[]   = { 0, 1, 2, 3, 4, 5, 6, 7, -2 };
	int    order2[]   = { 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -2 };
	int    order3[]   = { -1, -1, 2, 3, 4, 5, 6, 7, 0, 1, -2 };
	int  * order      = NULL;
	unsigned char * E0 = NULL;

	int    charName   = -2; /* The real index of this character in the character set $00 to $7f */
	int  lastCharName = -2; /* Remember the last charname output. If this - last != 1, then asm needs an address */

	firstCCA = 0;    /* flag that C/Asm format did/did not output the array start declarations */
	dataCC   = 0;    /* flag that C data was/was not output... so, a comma must be added for more data */
	E0       = getE000();
	
	if ( !E0 && gDoDiff )  /* No internal font means Diff is not possible */
	{
		fprintf( stderr,
				 "Error: No internal default font for comparison.\n" );
		return -1;
	}

	romStart  = romPage * 256;
	dataStart = gPages[ thisPage ] * 256;
/*
CKP("outputData()");
fprintf(stderr,	"romStart (%d) == romPage (%d) * 256\n",romStart, romPage);
fprintf(stderr, "dataSTart (%d) == gPages[thisPage (%d)] (%d) * 256\n",dataStart , thisPage, gPages[thisPage]);
fflush(NULL);
*/
	while ( !theEnd )
	{	
		if ( gDoDiff ) /* Flag newLine based on ROM vs file difference */ 
		{	
			if ( memcmp( E0    + romStart  + index,
						 gCset + dataStart + index,
						 8 ) )
				newLine = 1;
			else 
				newLine = 0;
		} /* If in Diff mode, flag for output or not based on ROM vs file */
		
		if ( !theEnd && newLine ) /* Not finished yes, and it's time to output a line */
		{	
			dataOut++; /* So, we know later there was data output */
			
			lastCharName = charName;
			charName = ( gPages[ thisPage ] * 32 ) + ( index / 8 ) ; /* Real index in cset, $00 to $7f */
			
			if ( gPageComment )
			{
				if ( !pageOut[ gPages[thisPage] ] )
				{
					doFirstCCA(); /* Force Asm ORG, or C data declaration if needed */

					doLine(); /* Output Line number if needed */
				
					if ( gDoAsmOrBasiC == 2 )
						fputc( '\t', stdout );
					
					fprintf(stdout, "%s", sComment[ gDoAsmOrBasiC ] );					
					fprintf(stdout, "%s", pageTitles[ gPages[ thisPage ] ] );
					fprintf(stdout, "%s\n", eComment[ gDoAsmOrBasiC ] );

					pageOut[ gPages[thisPage] ]++; 
				}
			}
			
			if ( ( gDoAsmOrBasiC == 0 ) && /* Asm */
			     ( ( charName - lastCharName ) != 1 ) )
			{
				doFirstCCA(); /* Force Asm ORG, or C data declaration if needed */
			
				doLine(); /* Output Line number if needed */
				if ( ( dataStart + index ) != 0 ) 
					fprintf( stdout, 
							 "  *=CSET+$%0*X\n",
							 ( ( dataStart + index ) < 0x100 ) ? 2 : 4, 
							 dataStart + index );
				else
					fprintf( stdout,
							 "  *=CSET\n");
			}
				
			if ( gCharComment )
			{
				doFirstCCA(); /* Force Asm ORG, or C data declaration if needed */
			
				doLine(); /* Output Line number if needed */

				if ( gDoAsmOrBasiC == 2 ) /* C */
					fprintf( stdout, "\t\t"  );
					
				fprintf(stdout, "%sChar ", sComment[ gDoAsmOrBasiC ] );	
				decOrHex( 3, charName);
				fprintf(stdout, ": ");
				fprintf(stdout, "%s", charTitles[ charName ] );
				fprintf(stdout, "%s ", eComment[ gDoAsmOrBasiC ] );
					
				if ( gDoAsmOrBasiC != 2 ) /* If this is C, no EOL needed */
					fputc( '\n', stdout );
			}

			/* Stay on same line for C format + Char Comment.  
			 * New line for BASIC or Assembly, and C when no Char comment
			 */
			if ( !( gCharComment && ( gDoAsmOrBasiC == 2 ) ) )
				doLine(); /* Output Line number if needed */
			
			switch ( gDoAsmOrBasiC ) /* Start data with BASIC or ASM directive. */
			{
				case 0: /* Asm  */
					doFirstCCA(); /* Force Asm ORG if needed */
					fprintf(stdout, "%s ", dtype[ gDoBDW ] );
					break;
					
				case 1: /* Basic */
					fprintf(stdout, "DATA ");
					break;
					
				case 2: /* C */
					doFirstCCA(); /* Force  C data declaration if needed */
					if ( !gCharComment ) /* If no comment, then tab for data */
						fputc( '\t', stdout );
					break;
			}
				
			/* In the case of the index only one byte ( that is, a simple, tiny int) goes 
			 * out, so the basic or asm flags do not matter 
			 */
			if ( gDoIndex ) /* Include this character's font entry index? */
			{
				doDataCC(); /* Check if prior C data needs end of line */
				
				decOrHex( 2, charName );
				fputc( ',', stdout );
			}
		
			for ( loop = 0; loop <= 7 ;  loop++ )  /* Output the 8 data bytes */
			{
				loB = (int)*(gCset + dataStart + index + loop); /* Get a (low) byte */
				
				switch ( gDoBDW ) /* How is output data grouped? bytes, or ints?*/
				{
					case 0: /* Byte */
						decOrHex( 2, loB);
						break;

					case 1: /* dbyte, low-endian*/
						hiB = loB; /* The low byte is really the high byte */
						loop++;
						loB = (int)*(gCset + dataStart + index + loop);
						
						decOrHex( 4, ( hiB * 256 ) + loB );
						break;

					case 2: /* word, high-endian */
						loop++;
						hiB = (int)*(gCset + dataStart + index + loop);
						
						decOrHex( 4, ( hiB * 256 ) + loB );
				} /* Switch output data grouping */

				if ( loop < 7 ) /* Comma out after the first 7 values */ 
					fputc(',', stdout);
			}  /* Output next byte in this character */	

			/* C may need to end this data line with a comma if more data will be output.  But it
			 * ends with a regular EOL if no more data follows.   So, traverse the character 
			 * data to determine if more character data will be output. ...
			 */ 			 
			if ( gDoAsmOrBasiC == 2) /* If C format then suppress EOL, but check if more data follows, */ 
			{                        /* and then remember there is another comma to output... */
				dataCC = lookAheadCC( E0,         /* ROM Character Set */
									  romStart,   /* Page * 256. */
									  gCset,      /* User's Character Set */
									  dataStart,  /* Input page * 256. */
									  index,      /* The per page byte counter 0 to 256-1 */
									  thisPage ); /* Which input page are we outputting */
				if ( !dataCC )
					fputc('\n', stdout); /* No character will follow */
			}
			else                     /* Asm and BASIC always ends a line of data with the EOL */ 
				fputc('\n', stdout);
						
			if ( gBitsComment )  			/* Would we like a pretty picture included? */
			{
				/* Determine the order of reading bytes.  Mode 3 is a special 
				 * mode that juggles bytes around to provide true lowercase
				 * descenders.  The byte order is changed for this glyph type
				 * to properly display the bits' picture.
				 */
				if ( gGlyphType == 3 )  /* The weird cases for lowercase descenders */
				{
					if ( gPages[ thisPage ] == 3 ) /* Lowercase pattern */
						order = order3;
					else                 /* Regular pattern */
						order = order2;
				}
				else
					order = order1; /* Regular characters bytes ordered 0 to 7 */
					
				while ( *order > -2 ) /* -2 is the done/no more data flag */
				{		
					doLine();

					if ( gDoAsmOrBasiC == 2 )
						fprintf( stdout, "\t\t\t\t\t" );

					fprintf(stdout, "%s", sComment[ gDoAsmOrBasiC ] );					

					if ( *order == -1 )
						loB = 0x00; /* Mode 3 has empty filler lines */
					else
						loB = (int)*(gCset + dataStart + index + *order);

					decOrHex( 3, loB ); /* Hex format will make this 3 into 2 */
					fprintf(stdout, ": " );

					charPicture( loB ); /* Produce the text "picture" of this byte */
					
					fprintf(stdout, "%s\n", eComment[ gDoAsmOrBasiC ] );
					
					order++;
				}  /* Output next byte in this character */
				
			} /* end if (gBitsComment ) */
			
		} /* end if Output this character if this is not the end */ 
			
		index += 8; /* Step to next character's bytes */

		if ( index >= 256 ) /* Go to next page?  Or is it the end? */
		{
			index = 0;
			romStart += 256;
			thisPage++;
					
			if ( thisPage == gNumPages ) /* Output all that was requested? */
				theEnd++;
			else
				dataStart = gPages[thisPage] * 256;
		}

		if ( theEnd )
		{
			if ( ( gDoAsmOrBasiC == 2) && dataCC )
				fprintf( stdout, "\n" );
				
			if ( ( gDoAsmOrBasiC == 2 ) && dataOut ) /* If C, then close the array... */
				fprintf( stdout, "};\n" );
		}
	} /* Loop to the next character if this is not the end.*/
	
	return 0;
}

