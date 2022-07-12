//
// Copyright 2022 Vlad
//

#include "ImageDif.h"

#define USE_THREADS

void ImageDiff ::getDiff(const FImage &imageA, const FImage &imageB,
                         float distBarrier, FImage &imageDiff) {

  FImage imageKernel = getGaussianKernel(7, 7, 0.4F);
  #ifdef USE_THREADS
    FImage smoothA = imageA.getGaussSmoothViaThreads(imageKernel);
    FImage smoothB = imageB.getGaussSmoothViaThreads(imageKernel);
  #else
    FImage smoothA = imageA.getGaussSmooth(imageKernel);
    FImage smoothB = imageB.getGaussSmooth(imageKernel);
  #endif

  #ifdef DEEP_DEBUG
    QImage imgA = smoothA.getQImage();
    QImage imgB = smoothB.getQImage();
    imgA.save("log/smooth_a.png");
    imgB.save("log/smooth_b.png");
  #endif


  const float *floatA = smoothA.getBits();
  const float *floatB = smoothB.getBits();
  float *floatDiff = imageDiff.getBits();

  const int wSrc = imageA.width();
  const int hSrc = imageA.height();

  const float distBarrier2 = (float)distBarrier * distBarrier;

  int k = 0;  // dest offset
  for (int y = 0; y < hSrc; y++) {
    for (int x = 0; x < wSrc; x++) {

      float dif = floatA[k] - floatB[k];
      if (dif * dif <= distBarrier2) {
        floatDiff[k] = 0.0F;
      } else {
        floatDiff[k] = 255.0F;
      }
      k++;
    }  // for x
  }    // for y
}
