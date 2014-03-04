/*
 * backup ram defines
 */

#define  BM_OK             0
#define  BM_NOT_FOUND      1
#define  BM_BAD_CHECKSUM   2
#define  BM_DIR_CORRUPTED  3
#define  BM_FILE_EMPTY     4
#define  BM_FULL           5
#define  BM_NOT_FORMATED   0xFF

#define  BRAM_STARTPTR     0x8010

/*
 * sprite defines
 */

#define  FLIP_X_MASK 0x08
#define  FLIP_Y_MASK 0x80
#define  FLIP_MAS   0x88
#define  SIZE_MAS   0x31

#define  NO_FLIP    0x00
#define  NO_FLIP_X  0x00
#define  NO_FLIP_Y  0x00
#define  FLIP_X     0x08
#define  FLIP_Y     0x80
#define  SZ_16x16   0x00
#define  SZ_16x32   0x10
#define  SZ_16x64   0x30
#define  SZ_32x16   0x01
#define  SZ_32x32   0x11
#define  SZ_32x64   0x31

/*
 * joypad defines
 */

#define	JOY_A		0x01
#define	JOY_B		0x02
#define	JOY_SLCT	0x04
#define	JOY_STRT	0x08
#define	JOY_UP		0x10
#define	JOY_RGHT	0x20
#define	JOY_DOWN	0x40
#define	JOY_LEFT	0x80

#define	JOY_C		0x0100
#define	JOY_D		0x0200
#define	JOY_E		0x0400
#define	JOY_F		0x0800

#define JOY_SIXBUT	0x5000


/*
 * screen defines
 */
#define	SCR_SIZE_32x32	0
#define	SCR_SIZE_64x32	1
#define	SCR_SIZE_128x32	3
#define	SCR_SIZE_32x64	4
#define	SCR_SIZE_64x64	5
#define	SCR_SIZE_128x64	7

#define	XRES_SHARP	0
#define	XRES_SOFT	4

/*
 * CD defines
 */
#define	CDPLAY_MUTE		0
#define	CDPLAY_REPEAT		1
#define	CDPLAY_NORMAL		2
#define	CDPLAY_ENDOFDISC	0

#define	CDFADE_CANCEL	0
#define	CDFADE_PCM6	8
#define	CDFADE_ADPCM6	10
#define	CDFADE_PCM2	12
#define	CDFADE_ADPCM2	14

#define CDTRK_AUDIO	0
#define CDTRK_DATA	4

/*
 * ADPCM defines
 */
#define	ADPLAY_AUTOSTOP		0
#define	ADPLAY_REPEAT		0x80

#define ADPLAY_FREQ_16KHZ	0xE
#define ADPLAY_FREQ_10KHZ	0xD
#define ADPLAY_FREQ_8KHZ	0xC
#define ADPLAY_FREQ_6KHZ	0xB
#define ADPLAY_FREQ_5KHZ	0xA

#define ADREAD_RAM	0
#define ADREAD_VRAM	0xFF

#define ADWRITE_RAM	0
#define ADWRITE_VRAM	0xFF
