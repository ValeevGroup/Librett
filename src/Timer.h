/******************************************************************************
MIT License

Copyright (c) 2016 Antti-Pekka Hynninen
Copyright (c) 2016 Oak Ridge National Laboratory (UT-Batelle)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#ifndef LIBRETTTIMER_H
#define LIBRETTTIMER_H

#include <vector>
#include <chrono>
#include <cstdlib>
#include <unordered_map>
#include <set>
// -------------------------------------------------
// By default uses GPU event timer. Comment out
// this line if you want to use the wallclock 
#define GPU_EVENT_TIMER
// -------------------------------------------------
#ifdef GPU_EVENT_TIMER
  #if SYCL
    #include <sycl/sycl.hpp>
  #elif HIP
    #include <hip/hip_runtime.h>
  #else
    #include <cuda_runtime.h>
  #endif
#endif

//
// Simple raw timer
//
class Timer {
private:
#ifdef GPU_EVENT_TIMER
  #if SYCL
    sycl::event tmstart, tmend;
    std::chrono::time_point<std::chrono::steady_clock> tmstart_ct1;
    std::chrono::time_point<std::chrono::steady_clock> tmend_ct1;
  #elif HIP
    hipEvent_t tmstart, tmend;
  #else // CUDA
    cudaEvent_t tmstart, tmend;
  #endif
#else
  std::chrono::high_resolution_clock::time_point tmstart, tmend;
#endif
public:
#ifdef GPU_EVENT_TIMER
  Timer();
  ~Timer();
#endif
  void start();
  void stop();
  double seconds();
};

//
// Records timings for LIBRETT and gives out bandwidths and other data
//
class librettTimer {
private:
  // Size of the type we're measuring
  const int sizeofType;

  // Dimension and permutation of the current run
  std::vector<int> curDim;
  std::vector<int> curPermutation;

  // Bytes transposed in the current run
  size_t curBytes;

  // Timer for current run
  Timer timer;

  struct Stat {
    double totBW;
    double minBW;
    double maxBW;
    std::vector<double> BW;
    std::vector<int> worstDim;
    std::vector<int> worstPermutation;
    Stat() {
      totBW = 0.0;
      minBW = 1.0e20;
      maxBW = -1.0;
    }
  };

  // List of ranks that have been recorded
  std::set<int> ranks;

  // Statistics for every rank
  std::unordered_map<int, Stat> stats;

public:
  librettTimer(int sizeofType);
  ~librettTimer();
  void start(std::vector<int>& dim, std::vector<int>& permutation);
  void stop();
  double seconds();
  double GBs();
  double GiBs();
  double getBest(int rank);
  double getWorst(int rank);
  double getWorst(int rank, std::vector<int>& dim, std::vector<int>& permutation);
  double getMedian(int rank);
  double getAverage(int rank);
  std::vector<double> getData(int rank);

  double getWorst(std::vector<int>& dim, std::vector<int>& permutation);

  std::set<int>::const_iterator ranksBegin() {
    return ranks.begin();
  }

  std::set<int>::const_iterator ranksEnd() {
    return ranks.end();
  }
};

#endif // LIBRETTTIMER_H
