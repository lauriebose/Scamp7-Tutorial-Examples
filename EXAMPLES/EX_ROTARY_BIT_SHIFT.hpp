#include <scamp7.hpp>
#include "MISC/MISC_FUNCS.hpp"
using namespace SCAMP7_PE;

vs_stopwatch rotary_shift_timer;

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

		int disp_size = 2;
		auto display_00 = vs_gui_add_display("Image",0,0,disp_size);
		auto display_01 = vs_gui_add_display("Thresholded Image",0,disp_size,disp_size);

		auto display_bit0 = vs_gui_add_display("B0",disp_size,0,1);
		auto display_bit1 = vs_gui_add_display("B1",disp_size,1,1);
		auto display_bit2 = vs_gui_add_display("B2",disp_size,2,1);
		auto display_bit3 = vs_gui_add_display("B3",disp_size,3,1);
		auto display_bit4 = vs_gui_add_display("B4",disp_size+1,0,1);
		auto display_bit5 = vs_gui_add_display("B5",disp_size+1,1,1);
		auto display_bit6 = vs_gui_add_display("B6",disp_size+1,2,1);
		auto display_bit7 = vs_gui_add_display("B7",disp_size+1,3,1);
		auto display_bit8 = vs_gui_add_display("B8",disp_size+1,4,1);

		vs_handle rotary_bit_display_handles[9] =
		{display_bit0,display_bit1,display_bit2,display_bit3,display_bit4,
				display_bit5,display_bit6,display_bit7,display_bit8};


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

		int threshold = 0;
		vs_gui_add_slider("Threshold", -127, 127, threshold, &threshold);

		bool store_thresholded_image_to_rotary = 0;
		auto gui_btn_store_to_rotary = vs_gui_add_button("Store & Rotate");
		vs_on_gui_update(gui_btn_store_to_rotary,[&](int32_t new_value)
		{
			store_thresholded_image_to_rotary = true;
		 });

		int shift_rotary = 0;
		auto gui_btn_shift_rotary = vs_gui_add_button("shift_rotary");
		vs_on_gui_update(gui_btn_shift_rotary,[&](int32_t new_value)
		{
			shift_rotary = 1;
		});

		auto gui_btn_shift_rotary_reverse = vs_gui_add_button("shift_rotary reverse");
		vs_on_gui_update(gui_btn_shift_rotary_reverse,[&](int32_t new_value)
		{
			shift_rotary = -1;
	   });


    //MAIN FRAME LOOP
    while(1)
    {
       	vs_disable_frame_trigger();
        vs_frame_loop_control();

    	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//CAPTURE IMAGE AND CREATE THRESHOLDED IMAGE

        	scamp7_in(F,threshold);
			scamp7_kernel_begin();
				get_image(A,E);
				sub(E,A,F);
				where(E);
					MOV(S0,FLAG);
				all();
			scamp7_kernel_end();

    	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//STORE THRESHOLDED IMAGE INTO ROTARY IF BUTTON WAS PRESSED

			if(store_thresholded_image_to_rotary)
			{
				store_thresholded_image_to_rotary = false;
				scamp7_kernel_begin();
					MOV(RZ,S0);
					ROTATE();
				scamp7_kernel_end();
			}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//PERFORM BIT SHIFTING OF CONTENT IN ROTARY REGISTER

			//shift in the forward direction
			if(shift_rotary == 1)
			{
				rotary_shift_timer.reset();
				scamp7_kernel_begin();
					MOV(RN,RZ);//Copy rotary content

					//Bits 1-8
					for(int n = 0 ; n < 4 ; n++)
					{
						ROTATE();
						MOV(RS,RZ);//Copy rotary content
						MOV(RZ,RN);//Paste content of previous rotary bit
						ROTATE();
						MOV(RN,RZ);//Copy rotary content
						MOV(RZ,RS);//Paste content of previous rotary bit
					}

					//Bit 0
					ROTATE();
					MOV(RZ,RN);//Paste content of previous rotary bit
				scamp7_kernel_end();
				int tmp_time = rotary_shift_timer.get_usec();
				vs_post_text("rotary shift timer %d \n",tmp_time);
			}

			//shift in the reverse direction
			//follows same method as forward direction
			if(shift_rotary == -1)
			{
				rotary_shift_timer.reset();
				scamp7_kernel_begin();
					MOV(RN,RZ);

					for(int n = 0 ; n < 4 ; n++)
					{
						//Unfortunately the index of the rotary register can only be moved "forward" via "ROTATE()"
						//To move "backwards" through the rotary we must move forward multiple times until we loop back around...
						for(int r = 0 ; r < 8; r++)
						{
							ROTATE();//Rotate 8 times, there are 9 bits, so this is equivalent to moving backwards 1 bit...
						}
						MOV(RS,RZ);
						MOV(RZ,RN);

						for(int r = 0 ; r < 8; r++)
						{
							ROTATE();
						}
						MOV(RN,RZ);
						MOV(RZ,RS);
					}

					for(int r = 0 ; r < 8; r++)
					{
						ROTATE();
					}

					MOV(RZ,RN);
				scamp7_kernel_end();
				int tmp_time = rotary_shift_timer.get_usec();
				vs_post_text("rotary shift timer %d \n",tmp_time);
			}

			shift_rotary = 0;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT RESULTS STORED IN VARIOUS REGISTERS

			//Display contents of rotary
			int current_rotary_pos = scamp7_kernel_get_rotary_position();
			scamp7_kernel_set_rotary_position(0);
			for(int n = 0 ; n < 9 ; n++)
			{
				scamp7_output_image(RZ,rotary_bit_display_handles[n]);
				scamp7_kernel_begin();
					ROTATE();
				scamp7_kernel_end();
			}
			scamp7_kernel_set_rotary_position(current_rotary_pos);//Restore rotary position

	    	output_4bit_image_via_DNEWS(A,display_00);
	    	scamp7_output_image(S0,display_01);
    }
    return 0;
}
