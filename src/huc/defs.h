/*	File defs.h: 2.1 (83/03/21,02:07:20) */

#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

/*
 *	INTSIZE is the size of an integer in the target machine
 *	BYTEOFF is the offset of an byte within an integer on the
 *		target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *		360 = 3)
 *	This compiler assumes that an integer is the SAME length as
 *	a pointer - in fact, the compiler uses INTSIZE for both.
 */

#define	INTSIZE	2
#define	BYTEOFF	0

/* pseudo instruction arg types */
#define T_NOP		-1
#define T_VALUE		 1
#define T_LABEL		 2
#define T_SYMBOL	 3
#define T_PTR		 4
#define T_STACK		 5
#define T_STRING	 6
#define T_LIB		 7
#define T_SIZE		 8
#define T_BANK		 9
#define T_VRAM		10
#define T_PAL		11

/* basic pseudo instructions */
#define I_LDB		 1
#define I_LDBP		 2
#define I_LDW		 3
#define I_LDWI		 4
#define I_LDWP		 5
#define I_STB		 6
#define I_STBPS		 7
#define I_STW		 8
#define I_STWPS		 9
#define I_ADDWI		 10
#define I_ADDWS		 11
#define I_ADDMI		 12
#define I_SUBWI		 13
#define I_SUBWS		 14
#define I_ORWS		 15
#define I_EORWS		 16
#define I_ANDWS		 17
#define I_ASLW		 18
#define I_ASLWS		 19
#define I_ASRW		 20
#define I_COMW		 21
#define I_NEGW		 22
#define I_SWAPW		 23
#define I_EXTW		 24
#define I_BOOLW		 25
#define I_NOTW		 26
#define I_JMP		 27
#define I_JSR		 28
#define I_RTS		 29
#define I_CALL		 30
#define I_CALLS      31
#define I_PUSHW      32
#define I_POPW       33
#define I_TSTW       34
#define I_LBRA       35
#define I_LBEQ       36
#define I_LBNE       37
#define I_BANK       38
#define I_OFFSET     39
#define I_FARPTR     40
#define I_FARPTR_I   41
#define I_FARPTR_GET 42
#define I_FGETB      43
#define I_FGETW      44
#define I_VGETW      45
#define I_VPUTW      46
#define I_NARGS      47
#define I_PHB        48
#define I_PHW        49
#define I_INCW       50
#define I_ANDWI      51
#define I_ORWI       52
#define I_ADDW       53
#define I_SUBW       54
#define I_LDUB       55
#define I_LDUBP	     56
#define I_FGETUB     57
#define I_ADDBS	     58
#define I_ADDBI      59
#define I_LABEL      60
#define I_STWIPP     61

/* optimized pseudo instructions */
#define X_MASK		0xFFFF0
#define X_LDB		0x10000
#define X_LDB_S		0x10001
#define X_LDB_P		0x10002
#define X_LDW_S		0x10011
#define X_LDD_I		0x10100
#define X_LDD_B		0x10101
#define X_LDD_W		0x10102
#define X_LDD_S_B	0x10105
#define X_LDD_S_W	0x10106
#define X_LEA_S		0x10021
#define X_PEA		0x10030
#define X_PEA_S		0x10031
#define X_PUSHW_A	0x10040
#define X_STB_S		0x10051
#define X_STW_S		0x10061
#define X_STBI_S	0x10071
#define X_STWI_S	0x10081
#define X_ADDW_S	0x10091
#define X_INCW_S	0x100A1
#define X_DECW_S	0x100B1
#define X_LDUB		0x10003
#define X_LDUB_S	0x10004
#define X_LDUB_P	0x10005


#define	FOREVER	for(;;)
#define	FALSE	0
#define	TRUE	1
#define	NO	0
#define	YES	1

/* miscellaneous */

#define	EOS	0
#define	EOL	10
#define	BKSP	8
#define	CR	13
#define	FFEED	12
#define TAB	9

#define FILENAMESIZE	256

/* symbol table parameters */

/* old values, too restrictive
 * #define	SYMSIZ	14
 * #define	SYMTBSZ	32768
 * #define	NUMGLBS	1500
 */
#define SYMTBSZ	4096
#define NUMGLBS	2048

#define	STARTGLB	symtab
#define	ENDGLB	(STARTGLB+NUMGLBS)
#define	STARTLOC	(ENDGLB+1)
#define	ENDLOC	(symtab+SYMTBSZ-1)

/* symbol table entry format */

#define NAMESIZE	26
#define NAMEMAX	25

struct symbol {
	char name[NAMESIZE];
	char ident;
	char type;
	char storage;
	char far;
	short offset;
	short tagidx;
	int size;
	int ptr_order;
};

typedef struct symbol SYMBOL;

#define NUMTAG	10

struct tag_symbol {
	char name[NAMESIZE];    // structure tag name
	int size;               // size of struct in bytes
    int member_idx;         // index of first member
    int number_of_members;  // number of tag members
};
#define TAG_SYMBOL struct tag_symbol

#define NULL_TAG 0

// Define the structure member table parameters
#define NUMMEMB		30

/* possible entries for "ident" */

#define	VARIABLE	1
#define	ARRAY	2
#define	POINTER	3
#define	FUNCTION	4

/* possible entries for "type" */

#define	CCHAR	1
#define	CINT	2
#define CVOID	3
#define CSTRUCT	4
#define CSIGNED 0
#define CUNSIGNED 8
#define CUINT (CINT | CUNSIGNED)
#define CUCHAR (CCHAR | CUNSIGNED)

/* possible entries for storage */

#define	PUBLIC	1
#define	AUTO	2
#define	EXTERN	3

#define	STATIC	4
#define	LSTATIC	5
#define	DEFAUTO	6
#define	CONST	7

#define WRITTEN 128

/* "do"/"for"/"while"/"switch" statement stack */

#define	WSTABSZ	100
#define	WSSIZ	7
#define	WSMAX	ws+WSTABSZ-WSSIZ

/* entry offsets in "do"/"for"/"while"/"switch" stack */

#define	WSSYM	0
#define	WSSP	1
#define	WSTYP	2
#define	WSCASEP	3
#define	WSTEST	3
#define	WSINCR	4
#define	WSDEF	4
#define	WSBODY	5
#define	WSTAB	5
#define	WSEXIT	6

/* possible entries for "wstyp" */

#define	WSWHILE	0
#define	WSFOR	1
#define	WSDO	2
#define	WSSWITCH	3

/* "switch" label stack */

#define	SWSTSZ	100

/* literal pool */

#define	LITABSZ	8192
#define	LITMAX	LITABSZ-1

/* input string */

#define	LITMAX2	LITABSZ-1

/* input line */

#define	LINESIZE	150
#define	LINEMAX	(LINESIZE-1)
#define	MPMAX	LINEMAX

/* macro (define) pool */

#define	MACQSIZE	16384
#define	MACMAX	(MACQSIZE-1)

/* "include" stack */

#define	INCLSIZ	3

/* statement types (tokens) */

#define	STIF	1
#define	STWHILE	2
#define	STRETURN	3
#define	STBREAK	4
#define	STCONT	5
#define	STASM	6
#define	STEXP	7
#define	STDO	8
#define	STFOR	9
#define	STSWITCH	10

#define DEFLIB	""

/* pseudo instruction structure */

typedef struct {
	long   code;
	long   type;
	long   data;
	long   imm;
	long   arg[3];
	SYMBOL *sym;
} INS;

/* constant array struct */

#define MAX_CONST        1024
#define MAX_CONST_VALUE  8192
#define MAX_CONST_DATA  65536

struct const_array {
	SYMBOL *sym;
	long   typ;
	long   size;
	long   data;
};

/* fastcall func struct */

struct fastcall {
	struct fastcall *next;
	char   fname[NAMESIZE];
	long    nargs;
	long    argsize;
	long    flags;
	char   argtype[8];
	char   argname[8][NAMESIZE];
};

SYMBOL *find_member(TAG_SYMBOL *tag, char *sname);

struct lvalue {
	SYMBOL *symbol;
	long indirect;
	long ptr_type;
	SYMBOL *symbol2;
	long value;
	TAG_SYMBOL *tagsym;
};
#define LVALUE struct lvalue

#endif
