#include <scamp7.hpp>
using namespace SCAMP7_PE;

#ifndef OUTPUT_AREG_BITSTACK_HPP
#define OUTPUT_AREG_BITSTACK_HPP
	const int preset_in_val1 = 127;
	const int preset_in_val2 = 74;
	const int preset_in_val3 = 38;
	const int preset_in_val4 = 20;

	void output_4bit_F_via_DNEWS(areg_t reg,vs_handle display,bool use_div = false, int in_val1 = preset_in_val1, int in_val2 = preset_in_val2, int in_val3 = preset_in_val3, int in_val4 = preset_in_val4)
	{
		if(use_div)
		{
			scamp7_in(E,127);
			scamp7_kernel_begin();
				CLR(RS,RW,RN,RE);

				where(F);
					MOV(RE,FLAG);
					NOT(RF,FLAG);
				WHERE(RF);
					add(F,F,E);
				all();

				divq(D,E);
				mov(E,D);

				sub(D,F,E);
				where(D);
					MOV(RN,FLAG);
					mov(F,D);
				all();

				divq(D,E);
				mov(E,D);

				sub(D,F,E);
				where(D);
					MOV(RW,FLAG);
					mov(F,D);
				all();

				divq(D,E);
				mov(E,D);

				sub(D,F,E);
				where(D);
					MOV(RS,FLAG);
					mov(F,D);
				all();
			scamp7_kernel_end();

	        scamp7_output_bitstack_begin(display,4);
	        scamp7_output_bitstack_bit(RE);
	        scamp7_output_bitstack_bit(RN);
	        scamp7_output_bitstack_bit(RW);
	        scamp7_output_bitstack_bit(RS);
	        scamp7_output_bitstack_end();
		}
		else
		{

			scamp7_in(E,127);
			scamp7_kernel_begin();
				CLR(RS,RW,RN,RE);

				where(F);
					MOV(RE,FLAG);
					NOT(RF,FLAG);
				WHERE(RF);
					add(F,F,E);
				all();
			scamp7_kernel_end();

			scamp7_in(E,64);
			scamp7_kernel_begin();
				sub(E,E,F);
				where(E);
					NOT(RN,FLAG);
				WHERE(RN);
					bus(F,E);
				all();
			scamp7_kernel_end();

			scamp7_in(E,32);
			scamp7_kernel_begin();
				sub(E,E,F);
				where(E);
					NOT(RW,FLAG);
				WHERE(RW);
					bus(F,E);
				all();
			scamp7_kernel_end();

			scamp7_in(E,16);
			scamp7_kernel_begin();
				sub(E,E,F);
				where(E);
					NOT(RS,FLAG);
				WHERE(RS);
					bus(F,E);
				all();
			scamp7_kernel_end();

	        scamp7_output_bitstack_begin(display,4);
	        scamp7_output_bitstack_bit(RE);
	        scamp7_output_bitstack_bit(RN);
	        scamp7_output_bitstack_bit(RW);
	        scamp7_output_bitstack_bit(RS);
	        scamp7_output_bitstack_end();
		}
	}

	void output_4bit_image_via_DNEWS(areg_t reg,vs_handle display,bool use_div = false, int in_val1 = 127, int in_val2 = 77, int in_val3 = 40, int in_val4 = 21)
	{
		scamp7_dynamic_kernel_begin();
			mov(F,reg);
		scamp7_dynamic_kernel_end();
		output_4bit_F_via_DNEWS(reg,display,use_div,in_val1,in_val2,in_val3,in_val4);
	}
#endif
