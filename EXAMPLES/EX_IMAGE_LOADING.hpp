#include <scamp7.hpp>
#include "MISC/MISC_FUNCS.hpp"
#include "MISC/image_loader.hpp"
using namespace SCAMP7_PE;
vs_stopwatch frame_timer;

int main()
{
    vs_init();

    scamp7_image_loader image_loader(A,S0,F,RW);

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





	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//SETUP IMAGE LOADER

		const char*video_filepath = "C:/Users/lauri/Desktop/gaze_frames_sample/%d.bmp";//Change this to the folder containing the bmp files to load
		int first_index = 1;
		int last_index = 200;
		bool ping_pong_playback = false;
		image_loader.set_verbose_mode(1);//Makes the image loader output text showing loads & errors

		image_loader.init_video_frames(video_filepath,first_index,last_index,ping_pong_playback);

		//This will add a slider letting us directly select
		image_loader.add_gui_items();

    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

        vs_frame_loop_control();
        vs_disable_frame_trigger();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //LOAD IMAGE FROM COMPUTER

			image_loader.load_video_frame_areg();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES
			if(image_output)
			{
				//output register plane as image
				if(use_4bit_image_output)
				{
					//output AREG quickly using 4bit approximation
					output_4bit_image_via_DNEWS(A,display_00);
				}
				else
				{
					//output AREG at higher accuracy but taking significantly longer
					scamp7_output_image(A,display_00);
				}
			}

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame

			vs_post_text("!clear");
			//calculate the possible max FPS
			int max_possible_frame_rate = 1000000/frame_time_microseconds;
			//display this values on host
			vs_post_text("frame time %d microseconds, potential FPS ~%d \n",frame_time_microseconds,max_possible_frame_rate);
    }
    return 0;
}

