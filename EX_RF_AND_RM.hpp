


#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch areg_shift_timer;

void DREG_load_centered_rect(dreg_t dr, int centre_x, int centre_y, int width, int height);

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

		int disp_size = 2;
		auto display_00 = vs_gui_add_display("S0",0,0,disp_size);
		auto display_01 = vs_gui_add_display("RF DIGITAL ""FLAG""",0,disp_size,disp_size);
		auto display_02 = vs_gui_add_display("RM = S0 FLAGGED BY RF ",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

	    int box_x, box_y, box_width, box_height;
	    vs_gui_add_slider("box x: ", 0, 255, 128, &box_x);
	    vs_gui_add_slider("box y: ", 0, 255, 128, &box_y);
	    vs_gui_add_slider("box width: ", 0, 255, 96, &box_width);
	    vs_gui_add_slider("box height: ", 0, 255, 96, &box_height);

		int threshold_value = 64;
		vs_gui_add_slider("threshold_value",-127,127,threshold_value,&threshold_value);

		int clear_RM = 0;
		vs_gui_add_switch("clear_RM",clear_RM == 1, &clear_RM);

		int refresh_RM = 0;
		vs_gui_add_switch("refresh_RM",refresh_RM == 1, &refresh_RM);


    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//CAPTURE FRAME AND PERFORM THRESHOLDING

			//load threshold value into C across all PEs
			scamp7_in(C,threshold_value);

			scamp7_kernel_begin();
				//A = pixel data of latest frame, F = intermediate result
				get_image(A,F);

				//C = (A - C) == (latest frame pixel - threshold)
				sub(F,A,C);

				//sets FLAG = 1 in PEs where F > 0 (i.e. where A > C), else FLAG = 0
				where(F);
					//copy FLAG into S0
					MOV(S0,FLAG);
				all();
			scamp7_kernel_end();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //EXAMPLE OF PERFORMING "FLAGGED" DIGITAL OPERATIONS WITH RF & RM

			if(clear_RM)
			{
				scamp7_kernel_begin();
					//Set RF=1 in all PEs
					SET(RF);
						//Set RM=0 in all PEs where RF=1 (currently all of them!)
						CLR(RM);
				scamp7_kernel_end();
			}


			//Load a rectangular region into S6, this will be copied to RF later
			DREG_load_centered_rect(S6,box_x,box_y,box_width,box_height);

			//All digital operations targeting RM are flagged by RF
			//I.E. when a digital operation is performed that would change the content of RM, it is only performed in PEs where RF = 1
			scamp7_kernel_begin();
				//Copy rectangular region in S6 into FLAG
				MOV(RF,S6);

				    //Copy thresholded image, stored in S0, into RM.
				    //Will only be performed in PEs where RF=1 (i.e. only inside the rectangle)
					MOV(RM,S0);
			scamp7_kernel_end();

			if(refresh_RM)
			{
				scamp7_kernel_begin();
					SET(RF);
						REFRESH(RM);
				scamp7_kernel_end();
			}


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES

			output_timer.reset();
			scamp7_output_image(S0,display_00);
			scamp7_output_image(S6,display_01);
			scamp7_output_image(RM,display_02);
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
