program emitSnlEmu_v1

/* ***  Process Variables *** */
short actionEmit;
assign actionEmit to "LEBT:EMIT:Actemit";
monitor actionEmit;
short actionScan;
assign actionScan to "LEBT:EMIT:Actscan";
monitor actionScan;
short actionUpper;
assign actionUpper to "LEBT:EMIT:Actupper";
monitor actionUpper;
short actionLower;
assign actionLower to "LEBT:EMIT:Actlower";
monitor actionLower;

short pauseEmit;
assign pauseEmit to "LEBT:EMIT:Pausemit";
monitor pauseEmit;

short saveScan;
assign saveScan to "LEBT:EMIT:Savescan";
monitor saveScan;
short saveEmit;
assign saveEmit to "LEBT:EMIT:Savemit";
monitor saveEmit;

string scanDialog;
assign scanDialog to "LEBT:EMIT:Scantext";
monitor scanDialog;
string emitDialog;
assign emitDialog to "LEBT:EMIT:Emitext";
monitor emitDialog;

double debutScan;
assign debutScan to "LEBT:EMIT:Debscanl";
monitor debutScan;
double finScan;
assign finScan to "LEBT:EMIT:Finscanl";
monitor finScan;
double pasSL;
assign pasSL to "LEBT:EMIT:Passcanl";
monitor pasSL;
double npointSL;
assign npointSL to "LEBT:EMIT:Nptscanl";
monitor npointSL;
double npointDet;
assign npointDet to "LEBT:EMIT:Nptdet";
monitor npointDet;
double limDet;
assign limDet to "LEBT:EMIT:Limdet";
monitor limDet;
double Vpic;
assign Vpic to "LEBT:EMIT:Vpic";
monitor Vpic;
double deltaV;
assign deltaV to "LEBT:EMIT:Deltav";
monitor deltaV;
double borderUp;
assign borderUp to "LEBT:EMIT:Borderup";
monitor borderUp;
double borderLow;
assign borderLow to "LEBT:EMIT:Borderlow";
monitor borderLow;
double pasAlim;
assign pasAlim to "LEBT:EMIT:Pasald";
monitor pasAlim;
double pasMoteur;
assign pasMoteur to "LEBT:EMIT:Pasmotdet";
monitor pasMoteur;
double raz=1.0;
assign raz to"LEBT:EMIT:Razgraph";
double emufic;
assign emufic to "LEBT:EMIT:Emufic";
monitor emufic;


/* --- alimentations --- */
double consHT1;
assign consHT1 to "LEBT:EMIT_PS1:VSet";
/*monitor consHT1;*/
double consHT2;
assign consHT2 to "LEBT:EMIT_PS2:VSet";
/*monitor consHT2;*/
double lectHT1;
assign lectHT1 to "LEBT:EMIT_PS1:VRdbk";
monitor lectHT1;
double lectHT2;
assign lectHT2 to "LEBT:EMIT_PS2:VRdbk";
monitor lectHT2;
double lectPSR;
assign lectPSR to "LEBT:EMIT_PSR:VRdbk";
monitor lectPSR;
double consVHT=0.;
assign consVHT to "LEBT:EMIT:Vht";
monitor consVHT;

short etatPS1;
assign etatPS1 to "LEBT:EMIT_PS1:OnOffCmd.RVAL";
monitor etatPS1;
short etatPS2;
assign etatPS2 to "LEBT:EMIT_PS2:OnOffCmd.RVAL";
monitor etatPS2;
short comPS1;
assign comPS1 to "LEBT:EMIT_PS1:Softbo";
monitor comPS1;
short comPS2;
assign comPS2 to "LEBT:EMIT_PS2:Softbo";
monitor comPS2;


/* --- faisceau + flag --- */
double iFaisc;
assign iFaisc to "LEBT:EMIT_FC:I";
monitor iFaisc;
evflag flagFaisc;
sync iFaisc flagFaisc;

double trep;
assign trep to "SRC:TIM:Trep";
monitor trep;
double tacq;
assign tacq to "LEBT:FACQ:Tdisp2";
monitor tacq;


/* --- parametres moteur --- */
double highLimit;
assign highLimit to "LEBT:EMIT:MOTOR.HLM";
monitor highLimit;
double lowLimit;
assign lowLimit to "LEBT:EMIT:MOTOR.LLM";
monitor lowLimit;
double ecrPosition;
assign ecrPosition to "LEBT:EMIT:MOTOR.VAL";
monitor ecrPosition;
double lectPosition;
// assign lectPosition to "LEBT:EMIT:MOTOR.RBV"; // Patricia commented this
assign lectPosition to "LEBT:EMIT:MOTOR_RBV";
monitor lectPosition;
string enableMoteur;  /* commandes = Enable (1) / Disable (0) */
assign enableMoteur to "LEBT:EMIT:MOTOR.CNEN";
monitor enableMoteur;
short homeR;
assign homeR to "LEBT:EMIT:MOTOR.HOMR";
monitor homeR;
short atHome;
assign atHome to "LEBT:EMIT:MOTOR.ATHM";
monitor atHome;
short hlm;         /* switch high limit */
assign hlm to "LEBT:EMIT:MOTOR.HLS";
monitor hlm;
short llm;        /* switch low limit */
assign llm to "LEBT:EMIT:MOTOR.LLS";
monitor llm;
string spmg ;  /* commandes =  Stop (0) / Pause (1) / Move (2) / Go (3) */
assign spmg to "LEBT:EMIT:MOTOR.SPMG";
monitor spmg;

/* --- parametres controle process --- */
float verifPosMoteur=1.;
float verifHT1=1.;
float verifHT2=1.;

/* --- parametres mesure Emittance --- */
double vstart;
assign vstart to "LEBT:EMIT:Vstart";
monitor vstart;
double vend;
assign vend to "LEBT:EMIT:Vend";
monitor vend;
double vstep;
assign vstep to "LEBT:EMIT:Vstep";
monitor vstep;
double nvpoints;
assign nvpoints to "LEBT:EMIT:Nptemit";
monitor nvpoints;
double upper;
assign upper to "LEBT:EMIT:Upper";
monitor upper;
double lower;
assign lower to "LEBT:EMIT:Lower";
monitor lower;
double motorstep;
assign motorstep to "LEBT:EMIT:Mstep";
monitor motorstep;
short validate;
assign validate to "LEBT:EMIT:Validate";
monitor validate;

/* --- constantes physiques Emittancemetre --- */
#define LFEP 0.01	/* L1 distance fenteEntree-plaques - en metre */
#define LP 0.09		/* L2 longueur des plaques         - en metre */
#define LFSP 0.002	/* L3 distance plaques-fenteSortie - en metre */
#define GP 0.009	/* GP distance entre les plaques   - en metre */
/* Commented by Patricia */
// double Voltage;
// assign Voltage to "LEBT:HV_PS:HTCmd";
// monitor Voltage;
double Energy;
assign Energy to "LEBT:EMIT:Energy";
monitor Energy;

%%#include <stdio.h>
%%#include <string.h>
%%#include <math.h>
%%#include <stdlib.h>
%%#include <time.h>
%%#include <unistd.h>
%%#include <stdlib.h>

%%FILE *fdscan;
%%FILE *fdemit;
%%time_t secondes;
%%char tabDate[24];
%%char *fmtDate=tabDate;
%%struct tm *ptdate;

%%char dataDir[25];
%%char fileName[23];
%%char scanFile[55];
%%char emittanceFile[55];
%%int mstep; 
%%int hvstep; 
%%int ldetect;
%%int nbpos;
int debutMesure=-1;
int n=0;
float emu=0.;
double dist;
double angle;
double startAngle=0.;
double endAngle=0.;
double stepAngle=0.;
double imax=0.;
double vpkmax=0.;
double sumVHT=0.;
int nhv=0;


%%struct timespec retard;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
ss mesEmit {
  state connectEmit {
    when ( delay(2.0) && ( pvConnectCount() == pvChannelCount() ) ) {
      printf("\n");
      printf("Emit : connexions PVs etablies\n");
      printf("Emit : attente\n");
    } state wait
  }
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  state wait {
    /* ------------------------------------------------------------------------ */
    when ( actionEmit == 1 ) {
      printf("Emit : action mesure emittance recue\n");
      printf("Emit : debut processus \n");
      if ( saveEmit == 1 ) { 
        time(&secondes);
	/* ------ darty hack: since ctime(), localtime() returns GMT although we set JST, just add +9 hours ---*/
	secondes += 9 * 60 * 60;
        fmtDate=ctime(&secondes);
        printf("\n%s",fmtDate);
        ptdate=localtime(&secondes);
        strftime(fileName,23,"emit%Y%m%d-%H%M",ptdate);
//        strcpy (dataDir, getenv("IFMIF_DATA"));
//        strcpy (dataDir, getenv("IFMIF_DATA_INJ")); // Patricia commented this
//        sprintf(dataDir,"/home/ifmif/data");
//        sprintf(emittanceFile,"%s/emit/%s.txt",dataDir,fileName); // Patricia commented this
	sprintf(emittanceFile, "/home/ifmif/data/data_INJ/emit_test/%s.txt", fileName); // Patricia added this
        printf("sauvegarde des points\n");
        printf("fichier=%s\n",emittanceFile);
        fdemit=fopen(emittanceFile, "w"); 
        if (fdemit == NULL ) printf("erreur open fic \n");
        if ( fdemit ) {
          fprintf(fdemit,"processus = Emittance Measurement vers. 1.2.1\n");
          fprintf(fdemit,"date      = %s",fmtDate);
          fprintf(fdemit,"file=%s \n",emittanceFile);
          fprintf(fdemit,"Energy  =%5.0f keV               PSR=%7.1f \n"\
                  ,Energy,lectPSR);
          fprintf(fdemit,"Vpk init=%7.0f deltaV=%7.0f HVpoints=%7.0f \n"\
                  ,Vpic,deltaV,nvpoints);
          fprintf(fdemit,"V1 start=%7.0f V1 end=%7.0f Vstep   =%7.0f \n"\
                  ,vstart/2.0,vend/2.0,vstep);
          fprintf(fdemit,"VHT from %7.0f to     %7.0f                \n"\
                  ,vstart,vend);
          fprintf(fdemit,"Upper   =%7.0f Lower =%7.0f Mstep   =%7.0f \n"\
                  ,borderUp,borderLow,motorstep);
          fprintf(fdemit,"\n");
          fprintf(fdemit,"Pos(mm) \t V1-V2(V) \t Angle(rd) \t I(ma) \n");
        }
      }
      retard.tv_nsec=100000000;  /* 100 ms */
    } state initMesEmit
    /* ------------------------------------------------------------------------ */
    when ( actionScan == 1 ) {
      printf(" \n");
      printf("scan large : action scan Large recue\n");
      if ( saveScan == 1 ) { 
        time(&secondes);
        fmtDate=ctime(&secondes);
        printf("\n%s",fmtDate);
        ptdate=localtime(&secondes);
        strftime(fileName,23,"scan%Y%m%d-%H%M",ptdate);
        //sprintf(dataDir,"/home/ifmif/data");;
        strcpy (dataDir, getenv("IFMIF_DATA"));
        sprintf(scanFile,"%s/scan/%s.txt",dataDir,fileName);
        printf("sauvegarde des points\n");
        printf("fichier=%s\n",scanFile);
        fdscan=fopen(scanFile, "w"); 
        if (fdscan == NULL ) printf("erreur open fic \n");
      }
      raz=0.;      pvPut(raz);
      printf(" \n");
      printf("scan large : debut processus \n");
      printf("debut=%8.0f V fin=%8.0f V pas=%7.0f V Npoints=%7.0f \n"\
             ,debutScan,finScan,pasSL,npointSL);
      consHT1=debutScan;  
      consHT2=-1.0*debutScan;
      printf("Position=%7.2f mm \n",lectPosition);
      printf(" \n");
      pvPut(consHT1); 
      pvPut(consHT2);
      printf("ht1 = %6.0f (%6.0f) ** ht2 = %6.0f (%6.0f) ** I = %7.0f\n"\
            ,consHT1,lectHT1,consHT2,lectHT2,iFaisc);
      raz=1.;      pvPut(raz);
      if ( saveScan == 1 && fdscan ) {
        fprintf(fdscan,"processus = Wide Scan \n");
        fprintf(fdscan,"date=%s",fmtDate);
        fprintf(fdscan,"file=%s \n",scanFile);
        fprintf(fdscan,"    emu    step   npts \n");
        fprintf(fdscan,"%7.1f %6.0f %6.0f \n"\
               ,lectPosition,pasSL,npointSL);
        fprintf(fdscan,"VHT \t I mA \n");
      }
    } state scanLarge
    /* ------------------------------------------------------------------------ */
    when ( actionUpper == 1 ) {
      printf("Detect upper : action detection recue\n");
      printf(" \n");
      printf("Detect upper : debut processus \n");
      printf("motor= %s  Pos=%7.1f mm\n",enableMoteur,lectPosition);
      verifPosMoteur=1.;
    } state initDetectUp
    /* ------------------------------------------------------------------------ */
    when ( actionLower == 1 ) {
      printf("Detect lower : action detection recue\n");
      printf(" \n");
      printf("Detect lower : debut processus \n");
      printf("motor= %s  Pos=%7.1f mm\n",enableMoteur,lectPosition);
      verifPosMoteur=1.;
    } state initDetectLow
    /* ------------------------------------------------------------------------ */
    when ( validate == 1 ) {
      printf("Emit : Action validate recue\n");
    } state actValidate

}  /* wait */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  state verifAlimsSL {
    when ( verifHT1 > 0.02 ) {
      if ( abs(consHT1) > 1000. ) verifHT1=fabs((consHT1/lectHT1)-1.0);\
      else verifHT1=0.005;
    } state verifAlimsSL
    when ( verifHT1 < 0.02 ) {
    } state syncScanLarge
  }  /* verifAlimsSL */

  state syncScanLarge {
    when ( efTestAndClear(flagFaisc) ) {
      printf("ht1 = %6.0f (%6.0f) ** ht2 = %6.0f (%6.0f) ** I = %7.0f\n"\
             ,consHT1,lectHT1,consHT2,lectHT2,iFaisc);
      if ( saveScan == 1 && fdscan ) {
        fprintf(fdscan,"%7.1f \t %7.1f \n",consVHT,iFaisc);
      }
    } state scanLarge

  }   /* syncScanLarge */

  /* ----------------------------------------------------------------------------- */

  state scanLarge {
    when ( consHT1 > finScan ) {
      consHT1-=pasSL; 
      consHT2+=pasSL;
      pvPut(consHT1); 
      pvPut(consHT2);
      verifHT1=1.;
      verifHT2=1.;
    } state verifAlimsSL
    when ( consHT1 == finScan ) {
      actionScan=0; 
      pvPut(actionScan);
      printf("scan large : action terminee \n");
      if ( saveScan == 1 && fdscan ) {
         fprintf(fdscan,"%7.1f  \t %7.0f \n",consVHT,iFaisc);
         fclose(fdscan);
         saveScan=0;
         pvPut(saveScan);
      }
    } state wait
    when ( actionScan == 0 ) {
      printf("scan large : processus arrete \n");
      if ( saveScan == 1 ) fclose(fdscan);
      saveScan=0;
      pvPut(saveScan);
    } state wait
  }  /* scanLarge */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  state initDetectUp {
    when ( delay(1.0) ) {
      if ( strcmp(enableMoteur,"Disable") == 0 ) {
        //enableMoteur="Enable";
        strcpy(enableMoteur,"Enable");
        pvPut(enableMoteur);
      }
      if ( lectPosition > 1.0 ) {
        ecrPosition= 0.;
        pvPut(ecrPosition);
      }
    } state initDetectUp
    when ( ( lectPosition == 0. ) && (strcmp(enableMoteur,"Enable") == 0) ) {
      printf("conditions requises pour detection bord faisceau !\n");
      printf("Position=%7.1f mm -- consHT1=%6.0f -- consHT2=%6.0f\n"\
             ,ecrPosition,consHT1,consHT2);
    } state incMoteur
    when ( actionUpper == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* initDetectUp */

  state incMoteur {
    when ( delay(0.5) ) {
      ecrPosition+=pasMoteur;
      pvPut(ecrPosition);
      raz=0.;
      pvPut(raz);
      verifPosMoteur=1.;
      consHT1=debutScan;
      consHT2=-1.0*debutScan;
      pvPut(consHT1); 
      pvPut(consHT2);
      verifHT1=1.;
      verifHT2=1.;
      printf("\n");
      if ( ecrPosition > limDet ) {
        printf("Detect upper : limite atteinte sur moteur -- arret detection \n");
        actionUpper=0;
        pvPut(actionUpper);
      }
    } state verifMoteurUp
    when ( actionUpper == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* incMoteur */

  state verifMoteurUp {
    when ( delay(0.1) ) {
      verifPosMoteur=fabs((ecrPosition/lectPosition)-1.0);
    } state verifMoteurUp
    when ( verifPosMoteur < 0.01 ) {
      raz=1.;
      pvPut(raz);
    } state verifAlimsUp
    when ( actionUpper == 0 ) {
      printf("Detect : processus arrete \n");
    } state wait
  }  /* verifMoteur */

  state verifAlimsUp {
    when ( verifHT1 > 0.02 ) {
      if ( abs(consHT1) > 1000. ) verifHT1=fabs((consHT1/lectHT1)-1.0);\
      else verifHT1=0.005;
    } state verifAlimsUp
    when ( verifHT1 < 0.02 ) {
    } state syncDetectUp
  }  /* verifAlimsUp */

  state syncDetectUp {
    when ( efTestAndClear(flagFaisc) ) {
      printf("Pos.=%7.1f mm * ht1 = %6.0f (%6.0f) * ht2 = %6.0f (%6.0f) * I = %7.0f\n"\
             ,lectPosition,consHT1,lectHT1,consHT2,lectHT2,iFaisc);
    } state scanDetectUp
  }   /* syncDetecUp */

  /* ------------------------------------------------------------------------ */

  state scanDetectUp {
    when ( consHT1 > finScan ) {
      consHT1-=pasAlim; 
      consHT2+=pasAlim;
      pvPut(consHT1); 
      pvPut(consHT2);
      verifHT1=1.;
      verifHT2=1.;
    } state verifAlimsUp
    when ( consHT1 == finScan ) {
      printf("scan detect : point %7.1f mm termine \n",lectPosition);
    } state incMoteur
    when ( lectPosition >= limDet ) {
      actionUpper=0;
      pvPut(actionUpper);
      printf("Detect upper : limite atteinte sur moteur -- arret detection\n");
    } state wait
    when ( actionUpper == 0 ) {
      printf("Detect upper : processus arrete \n");
    } state wait
  }  /* scanDetectUp */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

 state initDetectLow {
    when ( delay(1.0) ) {
      if ( strcmp(enableMoteur,"Disable") == 0 ) {
        //enableMoteur="Enable";
        strcpy(enableMoteur,"Enable");
        pvPut(enableMoteur);
      }
      if ( lectPosition < 180.0 ) {
        ecrPosition= 180.;
        pvPut(ecrPosition);
      }
      verifPosMoteur=fabs((ecrPosition/lectPosition)-1.0);
    } state initDetectLow
    when ( ( verifPosMoteur < 0.01 ) && (strcmp(enableMoteur,"Enable") == 0) ) {
      printf("conditions requises pour detection bord faisceau !\n");
      printf("Position=%7.1f mm -- consHT1=%6.0f -- consHT2=%6.0f\n"\
             ,ecrPosition,consHT1,consHT2);
    } state decMoteur
    when ( actionLower == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* initDetectLow */

  state decMoteur {
    when ( delay(0.5) ) {
      ecrPosition-=pasMoteur;
      pvPut(ecrPosition);
      raz=0.;
      pvPut(raz);
      verifPosMoteur=1.;
      consHT1=debutScan;
      consHT2=-1.0*debutScan;
      pvPut(consHT1); 
      pvPut(consHT2);
      verifHT1=1.;
      verifHT2=1.;
      printf("\n");
      if ( ecrPosition < limDet ) {
        printf("Detect lower : limite atteinte sur moteur -- arret detection \n");
        actionLower=0;
        pvPut(actionLower);
      }
    } state verifMoteurLow
    when ( actionLower == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* decMoteur */

  state verifMoteurLow {
    when ( delay(0.1) ) {
      verifPosMoteur=fabs((ecrPosition/lectPosition)-1.0);
    } state verifMoteurLow
    when ( verifPosMoteur < 0.005 ) {
      raz=1.;
      pvPut(raz);
    } state verifAlimsLow
    when ( actionLower == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* verifMoteur */

  state verifAlimsLow {
    when ( verifHT1 > 0.02 ) {
      if ( abs(consHT1) > 1000. ) verifHT1=fabs((consHT1/lectHT1)-1.0);\
      else verifHT1=0.005;
    } state verifAlimsLow
    when ( verifHT1 < 0.02 ) {
   } state syncDetectLow
  }  /* verifAlimsLow */

  state syncDetectLow {
    when ( efTestAndClear(flagFaisc) ) {
      printf("Pos.=%7.1f mm * ht1 = %6.0f (%6.0f) * ht2 = %6.0f (%6.0f) * I = %7.0f\n"\
             ,lectPosition,consHT1,lectHT1,consHT2,lectHT2,iFaisc);
    } state scanDetectLow
  }   /* syncDetecLow */

  /* ------------------------------------------------------------------------ */

  state scanDetectLow {
    when ( consHT1 > finScan ) {
      consHT1-=pasAlim; 
      consHT2+=pasAlim;
      pvPut(consHT1); 
      pvPut(consHT2);
      verifHT1=1.;
      verifHT2=1.;
    } state verifAlimsLow
    when ( consHT1 == finScan ) {
      printf("scan detect : point %7.1f mm termine \n",lectPosition);
    } state decMoteur
    when ( lectPosition <= limDet ) {
      actionLower=0;
      pvPut(actionLower);
      printf("Detect lower : limite atteinte sur moteur -- arret detection\n");
    } state wait
    when ( actionLower == 0 ) {
      printf("Detect lower : processus arrete \n");
    } state wait
  }  /* scanDetectLow */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  state actValidate {
    when ( delay(2.0) ) {
      vstart=Vpic+(deltaV/2.);  pvPut(vstart);
      vend=Vpic-(deltaV/2.);    pvPut(vend);
      /*vstep=(vstart-vend)/200.; pvPut(vstep);*/
      vstep=(vstart-vend)/(2.*nvpoints); pvPut(vstep);
      upper=borderUp;           pvPut(upper);
      lower=borderLow;          pvPut(lower);
      motorstep=pasMoteur;      pvPut(motorstep);
      /*Energy=Voltage;		pvPut(Energy);*/
      /* dist=(LP+LFSP-LFEP)/(4.*GP); correction avril 2012 N.Chauvin */
      dist=(LP+2.*LFSP)*LP/(4.*GP*(LFEP+LP+LFSP));
      angle=(dist*consVHT)/(Energy*1000.);
      startAngle=(dist*vstart)/(Energy*1000.);
      endAngle=(dist*vend)/(Energy*1000.);
      stepAngle=(dist*vstep)/(Energy*1000.);
      verifHT1=fabs((consHT1/lectHT1)-1.0);
      verifHT2=fabs((consHT2/lectHT2)-1.0);
      validate=0;               pvPut(validate);
      printf("Emit : start = %7.3f rd end = %7.3f rd step = %7.3f mrd \n"\
            ,startAngle,endAngle,stepAngle*1000.);
    } state wait
  }  /* actValidate */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  state initMesEmit {
    when ( delay(1.0) ) {
      if ( strcmp(enableMoteur,"Disable") == 0 ) {
        //enableMoteur="Enable";
        strcpy(enableMoteur,"Enable");
        pvPut(enableMoteur);
      }
    } state initMesEmit
    when ( (strcmp(enableMoteur,"Enable") == 0) ) {
      debutMesure=1;
      printf("moteur Enable !\n");
      if ( etatPS1 == 0 || etatPS2 == 0 ) { 
        printf("power supplies are OFF !\n");
        printf("(waiting 30 seconds)\n");
      }
    } state chkAlims
    when ( actionEmit == 0 ) {
      printf("Emit : processus arrete \n");
    } state wait
  }  /* initMesEmit */

  state chkAlims {
    when ( delay(2.0) ) {
      //if ( etatPS1 == 0 || etatPS2 == 0 ) { printf("power supplies are OFF !\n");}
      if (comPS1 == 0) { comPS1=1; pvPut(comPS1); }
      if (comPS2 == 0) { comPS2=1; pvPut(comPS2); }
    } state chkAlims
    when ( etatPS1 == 1 && etatPS2 == 1  ) {
      printf("power supplies are ON !\n");
    } state chkFacq
    when ( actionEmit == 0 ) {
      printf("Emit : processus arrete \n");
    } state wait
  }  /* chkAlims */

  state chkFacq {
    when ( delay(1.0) ) {
      printf("Emit : end of acquisition must be less or equal than ext trigger!!!\n");
      printf("Emit : Tacq=%6.0f ms Text=%6.0f ms \n",tacq,trep);
      printf("\n");
   } state cmdMoteur
  }  /* chkFacq */

  state cmdMoteur {
    entry{
      printf("State: cmdMoteur \n");
      fflush(stdout);
    }
    when ( delay(0.1) ) {
      printf("cmdMoteur: entered 'when' block\n");

      if ( debutMesure == 1 ) {
        printf("cmdMoteur: debutMesure == 1, assigning ecrPosition = upper (%.2f) \n", upper); 
        ecrPosition=upper; 
        printf("cmdMoteur: after assigning ecrPosition = %.2f \n", ecrPosition);
      }
      else { 
        printf("cmdMoteur: debutMesure != 1, incrementing ecrPosition by motorstep (%.2f)\n", motorstep);
        ecrPosition+=motorstep; 
        printf("cmdMoteur: after increment, ecrPosition = %.2f\n", ecrPosition); 
      }
      printf("cmdMoteur: checking if ecrPosition (%.2f) < highLimit (%.2f)\n", ecrPosition, highLimit);
      if ( ecrPosition < highLimit ) { 
        printf("cmdMoteur: condition true, attempting pvPut(ecrPosition)\n"); 
        pvPut(ecrPosition); 
        printf("cmdMoteur: pvPut(ecrPosition) successful\n"); 
      } else {
        printf("cmdMoteur: ecrPosition >= highLimit, skipping pvPut\n");
      }
      printf("cmdMoteur: writing raz = 0\n");
      raz=0.; pvPut(raz);
      printf("cmdMoteur: pvPut(raz) done\n");
      verifPosMoteur=1.;
      printf("cmdMoteur: set veriPosMoteur = 1\n");
      consHT1=(vstart/2.);
      consHT2=-1.*(vstart/2.);
      sumVHT=consHT1-consHT2;
      printf("cmdMoteur: consHT1 = %.2f, consHT2 = %.2f, sumVHT = %.2f\n", consHT1, consHT2, sumVHT);
      pvPut(consHT1); printf("cmdMoteur: pvPut(consHT1) done\n");
      pvPut(consHT2); printf("cmdMoteur: pvPut(consHT2) done\n");
      verifHT1=1.; 
      verifHT2=1.;
      printf("cmdMoteur: verifHT1 = %.1f, verifHT2 = %.1f", verifHT1, verifHT2);
      /* init des variables recherche max avant scan */
      imax=0.; vpkmax=0.;
      printf("cmdMoteur: imax and vpkmax initialized to 0.0\n");
    } state verifMoteur
    when ( actionEmit == 0 ) {
      printf("Emit : processus arrete \n");
    } state wait
    when ( actionEmit == 2 ) {
      // pour sortie propre mesure terminee
      printf("\n");
      actionEmit=0; pvPut(actionEmit);
    } state wait
    when ( pauseEmit == 1 ) {
      printf("Emit : pause demandee \n");
    } state pause4
  }  /* cmdMoteur */

  state verifMoteur {
    entry {
      printf("\nState: verifMoteur\n");
      fflush(stdout);
    }
    when ( verifPosMoteur > 0.01 ) {
      printf("verifMoteur: verifPosMoteur = %.6f > 0.01 -> recomputing\n", verifPosMoteur);
      printf("verifMoteur: ecrPosition = %.6f, lectPosition = %.6f\n", ecrPosition, lectPosition);
      verifPosMoteur=fabs((ecrPosition/lectPosition)-1.0);
    } state verifMoteur
    when ( verifPosMoteur < 0.01 ) {
      raz=1.; pvPut(raz);
      printf("verifMoteur: pvPut(raz=1) done\n");
      debutMesure=0; 
      printf("verifMoteur: debutMesure set to 0 \n");
      angle=(dist*consVHT)/(Energy*1000.);
      printf("verifMoteur: computed angle: %.6f\n", angle);
      %%nanosleep(&retard, NULL);
      printf("verifMoteur: first nanosleep done\n");
      %%nanosleep(&retard, NULL);
      printf("verifMoteur: second nanosleep done\n");
    } state waitAlims
  } 

  state waitAlims {
    entry{
       printf("\nState: waitAlims\n");
       printf("waitAlims: verifHT1 = %.2f\n", verifHT1);
    }
    when ( verifHT1 > 0.01 ) {
       //verifHT1=fabs((consHT1/lectHT1)-1.0);
       // retard pour val alim atteinte supprimee 
       //%%nanosleep(&retard, NULL);
       printf("waitAlims: verifHT1 = %.2f\n", verifHT1);
       verifHT1=0.005;
       printf("waitAlims: verifHT1 = %.3f\n", verifHT1);
    } state waitAlims
    when ( verifHT1 <= 0.01 ) {
      /* delai atteint */
    } state syncMesure2
  }   /* waitAlims */

  //  commentee pour une seule attente sur evenement Facq
  //  state syncMesure1 {
  //    when ( efTestAndClear(flagFaisc) ) {
  //      } state syncMesure2
  //  } 

  state syncMesure2 {
    entry{
      printf("\nState: syncMesure2\n");
      printf("syncMesure2: flagFaisc = %.2f\n", flagFaisc);
      fflush(stdout);
    }
    when ( delay(1.0) ) { // Patricia: og: when(efTestAndClear(flagFaisc))
      angle=(dist*sumVHT)/(Energy*1000.);
      printf("ht1=%6.0f (%6.0f) ht2=%6.0f (%6.0f) Vht=%7.0f Pos=%6.1f I=%7.0f yp=%7.3f \n"\
      ,consHT1,lectHT1,consHT2,lectHT2,sumVHT,lectPosition,iFaisc,angle);
      printf("syncMesure2: saveEmit = %d\n", saveEmit);
      printf("syncMesure2: fdemit = %p\n", fdemit);
      if ( saveEmit == 1 && fdemit ) {
        // utilisation variable locale sumVHT au lieu de consVHT
        // car consVHT variable DBs scan=100 ms
        fprintf(fdemit,"%7.1f  \t %7.0f  \t %7.4f  \t %7.0f \n",\
        lectPosition,sumVHT,angle,iFaisc);
      }
    } state actEmit
  }   /* syncMesure */

  /* ------------------------------------------------------------------------ */

  state actEmit {
    entry {
      printf("\nState: actEmit\n");
      printf("actEmit: consHT1 = %.2f, vend/2 = %.2f\n", consHT1, vend/2);
      printf("actEmit: ecrPosition = %.2f, lower + motorstep = %.2f, vstep = %.2f\n", ecrPosition, lower+motorstep, vstep);
      fflush(stdout);
    }
    when ( (consHT1 > (vend/2.) ) && (ecrPosition < (lower+motorstep)) ) {
      printf("actEmit: consHT1 > vend/2 satisfied\n");
      consHT1-=vstep; pvPut(consHT1);
      consHT2+=vstep; pvPut(consHT2);
      printf("actEmit: pvPut(consHT1)and pvPut(consHT2 done)\n");
      sumVHT=consHT1-consHT2;
      verifHT1=1.; verifHT2=1.; 
      printf("actEmit: verifHT1 and verifHT2 = 1\n");
      // recherche max 
      /*if (iFaisc > imax) {
        imax=iFaisc;
        vpkmax=(consHT1-consHT2);
      }*/
    } state waitAlims
    when ( consHT1 <= (vend/2.) ) {
      printf("Emit : scan %7.1f mm termine \n",lectPosition);
      // validation Vpic et modifications vstart vend
      // Vpic+=vpkmax;  pvPut(Vpic);
      Vpic+=0.;  pvPut(Vpic);
      vstart=Vpic+(deltaV/2.);  pvPut(vstart);
      vend=Vpic-(deltaV/2.);    pvPut(vend);
      raz=0.; pvPut(raz);
      if ( saveEmit == 1 && fdemit ) fprintf(fdemit,"\n");
      if ( ecrPosition >= lower ) {
        printf("Emit : mesure terminee - / Envois emit=0, HTs=0 \n");
        if ( saveEmit == 1 && fdemit ) {
          fclose(fdemit);
          saveEmit=0;
          pvPut(saveEmit);
        }
        ecrPosition= 0.; pvPut(ecrPosition);
        consHT1=0.; pvPut(consHT1);
        consHT2=0.; pvPut(consHT2);
        actionEmit=2; pvPut(actionEmit);
      }
    } state cmdMoteur
    when ( ecrPosition >= (lower+motorstep) ) {
      actionEmit=0; pvPut(actionEmit);
      printf("Emit : mesure terminee + / Envois emit=0, HTs=0 \n");
      if ( saveEmit == 1 && fdemit ) {
        fclose(fdemit);
        saveEmit=0;
        pvPut(saveEmit);
      }
      ecrPosition= 0.; pvPut(ecrPosition);
      consHT1=0.; pvPut(consHT1);
      consHT2=0.; pvPut(consHT2);
    } state wait
    when ( actionEmit == 0 ) {
      printf("Emit : processus arrete \n");
      if ( saveEmit == 1 && fdemit ) fclose(fdemit);
    } state wait
  }  /* actEmit */

  /* ------------------------------------------------------------------------ */

  state pause4 {
    when ( pauseEmit == 0 ) {
      printf("Emit : reprise \n");
    } state actEmit
    when ( delay(2.0) ) {
    } state pause4
  }  /* pause4 */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
} /* FIN ss mesEmit */
