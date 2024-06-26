#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

const areg_t AREG_image = A;
const areg_t AREG_vertical_edges = B;
const areg_t AREG_horizontal_edges = C;

const dreg_t DREG_combined_edges = S0;
const dreg_t DREG_vertical_edges = S1;
const dreg_t DREG_horizontal_edges = S2;

void DREG_load_centered_rect(dreg_t dr, int x, int y, int width, int height)
{
	//PARAMETERES (DREG, top left row, top left column, bottom right row , bottom left column)
	int top_left_row = y-height/2;
	if(top_left_row < 0)
	{
		height += top_left_row;
		top_left_row = 0;
	}
	int top_left_column = x-width/2;
	if(top_left_column < 0)
	{
		width += top_left_column;
		top_left_column = 0;
	}

	scamp7_load_region(dr, top_left_row, top_left_column, top_left_row+height, top_left_column+width);
}

void get_adjusted_image_into_A(int offset = 0, int amp = 0)
{
	scamp7_in(F,offset);
	scamp7_kernel_begin();
		get_image(A,E);
		add(A,A,F);
		mov(F,A);
	scamp7_kernel_end();
	for(int n = 0 ; n < amp ; n++)
	{
		scamp7_kernel_begin();
			add(A,A,F);
		scamp7_kernel_end();
	}
}

//variables for referring to the DREGs into which boxes are loaded, simply used for code readability
const dreg_t DREG_box1 = S0;

int main()
{
    vs_init();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP IMAGE DISPLAYS

    int disp_size = 2;
    auto display_00 = vs_gui_add_display("Captured Image",0,0,disp_size);
    auto display_01 = vs_gui_add_display("Vertical Edges",0,disp_size,disp_size);
    auto display_02 = vs_gui_add_display("Vertical Edges",0,disp_size*2,disp_size);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //SETUP GUI ELEMENTS & CONTROLLABLE VARIABLES

    int image_offset = -30;
    vs_gui_add_slider("image_offset x",-128,128,image_offset,&image_offset);
    int image_amp = 0;
    vs_gui_add_slider("image_amp x",0,10,image_amp,&image_amp);

    int box1_x, box1_y, box1_width, box1_height;
    vs_gui_add_slider("box1 x: ", 0, 255, 64, &box1_x);
    vs_gui_add_slider("box1 y: ", 0, 255, 64, &box1_y);
    vs_gui_add_slider("box1 width: ", 0, 128, 96, &box1_width);
    vs_gui_add_slider("box1 height: ", 0, 128, 64, &box1_height);

    int input_value = -127;
    vs_gui_add_slider("input_value",-127,127,input_value,&input_value);

    vs_stopwatch timer;

    // Frame Loop
    while(1)
    {
        vs_frame_loop_control();


        get_adjusted_image_into_A(image_offset,image_amp);



        timer.reset();

        scamp7_in(B,input_value);

        //load box1 & box2 into DREGs
        DREG_load_centered_rect(DREG_box1,box1_x,box1_y,box1_width,box1_height);

        scamp7_kernel_begin();
        	//FIRST COPY DREG_box1 INTO FLAG
			WHERE(DREG_box1);
				//NOW COPY CAPTURED IMAGE INTO B
				//THIS WILL ONLY BE PERFORMED AT PES WITH FLAG == 1
				//IE ONLY THOSE PES WITHIN THE BOX SHAPE LOADED INTO DREG_box1
				mov(B,A);
			all();
	    scamp7_kernel_end();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT RESULTS STORED IN VARIOUS REGISTERS
	    output_areg_via_bitstack_DNEWS(A,display_00);
	    scamp7_output_image(DREG_box1,display_01);
	    output_areg_via_bitstack_DNEWS(B,display_02);


        vs_post_text("shift time %d microseconds \n",timer.get_usec());

    }
    return 0;
}

