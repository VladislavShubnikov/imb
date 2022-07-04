//
// Copyright 2022 Vlad
//
// Image compare dialog
//

#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <QtWidgets/QDialog>
#include <QtGui/QImage>

#include "ui_WidCompare.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif


class WidCompare: public QDialog { 
  Q_OBJECT
public:
  explicit WidCompare(QWidget *parent = nullptr);
  void setImages(QImage& imageA, QImage& imageB, QImage& imageDif);

private:
  void showImages();

protected slots:
  void onDialogFinished(int res);
  void onSliderBlend(int v);

private:
  Ui::m_dialogCompare       m_ui;

  QImage                    m_imageA;
  QImage                    m_imageB;
  QImage                    m_imageDif;
  float                     m_blend;
};
