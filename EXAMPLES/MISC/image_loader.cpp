
#include "image_loader.hpp"
#include <vector>

const char*CSTR_MODULE_NAME = "[image_loader]";
const char*CSTR_ERROR_TIMEOUT = "[image_loader] image not received in time!\n";


scamp7_image_loader::scamp7_image_loader(AREG _dst1,DREG _dst2,AREG _tmp1,DREG _tmp2){
	using namespace std::placeholders;

	verbose_mode = 0;
	filepath_format = NULL;
	image_index_first = 0;
	image_index_last = 0;
	image_index = 0;
	progress_bar = NULL;
	waiting_callback = NULL;

	_areg_dst_in = std::bind(scamp7_in,_dst1,_1);
	_areg_tmp_in = std::bind(scamp7_in,_tmp1,_1);
	_dreg_dst_load = std::bind(scamp7_load_dreg,_dst2,_1,_2,_3);
	_dreg_tmp_load = std::bind(scamp7_load_dreg,_tmp2,_1,_2,_3);
	load_tmp = &_tmp2;

	using namespace scamp7_kernel_api;
	kernel_1([&](){
		WHERE(_tmp2);
		add(_dst1,_dst1,_tmp1);
		ALL();
	});
}


void scamp7_image_loader::load_areg_image(const char*filepath,bool centering,bool halfscale){
	vs_gui_request_image(filepath,D2A_BITS,[this,centering,halfscale](vs_dotmat const& dotmat,int s){
		if(s<0){
			vs_post_text(CSTR_ERROR_TIMEOUT);
			return;
		}else
		if(s<D2A_BITS){
			int w = image_width = dotmat.get_width();
			int h = image_height = dotmat.get_height();
			int r0 = 0;
			int c0 = 0;
			if(centering){
				r0 = 128 - h/2;
				c0 = 128 - w/2;
			}
			_dreg_tmp_load(dotmat,r0,c0);
			if(s==0){
				if(halfscale){
					_areg_dst_in(0);
				}else{
					_areg_dst_in(-127);
				}
			}
			int level = (1<<(8 - D2A_BITS))*(1<<s);
			if(halfscale){
				level /= 2;
			}
			if(level>126){
				level = 126;
			}
			_areg_tmp_in(level);// note: range of in is [-128,127], thus +128 is out of range
			scamp7_launch_kernel(kernel_1);
		}
	});
	vs_stopwatch refresh_timer;
	do{
		if(refresh_timer.get_usec()>5000){
			refresh_timer.reset();
			scamp7_kernel_refresh_dreg();
		}
		vs_process_message();
		if(waiting_callback){
			waiting_callback();
		}
	}while(!vs_gui_request_done());
}


void scamp7_image_loader::load_dreg_image(const char*filepath,bool centering){
	vs_gui_request_image(filepath,1,[this,centering](vs_dotmat const& dotmat,int s){
		if(s<0){
			vs_post_text(CSTR_ERROR_TIMEOUT);
			return;
		}else{
			int w = image_width = dotmat.get_width();
			int h = image_height = dotmat.get_height();
			int r0 = 0;
			int c0 = 0;
			if(centering){
				r0 = 128 - h/2;
				c0 = 128 - w/2;
			}
			_dreg_dst_load(dotmat,r0,c0);
		}
	});
	vs_stopwatch refresh_timer;
	do{
		if(refresh_timer.get_usec()>1000){
			refresh_timer.reset();
			scamp7_kernel_refresh_dreg();
		}
		vs_process_message();
		if(waiting_callback){
			waiting_callback();
		}
	}while(!vs_gui_request_done());
}


void scamp7_image_loader::load_bits_image(const char*filepath,const std::initializer_list<const dreg_t>& dreg_args,bool centering){
	if(dreg_args.size()<1){
		return;
	}
	if(dreg_args.size()>8){
		return;
	}
	std::vector<dreg_t> dreg_bits;
	for(auto&o:dreg_args){
		dreg_bits.push_back(o);
	}
	vs_gui_request_image(filepath,dreg_bits.size(),[this,centering,&dreg_bits](vs_dotmat const& dotmat,int s){
		if(s<0){
			vs_post_text(CSTR_ERROR_TIMEOUT);
			return;
		}else
		if(s<dreg_bits.size()){
			int w = image_width = dotmat.get_width();
			int h = image_height = dotmat.get_height();
			int r0 = 0;
			int c0 = 0;
			if(centering){
				r0 = 128 - h/2;
				c0 = 128 - w/2;
			}
			_dreg_tmp_load(dotmat,r0,c0);
			scamp7_dynamic_kernel_begin();
				MOV(dreg_bits[dreg_bits.size() - s - 1],*load_tmp);
			scamp7_dynamic_kernel_end();
		}
	});
	vs_stopwatch refresh_timer;
	do{
		if(refresh_timer.get_usec()>5000){
			refresh_timer.reset();
			scamp7_kernel_refresh_dreg();
		}
		vs_process_message();
		if(waiting_callback){
			waiting_callback();
		}
	}while(!vs_gui_request_done());
}


void scamp7_image_loader::add_gui_items(){
	progress_bar = vs_gui_add_slider("video time: ",image_index_first,image_index_last,0,NULL);
	vs_on_gui_update(progress_bar,[this](int32_t new_value){
		if(new_value != image_index)
		{
			vs_post_text("%s video time: %d\n",CSTR_MODULE_NAME,new_value);
			image_index = new_value;
		}
	});

	vs_gui_add_slider("playback speed: ",-5,5,playback_speed,&playback_speed);
}


void scamp7_image_loader::init_video_frames(const char*filepath,uint16_t first_idx,uint16_t last_idx,bool perform_ping_pong_playback){
	filepath_format = filepath;
	image_index_first = first_idx;
	image_index_last = last_idx;
	ping_pong_playback = perform_ping_pong_playback;
	image_index = -1;
	playback_direction = 1;
}


void scamp7_image_loader::_load_video_frame(int index_change = -1,int type = 2,bool centering = true)
{
	char filepath[256] = "";

	if(index_change = -1)
	{
		index_change = playback_speed;
	}

	if(vs_gui_request_done() && vs_gui_is_on())
	{
		if(index_change != 0)
		{

			//Update the index of the current image to load
			image_index += index_change * playback_direction;
			if(image_index <= image_index_first)
			{
				if(ping_pong_playback)
				{
					playback_direction *=-1;
					image_index = image_index_first;
				}
				else
				{
					image_index = image_index_last;
				}
			}
			else
			{
				if(image_index >= image_index_last)
				{
					if(ping_pong_playback)
					{
						playback_direction *=-1;
						image_index = image_index_last;
					}
					else
					{
						image_index = image_index_first;
					}
				}
			}
		}

		snprintf(filepath,256,filepath_format,image_index);
		if(verbose_mode)
		{
			vs_post_text("%s %s\n",CSTR_MODULE_NAME,filepath);
		}

		if(progress_bar)
		{
			vs_gui_move_slider(progress_bar,image_index,false);
		}

		switch(type)
		{
			case 0:
				load_dreg_image(filepath,centering);
				break;

			case 1:
				load_areg_image(filepath,centering,false);
				break;

			case 2:
				load_areg_image(filepath,centering,true);
				break;
		}

	}
}

