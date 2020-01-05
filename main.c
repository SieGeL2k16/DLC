/*
 *    mEGA-DLC.FIM by SieGeL (tRSi/F-iNNOVATiON) - Download Counter Tool
 *
 *         (c) 1997-98 by (tRSi-iNNOVATiONs) & IN-Ovation Software
 */

#define FIM_USERSIGMODE 0

#include <Fame/fameDoorProto.h>
#include <clib/utility_protos.h>
#include <proto/utility.h>
#include "global_defines.h"
#include "struct.h"
#include "proto.h"

STATIC 	char 	*Ver="\0$VER: FAME mEGA-DLC "VER" "__AMIGADATE__"\0";

STATIC BOOL	bgflag=FALSE;				// Indicates if door should be run in background

extern char *_ProgramName;

/* Prototypes */

int main(void);
void CloseLibs(void);
void ShutDown(long error);
void wb(char *err,BOOL ShowRegister);
BOOL ParseMainLine(void);
void GetFileList(void);

/*
 *  Main Entry Point
 */

int main(void)
	{
	struct 	RDArgs *rda=NULL;
	long    ArgArray[1]={0L};
	BOOL		exitflag=TRUE;

	*XYZ='\0';
  strcpy(XYZ,"This Version is registered to the Public Masses");

	if(rda=ReadArgs("NODE-NR./A/N",ArgArray,rda))
		{
		node=*(LONG *)ArgArray[0];
		SPrintf((STRPTR)FAMEDoorPort,"FAMEDoorPort%ld",node);
		FreeArgs(rda);rda=NULL;
		}
	else
		{
		FreeArgs(rda);
		PrintFault(IoErr(),FilePart(_ProgramName));
		Printf("\n%s is a FAME BBS-Door and could only be used via FAME !\n\n%s\n\n",FilePart(_ProgramName),XYZ);
		exit(20);
		}
	if(!(UtilityBase=(struct Library *) OpenLibrary("utility.library",37L))) { SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);exit(20);}
	if(!(FAMEBase=(struct FAMELibrary *) 	OpenLibrary(FAMENAME,0))) { SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);CloseLibs();}
	if(FIMStart(0,0UL)) CloseLibs();
 	if(!(dlc_pool=CreatePool(MEMF_PUBLIC|MEMF_CLEAR,102400L,51200L))) wb(NO_MEM,TRUE);
	if(!(mytemp1=AllocPooled(dlc_pool,MAX_BUF))) wb(NO_MEM,TRUE);
	if(ParseMainLine()==TRUE)
		{
		if(ReadDLCPrefs()==TRUE)
			{
			PutString("\n\r",1);
			if(!(dlc_flags & SHOW_NO_OUTPUT))
				{
				SPrintf(mytemp1,"mEGA-DLC [mV%s [36mby SieGeL [m([36mtRSi[m/[36mF[m-[36miNNOVATiON[m)",VER);
				Center(mytemp1,1,35);
				PutString("",1);
				Center("Download-Counter Tool for [37mF[34m.[37mA[34m.[37mM[34m.[37mE[34m.",1,32);
				PutString("",1);
	      Center(XYZ,1,34);
				}
			GetCommand("",0,0,0,SR_LogonType);
			if((MyFAMEDoorMsg->fdom_Data2!=3) && (dlc_flags & ALLOW_LOCAL_DL))
				{
   			PutString("",1);
	      if(!(dlc_flags & SHOW_NO_OUTPUT)) Center("Local Download detected, aborting...",1,35);
				bgflag=FALSE;
				wb("",FALSE);
				}
			GetFileList();
      bgflag=TRUE;				// For Shutdown(), door won't be exited yet!
			wb("",FALSE);
      AddToFileList();
			ShutDown(0);
			}
		else exitflag=FALSE;
		}
  else exitflag=FALSE;
	wb("",exitflag);
	}

/*
 *  Parses the Mainline and extracts fields mode & files to their global vars
 *  Returns TRUE if a valid download was found, else returns FALSE
 */

BOOL ParseMainLine(void)
	{
	struct 	RDArgs *myargs=NULL;
  struct 	CSource mysource;
	long   	myarray[7]={0L,0L,0L,0L,0L,0L,0L};
	char		mline[202];
	enum 		{ARG_DLSTAT=0,ARG_FILES,ARG_BYTES,ARG_CPS,ARG_EFF,ARG_MODE,ARG_INT};

	*mytemp1='\0';
	GetCommand(mline,0,0,0,NR_MainLine);
	if(!(myargs=AllocDosObject(DOS_RDARGS,NULL))) wb(NO_MEM,TRUE);
	strcat(mline,"\n");
	mysource.CS_Buffer=mline;
  mysource.CS_Length=strlen(mline);
	mysource.CS_CurChr=0;
	myargs->RDA_Source=mysource;
	myargs->RDA_Flags=RDAF_NOPROMPT;
	if(!ReadArgs("DLSTAT,FILES/N,BYTES/N,CPS/N,EFF/N,MODE/N,INT/N",myarray,myargs))
		{
		FreeDosObject(DOS_RDARGS,myargs);
		wb("\n\r[37mArgument Error !![m\n\r",TRUE);
		}
	files=*(LONG *)myarray[ARG_FILES];
	mode=*(LONG *)myarray[ARG_MODE];
	FreeDosObject(DOS_RDARGS,myargs);
	if(!files) return(FALSE);
	switch(mode)
		{
		case ULSTAT_UPLOAD_UL:
		case ULSTAT_UPLOAD_UG:
		case ULSTAT_UPLOAD_U:
		case ULSTAT_UPLOAD_RZ:
		case ULSTAT_UPLOAD_PARTUL_RESUME:
		case NORMAL_DL_XFER:							return(TRUE);
    default:													return(FALSE);
		}
	}

/*
 *  --- Termination Routines ---
 */

void wb(char *err,BOOL ShowRegister)
	{
	if(*err) PutString(err,1);
	if(ShowRegister==TRUE) PutStringFormat("[m\n\r%s\n\r",XYZ);
	FIMEnd(0);
	}

void ShutDown(long error)
	{
	if(bgflag==TRUE)
		{
		bgflag=FALSE;
		return;
		}
	CloseLibs();
	}

void CloseLibs(void)
	{
	Remove_DLC_List();
	if(UtilityBase)		CloseLibrary(UtilityBase); UtilityBase=NULL;
	if(FAMEBase) 			CloseLibrary((struct Library *)FAMEBase); FAMEBase=NULL;
	if(dlc_pool)			DeletePool(dlc_pool);dlc_pool=NULL;
	exit(0);
	}

/*
 *  GetFileList() recieves the list of downloaded files from FAME and stores
 *  the infos in the memoryblock 'FileArray'.
 */

void GetFileList(void)
	{
	struct NumFlagList *baseptr=NULL;
	long	cnt=NULL;

	if(!(FileArray=AllocPooled(dlc_pool,((files+1)*sizeof(struct MyFiles))))) wb(NO_MEM,TRUE);
	GetCommand("",0,0,0,RD_GetDldFileBase);
	baseptr=MyFAMEDoorMsg->fdom_StructDummy1;	// Recieve BasePointer
	if(baseptr)
		{
		baseptr=baseptr->Next;				// Go to first File
		}
  while(baseptr)
  	{
    FAMEStrCopy(baseptr->FileName,FileArray[cnt].FileName,101);
  	FileArray[cnt].ConfNum=baseptr->FileNumber;
		cnt++;
		baseptr=baseptr->Next;
		}
	}
