#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
#include "../../s5d_m0_scamp7/src/debug_gui.hpp"
using namespace SCAMP7_PE;


void DREG_load_centered_rect(dreg_t dr, int centre_x, int centre_y, int width, int height);

int main()
{
    vs_init();
    setup_voltage_configurator(false);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

    int disp_size = 2;
    auto display_00 = vs_gui_add_display("A",0,0,disp_size);
    auto display_01 = vs_gui_add_display("",0,disp_size,disp_size);
    auto display_02 = vs_gui_add_display("ERROR",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

    int AREG_copy_operations = 0;
    vs_gui_add_slider("AREG_copy_operations", 0, 128, AREG_copy_operations, &AREG_copy_operations);

    int AREG_copy_register = 0;
    vs_gui_add_slider("AREG_copy_register", 0, 4, AREG_copy_register, &AREG_copy_register);


    // Frame Loop
    while(1)
    {
       	vs_disable_frame_trigger();
        vs_frame_loop_control();

    	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//capture new image into AREG A

			scamp7_kernel_begin();
				get_image(A,E);
				mov(B,A);
			scamp7_kernel_end();
			scamp7_output_image(A,display_00);

			if(AREG_copy_register == 0)
			{
				vs_post_text("copying B into C and back %d times\n",AREG_copy_operations);
				for(int n = 0 ; n < AREG_copy_operations ; n++)
				{
					scamp7_kernel_begin();
						bus(C,B);
						bus(B,C);
					scamp7_kernel_end();
				}
			}

			if(AREG_copy_register == 1)
			{
				vs_post_text("copying B into D and back %d times\n",AREG_copy_operations);
				for(int n = 0 ; n < AREG_copy_operations ; n++)
				{
					scamp7_kernel_begin();
						bus(D,B);
						bus(B,D);
					scamp7_kernel_end();
				}
			}

			if(AREG_copy_register == 2)
			{
				vs_post_text("copying B into E and back %d times\n",AREG_copy_operations);
				for(int n = 0 ; n < AREG_copy_operations ; n++)
				{
					scamp7_kernel_begin();
						bus(E,B);
						bus(B,E);
					scamp7_kernel_end();
				}
			}

			if(AREG_copy_register == 3)
			{
				vs_post_text("copying B into F and back %d times\n",AREG_copy_operations);
				for(int n = 0 ; n < AREG_copy_operations ; n++)
				{
					scamp7_kernel_begin();
						bus(F,B);
						bus(B,F);
					scamp7_kernel_end();
				}
			}

			if(AREG_copy_register == 4)
			{
				vs_post_text("copying B into NEWS and back %d times\n",AREG_copy_operations);
				for(int n = 0 ; n < AREG_copy_operations ; n++)
				{
					scamp7_kernel_begin();
						bus(NEWS,B);
						bus(B,NEWS);
					scamp7_kernel_end();
				}
			}



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT RESULTS STORED IN VARIOUS REGISTERS

			scamp7_output_image(B,display_01);
			scamp7_kernel_begin();
				sub(F,A,B);
			scamp7_kernel_end();
			scamp7_output_image(F,display_02);

    }
    return 0;
}

void DREG_load_centered_rect(dreg_t dr, int centre_x, int centre_y, int width, int height)
{

	int top_left_row = centre_y-height/2;
	if(top_left_row < 0)
	{
		height += top_left_row;
		top_left_row = 0;
	}
	int top_left_column = centre_x-width/2;
	if(top_left_column < 0)
	{
		width += top_left_column;
		top_left_column = 0;
	}

	scamp7_load_region(dr, top_left_row, top_left_column, top_left_row+height, top_left_column+width);
}

