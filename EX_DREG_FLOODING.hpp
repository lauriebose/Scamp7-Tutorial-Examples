#include <scamp7.hpp>
#include "MISC/MISC_FUNCS.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch flooding_timer;
vs_stopwatch drawing_timer;

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

        const int disp_size = 2;
	    auto display_00 = vs_gui_add_display("Flooding source",0,0,disp_size);
	    auto display_01 = vs_gui_add_display("Flooding mask",0,disp_size,disp_size);
	    auto display_02 = vs_gui_add_display("Source after flooding",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

	    //control the box drawn into DREG & used as the flooding source
	    int box_posx, box_posy,box_width,box_height;
		vs_gui_add_slider("box_posx: ",1,256,64,&box_posx);
		vs_gui_add_slider("box_posy: ",1,256,125,&box_posy);
		vs_gui_add_slider("box_width: ",1,100,20,&box_width);
		vs_gui_add_slider("box_height: ",1,100,20,&box_height);

		//The number of iterations performed, determines the extent/distance flooding can reach
	    int flood_iterations = 10;
	    vs_gui_add_slider("flood_iterations", 0,20,flood_iterations,&flood_iterations);

	    //Select if PEs on the edge of the array/image act as sources during flooding
		int flood_from_boundaries = 0;
		vs_gui_add_switch("flood_from_boundaries",true,&flood_from_boundaries);

		//Add switches for selecting the content of the RN,RS,RE,RW DREG, which control the directions of flooding
		int set_RN = 1;
		vs_gui_add_switch("set_RN",set_RN == 1,&set_RN);
		int set_RS = 1;
		vs_gui_add_switch("set_RS",set_RS == 1,&set_RS);
		int set_RE = 1;
		vs_gui_add_switch("set_RE",set_RE == 1,&set_RE);
		int set_RW = 1;
		vs_gui_add_switch("set_RW",set_RW == 1,&set_RW);

		//Switch between performing flooding using native commands or with the library Macro
		int use_api = 0;
		vs_gui_add_switch("use_api",use_api == 1,&use_api);

		//Use a fixed single kernel flooding routine to demonstrate maximum speed
		int use_quick_flood_example = 0;
		vs_gui_add_switch("use_quick_flood_example",use_quick_flood_example == 1,&use_quick_flood_example);

    //CONTINOUS FRAME LOOP
    while(true)
    {
        frame_timer.reset();//reset frame_timer

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //DRAW CONTENT INTO DIGITAL REGISTERS FOR FLOODING EXAMPLE

        	drawing_timer.reset();

			//DRAW CONTENT INTO THE FLOODING SOURCE REGISTER, FLOODING STARTS FROM THE WHITE PIXEL OF THIS REGISTER
			{
    			scamp7_kernel_begin();
    				CLR(S1);
    			scamp7_kernel_end();
				scamp7_draw_begin(S1);
					scamp7_draw_rect(box_posy,box_posx,box_height+box_posy,box_width+box_posx);
				scamp7_draw_end();
				scamp7_kernel_begin();
					MOV(S3,S1);
				scamp7_kernel_end();
			}


			//DRAW CONTENT INTO THE FLOODING MASK REGISTER, FLOODING IS RESTRICTED TO THE WHITE PIXEL OF THIS REGISTER
			{
				scamp7_kernel_begin();
					CLR(S2);
				scamp7_kernel_end();

				scamp7_draw_begin(S2);
					scamp7_draw_circle(127,127,100);
					scamp7_draw_circle(127,127,50);
					scamp7_draw_line(10,0,10,200);
					scamp7_draw_line(10,200,100,200);
				scamp7_draw_end();
			}

			int time_spent_drawing = drawing_timer.get_usec();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//SET DREG WHICH CONTROL THE DIRECTIONS IN WHICH FLOODING MAY OCCUR

			scamp7_kernel_begin();
				CLR(RN,RS,RE,RW);
			scamp7_kernel_end();

			if(set_RN)
			{
				scamp7_kernel_begin();
					SET(RN);
				scamp7_kernel_end();
			}
			if(set_RS)
			{
				scamp7_kernel_begin();
					SET(RS);
				scamp7_kernel_end();
			}
			if(set_RE)
			{
				scamp7_kernel_begin();
					SET(RE);
				scamp7_kernel_end();
			}
			if(set_RW)
			{
				scamp7_kernel_begin();
					SET(RW);
				scamp7_kernel_end();
			}


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//shift content of S1 using DNEWS

			//PERFORM FLOODING
			flooding_timer.reset();

			if(!use_quick_flood_example)
			{
				if(!use_api)
				{
					//FLOODING USING NATIVE INSTRUCTIONS
					scamp7_kernel_begin();
						SET(RF);
						MOV(RZ,S1);
						MOV(RF,S2);
					scamp7_kernel_end();
					if(flood_from_boundaries)
					{
						for(int n = 0 ; n < flood_iterations ; n++)
						{
							scamp7_kernel_begin();
								PROP_R(RZ);
								PROP_1(RZ);
							scamp7_kernel_end();
						}
					}
					else
					{
						for(int n = 0 ; n < flood_iterations ; n++)
						{
							scamp7_kernel_begin();
								PROP_R(RZ);
								PROP_0(RZ);
							scamp7_kernel_end();
						}
					}
					scamp7_kernel_begin();
						MOV(S1,RZ);
						SET(RF);
					scamp7_kernel_end();
				}
				else
				{
					//FLOODING USING API
					scamp7_flood(S1,S2,flood_from_boundaries,flood_iterations,true);
				}
			}



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



			if(use_quick_flood_example)
			{
				//EXAMPLE OF PERFORMING EFFICIENTLY PERFORMING FLOODING USING A SINGLE KERNEL
				scamp7_kernel_begin();
					SET(RF);
					MOV(RZ,S1);
					MOV(RF,S2);

					for(int n = 0 ; n < 11 ; n++)
					{
						PROP_R(RZ);
						PROP_1(RZ);
					}

					MOV(S1,RZ);
					SET(RF);
				scamp7_kernel_end();
			}


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			int time_spent_flooding = flooding_timer.get_usec();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES

			output_timer.reset();

			scamp7_output_image(S3,display_00);
			scamp7_output_image(S2,display_01);
			scamp7_output_image(S1,display_02);
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			int image_output_time_percentage = (output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
			vs_post_text("time spent drawing %d \n",time_spent_drawing);
			vs_post_text("time spent flooding %d \n",time_spent_flooding);
			vs_post_text("frame time %d microseconds(%%%d image output), potential FPS ~%d \n",frame_time_microseconds,image_output_time_percentage,max_possible_frame_rate); //display this values on host
    }
    return 0;
}
