program emitSnlMotor

/******************************/
/* ***  Process Variables *** */
/******************************/

/* MAXV (moteur) : commandes = Enable (1) / Disable (0) */
string cmdMaxv;
assign cmdMaxv to "LEBT:EMIT:MOTOR.CNEN";
monitor cmdMaxv;

/* Interlock : ok = ON */
string extSec;
assign extSec to "LEBT:EMIT:ExtSec";
monitor extSec;

/* servo-moteur */
short cmdServo;
assign cmdServo to "LEBT:EMIT_SERV:EnaDisa";
monitor cmdServo;
short lectServo;
assign lectServo to "LEBT:EMIT_SERV:Refresh";
monitor lectServo;
/* double statusServo;
assign statusServo to "LEBT:EMIT_SERV:Status";
monitor statusServo; */

/* actions snl */
short actEnable;
assign actEnable to "LEBT:EMIT:Actenable";
monitor actEnable;

short actDisable;
assign actDisable to "LEBT:EMIT:Actdisable";
monitor actDisable;

%%#include <stdio.h>
%%#include <string.h>
%%#include <math.h>
%%#include <stdlib.h>
%%#include <time.h>
%%#include <unistd.h>
%%#include <stdlib.h>

ss enableDisable {

  state connectMotor {
    when ( delay(2.0) && ( pvConnectCount() == pvChannelCount() ) ) {
      printf("\n");
      printf("snl motor : connexions PVs etablies\n");
    } state initMotor
  }

  state initMotor {
    when ( delay(1.0) ) {
    } state wait
  }  /* init */

  state wait {
    when ( actEnable == 1 ) {
      printf("\n");
      printf("snl motor : enable maxv+servo received \n");
    } state enableMaxv
    when ( actDisable == 1 ) {
      printf("\n");
      printf("snl motor : disable servo+maxv received \n");
    } state disableServo
  }  /* wait */

  state enableMaxv {
    when ( delay(1.0) ) {
      if ( strcmp(cmdMaxv,"Disable") == 0 ) {
        /* cmdMaxv="Enable"; pvPut(cmdMaxv); */
        strcpy(cmdMaxv,"Enable"); pvPut(cmdMaxv);
        printf("snl motor : enable maxv sent \n");
      }
    } state enableServo
  }  /* enableMaxv */
  state enableServo {
    when ( delay(1.0) ) {
        cmdServo=1;    pvPut(cmdServo);
        printf("snl motor : enable servo sent \n");
   } state statusServo
  }  /* enableServo */
  state statusServo {
    when ( delay(1.0) ) {
      lectServo=1; pvPut(lectServo);
      actEnable=0; pvPut(actEnable);
   } state wait
  }  /* statusServo */

  state disableServo {
    when ( delay(1.0) ) {
      cmdServo=0; pvPut(cmdServo);
      printf("snl motor : disable servo sent \n");
    } state stsServo
  }  /* disableServo */
  state stsServo {
    when ( delay(1.0) ) {
      lectServo=1; pvPut(lectServo);
   } state disableMaxv
  }  /* stsServo */
  state disableMaxv {
    when ( delay(1.0) ) {
      if ( strcmp(cmdMaxv,"Enable") == 0 ) {
        /* cmdMaxv="Disable"; pvPut(cmdMaxv); */
        strcpy(cmdMaxv,"Disable"); pvPut(cmdMaxv);
        printf("snl motor : disable maxv sent \n");
      }
      actDisable=0; pvPut(actDisable);
    } state wait
  }  /* disableMaxv */


} /* ss enableDisable */
