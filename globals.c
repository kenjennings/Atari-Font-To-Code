/* Dump binary data as program text either for BASIC or assembly or C or text dump
 */


/* $Id: globals.c,v 1.13 2014/04/08 17:28:43 kjenning Exp $
 *
 * $Log: globals.c,v $
 * Revision 1.13  2014/04/08 17:28:43  kjenning
 * Updated for use in Binary 2 Code program.
 *
 * Revision 1.12  2014/03/31 20:53:36  kjenning
 * Added Assembly origin command line argument.
 * Cleaned up assemby output.
 * Probably made the code even cruftier.
 * Rearranged some of the help text.
 *
 * Revision 1.11  2014/03/26 20:44:11  kjenning
 * Slight cleanup of bloaty cruftiness.   A little modularity
 * inflicted on the I/O section.  Probably fixed the C
 * output bug that resulted in a trailing comma at the end
 * of data when character comments were used.
 *
 * Revision 1.10  2014/03/18 19:47:11  kjenning
 * Bells and Whistles implemented and some sanity
 * testing completed.  This seems to be working
 * well enough, so here's a check-in baseline.
 *
 * Revision 1.9  2014/03/13 20:41:56  kjenning
 * Interim check-in after myglobals and documentation
 * text formatting were revised and tested.   Still
 * more features to add per the configs.
 *
 * Revision 1.8  2013/11/15 21:15:40  kjenning
 * Added options for commented output.  No supporting code.
 * This is a temporary check-in with no build verification.
 * Likely does not build at all.
 *
 * Revision 1.7  2013/11/07 21:38:45  kjenning
 * Added command line controls for outputting
 * decorative comments with the data.  The
 * actual code to do the decorating is not
 * included here. Wait for a future version.
 *
 * Revision 1.6  2013/11/01 21:05:15  kjenning
 * Cut out the redundant, per-value function parser replacing
 * most of them with a generic argument/value parser.
 *
 * Revision 1.5  2013/11/01 20:50:32  kjenning
 * Check-In after buildable, untested globals overhaul
 * prior to gutting the old, bloaty, redundant parsing
 * functions in globals.c.
 *
 * Revision 1.4  2013/10/21 20:48:50  kjenning
 * Temporary Checkin.
 * Most functionality had been working.
 * Next unecessary revision of the globals due
 * to begin after this version.
 *
 * Revision 1.3  2013/10/17 20:36:08  kjenning
 * Check-In after modular-ity-izing some parts of code.
 * Several options still not working properly.
 *
 * Revision 1.2  2013/10/10 20:52:38  kjenning
 * Stupid compiling issues resolved.
 * Currently appears to be functioning.
 * TO DO: Fix behavior of diff argument.
 * TO DO: separate I/O parts from globals.
 *
 * Revision 1.1  2013/10/09 17:54:52  kjenning
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
#include <errno.h>
#include <limits.h>

#ifndef _GLOBALS_INCL_
#include "globals.h"
#endif

#define CKP( STR ) fprintf(stderr, "Checkpoint in %s at %d: ",__FILE__,__LINE__); fprintf(stderr, STR); fputc( '\n', stderr ); fflush(NULL);


static char * rcs_id = "$Id: globals.c,v 1.13 2014/04/08 17:28:43 kjenning Exp $";

char * global_rcs_id( void ) /* report file version for globals.o */
{
	return rcs_id;
}

static GlobalArg ** GlobalArgs = NULL;

static int gaDump( GlobalArg * ga )
{
	if ( !ga )
		return -1;
		
	fprintf(stderr," gaDump: GA           * = %p\n", (void *)ga ); 
	fprintf(stderr," gaDump: GA.longName  * = %p\n", (void *)ga->longName ); 
	fprintf(stderr," gaDump: GA.longName    = %s\n", ga->longName ? ga->longName : "NULL" ); 
	fprintf(stderr," gaDump: GA.docs      * = %p\n", (void *)ga->docs ); 
	fprintf(stderr," gaDump: GA.parm        = %d\n", ga->parm ); 
	fprintf(stderr," gaDump: GA.cmdArg    * = %p\n", (void *)ga->cmdArg ); 
	fprintf(stderr," gaDump: GA.cmdArg      = %s\n", ga->cmdArg ? ga->cmdArg : "NULL" ); 
	fprintf(stderr," gaDump: GA.type        = %d\n", ga->type ); 
	fprintf(stderr," gaDump: GA.iMin        = %d\n", ga->iMin ); 
	fprintf(stderr," gaDump: GA.iMax        = %d\n", ga->iMax ); 
	fprintf(stderr," gaDump: GA.ga        * = %p\n", (void *)ga->ga ); 
	fprintf(stderr," gaDump: GA.isSet       = %d\n", ga->isSet ); 
	fprintf(stderr," gaDump: GA.ifNotSet  * = %p\n", (void *)ga->ifNotSet ); 
	fprintf(stderr," gaDump: GA.ifNotSet    = %s\n", ga->ifNotSet ? ga->ifNotSet : "NULL" );
	fprintf(stderr," gaDump: GA.gaDefault * = %p\n", (void *)ga->gaDefault );
	fprintf(stderr," gaDump: GA.func      * = %p\n", (void *)ga->func );
	fprintf(stderr," ========================\n" );
	fflush(NULL);
	return 0;
}	


int dumpArgs( void ) /* Debugging.  Output the GlobalArgs array. */
{
	int loop = 0;
		
	if ( !GlobalArgs )
		return -1;
		
	while ( GlobalArgs[ loop ] )
		gaDump( GlobalArgs[ loop++ ] );

	return 0;
}



/* Output text word-wrapped to a specified length.
 * (optional) indenting to a specified position.
 * (optionally) suppress indenting on first line.
 */ 
int docWrap( char * doc,
			 int    docWidth,       /* Max line length including indenting */
			 int    indentLen,      /* Number of characters to indent */
			 int    doFirstIndent ) /* 1=indent first line, 0=do not indent first line. */
{
	char * lStart     = NULL; /* Start of the line.  */
	char * lEnd       = NULL; /* End of line */
	char * lSpace     = NULL; /* most recently encountered while space on the line */
	char   cEnd       = '\0'; /* The character at the last position of the line */
	char   cNext      = '\0'; /* The character at the first position of the next line */
	int    firstLine  = 1 ;
	
	if ( !doc )
		return -1;
	
	if ( docWidth < 39 )
		docWidth = 39;
	
	if ( ( indentLen < 0 ) || ( indentLen > ( docWidth - 10 ) ) )
		return -2;
	
	lStart = doc;
	lEnd   = NULL;
					
	while ( *lStart ) /* Loop until we run out of text */
	{
		/* scenario... At the start of parsing an EOL occurs immediately.... 
		 * The goal here is to insure we aren't dumping insane amounts of 
		 * new lines in the output.  If multiple new lines are intended then
		 * the routine can be gamed by inserting a space between each 
		 * new line character.  "text\n \n \nMore text" 
		 */
		if ( *lStart == '\n')
		{
			/* if this EOL happens at the first doc text text before any 
			* other text, then suppress this badly conceived foolishness.
			*/
			if ( firstLine ) /* Eat the EOL.  Continue.  And yes, stay on firstLine. */
			{
				lStart++;
				continue;
			}

			/* if there was previous text output that did NOT end with the EOL, 
			 * then this EOL is a victim of word break and was intended to 
			 * end the previously output text, and since the code has already
			 * output EOL for that line this means the EOL we have now can
			 * be ignored.
			 */
			if ( lEnd && ( *lEnd != '\n' ) )
			{
				lStart++;
				continue;
			}
					
			/* Other possibilities have been disposed, so this means 
			 * the current EOL is intentional.
			 */
			fputc( '\n', stderr );
			lStart++;
			firstLine = 0;
			continue;
		}
				
		/* Embedded EOLs are done... So, here eat up any leading white  
		 * space.  The EOLs were checked separately, since EOL is 
		 * counted as white space by isspace() 
		 */
		if ( isspace( *lStart ) )
		{
			lStart++;
			continue;
		}
			
		/* All the leading line trash is done, so this must be a real 
		 * character (hopefully followed by more characters which we
		 * humans commonly refer to as "text".)
		 */
	
		lEnd   = lStart;
		lSpace = NULL;
		cEnd   = *lEnd;
		cNext  = '\0';
		
		/* Walk forward through the string until:
		 * the character encountered in the string terminator, or
		 * we encounter the EOL character, or
		 * the number of characters walked through reaches the size of the line.
		 * Note that we intentionally walk one too far to determine if a word
		 * happens to cross over the end of line.
		 * While walking, remember the last white space position.
		 */
		while ( *lEnd && ( ( lEnd - lStart + 1) <= ( docWidth - indentLen ) )  && ( *lEnd != '\n' ) )
		{
			if ( *lEnd && isspace( *lEnd ) )
				lSpace = lEnd;
						
			cEnd   = *lEnd;	/* Remember the current char */
			lEnd++;
			cNext  = *lEnd; /* Remember the next +1 position */
		}
				
		 /* If we walked past the limit then this is a long text segment.  Test scenarios: */

		if ( ( lEnd - lStart ) == ( docWidth - indentLen ) )
		{
			/* Is the last character of the current line or the first character 
			 * of the next line the string terminator or white space?
			 * If so, then backup one character to the end of line, and 
			 * this line can be output.
			 */
			if ( ( !cEnd  || ( isspace( cEnd  ) && ( cEnd != '\n' ) ) ) ||
			     ( !cNext || isspace( cNext ) ) )
				lEnd--;
			else 
			{
				/* Having failed that test, it means a word crosses the end of line.
				 * If a recent white space was identified, then move the end of 
				 * line back to that prior white space.
				 * If no prior whitespace was identified, then just backup one place 
				 * to set the end of the line.
				 */
				if ( lSpace && ( lSpace < lEnd ) )
					lEnd = lSpace;
				else
					lEnd--;
			}
		}
			
		/* If this is not the first line, then do the indenting as needed. */

		if ( ( indentLen > 0 ) &&  
		     ( ( firstLine && doFirstIndent ) || !firstLine) )
			fprintf(stderr, "%*s", indentLen, " ");
				
		firstLine = 0;
				
		/* Output the text. */
		
		while ( lStart <= lEnd )
		{
			if ( *lStart )
				fputc( *lStart, stderr );
						
			lStart++;
		}
				
		/* If the text did not end at the EOL, then output one */
				
		if ( *lEnd != '\n' ) 
			fputc( '\n', stderr );
			
		/* That's the end of that line of text... */ 
		lStart = lEnd;
				
		/* If this isn't yet the end of the string, then start at the next character */
		if ( *lEnd )
			lStart++;
	}					
			
	fputc( '\n', stderr );

		return 0;
}
			 

/* Output the Usage/documentation.  This is data from the GA_DOC 
   structure which is really just a char ** array.  The first 
   entry is a short description of the arguments which should be
   an empty string if it is unused. The remaining array entries
   are text which is parsed and displayed if present.  
   NULL terminate the array.  
   An example "doc" output looks like this:
   12345678901234567890123456789012345678901234567890 
   command argument(s) == Text describing the feature 
                          appears here indented to 
						  keep alignment and word
						  wrapped at the line width 
						  specified by docWidth.
	com2        arg(s) == This is a second option for
                          purposes of illustration.	
	The arguments text appears right aligned.
	So, this requires knowing the maximum length of commands,
	plus one space, plus the maximum length of the argument(s)
	text (or doc[0]) plus one space, plus "==", plus one space.
	That complete length is the indent level for all descriptive
	text not on the same line as the command.
	If the last character in the doc[] entry is not the usual 
	end of line, then the code will automatically add one.
	So, logically, each doc[] entry is considered a paragraph.
*/
int doDocs( int docWidth ) /* Usage/documentation.  Output from the GlobalArgs' array's docs entry */
{
	int    loop       = 0;
	int    maxNameLen = 0;    /* Command names */
	int    maxArgLen  = 0;    /* Argument texts */
	int    temp       = 0; 
	int    indentLen  = 0;    /* Length of command, arg, and formatting */ 
	int    docIndex   = 1;    /* walk through doc[] array.  entry 1 is where text begins. */
	int    safeToUse  = 0;    /* Flag indicating enough information is present to use the docs[1] index. */
	char * doc        = NULL;

	
	if ( !GlobalArgs )
		return -1;

	if ( docWidth < 39 ) /* comedians are discouraged. */
		docWidth = 39;

	/* First loop...  Get max length of longname, and docs[0] */

	while ( GlobalArgs[ loop ] )
	{
		if ( GlobalArgs[ loop ]->longName ) /* Collecting the longest command name */
		{
			temp = strlen( GlobalArgs[ loop ]->longName );
			
			if ( maxNameLen < temp )
				maxNameLen = temp;
		}
		
		if ( GlobalArgs[ loop ]->docs && GlobalArgs[ loop ]->docs[0] ) /* Collecting the longest argument text */
		{
			temp = strlen( GlobalArgs[ loop ]->docs[0] );
			
			if ( maxArgLen < temp )
				maxArgLen = temp;
		}

		loop++;
	}
	
	if ( !maxNameLen ) /* There must be at least one command name */
		return -2;

	/* +5 is the space between command and arg, plus this: " == " after arg. */		
	indentLen = maxNameLen + maxArgLen + 5; 

	if ( ( indentLen >= 39) || 
	     ( indentLen > (docWidth - 10 ) ) ) /* whuddaryou crazy? Unreasonable. */
	{
		fprintf( stderr, 
				 "Error: command and arguments are too long (length %d).\n", 
				 indentLen );
		return -3;
	}	

	loop = 0;	

	while ( GlobalArgs[ loop ] )
	{
		safeToUse = ( GlobalArgs[ loop ]->docs && GlobalArgs[ loop ]->docs[0] ) ;
	
		/* If the array is NULL, we don't want to go there.
		   If the first index is NULL, we can't go any farther.
		   If they are populated, then we should be safe to 
		   at least test the second index...
		 */
		if ( safeToUse )
			fprintf(stderr,
					"%-*s %*s == ", /* command...  ...arg " */
					maxNameLen,
					GlobalArgs[ loop ]->longName,
					maxArgLen,
					safeToUse ?  GlobalArgs[ loop ]->docs[0] : " " );
		else /* short cutting here. */
		{
			loop++;		
			continue;
		}
					
		docIndex  = 1; /* First possible doc text */

		while ( ( doc = GlobalArgs[ loop ]->docs[docIndex] )  )
		{
			/* If this is the first line of doc[] text, do not 
			 * indent the first line output, since the text 
			 * is already indented by the command and args.
			 */
			temp = docWrap( doc, 
							docWidth,  
							indentLen, 
							( docIndex == 1 ) ? 0 : 1 );
			
			if ( temp )
				fprintf(stderr,
						"Error: docWrap() failed. %d\n",
						temp );

			docIndex++;
		} /* Let's do the next doc[] */	
		
		loop++; 	
	}	/* Let's do the next Loop on GA args */
	
	return 0;
}


/* Given a string, convert to an integer.
 * Hex values leading with "0x" are supported.
 * Hex values leading with '$' are supported.
 * The resulting value is truncated to an int.
 * Range checking by the user-specific portion 
 * can limit this later to a 16-bit integer. 
 */
static int text2Int(char * s)
{
    unsigned long k   = 0;
	int           hex = 0;
	
    if (!s || !*s)
        return 0;

    if (!strncmp("0x", s, 2) ||
        !strncmp("0X", s, 2) )
	{
		hex++;
		s+=2;
	}
	else if ( *s == '$')
	{
		hex++;
		s++;
	}
	
	if ( hex )
        k = strtoul(s, NULL, 16);
    else
        k = strtoul(s, NULL, 10);

    if ( (errno == ERANGE)   &&
         (k     == ULONG_MAX) )
        return 0;
	
    return (int)k;
}



static int GA_GenericArg( GlobalArg * arg,    /* This matching arg */
						  char      * name,   /* The actual option text matched */
						  char      * value ) /* The actual argument */
{
	int  *  tempInt;
	char *  tempChar;
	char ** tempArg;
	int     retVal;

	if ( !arg || !name || !value || !arg->ga )
		return -1;

	/* If this object has it's own shmancy, fancy value 
	 * parser then call it instead.
	 */
	if ( arg->func && ( arg->func != GA_GenericArg ) )
	{
		retVal = (arg->func)( arg, name, value );
		return retVal;
	}

	tempInt  =   (int *)(arg->ga);
	tempChar =  (char *)(arg->ga);
	tempArg  = (char **)(arg->ga);
	
	switch ( arg->type )
	{
		case GA_INT:
			if ( !tempInt )
				return -1;
				
			*tempInt = text2Int( value );
			
			if ( ( *tempInt < arg->iMin ) || ( *tempInt > arg->iMax ) )
				return -1;

			break;

		case GA_CHAR:
			if ( !tempChar )
				return -1;
				
			*tempChar = *value;
			
			if ( !*tempChar )
				return -1;
			
			break;

		case GA_STRING:
			tempChar = strdup( value ); /* Yes, this means string values "bleed" memory flagged by valgrind at exit */
/* CKP("GA_STRING");
 * fprintf(stderr,"1) value %s, tempChar %s, tempArg %p, arg->ga %p\n", value ? value : "NULL", tempChar ? tempChar : "NULL", tempArg, arg->ga );fflush(NULL);			
 */
			if ( !tempChar )
				return -1;
			
			*tempArg = tempChar;
/* fprintf(stderr,"1) value %s, tempChar %s, tempArg %p, arg->ga %p\n", value ? value : "NULL", tempChar ? tempChar : "NULL", tempArg, arg->ga );fflush(NULL);
 */			
			break;

		case GA_IFLAG:
			*(int *)(arg->ga) = arg->iMin;

			break;
			
		case GA_CFLAG:
			*(char *)(arg->ga) = (char )(arg->iMin);

			break;
			
		case GA_OTHER: /* Should never get here. the custom function should be different */
	
		default:       /* Should never reach this point either */
			return -1;
			break;
	}

	arg->isSet = 1; /* Made it this far.  We must have a valid option/argument set */
	
	if ( arg->gaDefault )          /* If this entry is not the default */
		arg->gaDefault->isSet = 1; /* then set the default linked to this entry */
			
	return 0;
}



/* The entry point for processing command line arguments.
 * Simple, easy to confuse.  Tolerates all manner of abuse.
 * Any number of '-' can precede the parameter name.
 */
int doArgs( int argc, char ** argv , GlobalArg ** ga )
{
	char * thisArg  = NULL;
	char * thisParm = NULL;
	int    thisArgc = 0;
	int    argLoop  = 0;
	int    retVal   = 0;
	int    matched  = 0;
	
	if ( !ga )
	{
		fprintf(stderr, "config: GlobalArg ** is NULL?\n");
		return -1;
	}
	
	GlobalArgs = ga;
		
	thisArgc = 1;
	
	while ( ( argc > thisArgc ) && !retVal)
	{
		thisArg = argv[thisArgc] ;

		while ( *thisArg == '-' ) /* skip over leading minus */
			thisArg++;
			
		argLoop = 0;
		
		while ( GlobalArgs[ argLoop ] && !retVal )
		{
			matched = !strncmp( thisArg, 
								GlobalArgs[ argLoop ]->longName,
								strlen( GlobalArgs[ argLoop ]->longName ) );
			
			if ( matched )
			{
/*CKP( "matched" );
fprintf(stderr,"thisArg = %s\n",thisArg );fflush(NULL);
*/
				if ( GlobalArgs[ argLoop ]->parm ) /* collect a parameter */
				{
					thisArgc++;

					if ( argc <= thisArgc ) /* no parameters left */
					{
						fprintf( stderr,
								 "Error: No Argument provided for command \"%s\" recognized as \"%s\"\n",
								 thisArg, 
								 GlobalArgs[ argLoop ]->longName );
						retVal = -1;
						break;
					}
					thisParm = argv[ thisArgc ]; /* here is the parameter */
				}
				else
					thisParm = ""; /* Just in case.  Use empty string */

				/* Call the command/argument function.  A bit of laziness here.*/
				/* GA_GenericArg will call func() itself if it is not NULL */ 
				retVal = GA_GenericArg( GlobalArgs[ argLoop ], 
										GlobalArgs[ argLoop ]->longName,
										thisParm );

				if ( retVal )
				{
					fprintf(stderr,
							"Error: Failed to process command \"%s\" recognized as \"%s\"\n",
							thisArg, 
							GlobalArgs[ argLoop ]->longName );
					break;		
				}
				
				break; /* No need to continue parameter looping */
			} /* If it matched a known argument */
			argLoop++;
		} /* next Argument */
		
		if ( !matched )
		{
			fprintf( stderr,
					 "Error: What is command \"%s\"?\n",
					 thisArg );
			retVal = -1;
			break;
		}
		
		thisArgc++;
	}
		
	/* If all commands/parameters are processed, then cycle through each command
	 * and set the default value if the argument has no value set
	 */
	if ( !retVal )
	{
		argLoop = 0;
		
		while ( GlobalArgs[ argLoop ] )
		{	
/* CKP( "Check If Set" );
 * fprintf(stderr, "ga->longName = %s, ga->isSet == %d\n",GlobalArgs[ argLoop ]->longName ? GlobalArgs[ argLoop ]->longName : "NULL", GlobalArgs[ argLoop ]->isSet );fflush(NULL);
*/
			if ( !GlobalArgs[ argLoop ]->isSet )
			{		
				retVal = GA_GenericArg( GlobalArgs[ argLoop ], 
										GlobalArgs[ argLoop ]->longName,
										GlobalArgs[ argLoop ]->ifNotSet );
			}

			argLoop++;
		}
	}

	return retVal;
}

