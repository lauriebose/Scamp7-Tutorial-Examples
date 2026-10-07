#include <scamp7.hpp>
#include <vector>
#include "MISC/MISC_FUNCS.hpp"
using namespace SCAMP7_PE;

vs_stopwatch frame_timer;
vs_stopwatch output_timer;
vs_stopwatch sum_timer;

namespace AUTO_EXPOSURE
{
	constexpr areg_t AREG_IMGAGE_REG = A;

	vs_stopwatch timer;

	int use_sum_64 = true;

	int exposure_time;
	int exposure_target_sum;

	//Variables used for calculating statistics for the difference global summation functions
	int sum64_max, sum64_min,sum64_result_normalised;
	int sum16_max, sum16_min,sum16_result_normalised;

	void calibrate_summations()
	{
		//Make register plane A as negative as possible
			scamp7_in(F,-127);
			scamp7_kernel_begin();
				mov(A,F);
				add(A,A,F);
			scamp7_kernel_end();

			sum64_min = scamp7_global_sum_64(A) + 1;//+1s to avoid possibility of divide by 0 later
			sum16_min = scamp7_global_sum_16(A) + 1;
		//	sum_fast_min = scamp7_global_sum_fast(A) + 1;

			//Make register plane A as positive  as possible
			scamp7_in(F,127);
			scamp7_kernel_begin();
				mov(A,F);
				add(A,A,F);
				add(A,A,F);
			scamp7_kernel_end();

			sum64_max = scamp7_global_sum_64(A);
			sum16_max = scamp7_global_sum_16(A);
		//	sum_fast_max = scamp7_global_sum_fast(A);
	}

	void update_exposure_time_with_sum_16()
	{
		//Calculate normalised results, places each value in the range 0-100
		int16_t sum16_result = scamp7_global_sum_16(AREG_IMGAGE_REG);
		sum16_result_normalised = (100*sum16_result)/(sum16_max - sum16_min);
		exposure_time-=(sum64_result_normalised-exposure_target_sum)*10;

		if(exposure_time < 0)
		{
			exposure_time = 0;
		}
	}

	void update_exposure_time_with_sum_64()
	{
		//Calculate normalised results, places each value in the range 0-100
		int16_t sum64_result = scamp7_global_sum_64(AREG_IMGAGE_REG);
		sum64_result_normalised = (100*sum64_result)/(sum64_max - sum64_min);
		exposure_time-=(sum64_result_normalised-exposure_target_sum)*10;

		if(exposure_time < 0)
		{
			exposure_time = 0;
		}
	}

	void update()
	{
		if(use_sum_64)
		{
			update_exposure_time_with_sum_64();
		}
		else
		{
			update_exposure_time_with_sum_16();
		}
	}

	void capture_image()
	{
		int time_since_last_capture = timer.get_usec();

		if(time_since_last_capture > exposure_time)
		{
			scamp7_kernel_begin();
				respix();
			scamp7_kernel_end();
			vs_wait(exposure_time);
			//Capture an Image
			scamp7_kernel_begin();
				get_image(AREG_IMGAGE_REG,E);
			scamp7_kernel_end();
		}
		else
		{
			vs_wait(exposure_time-time_since_last_capture);
			//Capture an Image
			scamp7_kernel_begin();
				get_image(AREG_IMGAGE_REG,E);
				respix();
			scamp7_kernel_end();
		}

		timer.reset();
	}
}

int main()
{
    vs_init();

	VS_GUI_DISPLAY_STYLE(style_plot,R"JSON(
			{
				"plot_palette": "plot_4",
				"plot_palette_groups": 4
			}
			)JSON");

    int display_size = 2;
    auto display_00 = vs_gui_add_display("Image to Sum",0,0,display_size);

    //Add display to plot the results from each global summation function
    auto display_plot = vs_gui_add_display("Red:Sum64, Yellow:Sum16, Green:Sum_fast",0,display_size,display_size,style_plot);
    const int plot_min = 0;
    const int plot_max = 100;
    const int plot_time_frame = 256;
    vs_gui_set_scope(display_plot,plot_min,plot_max,plot_time_frame);

    vs_gui_add_switch("use_sum_64", AUTO_EXPOSURE::use_sum_64 == 1, &AUTO_EXPOSURE::use_sum_64);
    vs_gui_add_slider("exposure_target_sum",0,100,AUTO_EXPOSURE::exposure_target_sum,&AUTO_EXPOSURE::exposure_target_sum);

    AUTO_EXPOSURE::calibrate_summations();

    // Frame Loop
    while(1)
    {
        frame_timer.reset();//reset frame_timer

       	vs_disable_frame_trigger();
        vs_frame_loop_control();


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//CAPTURE OR CREATE THE ANALOGUE DATA TO TEST GLOBAL SUMMATION UPON

			AUTO_EXPOSURE::capture_image();
			AUTO_EXPOSURE::update();

			vs_post_text("Exposure time us %d %d \n",AUTO_EXPOSURE::exposure_time,AUTO_EXPOSURE::sum64_result_normalised );

	   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	   //OUTPUT


			//Create an array of data with the latest values to plot
			int32_t plot_data[3];
			plot_data[0] = AUTO_EXPOSURE::sum64_result_normalised;
			plot_data[1] = AUTO_EXPOSURE::sum16_result_normalised;
//			plot_data[2] = sum_fast_result_normalised;
			plot_data[2] = 0;
			vs_post_set_channel(display_plot);//Set target to "post" data to, the display setup for plotting
			vs_post_int32(plot_data,1,3);//Post the array of data, this should now be plotted in the display

			output_timer.reset();
			{
				output_4bit_image_via_DNEWS(A,display_00);
			}
			int image_output_time_microseconds = output_timer.get_usec();//get the time taken for image output

	    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT TEXT INFO

			int frame_time_microseconds = frame_timer.get_usec(); //get the time taken this frame
			int max_possible_frame_rate = 1000000/frame_time_microseconds; //calculate the possible max FPS
			int image_output_time_percentage = (image_output_time_microseconds*100)/frame_time_microseconds; //calculate the % of frame time which is used for image output
			vs_post_text("frame time %d microseconds(%%%d image output), potential FPS ~%d \n",frame_time_microseconds,image_output_time_percentage,max_possible_frame_rate); //display this values on host
    }

    return 0;
}
