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
/*----------------------------------------------------------------------------*/
/*     Interface ecriture pour modbus IOCASTE                                 */
/*     Ecriture consigne ( long 32 bits) sous forme de 2 mots de 16 bits      */
/*----------------------------------------------------------------------------*/

long writeNMots( aSubRecord *pgsub )
{
  double val_Double;
  long val_Long;
  float val_Float;
  unsigned short tab[10];

union
{
  float Rval;
/*  unsigned short consigne[2];     -> Ported to Linux: 29-May-2017 */
  unsigned short int consigne[2];
}valeur_a_envoyer;

  double *ptr;   /*  definir pointeur */

/* Input A = valeur sur 64 bits (double) */


  ptr =( double*)pgsub->a;

  val_Double = *(ptr);
 /*  val_Long = (long)val_Double;*/
  val_Float = (float)val_Double;
  valeur_a_envoyer.Rval = val_Float;
/*-----------------------------------------------------*/
/*  modif du 15 avril 2010 DB inversion PF-pf          */
/*-----------------------------------------------------*/
/*  tab[0]=valeur_a_envoyer.consigne[0];*/  
/*  tab[1]=valeur_a_envoyer.consigne[1];*/

  tab[0]=valeur_a_envoyer.consigne[1];  
  tab[1]=valeur_a_envoyer.consigne[0];

              printf("NEW_GENSUB  :fin programme \n");

/*  memcpy( pgsub->vala,tab , 2*sizeof(unsigned short) );
Ported to Linux: 29-May-2017 */
    memcpy( pgsub->vala,tab , 2*sizeof(unsigned short int) );
  tab[0]=1;  
/*  memcpy( pgsub->valb,tab , 1*sizeof(unsigned short) );
Ported to Linux: 29-May-2017 */
  memcpy( pgsub->valb,tab , 1*sizeof(unsigned short int) );
  return(0);
              
}



/* Register these symbols for use by IOC code */

epicsRegisterFunction( writeNMots   );

