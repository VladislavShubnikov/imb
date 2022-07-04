//
// Copyright 2022 Vlad
//
// Image compare dialog
//

#include "WidCompare.h"

WidCompare::WidCompare(QWidget *parent) : QDialog(parent) {
  m_ui.setupUi(this);

  m_blend = 0.0F;

  connect(this, SIGNAL(finished(int)), this, SLOT(onDialogFinished(int)));
  connect(m_ui.m_sliderBlend, SIGNAL(valueChanged(int)), this, SLOT(onSliderBlend(int)));
}

void WidCompare::onDialogFinished(int res) {
  res;
  // accept();
}

void WidCompare::onSliderBlend(int v) {
   m_blend = v / 100.0F;
   showImages();
}

void WidCompare::setImages(QImage &imageA, QImage &imageB, QImage &imageDif) {
  m_imageA = imageA;
  m_imageB = imageB;
  m_imageDif = imageDif;
  showImages();
}

void WidCompare::showImages() {
  // blend 2 source images into one
  const int w = m_imageA.width();
  const int h = m_imageA.height();
  const int numPixels = w * h;
  const QImage::Format fmt = m_imageA.format();
  assert( (fmt == QImage::Format::Format_ARGB32) ||
      (fmt == QImage::Format::Format_RGB32) );
  QImage image = m_imageA.copy();

  const uint8_t *pixA = m_imageA.bits();
  const uint8_t *pixB = m_imageB.bits();
  const uint8_t *pixDif = m_imageDif.bits();
  uint8_t *pixDst = image.bits();
  int j = 0;
  for (int i = 0; i < numPixels; i++, j += 4) {
    auto valA = (uint32_t)pixA[j];
    auto valB = (uint32_t)pixB[j];
    uint8_t valDif = pixDif[j];

    // blend 2 images as a * (1 - r) + b * r
    auto val = (uint8_t)(valA * (1.0F - m_blend) + valB * m_blend);
    if (valDif < 5) {
      pixDst[j + 0] = val;
      pixDst[j + 1] = val;
      pixDst[j + 2] = val;
    } else {
      // make more red-colored
      uint8_t valLess = val - val / 4;
      pixDst[j + 0] = valLess;
      pixDst[j + 1] = valLess;
      pixDst[j + 2] = val;
    } //if
  }// for i , all pixels


  QPixmap pixmap = QPixmap::fromImage(image);
  m_ui.m_labelImage->setPixmap(pixmap);
}

