


#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch errode_expand_timer;

void DREG_load_centered_rect(dreg_t dr, int centre_x, int centre_y, int width, int height);

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

		int disp_size = 2;
		auto display_00 = vs_gui_add_display("DREG content",0,0,disp_size);
		auto display_01 = vs_gui_add_display("Content after expansion & erosion",0,disp_size,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

	    int expansion_steps = 20;
	    vs_gui_add_slider("expansion_steps",0,50,expansion_steps,&expansion_steps);

	    int erosion_steps = 0;
	    vs_gui_add_slider("erosion_steps",0,50,erosion_steps,&erosion_steps);

	    int box1_x, box1_y, box1_width, box1_height;
	    vs_gui_add_slider("box1 x: ", 0, 255, 196, &box1_x);
	    vs_gui_add_slider("box1 y: ", 0, 255, 146, &box1_y);
	    vs_gui_add_slider("box1 width: ", 0, 128, 13, &box1_width);
	    vs_gui_add_slider("box1 height: ", 0, 128, 129, &box1_height);

	    int box2_x, box2_y, box2_width, box2_height;
	    vs_gui_add_slider("box2 x: ", 0, 255, 82, &box2_x);
	    vs_gui_add_slider("box2 y: ", 0, 255, 96, &box2_y);
	    vs_gui_add_slider("box2 width: ", 0, 128, 37, &box2_width);
	    vs_gui_add_slider("box2 height: ", 0, 128, 37, &box2_height);

	    //Setup switches for setting/clearing the content of the RN,RS,RE,RW DREG, which control the behaviour of DNEWS
		int horizontal_DNEWS = 1;
		vs_gui_add_switch("horizontal_DNEWS",horizontal_DNEWS == 1,&horizontal_DNEWS);
		int vertical_DNEWS = 1;
		vs_gui_add_switch("vertical_DNEWS",vertical_DNEWS == 1,&vertical_DNEWS);


    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //GENERATE SOME CONTENT IN DREG S0 TO DEMONSTRATE EROSION / EXPANSION UPON

			DREG_load_centered_rect(S6,box1_x,box1_y,box1_width,box1_height);
			DREG_load_centered_rect(S5,box2_x,box2_y,box2_width,box2_height);
  			scamp7_kernel_begin();
  				MOV(S0,S6);
  				OR(S0,S5);

  				MOV(S2,S0);//make a copy of S0
  			scamp7_kernel_end();


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//PERFORM DREG EXPANSION AND EROSION

  			errode_expand_timer.reset();

			//Setup DREG controlling DNEWS behaviour
  			{
				scamp7_kernel_begin();
					CLR(RN,RS,RE,RW);//sets each DREG to 0 in all PEs
				scamp7_kernel_end();

				if(horizontal_DNEWS)
				{
					scamp7_kernel_begin();
						SET(RE,RW);
					scamp7_kernel_end();
				}

				if(vertical_DNEWS)
				{
					scamp7_kernel_begin();
						SET(RN,RS);
					scamp7_kernel_end();
				}
  			}

  			//Perform expansion steps
  			{
				for(int n = 0 ; n < expansion_steps ; n++)
				{
					scamp7_kernel_begin();
						DNEWS0(S6,S0);//S6 will contain 1s at locations that S0 will expand into this step
						OR(S0,S6);//Combine expanded locations with S0 itself
					scamp7_kernel_end();
				}
  			}

  			//Perform erosion steps
  			{
				scamp7_kernel_begin();
					NOT(S1,S0);//invert DREG content
				scamp7_kernel_end();

				//expand inverted content
				for(int n = 0 ; n < erosion_steps ; n++)
				{
					scamp7_kernel_begin();
						DNEWS0(S6,S1);
						OR(S1,S6);
					scamp7_kernel_end();
				}

				scamp7_kernel_begin();
					NOT(S0,S1);//invert back again
				scamp7_kernel_end();
  			}


  			int time_spent_on_errode_expand = errode_expand_timer.get_usec();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES

			output_timer.reset();
			scamp7_output_image(S2,display_00);
			scamp7_output_image(S0,display_01);
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			int image_output_time_percentage = (output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
			vs_post_text("time spent on erode//exapand %d \n",time_spent_on_errode_expand);
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
