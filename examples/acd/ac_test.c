
/*
	Arcade card and SGX library example.
	
*/


#include <huc.h>
#include <AC.h>
#include <sgx.h>

#incbin(map,"pce_bat1.bin");
#incbin(tile,"pce_tile1.bin");
#incbin(pal,"pce_pal1.bin");


main()
{
	
	char i,j,k,l,m,n;
	
	
		
	disp_off();		/* <- I don't think this works.... */
	spr_set();
	spr_hide();
	load_default_font();
	set_screen_size(SCR_SIZE_32x32);
	disp_on();

	if(!sgx_detect())
	{  put_string("Halt: SGX not hardware found", 2, 13); for(;;){} }


	/* These NEED to be set, else you won't see the SGX 2nd layer BG or sprites. */
	vpc_win_size( VPC_WIN_A, 0x01ff);
	vpc_win_size( VPC_WIN_B, 0x01ff);
	vpc_win_reg( VPC_WIN_A, VDC_ON+VPC_NORM);
	vpc_win_reg( VPC_WIN_B, VDC_ON+VPC_NORM);
	vpc_win_reg( VPC_WIN_AB, VDC_ON+VPC_NORM);
	vpc_win_reg( VPC_WIN_NONE, VDC_ON+VPC_NORM);
	
	set_font_pal(4);
	set_font_color(14,0);
	load_default_font();
	put_string("SGX hardware found", 2, 3);
	
	sgx_set_screen_size(SCR_SIZE_32x32);
	sgx_load_vram(0x0000,map, 0x400);
	sgx_load_vram(0x1000,tile, 0x4000);
	load_palette(0, pal,16);
	
	sgx_spr_hide();
	sgx_spr_set(1);

	sgx_disp_on();
	
	
	put_string("Arcade Card: ", 2, 4);


	if ( ac_init() )
		{
			put_string("detected.", 15, 4);

			/* initialize AC register 0 to address 0x000000 and +1 auto-increment */
			ac_full_reg0(0x00,0x0000,0x0000,0x0001,0x11);
							
			put_string("CD->AC xfer... ", 2, 5);

			/* transfer 8k at a time from CD to AC memory via AC reg #0 */
			ac_cd_xfer(AC_REG0,0,0x1c2,4);
			ac_cd_xfer(AC_REG0,0,0x1c6,4);
			ac_cd_xfer(AC_REG0,0,0x1ca,4);
			ac_cd_xfer(AC_REG0,0,0x1ce,4);
			ac_cd_xfer(AC_REG0,0,0x1d2,4);
			ac_cd_xfer(AC_REG0,0,0x1d6,4);
			ac_cd_xfer(AC_REG0,0,0x1da,4);
			ac_cd_xfer(AC_REG0,0,0x1de,4);

			put_string("finished.", 17, 5);
						
			sgx_bg_on();
			
			/* reset AC reg #0 address to 0x000000 */		
			ac_addr_reg0(0x00,0x0000);
			ac_vram_dma(AC_REG0 ,0x1000,0x3c00, SGX);

			ac_addr_reg0(0x00,0x8000);
			ac_vram_dma(AC_REG0 ,0x0000,0x800, SGX);

			ac_addr_reg0(0x00,0x8800);
			ac_vce_copy( AC_REG0, 0x00, 0x100 );
			

			vsync(60);
			vsync(60);


		}
	else
		{ put_string("not detected.", 15, 4); for(;;){} }
			
	put_string("Scrolling SGX layer ", 2, 6);
	for(;;)
	{
		for( j=0; j<0xff; j++)
		{
			vsync();
			sgx_scroll( j , j);
		}
	}		
	
	
}