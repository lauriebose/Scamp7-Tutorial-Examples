#include <scamp7.hpp>
using namespace SCAMP7_PE;
#include "MISC/MISC_FUNCS.hpp"
#include <iostream>
#include <string>

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

void output_byte_as_binary(int number)
{
	std::string event_data_string;
	for(int b = 0 ; b < 8 ; b++)
	{
		int bit_value = 128 >> b;
		(number & bit_value) ? event_data_string += "1" : event_data_string += "0";
	}
	vs_post_text("%s",event_data_string.c_str());
}

int main()
{
    vs_init();

    const int display_size = 3;
    vs_handle display_00 = vs_gui_add_display("Loaded Pattern",0,0,display_size);

    int row;
    vs_gui_add_slider("Row: ",0,255,155,&row);
    int rowx;
    vs_gui_add_slider("RowX: ",0,255,14,&rowx);

    int col;
    vs_gui_add_slider("Col: ",0,255,1,&col);
    int colx;
    vs_gui_add_slider("ColX: ",0,255,254,&colx);


    while(1)
    {
    	frame_timer.reset();

    	vs_disable_frame_trigger();
        vs_frame_loop_control();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //LOAD PATTERN
        	scamp7_load_pattern(S1, row, col, rowx, colx);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT PARAMETERS AS BINARY

			vs_post_text("Row: ");
			output_byte_as_binary(row);
			vs_post_text("	");

			vs_post_text("RowX: ");
			output_byte_as_binary(rowx);
			vs_post_text("	");

			vs_post_text("Col: ");
			output_byte_as_binary(col);
			vs_post_text("	");

			vs_post_text("ColX: ");
			output_byte_as_binary(colx);
			vs_post_text("\n");

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //OUTPUT IMAGES

        	output_timer.reset();
			 //show DREG of box1 & box2
			scamp7_output_image(S1,display_00);
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


