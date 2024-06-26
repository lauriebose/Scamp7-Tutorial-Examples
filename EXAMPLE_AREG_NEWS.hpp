#include <scamp7.hpp>
#include "MISC/OUTPUT_AREG_BITSTACK.hpp"
using namespace SCAMP7_PE;

int main()
{
    vs_init();

    int display_size = 2;
    auto display_00 = vs_gui_add_display("Captured Image",0,0,display_size);
    auto display_01 = vs_gui_add_display("Shifted Image",0,display_size,display_size);
    auto display_10 = vs_gui_add_display("Captured Image - Shifted Image",display_size,0,display_size);
    auto display_11 = vs_gui_add_display("Abs(Captured Image - Shifted Image)",display_size,display_size,display_size);

    int image_offset = -30;
    vs_gui_add_slider("image_offset x",-128,128,image_offset,&image_offset);
    int image_amp = 0;
    vs_gui_add_slider("image_amp x",0,10,image_amp,&image_amp);


    int shift_x = -3;
    vs_gui_add_slider("shift x",-128,128,shift_x,&shift_x);
    int shift_y = 3;
    vs_gui_add_slider("shift y",-128,128,shift_y,&shift_y);

    vs_stopwatch timer;

    // Frame Loop
    while(1){
        vs_frame_loop_control();


        scamp7_in(F,image_offset);
        scamp7_kernel_begin();
        	get_image(A,E);
        	add(A,A,F);
        scamp7_kernel_end();


        scamp7_kernel_begin();
     		mov(F,A);
		scamp7_kernel_end();
        for(int n = 0 ; n < image_amp ; n++)
        {
            scamp7_kernel_begin();
            	add(A,A,F);
			scamp7_kernel_end();
        }


        scamp7_kernel_begin();
			mov(B,A);
		scamp7_kernel_end();
        timer.reset();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SHIFT AREG HORIZONTALLY
        if(shift_x > 0)
        {
            for(int x = 0; x < shift_x; x++)
            {
            	//KERNEL SHIFTS B ONE "PIXE"L RIGHT
                scamp7_kernel_begin();
                	bus(NEWS,B);//NEWS = -B
                	bus(B,XW);//B = -NEWS OF WEST NEIGHBOR
    			scamp7_kernel_end();
            }
        }
        else
        {
        	for(int x = 0; x < -shift_x; x++)
			{
        	 	//KERNEL SHIFTS B ONE "PIXEL" LEFT
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XE);//B = -NEWS OF EAST NEIGHBOR
				scamp7_kernel_end();
			}
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //NOW SHIFT AREG HORIZONTALLY
        if(shift_y > 0)
		{
			for(int y = 0; y < shift_y; y++)
			{
				//KERNEL SHIFTS B ONE "PIXEL" UP
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XS);//B = -NEWS OF SOUTH NEIGHBOR
				scamp7_kernel_end();
			}
		}
		else
		{
			for(int y = 0; y < -shift_y; y++)
			{
				//KERNEL SHIFTS B ONE "PIXEL" DOWN
				scamp7_kernel_begin();
					bus(NEWS,B);//NEWS = -B
					bus(B,XN);//B = -NEWS OF NORTH NEIGHBOR
				scamp7_kernel_end();
			}
		}
        int naive_shift_time = timer.get_usec();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //SUBTRACT SHIFTED IMAGE FROM ORIGINAL
		scamp7_kernel_begin();
			sub(C,A,B);
			abs(D,C);
		scamp7_kernel_end();

        vs_post_text("shift time %d microseconds \n",naive_shift_time);

        output_areg_via_bitstack_DNEWS(A,display_00);
        output_areg_via_bitstack_DNEWS(B,display_01);
        output_areg_via_bitstack_DNEWS(C,display_10);
        output_areg_via_bitstack_DNEWS(D,display_11);
    }

    return 0;
}

void shift_F(int shiftx, int shifty)
{
	if(shiftx > 0)
	{
		while(shiftx > 0)
		{
			if(shiftx % 8 == 0)
			{
				scamp7_kernel_begin();
					bus(XE,F);
					bus(F,XW);
					bus(XE,F);
					bus(F,XW);
					bus(XE,F);
					bus(F,XW);
					bus(XE,F);
					bus(F,XW);
				scamp7_kernel_end();
				shiftx-=8;
			}
			else
			{
				if(shiftx % 4 == 0)
				{
					scamp7_kernel_begin();
						bus(XE,F);
						bus(F,XW);
						bus(XE,F);
						bus(F,XW);
					scamp7_kernel_end();
					shiftx-=4;
				}
				else
				{
					if(shiftx % 2 == 0)
					{
						scamp7_kernel_begin();
							bus(XE,F);
							bus(F,XW);
						scamp7_kernel_end();
						shiftx-=2;
					}
					else
					{
						scamp7_kernel_begin();
							bus(NEWS,F);
							bus(F,XW);
						scamp7_kernel_end();
						shiftx--;
					}
				}
			}
		}
	}
	else
	{
		while(shiftx < 0)
		{

			if(shiftx % 8 == 0)
			{
				scamp7_kernel_begin();
					bus(XW,F);
					bus(F,XE);
					bus(XW,F);
					bus(F,XE);
					bus(XW,F);
					bus(F,XE);
					bus(XW,F);
					bus(F,XE);
				scamp7_kernel_end();
				shiftx+=8;
			}
			else
			{
				if(shiftx % 4 == 0)
				{
					scamp7_kernel_begin();
						bus(XW,F);
						bus(F,XE);
						bus(XW,F);
						bus(F,XE);
					scamp7_kernel_end();
					shiftx+=4;
				}
				else
				{
					if(shiftx % 2 == 0)
					{
						scamp7_kernel_begin();
							bus(XW,F);
							bus(F,XE);
						scamp7_kernel_end();
						shiftx+=2;
					}
					else
					{
						scamp7_kernel_begin();
							bus(NEWS,F);
							bus(F,XE);
						scamp7_kernel_end();
						shiftx++;
					}
				}
			}
		}
	}

	if(shifty > 0)
	{
		while(shifty > 0)
		{
			if(shifty % 8 == 0)
			{
				scamp7_kernel_begin();
					bus(XN,F);
					bus(F,XS);
					bus(XN,F);
					bus(F,XS);
					bus(XN,F);
					bus(F,XS);
					bus(XN,F);
					bus(F,XS);
				scamp7_kernel_end();
				shifty-=8;
			}
			else
			{
				if(shifty % 4 == 0)
				{
					scamp7_kernel_begin();
					bus(XN,F);
					bus(F,XS);
					bus(XN,F);
					bus(F,XS);
					scamp7_kernel_end();
					shifty-=4;
				}
				else
				{
					if(shifty % 2 == 0)
					{
						scamp7_kernel_begin();
						bus(XN,F);
						bus(F,XS);
						scamp7_kernel_end();
						shifty-=2;
					}
					else
					{
						scamp7_kernel_begin();
							bus(NEWS,F);
							bus(F,XS);
						scamp7_kernel_end();
						shifty--;
					}
				}
			}
		}
	}
	else
	{
		while(shifty < 0)
		{
			if(shifty % 8 == 0)
			{
				scamp7_kernel_begin();
					bus(XS,F);
					bus(F,XN);
					bus(XS,F);
					bus(F,XN);
					bus(XS,F);
					bus(F,XN);
					bus(XS,F);
					bus(F,XN);
				scamp7_kernel_end();
				shifty+=8;
			}
			else
			{
				if(shifty % 4 == 0)
				{
					scamp7_kernel_begin();
					bus(XS,F);
					bus(F,XN);
					bus(XS,F);
					bus(F,XN);
					scamp7_kernel_end();
					shifty+=4;
				}
				else
				{
					if(shifty % 2 == 0)
					{
						scamp7_kernel_begin();
						bus(XS,F);
						bus(F,XN);
						scamp7_kernel_end();
						shifty+=2;
					}
					else
					{
						scamp7_kernel_begin();
							bus(NEWS,F);
							bus(F,XN);
						scamp7_kernel_end();
						shifty++;
					}
				}
			}
		}
	}
}
