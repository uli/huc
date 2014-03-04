/*	File data.c: 2.2 (84/11/27,16:26:13) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"

/* constant arrays storage */

struct const_array *const_ptr;
struct const_array  const_var[MAX_CONST];
int  const_val[MAX_CONST_VALUE];
char const_data[MAX_CONST_DATA];
int  const_val_start;
int  const_val_idx;
int  const_data_start;
int  const_data_idx;
int  const_size;
int  const_nb;

/* storage words */

char symtab[SYMTBSZ];
char *glbptr, *rglbptr, *locptr;
int  ws[WSTABSZ];
int  *wsptr;
int   swstcase[SWSTSZ];
int   swstlab[SWSTSZ];
int   swstp;
char  litq[LITABSZ];
char  litq2[LITABSZ];
int   litptr;
char  macq[MACQSIZE];
int   macptr;
char  line[LINESIZE];
char  mline[LINESIZE];
int   lptr, mptr;

char  asmdefs[LITABSZ];

/* miscellaneous storage */

int	nxtlab,
	litlab,
	stkp,
	zpstkp,
	argstk,
	ncmp,
	errcnt,
	glbflag,
	indflg,
	ctext,
	cmode,
	lastst,
	overlayflag,
	optimize,
	globals_h_in_process;

FILE	*input, *input2, *output;
FILE	*inclstk[INCLSIZ];

char    inclstk_name[INCLSIZ][FILENAMESIZE];
char    fname_copy[FILENAMESIZE];
int     inclstk_line[INCLSIZ];
int     line_number;

int		inclsp;
char	fname[FILENAMESIZE];

char	quote[2];
char	*cptr;
int	*iptr;
int	fexitlab;
int	iflevel, skiplevel;
int	errfile;
int	sflag;
int	cdflag;
int	verboseflag;
int	startup_incl;
int	errs;
