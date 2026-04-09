#include <scamp7.hpp>

#include "MISC/MISC_FUNCS.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

		int disp_size = 1;
		auto display_00 = vs_gui_add_display("Captured Image, AREG A",0,0,disp_size);
		auto display_01 = vs_gui_add_display("Threshold, AREG C",0,disp_size,disp_size);
		auto display_02 = vs_gui_add_display("Thresholded Image, DREG S0 (""where"" A-C > 0)",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

		int output_image = 1;
		vs_gui_add_switch("output_image",1,&output_image);

		int threshold_value = 64;
		vs_gui_add_slider("threshold_value",-127,127,threshold_value,&threshold_value);

		int eliminate_horizontal = 1;
		vs_gui_add_switch("eliminate_horizontal",1,&eliminate_horizontal);

		int eliminate_vertical = 1;
		vs_gui_add_switch("eliminate_vertical",1,&eliminate_vertical);

		int expand_and_sweep_elimination = 1;
		vs_gui_add_slider("sweep_elimination",0,16,expand_and_sweep_elimination,&expand_and_sweep_elimination);

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
					MOV(S1,FLAG);
				all();
			scamp7_kernel_end();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//REDUCE DIGITAL CONTENT TO SPARSE POINNTS

			const dreg_t DREG_TO_REDUCE = S0;

			if(eliminate_horizontal == 1)
			{
				//ELIMINSTATE ALL 1S WHICH HAVE A 1 TO THE RIGHT OF THEM
				//SHIFT HORIZONTALLY, INVERT, AND WITH UNSHIFTED
				scamp7_kernel_begin();
					CLR(RN,RS,RE,RW);
					SET(RE);
					DNEWS0(S6,DREG_TO_REDUCE);
					NOT(S5,S6);
					AND(DREG_TO_REDUCE,DREG_TO_REDUCE,S5);
				scamp7_kernel_end();
			}

			if(eliminate_vertical == 1)
			{
				//ELIMINSTATE ALL 1S WHICH HAVE A 1 ABOVE THEM
				//SHIFT VERTICALLY, INVERT, AND WITH UNSHIFTED
				scamp7_kernel_begin();
					CLR(RN,RS,RE,RW);
					SET(RN);
					DNEWS0(S6,DREG_TO_REDUCE);
					NOT(S5,S6);
					AND(DREG_TO_REDUCE,DREG_TO_REDUCE,S5);
				scamp7_kernel_end();
			}

			if(expand_and_sweep_elimination> 0)
			{
				//REMAINING 1S DO NOT HAVE NEIGHOURING 1S DIRECTLY TO THE RIGHT OR ABOVE
				//HOWEVER MAY HAVE "NEIGHBOURS" ADJACENT TO THEM ON THE DIAGONALS
				//NEED TO ELIMINATE THESE DIAGONAL "NEIGHBOURS"

				//COPY CURRENT RESULT AND SPREAD HORIZONTALLY IN BOTH DIRECTIONS
				scamp7_kernel_begin();
					MOV(S6,DREG_TO_REDUCE);
					CLR(RN,RS);
					SET(RE,RW);
				scamp7_kernel_end();
				for(int n = 0 ; n < expand_and_sweep_elimination ; n++)
				{
					scamp7_kernel_begin();
						DNEWS0(S5,S6);
						OR(S6,S5);
					scamp7_kernel_end();
				}

				//NOW SPREAD VERTICALLY DOWN
				scamp7_kernel_begin();
					CLR(RE,RW,RS);
					SET(RN);
				scamp7_kernel_end();
				for(int n = 0 ; n < expand_and_sweep_elimination ; n++)
				{
					scamp7_kernel_begin();
						DNEWS0(S5,S6);
						OR(S6,S5);
					scamp7_kernel_end();
				}

				//SHIFT COPY DOWNWARDS, INVERT, PERFROM AND WITH UNSHIFTED CONTENT
				scamp7_kernel_begin();
					CLR(RN,RS,RE,RW);
					SET(RN);
					DNEWS0(S5,S6);
					NOT(S6,S5);
					AND(DREG_TO_REDUCE,DREG_TO_REDUCE,S6);
				scamp7_kernel_end();
			}


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //OUTPUT IMAGES

			output_timer.reset();
			if(output_image)
			{
				//output AREG quickly using 4bit approximation
				output_4bit_image_via_DNEWS(A,display_00);
//				output_4bit_image_via_DNEWS(C,display_01);
			}

			scamp7_output_image(S1,display_01);
			scamp7_output_image(S0,display_02);//display thresholded image
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

