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

    int display_size = 2;
    auto display_00 = vs_gui_add_display("Captured Image",0,0,display_size);
    auto display_01 = vs_gui_add_display("Shifted Image",0,display_size,display_size);
    auto display_10 = vs_gui_add_display("Captured Image - Shifted Image",display_size,0,display_size);
    auto display_11 = vs_gui_add_display("Captured Image - Shifted Image",display_size,display_size,display_size);

    int steps = 0;
    vs_gui_add_slider("steps",0,64,steps,&steps);



    // Frame Loop
    while(1)
    {
        frame_timer.reset();//reset frame_timer

       	vs_disable_frame_trigger();
        vs_frame_loop_control();

        scamp7_kernel_begin();
			CLR(RN,RS,RE,RW);
		scamp7_kernel_end();

        DREG_load_centered_rect(RW,64,128,128,255);
        DREG_load_centered_rect(RE,196,128,128,255);

        DREG_load_centered_rect(S0,130,128,2,255);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//capture new image into AREG A and create copy in B

        	scamp7_in(C,-127);
			scamp7_kernel_begin();
				get_image(A,E);
				mov(B,A);
			scamp7_kernel_end();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SHIFT AREG HORIZONTALLY

			errode_expand_timer.reset();


			for(int x = 0; x < steps; x++)
			{
				//KERNEL SHIFTS B ONE "PIXE"L RIGHT
				scamp7_kernel_begin();

					WHERE(S0);
						mov(C,B);
					all();

					WHERE(RE);
						bus(NEWS,B);//NEWS = -B
						bus(B,XW);//B = -NEWS OF WEST NEIGHBOR

//					ALL();//Not needs as next operation is just setting the FLAG again anyway with WHERE
					WHERE(RW);
						bus(NEWS,B);//NEWS = -B
						bus(B,XE);//B = -NEWS OF WEST NEIGHBOR
					ALL();

					DNEWS0(S6,S0);
					MOV(S0,S6);
				scamp7_kernel_end();
			}




			int time_spent_on_shifting = errode_expand_timer.get_usec();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT REGISTERS AS IMAGES

        	output_timer.reset();
			output_areg_via_bitstack_DNEWS(A,display_00);//captured image
			output_areg_via_bitstack_DNEWS(B,display_01);//shifted image
			output_areg_via_bitstack_DNEWS(C,display_10);//captured image
			scamp7_output_image(S0,display_11);//captured image - shifted image
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

	        vs_post_text("time spent on shifting %d microseconds\n",time_spent_on_shifting);

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
