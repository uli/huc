
#define	ISOLINK_VERSION	"ISOLINK (v3.21, 2005/04/09, Denki Release)"


#if defined(DJGPP) || defined(MSDOS) || defined(WIN32)
#define PATH_SEPARATOR		'\\'
#define PATH_SEPARATOR_STRING	"\\"
#else
#define PATH_SEPARATOR		'/'
#define PATH_SEPARATOR_STRING	"/"
#endif

