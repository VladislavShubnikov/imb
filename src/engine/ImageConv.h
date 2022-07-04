//
// Copyright 2022 Vlad
//

#ifndef _IMAGE_CONV_H__
#define _IMAGE_CONV_H__

#include "FImage.h"

class ImageConvolutions 
{
public:
  static void getWindowedMean(FImage& imageSrc, FImage& imageDst, int wSize);
  static void getWindowedStdDev(const FImage& imageSrc,
                               const FImage& imageMean,
                               FImage& imageDst, 
                               int wSize);
  static void getSauvolaThreshold(const FImage& imageFloatMean,
                                  const FImage& imageFloatStdDev,
                                  float factor, FImage& imageFloatThresholds);
  static void applyThresholds(const FImage& imageFloatSrc, 
                              const FImage& imageFloatThresholds,
                              FImage& imageFloatDest);
};

#endif