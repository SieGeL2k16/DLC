/*
 *  All global Variables used by mEGA-DLC
 */

/* Library Base Pointer */

extern struct FAMELibrary *FAMEBase;
extern struct Library	*UtilityBase;

/* Global Memory Pool Pointer, used for all Allocations */

extern APTR dlc_pool;

/* Char Pointer */

extern char *mytemp1,XYZ[80];

/* Longs */

extern long	node,									// Started from this node
      			files,								// How much files are downloaded
						mode;									// Download-Mode

extern ULONG	dlc_flags;					// Contains up to 32 switches used for preferences

/*
 *  Preferences Struct
 */

extern struct DLC_Prefs
	{
	struct 	DLC_Prefs *next;
	BOOL  	All_Conf;
	char	 	Counter[108],							// The Counter to use for ConfNum
					ConfLoc[256];							// The path to the conference
	long    ConfNum,									// The number of the conference to use
					CounterPos,								// Position of counter inside Counterstring
					ConfMaxDirs;							// Amount of dirs for this conf
	}*pr1,*pr2;

extern struct MyFiles
	{
	char  FileName[102];
	long	ConfNum;						// The conference number from where the file came from
	};

extern struct MyFiles *FileArray;

extern char *NO_MEM;
