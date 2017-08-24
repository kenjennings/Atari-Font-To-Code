/* Dump binary data as program text either for BASIC or assembly or C or text dump
 */

/* $Id: globals.h,v 1.4 2014/04/08 17:28:43 kjenning Exp $
 *
 * $Log: globals.h,v $
 * Revision 1.4  2014/04/08 17:28:43  kjenning
 * Updated for use in Binary 2 Code program.
 *
 * Revision 1.3  2014/03/26 20:44:11  kjenning
 * Slight cleanup of bloaty cruftiness.   A little modularity
 * inflicted on the I/O section.  Probably fixed the C
 * output bug that resulted in a trailing comma at the end
 * of data when character comments were used.
 *
 * Revision 1.2  2014/03/13 20:41:56  kjenning
 * Interim check-in after myglobals and documentation
 * text formatting were revised and tested.   Still
 * more features to add per the configs.
 *
 * Revision 1.1  2013/11/15 21:15:40  kjenning
 * Initial revision
 *
 */

#ifndef _GLOBALS_INCL_
#define _GLOBALS_INCL_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>


char * global_rcs_id( void );

/* Stupid simple and yet irresponsibly over-engineered 
 * command line arguments controller.
 * DISCLAIMER:
 * The GA_STRING type uses strdup() for the global 
 * value which isn't disposed of properly later, so 
 * this causes valgrind to complain about allocated
 * memory that is not freed.  
 * In this lazy version, the main (or any other) code 
 * could also force (re)definition of a GA_STRING value, 
 * so it's not actually possible to determine whether 
 * or not a string value was dynamically allocated.
 */
typedef enum _GA_TYPE_
{
	GA_INT = 1,  /* Generically process one following argument as a number */
	GA_CHAR,     /* Generically process one character from following argument */
	GA_STRING,   /* Generically process one following argument as string */
	GA_IFLAG,    /* Generic flag (int) */
	GA_CFLAG,    /* Generic flag (char) */
	GA_OTHER     /* Custom-ish rule for processing (and optional argument) */
} GA_TYPE;

/* Document the command for usage and help.
 * The docs are just an array of character strings.  
 * The usage/help function will align text and apply line 
 * breaks as needed to make the text fit the desired line
 * length.  
 * Entry 0 is the short description of the arguments.  
 * If there are no arguments an empty string ( "" ) is required.
 * Entry 1 and later is the text of the documentation
 * up to the entry containing NULL.  Each entry is processed 
 * individually, so for the purposes of line-wrap the start of 
 * entry 3 is not merged with the end of entry 2. 
 *
 * An Example of command docs where the GlobalArg longname is 
 * "file" and the GA_DOC entry 0 for the arguments is "filename"
 * and the requested total line length is 52 characters wide:
 *
 * file filename = The input file to read. Providing 
 *                 filename 'stdin' will use stdio 
 *                 instead.
 *
 * The command name and arguments list appear to the left, separated
 * by an equals, and then the documentation is aligned to the right 
 * of that and space-padded for alignment under that block. 
 * The command names and arguments will be scanned in the first pass
 * of the docs to determine the maximum length of the command plus
 * the arguments, and all documentation will be aligned within the 
 * same space defined by the LONGEST command/arguments length.
 *
 * Newline ('\n') may be embedded in the string and will
 * be recognized as a forced new line. A newline will
 * always be output at the end of each entry (string).
 * Blank spaces ( i.e. isspace() ) occurring at the start  
 * of the line and the end of the line will be eaten.
 * "End of the line" also includes where the function decides
 * to break the text, so prepare for disappointment if 
 * documentation includes huge ASCII art monuments.
 * Embedded tabs do not get any special consideration  
 * meaning isspace() could count them as edible, so don't 
 * be goofy here, either.
 * Also, do not suggest a silly line length for output 
 * such as 5 for the docs.  The code will assume such a value 
 * was intended as a joke and use 39 as a minimum.
 * If a command is intended to have no documentation, then 
 * use NULL for the GA_DOC value and it will be quietly skipped.
 */ 
typedef char ** GA_DOC;

/* A GlobalArg using type == GA_OTHER is expected to supply a custom
 * func to assign and manage the value. 
 * In doing this, it should (or "may") make moot these values values 
 * in the GlobalArg: iMin, iMax, ga, gaDefault.
 *
 * Also, where a group of GlobalArgs manage the same global variable
 * (such as a group of parameters that set the same flag, or a yes/no 
 * toggle) the GlobalArg holding the default to be used when none
 * of the options has been set is expected to be declared FIRST.
 * That first GlobalArg should use a NULL gaDefault while the 
 * subsequent entries should use the address of the First GlobalArg
 * as the gaDefault value.
 */
typedef struct _GlobalArg_ 
{
	char               * longName;    /* (config)        Longer, verbose name */ 
	GA_DOC               docs;        /* (config)        Text for help/usage */
	int                  parm;        /* (config)        0 means no argument.  1 means argument */
	char               * cmdArg;      /* (input)         the command line argument value, if parm says collect it */
	GA_TYPE              type;        /* (input)         type of value expected */
	int                  iMin;        /* (input)         minimum for integer */
	int                  iMax;        /* (input)         maximum for integer */
	void               * ga;          /* (input)         address of int or char (or array) of global variable */
	int                  isSet;       /* (config/in-out) init to 0.  Set when parsing to indicate value is set. Set to 1 for non-default values.*/
	char               * ifNotSet;    /* (config)        Establish default value if not set */
	struct _GlobalArg_ * gaDefault;   /* (config)        In a group of flags, the GlobalArg that holds the default (NULL == self) */
    int (*func)(struct _GlobalArg_ *, /* (config)        Function to assign the value. */ 
				char *, 
				char *); 
} GlobalArg ;


int dumpArgs( void ); /* Debugging.  Output the GlobalArgs array. */

int docWrap( char * doc,             /* Output word-wrapped text from doc. */
			 int    docWidth,        /* Max line length including indenting. */
			 int    indentLen,       /* Number of characters to indent */
			 int    doFirstIndent ); /* 1=indent first line, 0=do not indent first line. */
			 
int doDocs( int docWidth ); /* Usage/documentation.  Output from the GlobalArgs' array's docs entry */


/* The entry point for processing command line arguments.
 * Simple, easy to confuse.  Tolerates all manner of abuse.
 * Any number of '-' may precede the parameter name.
 */
int doArgs( int argc, char ** argv, GlobalArg ** ga );

#endif /* _GLOBALS_INCL_ */
