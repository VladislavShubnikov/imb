//
// Copyright 2022 Vlad
//

#ifndef _RECOG_RES_H_
#define _RECOG_RES_H_

#pragma once

#include <QtGui/QImage>

class TextBox {
public:
  // rectangle around text
  QRect     m_rect;
  // recognized text
  QString   m_text;
  // is selected
  bool      m_selected;

  TextBox();
};

const int MAGIC_REC = 0x1237;

class RecognitionResult {
public:
  // magix sign
  int                   m_magic;         
  // binarized image
  QImage                m_image;
  // set of recognized text boxes on the binarized image
  std::vector<TextBox>  m_textBoxes;

  RecognitionResult();
  int getBoxIndexInside(int x, int y);
  bool markSelectedByPoly(QVector<QPoint>& points, float scaleRender);
  void removeSelected();
};
#endif
