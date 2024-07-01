#define EX_KERNEL_COST -1
#define EX_AREG_DECAY -2

#define EX_IMAGE_CAPTURE_AND_AREG 1
#define	EX_DREG_BASICS 2
#define EX_FLAG 3
#define EX_IMAGE_THRESHOLDING 4
#define	EX_AREG_NEWS 5
#define	EX_SIMPLE_EDGE_DETECTION 6
#define	EX_DNEWS 7
#define EX8_DREG_SHIFTING 8
#define EX8_DREG_EXPAND_AND_ERODE 9

#define selected_algo 9


#if selected_algo == 999
#elif selected_algo == EX_IMAGE_CAPTURE_AND_AREG
	#include "EX1_IMAGE_CAPTURE_AND_AREG.hpp"
#elif selected_algo == EX_DREG_BASICS
	#include "EX2_DREG_BASICS.hpp"
#elif selected_algo == EX_FLAG
	#include "EX3_FLAG.hpp"
#elif selected_algo == EX_IMAGE_THRESHOLDING
	#include "EX4_IMAGE_THRESHOLDING.hpp"
#elif selected_algo == EX_AREG_NEWS
	#include "EX5_AREG_NEWS.hpp"
#elif selected_algo == EX_SIMPLE_EDGE_DETECTION
	#include "EX6_SIMPLE_EDGE_DETECTION.hpp"
#elif selected_algo == EX_DNEWS
	#include "EX7_DNEWS.hpp"
#elif selected_algo == EX8_DREG_SHIFTING
	#include "EX8_DREG_SHIFTING.hpp"
#elif selected_algo == EX8_DREG_EXPAND_AND_ERODE
	#include "EX9_DREG_EXPAND_AND_ERODE.hpp"


#elif selected_algo == EX_AREG_DECAY
	#include "EX_AREG_DECAY.hpp"
#elif selected_algo == EX_KERNEL_COST
	#include "EX_KERNEL_COST.hpp"
#endif

