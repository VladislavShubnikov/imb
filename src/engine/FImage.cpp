//
// Copyright 2022 Vlad
//

#include <cassert>
#include <thread>
#include <vector>
#include <list>

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

FImage FImage::getGaussSmooth(FImage &imageKernel, float *timeMsec) const {
  // init result
  FImage imageDst(*this);
  float *matDst = imageDst.getBits();
  const float *matGauss = imageKernel.getBits();
  const float *matImage = getBits();

  const int wGauss = imageKernel.width();
  const int hGauss = imageKernel.height();
  assert(wGauss == hGauss);

  const int xRad = wGauss / 2;
  const int yRad = hGauss / 2;

  // timing
  std::chrono::high_resolution_clock::time_point timeS, timeE;
  timeS = std::chrono::high_resolution_clock::now();

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

  // timing
  timeE = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> timeSpan = timeE - timeS;
  auto difMs = (float)timeSpan.count();
  if (timeMsec != nullptr) 
    *timeMsec = difMs;

  return imageDst;
}

static void gaussProcessRows( const float *matSrc,
                  const float *matKernel,
                  float *matDst, 
                  const int wSrc, const int hSrc,
                  const int wKernel,
                  const int yStart, const int yEnd) {
  const int xRad = wKernel / 2;
  int k = yStart * wSrc;  // dest index
  for (int y = yStart; y < yEnd; y++) {
    for (int x = 0; x < wSrc; x++) {
      float sum = 0.0F, sumWeights = 0.0F;

      int m = 0;  // index in kernel
      for (int dy = -xRad; dy <= xRad; dy++) {
        int yy = y + dy;
        if ((yy < 0) || (yy >= hSrc)) {
          m += wKernel;
          continue;
        }
        const int yyOff = yy * wSrc;
        for (int dx = -xRad; dx <= xRad; dx++) {
          int xx = x + dx;
          if ((xx < 0) || (xx >= wSrc)) {
            m++;
            continue;
          }
          float vImage = matSrc[xx + yyOff];
          float we = matKernel[m];
          m++;
          sum += vImage * we;
          sumWeights += we;

        }  // for dx
      }    // for dy
      // normalize sum
      sum = sum / sumWeights;
      matDst[k++] = sum;
    }  // for x

  } // for y, all rows
} // end process rows

FImage FImage::getGaussSmoothViaThreads(FImage &imageKernel,
                                        float *timeMsec) const {
  // init result image
  FImage imageDst(*this);

  // timing
  std::chrono::high_resolution_clock::time_point timeS, timeE;
  timeS = std::chrono::high_resolution_clock::now();


  const int numCpuCores = std::thread::hardware_concurrency();
  const int numPrc = (numCpuCores - 2 >= 1) ? numCpuCores - 2: 1;
  // divide height by numPrc and 
  // use parallel threads to speed up
  if (numPrc >= 2) {
    const int numParts = numPrc;
    float *matSrc = m_bits;
    float *matDst = imageDst.getBits();
    float *matKernel = imageKernel.getBits();
    const int wSrc = m_wImage;
    const int hSrc = m_hImage;
    const int wKernel = imageKernel.width();

    std::vector<std::thread> vecThreads;

    int yStart = 0;
    for (int i = 0; i < numParts; i++) {
      // calculate end of image part (low row)
      const int yEnd = (i + 1) * m_hImage / numParts;
      std::thread th(gaussProcessRows, matSrc, matKernel, matDst, wSrc, hSrc,
                     wKernel, yStart, yEnd);
      vecThreads.push_back(std::move(th));
      //  next part of image
      yStart = yEnd;
    }
    // run threads and process all image parts simultaneously
    for (auto &t: vecThreads) {
      t.join();
    }
  } else {
    // in case of single - thread PC, use default, slow function
    return this->getGaussSmooth(imageKernel, timeMsec);
  }

  // timing
  timeE = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> timeSpan = timeE - timeS;
  auto difMs = (float)timeSpan.count();
  if (timeMsec != nullptr)
    *timeMsec= difMs;

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
