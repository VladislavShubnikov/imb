//
// Copyright 2022 Vlad
//

#include <cassert>

#include "ImageConv.h"
#include "FImage.h"

void ImageConvolutions::getWindowedMean(FImage &imageSrc, FImage &imageDst,
                                        int wSize) {
  float *floatSrc = imageSrc.getBits();
  float *floatDst = imageDst.getBits();

  const int wSrc = imageSrc.width();
  const int hSrc = imageSrc.height();

  assert((wSize & 1) == 1);
  const int numNeighbours = wSize * wSize;
  const int rad = wSize / 2;

  int k = 0;  // dest offset
  for (int y = 0; y < hSrc; y++) {
    for (int x = 0; x < wSrc; x++) {
      float sum = 0.0;
      int numNeibs = 0;

      for (int dy = -rad; dy <= rad; dy++) {
        int yy = y + dy;
        if ( (yy < 0) || (yy >= hSrc))
          continue;
        const int yyOff = yy * wSrc;

        for (int dx = -rad; dx <= rad; dx++) {
          int xx = x + dx;
          if ((xx < 0) || (xx >= wSrc)) 
            continue;

          sum += floatSrc[xx + yyOff];
          numNeibs++;
        }  // for dx
      }    // for dy
      floatDst[k++] = sum / numNeibs;
    }  // for x
  }    // for y
}
void ImageConvolutions::getWindowedStdDev(const FImage &imageSrc,
                                          const FImage &imageMean,
                                          FImage &imageDst, int wSize) {
  float *floatSrc = imageSrc.getBits();
  float *floatMean = imageMean.getBits();
  float *floatDst = imageDst.getBits();

  const int wSrc = imageSrc.width();
  const int hSrc = imageSrc.height();

  float valMin = 100000.0F;
  float valMax = 0.0F;

  assert((wSize & 1) == 1);
  const int rad = wSize / 2;

  int k = 0;  // dest offset
  for (int y = 0; y < hSrc; y++) {
    for (int x = 0; x < wSrc; x++) {
      const float valMean = floatMean[k];
      float sum = 0.0;
      int numNeibs = 0;

      for (int dy = -rad; dy <= rad; dy++) {
        int yy = y + dy;
        if ((yy < 0) || (yy >= hSrc)) 
          continue;
        const int yyOff = yy * wSrc;
        for (int dx = -rad; dx <= rad; dx++) {
          int xx = x + dx;
          if ((xx < 0) || (xx >= wSrc)) 
            continue;

          const float dv = floatSrc[xx + yyOff] - valMean;
          sum += dv * dv;
          numNeibs++;
        }  // for dx
      }    // for dy
      const float s = sqrt(sum / numNeibs);
      floatDst[k++] = s;
      valMin = (s < valMin) ? s : valMin;
      valMax = (s > valMax) ? s : valMax;
    }  // for x
  }    // for y
  valMax += 1.0F;

  const bool NORMALIZE_STDDEV = false;
  // normalize
  if (NORMALIZE_STDDEV) {
    const int numPixels = wSrc * hSrc;
    for (int i = 0; i < numPixels; i++) {
      floatDst[i] = 256.0F * floatDst[i] / valMax;
    }
  }
}

void ImageConvolutions::getSauvolaThreshold(const FImage &imageFloatMean,
                                            const FImage &imageFloatStdDev,
                                            float factor,
                                            FImage &imageFloatThresholds) {
  float *floatMean = imageFloatMean.getBits();
  float *floatStd = imageFloatStdDev.getBits();
  float *floatDst = imageFloatThresholds.getBits();

  const int wSrc = imageFloatMean.width();
  const int hSrc = imageFloatMean.height();
  const int numPixels = wSrc * hSrc;
  for (int i = 0; i < numPixels; i++) {
    // According to
    // https://craftofcoding.wordpress.com/2021/10/06/thresholding-algorithms-sauvola-local/
    // threshold value is calculated as:
    // t = M * (1 + k * ((S/128) - 1))
    // t: result threshold
    // M: media value
    // S: standard deviation value
    // k: factor in [0.2 .. 0.5]
    const float t =
        floatMean[i] * (1.0 + factor * ((floatStd[i] / 128.0) - 1.0));
    floatDst[i] = t;
  }  // for i
}

void ImageConvolutions::applyThresholds(const FImage &imageFloatSrc,
                                        const FImage &imageFloatThresholds,
                                        FImage &imageFloatDest) {
  float *floatSrc = imageFloatSrc.getBits();
  float *floatThr = imageFloatThresholds.getBits();
  float *floatDst = imageFloatDest.getBits();

  const int wSrc = imageFloatSrc.width();
  const int hSrc = imageFloatSrc.height();
  const int numPixels = wSrc * hSrc;
  for (int i = 0; i < numPixels; i++) {
    const float t = (floatSrc[i] < floatThr[i]) ? 0.0F : 255.0F;
    floatDst[i] = t;
  }  // for i
}
