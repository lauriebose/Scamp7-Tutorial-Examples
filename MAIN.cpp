
#define EX1_IMAGE_CAPTURE_AND_AREG 1
#define	EX2_DREG_BASICS 2
#define EX3_FLAG 3
#define	EXAMPLE_AREG_NEWS 4
#define	EXAMPLE_SIMPLE_EDGE_DETECTION 5


#define selected_algo EX3_FLAG


#if selected_algo == -1
#elif selected_algo == EX1_IMAGE_CAPTURE_AND_AREG
	#include "EX1_IMAGE_CAPTURE_AND_AREG.hpp"
#elif selected_algo == EX2_DREG_BASICS
	#include "EX2_DREG_BASICS.hpp"
#elif selected_algo == EX3_FLAG
	#include "EX3_FLAG.hpp"
#elif selected_algo == EXAMPLE_AREG_NEWS
	#include "EXAMPLE_AREG_NEWS.hpp"
#elif selected_algo == EXAMPLE_SIMPLE_EDGE_DETECTION
	#include "EXAMPLE_SIMPLE_EDGE_DETECTION.hpp"
#endif

