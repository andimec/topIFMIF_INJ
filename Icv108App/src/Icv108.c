/******************************************************************************
 * CEA - Direction des Sciences de la Matiere - IRFU/SIS
 * CE-SACLAY, 91191 Gif-sur-Yvette, France
 *
 *
 * ADAS ICV 108 Device Support
 *
 *      Author: JFD & FG
 *      Date:   08/05/2009
 *
 */

/**
 * @file
 * @brief ADAS ICV108/178 Device Support for EPICS R3.14.
 *
 * ICV108 Device Support accepts up to 2 cards in a VME crate, starting
 * from address @b 0x8000 with an increment of 0x100. For the moment only 2 ICV178 cards are supported.
 * When we add a second ICV108 card the first ICV178 adressbase is adressbase+2*ICV178_SIZE and the number card in database start at 2 => If we use only one ICV178 card for the first ICV108 it's the same.
 *
 * ICV 108 cards are many possibilities to works. Driver works only on the FLIP/FLOP mode with a external or soft trigger
 *
 * It supports :
 * @b BO record type for configure Type of trigger (External or Soft) and process the Trigger if it is configured in Soft
 * @b BI record type to show the activity of the ICV108 card
 * @b AI record type for the average measures
 * @b AO record type to configure time of the waveform and the time during the calcul of average measure
 * @b MBBO record type to configure the sample frequency and to configure which channel to sample
 * @b WAVEFORM record type to display samples measures.
 * @b STRINGOUT record type to write information status
 * The device type @b DTYP for both record types is @b ICV108.
 *
 */

#include <vxWorks.h>
#include <math.h>
#include "vxWorksCommon.h"
#include "taskLibCommon.h"
#include <sysLib.h>
#include <vxLib.h>
#include <types.h>
#include <stdio.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <string.h>
#include <iv.h>
#include <intLib.h>
#include "semLib.h"
#include <vme.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <drvSup.h>
#include <recSup.h>
#include <recGbl.h>
#include <devSup.h>
#include <link.h>
#include <dbScan.h>
#include <waveformRecord.h>
#include <aiRecord.h>
#include <aoRecord.h>
#include <boRecord.h>
#include <biRecord.h>
#include <mbboRecord.h>
#include <stringoutRecord.h>
#include <epicsExport.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsThread.h>
#include <iocsh.h>
#include "errlog.h"
#include <string.h>
#include "time.h"
#include "tickLib.h"
#include "sysLib.h"
#include "rebootLib.h"

#define ICV178_BASE 0x084200
#define ICV178_SIZE 0x1000
#define ICV178_MAX_CARDS 3

#define ICV108_MAX_CARDS 2
#define MAX_CHANNELS 8
#define OFFBOARD_DATA_ADDR 0x83800000

/* adresse VME
devICV108: Adresse A16 (ICV108)=> f0f08000
devICV108: Adresse A24 (ICV108)=> f0700000
devICV108: Adresse A32 (ICV108)=> 83800000*/

#define ICV108A16_BASE 0x8000
#define ICV108A24_BASE 0x800000
#define ICV108A32_BASE 0x8000000

#define ICV108A16_BASE_SIZE 0x100
#define ICV108A24_BASE_SIZE 0x10000
#define ICV108A32_BASE_SIZE 0x400000

#define IT_LEVEL_Card0   1	/* interrupt level */
#define IT_LEVEL_Card1   2	/* interrupt level */

#define IT_VECTOR_Card0  0xD0	/* interrupt vector for board 0 */
#define IT_VECTOR_Card1  0xD8	/* interrupt vector for board 1 */

#define DMA_BUFFER_SIZE 4200000
#define BLOCK_ALIGN 2048
#define SUR_ECHANTILLONNAGE 16
#define MAX_OCTECT 2000000

SEM_ID semDMACom;
int NB_CARD_ICV108 = 0;

epicsUInt32 FrequenceMain = 48000;	/* Freq Timer H.OUT 48 MHz => 48Mhz/10 pour simplifier calcul */
/* n=29 => 100Khz; n=11 => 250Khz*/
/*** signal du MBBO pour changer la frequence ****/
int SIG_FREQUENCE_MBBO = 1;
/**** Signal du MBBO pour changer le mode ***/
int SIG_MODE_MBBO = 2;

int verboseICV108 = 0;
int verboseICV108_2 = 0;

int RetardSigma = 6;		// retard de 60us du aux convertisseurs SIGMA/DELTA

/**
 * \struct timespec
 *
 * timespec est un objet qui permet de mesurer un temps entre deux actions
 */
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

/**
 * \struct card_ICV108
 * \brief Object de la carte ICV108
 *
 * card_ICV108 est un objet qui permet de definir toutes les caracteristique d une carte ICV108 ( Adresse,tableaux, etc...)
 */
typedef struct PARAM_CARD_ICV108 {

	int numberCard;
	char *icv108A16local;
	char *icv108A24local;
	char *icv108A32local;

	char *icv178A24local[ICV178_MAX_CARDS];

	int NbEchDebut;
	int NbEchFin;
	int NbEch;
	int MotifDebut;
	int MotifFin;
	int nbVoiesWfGraph;
	int nbVoiesActives;
	float retard;
	int nbPointToDivis;
	int stopMain;
	int RamUsed;
	int bufferCPU;

	SEM_ID semDMA;
	SEM_ID semCALC;
	SEM_ID semFIN_CALC;
	SEM_ID semWF;
	SEM_ID semCmdProtection;

	double T0;
	double T1;
	double T2;
	double T3;

	struct waveformRecord *pRecordWf[ICV178_MAX_CARDS * MAX_CHANNELS];
	struct waveformRecord *pRecordWfGraph[ICV178_MAX_CARDS * MAX_CHANNELS];
	struct waveformRecord *pRecordWfAxeX;
	struct aiRecord *pRecordAI[ICV178_MAX_CARDS * MAX_CHANNELS];

	struct biRecord *LedRecord;
	struct stringoutRecord *statut;

	float *adrWf[ICV178_MAX_CARDS * MAX_CHANNELS];
	float *adrWfGraph[ICV178_MAX_CARDS * MAX_CHANNELS];
	float ParamConversionOPR[ICV178_MAX_CARDS * MAX_CHANNELS];
	float MoyRecAi[ICV178_MAX_CARDS * MAX_CHANNELS];

	float *pWfAbsXValue;
	int voiesActives[ICV178_MAX_CARDS * MAX_CHANNELS];

	int *voiesRecWf;

	int RapCompressWF;

	int TailleMinNELM;

	int MaskWF;
	int MaskAi;

	int nbSousMotif;
	int nbMotifs;
	int nbOctDMA2L;

	int NB_CARD_ICV178;
	int mode;
	epicsUInt32 Configuration;
	epicsUInt32 FrequenceEch;
	double periode;
	double tempPeriode;
	unsigned short IndiceFreq;
	unsigned short valConfigMbbo;
	epicsThreadId ID_thread_DMA;
	epicsThreadId ID_thread_GRAPH;
	epicsThreadId ID_thread_CONS;
	epicsThreadId ID_thread_MOY;
	int init_process;
	int config_trig;
	int trigger;
	int mbboType;
	int typeCons;
	int statutOperationnel;
	unsigned long maskVoiesDispo;
	int interruptLevel;
	int compteur;
	int finProcess;
	struct timespec PeriodeTriggerExterne;

} card_ICV108;

card_ICV108 param_ICV108[2];

static char *parmValue[5] = { "T0", "T1", "FrequenceEchao", "T2", "T3" };
static char *parmBoValue[3] = { "TriggerType", "TRIG", "START" };
static char *parmStringoutValue[1] = { "STATUS" };

typedef struct devPvt {
	int type;
} devPvt;

/***********************************/
/*********fonctions Record**********/
/***********************************/
static long init_bo_record();
static long write_bo();
static long init_bi_record();
static long read_bi();
static long init_ai_record();
static long init_mbbo_record();
static long init_ao_record();
static long get_ioint_info();
static long read_ai();
static long write_ao();
static long write_mbbo();
static long init_wf_record();
static long read_wf();
static long init();
static long init_stringout_record();
static long write_stringout();
static IOSCANPVT ioscanpvt;

/***********************************/
/*********Creation Threads**********/
/***********************************/
void ICV108_DMA(int numCardICV108);
void ICV108_GRAPHES(int numCardICV108);
void ICV108_MOYENNES(int numCardICV108);
void ICV108_LOAD_CONFIG(int numCardICV108);

void card0_icv108_int_service(int card);
void card1_icv108_int_service(int card);

/**************************************/
/*********fonctions programme**********/
/**************************************/
void Icv178Config(int numCardICV108);
void initSem(int numCardICV108);
void DeleteSem(int numCardICV108);
void Icv108Config(int numCardICV108);
void RemiseMoyenneAZero(int numCardICV108);
short VerifOverflow(double PER, int numCardICV108);
short TestConfiguration(epicsUInt32 ValConfig, int numCardICV108);
void RetardFech(int numCardICV108);
void FrequenceAcquisition(epicsUInt32 FrequenceAcq, int numCardICV108);
void CalculNombreMotif(int numCardICV108);
void ConstructionAbcisse(int numCardICV108);
void StopAcquisition(int numCardICV108);
void StartAcquisition(int numCardICV108);
void ChargementConfiguration(epicsUInt32 valconfig, int numCardICV108);
void CalculSousMotif(int numCardICV108);
int EcritureTableMotif(int numCardICV108);
int compare();
int calculNbVoies(int nbAi, int nbWf, int numCardICV108);
short VerifPrecisionMoyenne(double t3, int numCardICV108);
void CalcRapportCompressN(int numCardICV108);
void WriteStatutString(int numCardICV108, char *Status);
void EnableVMEInterrupts(int numCardICV108);
void ConfigParamConversion(int numCardICV108);
void epicsFin();
/******************************************************************************************************/
/******************************************************************************************************/
/*/////////////////////////////////////RECORD ////////////////////////////////////////////////////////*/
/******************************************************************************************************/
/******************************************************************************************************/

/***************************************************************/
/***************************************************************/
/*/////////////////////////// RECORD STRINGOUT ////////////////*/
/***************************************************************/
/***************************************************************/

/**
 * \struct devStringoutcv108
 * \brief device support pour le record "stringout"
 *
 * devStringoutIcv108 est le device support pour le record "stringout"
 */

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write;
} devStringoutIcv108 =
{
	5, NULL, NULL, init_stringout_record, NULL, write_stringout};

epicsExportAddress(dset, devStringoutIcv108);

/****************************************************************************/
/*                 init_record stringout                                    */
/****************************************************************************/
static long init_stringout_record(struct stringoutRecord *pso)
{
	int type;
	devPvt *pvt = NULL;
	char *parm;
	card_ICV108 *ptParamICV108;
	struct vmeio *pvmeio;
	int card;

	/*pso->dpvt = (void *)0; */

	/* Numero de la carte */
	card = pso->out.value.vmeio.card;

	if (card == 0)
	{
		ptParamICV108 = &param_ICV108[0];
	} else
	{
		ptParamICV108 = &param_ICV108[1];
	}

	if (pso->out.type != VME_IO)
	{
		recGblRecordError(S_db_badField, (void *)pso,
				"\ndevStringoutStats (init_record) Illegal OUT field");
		return S_db_badField;
	}

	/*on recupere le nom du param du record ICV108 de type ao */
	parm = pso->out.value.vmeio.parm;

	/*attribution d'un type pour chaque parametre de l'ICV108 */
	if (strcmp(parm, parmStringoutValue[0]) != 0)
	{
		recGblRecordError(S_db_badField, (void *)pso,
				"\ndevStringoutStats (init_ecord) Illegal OUT parm field");
		return S_db_badField;
	}

	/** Stockage du pointeur pour le record de la Led **/
	ptParamICV108->statut = pso;
	WriteStatutString(ptParamICV108->numberCard, "DETECTION CARTE... ");
	return 0;
}

/****************************************************************************/
/*                         stringoutWrite                                   */
/****************************************************************************/
static long write_stringout(struct stringoutRecord *pso)
{
	return (0);
}

/***************************************************************/
/***************************************************************/
/*/////////////////////////// RECORD BI ///////////////////////*/
/***************************************************************/
/***************************************************************/

/**
 * \struct devBiIcv108
 * \brief device support pour le record "bi"
 *
 * devBiIcv108 est le device support pour le record "bi"
 */

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_bi_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_bi;
} devBiIcv108 =
{
	5, NULL, NULL, init_bi_record, NULL, read_bi};

epicsExportAddress(dset, devBiIcv108);

/****************************************************************************/
/*                 init_record bi                                           */
/****************************************************************************/
static long init_bi_record(struct biRecord *pbi)
{
	struct vmeio *pvmeio;
	int card = 0;
	card_ICV108 *ptParamICV108;

	/* bi.inp must be an VME_IO */
	switch (pbi->inp.type)
	{
		case (VME_IO):
			break;
		default:
			recGblRecordError(S_db_badField, (void *)pbi,
					"\ndevAiIcv108 (init_ai_record) Illegal INP field");
			return (S_db_badField);
	}

	pvmeio = (struct vmeio *)&(pbi->inp.value);
	card = pvmeio->card;

	if (card == 0)
	{
		ptParamICV108 = &param_ICV108[0];
	} else
	{
		ptParamICV108 = &param_ICV108[1];
	}

	/** Stockage du pointeur pour le record de la Led **/
	ptParamICV108->LedRecord = pbi;

	return (0);
}

/****************************************************************************/
/*         read_bi                                                         */
/****************************************************************************/

static long read_bi(struct biRecord *pbi)
{
	int status = 0;
	return (status);
}

/***************************************************************/
/***************************************************************/
/*/////////////////////////// RECORD BO ///////////////////////*/
/***************************************************************/
/***************************************************************/

/*
 * Create the dset for devBoIcv108
 */

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_bo_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_bo;
} devBoIcv108 =
{
	5, NULL, NULL, init_bo_record, NULL, write_bo};

epicsExportAddress(dset, devBoIcv108);

/****************************************************************************/
/*                 init_record bo                                           */
/****************************************************************************/
static long init_bo_record(struct boRecord *pbo)
{
	int type;
	devPvt *pvt = NULL;
	char *parm;

	pbo->dpvt = (void *)0;

	if (pbo->out.type != VME_IO)
	{
		recGblRecordError(S_db_badField, (void *)pbo,
				"\ndevBoStats (init_record) Illegal OUT field");
		return S_db_badField;
	}

	/*on recupere le nom du param du record ICV108 de type bo */
	parm = pbo->out.value.vmeio.parm;

	/*attribution d'un type pour chaque parametre de l'ICV108 */
	for (type = 0; type < 3; type++)
	{
		if (strcmp(parm, parmBoValue[type]) == 0)
		{
			pvt = (devPvt *) malloc(sizeof(devPvt));
			pvt->type = type;
			if (type == 0)
			{
				pbo->rval = 1;
			}			// configuration du triger en externe
			else
			{
				pbo->val = 0;
			}
		}
	}
	if (pvt == NULL)
	{
		recGblRecordError(S_db_badField, (void *)pbo,
				"\ndevBoStats (init_record) Illegal OUT parm field");
		return S_db_badField;
	}
	/* Make sure record processing routine does not perform any conversion */
	pbo->dpvt = pvt;
	return 0;
}

/****************************************************************************/
/*         write_bo                                                         */
/****************************************************************************/

static long write_bo(struct boRecord *pbo)
{
	int status;
	unsigned short *preg;
	int card = 0;
	card_ICV108 *ptParamICV108;

	devPvt *pvt = (devPvt *) pbo->dpvt;

	/*on recupere le numero de la carte ICV108 de type bo */
	card = pbo->out.value.vmeio.card;

	ptParamICV108 = &param_ICV108[card];

	if (ptParamICV108->statutOperationnel != 1)
	{
		return (0);
	}

	/*En fonction du type de chaque bo de l'ICV108, on effectue une action */
	switch (pvt->type)
	{

		case 0:			/*Changement trigger */
			if (ptParamICV108->ID_thread_CONS == 0)
			{
				errlogSevPrintf(errlogInfo,
						"\nCHANGEMENT MODE TRIGGER >> carte %i >> valeur = %i",
						card, pbo->val);
				ptParamICV108->trigger = (pbo->val) << 14;
				ptParamICV108->config_trig = pbo->val;
				ptParamICV108->ID_thread_CONS =
					epicsThreadCreate("ICV108_CONFIG", epicsThreadPriorityHigh,
							epicsThreadGetStackSize(epicsThreadStackBig),
							(EPICSTHREADFUNC) ICV108_LOAD_CONFIG,
							ptParamICV108->numberCard);
			}
			break;

		case 1:			/*trigger soft */
			if (ptParamICV108->config_trig != 1)
			{
				errlogSevPrintf(errlogInfo, "\nTRIGGER SOFT >> carte %i", card);
				preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x34);
				*preg = 1;
			}
			break;

		case 2:
			/*STOP*/ break;
	}

	return (0);
}

/***************************************************************/
/***************************************************************/
/*/////////////////////////// RECORD AI ///////////////////////*/
/***************************************************************/
/***************************************************************/
struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_ai_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_ai;
	DEVSUPFUN special_linconv;
} devAiIcv108 =
{
	6, NULL, NULL, init_ai_record, NULL, read_ai, NULL};

epicsExportAddress(dset, devAiIcv108);

/****************************************************************************/
/*                 init_record ai                                           */
/****************************************************************************/
/* LOCAL ou static */
static long init_ai_record(struct aiRecord *pai)
{
	struct vmeio *pvmeio;
	int card = 0;
	card_ICV108 *ptParamICV108;

	/* wf.inp must be an VME_IO */
	switch (pai->inp.type)
	{
		case (VME_IO):
			pai->udf = FALSE;
			break;
		default:
			recGblRecordError(S_db_badField, (void *)pai,
					"\ndevAiIcv108 (init_ai_record) Illegal INP field");
			return (S_db_badField);
	}

	pvmeio = (struct vmeio *)&(pai->inp.value);
	card = pvmeio->card;

	/***** Les premiers numero des cartes ICV178 sont reserve pour la premiere ICV108 *****/
	if (card < ICV178_MAX_CARDS)
	{
		ptParamICV108 = &param_ICV108[0];
	} else
	{
		ptParamICV108 = &param_ICV108[1];
	}

	if (card >= (ptParamICV108->numberCard + 1) * ICV178_MAX_CARDS)
	{
		errlogSevPrintf(errlogFatal,
				"\nINIT_AI_RECORD: error card number >= %d\n",
				ICV108_MAX_CARDS);
		return (S_dev_badCard);
	}

	if (pvmeio->signal >= (MAX_CHANNELS * ptParamICV108->NB_CARD_ICV178))
	{
		errlogSevPrintf(errlogFatal,
				"\nINIT_AI_RECORD: error channel number %d card ICV178 n %i \n",
				pvmeio->signal, card);

		return (S_dev_badSignal);
	}

	errlogSevPrintf(errlogInfo,
			"\nINIT_AI_RECORD: configuration signal %s card n %i => OK ",
			pai->name, card);

	/* set linear conversion slope */
	pai->eslo = (pai->eguf - pai->egul) / 65535.0;	/* 16 bit */

	pvmeio = (struct vmeio *)&(pai->inp.value);
	ptParamICV108->pRecordAI[pvmeio->signal] = pai;

	ptParamICV108->MaskAi = ptParamICV108->MaskAi + (1 << pvmeio->signal);

	pai->mdel = -1;
	pai->adel = -1;

	return (0);
}

/****************************************************************************/
/*         read_ai                                                         */
/****************************************************************************/
static long read_ai(pai)
	struct aiRecord *pai;
{
	return (0);
}

/***************************************************************/
/***************************************************************/
/*/////////////////////////// RECORD AO ///////////////////////*/
/***************************************************************/
/***************************************************************/
struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_ao_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_ao;
	DEVSUPFUN special_linconv;
} devAoIcv108 =
{
	6, NULL, NULL, init_ao_record, NULL, write_ao, NULL};

epicsExportAddress(dset, devAoIcv108);

/****************************************************************************/
/*                 init_record ao                                           */
/****************************************************************************/

static long init_ao_record(struct aoRecord *pao)
{

	int type;
	devPvt *pvt = NULL;
	char *parm;

	if (pao->out.type != VME_IO)
	{
		recGblRecordError(S_db_badField, (void *)pao,
				"\ndevAoStats (init_record) Illegal OUT field");
		return S_db_badField;
	}

	/*on recupere le nom du param du record ICV108 de type ao */
	parm = pao->out.value.vmeio.parm;

	/*attribution d'un type pour chaque parametre de l'ICV108 */
	for (type = 0; type < 5; type++)
	{
		if (strcmp(parm, parmValue[type]) == 0)
		{
			pvt = (devPvt *) malloc(sizeof(devPvt));
			pvt->type = type;
			pao->val = 0;
		}
	}
	if (pvt == NULL)
	{
		recGblRecordError(S_db_badField, (void *)pao,
				"\ndevAoStats (init_record) Illegal OUT parm field");
		return S_db_badField;
	}
	/* Make sure record processing routine does not perform any conversion */
	pao->linr = 0;
	pao->dpvt = pvt;
	return (0);
}

/****************************************************************************/
/*                 get_ioint_info                                           */
/****************************************************************************/

/* This routine is called by the ioEventscan system each time the record is added or deleted from an I/O event scan list. cmd has the value(0,1) if the record is being ((added to,                     deleted from) an I/O event list. It must be provided for any device type that can use the ioEvent scanner. */

static long get_ioint_info(int cmd, struct waveformRecord *pr, IOSCANPVT * ppvt)
{
	*ppvt = ioscanpvt;
	return (0);
}

/****************************************************************************/
/*                 write_record ao                                           */
/****************************************************************************/

static long write_ao(struct aoRecord *pao)
{
	double OldVal = 0.;
	short status = 0;
	int card = 6;
	card_ICV108 *ptParamICV108;

	devPvt *pvt = (devPvt *) pao->dpvt;

	/*on recupere le numero de la carte ICV108 de type ao */
	card = pao->out.value.vmeio.card;

	ptParamICV108 = &param_ICV108[card];

	if (ptParamICV108->statutOperationnel != 1)
	{
		return (0);
	}

	if (pvt->type != 2)
	{
		/*verification de la valeur max (HOPR) et min (LOPR) */
		if (pao->val > pao->hopr)
		{
			pao->val = pao->hopr;
		}
		if (pao->val < pao->lopr)
		{
			pao->val = pao->lopr;
		}
	}

	/*En fonction du type de chaque parametre de l'ICV108, on effectue une action */
	switch (pvt->type)
	{

		case 0:			/*param T0 */
			errlogSevPrintf(errlogInfo,
					"\nCHANGEMENT CONSIGNE >> TO =%f, carte n %i => OK ",
					pao->val, card);
			if ((pao->val > ptParamICV108->T2) || (pao->val > 200))
			{
				pao->val = ptParamICV108->T0;
				errlogSevPrintf(errlogInfo,
						"CHANGEMENT CONSIGNE >> TO >> RIEN A FAIRE");
			} else
			{
				OldVal = ptParamICV108->T0;
				ptParamICV108->T0 = pao->val;

				/*Verification overflow */
				status =
					VerifOverflow(ptParamICV108->periode,
							ptParamICV108->numberCard);
				if (status == 1)
				{
					pao->val = OldVal;
					ptParamICV108->T0 = OldVal;
					WriteStatutString(ptParamICV108->numberCard, "OVERFLOW!");
					taskDelay(120);
					WriteStatutString(ptParamICV108->numberCard, "OK");
				} else if (ptParamICV108->ID_thread_CONS != 0)
				{
					errlogSevPrintf(errlogInfo,
							"CHANGEMENT CONSIGNE >> TO , carte n %i >> ECRITURE PAS POSSIBLE",
							card);
					pao->val = OldVal;
					ptParamICV108->T0 = OldVal;
				} else
				{
					errlogSevPrintf(errlogInfo,
							"CHANGEMENT CONSIGNE >> TO , carte n %i>> EN COURS...",
							card);
					/* dans le thread LOAD_CONFIG, cette action est equivalente au changement de mode */
					ptParamICV108->mbboType = SIG_MODE_MBBO;
					ptParamICV108->retard = ptParamICV108->T0;
					RetardFech(ptParamICV108->numberCard);
					/* creation d un thread pour arreter acquisition/charger config/restart */
					ptParamICV108->ID_thread_CONS =
						epicsThreadCreate("ICV108_CONFIG", epicsThreadPriorityHigh,
								epicsThreadGetStackSize
								(epicsThreadStackBig),
								(EPICSTHREADFUNC) ICV108_LOAD_CONFIG,
								ptParamICV108->numberCard);
				}
			}
			break;

		case 1:			/*param T1 */
			errlogSevPrintf(errlogInfo,
					"\nCHANGEMENT CONSIGNE >> T1 =%f, carte n %i => OK ",
					pao->val, card);
			OldVal = ptParamICV108->T1;
			ptParamICV108->T1 = pao->val;
			/*Verification overflow */
			status =
				VerifOverflow(ptParamICV108->periode, ptParamICV108->numberCard);
			if (status == 1)
			{
				errlogSevPrintf(errlogInfo,
						"CHANGEMENT CONSIGNE >> T1 >> OVERFLOW!!!");
				pao->val = OldVal;
				ptParamICV108->T1 = OldVal;
				WriteStatutString(ptParamICV108->numberCard, "OVERFLOW!");
				taskDelay(120);
				WriteStatutString(ptParamICV108->numberCard, "OK");
			} else if (ptParamICV108->ID_thread_CONS != 0)
			{
				errlogSevPrintf(errlogInfo,
						"CHANGEMENT CONSIGNE >> T1, carte n %i >> ECRITURE PAS POSSIBLE",
						card);
				pao->val = OldVal;
				ptParamICV108->T1 = OldVal;
			} else
			{
				/* Si le temps de mesure est superieur au temps de visu, alors le temps de visu max prend la valeur max du temps de calcul */
				if (ptParamICV108->T1 < ptParamICV108->T3)
				{
					pao->val = ptParamICV108->T3;
					ptParamICV108->T1 = pao->val;
				}
				/* creation d un thread pour arreter acquisition/charger config/restart */
				errlogSevPrintf(errlogInfo,
						"CHANGEMENT CONSIGNE >> T1, carte n %i  >> EN COURS...",
						card);
				ptParamICV108->ID_thread_CONS =
					epicsThreadCreate("ICV108_CONFIG", epicsThreadPriorityHigh,
							epicsThreadGetStackSize(epicsThreadStackBig),
							(EPICSTHREADFUNC) ICV108_LOAD_CONFIG,
							ptParamICV108->numberCard);
			}
			break;

		case 2:			/*param FrequenceEchao */
			break;

		case 3:			/*param T2 */
			if ((pao->val > ptParamICV108->T3) || (pao->val < ptParamICV108->T0))
			{
				pao->val = ptParamICV108->T2;
				errlogSevPrintf(errlogInfo,
						"CONSIGNE >> T2, carte n %i  >> RIEN A FAIRE ",
						card);
			} else
			{
				ptParamICV108->T2 = pao->val;
				errlogSevPrintf(errlogInfo,
						"\nCHANGEMENT CONSIGNE >> T2, carte n %i  >> EN COURS...",
						card);
			}
			break;

		case 4:			/*param T3 */
			if ((pao->val < ptParamICV108->T2) || (pao->val > ptParamICV108->T1))
			{
				pao->val = ptParamICV108->T3;
				errlogSevPrintf(errlogInfo,
						"CONSIGNE >> T3, carte n %i >> RIEN A FAIRE", card);
			} else
			{
				ptParamICV108->T3 = pao->val;
				errlogSevPrintf(errlogInfo,
						"\nCHANGEMENT CONSIGNE >> T3, carte n %i >> EN COURS...",
						card);
			}
			break;
	}

	return (0);
}

/***************************************************************/
/***************************************************************/
/*///////////////////// RECORD mbbo ///////////////////////////*/
/***************************************************************/
/***************************************************************/

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_mbbo;
} devMbboIcv108 =
{
	5, NULL, NULL, init_mbbo_record, NULL, write_mbbo};

epicsExportAddress(dset, devMbboIcv108);

/****************************************************************************/
/*                 init_record mbbo                                         */
/****************************************************************************/

static long init_mbbo_record(struct mbboRecord *pmbbo)
{
	pmbbo->dpvt = (void *)0;
	if (pmbbo->out.type != VME_IO)
		return ERROR;
}

/****************************************************************************/
/*                 write record mbbo                                        */
/****************************************************************************/
static long write_mbbo(struct mbboRecord *pmbbo)
{
	int status = 0;
	epicsUInt32 val = 0;
	int SIGNAL_MBBO;
	int card = 0;
	epicsUInt32 OldConfig = 0;
	struct vmeio *pvmeio;
	card_ICV108 *ptParamICV108;
	float Per = 0.000001;

	pvmeio = (struct vmeio *)&(pmbbo->out.value);

	SIGNAL_MBBO = pvmeio->signal;
	card = pvmeio->card;

	ptParamICV108 = &param_ICV108[card];

	errlogSevPrintf(errlogInfo, "\nCHANGEMENT CONSIGNE MBBO >> numbercard %i",
			ptParamICV108->numberCard);

	ptParamICV108->mbboType = SIGNAL_MBBO;

	if (ptParamICV108->statutOperationnel != 1)
	{
		errlogSevPrintf(errlogInfo,
				"\nERROR CHANGEMENT CONSIGNE MBBO >> card %i", card);
		return (0);
	}

	/*******************************/

	/***** Changement de frequence */
	/*******************************/

	if (SIGNAL_MBBO == SIG_FREQUENCE_MBBO)
	{

		ptParamICV108->FrequenceEch = pmbbo->rval;

		errlogSevPrintf(errlogInfo,
				"CHANGEMENT CONSIGNE >> FREQUENCE = %i, carte %i ",
				ptParamICV108->FrequenceEch, ptParamICV108->numberCard);

		/* Verification de la configuration avant de changer */
		status =
			VerifOverflow((1 / (1000 * ((double)ptParamICV108->FrequenceEch))),
					ptParamICV108->numberCard);

		if (status != 0)
		{
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> FREQUENCE = %i, carte %i >>> OVERFLOW!!!!!!!",
					ptParamICV108->FrequenceEch,
					ptParamICV108->numberCard);
			/*Configuration incorrecte, donc on reste comme avant */
			WriteStatutString(ptParamICV108->numberCard, "OVERFLOW!");
			taskDelay(120);
			WriteStatutString(ptParamICV108->numberCard, "OK");
			ptParamICV108->FrequenceEch = pmbbo->oraw;
			pmbbo->rval = pmbbo->oraw;
			switch (pmbbo->oraw)
			{
				case 100:
					pmbbo->val = 0;
					break;
				case 250:
					pmbbo->val = 1;
					break;
				case 1000:
					pmbbo->val = 2;
					break;
			}
		} else if (ptParamICV108->ID_thread_CONS != 0)
		{
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> FREQUENCE = %i,carte %i >> ECRITURE PAS POSSIBLE!!!!!!",
					ptParamICV108->FrequenceEch,
					ptParamICV108->numberCard);
			ptParamICV108->FrequenceEch = pmbbo->oraw;
			pmbbo->rval = pmbbo->oraw;
			switch (pmbbo->oraw)
			{
				case 100:
					pmbbo->val = 0;
					break;
				case 250:
					pmbbo->val = 1;
					break;
				case 1000:
					pmbbo->val = 2;
					break;
			}
		} else			/* Configuration correcte */
		{
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> FREQUENCE,carte  %i >> ECRITURE EN COURS ....",
					ptParamICV108->FrequenceEch,
					ptParamICV108->numberCard);
			/* creation d un thread pour arreter acquisition/charger config/restart */
			ptParamICV108->ID_thread_CONS =
				epicsThreadCreate("ICV108_CONFIG", epicsThreadPriorityHigh,
						epicsThreadGetStackSize(epicsThreadStackBig),
						(EPICSTHREADFUNC) ICV108_LOAD_CONFIG,
						ptParamICV108->numberCard);
		}
	} else
	{
		/***********************************/
		/***** Changement de configuration */
		ptParamICV108->Configuration = pmbbo->rval;
		errlogSevPrintf(errlogInfo,
				"\nCHANGEMENT CONSIGNE >> CONFIGURATION = %i,carte  %i >> ECRITURE EN COURS ....",
				ptParamICV108->Configuration,
				ptParamICV108->numberCard);

		/* Test configuration */
		status =
			TestConfiguration(ptParamICV108->Configuration,
					ptParamICV108->numberCard);

		if (status != 0)
		{			/* Test ERREUR */
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> CONFIGURATION = %i,carte  %i >> OVERFLOW!!!!",
					ptParamICV108->Configuration,
					ptParamICV108->numberCard);
			WriteStatutString(ptParamICV108->numberCard, "OVERFLOW!");
			taskDelay(120);
			WriteStatutString(ptParamICV108->numberCard, "OK");
			pmbbo->rval = pmbbo->oraw;
			pmbbo->val = ptParamICV108->valConfigMbbo;
			ptParamICV108->Configuration = pmbbo->oraw;
		} else if (ptParamICV108->ID_thread_CONS != 0)
		{
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> CONFIGURATION = %i,carte  %i >> ECRITURE PAS POSSIBLE!!!",
					ptParamICV108->Configuration,
					ptParamICV108->numberCard);
			pmbbo->rval = pmbbo->oraw;
			pmbbo->val = ptParamICV108->valConfigMbbo;
			ptParamICV108->Configuration = pmbbo->oraw;
		} else			/* test OK */
		{
			ptParamICV108->valConfigMbbo = pmbbo->val;
			errlogSevPrintf(errlogInfo,
					"CHANGEMENT CONSIGNE >> CONFIGURATION = %i,carte  %i >> ECRITURE EN COURS...",
					ptParamICV108->valConfigMbbo,
					ptParamICV108->numberCard);
			/* creation d un thread pour arreter acquisition/charger config/restart */
			ptParamICV108->ID_thread_CONS =
				epicsThreadCreate("ICV108_CONFIG", epicsThreadPriorityHigh,
						epicsThreadGetStackSize(epicsThreadStackBig),
						(EPICSTHREADFUNC) ICV108_LOAD_CONFIG,
						ptParamICV108->numberCard);
		}
	}

	return (0);
}

/***************************************************************/
/***************************************************************/
/*///////////////////// RECORD Waveform ///////////////////////*/
/***************************************************************/
/***************************************************************/
struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_wf_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_wf;
} devWfIcv108 =
{
	5, NULL, init, init_wf_record, get_ioint_info, read_wf};

epicsExportAddress(dset, devWfIcv108);
/****************************************************************************/
/*                               Fonction init driver                       */
/****************************************************************************/
static long init(int pass)
{

	unsigned short *preg;
	unsigned short dum;
	unsigned short dum2;
	int CARD_178 = 0;
	int CARD_108 = 0;
	card_ICV108 *ptParamICV108;
	int nbTotalIC178 = 0;
	STATUS success = FALSE;

	if (pass == 0)
	{
		nbTotalIC178 = 0;

		for (CARD_108 = 0; CARD_108 < ICV108_MAX_CARDS; CARD_108++)
		{
			ptParamICV108 = &param_ICV108[CARD_108];

			errlogSevPrintf(errlogInfo,
					"\ndevICV108: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  ");
			errlogSevPrintf(errlogInfo,
					"\ndevICV108: >>>>>Initialisation de la carte ICV108 (Adresse VME) n %i <<<<<<  ",
					CARD_108);
			errlogSevPrintf(errlogInfo,
					"\ndevICV108: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  ");

			ptParamICV108->TailleMinNELM = 1000000;
			ptParamICV108->T0 = 0;
			ptParamICV108->T1 = 20;
			ptParamICV108->T2 = 0;
			ptParamICV108->T3 = 0;
			ptParamICV108->FrequenceEch = 100;
			ptParamICV108->periode = 0.00001;
			ptParamICV108->IndiceFreq = 11;
			ptParamICV108->valConfigMbbo = 0;
			ptParamICV108->init_process = 1;
			ptParamICV108->config_trig = 1;
			ptParamICV108->trigger = 0x4000;
			ptParamICV108->ID_thread_DMA = 0;
			ptParamICV108->ID_thread_GRAPH = 0;
			ptParamICV108->ID_thread_MOY = 0;
			ptParamICV108->mbboType = 0;
			ptParamICV108->mode = 0;
			ptParamICV108->numberCard = CARD_108;
			ptParamICV108->statutOperationnel = -1;
			ptParamICV108->interruptLevel = 0;
			ptParamICV108->compteur = 0;
			ptParamICV108->finProcess = 0;

			/* Espace short I/O 108 esclave Mode A16 */
			if (sysBusToLocalAdrs
					(VME_AM_USR_SHORT_IO,
					 (char *)ICV108A16_BASE + (CARD_108 * ICV108A16_BASE_SIZE),
					 (char **)&ptParamICV108->icv108A16local) != OK)
			{
				continue;
			}

			/* Mode A24  */
			/* l'espace standard A24 du bus VME permet  d'acceder a la table des motifs de la carte 108 */
			if (sysBusToLocalAdrs
					(VME_AM_STD_SUP_DATA,
					 (char *)ICV108A24_BASE + (CARD_108 * ICV108A24_BASE_SIZE),
					 (char **)&ptParamICV108->icv108A24local) != OK)
			{
				continue;
			}

			if (vxMemProbe
					(ptParamICV108->icv108A16local, VX_READ, sizeof(unsigned short),
					 &dum) != OK)
			{
				errlogSevPrintf(errlogInfo,
						"\ndevICV108: >>>>>>>> Carte ICV108 NON PRESENTE<<<<<<  ");
				continue;
			} else
			{
				NB_CARD_ICV108++;
			}

			/* Mode A32  */
			/* l'espace etendu A32 du bus VME permet d'acceder aux tables de mesure de la 108 */
			if (sysBusToLocalAdrs
					(VME_AM_EXT_SUP_DATA,
					 (char *)ICV108A32_BASE + (CARD_108 * ICV108A32_BASE_SIZE),
					 (char **)&ptParamICV108->icv108A32local) != OK)
			{
				continue;
			}

			errlogSevPrintf(errlogInfo, "\ndevICV108: Adresse A16 (ICV108)=> %lx  ",
					ptParamICV108->icv108A16local);
			errlogSevPrintf(errlogInfo, "\ndevICV108: Adresse A24 (ICV108)=> %lx  ",
					ptParamICV108->icv108A24local);
			errlogSevPrintf(errlogInfo, "\ndevICV108: Adresse A32 (ICV108)=> %lx  ",
					ptParamICV108->icv108A32local);
			errlogSevPrintf(errlogInfo,
					"\ndevICV108: Initialisation de la (des) carte(s) ICV178 (Adresse VME)  ");

			/* Mode A24  */
			/* l'espace standard A24 du bus VME */
			ptParamICV108->NB_CARD_ICV178 = 0;
			for (CARD_178 = 0; CARD_178 < ICV178_MAX_CARDS; CARD_178++)
			{
				if (sysBusToLocalAdrs
						(VME_AM_STD_USR_DATA,
						 (char *)(ICV178_BASE +
							 (CARD_108 * ICV178_MAX_CARDS * ICV178_SIZE) +
							 CARD_178 * ICV178_SIZE),
						 (char **)&ptParamICV108->icv178A24local[CARD_178]) != OK)
				{
					return ERROR;
				}

				if (vxMemProbe
						(ptParamICV108->icv178A24local[CARD_178], VX_READ,
						 sizeof(unsigned short), &dum2) != OK)
				{
					break;
				} else
				{
					errlogSevPrintf(errlogInfo,
							"\ndevICV108: devICV108: Adresse A24 (ICV178) => %x  ",
							ptParamICV108->icv178A24local[CARD_178]);
					ptParamICV108->NB_CARD_ICV178++;
					nbTotalIC178++;
					if ((CARD_178 >= 1) && (ptParamICV108->NB_CARD_ICV178 == 1))
					{
						errlogSevPrintf(errlogInfo,
								"\ndevICV108: Carte ICV178 n %i inactive\n ERREUR ADRESSE CARD!!  ",
								CARD_178);
						ptParamICV108->NB_CARD_ICV178 = 0;
					} else
					{
						errlogSevPrintf(errlogInfo,
								"\ndevICV108: Carte ICV178 n %i active  ",
								CARD_178);
					}
				}
			}
			if (ptParamICV108->NB_CARD_ICV178 == 0)
			{
				return ERROR;
			} else
			{
				errlogSevPrintf(errlogInfo,
						"\ndevICV108: Nombre de cartes ICV178 active= %i  ",
						ptParamICV108->NB_CARD_ICV178);
				/* MAJ du statut operationnel de l'ensemble du couple -1 = erreur; 1 = OK */
				ptParamICV108->statutOperationnel = 1;
			}
			if (ptParamICV108->statutOperationnel == 1)
			{
				errlogSevPrintf(errlogInfo,
						"\ndevICV108: Couple de carte ICV108+ICV178 operationnel  ");
			} else
			{
				errlogSevPrintf(errlogInfo,
						"\ndevICV108: Couple de carte ICV108+ICV178 NON operationnel  ");
			}
		}

		errlogSevPrintf(errlogInfo,
				"\ndevICV108: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>><<>>>>>>>>>>>>>>>>>>>>>>  ");
		errlogSevPrintf(errlogInfo, "devICV108: Bilan des cartes detectees  ");
		errlogSevPrintf(errlogInfo,
				"\ndevICV108: ====>>>>>>>> devICV108: Nombre de cartes ICV178 active= %i <<<<<<<  ",
				nbTotalIC178);
		errlogSevPrintf(errlogInfo,
				"\ndevICV108: ====>>>>>>>> devICV108: Nombre de cartes ICV108 active= %i <<<<<<<  ",
				NB_CARD_ICV108);
		errlogSevPrintf(errlogInfo,
				"\ndevICV108: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>><<>>>>>>>>>>>>>>>>>>>>>>  ");



	}				// fin PASS=0

	if (pass == 1)
	{

		if (param_ICV108[0].statutOperationnel == 1)
		{

			/*****************************************/
			/* Activation de la premiere carte ICV108 */
			/*****************************************/

			errlogSevPrintf(errlogInfo,
					"\ndevICV108: Initialisation de la(des) carte(s) ICV178 (registre) associee(s) a la carte ICV108 n0  ");
			Icv178Config(0);

			/* creation d'une semaphore commune lorsqu'il y a deux cartes ICV108= permet de synchroniser la gestion d'interruption */
			semDMACom = semBCreate(SEM_Q_FIFO, SEM_FULL);
			errlogSevPrintf(errlogInfo, "\ndevICV108: Creation semDMACom=%i  ",
					semDMACom);

			/* creation d'une semaphore pour s assurer qu'un seul record a acces aux fonctions arret/chargement/start */
			param_ICV108[0].semCmdProtection =
				semBCreate(SEM_Q_FIFO, SEM_EMPTY);
			errlogSevPrintf(errlogInfo, "\ndevICV108: Creation semDMAcmd%i  ",
					param_ICV108[0].semCmdProtection);

			/* enable a bus interrupt level validation de l'IT au niveau du bus VME */
			ptParamICV108[0].interruptLevel = IT_LEVEL_Card0;

			if (sysIntEnable(ptParamICV108[0].interruptLevel) != OK)
			{
				errlogSevPrintf(errlogFatal, "\ndevICV108: sysIntEnable error  ");
			}

			/* allocation d'une routine d'interruption icv108_int_service pour la carte 0 */
			if (intConnect
					(INUM_TO_IVEC(IT_VECTOR_Card0), card0_icv108_int_service,
					 0) != OK)
			{
				errlogSevPrintf(errlogFatal,
						"\ndevICV108: intConnect Card 0 error  ");
			}
		}

		/*****************************************/
		/* Activation de la deuxieme carte ICV108 */
		/*****************************************/

		if (param_ICV108[1].statutOperationnel == 1)
		{

			errlogSevPrintf(errlogInfo,
					"\ndevICV108: Initialisation de la(des) carte(s) ICV178 (registre) associee(s) a la carte ICV108 n1  ");
			Icv178Config(1);

			/* creation d'une semaphore pour s assurer qu'un seul record a acces aux fonctions arret/chargement/start */
			param_ICV108[1].semCmdProtection =
				semBCreate(SEM_Q_FIFO, SEM_EMPTY);
			errlogSevPrintf(errlogInfo, "\ndevICV108: Creation semDMACmd=%i  ",
					param_ICV108[1].semCmdProtection);

			/* si la carte ICV108 n0 n est pas detecte */
			if (param_ICV108[0].statutOperationnel != 1)
			{
				semDMACom = semBCreate(SEM_Q_FIFO, SEM_FULL);
			}

			/* enable a bus interrupt level validation de l'IT au niveau du bus VME */
			ptParamICV108[1].interruptLevel = IT_LEVEL_Card1;

			if (sysIntEnable(ptParamICV108[1].interruptLevel) != OK)
			{
				errlogSevPrintf(errlogFatal, "\ndevICV108: sysIntEnable error  ");
			}

			/* allocation d'une routine d'interruption icv108_int_service pour la carte 1 */
			if (intConnect
					(INUM_TO_IVEC(IT_VECTOR_Card1), card1_icv108_int_service,
					 1) != OK)
			{
				errlogSevPrintf(errlogFatal,
						"\ndevICV108: intConnect Card 1 error  ");
			}
		}
		success = sysVmeDmaInit();
		if (success != OK)
		{
			errlogSevPrintf(errlogFatal, "\ndevICV108: SysVmeDmaInit Erreur  ");
		}
		/*desative le logger sur la console */
		eltc(0);

		rebootHookAdd(epicsFin);


	}	// fin PASS=1

	return OK;
}

/****************************************************************************/
/*                       Focntion init_wf_record                            */
/****************************************************************************/
static long init_wf_record(struct waveformRecord *pwf, int pass)
{

	struct vmeio *pvmeio;
	int card = 0;
	card_ICV108 *ptParamICV108;

	/* wf.inp must be an VME_IO */
	if (pwf->inp.type != VME_IO)
	{
		recGblRecordError(S_db_badField, (void *)pwf,
				"\ndevICV108: (init_wf_record) Illegal INP field");
		return (S_db_badField);
	}

	pvmeio = (struct vmeio *)&(pwf->inp.value);
	card = pvmeio->card;

	if (((card < ICV178_MAX_CARDS) && (pvmeio->signal < 32)) || ((card == 0) && (pvmeio->signal == 32)))
	{
		ptParamICV108 = &param_ICV108[0];
		errlogSevPrintf(errlogInfo, "\ndevICV108: signal %s card 178 n %i >> card %i ",
				pwf->name, card, ptParamICV108->numberCard);
	} else
	{
		ptParamICV108 = &param_ICV108[1];
		errlogSevPrintf(errlogInfo, "\ndevICV108: signal %s card 178 n %i >> card %i ",
				pwf->name, card, ptParamICV108->numberCard);
	}

	if (ptParamICV108->statutOperationnel != 1)
	{
		return (0);
	}

	if (card >= (ptParamICV108->numberCard + 1) * ICV178_MAX_CARDS)
	{
		errlogSevPrintf(errlogFatal,
				"\ndevICV108: INIT_WF_RECORD >> error card number >= %d\n",
				ICV108_MAX_CARDS);
		return (S_dev_badCard);
	}

	if ((pvmeio->signal >= (MAX_CHANNELS * ptParamICV108->NB_CARD_ICV178))
			&& (pvmeio->signal != 32))
	{
		errlogSevPrintf(errlogFatal,
				"\ndevICV108: INIT_WF_RECORD >> error channel number %d card ICV178 n %i\n",
				pvmeio->signal, card);
		return (S_dev_badSignal);
	}

	/* maximum number of elements is 1000000 */
	if (pwf->nelm > 1000000)
	{
		recGblRecordError(S_db_badField, (void *)pwf,
				"\ndevICV108: (init_wf_record) number of elements > 1000000");
		return (S_db_badField);
	}

	if (strcmp(pvmeio->parm, "graph") == 0)
	{
		/* Creation masque des voies disponibles par couple de carte ICV108/178 */
		if ((card == 0) || (card == 3))
		{
			ptParamICV108->maskVoiesDispo = (1 << pvmeio->signal) + ptParamICV108->maskVoiesDispo;
		} else
		{
			ptParamICV108->maskVoiesDispo = (1 << ((pvmeio->signal +  (card - (ptParamICV108->numberCard * ICV178_MAX_CARDS)) * 9)) + ptParamICV108->maskVoiesDispo);
		}
		errlogSevPrintf(errlogInfo,
				"\ndevICV108: Masque des voies disponibles  %i\n",
				ptParamICV108->maskVoiesDispo);

		if (ptParamICV108->TailleMinNELM > pwf->nelm)
		{
			ptParamICV108->TailleMinNELM = pwf->nelm;
		}
		ptParamICV108->pRecordWfGraph[pvmeio->signal] = pwf;
	} else
	{
		if (pvmeio->signal == 32)
		{
			ptParamICV108->pRecordWfAxeX = pwf;
		} else
		{
			if (pvmeio->signal < ICV178_MAX_CARDS * 8)
			{
				ptParamICV108->pRecordWf[pvmeio->signal] = pwf;
			} else
			{
				recGblRecordError(S_db_badField, (void *)pwf,
						"\ndevICV108: (init_wf_record) This signal isn't avalaible");
				return (S_db_badField);
			}
		}
	}
	return (0);
}

/****************************************************************************/
/*                               Focntion read_wf                           */
/****************************************************************************/

static long read_wf(struct waveformRecord *pwf0)
{

	return (0);
}

/******************************************************************************************************/
/******************************************************************************************************/
/*/////////////////////////////////////DRIVER/////////////////////////////////////////////////////////*/
/******************************************************************************************************/
/******************************************************************************************************/

/**
 * This function waits the Sem_DMA semaphore. When the semaphore is ready, it take it and transfers all measures from memory of ICV108 card
 * to memory of MVME5500 card. When it finish, he gives the semCALC and semWF semaphores
 */

void ICV108_DMA( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	struct rset *prset;		/* voir si utile = proc */
	UCHAR *localVmeXfer = (UCHAR *) OFFBOARD_DATA_ADDR;
	UCHAR *bufferXfer;
	STATUS success = FALSE;
	short semper;
	register unsigned short *ptV0;
	int i, nbXDma, RestX;
	unsigned short *preg;
	card_ICV108 *ptParamICV108;
	struct timespec tp, tp1, tp2;
	struct timespec diffT;

	errlogSevPrintf(errlogInfo,
			"\nDEMARRAGE Thread TRANSFERT VME/DMA carte n %i",
			numCardICV108);

	ptParamICV108 = &param_ICV108[numCardICV108];

	ptParamICV108->bufferCPU = memalign(BLOCK_ALIGN, DMA_BUFFER_SIZE);
	/* 4 Mega au total 4 194 304 = 0x400000 */
	/* 0x83800000 + 0x200000 = 0x83a00000 */
	/* buffer = memalign (2048, 4200000); */

	semper = TRUE;
	/* les parametres pr initialiser DMA st ds config.h */
	/* warning */

	while (semper == TRUE)
	{
		ptV0 = 0;

		clock_gettime(CLOCK_REALTIME, &tp);
		/* c'est la routine d'interruption qui fait le semGive */
		semTake(ptParamICV108->semDMA, WAIT_FOREVER);
		semTake(semDMACom, WAIT_FOREVER);

		if (ptParamICV108->statutOperationnel != 1)
		{
			return (0);
		}

		/* synchronisation process */
		ptParamICV108->finProcess = 0;

		tp1.tv_nsec = 0;
		clock_gettime(CLOCK_REALTIME, &tp1);
		ptParamICV108->PeriodeTriggerExterne = diff(tp, tp1);

		/*errlogSevPrintf(errlogInfo,"TRANSFERT DMA de la carte n %i",numCardICV108); */

		/*Quelle RAM est disponible la lecture? */
		preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x1C);
		ptParamICV108->RamUsed = *preg;

		ptV0 = &ptParamICV108->bufferCPU;

		nbXDma = ptParamICV108->nbOctDMA2L / 32768;

		RestX = ptParamICV108->nbOctDMA2L - (nbXDma * 32768);

		if (ptParamICV108->nbMotifs == 0)
			RestX = ptParamICV108->NbEch;	/* cas tres pt pulse  */

		bufferXfer = ptParamICV108->bufferCPU;

		if (ptParamICV108->RamUsed != 0)
		{
			localVmeXfer =
				(UCHAR *) OFFBOARD_DATA_ADDR +
				(ICV108A32_BASE_SIZE * numCardICV108);
		} else
		{
			localVmeXfer =
				(UCHAR *) (OFFBOARD_DATA_ADDR +
						(ICV108A32_BASE_SIZE * numCardICV108) + 0x200000);
		}

		for (i = 0; i < nbXDma; i++)
		{
			success = sysVmeDmaV2LCopy(localVmeXfer, bufferXfer, 32768);
			localVmeXfer = localVmeXfer + 32768;
			bufferXfer = bufferXfer + 32768;
		}

		/* quand RestX=0 pb ds sysVmeDmaLCopy  RestX=0 A REVOIR si tres pt pulse */
		if (RestX != 0)
			success = sysVmeDmaV2LCopy(localVmeXfer, bufferXfer, RestX);

		/*** process de la Led activite *******/
		ptParamICV108->LedRecord->rval = !ptParamICV108->LedRecord->rval;
		prset = (struct rset *)(ptParamICV108->LedRecord->rset);
		(*prset->process) (ptParamICV108->LedRecord);

		tp2.tv_nsec = 0;
		clock_gettime(CLOCK_REALTIME, &tp2);
		diffT = diff(tp1, tp2);

		errlogSevPrintf(errlogInfo,
				"FIN TRANSFERT DMA carte %i << compteur =%i >> temps transfert =%ld ms >>> Periode Trigger =%ld",
				numCardICV108, ptParamICV108->compteur,
				diffT.tv_nsec / 1000000,
				ptParamICV108->PeriodeTriggerExterne.tv_nsec / 1000000);
		semGive(ptParamICV108->semCALC);
		semGive(ptParamICV108->semFIN_CALC);
		semGive(ptParamICV108->semWF);
		semGive(semDMACom);

	}				/* while */
}

/***************************************************************************
 * Thread : Remplissage des waveform                                        *
 ***************************************************************************/
/**
 * This function waits the Sem_WF semaphore. When the semaphore is ready, it take it and transfers all measures on waveforms
 */
void ICV108_GRAPHES( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{

	struct rset *prset;		/* voir si utile = proc */
	register unsigned short *ptV0;
	short toujours;
	int nbMot = 0;
	short ssM = 0;
	unsigned short Data;
	int i;
	int u;
	int InitGraphXY = 1;
	int indWF;			/* indice NELM waveform complet */
	int indWFGraph;		/* indice NELM waveform graphe */
	card_ICV108 *ptParamICV108;
	int NbEchDebut = 0;
	int NbEchFin = 0;
	int NbEchDiff = 0;
	int MotifDebut = 0;
	int MotifFin = 0;
	int NbSousMotifDebut = 0;
	int NbSousMotifFin = 0;
	int indCompress = 0;
	float lastValue = 0;
	float value = 0;
	struct timespec tp, tp2;
	struct timespec diffT;

	errlogSevPrintf(errlogInfo, "DEMARRAGE Thread GRAPH >> carte n %i",
			numCardICV108);

	ptParamICV108 = &param_ICV108[numCardICV108];

	toujours = TRUE;
	while (toujours == TRUE)
	{
		ptV0 = 0;

		semTake(ptParamICV108->semWF, WAIT_FOREVER);

		errlogSevPrintf(errlogInfo, "THREAD GRAPH ACTIF >> carte n %i",
				numCardICV108);

		clock_gettime(CLOCK_REALTIME, &tp);

		/*assignment makes pointer from integer without a cast */
		ptV0 = ptParamICV108->bufferCPU;

		indWF = 0;
		indWFGraph = 0;
		NbEchDebut = ptParamICV108->NbEchDebut;
		NbEchFin = ptParamICV108->NbEchFin;
		NbEchDiff = ptParamICV108->NbEch;
		MotifDebut = ptParamICV108->MotifDebut;
		MotifFin = ptParamICV108->MotifFin;
		NbSousMotifDebut = 0;
		NbSousMotifFin = 0;
		indCompress = 0;

		for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
		{
			ptParamICV108->adrWf[i] =
				ptParamICV108->pRecordWf[ptParamICV108->voiesActives[i]]->bptr;
			ptParamICV108->adrWfGraph[i] =
				ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]->
				bptr;
		}

		NbSousMotifDebut = NbEchDebut;
		if (MotifDebut == MotifFin)
		{
			MotifFin++;
			NbSousMotifFin = NbEchFin;
		} else
		{
			NbSousMotifFin = ptParamICV108->nbSousMotif;
		}

		/******* boucle Motifs *********/
		for (nbMot = MotifDebut; nbMot <= MotifFin; nbMot++)
		{
			ptV0++;
			ptV0++;

			if (NbSousMotifDebut != 0)
			{
				ptV0 =
					ptV0 + (NbSousMotifDebut * ptParamICV108->nbVoiesActives);
			}

			/******* boucle sous Motifs *********/
			for (ssM = NbSousMotifDebut; ssM < NbSousMotifFin; ssM++)
			{
				indWF++;
				indCompress++;

				/******* boucle valeurs  *********/
				for (u = 0; u < ptParamICV108->nbVoiesActives; u++)
				{
					Data = (short)(*ptV0);
					value = (ptParamICV108->ParamConversionOPR[ptParamICV108->voiesActives[u]] * Data) + ptParamICV108->pRecordWf[ptParamICV108->voiesActives[u]]->lopr;
					*ptParamICV108->adrWf[u]++ = value;

					if (indCompress == ptParamICV108->RapCompressWF)
					{
						/*On rempli les waveforms compresses */
						*ptParamICV108->adrWfGraph[u]++ = value;
					}

					/* increment pointeur buffer */
					ptV0++;
				}// Fin FOR value

				/* on remet a 0 l'indice */
				if (indCompress == ptParamICV108->RapCompressWF)
				{
					indWFGraph++;
					indCompress = 0;
				}

			}			// Fin FOR sous motif
			NbEchDiff = NbEchDiff - (NbSousMotifFin - NbSousMotifDebut);
			if (NbEchDiff <= 0)
			{
				indCompress = 0;
				break;
			} else
			{
				if (NbEchDiff < ptParamICV108->nbSousMotif)
				{
					NbSousMotifDebut = 0;
					NbSousMotifFin = NbEchDiff;
				} else
				{
					NbSousMotifDebut = 0;
					NbSousMotifFin = ptParamICV108->nbSousMotif;
				}
			}
		}			// Fin FOR motif

		/*On rempli le weveform graph avec la derniere valeur */
		if (indWFGraph < ptParamICV108->TailleMinNELM)
		{
			for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
			{
				ptParamICV108->adrWfGraph[i] =
					ptParamICV108->pRecordWfGraph[ptParamICV108->
					voiesActives[i]]->bptr;
				ptParamICV108->adrWfGraph[i] =
					ptParamICV108->adrWfGraph[i] + indWFGraph - 1;
				lastValue = *ptParamICV108->adrWfGraph[i];
				for (u = indWFGraph; u < ptParamICV108->TailleMinNELM + 1; u++)
				{
					*ptParamICV108->adrWfGraph[i]++ = lastValue;
				}
			}
		}

		/*On process uniquement les wf de la configuration chargee => mode nominal */
		for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
		{
			/**** Process waveform complet *****/
			ptParamICV108->pRecordWf[ptParamICV108->voiesActives[i]]->nord =
				indWF;
			prset =
				(struct rset *)(ptParamICV108->
						pRecordWf[ptParamICV108->voiesActives[i]]->
						rset);
			(*prset->process) (ptParamICV108->
					pRecordWf[ptParamICV108->voiesActives[i]]);

			/**** Process waveform graphe *****/
			ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]->
				nord = ptParamICV108->TailleMinNELM;
			prset =
				(struct rset *)(ptParamICV108->
						pRecordWfGraph[ptParamICV108->voiesActives[i]]->
						rset);
			(*prset->process) (ptParamICV108->
					pRecordWfGraph[ptParamICV108->voiesActives[i]]);
		}

		/* Modification d waveform X lors du changement de la configuration */
		if (InitGraphXY != 0)
		{
			ConstructionAbcisse(ptParamICV108->numberCard);
			InitGraphXY = 0;
		}

		/* Si cette tache termine avant le thread des Moyennes alors elle reactive les interruptions */
		semTake(ptParamICV108->semFIN_CALC, WAIT_FOREVER);

		/* Si cette tache termine avant le thread des Moyennes alors elle reactive les interruptions */
		/*while(ptParamICV108->finProcess!=1) {taskDelay(2);} */

		EnableVMEInterrupts(ptParamICV108->numberCard);

		tp2.tv_nsec = 0;
		clock_gettime(CLOCK_REALTIME, &tp2);
		diffT = diff(tp, tp2);

		/*creation VME interruptions */
		errlogSevPrintf(errlogInfo,
				"THREAD GRAPH FIN >> carte n %i >> temps du transfert =%ld ms",
				numCardICV108, diffT.tv_nsec / (1000000));

	}				/* while */
}

/***************************************************************************
 * Thread : Calcul de la moyenne et remplissage des ai                      *
 ***************************************************************************/
/**
 * This function waits the Sem_CALC semaphore. When the semaphore is ready, it take it and does all averages
 */
void ICV108_MOYENNES( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	struct rset *prset;		/* voir si utile = proc */
	register unsigned short *ptV1;
	short toujours;
	int nbMot;
	short ssM;
	unsigned short Data1;
	int i;
	int u;

	int NbEchDebut = 0;
	int NbEchFin = 0;
	int NbEchDiff = 0;
	int MotifDebut = 0;
	int MotifFin = 0;
	int SousMotifDebut = 0;
	int SousMotifFin = 0;

	card_ICV108 *ptParamICV108;

	errlogSevPrintf(errlogInfo, "DEMARRAGE Thread CALC >> carte n %i",
			numCardICV108);
	ptParamICV108 = &param_ICV108[numCardICV108];

	toujours = TRUE;
	while (toujours == TRUE)
	{

		semTake(ptParamICV108->semCALC, WAIT_FOREVER);
		semTake(ptParamICV108->semFIN_CALC, WAIT_FOREVER);

		errlogSevPrintf(errlogInfo, "THREAD CALC ACTIF >> carte n %i",
				numCardICV108);

		ptV1 = 0;

		/*assignment makes pointer from integer without a cast */
		ptV1 = ptParamICV108->bufferCPU;

		/* calcul Nombre echantillons */
		NbEchDebut =
			ptParamICV108->T2 / (ptParamICV108->periode * 1000) -
			(ptParamICV108->T0 / (ptParamICV108->periode * 1000));
		NbEchFin =
			ptParamICV108->T3 / (ptParamICV108->periode * 1000) -
			(ptParamICV108->T0 / (ptParamICV108->periode * 1000));;
		NbEchDiff = NbEchFin - NbEchDebut;
		if ((NbEchDiff == 0) && ((ptParamICV108->T3 - ptParamICV108->T2) != 0))
		{
			NbEchFin = NbEchFin + 1;
			NbEchDiff = 1;
		}
		ptParamICV108->nbPointToDivis = NbEchFin - NbEchDebut;
		/*if(verboseICV108)printf("\ndevICV108: Nombre de points a moyennee=%i",ptParamICV108->nbPointToDivis); */

		MotifDebut = 0;
		MotifFin = 0;

		while (NbEchDebut >= 0)
		{
			MotifDebut++;
			NbEchDebut = NbEchDebut - ptParamICV108->nbSousMotif;
			if (NbEchDebut <= 0)
			{
				MotifDebut--;
				NbEchDebut = ptParamICV108->nbSousMotif + NbEchDebut;
				break;
			}
		}

		while (NbEchFin >= 0)
		{
			MotifFin++;
			NbEchFin = NbEchFin - ptParamICV108->nbSousMotif;
			if (NbEchFin <= 0)
			{
				MotifFin--;
				NbEchFin = ptParamICV108->nbSousMotif + NbEchFin;
				break;
			}
		}

		/*On saute les valeurs inutiles */
		for (nbMot = 0; nbMot < MotifDebut; nbMot++)
		{
			ptV1++;
			ptV1++;
			for (ssM = 0; ssM < ptParamICV108->nbSousMotif; ssM++)
			{
				for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
				{
					ptV1++;
				}
			}
		}

		/*on initialise le tableau des moyennes 0 */
		for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
		{
			ptParamICV108->MoyRecAi[i] = 0.;
		}

		SousMotifDebut = NbEchDebut;
		if ((MotifDebut == MotifFin) && (NbEchDiff != 0))
		{
			MotifFin++;
			SousMotifFin = NbEchFin;
		} else
		{
			SousMotifFin = ptParamICV108->nbSousMotif;
		}

		/******* boucle Motifs *********/
		for (nbMot = MotifDebut; nbMot <= MotifFin; nbMot++)
		{
			ptV1++;
			ptV1++;

			if (SousMotifDebut != 0)
			{
				ptV1 = ptV1 + (SousMotifDebut * ptParamICV108->nbVoiesActives);
			}

			/******* boucle sous Motifs *********/
			for (ssM = SousMotifDebut; ssM < SousMotifFin; ssM++)
			{
				/******* boucle valeurs  *********/
				for (u = 0; u < ptParamICV108->nbVoiesActives; u++)
				{
					Data1 = (short)(*ptV1);
					ptParamICV108->MoyRecAi[u] =
						ptParamICV108->MoyRecAi[u] + Data1;
					/* increment pointeur buffer */
					ptV1++;
				}		// Fin FOR value

			}			// Fin FOR sous motif
			NbEchDiff = NbEchDiff - (SousMotifFin - SousMotifDebut);
			if (NbEchDiff <= 0)
			{
				break;
			} else
			{
				if (NbEchDiff < ptParamICV108->nbSousMotif)
				{
					SousMotifDebut = 0;
					SousMotifFin = NbEchDiff;
				} else
				{
					SousMotifDebut = 0;
					SousMotifFin = ptParamICV108->nbSousMotif;
				}
			}
		}			// Fin FOR motif

		if (ptParamICV108->nbPointToDivis > 0)
		{
			for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
			{
				ptParamICV108->pRecordAI[ptParamICV108->voiesActives[i]]->rval =
					ptParamICV108->MoyRecAi[i] / ptParamICV108->nbPointToDivis;
				prset =
					(struct rset *)(ptParamICV108->
							pRecordAI[ptParamICV108->voiesActives[i]]->
							rset);
				(*prset->process) (ptParamICV108->
						pRecordAI[ptParamICV108->voiesActives[i]]);
			}
		} else
		{
			RemiseMoyenneAZero(ptParamICV108->numberCard);
		}

		/* synchronisation process entre le calcul et l'affichage */
		semGive(ptParamICV108->semFIN_CALC);

		errlogSevPrintf(errlogInfo, "THREAD CALC FIN >> carte n %i",
				numCardICV108);

		/* synchronisation process */
		ptParamICV108->finProcess = 1;

	}				/* while */

}

/***************************************************************************
 * Thread : Remplissage des waveform                                        *
 ***************************************************************************/
/**
 * This function waits the Sem_WF semaphore. When the semaphore is ready, it take it and transfers all measures on waveforms
 */
void ICV108_LOAD_CONFIG( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	card_ICV108 *ptParamICV108;
	unsigned short *preg;
	ptParamICV108 = &param_ICV108[numCardICV108];
	int cases = 0;
	int status = 0;

	errlogSevPrintf(errlogInfo, "LOAD CONFIG >> carte n %i", numCardICV108);

	if (ptParamICV108->init_process == 1)
	{
		errlogSevPrintf(errlogInfo, "INIT");
	}

	/* au demarrage lors du chargement de la premiere configuration; les process n'existe pas d'ou cette ce test */
	if (ptParamICV108->init_process != 1)
	{

		/* prends la semaphore pour passer une commande */
		semTake(ptParamICV108->semCmdProtection, WAIT_FOREVER);

		/*errlogSevPrintf(errlogInfo,"Mode nominal >> carte n %i",numCardICV108); */

		/* Tue toutes les threads */
		while ((taskIdVerify(ptParamICV108->ID_thread_DMA) != ERROR)
				&& (taskIdVerify(ptParamICV108->ID_thread_GRAPH) != ERROR)
				&& (taskIdVerify(ptParamICV108->ID_thread_MOY) != ERROR))
		{
			/* avant d'arreter verification si en cours d acquisition; si oui, attends la fin
			   while(((unsigned short *)(ptParamICV108->icv108A16local + 0x06)&& 0x8000)==0){taskDelay(1);} */

			/* Arrete la carte ICV108/ICV178 */
			StopAcquisition(ptParamICV108->numberCard);

			if (taskDeleteForce(ptParamICV108->ID_thread_DMA) == ERROR)
				errlogSevPrintf(errlogFatal, "TaskDelete DMA failed\n");

			if (taskDeleteForce(ptParamICV108->ID_thread_MOY) == ERROR)
				errlogSevPrintf(errlogFatal, "TaskDelete MOY failed\n");

			if (taskDeleteForce(ptParamICV108->ID_thread_GRAPH) == ERROR)
				errlogSevPrintf(errlogFatal, "TaskDelete GRAPH failed\n");

		}

		/* suppression des semaphores associe aux graphes/DMA/moyennes */
		DeleteSem(numCardICV108);

		/* Liberation de la memoire du buffer de 4M */
		cfree(ptParamICV108->bufferCPU);

		WriteStatutString(ptParamICV108->numberCard, "STOP");
	}

	/* etape init ok */
	ptParamICV108->init_process = 0;

	/* changement frequence */
	FrequenceAcquisition(ptParamICV108->FrequenceEch,
			ptParamICV108->numberCard);

	RemiseMoyenneAZero(ptParamICV108->numberCard);
	ChargementConfiguration(ptParamICV108->Configuration,
			ptParamICV108->numberCard);

	/*calcul du nombre de motif */
	CalculNombreMotif(ptParamICV108->numberCard);

	/* chargement config ICV108 */
	Icv108Config(ptParamICV108->numberCard);

	/* redemarre l'acquisition */
	StartAcquisition(ptParamICV108->numberCard);

	/* relache semaphore pour les commandes */
	semGive(ptParamICV108->semCmdProtection);

	errlogSevPrintf(errlogInfo, "LOAD CONFIG FIN >> carte n %i", numCardICV108);

	ptParamICV108->ID_thread_CONS = 0;
	exit(0);
}

/****************************************************************************/
/*                Fonction calcul pour les conversions                      */
/****************************************************************************/
/**
 * This function calcul pour les conversions
 */
void ConfigParamConversion( /** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[numCardICV108];
	unsigned int i = 0;

	for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
	{
		ptParamICV108->ParamConversionOPR[ptParamICV108->voiesActives[i]] = ((ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]->hopr) - (ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]-> lopr)) / 65535.;
		errlogSevPrintf(errlogInfo, "LOAD ParamConversionOPR >> carte n %i >> valeur OPR=%f", numCardICV108,ptParamICV108->ParamConversionOPR[ptParamICV108->voiesActives[i]]);
	}
}

/****************************************************************************/
/*                Fonction ecriture statut                                 */
/****************************************************************************/
/**
 * This function writes a string in the status record
 */
void WriteStatutString( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108, char *statutString)
{
	struct rset *prset;		/* voir si utile = proc */
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[numCardICV108];

	/*** process du statut *******/
	strcpy(ptParamICV108->statut->val, statutString);
	prset = (struct rset *)(ptParamICV108->statut->rset);
	(*prset->process) (ptParamICV108->statut);
}

/****************************************************************************/
/*                Fonction calcul retard                                    */
/****************************************************************************/
/**
 * This function writes the delay between Start signal and Start of acquisition
 */
void RetardFech( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	unsigned short *preg;
	float pas = 0.;
	float IndiceRetard = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	/*Calcul du pas de la frequence pour le retard */
	pas = (float)(48 * ptParamICV108->periode) / SUR_ECHANTILLONNAGE;

	/*Calcul de l'indice du retard */
	IndiceRetard = ((ptParamICV108->retard) / (pas * 1000)) + RetardSigma;

	/*registre retard sur START */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x2C);
	*preg = IndiceRetard;
}

/****************************************************************************/
/*                Fonction changement de frequence en Live                  */
/****************************************************************************/
/**
 * This function changes the Frequency of acquisition
 */
void FrequenceAcquisition( /** [in] Freq Ech. Valid value: 100,250,1000 */
		epicsUInt32 FrequenceAcq,
		/** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{

	unsigned short *preg;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	if (FrequenceAcq == 100)
	{
		RetardSigma = 6;
	}				// retard de 132us du aux convertisseurs SIGMA/DELTA
	else if (FrequenceAcq == 250)
	{
		RetardSigma = 6;
	}				// retard de 60us du aux convertisseurs SIGMA/DELTA
	else
	{
		RetardSigma = 6;
	}				// retard de 18us du aux convertisseurs SIGMA/DELTA

	/*Calcul de la periode d'acquisition */
	ptParamICV108->periode = (double)(1 / (FrequenceAcq * 1000.));

	/*Calcul de l'indice de frequence */
	ptParamICV108->IndiceFreq =
		(unsigned short)((FrequenceMain - (FrequenceAcq * 16)) /
				(FrequenceAcq * 16));

	if (ptParamICV108->retard != 0)
	{
		RetardFech(ptParamICV108->numberCard);
	}

}

/****************************************************************************/
/*                Verifie coherence de T1-T0  (Overflow?)                   */
/****************************************************************************/
/**
 * This function checks if the total number of samples is not too big for the 4M memory of the ICV178 card.
 * If it is too big, there isn't modification of the card configuration
 */
short VerifOverflow( /** [in] Period*/ double PER,
		/** [in] Freq Ech. Valid value: 100,250,1000 */
		int numCardICV108)
{
	short status = 0;
	float Pulse = 0.;
	int NbMotif = 0;
	unsigned long MaxOctets = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	errlogSevPrintf(errlogInfo, "VERIF OVERFLOW >> carte n %i", numCardICV108);

	/*T1-T0= largeur de mesure */
	Pulse = ptParamICV108->T1 - ptParamICV108->T0;

	/*si Freq =1000Hz alors verifier que la configuration = 4 Voies, pas plus */
	if ((1 / (PER * 1000)) == 1000)
	{
		if (ptParamICV108->nbVoiesActives > 4)
		{
			errlogSevPrintf(errlogInfo,
					"VERIF OVERFLOW >> FREQ=1MHz & NbVoies >4");
			status = 1;
			return status;
		}
	}

	/* Calcul du nombre de motifs = Nombre d'echantillons/Nb sous motif */
	NbMotif = ((Pulse / (PER * 1000))) / (ptParamICV108->nbSousMotif) + 1;
	errlogSevPrintf(errlogInfo, "VERIF OVERFLOW >> Nb motifs = %i", NbMotif);

	/*overflow memoire > 2MOctets */
	MaxOctets =
		NbMotif *
		((ptParamICV108->nbVoiesActives * ptParamICV108->nbSousMotif) + 2) * 2;
	errlogSevPrintf(errlogInfo, "VERIF OVERFLOW >> MaxOctets = %i", MaxOctets);

	/*remplissage max de la memoire flash de l'ICV108 */
	if (MaxOctets > MAX_OCTECT)
	{
		status = 1;
	}
	errlogSevPrintf(errlogInfo, "VERIF OVERFLOW FIN >> carte n %i",
			numCardICV108);
	return status;
}

/****************************************************************************/
/*                Verifie coherence de T3-T2                              */
/****************************************************************************/
/**
 * This function checks if T2 and T3 parameters for average calculation are OK
 */
short VerifPrecisionMoyenne( /** [in] T3 parameter */ double t3,
		/** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{
	short status = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	if (((t3 -ptParamICV108->T2) / (ptParamICV108->periode * 1000 * ptParamICV108->nbSousMotif)) <= 0)
	{
		status = 1;
	}
	return status;
}

/****************************************************************************/
/*                Calcul rapport de compression des waveforms               */
/****************************************************************************/
/**
 * This function calculate the compression rate for the waveform display
 */
void CalcRapportCompressN( /** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{
	int i = 0;
	int res = 0;
	int rapportDiv = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	//calcul du nombre d'echantillons par voie*/
	res = ptParamICV108->nbMotifs * ptParamICV108->nbSousMotif;
	/*if(verboseICV108){printf("\nNb Motifs %i, Nb sousMotifs %i,nombre d'echantillons %i,ptParamICV108->TailleMinNELM %i carte n %i",ptParamICV108->nbMotifs,ptParamICV108->nbSousMotif,res,ptParamICV108->TailleMinNELM,ptParamICV108->numberCard);} */

	if (res < ptParamICV108->TailleMinNELM)
	{
		rapportDiv = 1;
		ptParamICV108->RapCompressWF = rapportDiv;
	} else
	{
		rapportDiv = 2;
		for (i = 1; i < 500; i++)
		{
			if (res < ptParamICV108->TailleMinNELM)
			{
				ptParamICV108->RapCompressWF = rapportDiv;
				break;
			}
			res = res / 2;
			rapportDiv = rapportDiv * 2;
		}
	}

	/*if(verboseICV108){printf("\ndevICV108: Nombre de points apres compression =%i pts, soit un rapport de compression R=%i",res,ptParamICV108->RapCompressWF);} */
}

/****************************************************************************/
/*                Calcul du nombre de motifs                            */
/****************************************************************************/
/**
 * This function calculate the number of patterns
 */
void CalculNombreMotif( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	unsigned short *preg;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	ptParamICV108->NbEchDebut =
		ptParamICV108->T0 / (ptParamICV108->periode * 1000);

	ptParamICV108->NbEchFin =
		(ptParamICV108->T1 / (ptParamICV108->periode * 1000)) + 1;

	/*Calcul du nombre d'chantillons */
	ptParamICV108->NbEch = ptParamICV108->NbEchFin - ptParamICV108->NbEchDebut;

	ptParamICV108->MotifDebut = 0;
	ptParamICV108->MotifFin = 0;

	while (ptParamICV108->NbEchDebut >= 0)
	{
		ptParamICV108->MotifDebut++;
		ptParamICV108->NbEchDebut =
			ptParamICV108->NbEchDebut - ptParamICV108->nbSousMotif;
		if (ptParamICV108->NbEchDebut <= 0)
		{
			ptParamICV108->MotifDebut--;
			ptParamICV108->NbEchDebut =
				ptParamICV108->nbSousMotif + ptParamICV108->NbEchDebut;
			break;
		}
	}

	while (ptParamICV108->NbEchFin >= 0)
	{
		ptParamICV108->MotifFin++;
		ptParamICV108->NbEchFin =
			ptParamICV108->NbEchFin - ptParamICV108->nbSousMotif;
		if (ptParamICV108->NbEchFin <= 0)
		{
			ptParamICV108->MotifFin--;
			ptParamICV108->NbEchFin =
				ptParamICV108->nbSousMotif + ptParamICV108->NbEchFin;
			break;
		}
	}

	/* Calcul du nombre de motifs = Nombre d'echantillon/Nb sous motif */
	if (ptParamICV108->MotifFin == ptParamICV108->MotifDebut)
	{
		ptParamICV108->nbMotifs = 1;
	} else
	{
		ptParamICV108->nbMotifs =
			ptParamICV108->MotifFin - ptParamICV108->MotifDebut + 1;
	}

	ptParamICV108->nbOctDMA2L =
		(ptParamICV108->nbMotifs * ptParamICV108->nbSousMotif *
		 ptParamICV108->nbVoiesActives * 2) + (ptParamICV108->nbMotifs * 4);

}

/****************************************************************************/
/*                Fonction construction abcisse                             */
/****************************************************************************/
/**
 * This function creates the waveform abscissa
 */
void ConstructionAbcisse( /** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{
	struct rset *prset;		/* voir si utile = proc */
	int i = 0;
	int u = 0;
	int cal = 0;
	int retard = 0;
	double periodeCompress = 0.0;
	double LastValue = 0.0;
	card_ICV108 *ptParamICV108;

	/*errlogSevPrintf(errlogInfo,"CONSTRUCTION AXE ABCISSE >> carte n %i",numCardICV108); */
	ptParamICV108 = &param_ICV108[numCardICV108];
	/*if(verboseICV108){printf("\nEntree ConstructionAbcisse carte n %i",ptParamICV108->numberCard);}
	  if(verboseICV108){printf("\ndevICV108: nbMotifs=%i,nbSousMotif=%i , RapCompressWF=%i",ptParamICV108->nbMotifs,ptParamICV108->nbSousMotif,ptParamICV108->RapCompressWF);} */

	/*Calcul du nombre (entier) de points par graphe */
	cal = ptParamICV108->NbEch / ptParamICV108->RapCompressWF;
	/*Calcul du modulo (reste de points par graphe) */
	cal = cal + (ptParamICV108->NbEch % ptParamICV108->RapCompressWF);
	/*if(verboseICV108){printf("\ndevICV108: Nombre de points (Abcisse) =%i",cal);} */

	periodeCompress = ptParamICV108->periode * ptParamICV108->RapCompressWF;

	/*Calcul du nombre de point pour le retard */
	retard = ptParamICV108->T0 / (periodeCompress * 1000);

	ptParamICV108->pWfAbsXValue = ptParamICV108->pRecordWfAxeX->bptr;

	/*Remplissage du tableau des abcisses */
	for (i = retard; i < cal + retard; i++)
	{
		*ptParamICV108->pWfAbsXValue++ = periodeCompress * i * 1000;
	}

	ptParamICV108->pWfAbsXValue--;
	LastValue = *ptParamICV108->pWfAbsXValue;
	ptParamICV108->pWfAbsXValue++;

	/*Remplissage du tableau des abcisses (reste) */
	for (u = cal + retard; u <= ptParamICV108->TailleMinNELM; u++)
	{
		*ptParamICV108->pWfAbsXValue++ = LastValue;
	}

	/*Process l'axe de abcisses */
	ptParamICV108->pRecordWfAxeX->nord = ptParamICV108->TailleMinNELM;
	prset = (struct rset *)(ptParamICV108->pRecordWfAxeX->rset);
	(*prset->process) (ptParamICV108->pRecordWfAxeX);

	/*errlogSevPrintf(errlogInfo,"AXE ABCISSE CONSTRUIT >> carte n %i",numCardICV108); */
}

/****************************************************************************/
/*       Configuration des registres de la carte Icv178                     */
/****************************************************************************/
/**
 * This function configures all basic registers of the ICV178 card
 */

void Icv178Config( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	unsigned short *preg178;
	int card = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	for (card = 0; card < ptParamICV108->NB_CARD_ICV178; card++)
	{
		preg178 =
			(unsigned short *)(ptParamICV108->icv178A24local[card] + 0x02);
		*preg178 = 0x800;
		preg178 =
			(unsigned short *)(ptParamICV108->icv178A24local[card] + 0x08);
		*preg178 = 0x0101;
		preg178 =
			(unsigned short *)(ptParamICV108->icv178A24local[card] + 0x0A);
		*preg178 = 0x0101;
		preg178 =
			(unsigned short *)(ptParamICV108->icv178A24local[card] + 0x0C);
		*preg178 = 0x0101;
		preg178 =
			(unsigned short *)(ptParamICV108->icv178A24local[card] + 0x0E);
		*preg178 = 0x0101;
	}
}

/***************************************************************************
 * Creation des semaphores						   *
 ***************************************************************************/
/**
 * This function creates all semaphores
 */
void initSem( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[numCardICV108];
	ptParamICV108->semDMA = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	ptParamICV108->semCALC = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	ptParamICV108->semFIN_CALC = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	ptParamICV108->semWF = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
}

/***************************************************************************
 * Suppression des semaphores	                                        *
 ***************************************************************************/
/**
 * This function deletes all semaphores
 */
void DeleteSem( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];
	semDelete(ptParamICV108->semDMA);
	semDelete(ptParamICV108->semCALC);
	semDelete(ptParamICV108->semWF);
	semDelete(ptParamICV108->semFIN_CALC);
}

/****************************************************************************/
/*     routine d'interruption carte n0 => icv108_int_service               */
/****************************************************************************/
/**
 * This function creates the interrupt (end of acquisition) for the card number 1
 */
void card0_icv108_int_service( /** [in] ICV108 card number. Valid range: 1 */
		int card)
{
	unsigned short *preg;
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[card];
	/* inhibition interruption ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
	*preg = 0;
	ptParamICV108->compteur++;
	semGive(ptParamICV108->semDMA);
}

/****************************************************************************/
/*     routine d'interruption carte n1 => icv108_int_service               */
/****************************************************************************/
/**
 * This function creates the interrupt (end of acquisition) for the card number 2
 */
void card1_icv108_int_service( /** [in] ICV108 card number. Valid range: 2 */
		int card)
{
	unsigned short *preg;
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[card];
	/* inhibition interruption ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
	*preg = 0;
	ptParamICV108->compteur++;
	semGive(ptParamICV108->semDMA);
}

/****************************************************************************/
/*                   Configuration des registres de la carte Icv108         */
/****************************************************************************/
/**
 *  This function configures all basic registers of ICV108 card and interruptions vectors
 */
void Icv108Config( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	unsigned short *preg;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	/*errlogSevPrintf(errlogInfo,"CONFIG ICV108 ACTIF >> carte n %i",numCardICV108); */

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x02);
	*preg = 0x8001 + (numCardICV108 * 0x100);

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x04);
	*preg = 0x801 + (numCardICV108 * 0x40);

	/* mode de fonctionnement */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x10);
	*preg = 0x0020;

	if (numCardICV108 == 0)
	{
		ptParamICV108->interruptLevel = IT_LEVEL_Card0;
		/* Configuration  interruption VECTOR */
		preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
		*preg = 0 + IT_VECTOR_Card0;
	} else
	{
		ptParamICV108->interruptLevel = IT_LEVEL_Card1;
		/* Configuration  interruption VECTOR */
		preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
		*preg = 0 + IT_VECTOR_Card1;
	}

	/* Ces 2 registres d'adresses VME master PF et pf servent ecrire l'adresse de base VME 32 bits la 108
	   va commencer son transfert a la fin du premier buffer acquis */
	/* comme on n'est pas en mode DMA maitre je mets 0 et 0 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x08);
	*preg = 0x0000;

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x0A);
	*preg = 0x0000;

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x0C);
	*preg = 0x0000;		/* nb de buffers PF */

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x0E);
	/*    j'essaie=0  doc dit 0  */
	/* Chgt conforme dvTrg */
	*preg = 0x0001;		/* nb de buffers pf 1 buffer de 25 ms Ds ce cas l'anneau memoire de la CPU est slt 1 buffer */
	/* bus differentiel */

	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x14);
	/*preg=0x7E; */

	/* Ecriture du nombre de motif dans le registre 0x26 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x26);
	*preg = ptParamICV108->nbMotifs;

	/* delay pulse start */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x2C);
	*preg = 0;

	/* delay pulse sync */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x2E);
	*preg = 0;

	/* delay gate */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x30);
	*preg = 0;

	/* delay freq echantillonnage */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x32);
	*preg = 0;

	/*Synchronisation des convertisseurs sigma Delta */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x3A);
	*preg = 0x0001;

	taskDelay(1);

	/* frequence d'echantillonnage */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x20);
	*preg = ptParamICV108->IndiceFreq;

	/* frequence F1 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x22);
	*preg = 0;

	/* DATE PILOTE date externe inhibe en sortie J2/J3 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x12);	/* page 62 */
	*preg = 0x0001;

	/*errlogSevPrintf(errlogInfo,"CONFIG ICV108 FIN >> carte n %i",numCardICV108); */
}

/****************************************************************************/
/*                        Arret acquisition                                 */
/****************************************************************************/
/**
 *  This function stop the acquisition
 */
void StopAcquisition( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	unsigned short *preg;
	STATUS status;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	/*errlogSevPrintf(errlogInfo,"ARRET CARTE EN COURS >> carte n %i",numCardICV108); */

	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x36);
	*preg = 0x0001;
	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x36);
	*preg = 0x0001;
	/* INHIB INTERRUPT CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
	*preg = 0;
	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x36);
	*preg = 0x0001;
	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x36);
	*preg = 0x0001;
	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x36);
	*preg = 0x0001;
	/* Changement mode de fonctionnement = null */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x10);
	*preg = 0x0020;
	/* mode de fonctionnement =  0 => ARRET CARTE ICV108 */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x3C);
	*preg = 0x0001;

	/*errlogSevPrintf(errlogInfo,"CARTE ARRETE >> carte n %i",numCardICV108); */
}

/****************************************************************************/
/*                        Remise a zero des moyennes (ai)                      */
/****************************************************************************/

void RemiseMoyenneAZero( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	struct rset *prset;
	int i = 0;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
	{
		ptParamICV108->pRecordAI[ptParamICV108->voiesActives[i]]->rval = 32767;
		prset =
			(struct rset *)(ptParamICV108->
					pRecordAI[ptParamICV108->voiesActives[i]]->rset);
		(*prset->process) (ptParamICV108->
				pRecordAI[ptParamICV108->voiesActives[i]]);
	}

}

/****************************************************************************/
/*                        Lancement acquisition                             */
/****************************************************************************/
/**
 *  This function start the acquisition
 */
void StartAcquisition( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{

	/*errlogSevPrintf(errlogInfo,"DEMARRAGE CARTE EN COURS >> carte n %i",numCardICV108); */
	unsigned short *preg;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	initSem(ptParamICV108->numberCard);

	/*calcul du rapport de compression des waveform */
	CalcRapportCompressN(ptParamICV108->numberCard);

	taskDelay(20);

	if (ptParamICV108->numberCard == 0)
	{
		/*thread calcul moyenne */
		ptParamICV108->ID_thread_MOY =
			epicsThreadCreate("ICV108_MOYENNES_0", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_MOYENNES,
					ptParamICV108->numberCard);
		/* errlogSevPrintf(errlogInfo,"START ICV108_MOYENNES_0 => %i", ptParamICV108->ID_thread_MOY); */
		/*thread construction du waveform */
		ptParamICV108->ID_thread_GRAPH =
			epicsThreadCreate("ICV108_GRAPHES_0", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_GRAPHES,
					ptParamICV108->numberCard);
		/*errlogSevPrintf(errlogInfo,"START ICV108_GRAPHES_0 => %i", ptParamICV108->ID_thread_GRAPH); */
		/*thread transfert VME -> DMA */
		ptParamICV108->ID_thread_DMA =
			epicsThreadCreate("ICV108_DMA_0", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_DMA,
					ptParamICV108->numberCard);
		/*errlogSevPrintf(errlogInfo,"START ICV108_DMA_0 => %i", ptParamICV108->ID_thread_DMA); */
	} else
	{
		/*thread calcul moyenne */
		ptParamICV108->ID_thread_MOY =
			epicsThreadCreate("ICV108_MOYENNES_1", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_MOYENNES,
					ptParamICV108->numberCard);
		/*errlogSevPrintf(errlogInfo, "START ICV108_MOYENNES_1 => %i", ptParamICV108->ID_thread_MOY);*/
		/*thread construction du waveform */
		ptParamICV108->ID_thread_GRAPH =
			epicsThreadCreate("ICV108_GRAPHES_1", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_GRAPHES,
					ptParamICV108->numberCard);
		/*errlogSevPrintf(errlogInfo, "START ICV108_GRAPHES_1 => %i", ptParamICV108->ID_thread_GRAPH);*/
		/*thread transfert VME -> DMA */
		ptParamICV108->ID_thread_DMA =
			epicsThreadCreate("ICV108_DMA_1", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackBig),
					(EPICSTHREADFUNC) ICV108_DMA,
					ptParamICV108->numberCard);
		/*errlogSevPrintf(errlogInfo, "START ICV108_DMA_1 => %i", ptParamICV108->ID_thread_DMA);*/
	}

	ptParamICV108->finProcess = 0;;

	WriteStatutString(ptParamICV108->numberCard, "OK");
	taskDelay(20);

	/*creation VME interruptions */
	EnableVMEInterrupts(ptParamICV108->numberCard);

	taskDelay(20);

	errlogSevPrintf(errlogInfo, "CARTE DEMARRE >> carte n %i", numCardICV108);
}

/****************************************************************************/
/*                        Creation interruption VME                         */
/****************************************************************************/
/**
 *  This function create VME interruptions
 */
void EnableVMEInterrupts( /** [in] ICV108 card number. Valid range: 1 to 2 */
		int numCardICV108)
{
	unsigned short *preg;
	card_ICV108 *ptParamICV108;
	ptParamICV108 = &param_ICV108[numCardICV108];
	unsigned short test;
	/*errlogSevPrintf(errlogInfo,"ACTIVATION INTERRUPTIONS >> carte %i",numCardICV108); */
	if (numCardICV108 == 0)
	{
		/* activation interruption */
		preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
		*preg = ptParamICV108->interruptLevel + IT_VECTOR_Card0;
	} else
	{
		/* Autorisation IT */
		preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x06);
		*preg = ptParamICV108->interruptLevel + IT_VECTOR_Card1;
	}

	/* mode de fonctionnement */
	preg = (unsigned short *)(ptParamICV108->icv108A16local + 0x10);
	*preg = 0x0020 | ptParamICV108->trigger | ptParamICV108->mode;

	/*errlogSevPrintf(errlogInfo,"INTERRUPTIONS ACTIVEES >> carte %i",numCardICV108); */
}

/****************************************************************************/
/*                        Fonction pour QSORT()                             */
/****************************************************************************/
int compare(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

/****************************************************************************/
/* Test de la nouvelle configuration : verifie la compatibilite avec la memoire dispo  */
/****************************************************************************/

short TestConfiguration(epicsUInt32 ValConfig, int numCardICV108)
{
	card_ICV108 *ptParamICV108;
	epicsUInt32 maskVoies = 1;
	int InterVal = 0;
	int i = 0;
	int status = 0;
	int nombreVoies = 0;
	int voiesMaxMotif = 0;
	int nbSousMotif = 0;
	float Pulse = 0;
	int nbMotif = 0;
	unsigned long maxOctets = 0;

	/*errlogSevPrintf(errlogInfo,"TEST CONFIGURATION >> carte %i",numCardICV108); */

	ptParamICV108 = &param_ICV108[numCardICV108];

	/* test si la nouvelle valeur de config est coherente  c est a dire superieure a 0 */
	if (ValConfig > 1)
	{

		for (i = 0; i < (MAX_CHANNELS * ICV178_MAX_CARDS); i++)
		{
			InterVal = ValConfig & maskVoies & ptParamICV108->maskVoiesDispo;
			if (InterVal == maskVoies)
			{
				nombreVoies++;
			}
			maskVoies = maskVoies << 1;
		}

		/*si Freq =1000Hz alors verifier que la configuration = 1 Voie, pas plus */
		if (ptParamICV108->FrequenceEch == 1000)
		{
			if (nombreVoies > 1)
			{
				status = 1;
				errlogSevPrintf(errlogInfo,
						"VERIF OVERFLOW >> FREQ=1MHz & NbVoies >1");
				return status;
			}
		}

		/* Calcul du nombre de sous motif */
		while (voiesMaxMotif < 39)
		{
			nbSousMotif++;
			voiesMaxMotif = nombreVoies * nbSousMotif;
		}

		/* Verif Overflow */
		/*T1-T0= largeur de mesure */
		Pulse = ptParamICV108->T1 - ptParamICV108->T0;

		/* Calcul du nombre de motifs = Nombre d'echantillons/Nb sous motif */
		nbMotif =
			((Pulse / (ptParamICV108->periode * 1000))) / (nbSousMotif) + 1;
		/*errlogSevPrintf(errlogInfo,"TEST CONFIGURATION >> nbMotif = %i",nbMotif); */

		/*overflow memoire > 2MOctets */
		maxOctets = nbMotif * ((nombreVoies * nbSousMotif) + 2) * 2;
		/*errlogSevPrintf(errlogInfo,"TEST CONFIGURATION >> maxOctets = %i",maxOctets); */

		/*remplissage max de la memoire flash de l'ICV108 */
		if (maxOctets > MAX_OCTECT)
		{
			status = 1;
		}
	} else
	{
		status = 1;
	}

	/*errlogSevPrintf(errlogInfo,"TEST CONFIGURATION FIN >> carte %i",numCardICV108); */

	return status;
}

/****************************************************************************/
/*  ent de la nouvelle configuration dans le tableau "configuration"  */
/****************************************************************************/

void ChargementConfiguration(epicsUInt32 ValConfig, int numCardICV108)
{
	epicsUInt32 maskVoies = 1;
	epicsUInt32 InterVal = 0;
	int i = 0;
	int NbNelmInter = 1000000;
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	ptParamICV108->nbVoiesActives = 0;

	/*errlogSevPrintf(errlogInfo,"CHARGEMENT CONFIGURATION ACTIF >> carte %i",numCardICV108);   */

	/***** traitement des voies actives ******************/
	for (i = 0; i < (MAX_CHANNELS * ICV178_MAX_CARDS); i++)
	{
		ptParamICV108->voiesActives[ptParamICV108->nbVoiesActives] = -1;
		InterVal = ValConfig & maskVoies & ptParamICV108->maskVoiesDispo;
		if (InterVal == maskVoies)
		{
			ptParamICV108->voiesActives[ptParamICV108->nbVoiesActives] = i;
			ptParamICV108->nbVoiesActives++;
		}
		maskVoies = maskVoies << 1;
	}

	for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
	{
		/* errlogSevPrintf(errlogInfo,"CHARGEMENT CONFIGURATION >> WF voie %i",ptParamICV108->voiesActives[i]);      */
	}

	/* Permet de detrminer le NELM par defaut avec tous les waveform */
	for (i = 0; i < ptParamICV108->nbVoiesActives; i++)
	{
		if (ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]->
				nelm < NbNelmInter)
		{
			NbNelmInter =
				ptParamICV108->pRecordWfGraph[ptParamICV108->voiesActives[i]]->
				nelm;
		}
	}
	ptParamICV108->TailleMinNELM = NbNelmInter;

	/*tri du tableau global */
	qsort(ptParamICV108->voiesActives, ptParamICV108->nbVoiesActives,
			sizeof(int), compare);

	// Calcul du nombre de sous motifs
	CalculSousMotif(ptParamICV108->numberCard);

	// Ecriture dans la table des motifs
	EcritureTableMotif(numCardICV108);

	// Configuration conversion des valeurs
	ConfigParamConversion(numCardICV108);

	/* errlogSevPrintf(errlogInfo,"CONFIGURATION CHARGEE >> carte %i",numCardICV108); */
}

/****************************************************************************/
/*Calcul du nombre de sous-motifs    dans la variable globale nbSousMotif*/
/****************************************************************************/

void CalculSousMotif( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	card_ICV108 *ptParamICV108;

	ptParamICV108 = &param_ICV108[numCardICV108];

	ptParamICV108->nbSousMotif = 0;
	int voiesMaxMotif = 0;

	/*errlogSevPrintf(errlogInfo,"CALCUL SOUS MOTIFS >> carte %i",numCardICV108);       */
	while (voiesMaxMotif < 39)
	{
		ptParamICV108->nbSousMotif++;
		voiesMaxMotif =
			ptParamICV108->nbVoiesActives * ptParamICV108->nbSousMotif;
	}
	/*errlogSevPrintf(errlogInfo,"CALCUL SOUS MOTIFS >> nbSousMotif = %i",ptParamICV108->nbSousMotif); */
}

/****************************************************************************/
/*  Fonction ecriture du tableau "configuration" dans la table des motifs   */
/****************************************************************************/
/**
 *  This function writes the pattern table
 */
int EcritureTableMotif( /** [in] ICV108 card number. Valid range: 1 to 2 */ int
		numCardICV108)
{
	int sm = 0;
	int v = 0;
	unsigned short *pMotif;
	int n = 0;
	card_ICV108 *ptParamICV108;
	unsigned long voiesActivesAvecCoupleur[ICV178_MAX_CARDS * 8];
	int coupleur = 0;

	ptParamICV108 = &param_ICV108[numCardICV108];

	/*errlogSevPrintf(errlogInfo,"ECRITURE TABLE MOTIFS ACTIF >> carte %i",numCardICV108);      */

	pMotif = ptParamICV108->icv108A24local;
	*pMotif++ = 0x5F04;
	*pMotif++ = 0x1F05;

	/* association des voies avec leur coupleur */
	for (n = 0; n < ICV178_MAX_CARDS * 8; n++)
	{
		voiesActivesAvecCoupleur[n] = ptParamICV108->voiesActives[n];
	}
	for (n = 0; n < ptParamICV108->nbVoiesActives; n++)
	{
		coupleur = 0;
		while (voiesActivesAvecCoupleur[n] > 7)
		{
			voiesActivesAvecCoupleur[n] = voiesActivesAvecCoupleur[n] - 8;
			coupleur++;
		}
		coupleur = coupleur << 8;
		voiesActivesAvecCoupleur[n] = coupleur + voiesActivesAvecCoupleur[n];
	}

	/*Ecriture du Premier sous motif et du numero de voie */
	for (n = 0; n < ptParamICV108->nbVoiesActives; n++)
	{
		*pMotif++ = 0x0000 + voiesActivesAvecCoupleur[n];
	}

	/*Sous motif suivants */
	for (sm = 1; sm < ptParamICV108->nbSousMotif - 1; sm++)
	{
		*pMotif++ = 0x4000 + voiesActivesAvecCoupleur[0];
		for (v = 1; v < ptParamICV108->nbVoiesActives; v++)
		{
			*pMotif++ = voiesActivesAvecCoupleur[v];
		}
	}

	/*Dernier sous motif */
	if (ptParamICV108->nbVoiesActives != 1)
	{
		*pMotif++ = 0x4000 + voiesActivesAvecCoupleur[0];
		for (n = 1; n < ptParamICV108->nbVoiesActives - 1; n++)
		{
			*pMotif++ = 0x0000 + voiesActivesAvecCoupleur[n];
		}
		*pMotif++ =
			0x8000 + voiesActivesAvecCoupleur[ptParamICV108->nbVoiesActives -
			1];
	} else
	{
		*pMotif++ =
			0xC000 + voiesActivesAvecCoupleur[ptParamICV108->nbVoiesActives -
			1];
	}

	/*errlogSevPrintf(errlogInfo,"ECRITURE TABLE MOTIFS FIN >> carte %i",numCardICV108); */

	return OK;
}

/****************************************************************************/
/*  Fonction arret des cartes lors du reboot VME                            */
/****************************************************************************/
/**
 *  This function stop all cards before the VME reboot
 */
void epicsFin()
{
StopAcquisition(0);
StopAcquisition(1);
}
