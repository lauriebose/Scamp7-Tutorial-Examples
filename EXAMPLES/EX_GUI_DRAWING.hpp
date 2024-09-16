#include <scamp7.hpp>
using namespace SCAMP7_PE;
#include "MISC/MISC_FUNCS.hpp"

vs_stopwatch frame_timer;
vs_stopwatch output_timer;

int main()
{
    vs_init();

    const int display_size = 2;
    vs_handle display_00 = vs_gui_add_display("S0 (box1)",0,0,display_size);

    int point_x, point_y, point_size;
    vs_gui_add_slider("point x: ", 0, 255, 180, &point_x);
    vs_gui_add_slider("point y: ", 0, 255, 180, &point_y);
    vs_gui_add_slider("point_size: ", 0, 100, 10, &point_size);

    int line_x1, line_y1, line_x2, line_y2;
    vs_gui_add_slider("line x1: ", 0, 255, 64, &line_x1);
    vs_gui_add_slider("line y1: ", 0, 255, 64, &line_y1);
    vs_gui_add_slider("line x2: ", 0, 255, 128, &line_x2);
    vs_gui_add_slider("line y2: ", 0, 255, 128, &line_y2);

    int rect_x, rect_y, rect_width, rect_height, rect_rotation;
    vs_gui_add_slider("rect x: ", 0, 255, 64, &rect_x);
    vs_gui_add_slider("rect y: ", 0, 255, 64, &rect_y);
    vs_gui_add_slider("rect_width: ", 0, 255, 64, &rect_width);
    vs_gui_add_slider("rect_height: ", 0, 255, 64, &rect_height);
    vs_gui_add_slider("rect_rotation: ", 0, 360, 0, &rect_rotation);

    static volatile int line_width = 2;
    vs_gui_add_slider("line_width: ",2,10,line_width,&line_width);

    int circle_x, circle_y, circle_rad, circle_detail;
    vs_gui_add_slider("circle x: ", 0, 255, 96, &circle_x);
    vs_gui_add_slider("circle y: ", 0, 255, 96, &circle_y);
    vs_gui_add_slider("circle radius: ", 1, 128, 64, &circle_rad);
    vs_gui_add_slider("circle_detail: ", 1, 50, 25, &circle_detail);

    static volatile int use_kernel_space = 1;
    vs_gui_add_switch("use_kernel_space: ",use_kernel_space,&use_kernel_space);

    int drawing_red, drawing_green, drawing_blue, drawing_alpha;
    vs_gui_add_slider("drawing_red", 0, 255, 255, &drawing_red);
    vs_gui_add_slider("drawing_green", 0, 255, 64, &drawing_green);
    vs_gui_add_slider("drawing_blue", 0, 255, 255, &drawing_blue);
    vs_gui_add_slider("drawing_alpha", 0, 255, 255, &drawing_alpha);

    while(1)
    {
    	frame_timer.reset();

    	vs_disable_frame_trigger();
        vs_frame_loop_control();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        vs_gui_display_graphics(display_00,[&]()
		{
				using namespace vs_gui_graphics_api;

				set_pointsize(point_size);
				set_linewidth(line_width);
				set_color({drawing_red,drawing_green,drawing_blue,drawing_alpha});

				if(use_kernel_space)
				{
					//Apply transformations matrices
					//All coordinates of drawing commands will be transformed by these
					//These are applied so that drawing coordinate align with coordinates of PEs in the array

					//Scale all drawing coordinates by the size of the display being drawn to
					m_scale(display_size,display_size,1);

					//Rotate all drawing coordinates 180 about the center of the array (128,128)
					m_translate(128,128);
					m_rotate(180);
					m_translate(-128,-128);
				}

				//draw rectangle
				{
					m_push();//save current drawing transformations to top of "matrix stack"

					//moves drawing origin to center of rectangle, and then rotate about this point
					m_translate(rect_x,rect_y);
					m_rotate(rect_rotation);

					draw_rect(-rect_width /2,-rect_height /2,rect_width /2,rect_height /2);
					m_pop();//Revert current drawing transformations to the top of the "matrix stack", and remove(pop) the top of the stack
				}

				draw_circle(circle_x,circle_y,circle_rad,circle_detail);
				draw_line(line_x1,line_y1,line_x2,line_y2);
				draw_point(point_x,point_y);
			});

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
