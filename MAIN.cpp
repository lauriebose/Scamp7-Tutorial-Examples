
#define EXAMPLE_AREG_FLAG 0
#define	EXAMPLE_DREG_BASICS 1
#define	EXAMPLE_AREG_NEWS 2
#define	EXAMPLE_SIMPLE_EDGE_DETECTION 3


#define selected_algo EXAMPLE_AREG_FLAG


#if selected_algo == -1
#elif selected_algo == EXAMPLE_AREG_FLAG
	#include "EXAMPLE_AREG_FLAG.hpp"
#elif selected_algo == EXAMPLE_AREG_NEWS
	#include "EXAMPLE_AREG_NEWS.hpp"
#elif selected_algo == EXAMPLE_SIMPLE_EDGE_DETECTION
	#include "EXAMPLE_SIMPLE_EDGE_DETECTION.hpp"
#elif selected_algo == EXAMPLE_DREG_BASICS
	#include "EXAMPLE_DREG_BASICS.hpp"
#endif

