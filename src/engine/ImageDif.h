//
// Copyright 2022 Vlad
//

#ifndef _IMAGE_DIFF_H__
#define _IMAGE_DIFF_H__

#include "FImage.h"

class ImageDiff {
 public:
  static void getDiff(const FImage& imageA, const FImage& imageB,
                      float distBarrier, FImage& imageDiff);
};

#endif