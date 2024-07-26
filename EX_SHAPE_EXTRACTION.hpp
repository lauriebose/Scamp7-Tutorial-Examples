#include <scamp7.hpp>
#include <random>
using namespace SCAMP7_PE;

void DREG_load_centered_rect(dreg_t reg, int x, int y, int width, int height);

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

int main()
{
    vs_init();

    const int display_size = 2;
    vs_handle display_00 = vs_gui_add_display("S0",0,0,display_size);
    vs_handle display_01 = vs_gui_add_display("S1 Flooding Source",0,display_size,display_size);
    vs_handle display_02 = vs_gui_add_display("S2 = S1 flooding, masked by S0",0,display_size*2,display_size);

    //The point from which flooding will start from
    int flood_source_x, flood_source_y;
	vs_gui_add_slider("flood_source_x: ",1,256,128,&flood_source_x);
	vs_gui_add_slider("flood_source_y: ",1,256,128,&flood_source_y);

	//The number of iterations performed, determines the extent/distance flooding can reach
    int flood_iterations = 10;
    vs_gui_add_slider("flood_iterations", 0,20,flood_iterations,&flood_iterations);

	bool generate_boxes = true;
	vs_handle gui_button_generate_boxes = vs_gui_add_button("generate_boxes");
	//function that will be called whenever button is pressed
	vs_on_gui_update(gui_button_generate_boxes,[&](int32_t new_value)
	{
		generate_boxes = true;//trigger generation of boxes
    });

	//Toggle if to refresh the content of S0 each frame to prevent it decaying
    int refresh_S0 = 1;
    vs_gui_add_switch("refresh_S0", true, &refresh_S0);

    //Objects for random number generation
		std::random_device rd;
		std::mt19937 gen(rd()); // Mersenne Twister generator
		int min = 0;
		int max = 255;
		std::uniform_int_distribution<> distr(min, max);//Create distribution to sample from

    while(1)
    {
    	frame_timer.reset();

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //GENERATES RANDOM BOXES IN DREG S0 IF BUTTON WAS PRESSED (OR PROGRAM JUST STARTED)

			if(generate_boxes)
			{
				scamp7_kernel_begin();
					CLR(S0); //Clear content of S0
				scamp7_kernel_end();
				for(int n = 0 ; n < 30 ; n++)
				{
					//Load box of random location and dimensions into S5
					int pos_x = distr(gen);
					int pos_y = distr(gen);
					int width = 1+distr(gen)/5;
					int height = 1+distr(gen)/5;
					DREG_load_centered_rect(S5,pos_x,pos_y,width,height);

					scamp7_kernel_begin();
						OR(S0,S5);//Add box in S5 to content of S0
					scamp7_kernel_end();
				}
				generate_boxes = false;
			}

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //PERFORM FLOODING OF S0, ORGINATING FROM A SPECIFIC POINT

			//Load specified point into S1
			scamp7_load_point(S1,flood_source_y,flood_source_x);

			//Perform Flooding using native instructions
			{
				scamp7_kernel_begin();
					SET (RN,RS,RE,RW);//Set all DNEWS register so flooding is performed in all directions across whole processor array

					SET(RF);//Reset digital Flag = 1 across whole processor array, as operations targeting RZ are Flagged by RF
					MOV(RZ,S1);//Copy loaded point into RZ
					MOV(RF,S0);//Copy the content of S0 into RF to act as the Mask during flooding
				scamp7_kernel_end();

				//Perform flooding iterations
				for(int n = 0 ; n < flood_iterations ; n++)
				{
					scamp7_kernel_begin();
						PROP_R(RZ);
						PROP_0(RZ);
					scamp7_kernel_end();
				}

				scamp7_kernel_begin();
					MOV(S2,RZ);//Copy result of flooding S2
				scamp7_kernel_end();
			}

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OPTIONALLY REFRESH CONTENT OF S0 TO PREVENT IT DECAYING

			if(refresh_S0)
			{
				scamp7_kernel_begin();
					REFRESH(S0);
				scamp7_kernel_end();
			}

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //OUTPUT IMAGES

        	output_timer.reset();

			scamp7_output_image(S0,display_00);
			scamp7_output_image(S1,display_01);
			scamp7_output_image(S2,display_02);
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

void DREG_load_centered_rect(dreg_t reg, int x, int y, int width, int height)
{
	int top_row = y-height/2;
	if(top_row < 0)
	{
		height += top_row;
		top_row = 0;
	}
	int right_column = x-width/2;
	if(right_column < 0)
	{
		width += right_column;
		right_column = 0;
	}
	int bottom_row = top_row+height;
	int left_column = right_column+width;
	scamp7_load_region(reg, top_row, right_column, bottom_row, left_column);
}

