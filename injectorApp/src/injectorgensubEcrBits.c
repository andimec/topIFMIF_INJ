#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <dbEvent.h>
#include <dbDefs.h>
#include <dbCommon.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <recSup.h>
#include <aSubRecord.h>
#include <pinfo.h>

/*-------------------------------------------------------------*/
/*    ecriture Nbits pour Modbus Phoenix                       */
/*-------------------------------------------------------------*/


long writeNBits( aSubRecord *pgsub )
{
  
  long      tab[15];
  long *ptr;

/* Transmit N bits (N=2) through output link A */
  ptr =( long*)pgsub->a;
  tab[0] = *(ptr) ;
  ptr =( long*)pgsub->b; 
  tab[1] = *(ptr) ;
  
  memcpy( pgsub->vala, tab, 2*sizeof(long) );
  return(0);

}



/* Register these symbols for use by IOC code */

epicsRegisterFunction( writeNBits   );

