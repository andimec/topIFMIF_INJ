program emitSnlEmu_v2
/* this state machine contains the ROI optimization + the voltage grid*/

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
/* Part of the optimization process */ 
double saveWf;
assign saveWf to "LEBT:EMIT:Savewf";
monitor saveWf;
/* -------------------------------- */
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
/* Part of the optimization process */
double VpicEnd;
assign VpicEnd to "LEBT:EMIT:VpicEnd";
monitor VpicEnd;
/* -------------------------------- */

/* Part of the optimization process */
double anglePicInit;
assign anglePicInit to "LEBT:EMIT:AnglePicInit";
monitor anglePicInit;

double anglePicEnd;
assign anglePicEnd to "LEBT:EMIT:AnglePicEnd";
monitor anglePicEnd;

double angleDelta;
assign angleDelta to "LEBT:EMIT:AngleDelta";
monitor angleDelta;
/* -------------------------------- */
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
assign consHT1 to "LEBT:EMIT_PS1:VSet";  // og: without .PROC
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

/* -------------- PARTICLES --------------*/
/* Part of the optimization process */
string D2valveTxt;
assign D2valveTxt to "SRC:SVL:D2CloseStat";
monitor D2valveTxt;
string H2valveTxt;
assign H2valveTxt to "SRC:SVL:H2CloseStat";
monitor H2valveTxt;
/* -------------------------------- */

/* --- faisceau + flag --- */
double iFaisc;
assign iFaisc to "LEBT:EMIT_FC:I";
monitor iFaisc;
evflag flagFaisc;
sync iFaisc flagFaisc;

/* Part of the optimization process */
/* ---------------- REGISTER WAVEFORMS ----------------- */
#define CAP 10000
#define EPS_T 1.0e-6

float iFaiscWaveform[CAP];
assign iFaiscWaveform to "LEBT:EMIT_FC:IwfGraph"; // Declare waveforms
monitor iFaiscWaveform;

float timeWaveform[CAP];
assign timeWaveform to "LEBT:FACQ:Xwf"; // x axis 
monitor timeWaveform;

double tacq1;
assign tacq1 to "LEBT:FACQ:Tdisp1";
monitor tacq1;
double tacq2;
assign tacq2 to "LEBT:FACQ:Tdisp2";
monitor tacq2;
double tmeas1;
assign tmeas1 to "LEBT:FACQ:Tmeas1";
monitor tmeas1;
double tmeas;
assign tmeas to "LEBT:FACQ:Tmeas2";
monitor tmeas;

double freqSamp;
assign freqSamp to "LEBT:FACQ:Fsamp.RVAL";
monitor freqSamp;

int Number = 1;
int i_loop;
int nest = 0;

/* -------------------------------- */


/* -------------- VOLTAGE GRID --------------*/
/* Part of the optimization process December 2025 */

%%#include <stdio.h>
%%#include <string.h>
%%#include <math.h>
%%#include <stdlib.h>
%%#include <time.h>
%%#include <unistd.h>

%%/* ---------- Voltage grid for VHT (V1-V2) ---------- */
%%#define MAX_VGRID 4096

%%static double VHTgrid[MAX_VGRID];
%%static int    Ngrid = 0;

%%static double VHTminGrid = 0.0;
%%static double VHTmaxGrid = 0.0;
%%static double dVHTgrid   = 0.0;

%%static int idxStart = 0;
%%static int idxEnd   = 0;
%%static int idxCur   = 0;

%%static int clampInt(int x, int lo, int hi)
%%{
%%  if (x < lo) return lo;
%%  if (x > hi) return hi;
%%  return x;
%%}

%%static int snapVHTIndex(double VHT)
%%{
%%  double r = (VHT - VHTminGrid) / dVHTgrid;
%%  int k = (int)(r + 0.5);
%%  return clampInt(k, 0, Ngrid - 1);
%%}

%%static void buildVHTGrid(void)
%%{
%%  double v;
%%  Ngrid = 0;
%%  v = VHTminGrid;

%%  while (Ngrid < MAX_VGRID && v <= VHTmaxGrid + 0.5*dVHTgrid) {
%%    VHTgrid[Ngrid] = v;
%%    Ngrid++;
%%    v += dVHTgrid;
%%  }

%%  if (Ngrid > 0) VHTgrid[Ngrid-1] = VHTmaxGrid;
%%  printf("VHT grid built: N=%d, Vmin=%.1f, Vmax=%.1f, dV=%.1f\n",
%%         Ngrid, VHTminGrid, VHTmaxGrid, dVHTgrid);
%%  fflush(stdout);
%%}

/* -------------------------------- */

double trep;
assign trep to "SRC:TIM:Trep";
monitor trep;

/* --- parametres moteur --- */
double highLimit;
assign highLimit to "LEBT:EMIT:MOTOR.DHLM"; /* HLM before */
monitor highLimit;
double lowLimit;
assign lowLimit to "LEBT:EMIT:MOTOR.DLLM"; /* LLM */
monitor lowLimit;
double ecrPosition;
assign ecrPosition to "LEBT:EMIT:MOTOR.VAL";
monitor ecrPosition;
double lectPosition;
 assign lectPosition to "LEBT:EMIT:MOTOR.RBV"; 
//assign lectPosition to "LEBT:EMIT:MOTOR_RBV"; // Patricia commented this on Aug 29
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

/* --- Feb 2026 extra variables --- */
int movn;
assign movn to "LEBT:EMIT:MOTOR.MOVN";
monitor movn;
int dmov;
assign dmov to "LEBT:EMIT:MOTOR.DMOV";
monitor dmov;

/*--------------------------------- */

/* --- parameters measurement Emittance --- */
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

/* --- physical constants of the Emittancemeter --- */
#define LFEP 0.01	/* L1 distance fenteEntree-plaques - en metre */
#define LP 0.09		/* L2 longueur des plaques         - en metre */
#define LFSP 0.002	/* L3 distance plaques-fenteSortie - en metre */
#define GP 0.009	/* GP distance entre les plaques   - en metre */
//double Voltage;
//assign Voltage to "LEBT:HV_PS:HTCmd"; // Patricia: not used in the code
//monitor Voltage;
double Energy;
assign Energy to "LEBT:EMIT:Energy";
monitor Energy;


%%FILE *fdscan;
%%FILE *fdemit;
%%FILE *fdwaveform; //new
%%time_t secondes;
%%char tabDate[24];
%%char *fmtDate=tabDate;
%%struct tm *ptdate;

%%char fullPath[1024];
%%char fullPathWaveform[1024]; //new
%%char dataDir[128];   /* 25 */
%%char fileName[32];   /* 23 */
%%char scanFile[64];   /* 55 */
%%char emittanceFile[128];  /* 55 */
%%char waveformFile[128]; //new
%%int mstep; 
%%int hvstep; 
%%int ldetect;
%%int nbpos;
int debutMesure=-1;
int n=0;
int i=0; //new
float emu=0.;
double dist;
double angle;
double startAngle=0.;
double endAngle=0.;
double stepAngle=0.;
double imax=0.;
double vpkmax=0.;
double sumVHT=0.;
double realVHT=0.;
int nhv=0;
double slope=0;
double beta=0;

%%struct timespec retard;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
ss mesEmit {
  state connectEmit {
    entry{
      // printf("State: connectEmit \n");
      // fflush(stdout); 
    }
    when ( delay(2.0) && ( pvConnectCount() == pvChannelCount() ) ) {
      printf("\n");
      printf("Emit: established PV connections \n");
      printf("Emit: waiting \n");
    } state wait
  }
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  state wait {
    entry{
      // printf("State: wait \n");
      // fflush(stdout); 
    }
    /* ------------------------------------------------------------------------ */
    when ( actionEmit == 1 ) {
      printf("Emit: emittance measurement action received \n");
      printf("Emit: process started!! \n");
      if ( saveEmit == 0 ) {
        saveEmit = 1;
        pvPut(saveEmit);
        printf("Emit: measurement will be saved by default \n");
      }
      if ( saveEmit == 1 ) { 
        time(&secondes);
	      /* ------ dirty hack: since ctime(), localtime() returns GMT although we set JST, just add +9 hours ---*/
	      //      secondes += 6 * 60 * 60; // 9 -> 6
        fmtDate=ctime(&secondes);
        if (fmtDate == NULL) {
          printf("ERROR: ctime() returned NULL \n");
        }
        printf("\n%s",fmtDate);

        ptdate=localtime(&secondes);
        if (ptdate == NULL) {
          printf("ERROR: localtime() returned NULL \n");
        }

        if(strftime(fileName,sizeof(fileName),"emit%Y%m%d-%H%M_optm",ptdate) == 0){
          printf("ERROR: strftime() failed \n");
        }
        // strcpy (dataDir, getenv("IFMIF_DATA"));
        // Get the current working directory
        if (getcwd(dataDir, sizeof(dataDir)) == NULL) {
          printf("ERROR: getcwd() failed\n");
        }
        printf("Current directory: %s\n", dataDir);

        // sprintf(dataDir,"/home/ifmif/data");
        // Create the file path
        if (snprintf(emittanceFile, sizeof(emittanceFile), "/home/ifmif/data/data_INJ/emit_test/%s.txt", fileName) >= sizeof(emittanceFile)) {
        // /home/ifmifmgr/topIFMIF_INJ_vme/tests_mgr/%s.txt
	// /home/ifmifmgr/tests_mgr
        // /home/ifmif/data/data_INJ/emit_test
            printf("ERROR: snprintf() output was truncated\n");
        }

        if (realpath(emittanceFile, fullPath) != NULL) {
          printf("Data storing: %s\n", fullPath);
        } else {
          printf("Data storing: %s\n", emittanceFile);
        }

        printf("Saving data points \n"); // LOOK INTO THIS !!
        printf("File = %s\n\n",emittanceFile);

        fdemit = fopen(emittanceFile, "w"); 
        if (fdemit == NULL ) printf("ERROR: Cannot open file: %s\n", emittanceFile);

        fprintf(fdemit,"processus = Emittance Measurement vers. 2.0.0\n");
        fprintf(fdemit,"date      = %s",fmtDate);
        fprintf(fdemit,"file = %s \n",emittanceFile);
        fprintf(fdemit,"Energy  =%7.0f keV   H2= %s   D2= %s   PSR=%7.1f \n", Energy, H2valveTxt, D2valveTxt, lectPSR);
        fprintf(fdemit,"VpicInit=%7.0f deltaV=%7.0f HVpoints=%7.0f VpicEnd=%7.0f \n", Vpic, deltaV, nvpoints, VpicEnd);
        fprintf(fdemit,"Upper   =%7.0f Lower =%7.0f Mstep   =%7.0f \n", upper, lower, motorstep); //before: borderUp, borderLow, motorstep (not even consistent)
        fprintf(fdemit,"Vpic (V)=%7.1f * position (mm) + %7.1f \n", slope, beta);
        fprintf(fdemit,"First position: VHT from %7.0f to %7.0f \n", vstart, vend);
        fprintf(fdemit,"                V1  from %7.0f to %7.0f Vstep=%5.0f \n", vstart/2.0, vend/2.0, vstep);
        //fprintf(fdemit,"                V1 start=%7.0f V1 end=%7.0f Vstep   =%5.0f \n", vstart/2.0, vend/2.0, vstep);
        fprintf(fdemit,"\n");
        //fprintf(fdemit,"Pos(mm) \t V1-V2(V) \t Angle(rd) \t I(ma) \n");
        fprintf(fdemit,"Pos(mm)\t\tV1-V2(V)-set\tAngle(rd)\tI(ma)\t\tV1-V2(V)-real\n");
        
        if ( saveWf == 1 ) {

          /* Second file for wavefomrs */
          if (strftime(fileName,sizeof(fileName),"emit%Y%m%d-%H%M_Xwf_IwfGraph",ptdate) == 0) {
            printf("ERROR: strftime() failed for waveform file\n");
          }

          // Create path for waveform file
          if (snprintf(waveformFile, sizeof(waveformFile), "/home/ifmif/data/data_INJ/emit_test/%s.txt", fileName) >= sizeof(waveformFile)) {
            printf("ERROR: snprintf() output was truncated (waveform file)\n");
          }

          if (realpath(waveformFile, fullPathWaveform) != NULL) {
            printf("Waveform data storing: %s\n", fullPathWaveform);
          } else {
            printf("Waveform data storing: %s\n", waveformFile);
          }

          printf("Saving waveform points \n"); 
          printf("Waveform File = %s\n\n", waveformFile);

          fdwaveform = fopen(waveformFile, "w");
          if (fdwaveform == NULL) printf("ERROR: Cannot open waveform file: %s\n", waveformFile);

	        // Calling most recent values and arrays
	        pvGet(freqSamp, SYNC);
          pvGet(tacq1, SYNC);
          pvGet(tacq2, SYNC);
          pvGet(timeWaveform, SYNC); //new: ensure PV has all the contents
          pvGet(iFaiscWaveform, SYNC); // to the fresh

          // Save header for waveform file
          fprintf(fdwaveform,"processus = Emittance Measurement - Waveform Log\n");
          fprintf(fdwaveform,"date      = %s", fmtDate);
          fprintf(fdwaveform,"file      = %s\n", waveformFile);
          fprintf(fdwaveform,"Energy  =%7.0f keV   H2= %s   D2= %s   PSR=%7.1f \n", Energy, H2valveTxt, D2valveTxt, lectPSR);
          fprintf(fdwaveform,"VpicInit=%7.0f deltaV=%7.0f HVpoints=%7.0f VpicEnd=%7.0f \n", Vpic, deltaV, nvpoints, VpicEnd);
          fprintf(fdwaveform,"Upper   =%7.0f Lower =%7.0f Mstep   =%7.0f \n", upper, lower, motorstep); //before: borderUp, borderLow, motorstep (not even consistent)
          fprintf(fdwaveform,"Vpic (V)=%7.1f * position (mm) + %7.1f \n", slope, beta);
          fprintf(fdwaveform,"First position: VHT from %7.0f to %7.0f \n", vstart, vend);
          fprintf(fdwaveform,"                V1  from %7.0f to %7.0f Vstep=%5.0f \n", vstart/2.0, vend/2.0, vstep);
	        fprintf(fdwaveform,"Frequency Sampling = %.0f kEch/s \n", freqSamp);
          fprintf(fdwaveform,"Acquisition times: Tdisp1 = %.3f Tdisp2 = %.3f ms\n", tacq1, tacq2); //PREC of PV is 3
	        fprintf(fdwaveform,"Measurement times: Tmeas1 = %.3f Tmeas2 = %.3f ms\n\n", tmeas1, tmeas);
          fprintf(fdwaveform,"Time [ms]\n");

          /* 1) Option 1: infer Number from increasing time axis */
          for (i_loop = 1; i_loop < CAP; i_loop++) {
            if ( !(timeWaveform[i_loop] > timeWaveform[i_loop - 1] + EPS_T)) {
              /* time stopped increasing (or never started) - cut here */
              Number = i_loop;
              break;
            }
          }
          if (i_loop == CAP) Number = CAP; /* fully filled or no padding */
          printf("SNL option 1: effective samples N = %d (Fsamp = %.0f kEch/s, T1 = %.3f ms, T2 = %.3f ms)\n", Number, freqSamp, tacq1, tacq2);

          /* 2) Option 2: if Number is useless */
          if (Number <= 1) {
            pvGet(freqSamp, SYNC);
            pvGet(tacq1, SYNC);
            pvGet(tacq2, SYNC);
            if(freqSamp > 0 && tacq2 > tacq1) {
              nest = (freqSamp * (tacq2 - tacq1) + 0.5);
              if (nest < 1) nest = 1;
              if (nest > CAP) nest = CAP;
              Number = nest;
            }
          }
          printf("SNL option 2: effective samples N = %d (Fsamp = %.0f kEch/s, T1 = %.3f ms, T2 = %.3f ms)\n", Number, freqSamp, tacq1, tacq2);

          for (i = 0; i < Number; i++) {
            fprintf(fdwaveform, "%.3f ", timeWaveform[i]);
          }
          fprintf(fdwaveform, "\n\n");

          fprintf(fdwaveform,"Pos(mm)\t\tV1-V2(V)\t\t\tI(ma)\t\tWaveform (mA)\n");
        }
      }
      // memset(&retard, 0, sizeof(retard)); // Clear struct memory
      // retard.tv_nsec = 100000000;

    } state initMesEmit

    when ( actionScan == 1 ) {
      printf(" \n");
      printf("Scan large: action scan Large received\n");
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
      printf("Scan large: process started!! \n");
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
        fprintf(fdscan,"process = Wide Scan \n");
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
      printf("Detect upper : action detection received\n");
      printf(" \n");
      printf("Detect upper: process started!! \n");
      printf("motor= %s  Pos=%7.1f mm\n",enableMoteur,lectPosition);
      verifPosMoteur=1.;
    } state initDetectUp
    /* ------------------------------------------------------------------------ */
    when ( actionLower == 1 ) {
      printf("Detect lower : action detection received\n");
      printf(" \n");
      printf("Detect lower: process started!! \n");
      printf("motor= %s  Pos=%7.1f mm\n",enableMoteur,lectPosition);
      verifPosMoteur=1.;
    } state initDetectLow
    /* ------------------------------------------------------------------------ */
    when ( validate == 1 ) {
      printf("Emit: validate action received\n");
    } state actValidate

}  /* wait */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* From verifAlimSL until actValidate: code remains unchanged */
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
    entry{
      // printf("State: actValidate \n");
      // fflush(stdout);
    }
    when ( delay(2.0) ) {
      if (anglePicInit == -9999 || anglePicEnd == 9999) printf("\nValues for 'angle peak (init)' and 'angle peak (end)' must be given before measurement starts! \n");
      if (borderUp < borderLow) printf("\nValue for 'upper' needs to be higher than value for 'lower'. \n");
      if (anglePicInit != -9999 && anglePicEnd != 9999 && borderUp > borderLow) {
        /* dist=(LP+LFSP-LFEP)/(4.*GP); correction avril 2012 N.Chauvin */
        dist = (LP+2.*LFSP)*LP/(4.*GP*(LFEP+LP+LFSP));
        Vpic = (anglePicInit*Energy)/dist; // took *1000 out, because input values are already in mrad
        VpicEnd = (anglePicEnd*Energy)/dist; 
        deltaV = (angleDelta*Energy)/dist;

        vstart = Vpic+(deltaV/2.);
        vend = Vpic-(deltaV/2.);
        vstep = (vstart-vend)/(2.*nvpoints);

        upper = 130 - borderUp;
        lower = 130 - borderLow;
        motorstep = pasMoteur;
        /*Energy=Voltage;		pvPut(Energy);*/
        
       //printf("\n EMU validated parameters:\n vstart = %7.0f   vend = %7.0f  vstep     = %7.1f \n upper  = %7.0f  lower = %7.0f  motorstep = %7.1f \n\n "\
          ,vstart,vend,vstep, upper, lower, motorstep);
        printf(" Input POSITION: borderUp  = %7.1f boderLow    = %7.1f pasMoteur  = %7.1f (mm) \n", borderUp, borderLow, pasMoteur);
        printf(" Motor POSITION: upper     = %7.1f lower       = %7.1f motorstep  = %7.1f \n", upper, lower, motorstep);
        printf(" Input ANGLE: anglePicInit = %7.1f anglePicEnd = %7.1f angleDelta = %7.1f (mrad) \n", anglePicInit, anglePicEnd, angleDelta);
        printf(" plates VOLTAGES: Vpic     = %7.1f VpicEnd     = %7.1f deltaV     = %7.1f \n\n", Vpic, VpicEnd, deltaV);
      
        /* These are to print in the shell */
        angle=(dist*consVHT)/(Energy*1000.);
        startAngle=(dist*vstart)/(Energy*1000.);
        endAngle=(dist*vend)/(Energy*1000.);
        stepAngle=(dist*vstep)/(Energy*1000.);

        verifHT1=fabs((consHT1/lectHT1)-1.0);
        verifHT2=fabs((consHT2/lectHT2)-1.0);
      
        slope = (VpicEnd - Vpic) / (lower - upper);
        beta  = VpicEnd - slope * lower;
        printf("VpicEnd = %5.0f\n", VpicEnd);
        printf("Vpic = %5.0f\n", Vpic);
        printf("Vpic (V) = %.1f * position (mm) + %.1f\n", slope, beta);
        
        /* Publishing for the OPI */
        pvPut(Vpic, SYNC);
        pvPut(VpicEnd, SYNC);
        pvPut(deltaV, SYNC);
        pvPut(vstart, SYNC);
        pvPut(vend, SYNC);
        pvPut(vstep, SYNC);
        pvPut(upper, SYNC);
        pvPut(lower, SYNC);
        pvPut(motorstep, SYNC);
      }
      validate=0;               pvPut(validate);
      printf("Emit: start = %7.3f rd end = %7.3f rd step = %7.3f mrd \n\n"\
            ,startAngle,endAngle,stepAngle*1000.);
    } state wait
  }  /* actValidate */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  state initMesEmit {
    entry { 
      // printf("State: initMesEmit \n");
      // fflush(stdout); 
    }
    when (delay(1.0)) {
      /*
      if (enableMoteur == NULL) {
        printf("ERROR: enableMoteur is NULL\n");
      } else {
        printf("enableMoteur = %s\n", enableMoteur);
      }*/

      if (enableMoteur != NULL && strcmp(enableMoteur, "Disable") == 0) {
        strcpy(enableMoteur,"Enable");
        pvPut(enableMoteur);
        //printf("Updated enableMoteur to: %s\n", enableMoteur);
        fflush(stdout);
      }
      /*
      if (motorInt == 0) {
        motorInt = 1;
        pvPut(motorInt);
        // printf("Updated enableMoteur to: %s\n", enableMoteur);
        printf("Updated motorInt to: %s\n", motorInt ? "Enable" : "Disable");
      }*/
    } state initMesEmit

    when ((strcmp(enableMoteur,"Enable") == 0) ) {
    // when (motorInt == 1) {
      debutMesure=1;
      printf("Motor enabled!\n");
      if ( etatPS1 == 0 || etatPS2 == 0 ) { 
        printf("Power supplies are OFF !\n");
        printf("(waiting 30 seconds)\n");
      }
    } state chkAlims

    when ( actionEmit == 0 ) {
      printf("Emit: process stopped\n");
    } state wait
  }  /* initMesEmit */

  state chkAlims {
    entry{
      // printf("State: chkAlims \n");
      // fflush(stdout);
    }
    when ( delay(2.0) ) {
      //if ( etatPS1 == 0 || etatPS2 == 0 ) { printf("power supplies are OFF !\n");}
      if (comPS1 == 0) { comPS1=1; pvPut(comPS1); }
      if (comPS2 == 0) { comPS2=1; pvPut(comPS2); }
    } state chkAlims
    when ( etatPS1 == 1 && etatPS2 == 1  ) {
      printf("Power supplies are ON !\n");
    } state chkFacq
    when ( actionEmit == 0 ) {
      printf("Emit: process stopped\n");
    } state wait
  }  /* chkAlims */

  state chkFacq {
    entry{
      // printf("State: chkFacq \n");
      // fflush(stdout);
    }
    when ( delay(1.0) ) {
      printf("Emit: end of acquisition must be less or equal than ext trigger!!!\n");
      printf("Emit: Tacq=%6.0f ms Text=%6.0f ms \n",tacq2,trep);
      printf("\n");
   } state buildGrid // before: state cmdMoteur
  }  /* chkFacq */

  state buildGrid {
    entry{
      // printf("State: buildGrid\n");
      // fflush(stdout);
    }

    when ( delay(0.1) ) {

      /* dVHTgrid must match the step of your scan in VHT.
        You step V1 by vstep, so VHT steps by 2*vstep. */
      dVHTgrid = 2.0 * vstep;

      /* Global range across the full scan.
        Your header logic matches this: max at lower, min at upper. */
      VHTminGrid = (Vpic - deltaV/2.0);        /* at upper */
      VHTmaxGrid = (VpicEnd + deltaV/2.0);     /* at lower */

      /* Safety: if slope negative, swap */
      if (VHTminGrid > VHTmaxGrid) {
        double tmp = VHTminGrid;
        VHTminGrid = VHTmaxGrid;
        VHTmaxGrid = tmp;
      }

      buildVHTGrid();

    } state cmdMoteur
  } /* buildGrid */ 


  state cmdMoteur {
    entry { 
      // printf("State: cmdMoteur \n");
      // fflush(stdout);
    }
    when ( delay(0.1) ) { //og: 0.1
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
        pvPut(ecrPosition); // before with SYNC
        printf("cmdMoteur: pvPut(ecrPosition) successful\n");
        pvGet(lectPosition);
        printf("pvGet(lectPosition) = %6.0f \n", lectPosition);
      } else {
        printf("cmdMoteur: ecrPosition >= highLimit, skipping pvPut\n");
      }
     
      /* Update vstart and vend if in ROI measurement mode  */
      printf("cmdMoteur: calculating the Vpic \n");  
      Vpic = slope * ecrPosition + beta;
      pvPut(Vpic);

      vstart = Vpic + (deltaV / 2.);  pvPut(vstart);
      vend   = Vpic - (deltaV / 2.);  pvPut(vend);

      /* Snap start/end to the VHT grid indices */
      idxStart = snapVHTIndex(vstart);
      idxEnd   = snapVHTIndex(vend);

      /* Ensure we scan from high -> low (consistent with your existing actEmit logic) */
      if (idxEnd > idxStart) {
        int tmp = idxStart; idxStart = idxEnd; idxEnd = tmp;
      }

      /* Set current index for this position */
      idxCur = idxStart;

      /* Apply snapped first VHT */
      sumVHT = VHTgrid[idxCur];          /* VHT = V1 - V2 */
      consHT1 =  sumVHT / 2.0;
      consHT2 = -sumVHT / 2.0;

      pvPut(consHT1, SYNC);
      pvPut(consHT2, SYNC);

      pvGet(lectHT1);
      pvGet(lectHT2);

      printf("cmdMoteur: pos=%.1f Vpic=%.1f raw VHTstart=%.1f -> snapped VHT=%.1f (idx %d)\n",
            ecrPosition, Vpic, vstart, sumVHT, idxCur);

      raz=0.; pvPut(raz);
      verifPosMoteur=1.;
      verifHT1=1.;
      verifHT2=1.;
      imax=0.; vpkmax=0.;

    } state verifMoteur
    when ( actionEmit == 0 ) {
      printf("Emit: process stopped\n");
    } state wait
    when ( actionEmit == 2 ) {
      // pour sortie propre mesure terminee
      printf("\n");
      actionEmit=0; pvPut(actionEmit);
    } state wait
    when ( pauseEmit == 1 ) {
      printf("Emit: pause requested\n");
    } state pause4
  }  /* cmdMoteur */

  state verifMoteur {
    entry {
      // printf("\nState: verifMoteur\n");
      // fflush(stdout);
    }
    /* recompute error continuously */
    when (delay(0.02)) {
      verifPosMoteur=fabs(ecrPosition-lectPosition);
    } state verifMoteur
    // when ( verifPosMoteur > 0.5 ) {
      // printf("verifMoteur: verifPosMoteur = %.6f > 0.01 -> recomputing\n", verifPosMoteur);
      // printf("verifMoteur: ecrPosition = %.6f, lectPosition = %.6f\n", ecrPosition, lectPosition);
      // verifPosMoteur=fabs((ecrPosition/lectPosition)-1.0);
    // } state verifMoteur
    /* ONLY gets out of the loop when motor is finished and within tolerance */
    when ( movn == 0 && dmov == 1 && verifPosMoteur < 0.01 ) {
      printf("Motor stable at target. RBV=%.4f VAL=%.4f diff=%.6f\n", lectPosition, ecrPosition, verifPosMoteur);
      raz=1.; pvPut(raz);
      // printf("verifMoteur: pvPut(raz=1) done\n");
      debutMesure=0;
      // printf("verifMoteur: debutMesure set to 0 \n");
      angle=(dist*consVHT)/(Energy*1000.);
      // printf("verifMoteur: computed angle: %.6f\n", angle);
      // %%nanosleep(&retard, NULL);
      // printf("verifMoteur: first nanosleep done\n");
      // %%nanosleep(&retard, NULL);
      // printf("verifMoteur: second nanosleep done\n");
    } state waitAlims
  }

  // Feb 2026 commented this
  state waitAlims {
    entry{
      // printf("\nState: waitAlims \n");
      // printf("waitAlims: verifHT1 = %.2f\n", verifHT1);
      // fflush(stdout);
    }
    when (verifHT1 > 0.01 ) {
       //verifHT1=fabs((consHT1/lectHT1)-1.0);
       // retard pour val alim atteinte supprimee 
       // %%nanosleep(&retard, NULL);
       // printf("When verifHT1 > 0.01, verifhHT1 = %6.3f \n", verifHT1);
      verifHT1=0.005;
       // verifHT1 = fabs((consHT1/lectHT1)-1.0);
    } state waitAlims
    when ( verifHT1 <= 0.01 ) {
      // printf("When verifHT1 <= 0.01, verifhHT1 = %6.3f \n", verifHT1);
      /* delai atteint */
    } state syncMesure2
  }   /* waitAlims */


  //  commentee pour une seule attente sur evenement Facq
  //  state syncMesure1 {
  //    when ( efTestAndClear(flagFaisc) ) {
  //      } state syncMesure2
  //  } 

  state syncMesure2 {
    when ( efTestAndClear(flagFaisc) ) { // before: efTestAndClear(flagFaisc)
      angle=(dist*sumVHT)/(Energy*1000.);
      //pvGet(iFaisc); //Patricia commented this on Feb09 2026

      pvGet(lectHT1); //new on Feb13 2026
      pvGet(lectHT2);
      realVHT = lectHT1 - lectHT2;

      printf(" ht1 = %6.2f (%6.2f) ht2 = %6.2f (%6.2f) Vht =%7.1f (%7.1f) Pos =%6.1f I =%7.1f yp =%7.3f \n"\
      ,consHT1,lectHT1,consHT2,lectHT2,sumVHT,realVHT,lectPosition,iFaisc,angle);
      if ( saveEmit == 1 && fdemit ) {
        // utilisation variable locale sumVHT au lieu de consVHT
        // car consVHT variable DBs scan=100 ms
        fprintf(fdemit,"%7.1f  \t %7.0f \t %7.4f  \t %7.0f \t %7.0f\n",\
        lectPosition,sumVHT, angle,iFaisc,realVHT);
        if (saveWf == 1 && fdwaveform) {
          fprintf(fdwaveform, "%7.1f  \t %7.0f \t %7.0f  \t  ",\
            lectPosition,sumVHT,iFaisc);
            for(i = 0; i < Number; i++) {
               fprintf(fdwaveform, "%.4f ", iFaiscWaveform[i]);
              }
          fprintf(fdwaveform, "\n");
        }
        
      }
    } state actEmit
  }   /* syncMesure */

  /* ------------------------------------------------------------------------ */

  state actEmit {
    entry{
      //printf("State: actEmit \n");
      //fflush(stdout);
    }

    when ( (idxCur > idxEnd) && (ecrPosition < (lower+motorstep)) && delay(0.05)) {
      /* step one grid bin */
      idxCur--;

      sumVHT  = VHTgrid[idxCur];
      consHT1 =  sumVHT / 2.0;
      consHT2 = -sumVHT / 2.0;
      
      pvPut(consHT1, SYNC);
      pvPut(consHT2, SYNC);

      // pvGet(lectHT1); // commented on Feb13 2026. Moved to syncMesure2
      // pvGet(lectHT2);
      // realVHT = lectHT1 - lectHT2;
      verifHT1=1.;
      verifHT2=1.;

    } state waitAlims
    when ( idxCur <= idxEnd ) {
      printf("Emit: scan %7.1f mm completed\n\n", lectPosition);
      
      if ( saveEmit == 1 && fdemit){
          fprintf(fdemit,"\n");
          if (saveWf == 1 && fdwaveform){
            fprintf(fdwaveform, "\n"); //new
          }
      }
      if ( ecrPosition >= lower ) {
        printf("Emit: measurement completed - / Sending emit=0, HTs=0\n");
        if ( saveEmit == 1 && fdemit ) {
          fclose(fdemit);
          // saveEmit=0;
          // pvPut(saveEmit);
          if (saveWf == 1 && fdwaveform) {
            fclose(fdwaveform); //new
            saveWf=0;
            pvPut(saveWf);
          }
        }
        ecrPosition= 0.; pvPut(ecrPosition);
        consHT1=0.; pvPut(consHT1);
        consHT2=0.; pvPut(consHT2);
        actionEmit=2; pvPut(actionEmit);
      }
    } state cmdMoteur
    when ( ecrPosition >= (lower+motorstep) ) {
      actionEmit=0; pvPut(actionEmit);
      printf("Emit: measurement completed + / Sending emit=0, HTs=0\n");
      if ( saveEmit == 1 && fdemit ) {
        fclose(fdemit);
        //saveEmit=0;
        //pvPut(saveEmit);
        if (saveWf == 1 && fdwaveform) {
          fclose(fdwaveform); //new
          saveWf=0;
          pvPut(saveWf);
        }    
      }
      ecrPosition= 0.; pvPut(ecrPosition);
      consHT1=0.; pvPut(consHT1);
      consHT2=0.; pvPut(consHT2);
    } state wait
    when ( actionEmit == 0 ) {
      printf("Emit: process stopped\n");
      if ( saveEmit == 1 && fdemit) {
        fclose(fdemit);
        if ( saveWf ==1 && fdwaveform) {
          fclose(fdwaveform); //new
        }
      } 
    } state wait
  }  /* actEmit */

  /* ------------------------------------------------------------------------ */

  state pause4 {
    entry{
      // printf("State: pause4 \n");
      // fflush(stdout); 
    }
    when ( pauseEmit == 0 ) {
      printf("Emit: resumed\n");
    } state actEmit
    when ( delay(2.0) ) {
    } state pause4
  }  /* pause4 */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
} /* FIN ss mesEmit */
