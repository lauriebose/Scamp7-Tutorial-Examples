#include <scamp7.hpp>
using namespace SCAMP7_PE;
#include "MISC/MISC_FUNCS.hpp"

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

int main()
{
    vs_init();

    const int display_size = 2;
    vs_handle display_00 = vs_gui_add_display("Loaded Pattern",0,0,display_size);
    vs_handle display_01 = vs_gui_add_display("Loaded Pattern",0,display_size,display_size);
    vs_handle display_02 = vs_gui_add_display("Loaded Pattern",0,display_size*2,display_size);

    vs_handle display_10 = vs_gui_add_display("S0",display_size,0,display_size);
    vs_handle display_11 = vs_gui_add_display("S1",display_size,1,display_size);
    vs_handle display_12 = vs_gui_add_display("S2",display_size,2,display_size);
    vs_handle display_13 = vs_gui_add_display("S3",display_size,3,display_size);

    int store_image = -1;

    vs_handle gui_btn_store_image_0 = vs_gui_add_button("store_image_0");
	vs_on_gui_update(gui_btn_store_image_0,[&](int32_t new_value)
	{
		store_image = 0;
    });

    vs_handle gui_btn_store_image_1 = vs_gui_add_button("store_image_1");
	vs_on_gui_update(gui_btn_store_image_1,[&](int32_t new_value)
	{
		store_image = 1;
    });

    vs_handle gui_btn_store_image_2 = vs_gui_add_button("store_image_2");
	vs_on_gui_update(gui_btn_store_image_2,[&](int32_t new_value)
	{
		store_image = 2;
    });

    vs_handle gui_btn_store_image_3 = vs_gui_add_button("store_image_3");
	vs_on_gui_update(gui_btn_store_image_3,[&](int32_t new_value)
	{
		store_image = 3;
    });


    int shown_image_index = 0;
    vs_gui_add_slider("shown_image_index: ",0,3,shown_image_index,&shown_image_index);

    while(1)
    {
    	frame_timer.reset();

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

    	scamp7_kernel_begin();
			get_image(A,F);
		scamp7_kernel_end();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //LOAD PATTERN

			scamp7_in(F,64);
        	if(store_image != -1)
        	{
        		int col = store_image % 2 == 0 ? 0 : 1;
        		int row = store_image / 2 == 0 ? 0 : 1;
        		scamp7_load_pattern(S6, row, col, 254, 254);
        		scamp7_kernel_begin();
        			NOT(S5,S6);
        			AND(S0,S0,S5);
        			AND(S1,S1,S5);
        			AND(S2,S2,S5);
        			AND(S3,S3,S5);
        		scamp7_kernel_end();


        		scamp7_in(E,127);
				scamp7_kernel_begin();
					where(A);
						AND(S5,S6,FLAG);
						OR(S0,S5);

						NOT(RF,FLAG);
					WHERE(RF);
						add(A,A,E);
					all();
				scamp7_kernel_end();

				scamp7_in(E,64);
				scamp7_kernel_begin();
					sub(E,E,A);
					where(E);// where A < E
						NOT(S5,FLAG);
						AND(S5,S5,S6);
						OR(S1,S5);
					WHERE(S1);
						bus(A,E);
					all();
				scamp7_kernel_end();

				scamp7_in(E,32);
				scamp7_kernel_begin();
					sub(E,E,A);
					where(E);
						NOT(S5,FLAG);
						AND(S5,S5,S6);
						OR(S2,S5);
					WHERE(S2);
						bus(A,E);
					all();
				scamp7_kernel_end();

				scamp7_in(E,16);
				scamp7_kernel_begin();
					sub(E,E,A);
					where(E);
						NOT(S5,FLAG);
						AND(S5,S5,S6);
						OR(S3,S5);
					WHERE(S3);
						bus(A,E);
					all();
				scamp7_kernel_end();


        		store_image = -1;
        	}




			//Create interleaved analogue image from the interleaved image data stored in S0,S1,S2,S3
        	scamp7_in(C,-127);
        	scamp7_in(E,127);
        	scamp7_kernel_begin();
				WHERE(S0);
					add(C,C,E);
				ALL();

				divq(D,E);//D = 64
				WHERE(S1);
					add(C,C,D);
				ALL();

				divq(E,D);//E = 32
				WHERE(S2);
					add(C,C,E);
				ALL();

				divq(D,E);//D = 16
				WHERE(S3);
					add(C,C,D);
				ALL();
			scamp7_kernel_end();

			//Extract and show a specific image from the interleaved analogue image
			{
				int col = shown_image_index % 2 == 0 ? 0 : 1;
				int row = shown_image_index / 2 == 0 ? 0 : 1;
				scamp7_load_pattern(S6, row, col, 254, 254);
				scamp7_in(D,0);
				scamp7_kernel_begin();
					mov(F,D);
					WHERE(S6);
						mov(D,C);
						mov(F,C);
					ALL();

					bus(NEWS,F);
					bus(F,XE);
					add(D,D,F);

					mov(F,D);
					bus(NEWS,F);
					bus(F,XS);
					add(D,D,F);
				scamp7_kernel_end();
			}


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT IMAGES


			output_timer.reset();
			output_4bit_image_via_DNEWS(C,display_00);
			output_4bit_image_via_DNEWS(D,display_02);
			output_4bit_image_via_DNEWS(A,display_01);

			scamp7_output_image(S0,display_10);
			scamp7_output_image(S1,display_11);
			scamp7_output_image(S2,display_12);
			scamp7_output_image(S3,display_13);
			int output_time_microseconds = output_timer.get_usec();//get the time taken for image output

//	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//OUTPUT TEXT INFO
//
//  		int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
//  		int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
//  		int image_output_time_percentage = (output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
//        vs_post_text("frame time %d microseconds(%%%d image output), potential FPS ~%d \n",frame_time_microseconds,image_output_time_percentage,max_possible_frame_rate); //display this values on host
    }

    return 0;
}


