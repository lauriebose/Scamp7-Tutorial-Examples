#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch shift_timer;


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
		auto display_00 = vs_gui_add_display("Captured Image",0,0,disp_size);
		auto display_01 = vs_gui_add_display("Binary Thresholded Image",0,disp_size,disp_size);
		auto display_02 = vs_gui_add_display("Shifted Binary Thresholded Image",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

	    int kernel_selection = 0;
	    vs_gui_add_slider("kernel_selection",0,8,kernel_selection,&kernel_selection);

    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //CAPTURE FRAME AND PERFORM THRESHOLDING

        DREG_load_centered_rect(S0,128,128,32,96);
    	scamp7_kernel_begin();
			MOV(S1,S0);
		scamp7_kernel_end();


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//shift content of S1 using DNEWS

			int time_spent_shifting = -1;

			scamp7_kernel_begin();
				CLR(RN,RS,RE,RW);//Clear all DREG controlling DNEWS behaviour
				SET(RE);
			scamp7_kernel_end();

			if(kernel_selection == 0)
			{
				vs_post_text("perform 100 kernels of length 2\n");
				shift_timer.reset();
				for(int n = 0 ; n < 100 ; n++)
				{
					scamp7_kernel_begin();
						DNEWS1(S6,S1);
						MOV(S1,S6);
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 1)
			{
				vs_post_text("perform 50 kernels of length 4\n");
				shift_timer.reset();
				for(int n = 0 ; n < 50 ; n++)
				{
					scamp7_kernel_begin();
						DNEWS1(S6,S1);
						MOV(S1,S6);
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 2)
			{
				vs_post_text("perform 25 kernels of length 8\n");
				shift_timer.reset();
				for(int n = 0 ; n < 25 ; n++)
				{
					scamp7_kernel_begin();
						DNEWS1(S6,S1);
						MOV(S1,S6);
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 3)
			{
				vs_post_text("perform 20 kernels of length 10\n");
				shift_timer.reset();
				for(int n = 0 ; n < 20 ; n++)
				{
					scamp7_kernel_begin();
						DNEWS1(S6,S1);
						MOV(S1,S6);
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}


			if(kernel_selection == 4)
			{
				vs_post_text("perform 10 kernels of length 20\n");
				shift_timer.reset();
				for(int n = 0 ; n < 10 ; n++)
				{
					scamp7_kernel_begin();
						//this loop is constant and never changes behaviour, and thus it is "safe" to put it inside of the kernel block
						//really this is just to avoid writing out the two instructions in the loop 10 times.... -_-
						for(int i = 0; i < 1 ; i++)
						{
							DNEWS1(S6,S1);
							MOV(S1,S6);
						}
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 5)
			{
				vs_post_text("perform 5 kernels of length 40\n");
				shift_timer.reset();
				for(int n = 0 ; n < 5 ; n++)
				{
					scamp7_kernel_begin();
						//loop to avoid writing this out 20 times... -__-
						for(int i = 0; i < 1 ; i++)
						{
							DNEWS1(S6,S1);
							MOV(S1,S6);
						}
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 6)
			{
				vs_post_text("perform 4 kernels of length 50\n");
				shift_timer.reset();
				for(int n = 0 ; n < 4 ; n++)
				{
					scamp7_kernel_begin();
						//loop to avoid writing this out 25 times... -__-
						for(int i = 0; i < 1 ; i++)
						{
							DNEWS1(S6,S1);
							MOV(S1,S6);
						}
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}


			if(kernel_selection == 7)
			{
				vs_post_text("perform 2 kernels of length 100\n");
				shift_timer.reset();
				for(int n = 0 ; n < 2 ; n++)
				{
					scamp7_kernel_begin();
						//loop to avoid writing this out 50 times... -___-
						for(int i = 0; i < 1 ; i++)
						{
							DNEWS1(S6,S1);
							MOV(S1,S6);
						}
					scamp7_kernel_end();
				}
				time_spent_shifting = shift_timer.get_usec();
			}

			if(kernel_selection == 8)
			{
				vs_post_text("perform 1 kernel of length 200\n");
				shift_timer.reset();
				scamp7_kernel_begin();
					//loop to avoid writing this out 100 times!
					for(int i = 0; i < 1 ; i++)
					{
						DNEWS1(S6,S1);
						MOV(S1,S6);
					}
				scamp7_kernel_end();
				time_spent_shifting = shift_timer.get_usec();
			}

			vs_post_text("time spent shifting %d \n",time_spent_shifting);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES

			output_timer.reset();
			scamp7_output_image(S0,display_01);//display thresholded image
			scamp7_output_image(S1,display_02);//displayed shifted thresholded image
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

