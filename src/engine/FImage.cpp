//
// Copyright 2022 Vlad
//

#include <cassert>

#include <QtCore/QDebug>

#include "FImage.h"
#include "ImageConv.h"


FImage::FImage() {
  m_wImage = 0;
  m_hImage = 0;
  m_bits = nullptr;
}

FImage::FImage(int w, int h) {
  m_wImage = w;
  m_hImage = h;
  m_bits = new float[(int64_t)w * h];
}


FImage::FImage(QImage& imageSrc) {
  m_wImage = imageSrc.width();
  m_hImage = imageSrc.height();
  assert(m_wImage > 0);
  assert(m_hImage > 0);
  const int numPixels = m_wImage * m_hImage;
  m_bits = new float[numPixels];
  const uchar *pixSrc = imageSrc.bits();
  const QImage::Format fmt = imageSrc.format();
  if ( (fmt == QImage::Format::Format_RGB32) ||
       (fmt == QImage::Format::Format_ARGB32)) {
    for (int i = 0; i < numPixels; i++) {
      auto r = (ulong)pixSrc[0];
      pixSrc++;
      auto g = (ulong)pixSrc[0];
      pixSrc++;
      auto b = (ulong)pixSrc[0];
      pixSrc++;
      m_bits[i] = (r + g + b) / 3.0F;
      pixSrc++; // skip a
    } // for i , all pixels
  } else { // if format is argb32
    // unsupported format
    assert(fmt == QImage::Format::Format_RGB32);
  }
}

FImage::FImage(const FImage& imageSrc) {
  // qInfo() << "FImage copy constructor is called: deep copy";
  m_wImage = imageSrc.m_wImage;
  m_hImage = imageSrc.m_hImage;
  m_bits = new float[(uint64_t)m_wImage * m_hImage];
  memcpy(m_bits, imageSrc.m_bits,
         (uint64_t)m_wImage * m_hImage * sizeof(float));
}

FImage::FImage(FImage&& imageSrc) noexcept {
  // qInfo() << "FImage move constructor is called";
  m_wImage = imageSrc.m_wImage;
  m_hImage = imageSrc.m_hImage;
  m_bits = imageSrc.m_bits;
  imageSrc.m_bits = nullptr;
}

QImage FImage::getQImage()
{
  QImage img(m_wImage, m_hImage, QImage::Format::Format_ARGB32);
  uchar *pixDst = img.bits();
  const int numPixels = m_wImage * m_hImage;
  for (int i = 0; i < numPixels; i++) {
    const auto val = (uchar)m_bits[i];
    pixDst[0] = val;
    pixDst[1] = val;
    pixDst[2] = val;
    pixDst[3] = 255;
    pixDst += 4;
  }
  return img;
}


FImage ::~FImage() {
  if (m_bits)
    delete [] m_bits;
  m_bits = nullptr;
  m_wImage = m_hImage = 0;
}

FImage FImage::getWindowedMean(int winSize) {
  FImage imageDst(*this);
  ImageConvolutions::getWindowedMean(*this, imageDst, winSize);
  return imageDst;
}

FImage FImage::getWindowedStdDev(const FImage& imageFloatMean,
                                 int winSize) {
  FImage imageDst(*this);
  ImageConvolutions::getWindowedStdDev(*this, imageFloatMean, imageDst,
                                       winSize);
  return imageDst;
}
FImage FImage::getSauvolaThreshold(const FImage& imageFloatStdDev, float factor) {
  FImage imageDst(*this);
  ImageConvolutions::getSauvolaThreshold(*this, imageFloatStdDev,
                                         factor, imageDst);
  return imageDst;
}
FImage FImage::applyThresholds(const FImage& imageFloatThresholds) {
  FImage imageDst(*this);
  ImageConvolutions::applyThresholds(*this, imageFloatThresholds,
                                     imageDst);
  return imageDst;
}


FImage getGaussianKernel(const int w, const int h, const float sigma) {
  assert(w == h);
  assert((w & 1) == 1);

  const int xc = w / 2;
  const int yc = h / 2;
  FImage image(w, h);
  float *mat = image.getBits();

  float ks = 1.0F / (2 * sigma * sigma);

  float sum = 0.0F;
  for (int y = 0; y < h; y++) {
    float dy = (float)(y - yc) / yc;

    for (int x = 0; x < w; x++) {
      float dx = (float)(x - xc) / xc;
      float we = expf( -(dx * dx + dy * dy) * ks );
      mat[x + y * w] = we;
      sum += we;
    } // for x
  } // for y
  // normalize
  float scale = 1.0F / sum;
  int numPixels = w * h;
  for (int i = 0; i < numPixels; i++) {
    mat[i] = mat[i] * scale;
  }
  return image;
}

FImage FImage::getGaussSmooth(FImage &imageKernel) const {
  FImage imageDst(*this);
  float *matDst = imageDst.getBits();
  const float *matGauss = imageKernel.getBits();
  const float *matImage = getBits();

  const int wGauss = imageKernel.width();
  const int hGauss = imageKernel.height();
  assert(wGauss == hGauss);

  const int xRad = wGauss / 2;
  const int yRad = hGauss / 2;

  int k = 0; // dest index
  for (int y = 0; y < m_hImage; y++) {
    for (int x = 0; x < m_wImage; x++) {
      float sum = 0.0F, sumWeights = 0.0F;

      int m = 0; // index in kernel
      for (int dy = -yRad; dy <= yRad; dy++) {
        int yy = y + dy;
        if ((yy < 0) || (yy >= m_hImage)) {
          m += wGauss;
          continue;
        }
        for (int dx = -xRad; dx <= xRad; dx++) {
          int xx = x + dx;
          if ((xx < 0) || (xx >= m_wImage)) {
            m++;
            continue;
          }
          float vImage = matImage[xx + yy * m_wImage];
          float we = matGauss[m];
          m++;
          sum += vImage * we;
          sumWeights += we;
          
        } // for dx
      } // for dy
      
      // normalize sum
      sum = sum / sumWeights;
      matDst[k++] = sum;
    } // for x
  } // for y

  return imageDst;
}


FImage FImage::getIntegralImage() const { 
  FImage imageDst(*this);

  const float *pixelsSrc = this->getBits();
  float *pixelsDst = imageDst.getBits();
  float sum;
  int x, y;

  sum = 0.0F;
  for (x = 0; x < m_wImage; x++) {
    sum += pixelsSrc[x];
    pixelsDst[x] = sum;
  }
  sum = 0.0F;
  int yOff = 0;
  for (y = 0; y < m_hImage; y++, yOff += m_wImage) {
    sum += pixelsSrc[yOff];
    pixelsDst[yOff] = sum;
  }
  yOff = m_wImage;
  for (y = 1; y < m_hImage; y++, yOff += m_wImage) {
    for (x = 1; x < m_wImage; x++) {
      const int off = yOff + x;
      sum = pixelsSrc[off] + pixelsDst[off - 1] + pixelsDst[off - m_wImage] -
            pixelsDst[off - m_wImage - 1];
      pixelsDst[off] = sum;
    }
  }
  return imageDst;
}
float FImage::getSum(int xMin, int yMin, int xMax,
                     int yMax) const {
  const float *pixels = this->getBits();
  float sum;

  // zero left top corner
  if ((xMin == 0) && (yMin == 0)) {
    sum = pixels[yMax * m_wImage + xMax];
  }  else if ((yMin == 0) && (xMin > 0)) {
    // right rect
    sum = pixels[yMax * m_wImage + xMax] - pixels[yMax * m_wImage + (xMin - 1)];
  } else if ((xMin == 0) && (yMin > 0)) {
    // bot rect
    sum = pixels[yMax * m_wImage + xMax] - pixels[(yMin - 1) * m_wImage + xMax];
  } else {
    sum = pixels[yMax * m_wImage + xMax] - pixels[yMax * m_wImage + (xMin - 1)] -
          pixels[(yMin - 1) * m_wImage + xMax] + pixels[(yMin - 1) * m_wImage + (xMin -1 )];
  }
  return sum;
}

FImage FImage::getIntegralImage2() const {
  FImage imageDst(*this);

  const float *pixelsSrc = this->getBits();
  float *pixelsDst = imageDst.getBits();
  float sum;
  int x, y;

  sum = 0.0F;
  for (x = 0; x < m_wImage; x++) {
    sum += (pixelsSrc[x] * pixelsSrc[x]);
    pixelsDst[x] = sum;
  }
  sum = 0.0F;
  int yOff = 0;
  for (y = 0; y < m_hImage; y++, yOff += m_wImage) {
    sum += (pixelsSrc[yOff] * pixelsSrc[yOff]);
    pixelsDst[yOff] = sum;
  }
  yOff = m_wImage;
  for (y = 1; y < m_hImage; y++, yOff += m_wImage) {
    for (x = 1; x < m_wImage; x++) {
      const int off = yOff + x;
      sum = (pixelsSrc[off] * pixelsSrc[off]) + pixelsDst[off - 1] +
            pixelsDst[off - m_wImage] -
            pixelsDst[off - m_wImage - 1];
      pixelsDst[off] = sum;
    }
  }
  return imageDst;
}
float FImage::getSum2(int xMin, int yMin, int xMax,
                     int yMax) const {
  const float *pixels = this->getBits();

  float sum;
  if ((xMin == 0) && (yMin == 0)) {
    sum = pixels[yMax * m_wImage + xMax];
  } else if ((yMin == 0) && (xMin > 0)) {
    // right rect
    sum = pixels[yMax * m_wImage + xMax] - pixels[yMax * m_wImage + (xMin - 1)];
  } else if ((xMin == 0) && (yMin > 0)) {
    // bot rect
    sum = pixels[yMax * m_wImage + xMax] - pixels[(yMin - 1) * m_wImage + xMax];
  } else {
    sum = pixels[yMax * m_wImage + xMax] -
          pixels[yMax * m_wImage + (xMin - 1)] -
          pixels[(yMin - 1) * m_wImage + xMax] +
          pixels[(yMin - 1) * m_wImage + (xMin - 1)];
  }
  return sum;
}
