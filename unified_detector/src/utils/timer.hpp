/**
 * @file   timer.hpp
 * @brief  This file implements the timer utility apis
 *
 * This file implements timer utility apis for measuring the time
 */
#ifndef __TIMER_UTILITY__
#define __TIMER_UTILITY__

#include <chrono>

#define TIME_IN_MILLISECONDS(x) (std::chrono::duration_cast<std::chrono::milliseconds>(x).count())
#define TIME_IN_MICROSECONDS(x) (std::chrono::duration_cast<std::chrono::microseconds>(x).count())
#define CURRENT_TIME_POINT std::chrono::steady_clock::now()
#define CURRENT_TIME_SINCE_EPOCH CURRENT_TIME_POINT.time_since_epoch()
// #define CURRENT_TIME ((float)TIME_IN_MICROSECONDS(CURRENT_TIME_SINCE_EPOCH) * 0.001f)
#define CURRENT_TIME TIME_IN_MILLISECONDS(CURRENT_TIME_SINCE_EPOCH)

#endif