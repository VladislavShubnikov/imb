//
// Copyright 2022 Vlad
//

#ifndef _FAST_MEAN_H__
#define _FAST_MEAN_H__

#include "FImage.h"

class FastMeanStd {
 public:
  static void getFastMeanStd(const FImage& imageSrc, FImage& imageMean,
                             FImage& imageStd,  int neibSize);
};

#endif