/* Dump font data as program text either for BASIC or assembly or C
 */

/* $Id: io.h,v 1.2 2014/03/13 20:41:56 kjenning Exp $
 *
 * $Log: io.h,v $
 * Revision 1.2  2014/03/13 20:41:56  kjenning
 * Interim check-in after myglobals and documentation
 * text formatting were revised and tested.   Still
 * more features to add per the configs.
 *
 * Revision 1.1  2013/11/07 21:38:45  kjenning
 * Initial revision
 *
 *
 */

#ifndef _ATF_IO_INCL_
#define _ATF_IO_INCL_

char * io_rcs_id( void );

/* Shamefully, everything is in global vars. */

int readFile( void );

int outputData( void ); 



#endif /* _ATF_IO_INCL_ */


