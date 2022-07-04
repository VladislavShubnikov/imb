//
// Copyright 2022 Vlad
//

#include <cassert>

#include "Bmp.h"


Bmp::Bmp() {
  m_buffer = nullptr;
  m_sizeInMem = 0;
}

Bmp::~Bmp() {
  if (m_buffer != nullptr) {
    delete[] m_buffer;
    m_buffer = nullptr;
  }
}

uint32_t Bmp::sizeInBytes() const { 
  return m_sizeInMem;
}

const int BPP = 8;


int Bmp::initFromQImage(QImage& img) {
  const unsigned char *pixelsSrc = img.bits();
  const int w = img.width();
  const int h = img.height();
  const QImage::Format fmt = img.format();
  const int wStride = (w + 3) & (~3);

  if (fmt == QImage::Format::Format_RGB32) {
    m_sizeInMem = sizeof(BmpHeader) + sizeof(BmpInfoHeader) + ((uint64_t)wStride * h * (BPP / 8));
    m_buffer = new uint8_t[m_sizeInMem];

    auto* head = (BmpHeader*)m_buffer;
    auto* info = (BmpInfoHeader *)(m_buffer + sizeof(BmpHeader));
    uint8_t *pixelsDst = m_buffer + sizeof(BmpHeader) + sizeof(BmpInfoHeader);

    head->_bfType = BMP_MAGIC_BF_TYPE;
    head->_bfSize = sizeof(BmpHeader);
    head->_bfReserved1 = 0;
    head->_bfReserved2 = 0;
    head->_bfOffBits = sizeof(BmpHeader) + sizeof(BmpInfoHeader);

    info->_nSize = sizeof(BmpInfoHeader);
    info->_nBitCount = BPP;
    info->_nClrImportant = 0;
    info->_nClrUsed = 0;
    info->_nCompression = 0;
    info->_nWidth = w;
    info->_nHeight = h;
    info->_nPlanes = 1;
    info->_nXPelsPerMeter = 0;
    info->_nYPelsPerMeter = 0;
    info->_nSizeImage = wStride * h * (BPP / 8);

    // copy bits with vertical image flip
    for (int y = 0; y < h; y++) {
      const unsigned char *pixSrc = pixelsSrc + ((int64_t)h - 1 - y) * w * 4;
      unsigned char *pixDst = pixelsDst + (uint64_t)y * wStride;
      for (int x = 0; x < w; x++) {
        auto b = (uint32_t)*pixSrc++;
        auto g = (uint32_t)*pixSrc++;
        auto r = (uint32_t)*pixSrc++;
        pixSrc++;
        uint32_t colGrey = (r + g + b) / 3;

        *pixDst++ = (uint8_t)colGrey;
      }  // for x
    } // for y

  }

  return 0;
}

QImage BmpPixToQImage(PIX *pixSrc) {
  const int w = pixGetWidth(pixSrc);
  const int h = pixGetHeight(pixSrc);
  const int d = pixGetDepth(pixSrc);
  assert(d == 1);  // should 1 bit per pixel
  const uint32_t *pixelsSrc = (uint32_t *)pixGetData(pixSrc);

  const int numDwordsPerLine = w / 32;

  QImage image(w, h, QImage::Format::Format_RGB32);
  uint8_t *pixelsDst = image.bits();

  // copy pixels
  int indSrc, indDst = 0;
  uint32_t dwordCur;
  int bitCur;

  for (int y = 0; y < h; y++) {
    const uint32_t *srcLine = pixelsSrc + ((uint64_t)y * numDwordsPerLine);
    indSrc = 0;
    dwordCur = srcLine[indSrc];
    bitCur = 31;

    for (int x = 0; x < w; x++) {
      const uint32_t bit = (dwordCur >> bitCur) & 1;
      const uint8_t val = (bit == 0) ? 255: 0;
      bitCur--;
      if (bitCur < 0) {
        bitCur = 31;
        indSrc++;
        dwordCur = srcLine[indSrc];
      }

      pixelsDst[indDst++] = val;
      pixelsDst[indDst++] = val;
      pixelsDst[indDst++] = val;
      pixelsDst[indDst++] = 255;
    }  // for x
  }    // for y

  return image;
}
QImage BmpFzPixToQImage(fz_pixmap *pixSrc) {
  const int w = pixSrc->w;
  const int h = pixSrc->h;
  const int stride = pixSrc->stride;
  assert(w * 3 == stride);
  uint8_t *pixelsSrc = pixSrc->samples;

  QImage image(w, h, QImage::Format::Format_RGB32);
  uint8_t *pixelsDst = image.bits();

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      *pixelsDst++ = *pixelsSrc++;
      *pixelsDst++ = *pixelsSrc++;
      *pixelsDst++ = *pixelsSrc++;
      *pixelsDst++ = 255;
      
    } // for x
  } /// for y


  return image;
}




