//
// Copyright 2022 Vlad
//

#ifndef _BMP_H__
#define _BMP_H__
#pragma once


// **********************************************
// includes
// **********************************************

#include <QtGui/QImage>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4611)
#endif

#include "leptonica/allheaders.h"
#include "leptonica/pix.h"
#include "mupdf/fitz/image.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// **********************************************
// defines
// **********************************************

#define BMP_MAGIC_BF_TYPE       19778


// **********************************************
// types
// **********************************************

#pragma pack(push, 2)

struct BmpHeader {
  uint16_t            _bfType;
  uint32_t            _bfSize;
  uint16_t            _bfReserved1;
  uint16_t            _bfReserved2;
  uint32_t            _bfOffBits;
};

struct BmpInfoHeader {
  uint32_t            _nSize;
  uint32_t            _nWidth;
  uint32_t            _nHeight;
  uint16_t            _nPlanes;
  uint16_t            _nBitCount;
  uint32_t            _nCompression;
  uint32_t            _nSizeImage;
  uint32_t            _nXPelsPerMeter;
  uint32_t            _nYPelsPerMeter;
  uint32_t            _nClrUsed;
  uint32_t            _nClrImportant;
};

#pragma pack(pop)


class Bmp {
public:
  Bmp();
  ~Bmp();

  int initFromQImage(QImage& img);
  uint8_t *bits() {
    return m_buffer;
  }
  uint32_t sizeInBytes() const;

 private:
  uint8_t        *m_buffer;
  uint32_t        m_sizeInMem;
};

QImage BmpPixToQImage(PIX *pixSrc);
QImage BmpFzPixToQImage(fz_pixmap *pixSrc);

#endif 
