#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;
vs_stopwatch frame_timer;

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS
		int disp_size = 2;
		vs_handle display_00 = vs_gui_add_display("Captured Image",0,0,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

		int image_output = 1;
		vs_gui_add_switch("image_output",1,&image_output);

		int use_4bit_image_output = 1;
		vs_gui_add_switch("use_4bit_image_output",1,&use_4bit_image_output);

    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //CAPTURE FRAME AND PERFORM AREG OPERATIONS

			scamp7_kernel_begin();
				//Each PE contains a PIX register which accumulates light entering the PE
				//"get_image" copies this accumulated signal into a AREG, and resets the accumulation within PIX
				//This effectively captures a image frame, with each PE storing 1 pixel of the image

				//A = pixel data of latest frame, F = intermediate result
				get_image(A,F);
			scamp7_kernel_end();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //OUTPUT IMAGES

			if(image_output)
			{
				//display the contents of PE registers as images, displaying 3 images for A,B,C
				if(use_4bit_image_output)
				{
					//output AREG quickly using 4bit approximation
					output_areg_via_bitstack_DNEWS(A,display_00);
				}
				else
				{
					//output AREG slowly at higher accuracy
					scamp7_output_image(A,display_00);
				}
			}

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			vs_post_text("frame time %d microseconds, potential FPS ~%d \n",frame_time_microseconds,max_possible_frame_rate); //display this values on host
    }
    return 0;
}

