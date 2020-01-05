/*
 *  All global Variables used by mEGA-DLC
 */

/* Library Base Pointer */

struct FAMELibrary *FAMEBase=NULL;
struct Library	*UtilityBase=NULL;

/* Global Memory Pool Pointer, used for all Allocations */

APTR dlc_pool=NULL;

/* Char Pointer */

char *mytemp1=NULL,XYZ[80];

/* Longs */

long 	node=NULL,									// Started from this node
      files=NULL,									// How much files are downloaded
			mode=NULL;									// Download-Mode

ULONG	dlc_flags=NULL;							// Contains up to 32 switches used for preferences

/*
 *  Preferences Struct
 */

struct DLC_Prefs
	{
	struct 	DLC_Prefs *next;
	BOOL  	All_Conf;
	char	 	Counter[108],							// The Counter to use for ConfNum
					ConfLoc[256];							// The path to the conference
	long    ConfNum,									// The number of the conference to use
					CounterPos,								// Position of counter inside Counterstring
					ConfMaxDirs;							// Amount of dirs for this conf
	}*pr1,*pr2;

/*
 *  MyFiles contains all downloaded filenames and their conference numbers
 *  inside an array
 */

struct MyFiles
	{
	char  FileName[102];			// The filename which was downloaded
	long	ConfNum;						// The conference number from where the file came from
	};

struct MyFiles *FileArray=NULL;

/* --- ERROR-STRINGS --- */

char *NO_MEM="\n\r[37mOut of Memory !\n\r";
