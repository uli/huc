#ifndef _SGX_H
#define _SGX_H

/*
 * SGX fastcall defines
 */


/*
 * sgx_vreg( char reg )
 * sgx_vreg( char reg, int data )
 */
#pragma fastcall sgx_vreg( byte acc );
void __fastcall sgx_vreg( unsigned char reg<al>, unsigned int data<cx> );

/*
 * sgx_read_vram(word vram_offset);
 */
#pragma fastcall sgx_read_vram( word ax );

/*
 * sgx_load_bat(int vaddr, int *bat_data, char w, char h)
 */
#pragma fastcall sgx_load_bat(word di, farptr bl:si, byte cl, byte ch)

/*
 * sgx_set_screen_size( char size )
 */
#pragma fastcall sgx_set_screen_size( byte acc )

/*
 * sgx_load_vram(int vaddr, int *data, int nb);
 */
#pragma fastcall sgx_load_vram(word di, farptr bl:si, word cx)
 
/* 
 * sgx_set_tile_data(char *tile_ex [di])
 * sgx_set_tile_data(char *tile [bl:si], int nb_tile [cx], char *ptable [al:dx])
 */
#pragma fastcall sgx_set_tile_data(word di)
#pragma fastcall sgx_set_tile_data(farptr bl:si, word cx, farptr al:dx)

 
/*
 * sgx_set_map_data(int *ptr)
 * sgx_set_map_data(char *map [bl:si], int w [ax], int h)
 * sgx_set_map_data(char *map [bl:si], int w [ax], int h [dx], char wrap)
 */
#pragma fastcall sgx_set_map_data(word acc)
#pragma fastcall sgx_set_map_data(farptr bl:si, word ax, word acc)
#pragma fastcall sgx_set_map_data(farptr bl:si, word ax, word dx, byte acc)

/* 
 * sgx_load_map(char x [al], char y [ah], int mx, int my, char w [dl], char h [dh])
 */ 
#pragma fastcall sgx_load_map(byte al, byte ah, word di, word bx, byte dl, byte dh)
 
 
/*
 * sgx_scroll( int X, int Y );
 */
#pragma fastcall sgx_scroll(word ax, word bx)


/*
 * sgx_spr_set( char num );
 */
#pragma fastcall sgx_spr_set( byte acc )

/*
 * sgx_satb_update();
 */
#pragma fastcall sgx_satb_update( byte acc )

/*
 * sgx_spr_hide( char num );
 */
#pragma fastcall sgx_spr_hide( byte acc )

/*
 * sgx_spr_show( char num );
 */
#pragma fastcall sgx_spr_show( byte acc )

/*
 * sgx_spr_ctrl(char mask, char value);
 */
#pragma fastcall sgx_spr_ctrl(byte al, byte acc)

/*
 * vpc_win_size( char window_num (& 0x01) , int size );
 */
#pragma fastcall vpc_win_size(byte al, word bx)

/*
 * vpc_win_reg( char window_num (& 0x01) , char var );
 */
#pragma fastcall vpc_win_reg(byte al, byte bl)


/*
 * SGX defines
 */
#define SGX				0x01
#define VPC_WIN_A			0x00
#define VPC_WIN_B			0x01
#define	VPC_WIN_AB			0x02
#define	VPC_WIN_NONE			0x03
#define	VPC_NORM			0x00
#define	VPC_SPR				0x04
#define	VPC_INV_SPR			0x08
#define VDC1_ON				0x01
#define	VDC1_OFF			0x00
#define VDC2_ON				0x02
#define	VDC2_OFF			0x00
#define VDC_ON				0x03
#define	VDC_OFF				0x00

#endif /* _SGX_H */
