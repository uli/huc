
#define ISOLINK_VERSION "isolink (v3.21-" GIT_VERSION ", " GIT_DATE ")"


#if defined(DJGPP) || defined(MSDOS) || defined(WIN32)
#define PATH_SEPARATOR          '\\'
#define PATH_SEPARATOR_STRING   "\\"
#else
#define PATH_SEPARATOR          '/'
#define PATH_SEPARATOR_STRING   "/"
#endif

