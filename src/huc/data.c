/*	File data.c: 2.2 (84/11/27,16:26:13) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"

/* constant arrays storage */

struct const_array *const_ptr;
struct const_array  const_var[MAX_CONST];
long  const_val[MAX_CONST_VALUE];
char const_data[MAX_CONST_DATA];
long  const_val_start;
long  const_val_idx;
long  const_data_start;
long  const_data_idx;
long  const_size;
long  const_nb;

/* storage words */

SYMBOL symtab[SYMTBSZ];
SYMBOL *glbptr, *rglbptr, *locptr;
long  ws[WSTABSZ];
long  *wsptr;
long   swstcase[SWSTSZ];
long   swstlab[SWSTSZ];
long   swstp;
char  litq[LITABSZ];
char  litq2[LITABSZ];
long   litptr;
struct macro  macq[MACQSIZE];
long   macptr;
char  line[LINESIZE];
char  mline[LINESIZE];
long   lptr, mptr;

TAG_SYMBOL  tag_table[NUMTAG]; // start of structure tag table
int         tag_table_index; // ptr to next entry

SYMBOL	member_table[NUMMEMB];	// structure member table
int	member_table_index;	// ptr to next member

char  asmdefs[LITABSZ];

/* miscellaneous storage */

long	nxtlab,
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

long top_level_stkp;

FILE	*input, *input2, *output;
FILE	*inclstk[INCLSIZ];

char    inclstk_name[INCLSIZ][FILENAMESIZE];
char    fname_copy[FILENAMESIZE];
char	user_outfile[FILENAMESIZE];
long     inclstk_line[INCLSIZ];
long     line_number;

long		inclsp;
char	fname[FILENAMESIZE];

char	quote[2];
char	*cptr;
long	*iptr;
long	fexitlab;
long	iflevel, skiplevel;
long	errfile;
long	sflag;
long	cdflag;
long	verboseflag;
long	startup_incl;
long	errs;

int norecurse = 0;
long locals_ptr;

struct type *typedefs;
int typedef_ptr = 0;
