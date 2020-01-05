
// #define DEBUG 1

// #define PUBLIC_VERSION	1

/*
 *  --- Global defines used by mEGA-DLC.FIM ---
 */

#define VER 		"1.0"
#define MAX_BUF	512

/*
 *  Defines used in Preferences.c
 */

#define	MAX_PREFS 4

#define PREFS_COUNTERLOOK 0
#define PREFS_COUNTERPOS  1
#define PREFS_LOCAL_DL		2
#define PREFS_SKIP_TEXT		3								// New for V0.5

#define DEF_COUNTERPOS	12

/*
 *  Bit-Defines used for dlc_flags variable
 */

#define ALLOW_LOCAL_DL 	(1<<0)						// Set if sysop allows local countings
#define SHOW_NO_OUTPUT	(1<<1)						// Set if sysop allows no text output...
