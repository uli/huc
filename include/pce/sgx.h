#ifndef _SGX_H
#define _SGX_H

/*
 * SGX fastcall defines
 */


void __fastcall sgx_vreg( unsigned char reg<acc> );
void __fastcall sgx_vreg( unsigned char reg<al>, unsigned int data<cx> );

void __fastcall sgx_read_vram( unsigned int vram_offset<ax> );

void __fastcall sgx_load_bat(unsigned int vaddr<di>, int far *bat_data<bl:si>, unsigned char w<cl>, unsigned char h<ch>);

void __fastcall sgx_set_screen_size( unsigned char size<acc> );

void __fastcall sgx_load_vram(unsigned int vaddr <di>, int far *data<bl:si>, int nb<cx>);
 
void __fastcall sgx_set_tile_data(char *tile_ex<di>);
void __fastcall sgx_set_tile_data(char far *tile<bl:si>, int nb_tile<cx>, char far *ptable<al:dx>);

 
void __fastcall sgx_set_map_data(int *ptr<acc>);
void __fastcall sgx_set_map_data(char far *map<bl:si>, int w<ax>, int h<acc>);
void __fastcall sgx_set_map_data(char far *map<bl:si>, int w<ax>, int h<dx>, char wrap<acc>);

void __fastcall sgx_load_map(char x<al>, char y<ah>, int mx<di>, int my<bx>, char w<dl>, char h<dh>);

void __fastcall sgx_scroll(int x<ax>, int y<bx>);

void __fastcall sgx_spr_set( char num<acc> );

void sgx_satb_update(void);
void __fastcall sgx_satb_update( unsigned char max<acc> );

void __fastcall sgx_spr_hide( char num<acc> );

void __fastcall sgx_spr_show( char num<acc> );

void __fastcall sgx_spr_ctrl(char mask<al>, char value<acc>);

void __fastcall vpc_win_size(char window_num<al>, int size<bx>);

void __fastcall vpc_win_reg(char window_num<al>, char var<bl>);


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
