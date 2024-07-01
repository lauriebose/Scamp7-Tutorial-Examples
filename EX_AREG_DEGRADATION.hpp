#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

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

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

    int disp_size = 2;
    auto display_00 = vs_gui_add_display("A",0,0,disp_size);
    auto display_01 = vs_gui_add_display("WHERE(S1) B = A ",0,disp_size,disp_size);
    auto display_10 = vs_gui_add_display("S0",disp_size,0,disp_size);
    auto display_11 = vs_gui_add_display("S1 = S1 OR S0",disp_size,disp_size,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

    int AREG_copy_operations = 0;
    vs_gui_add_slider("AREG_copy_operations", 0, 128, AREG_copy_operations, &AREG_copy_operations);

    int AREG_copy_register = 0;
    vs_gui_add_slider("AREG_copy_register", 0, 4, AREG_copy_register, &AREG_copy_register);

	auto gui_btn_clrs1 = vs_gui_add_button("CLR S1");
	vs_on_gui_update(gui_btn_clrs1,[&](int32_t new_value)
	{
	    scamp7_kernel_begin();
			CLR(S1);
		scamp7_kernel_end();
   });

    // Frame Loop
    while(1)
    {
        frame_timer.reset();//reset frame_timer

       	vs_disable_frame_trigger();
        vs_frame_loop_control();

    	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//capture new image into AREG A

			scamp7_kernel_begin();
				get_image(A,E);
				mov(B,A);
			scamp7_kernel_end();

			if(AREG_copy_register == 0)
			{
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

	    	output_timer.reset();
			scamp7_output_image(A,display_00);
			scamp7_output_image(B,display_01);
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			int image_output_time_percentage = (output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
			vs_post_text("frame time %d microseconds(%%%d image output), potential FPS ~%d \n",frame_time_microseconds,image_output_time_percentage,max_possible_frame_rate); //display this values on host
    }
    return 0;
}
