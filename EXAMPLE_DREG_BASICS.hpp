#include <scamp7.hpp>
using namespace SCAMP7_PE;

void DREG_load_centered_rect(dreg_t dr, int x, int y, int width, int height)
{
	//PARAMETERES (DREG, top left row, top left column, bottom right row , bottom left column)
	scamp7_load_region(dr, y-height/2, x-width/2, y+height/2+height%2, x+width/2+width%2);
}

//variables for referring to the DREGs into which boxes are loaded, simply used for code readability
const dreg_t DREG_box1 = S0;
const dreg_t DREG_box2 = S1;

int main()
{
    vs_init();

    vs_stopwatch timer;

    const int display_size = 1;
    vs_handle display_00 = vs_gui_add_display("box1",0,0,2);
    vs_handle display_01 = vs_gui_add_display("box2",0,2,2);
    vs_handle display_10 = vs_gui_add_display("box1 AND box2",2,0,display_size);
    vs_handle display_11 = vs_gui_add_display("box1 OR box2",2,display_size,display_size);
    vs_handle display_12 = vs_gui_add_display("NOT (box1 AND box2)",2,display_size*2,display_size);
    vs_handle display_13 = vs_gui_add_display("box1 XOR box2",2,display_size*3,display_size);

    int box1_x, box1_y, box1_width, box1_height;
    vs_gui_add_slider("box1 x: ", 0, 255, 128, &box1_x);
    vs_gui_add_slider("box1 y: ", 0, 255, 128, &box1_y);
    vs_gui_add_slider("box1 width: ", 0, 128, 32, &box1_width);
    vs_gui_add_slider("box1 height: ", 0, 128, 64, &box1_height);

    int box2_x, box2_y, box2_width, box2_height;
    vs_gui_add_slider("box2 x: ", 0, 255, 96, &box2_x);
    vs_gui_add_slider("box2 y: ", 0, 255, 96, &box2_y);
    vs_gui_add_slider("box2 width: ", 0, 128, 64, &box2_width);
    vs_gui_add_slider("box2 height: ", 0, 128, 64, &box2_height);

    while(1){

        vs_frame_loop_control();
        timer.reset();

//        scamp7_load_region(S5,box1_y,box1_x,box1_y+box1_height,box1_x+box1_width);

        //load box1 & box2 into DREGs
        DREG_load_centered_rect(DREG_box1,box1_x,box1_y,box1_width,box1_height);
        DREG_load_centered_rect(DREG_box2,box2_x,box2_y,box2_width,box2_height);

        //show DREG of box1 & box2
        scamp7_output_image(DREG_box1,display_00);
        scamp7_output_image(DREG_box2,display_01);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //COMPUTE VARIOUS LOGIC OPERATIONS
        scamp7_kernel_begin();
        	OR(S6,DREG_box1,DREG_box2);//S6 = box1 OR box2

        	AND(S5,DREG_box1,DREG_box2);//S5 = box1 AND box2

        	NOT(S4,S5);//S4 = NOT(S5) == NOT(box1 AND box2)

        	AND(S3,S6,S4); //S3 = S6 AND S4 == (box1 OR box2) AND NOT(box1 and box1) == box1 XOR box2
        scamp7_kernel_end();

        scamp7_output_image(S6,display_10);//show OR result
	    scamp7_output_image(S5,display_11);//show AND result
		scamp7_output_image(S4,display_12);//show NOT(AND) result
		scamp7_output_image(S3,display_13);//show XOR result

		int time = timer.get_usec();
		vs_post_text("box1(%d,%d,%d,%d) box2(%d,%d,%d,%d) frametime:%d microseconds \n",box1_x,box1_y,box1_width,box1_height,box2_x,box2_y,box2_width,box2_height,time);
    }

    return 0;
}
