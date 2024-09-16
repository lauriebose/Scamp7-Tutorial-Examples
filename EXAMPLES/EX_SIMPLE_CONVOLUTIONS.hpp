#include <scamp7.hpp>

#include "MISC/MISC_FUNCS.hpp"
using namespace SCAMP7_PE;
#include "../../s5d_m0_scamp7/src/debug_gui.hpp"

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch areg_shift_timer;

int main()
{
    vs_init();

    int display_size = 2;
    auto display_00 = vs_gui_add_display("Captured Image, AREG A",0,0,display_size);
    auto display_01 = vs_gui_add_display("Shifted Imag, AREG B",0,display_size,display_size);
    auto display_02 = vs_gui_add_display("Captured Image - Shifted Image, AREG C = A - B",0,2*display_size,display_size);

    int pre_divisions = 2;
    vs_gui_add_slider("pre_divisions",0,5,pre_divisions,&pre_divisions);

    int steps = 9;
    vs_gui_add_slider("steps",0,9,steps,&steps);

    int maxpooling = 0;
    vs_gui_add_slider("maxpooling",0,9,maxpooling,&maxpooling);


	setup_voltage_configurator(false);

    // Frame Loop
    while(1)
    {
        frame_timer.reset();//reset frame_timer

       	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//capture new image into AREG A and create copy in B

			scamp7_kernel_begin();
				get_image(A,E);
				mov(B,A);
			scamp7_kernel_end();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SHIFT AREG HORIZONTALLY

			areg_shift_timer.reset();

			for(int n = 0 ; n < pre_divisions ; n++)
			{
				scamp7_kernel_begin();
					diva(B,F,E);
				scamp7_kernel_end();
			}


			if(steps > 0)
			{
				scamp7_kernel_begin();
					neg(C,B);
				scamp7_kernel_end();
			}

			if(steps > 1)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XW);//B = -NEWS OF WEST NEIGHBOR
					sub(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 2)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XW);//B = -NEWS OF WEST NEIGHBOR
					sub(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 3)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XS);//B = -NEWS OF SOUTH NEIGHBOR
					sub(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 4)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XE);//B = -NEWS OF EAST NEIGHBOR
					sub(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 5)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XE);//B = -NEWS OF EAST NEIGHBOR
					add(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 6)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XS);//B = -NEWS OF SOUTH NEIGHBOR
					add(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 7)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XW);//B = -NEWS OF WEST NEIGHBOR
					add(C,C,B);
				scamp7_kernel_end();
			}

			if(steps > 8)
			{
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XW);//B = -NEWS OF WEST NEIGHBOR
					add(C,C,B);
				scamp7_kernel_end();
			}


			int time_spent_on_convolution = areg_shift_timer.get_usec();


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT REGISTERS AS IMAGES

        	output_timer.reset();
        	output_4bit_image_via_DNEWS(A,display_00);//captured image
        	output_4bit_image_via_DNEWS(B,display_01);//shifted image
        	output_4bit_image_via_DNEWS(C,display_02);//captured image - shifted image
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

	        vs_post_text("time spent on convolution %d microseconds\n",time_spent_on_convolution);

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			int image_output_time_percentage = (output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
			vs_post_text("frame time %d microseconds(%%%d image output), potential FPS ~%d \n",frame_time_microseconds,image_output_time_percentage,max_possible_frame_rate); //display this values on host
    }

    return 0;
}

