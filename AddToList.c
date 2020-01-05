/*
 *  AddToList.c contains the functions to add the Download Counter to the
 *  filelist. Note that the FAME DoorPort is no longer valid, so there is
 *  no other chance to inform the user about any errors! All errors will
 *  be logged to 'FAME:ExternEnv/Doors/mEGA-DLC.errors' to give the SysOp
 *  a bit more overview
 */

#include <proto/utility.h>
#include "struct_ex.h"
#include "proto.h"
#include "global_Defines.h"

void 	AddToFileList(void);

STATIC void WriteError(char *);
STATIC void CutSpaces(char *s);
STATIC BOOL ReadTheDir(BPTR dp);
STATIC BOOL AddToFList(char *name, long offset,BOOL ins);
STATIC void FreeFileList(void);
STATIC long CheckAndAdd(BPTR dp,struct DLC_Prefs *);
STATIC void InsertBlankLine(BPTR dir,struct FileList *,char *counta);

/*
 *  List of Files inside the Dirlisting
 */

struct FileList
	{
	struct FileList *next;		/* Pointer to next file */
	char	FileName[32];				/* The Filename */
	long	Offset;							/* The offset in the next line */
	BOOL	Insert,							/* If TRUE, mEGA-DLC has to insert a line for the counter ! */
				Processed;					/* If TRUE, the counter for that file was added already */

	}*f1,*f2;

#define	ADD_FAILED	-1
#define ADD_RELOAD	1
#define	ADD_DONE		0

/*
 *  Main Add Function. Here finally the dirlistings will be loaded in the
 *  order gained from the Preferences. After getting one dir, all files
 *  will be checked to find out if any of the downloaded files are from
 *  this dir. If this is the case, the directory will be loaded and all
 *  files which came from the according conf will be searched and if found,
 *  the counter will be added directly below the name+size+date line of
 *  the Filelisting.
 */

void AddToFileList(void)
	{
	long		mycount=NULL;
	BPTR		dirlock=NULL;
	struct	DLC_Prefs *test=pr1;
	BOOL  	readthatconf=FALSE,stop=FALSE;
	char		mytest[256];
  STATIC	char *err_nodir="Unable to open %sDir%ld !";

	*mytemp1='\0';*mytest='\0';
	test=pr1;
  while(test)
		{
		for(mycount=0;mycount<files;mycount++)
			{
			if(FileArray[mycount].ConfNum==test->ConfNum) readthatconf=TRUE;
			}
		if(readthatconf==FALSE)
			{
			test=test->next;
			continue;
			}
		readthatconf=FALSE;
		SPrintf(mytest,"%sDir%ld",test->ConfLoc,test->ConfMaxDirs);
		if(!(dirlock=Open(mytest,MODE_READWRITE)))
			{
			SPrintf(mytemp1,err_nodir,test->ConfLoc,test->ConfMaxDirs);
			WriteError(mytemp1);
			return;
	    }
		if(ReadTheDir(dirlock)==FALSE)
      {
			Close(dirlock);
			SPrintf(mytemp1,err_nodir,test->ConfLoc,test->ConfMaxDirs);
			WriteError(mytemp1);
			return;
			}
		while(stop==FALSE)
			{
			switch(CheckAndAdd(dirlock,test))
				{
				case	ADD_RELOAD:			Close(dirlock);dirlock=NULL;
															DeleteFile(mytest);
    	                        *mytemp1='\0';
															strcpy(mytemp1,mytest);
															strcat(mytemp1,"_new");
															Rename(mytemp1,mytest);
															if(!(dirlock=Open(mytest,MODE_READWRITE)))
																{
																SPrintf(mytemp1,err_nodir,test->ConfLoc,test->ConfMaxDirs);
																WriteError(mytemp1);
																return;
														    }
                        			continue;

				default:							stop=TRUE;
															break;
				}
			}
		Close(dirlock);dirlock=NULL;
		test=test->next;
		stop=FALSE;
		FreeFileList();
		}
	}

/*
 *  Function to add the counter. If neccessary, a blank line will be added to
 *  the dirlisting to avoid overwritten filenames.
 */

STATIC long CheckAndAdd(BPTR dp,struct DLC_Prefs *dlcp)
	{
  struct 	FileList *myptr=f1;
	long		mycount=NULL,filecount=NULL;

	for(mycount=0;mycount<files;mycount++)
 		{
		if(FileArray[mycount].ConfNum==dlcp->ConfNum)
			{
			myptr=f1;
			while(myptr)
				{
				if(!Stricmp(FileArray[mycount].FileName,myptr->FileName))
					{
  				if(myptr->Processed==FALSE)
						{
						if(myptr->Insert==TRUE)
							{
							InsertBlankLine(dp,myptr,dlcp->Counter);
							myptr->Processed=TRUE;
    					return(ADD_RELOAD);
							}
						else
							{
      	    	*mytemp1='\0';
							Seek(dp,myptr->Offset+dlcp->CounterPos,OFFSET_BEGINNING);
							Read(dp,mytemp1,7);
							mytemp1[6]='\0';
							CutSpaces(mytemp1);
							StrToLong(mytemp1,&filecount);
							filecount++;
							*mytemp1='\0';
      	  	  sprintf(mytemp1,dlcp->Counter,filecount);
							Seek(dp,myptr->Offset,OFFSET_BEGINNING);
							FPuts(dp,mytemp1);
							Flush(dp);
							myptr->Processed=TRUE;
							}
						}
					}
		    myptr=myptr->next;
	      }
			}
    }
	return(ADD_DONE);
	}

/*
 *  Function to read in the already opened filelisting and fill up the list of filenames
 *  and their seekpoints. Returns FALSE if anything fails.
 */

STATIC BOOL ReadTheDir(BPTR dp)
	{
	char	localbuf[256],namebuf[32];
	long	coffset=NULL;
	BOOL	checkins=FALSE;

  *localbuf='\0';*namebuf='\0';
  while(FGets(dp,localbuf,255))
		{
		if(localbuf[25]=='-'&&localbuf[28]=='-')
			{
			coffset=Seek(dp,0L,OFFSET_CURRENT);
			FAMEStrMid(localbuf,namebuf,1,12);
			CutSpaces(namebuf);
      FAMEMemSet(localbuf,'\0',255);
     	FGets(dp,localbuf,255);
			if(localbuf[25]=='-'&&localbuf[28]=='-')
				{
				Seek(dp,coffset,OFFSET_BEGINNING);
				checkins=TRUE;
				}
			else checkins=FALSE;
			if(AddToFList(namebuf,coffset,checkins)==FALSE) return(FALSE);
			}
		}
  return(TRUE);
	}

/*
 *  Function to remove leading spaces inside the counter string
 */

STATIC void CutSpaces(char *s)
	{
	char *d=s;

	while(*d==' ') *d++;
  while(*d)
		{
		if(*d==' ') *d++;
		else *s++=*d++;
		}
	*s='\0';
	}

/*
 *  WriteError(char *) - Reports all strange things to PROGDIR:mEGA-DLC.errors
 */

STATIC void WriteError(char *errtext)
	{
	BPTR	ptr=NULL;

#ifdef DEBUG

	if(ptr=Open("RAM:TEST",MODE_READWRITE))
#else

 if(ptr=Open("FAME:ExternEnv/Doors/mEGA-DLC.errors",MODE_READWRITE))

#endif

    {
		Seek(ptr,0L,OFFSET_END);
		FPrintf(ptr,"%s\n",errtext);
		Close(ptr);
		}
	}

/*
 *  List functions for the Filename-list, Allocation & Deallocation functions
 */

STATIC BOOL AddToFList(char *name, long offset,BOOL ins)
	{
	if(!f1)
		{
		if(!(f1=AllocPooled(dlc_pool,sizeof(struct FileList)))) return(FALSE);
		f2=f1;
		}
	else
		{
		if(!(f2->next=AllocPooled(dlc_pool,sizeof(struct FileList)))) return(FALSE);
		f2=f2->next;
		}
	f2->next=NULL;
	strcpy(f2->FileName,name);
	f2->Offset=offset;
	f2->Insert=ins;
	f2->Processed=FALSE;															// TRUE only when counter was added
	return(TRUE);
	}

/*
 *  Removes the FileList from memory
 */

STATIC void FreeFileList(void)
	{
	struct FileList *help=f1;

	while(help)
		{
		f1=help;
		help=help->next;
		FreePooled(dlc_pool,f1,sizeof(struct FileList));
		}
	f1=NULL;f2=NULL;
	}

/*
 *  Function to add a blank line to the dirlisting to make room for the counter (V0.5)
 *  First the dirlisting will be copied from offset 0 until the offset given, than a
 *  blank line will be append to new copied file and after this the rest of the file
 *  will be append and the new created file will be renamed to the old filename.
 */

STATIC void InsertBlankLine(BPTR dir,struct FileList *fl,char *counta)
	{
	BPTR 		destptr=NULL;
  char  	dirname[256];
	long		origlength=NULL,orgoffset=NULL,dummy=NULL,offset=NULL,diff=NULL;
	struct	FileInfoBlock __aligned fib;
	APTR    memblock=NULL;
	struct	FileList *mycheck=f1;

	offset=fl->Offset;
	*dirname='\0';
	if(!NameFromFH(dir,dirname,249))								// Required for dest name + _new extension
		{
		WriteError("NameFromFH(): Buffer too short!");
		return;
		}
  FAMEFillMem(&fib,'\0',sizeof(struct FileInfoBlock));
  ExamineFH(dir,&fib);
	origlength=fib.fib_Size;
	*mytemp1='\0';
	if(!(memblock=AllocPooled(dlc_pool,origlength)))
		{
		WriteError("CopyMemBlock(): Out of memory !!!");
		return;
		}
	strcat(dirname,"_new");
	if(!(destptr=Open(dirname,MODE_NEWFILE)))
		{
		WriteError("Unable to open directory for reading !");
    FreePooled(dlc_pool,memblock,origlength);
		return;
		}
	orgoffset=Seek(dir,0L,OFFSET_CURRENT);
	Seek(dir,0L,OFFSET_BEGINNING);
	Read(dir,memblock,offset);
	if(Write(destptr,memblock,offset)!=offset)
		{
    Close(destptr);
		FreePooled(dlc_pool,memblock,origlength);
    WriteError("Error while writing destination directory !");
		return;
    }
	*mytemp1='\0';
  sprintf(mytemp1,counta,1);											// First download of this file
	strcat(mytemp1,"\n\0");
	FPuts(destptr,mytemp1);
	Flush(destptr);
	dummy=Read(dir,memblock,origlength);
	if(Write(destptr,memblock,dummy)!=dummy)
		{
    Close(destptr);
		FreePooled(dlc_pool,memblock,origlength);
    WriteError("Error while writing 2nd part of destination directory !");
		return;
    }
  FAMEFillMem(&fib,'\0',sizeof(struct FileInfoBlock));
	ExamineFH(destptr,&fib);
	diff=fib.fib_Size-origlength;
	Close(destptr);destptr=NULL;
  FreePooled(dlc_pool,memblock,origlength);
	while(mycheck)
		{
		if(mycheck->Processed==FALSE)
			{
			if(mycheck->Offset>offset) mycheck->Offset+=diff;	// Correct offsets of all other offsets
			}
    mycheck=mycheck->next;
		}
	}

