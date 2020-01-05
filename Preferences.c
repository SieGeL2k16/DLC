/*
 *  Preferences.c - All Preferences-related functions for mEGA-DLC
 */

#include <proto/utility.h>
#include "struct_ex.h"
#include "proto.h"
#include "global_Defines.h"

/*
 *  Prototypes for Preferences.c
 */

BOOL 		ReadDLCPrefs(void);
void 		Remove_DLC_List(void);
BOOL 		AddPrefsItem(char *counter, long counterpos, long confnum);

STATIC 	long	CheckTheLine(char *pre,long confnum);

/*
 *  All possible Preferences Shortcuts and their Template definitions
 */

STATIC char *prefsarray[MAX_PREFS]={"COUNTERLOOK.%ld=","COUNTERPOS.%ld=","ALLOW_LOCAL_DOWNLOADS=","SKIP_ALL_OUTPUT="};
STATIC char *templates[MAX_PREFS]={"COUNTERLOOK.%ld/F","COUNTERPOS.%ld/N","ALLOW_LOCAL_DOWNLOADS/F","SKIP_ALL_OUTPUT/F"};

/*
 *  ReadDLCPrefs() parses the cfg file and builts a list of all found entries
 *  into struct DLC_Prefs *. If no cfg file could be found, all conferences
 *  will be used and the list will be filled with all conference infos
 *  required to work on the dir files.
 */

BOOL ReadDLCPrefs(void)
	{
	STATIC	char *defcounter="Downloaded->%ld";
	BPTR		prefspointer=NULL;
	char		mybuf[256];
	struct	RDArgs *myargs=NULL;
	struct	CSource mysource;
	long    myarray[]={0L},maxconfs=NULL,myretcode=NULL,confcheck=NULL;
	STATIC 	BOOL check=FALSE;

  pr1=NULL;pr2=NULL;
	GetCommand("",0,0,0,SR_NumberOfConfs);
	maxconfs=MyFAMEDoorMsg->fdom_Data2;
	if(!(prefspointer=Open("FAME:ExternEnv/Doors/mEGA-DLC.cfg",MODE_OLDFILE)))
		{
		for(myretcode=1;myretcode<maxconfs+1;myretcode++)
			{
			if(AddPrefsItem(defcounter,DEF_COUNTERPOS,myretcode)==FALSE) wb(NO_MEM,TRUE);
			}
		}
	else
		{
		if(!(myargs=AllocDosObject(DOS_RDARGS,NULL)))
			{
			Close(prefspointer);
  	  wb(NO_MEM,TRUE);
			}
		*mybuf='\0';
		while((FGets(prefspointer,mybuf,255))&&check==FALSE)
			{
			myretcode=-1;
			for(confcheck=1;confcheck<maxconfs+1;confcheck++)
				{
				myretcode=CheckTheLine(mybuf,confcheck);
				if(myretcode>-1) break;
				}
			if(myretcode<0||confcheck>maxconfs) continue;
			myarray[0]=NULL;
			mysource.CS_Buffer=mybuf;
			mysource.CS_Length=strlen(mybuf);
			mysource.CS_CurChr=0;
			myargs->RDA_Source=mysource;
			myargs->RDA_Flags=RDAF_NOPROMPT;
			*mytemp1='\0';
			SPrintf(mytemp1,templates[myretcode],confcheck);
	    if(!ReadArgs(mytemp1,myarray,myargs)) continue;
			else if(myarray[0])
				{
				switch(myretcode)
					{
					case	PREFS_COUNTERLOOK:  *mybuf='\0';
																		SPrintf(mybuf,"%s",myarray[0]);
                                  	if(AddPrefsItem(mybuf,0,confcheck)==FALSE) check=TRUE;
                      							break;

					case	PREFS_COUNTERPOS:		myretcode=*(long *)myarray[0];
																		if(AddPrefsItem("",myretcode,confcheck)==FALSE) check=TRUE;
							                      break;
					case	PREFS_LOCAL_DL:			*mybuf='\0';
																		SPrintf(mybuf,"%s",myarray[0]);
																		if(!Stricmp(mybuf,"NO")) dlc_flags|=ALLOW_LOCAL_DL;
																		else dlc_flags &= ~ALLOW_LOCAL_DL;
																		break;
					case	PREFS_SKIP_TEXT:		*mybuf='\0';
																		SPrintf(mybuf,"%s",myarray[0]);
																		if(!Stricmp(mybuf,"NO")) dlc_flags&=~SHOW_NO_OUTPUT;
																		else dlc_flags |=SHOW_NO_OUTPUT;
					}
				}
			}
		Close(prefspointer);
		FreeDosObject(DOS_RDARGS,myargs);
  	if(check==TRUE) wb("\n\r[37mAn error occured while parsing the config ![m\n\r",TRUE);
		}
	return(TRUE);
	}

/*
 *  Checks 'line' if any of the preferences keywords could be found and
 *  returns the index number of the prefsarray if any matching string could
 *  be found, else the Config will be checked for Conference Name Settings (0)
 */

STATIC LONG CheckTheLine(char *line,long confnum)
	{
	register long i=NULL;

	for(i=0;i<MAX_PREFS;i++)
		{
		*mytemp1='\0';
		SPrintf(mytemp1,prefsarray[i],confnum);
		if(!Strnicmp(line,mytemp1,strlen(mytemp1))) return(i);
		}
	return(-1L);
	}

/*
 *  List functions for the Preferences - Fills up the list with all required
 *  values.
 */

BOOL AddPrefsItem(char *counter, long counterpos, long num)
	{
	struct	DLC_Prefs *help=pr1;
	BOOL		checkforconf=FALSE;

	help=pr1;
	while(help)
		{
		if(help->ConfNum==num)
			{
			checkforconf=TRUE;
			break;
			}
		else help=help->next;
		}
	if(checkforconf==FALSE)
		{
		if(!pr1)
			{
			if(!(pr1=(struct DLC_Prefs *) AllocPooled(dlc_pool,sizeof(struct DLC_Prefs)))) return(FALSE);
			pr2=pr1;
			}
		else
			{
			if(!(pr2->next=(struct DLC_Prefs *) AllocPooled(dlc_pool,sizeof(struct DLC_Prefs)))) return(FALSE);
			pr2=pr2->next;
			}
		pr2->ConfNum=num;
		GetCommand(pr2->ConfLoc,num,0,0,SR_ConfLocation);
		GetCommand("",num,0,0,AR_NumberofDirs);
		pr2->ConfMaxDirs=MyFAMEDoorMsg->fdom_Data2;
		}
	else pr2=help;
	if(*counter) FAMEStrCopy(counter,pr2->Counter,107);
	if(counterpos) pr2->CounterPos=counterpos;
	return(TRUE);
	}

/*
 *  Removes DLC_Prefs List from Memory
 */

void Remove_DLC_List(void)
	{
	struct DLC_Prefs *d=pr1;

	while(d)
		{
		pr1=d;
		d=d->next;
		FreePooled(dlc_pool,pr1,sizeof(struct DLC_Prefs));
		}
	pr1=NULL; pr2=NULL;
	}
