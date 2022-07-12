//
// Copyright 2022 Vlad
//

#ifndef _FIMAGE_H__
#define _FIMAGE_H__

#include <QtGui/QImage>

class FImage
{
public:
  FImage();
  explicit FImage(int w, int h);
  explicit FImage(QImage& imageSrc);

  // copy constructor
  FImage(const FImage& imageSrc);
  // move constructor
  FImage(FImage&& imageSrc) noexcept;

  ~FImage();

  int width() const  {
    return m_wImage;
  }
  int height() const { 
    return m_hImage; 
  }
  float* getBits() const {
    return m_bits;
  }

  QImage getQImage();

  // interface to image processing - naive
  FImage getWindowedMean(int winSize);
  FImage getWindowedStdDev(const FImage& imageFloatMean, int winSize);
  FImage getSauvolaThreshold(const FImage& imageFloatStdDev, float factor);
  FImage applyThresholds(const FImage& imageFloatThresholds);

  // interface to optimized stddev image calculation
  FImage getIntegralImage() const;
  float  getSum(int xMin, int yMin, int xMax, int yMax) const;
  FImage getIntegralImage2() const;
  float  getSum2(int xMin, int yMin, int xMax, int yMax) const;

  // gauss smooth
  FImage getGaussSmooth(FImage& imageKernel, float* timeMsec = nullptr) const;
  FImage getGaussSmoothViaThreads(FImage& imageKernel, float *timeMsec = nullptr) const;


private:
  int       m_wImage;   // width
  int       m_hImage;   // height
  float*    m_bits;
};

FImage    getGaussianKernel(int w, int h, float sigma);


#endif