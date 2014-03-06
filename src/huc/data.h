/*	File data.h: 2.2 (84/11/27,16:26:11) */


#include "defs.h"

/* constant arrays storage */

extern struct const_array *const_ptr;
extern struct const_array  const_var[MAX_CONST];
extern long  const_val[MAX_CONST_VALUE];
extern char const_data[MAX_CONST_DATA];
extern long  const_val_start;
extern long  const_val_idx;
extern long  const_data_start;
extern long  const_data_idx;
extern long  const_size;
extern long  const_nb;

/* storage words */

extern	char  symtab[];
extern	char *glbptr, *rglbptr, *locptr;
extern	long   ws[];
extern	long  *wsptr;
extern	long   swstcase[];
extern	long   swstlab[];
extern	long   swstp;
extern	char  litq[];
extern	char  litq2[];
extern	long   litptr;
extern	char  macq[];
extern	long   macptr;
extern	char  line[];
extern	char  mline[];
extern	long   lptr, mptr;

extern TAG_SYMBOL  tag_table[NUMTAG]; // start of structure tag table
extern int	   tag_table_index;   // ptr to next entry

extern SYMBOL	member_table[NUMMEMB];	// structure member table
extern int	member_table_index;	// ptr to next member<

extern  char  asmdefs[];

/* miscellaneous storage */

extern	long	nxtlab,
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

extern	FILE	*input, *input2, *output;
extern	FILE	*inclstk[];

extern char    inclstk_name[INCLSIZ][20];
extern long     inclstk_line[];
extern char    fname_copy[20];
extern long     line_number;

extern	long		inclsp;
extern	char	fname[];

extern	char	quote[];
extern	char	*cptr;
extern	long	*iptr;
extern	long	fexitlab;
extern	long	iflevel, skiplevel;
extern	long	errfile;
extern	long	sflag;
extern	long	cdflag;
extern	long	verboseflag;
extern	long	startup_incl;
extern	long	errs;
