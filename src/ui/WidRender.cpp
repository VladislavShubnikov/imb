//
// Copyright 2022 Vlad
//

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QString>
#include <QTextStream>

#include "WidRender.h"
#include "WidImageBinarizer.h"

WidRender::WidRender(QWidget* parent) : QWidget(parent) {
  m_recognitionResult = nullptr;
  m_widMain = nullptr;
  m_scaleRender = 1.0f;
  m_indexBoxCurrent = -1;
  m_renderBinarizedBackgroundImage = true;
  m_renderRecognizedText = true;
  m_renderRectangles = true;

  m_mousePressed = false;


  // track mouse move
  setMouseTracking(true);
  // installEventFilter(this);
}

WidRender::~WidRender() {

}
void WidRender::setRecognitionResult(RecognitionResult* recogRes) {
  m_recognitionResult = recogRes;
}
void WidRender::setMainWindow(WidImageBinarizer* widMain) { 
  m_widMain = widMain; 
}
void WidRender::setRenderBinarizedBackgroundImage(bool isVis) {
  if (isVis != m_renderBinarizedBackgroundImage) {
    m_renderBinarizedBackgroundImage = isVis;
    repaint();
  }
}
void WidRender::setRenderRecognizedText(bool isVis) {
  if (isVis != m_renderRecognizedText) {
    m_renderRecognizedText = isVis;
    repaint();
  }
}
void WidRender::setRenderRectangles(bool isVis) {
  if (isVis != m_renderRectangles) {
    m_renderRectangles = isVis;
    repaint();
  }
}

void WidRender::renderBoxes(QPainter* painter, std::vector<TextBox>& boxes) {
  const int numBoxes = (int)boxes.size();
  for (int i = 0; i < numBoxes; i++) {
    TextBox tb = boxes[i];
    QRect r = tb.m_rect;
    const int x = (int)(r.x() * m_scaleRender);
    const int y = (int)(r.y() * m_scaleRender);
    const int w = (int)(r.width() * m_scaleRender);
    const int h = (int)(r.height() * m_scaleRender);
    QRectF rectRender(x, y, w, h);

    QColor color;
    if (i == m_indexBoxCurrent) {
      color = Qt::red;
    } else {
      if (tb.m_selected) {
        color = Qt::yellow;
      } else {
        color = Qt::green;
      }
    }
    painter->setPen(QPen(color, 2, Qt::SolidLine));


    if (i == m_indexBoxCurrent) {
      painter->setPen(QPen(Qt::red, 2, Qt::SolidLine));
    } else {
      if (tb.m_selected) {
        painter->setPen(QPen(Qt::yellow, 2, Qt::SolidLine));
      } else {
        painter->setPen(QPen(Qt::green, 2, Qt::SolidLine));
      }
    }
    if (m_renderRectangles) {
      painter->drawRect(rectRender);
    }

    if (m_renderRecognizedText) {
      painter->setPen(QPen(Qt::red, 2, Qt::SolidLine));

      // adjust font size for detected rectangle
      QFont font = painter->font();
      QFontMetrics fm(font);
      int wText = fm.horizontalAdvance(tb.m_text);
      int hText = fm.height();
      int des = fm.descent();

      float sx = (float)rectRender.width() / wText;
      float sy = (float)rectRender.height() / hText;

      painter->save();
      QTransform transf;

      transf.translate(rectRender.left(), rectRender.bottom() - ((int64_t)des * sy));
      transf.scale(sx, sy);
      painter->setTransform(transf);

      painter->drawText(0, 0, tb.m_text);
      painter->restore();
   }
  }
  painter->end();
}

void WidRender::drawLasso(QPainter* painter) {
  const int numPoints = m_lasso.size();
  painter->begin(this);
  painter->setPen(Qt::black);
  const int numToDraw = (m_mousePressed)
                            ? (numPoints - 1)
                            : (numPoints);
  for (int i = 0; i < numToDraw; i++) {
    int iNext = (i + 1 < numPoints)? (i + 1): 0;
    painter->drawLine(m_lasso[i], m_lasso[iNext]);
  } // for (i, all points in lasso)
  painter->end();
}

void WidRender::paintEvent(QPaintEvent* evt) { 
  evt;
  QPainter painter(this);

  int wRender = size().width();
  int hRender = size().height();
  if (m_recognitionResult) {

    // check magic
    const int magic = m_recognitionResult->m_magic;
    if (magic != MAGIC_REC)
      return;

    QImage image = m_recognitionResult->m_image;
    const int wSrc = image.width();
    const int hSrc = image.height();

    int wDst, hDst;
    wDst = wRender;
    hDst = wDst * hSrc / wSrc;
    if (hDst > hRender) {
      hDst = hRender;
      wDst = hDst * wSrc / hSrc;
      assert(wDst <= wRender);
    }
    if (m_renderBinarizedBackgroundImage) {
      painter.drawImage(QRect(0, 0, wDst, hDst), image);
    } else {
      painter.fillRect(0, 0, wRender, hRender, QColor(255, 255, 255));
    }

    std::vector<TextBox> boxes = m_recognitionResult->m_textBoxes;
    m_scaleRender = (float)wDst / wSrc;
    renderBoxes(&painter, boxes);

    // draw lasso
    drawLasso(&painter);

  } else {
    painter.fillRect(0, 0, wRender, hRender, QColor(255, 255, 255));
  }
}

void WidRender::detectRectsByLasso() {
  const bool isOneDetected = m_recognitionResult->markSelectedByPoly(m_lasso, m_scaleRender);
  if (isOneDetected)  {
    m_widMain->enableButtonRemoveSelectedRectangles();
  }
}

void WidRender::mousePressEvent(QMouseEvent* evt) {
  evt;
  m_mousePressed = true;
  m_lasso.clear();
}

void WidRender::mouseReleaseEvent(QMouseEvent* evt) { 
  evt;
  m_mousePressed = false;
  if (m_lasso.size() > 0) {
    detectRectsByLasso();
    // remove lasso
    m_lasso.clear();
    repaint();
  }
}



void WidRender::mouseMoveEvent(QMouseEvent* evt) {
  if (!m_mousePressed) {
    mouseMoveUnpressed(evt);
  } else {
    mouseMovePressed(evt);
  }
}

void WidRender::mouseMovePressed(QMouseEvent* evt) {
  const int x = (int)(evt->x());
  const int y = (int)(evt->y());

  m_lasso.append(QPoint(x, y));
  repaint();
}

void WidRender::mouseMoveUnpressed(QMouseEvent* evt) {
  const int x = (int)(evt->x() / m_scaleRender);
  const int y = (int)(evt->y() / m_scaleRender);
  const int indexBox = m_recognitionResult->getBoxIndexInside(x, y);
  // avoid unnecessary repaints
  int indexBoxCurrentNew = -1;

  QString strPrint("");
  if (indexBox >= 0) {
    TextBox tbox = m_recognitionResult->m_textBoxes[indexBox];
    QRect rect = tbox.m_rect;
    QString strText = tbox.m_text;

    QTextStream(&strPrint) << "Coord: (" << rect.x() << ", " << rect.y()
                           << ") text=" << strText;
    indexBoxCurrentNew = indexBox;
  } else {
    // box not found under mouse
    indexBoxCurrentNew = -1;
  }
  // invoke repaint widget, if current selection is changed
  if (indexBoxCurrentNew != m_indexBoxCurrent) {
    m_indexBoxCurrent = indexBoxCurrentNew;
    repaint();
    m_widMain->setStatusText(strPrint);
  }
}
