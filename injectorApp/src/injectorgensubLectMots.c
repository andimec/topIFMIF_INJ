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
/*     Interface lecture pour modbus IOCASTE                                  */
/*     traitement mots après lecture par un waveform                          */
/*----------------------------------------------------------------------------*/

long readNMots( aSubRecord *pgsub )
{
/*  double val_Double;
  long val_Long;
  float val_Float;*/

  unsigned short tab[10];
  long L1;
  long L2;

union
{
  float Rval;
  unsigned short Valeur_float[2];
}valeur_a_envoyer;

  long *ptr;   /*  definir pointeur */

/*------------------------------------------------------*/
/* Input A = tableau de 2 mots de 32 bits (long)        */
/* les mots sont assemblés par 2 pour former des float  */
/*------------------------------------------------------*/

  ptr =(long*)pgsub->a;

 L1 = *(ptr);
 L2 = *(ptr+1);
/*  L2 contient les Poids Forts */

/*printf(" Long_1 :%u \n", L1);
printf(" Long_2 :%u \n", L2);*/




         valeur_a_envoyer.Valeur_float[0]= (unsigned short)L1;
         valeur_a_envoyer.Valeur_float[1]= (unsigned short)L2;

/*printf(" valeur_1 :%u \n", valeur_a_envoyer.Valeur_float[0]);
printf(" valeur_2 :%u \n", valeur_a_envoyer.Valeur_float[1]);*/


/* Inversion PF--pf */

  tab[0]=valeur_a_envoyer.Valeur_float[1];  
  tab[1]=valeur_a_envoyer.Valeur_float[0];

/*printf(" Tab_1 :%u \n",tab[0] );
printf(" Tab_2 :%u \n",tab[1] );*/

/*------------------------------------------------------*/
/* Copie de tab vers la zone pointee par pgsub          */
/*                                                      */
/*------------------------------------------------------*/
  memcpy( pgsub->vala,tab , 2*sizeof(unsigned short) );

  /*   pgsub->vala = valeur_a_envoyer.Rval;*/

  return(0);

}



/* Register these symbols for use by IOC code */

epicsRegisterFunction( readNMots   );

