#ifndef _AC_H
#define _AC_H

/*
 * Arcade Card fastcall defines
   Arcade Card inline defines
 */


/*
 *	Unused test. single 24bit argument passing.
 */
#pragma fastcall test( dword cl:ax|bx  ) nop;


/*
 *	Arcade card address reg function: 24bit value, 1 byte(high) and 1 word(mid/low). 
 */ 

void __fastcall __nop ac_addr_reg0( unsigned char high<ac_reg_1_high>, unsigned int low<ac_reg_1_low> );
void __fastcall __nop ac_addr_reg1( unsigned char high<ac_reg_2_high>, unsigned int low<ac_reg_2_low> );
void __fastcall __nop ac_addr_reg2( unsigned char high<ac_reg_3_high>, unsigned int low<ac_reg_3_low> );
void __fastcall __nop ac_addr_reg3( unsigned char high<ac_reg_4_high>, unsigned int low<ac_reg_4_low> );


/*
 *	Arcade card full reg function: address byte high, address word mid/low, word offset,
 *	word increment, byte control. *Inlined*
 *
 */
void __fastcall __nop ac_full_reg0( unsigned char high<ac_reg_1_high>, unsigned int low<ac_reg_1_low>, unsigned int offset_l<ac_reg_1_offset_l>, unsigned int incrmt_l<ac_reg_1_incrmt_l>, unsigned char control<ac_reg_1_control>);
void __fastcall __nop ac_full_reg1( unsigned char high<ac_reg_2_high>, unsigned int low<ac_reg_2_low>, unsigned int offset_l<ac_reg_2_offset_l>, unsigned int incrmt_l<ac_reg_2_incrmt_l>, unsigned char control<ac_reg_2_control>);
void __fastcall __nop ac_full_reg2( unsigned char high<ac_reg_3_high>, unsigned int low<ac_reg_3_low>, unsigned int offset_l<ac_reg_3_offset_l>, unsigned int incrmt_l<ac_reg_3_incrmt_l>, unsigned char control<ac_reg_3_control>);
void __fastcall __nop ac_full_reg3( unsigned char high<ac_reg_4_high>, unsigned int low<ac_reg_4_low>, unsigned int offset_l<ac_reg_4_offset_l>, unsigned int incrmt_l<ac_reg_4_incrmt_l>, unsigned char control<ac_reg_4_control>);


/*
 *	Arcade card increment reg function: word size. *Inlined*
 */
void __fastcall __nop ac_inc_reg0( unsigned int incrmt_l<ac_reg_1_incrmt_l> );
void __fastcall __nop ac_inc_reg1( unsigned int incrmt_l<ac_reg_2_incrmt_l> );
void __fastcall __nop ac_inc_reg2( unsigned int incrmt_l<ac_reg_3_incrmt_l> );
void __fastcall __nop ac_inc_reg3( unsigned int incrmt_l<ac_reg_4_incrmt_l> );


/*
 *	Arcade card offset reg function: word size. *Inlined*
 */
void __fastcall __nop ac_offset_reg0( unsigned int offset_l<ac_reg_1_offset_l> );
void __fastcall __nop ac_offset_reg1( unsigned int offset_l<ac_reg_2_offset_l> );
void __fastcall __nop ac_offset_reg2( unsigned int offset_l<ac_reg_3_offset_l> );
void __fastcall __nop ac_offset_reg3( unsigned int offset_l<ac_reg_4_offset_l> );


/*
 *	Arcade card control reg function: byte size. *Inlined*
 */
void __fastcall __nop ac_control_reg0( unsigned char control<ac_reg_1_control_l> );
void __fastcall __nop ac_control_reg1( unsigned char control<ac_reg_2_control_l> );
void __fastcall __nop ac_control_reg2( unsigned char control<ac_reg_3_control_l> );
void __fastcall __nop ac_control_reg3( unsigned char control<ac_reg_4_control_l> );


/*
 *	CD sector to Arcade card transfer.
 */
void __fastcall ac_cd_xfer( unsigned char ac_reg<bl>, unsigned char sector_addr_h<cl>, unsigned int sector_addr_l<dx>, unsigned char sectors<al> );

void __fastcall ac_vram_xfer(unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int num_bytes<cx>, unsigned char size<dl> );
void __fastcall ac_vram_xfer(unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int num_bytes<cx>, unsigned char size<dl>, unsigned char flags<ah> );

void __fastcall ac_vram_dma( unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int num_bytes<cx> );
void __fastcall ac_vram_dma( unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int num_bytes<cx>, unsigned char flags<ah> );


void __fastcall ac_vram_copy( unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int bytes<cx> );
void __fastcall ac_vram_copy( unsigned char ac_reg<al>, unsigned int vram_addr<bx>, unsigned int bytes<cx>, unsigned char flags<ah> );


void __fastcall ac_vce_copy( unsigned char ac_reg<al>, unsigned int start_color<bx>, unsigned int num_colors<cx> );


/*
 * Arcade Card defines
 */
#define AC_REG0	0x00
#define AC_REG1	0x01
#define AC_REG2	0x02
#define AC_REG3	0x03

/*
 * Block Transfer defines for use with Arcade Call routines
 */
#define TIA_OP			0xE300
#define TAI_OP			0xF300
#define TIN_OP			0xD300
#define TII_OP			0x7300
#define TDD_OP			0xC300
#define RTS			0x0060

#endif /* _AC_H */
