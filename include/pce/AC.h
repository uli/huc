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

#pragma fastcall ac_addr_reg0( byte ac_reg_1_high, word ac_reg_1_low ) nop;
#pragma fastcall ac_addr_reg1( byte ac_reg_2_high, word ac_reg_2_low ) nop;
#pragma fastcall ac_addr_reg2( byte ac_reg_3_high, word ac_reg_3_low ) nop;
#pragma fastcall ac_addr_reg3( byte ac_reg_4_high, word ac_reg_4_low ) nop;


/*
 *	Arcade card full reg function: address byte high, address word mid/low, word offset,
 *	word increment, byte control. *Inlined*
 *
 */
#pragma fastcall ac_full_reg0( byte ac_reg_1_high, word ac_reg_1_low, word ac_reg_1_offset_l, word ac_reg_1_incrmt_l, byte ac_reg_1_control) nop;
#pragma fastcall ac_full_reg1( byte ac_reg_2_high, word ac_reg_2_low, word ac_reg_2_offset_l, word ac_reg_2_incrmt_l, byte ac_reg_2_control) nop;
#pragma fastcall ac_full_reg2( byte ac_reg_3_high, word ac_reg_3_low, word ac_reg_3_offset_l, word ac_reg_3_incrmt_l, byte ac_reg_3_control) nop;
#pragma fastcall ac_full_reg3( byte ac_reg_4_high, word ac_reg_4_low, word ac_reg_4_offset_l, word ac_reg_4_incrmt_l, byte ac_reg_4_control) nop;


/*
 *	Arcade card increment reg function: word size. *Inlined*
 */
#pragma fastcall ac_inc_reg0( word ac_reg_1_incrmt_l ) nop;
#pragma fastcall ac_inc_reg1( word ac_reg_2_incrmt_l ) nop;
#pragma fastcall ac_inc_reg2( word ac_reg_3_incrmt_l ) nop;
#pragma fastcall ac_inc_reg3( word ac_reg_4_incrmt_l ) nop;


/*
 *	Arcade card offset reg function: word size. *Inlined*
 */
#pragma fastcall ac_offset_reg0( word ac_reg_1_offset_l ) nop;
#pragma fastcall ac_offset_reg1( word ac_reg_2_offset_l ) nop;
#pragma fastcall ac_offset_reg2( word ac_reg_3_offset_l ) nop;
#pragma fastcall ac_offset_reg3( word ac_reg_4_offset_l ) nop;


/*
 *	Arcade card control reg function: byte size. *Inlined*
 */
#pragma fastcall ac_control_reg0( byte ac_reg_1_control_l ) nop;
#pragma fastcall ac_control_reg1( byte ac_reg_2_control_l ) nop;
#pragma fastcall ac_control_reg2( byte ac_reg_3_control_l ) nop;
#pragma fastcall ac_control_reg3( byte ac_reg_4_control_l ) nop;


/*
 *	CD sector to Arcade card transfer.
 *	
 *	ac_cd_xfer( ac reg (byte), sector addr high(byte),sector addr low(word),
 *	            sectors (byte) )
 */
void __fastcall ac_cd_xfer( unsigned char ac_reg<bl>, unsigned char sector_addr_h<cl>, unsigned int sector_addr_l<dx>, unsigned char sectors<al> );


/*
 * ac_vram_xfer( AC reg (word), vram addr (word), num bytes (word), chunk size(byte))
 * ac_vram_xfer( AC reg (word), vram addr (word), num bytes (word), chunk size(byte), const SGX )
 */
#pragma fastcall ac_vram_xfer(byte al, word bx, word cx, byte dl );
#pragma fastcall ac_vram_xfer(byte al, word bx, word cx, byte dl, byte ah );




/*
 * ac_vram_dma( AC reg (word), vram addr (word), num bytes (word) )
 * ac_vram_dma( AC reg (word), vram addr (word), num bytes (word), const SGX )
 */
#pragma fastcall ac_vram_dma( byte al, word bx, word cx );
#pragma fastcall ac_vram_dma( byte al, word bx, word cx, byte ah );


/*
 * ac_vram_copy( AC reg (word), vram addr (word), num bytes (word) )
 * ac_vram_copy( AC reg (word), vram addr (word), num bytes (word), const SGX )
 */
#pragma fastcall ac_vram_copy( byte al, word bx, word cx );
#pragma fastcall ac_vram_copy( byte al, word bx, word cx, byte ah );


/*
 * ac_vram_copy( AC reg (word), color number (word), num colors (word) )
 */
#pragma fastcall ac_vce_copy( byte al, word bx, word cx );


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
extern int XFER_TYPE,XFER_SRC,XFER_DEST,XFER_LEN,XFER_RTS;
#define TIA_OP			0xE300
#define TAI_OP			0xF300
#define TIN_OP			0xD300
#define TII_OP			0x7300
#define TDD_OP			0xC300
#define RTS				0x0060

#endif /* _AC_H */
