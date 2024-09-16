#include <scamp7.hpp>
using namespace SCAMP7_PE;
#include "MISC/MISC_FUNCS.hpp"

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

int main()
{
    vs_init();

    const int display_size = 1;
    vs_handle display_00 = vs_gui_add_display("S0 (box1)",0,0,2);

    int point_x, point_y;
    vs_gui_add_slider("point x: ", 0, 255, 180, &point_x);
    vs_gui_add_slider("point y: ", 0, 255, 180, &point_y);

    int line_x1, line_y1, line_x2, line_y2;
    vs_gui_add_slider("line x1: ", 0, 255, 64, &line_x1);
    vs_gui_add_slider("line y1: ", 0, 255, 64, &line_y1);
    vs_gui_add_slider("line x2: ", 0, 255, 128, &line_x2);
    vs_gui_add_slider("line y2: ", 0, 255, 128, &line_y2);

    int circle_x, circle_y, circle_rad;
    vs_gui_add_slider("circle x: ", 0, 255, 96, &circle_x);
    vs_gui_add_slider("circle y: ", 0, 255, 96, &circle_y);
    vs_gui_add_slider("circle radius: ", 1, 128, 64, &circle_rad);

    int clear_content_each_frame = 1;
    vs_gui_add_switch("clear_content_each_frame",clear_content_each_frame == 1 , &clear_content_each_frame);

    int negate_content = 1;
    vs_gui_add_switch("negate_content",negate_content == 1 , &negate_content);

    while(1)
    {
    	frame_timer.reset();

    	vs_disable_frame_trigger();
        vs_frame_loop_control();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if(clear_content_each_frame)
        {
        	scamp7_kernel_begin();
        		CLR(S0);
        	scamp7_kernel_end();
        }


		scamp7_draw_begin(S0);
			scamp7_draw_circle(circle_x,circle_y,circle_rad);
			scamp7_draw_line(line_x1, line_y1, line_x2, line_y2);
			scamp7_draw_point(point_x,point_y);
			if(negate_content)
			{
				scamp7_draw_negate();
			}
		scamp7_draw_end();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //OUTPUT IMAGES

        	output_timer.reset();

			 //show DREG of box1 & box2
			scamp7_output_image(S0,display_00);

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
