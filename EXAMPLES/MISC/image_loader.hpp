
#ifndef SCAMP7_IMAGE_LOADER_HPP
#define SCAMP7_IMAGE_LOADER_HPP

#include <scamp7.hpp>
#include <cstdio>
#include <functional>

using namespace SCAMP7_PE;


class scamp7_image_loader{

protected:
	const char*filepath_format;
	uint16_t image_index_first;
	uint16_t image_index_last;
	bool do_reverse;
	vs_handle progress_bar;
	int16_t playback_direction;
	int16_t playback_progress;
	int16_t verbose_mode;
	uint16_t image_index;
	uint16_t image_width;
	uint16_t image_height;
	scamp7_kernel kernel_1;
	scamp7_kernel kernel_2;
	const dreg_t *load_tmp;
	std::function<void(vs_dotmat const&,uint8_t,uint8_t)> _dreg_dst_load;
	std::function<void(vs_dotmat const&,uint8_t,uint8_t)> _dreg_tmp_load;
	std::function<void(int8_t)> _areg_dst_in;
	std::function<void(int8_t)> _areg_tmp_in;

	void _load_video_frame(int idx_inc,int type,bool centering);

public:
	static const int D2A_BITS = 5;

	inline int get_image_index(){
		return image_index;
	};

	inline uint16_t get_image_width(){
		return image_width;
	};

	inline uint16_t get_image_height(){
		return image_height;
	};

	inline void set_verbose_mode(int16_t v){
		verbose_mode = v;
	}

	scamp7_image_loader(AREG _dst1,DREG _dst2,AREG _tmp1,DREG _tmp2);

	std::function<void(void)> waiting_callback;

	void add_gui_items();

	// functions to load a single image
	void load_areg_image(const char*filepath,bool centering=true,bool halfscale=false);
	void load_dreg_image(const char*filepath,bool centering=true);
	/*
	 * @breif load an image by putting the same bit of each greyscale pixels into the same DREG (from high to low)
	 */
	void load_bits_image(const char*filepath,const std::initializer_list<const dreg_t>& dreg_list,bool centering=false);

	// functions to continuously load video frames
	void init_video_frames(const char*filepath,uint16_t first_idx,uint16_t last_idx,bool reverse=false);

	inline void load_video_frame_dreg(int idx_inc=1,bool centering=true){
		return _load_video_frame(idx_inc,0,centering);
	}

	inline void load_video_frame_areg(int idx_inc=1,bool centering=true){
		return _load_video_frame(idx_inc,1,centering);
	}

	inline void load_video_frame_areg_halfscale(int idx_inc=1,bool centering=true){
		return _load_video_frame(idx_inc,2,centering);
	}
};


#endif
