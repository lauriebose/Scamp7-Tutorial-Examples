#include <scamp7.hpp>

using namespace SCAMP7_PE;
void batched_shift_F(int shiftx, int shifty);
void trick_shift_F(int shiftx, int shifty);
void simple_shift_F(int shiftx, int shifty);

int main()
{
    vs_init();

    int display_size = 2;
    auto display_00 = vs_gui_add_display("00",0,0,display_size);
    auto display_01 = vs_gui_add_display("01",0,display_size,display_size);
    auto display_02 = vs_gui_add_display("02",0,display_size*2,display_size);

    int shiftx = 0;
    vs_gui_add_slider("shiftx",-64,64,shiftx,&shiftx);
    int shifty = 0;
    vs_gui_add_slider("shifty",-64,64,shifty,&shifty);

    vs_stopwatch timer;

    // Frame Loop
    while(1){
        vs_frame_loop_control();

        scamp7_kernel_begin();
        	get_image(A,E);
        	mov(F,A);
        scamp7_kernel_end();



        timer.reset();
        scamp7_shift(F,shiftx,shifty);
        int scamp7_shift_time = timer.get_usec();
        scamp7_kernel_begin();
			mov(B,F);
			mov(F,A);
		 scamp7_kernel_end();

        timer.reset();
        simple_shift_F(shiftx,shifty);
        int simple_shift_time = timer.get_usec();
        scamp7_kernel_begin();
			mov(C,F);
			mov(F,A);
		 scamp7_kernel_end();

	    timer.reset();
		trick_shift_F(shiftx,shifty);
		int trick_shift_time = timer.get_usec();
		scamp7_kernel_begin();
			mov(C,F);
			mov(F,A);
		 scamp7_kernel_end();

        timer.reset();
        batched_shift_F(shiftx,shifty);
        int my_shift_time = timer.get_usec();
        scamp7_kernel_begin();
			mov(D,F);
			mov(F,A);
		 scamp7_kernel_end();

        vs_post_text("Execution Times : scamp7_shift_time %d, simple shift %d, trick shift %d, optimised shift %d us \n", scamp7_shift_time,simple_shift_time,trick_shift_time,my_shift_time);
//		 vs_post_text("Execution Times :  %d,  %d, o %d us \n",my_shift_time, scamp7_shift_time,simple_shift_time);
		scamp7_output_image(B,display_00);
		scamp7_output_image(C,display_01);
		scamp7_output_image(D,display_02);
    }

    return 0;
}



void simple_shift_F(int shiftx, int shifty)
{
	if(shiftx > 0)
	{
		for(int n = 0 ; n < shiftx ; n++)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XW);
			scamp7_kernel_end();
		}
	}
	else
	{
		for(int n = 0 ; n < -shiftx ; n++)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XE);
			scamp7_kernel_end();
		}
	}

	if(shifty > 0)
	{
		for(int n = 0 ; n < shifty ; n++)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XS);
			scamp7_kernel_end();
		}
	}
	else
	{
		for(int n = 0 ; n < -shifty ; n++)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XN);
			scamp7_kernel_end();
		}
	}
}

void trick_shift_F(int shiftx, int shifty)
{
	if(shiftx > 0)
	{
		if(shiftx%2 != 0)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XW);
			scamp7_kernel_end();
		}
		for(int n = 0 ; n < shiftx/2 ; n++)
		{
			scamp7_kernel_begin();
				bus(XE,F);
				bus(F,XW);
			scamp7_kernel_end();
		}
	}
	else
	{
		if(shiftx%2 != 0)
		{
			scamp7_kernel_begin();
				bus(NEWS,F);
				bus(F,XE);
			scamp7_kernel_end();
		}
		for(int n = 0 ; n < -shiftx/2 ; n++)
		{
			scamp7_kernel_begin();
				bus(XW,F);
				bus(F,XE);
			scamp7_kernel_end();
		}
	}

	if(shifty > 0)
	{
		if(shifty%2 != 0)
		{
			scamp7_kernel_begin();
				bus(XN,F);
				bus(F,XS);
			scamp7_kernel_end();
		}
		for(int n = 0 ; n < shifty/2 ; n++)
		{
			scamp7_kernel_begin();
				bus(XN,F);
				bus(F,XS);
			scamp7_kernel_end();
		}
	}
	else
	{
		if(shifty%2 != 0)
		{
			scamp7_kernel_begin();
				bus(XS,F);
				bus(F,XN);
			scamp7_kernel_end();
		}
		for(int n = 0 ; n < -shifty/2 ; n++)
		{
			scamp7_kernel_begin();
				bus(XS,F);
				bus(F,XN);
			scamp7_kernel_end();
		}
	}
}

void batched_shift_F(int shiftx, int shifty)
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
