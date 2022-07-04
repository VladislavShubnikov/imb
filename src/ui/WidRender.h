//
// Copyright 2022 Vlad
//
#ifndef _WID_RENDER_H__
#define _WID_RENDER_H__
#pragma once

#include <QtWidgets/QWidget>
#include "RecogRes.h"

class WidImageBinarizer;


class WidRender: public QWidget {
  Q_OBJECT

public:
  explicit WidRender(QWidget* parent = nullptr);

  void setRecognitionResult(RecognitionResult* recogRes);
  void setMainWindow(WidImageBinarizer *widMain);

  void setRenderBinarizedBackgroundImage(bool isVis);
  void setRenderRecognizedText(bool isVis);
  void setRenderRectangles(bool isVis);
  
  virtual ~WidRender();

protected:
  void mousePressEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
  void paintEvent(QPaintEvent* evt) Q_DECL_OVERRIDE;

private:
  void renderBoxes(QPainter *painter, std::vector<TextBox>& boxes);
  void mouseMovePressed(QMouseEvent* evt);
  void mouseMoveUnpressed(QMouseEvent* evt);
  void drawLasso(QPainter* painter);
  void detectRectsByLasso();

// private data
  float                   m_scaleRender;
  RecognitionResult*      m_recognitionResult;
  WidImageBinarizer*      m_widMain;

  // mouse press or not
  bool                    m_mousePressed;
  // current box
  int                     m_indexBoxCurrent;

  // render mode
  bool                    m_renderBinarizedBackgroundImage;
  bool                    m_renderRecognizedText;
  bool                    m_renderRectangles;

  // lasso, drawed by mouse
  QVector<QPoint>         m_lasso;
};

#endif
