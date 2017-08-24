/* Dump font data as program text either for BASIC or assembly
 */

/* $Id: main.c,v 1.17 2014/04/04 18:37:10 kjenning Exp $
 *
 * $Log: main.c,v $
 * Revision 1.17  2014/04/04 18:37:10  kjenning
 * Typo in docs. Embarrrasssing.
 *
 * Revision 1.16  2014/04/04 17:00:07  kjenning
 * Minor documentation fix.  Docs referring to pixels representation
 * as characters plural, when the argument only accepts one
 * character for the pixel.
 *
 * Revision 1.15  2014/03/31 20:53:36  kjenning
 * Added Assembly origin command line argument.
 * Cleaned up assemby output.
 * Probably made the code even cruftier.
 * Rearranged some of the help text.
 *
 * Revision 1.14  2014/03/26 20:44:11  kjenning
 * Slight cleanup of bloaty cruftiness.   A little modularity
 * inflicted on the I/O section.  Probably fixed the C
 * output bug that resulted in a trailing comma at the end
 * of data when character comments were used.
 *
 * Revision 1.13  2014/03/18 20:04:15  kjenning
 * Changed program name to atf2code.
 *
 * Revision 1.12  2014/03/18 19:52:38  kjenning
 * Updated the commands and comments to reflect
 * the current program's usage output.
 *
 * Revision 1.11  2014/03/18 19:47:11  kjenning
 * Bells and Whistles implemented and some sanity
 * testing completed.  This seems to be working
 * well enough, so here's a check-in baseline.
 *
 * Revision 1.10  2014/03/13 20:41:56  kjenning
 * Interim check-in after myglobals and documentation
 * text formatting were revised and tested.   Still
 * more features to add per the configs.
 *
 * Revision 1.9  2013/11/15 21:15:40  kjenning
 * Added options for commented output.  No supporting code.
 * This is a temporary check-in with no build verification.
 * Likely does not build at all.
 *
 * Revision 1.8  2013/11/07 21:38:45  kjenning
 * Added command line controls for outputting
 * decorative comments with the data.  The
 * actual code to do the decorating is not
 * included here. Wait for a future version.
 *
 * Revision 1.7  2013/11/01 21:05:15  kjenning
 * Cut out the redundant, per-value function parser replacing
 * most of them with a generic argument/value parser.
 *
 * Revision 1.6  2013/11/01 20:50:32  kjenning
 * Check-In after buildable, untested globals overhaul
 * prior to gutting the old, bloaty, redundant parsing
 * functions in globals.c.
 *
 * Revision 1.5  2013/10/21 20:48:50  kjenning
 * Temporary Checkin.
 * Most functionality had been working.
 * Next unecessary revision of the globals due
 * to begin after this version.
 *
 * Revision 1.4  2013/10/17 20:36:08  kjenning
 * Check-In after modular-ity-izing some parts of code.
 * Several options still not working properly.
 *
 * Revision 1.3  2013/10/10 20:52:38  kjenning
 * Stupid compiling issues resolved.
 * Currently appears to be functioning.
 * TO DO: Fix behavior of diff argument.
 * TO DO: separate I/O parts from globals.
 *
 * Revision 1.2  2013/10/09 17:54:52  kjenning
 * Temp Check-In.
 * Building, non-running state.
 *
 * Revision 1.1  2013/09/23 17:15:46  kjenning
 * Initial revision
 *
 */

/* 
atf2code

Dump Atari font (1024 or 512 byte) files as data statements for a programming
language -- DATA for BASIC, a number of data directives for assembly language,
or an unsigned char array for C.
Data may be output as hex or decimal values.  Line numbers may or may not be
included.

Usage:

atf2code [options]

file          filename == Providing filename 'stdin' (default) will use stdio
                          instead.

lines                  == Include line numbers. The C format always overrides
                          this to turn off line numbers. (default for BASIC and
                          ASM).

nolines                == Do not include line numbers. (default for C)

start              num == Starting line number less than 32000 (default 1000).

inc                num == Line number increment. (default 10).

asm                    == Output for Assembly. (default)

basic                  == Output for BASIC. (DATA format only)

cc                     == Output for C. (unsigned char array)

org                num == Starting address for Assembly output. (default $9000)

hex                    == Output data using hex values. ($00 or $0000)
                          (default)

dec                    == Output data using decimal format.

byte                   == Output bytes as single byte values. (default)

dbyte                  == Outputs pairs of bytes as word values low-endian.
                          ($1234 == $12 $34)

word                   == Outputs pairs of bytes as word values high-endian.
                          ($1234 == $34 $12)

noindex                == Do not output the character index. (default)

index                  == Outputs the character index before the eight
                          character bytes.  See --diff for how this option
                          benefits the BASIC format.  This is of limited use in
                          the C format and corrupts the Assembly format output.

nodiff                 == Do not compare. Output all the characters. (default)

diff                   == Outputs the character bytes only if they differ from
                          the ROM character set. Where gaps occur the Assembly
                          format will insert a new origination address
                          directive declaring the next character bytes in the
                          correct location. The options --basic --index --diff
                          used together permit optimizing the data for BASIC.
                          Where assembly language can easily declare the bytes
                          assigned at specific memory addresses, BASIC must
                          read DATA statements which themselves occupy memory
                          in order to load character bit-map values into
                          memory. The output of these options produces the
                          minimum amount of data for use by a program such as
                          the example below, which should be compatible with
                          TurboBasic XL, and OSS BASIC XL. (The hex values and
                          memory move to copy the character set in lines 100 to
                          120 are not compatible with Atari BASIC.):
                          100 Rem Copy Rom (Eset) To RAM (Cset)
                          110 Eset = $e000: Cset = $9000
                          120 Move Eset, Cset, $0400
                          130 Rem Read the character index
                          140 Read Cindex
                          150 Rem Exit if index is end of data
                          160 If Cindex = -1 Then Goto 240
                          170 Rem Determine character location
                          180 Char = Cset + (Cindex * 8 )
                          190 Rem Read/Store 8 bytes
                          200   For Loop = Char to Char + 7
                          210   Read Cdata: Poke Loop, Cdata
                          220   Next Loop
                          230 Goto 140: Rem Read next index
                          240 Rem Program Continues Here...

rom                num == Specifies the ROM page to start at (0 to 3). For 1024
                          byte fonts, page must be 0. For 512 byte fonts page 0
                          or 2 may be used. (default 0)

pages             nums == Specifies which 256 byte page(s) of the input font to
                          output and the order they are output. The defaults
                          are "0,1,2,3" for 1024 byte fonts, and "0,1" for 512
                          byte fonts.

nopagecomment          == Do not output the page comment. (default)

pagecomment            == At the start of each page (32 character block) output
                          a descriptive comment.
                          Example for Page 3 in C format:
                          / * Page 0xE3.  Chars 96 to 127 -- lowercase * /

nocharcomment          == Do not output the character information comment.
                          (default)

charcomment            == Before each block of character bytes output the
                          character information.
                          Example for a control character in C format with Hex
                          data:
                          / * 0x7b ctrl-; * /
                          0x00,0x18,0x3c,0x7e,0x7e,0x18,0x3c,0x00,

nobitscomment          == Do not output the character bitmap comments.
                          (default)

bitscomment            == After the block of character bytes output a series of
                          lines of text representing the character bitmap as it
                          would be rendered according to the "glyphtype" value
                          using the "pixel" character values to represent the
                          font pixels, according to the "pixelsize".
                          Example shown for control character 0x7b as glyphtype
                          2 in C format for hex data:
                          / * 0x00:  . . . . . . . .  * /
                          / * 0x18:  . . . # # . . .  * /
                          / * 0x3c:  . . # # # # . .  * /
                          / * 0x7e:  . # # # # # # .  * /
                          / * 0x7e:  . # # # # # # .  * /
                          / * 0x18:  . . . # # . . .  * /
                          / * 0x3c:  . . # # # # . .  * /
                          / * 0x00:  . . . . . . . .  * /

glyphtype          num == Specify the glyph rendering type when the bitmap
                          comment is produced. The value is the corresponding
                          ANTIC text mode: 2, 3, 4. The values 5 through 7 are
                          also valid, but map to one of these other values: 5
                          is the same as 4, 6 and 7 are the same as 2.  Antic 2
                          is two-color (bitmap), and Antic 4 is four-color (bit
                          pairs).  Antic 3 is a two-color bitmap, but occupies
                          10 scan lines using 8 bytes of data by shifting the
                          first two data rows of the lowercase characters to
                          the last scan lines, and filling in 0 data for all
                          other "extra" scan lines. (default is 2)

pixelsize          num == Specifies how many characters wide a "pixel" is when
                          the bitmap comment is produced. Valid values are 1
                          through 4. (default 2)

pixelfill          num == Specifies how many times the character representing a
                          "pixel" will be written out within the pixel field
                          (specified by "pixelsize") when the bitmap comment is
                          produced.  If the size  of "pixelfill" is less than
                          "pixelsize" then the difference will be made up by
                          blank spaces. Valid values are 1 through 4. (default
                          1)

pixel0         char(s) == Specifies the character to use to represent pixel
                          value 0 when the bitmap comment is produced. Note
                          that this and all other "pixel" values can be a
                          string of multiple characters determined by
                          "pixelsize" and "pixelfill" which can be used to
                          simulate the on screen aspect ratios more accurately.
                          (default ".")

pixel1         char(s) == Specifies the character to use to represent pixel
                          value 1 when the bitmap comment is produced. (default
                          "#")

pixel2         char(s) == Specifies the character to use to represent pixel
                          value 2 when the bitmap comment is produced. (default
                          "O")

pixel3         char(s) == Specifies the character to use to represent pixel
                          value 3 when the bitmap comment is produced. (default
                          "X")

Later options will likely override earlier options on the command line, so
conflicting options will usually not result in an error. However, the "rom"
option should come before the "pages" option. As an example: If a line includes
--asm followed by --basic then the output format will be DATA statements for
BASIC.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libgen.h>

/* All the global values for command line options.  
 * This will also provide globals.h 
 */
#include "myglobals.h"  

/* The actual moving machinery for reading the data and outputting it. */
#include "io.h"

/* A standard font for comparison. */
extern char * e000_rcs_id( void ); /* report file version for e000.o */

extern unsigned char * getE000( void ); /* Get a pointer to the standard font */


#define CKP( STR ) fprintf(stderr, "Checkpoint in %s at %d: ",__FILE__,__LINE__); fprintf(stderr, STR); fputc( '\n', stderr ); fflush(NULL);

static char * rcs_id = "$Id: main.c,v 1.17 2014/04/04 18:37:10 kjenning Exp $";


static void usage( char * prgName )
{
    struct winsize wWin;
	int            conWidth = 64; /* A reasonable width needed to insure the example BASIC program in the usage() prints correctly */
	char *         prg    = NULL;
	int            retVal = 0;

	/* Yes, this officially makes the program not portable to Windoze.  Time for you to upgrade to a better OS. */
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wWin); 
 
	if ( wWin.ws_col > conWidth )
		conWidth = wWin.ws_col -1;

/*	printf ("lines %d\n", wWin.ws_row);
 *	printf ("columns %d\n", wWin.ws_col); fflush(NULL);
 */	
	if ( prgName )
		prg = basename( prgName );
	
	if ( !prg )
		prg = "atf2code";
	
	fprintf( stderr,
			 "\n%s\n\n%s\n%s\n%s\n%s\n%s\n\n",
			 prg,
 			 rcs_id,
			 global_rcs_id(),
			 myglobal_rcs_id(),
			 e000_rcs_id(),
			 io_rcs_id() );
	
	retVal = docWrap( "Dump Atari font (1024 or 512 byte) files as data statements for a " 
					  "programming language -- DATA for BASIC, a number of data directives "
					  "for assembly language, or an unsigned char array for C.\n"
					  "Data may be output as hex or decimal values.  Line numbers may or may not be included.\n",
					  conWidth, /* Line Length */
					  0,        /* Indenting, none */
					  0 );      /* N/A flag to indent first line */
					  
	if ( retVal )
		fprintf(stderr,"doDocs() (header block) failure %d\n\n", retVal );
			 
	fprintf( stderr, 
			 "Usage:\n\n%s [options]\n\n",
			 prg );

	retVal = doDocs( conWidth );
			
	if ( retVal )
		fprintf(stderr,"doDocs() failure %d\n\n", retVal );

	retVal = docWrap( "Later options will likely override earlier "
					  "options on the command line, so conflicting options "
					  "will usually not result in an error. "
					  "However, the \"rom\" option should come before the \"pages\" option. "
					  "As an example: If a line includes --asm followed by --basic then the "
					  "output format will be DATA statements for BASIC.\n",
					  conWidth, /* Line Length */
					  0,       /* Indenting, none */
					  0 );     /* N/A flag to indent first line */

	if ( retVal )
		fprintf(stderr,"doDocs() (trailer block) failure %d\n\n", retVal );

	fflush( NULL );
}


 
int main( int argc, char ** argv )
{
	if ( doArgs( argc, argv, get_GA() ) ) /* Over-engineered command line parser */
	{	
		usage( argv[0] );
		return -1;
	}

	if ( readFile() ) /* Read a file, or get data from stdio */
	{	
		usage( argv[0] );
		return -1;
	}
 
	if ( sanityCheck() ) /* Verify a few command line options agree */
	{	
		usage( argv[0] );
		return -1;
	}

	dumpMyArgs();	/* Does nothing if debug is not turned on */

	if ( outputData() ) /* Dump the output data */
		return -1;

	return 0;
}


