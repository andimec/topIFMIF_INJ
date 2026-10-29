program emitSnlAlims

short PS1;
assign PS1 to "LEBT:EMIT_PS1:OnOffCmd.VAL";
monitor PS1;
short PS2;
assign PS2 to "LEBT:EMIT_PS2:OnOffCmd.VAL";
monitor PS2;

float ODLY1;
assign ODLY1 to "LEBT:EMIT_PS1:Calcdly.ODLY";
monitor ODLY1;
float ODLY2;
assign ODLY2 to "LEBT:EMIT_PS2:Calcdly.ODLY";
monitor ODLY2;


%%#include <stdio.h>
%%#include <string.h>
%%#include <math.h>
%%#include <stdlib.h>
%%#include <time.h>
%%#include <unistd.h>
%%#include <stdlib.h>


ss alims
{
  state connectAlims {
    when ( delay(2.0) && ( pvConnectCount() == pvChannelCount() ) ) {
      printf("\n");
      printf("snl Alim : connexions PVs etablies\n");
     } state survAlims
  }
  
  state survAlims {
    when ( delay(2.0)  ) {
      /* emuPS1 */
      if ( PS1 == 1 ) { ODLY1=0.; pvPut(ODLY1); } else { ODLY1=30.; pvPut(ODLY1); }
      /* emuPS2 */
      if ( PS2 == 1 ) { ODLY2=0.; pvPut(ODLY2); } else { ODLY2=30.; pvPut(ODLY2);}
    } state survAlims
  }

}
