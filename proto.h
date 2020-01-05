/*
 *  --- Prototypes defined for mEGA-DLC by SieGeL (tRSi-iNNOVATiONs) ---
 */

/* --- Protos defined in main.o ---*/

extern void wb(char *,BOOL);											// General exit function

/* --- Protos defined in Preferences.o --- */

extern BOOL ReadDLCPrefs(void);										// Reads in the Prefs
extern void Remove_DLC_List(void);
extern BOOL	AddPrefsItem(char *counter, long counterpos, long confnum);

/* --- Protos defined in AddToList.o ---*/

extern void AddToFileList(void);									// Adds counter to dirlisting

/* --- Protos defined in Reg_Decrypt.o --- */

#ifndef PUBLIC_VERSION

extern  VOID  __asm Decrypt(register __a0 STRPTR);

#endif


extern void WriteDebug(char *text);

