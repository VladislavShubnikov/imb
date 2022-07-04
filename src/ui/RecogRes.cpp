//
// Copyright 2022 Vlad
//

#include <QPoint>
#include <QList>
#include <QPolygon>

#include "RecogRes.h"

TextBox::TextBox() {
  m_selected = false;
  m_text = "";
  m_rect.setCoords(-1, -1, 0, 0);
}

RecognitionResult::RecognitionResult() { 
  m_magic = MAGIC_REC;
}

int RecognitionResult::getBoxIndexInside(int x, int y) {
  const size_t numBoxes = m_textBoxes.size();
  for (size_t i = 0; i < numBoxes; i++) {
    TextBox tbox = m_textBoxes[i];
    QRect box = tbox.m_rect;
    if (box.contains(x, y)) {
      return (int)i;
    }
  }
  return -1;
}
void RecognitionResult::removeSelected() { 
  const int numBoxes = (int)m_textBoxes.size();
  for (int i = numBoxes - 1; i >= 0; i--) {
    TextBox tb = m_textBoxes[i];
    if (tb.m_selected) {
      m_textBoxes.erase( m_textBoxes.begin() + i);
    }
  }
}


bool RecognitionResult::markSelectedByPoly(QVector<QPoint>& points,
                                           const float scaleRender) {

  QPolygon polyLasso;
  for (QPoint pt : points) {
    const int x = (int)(pt.x() / scaleRender);
    const int y = (int)(pt.y() / scaleRender);
    polyLasso.append(QPoint(x, y));
  }
  bool hasSelected = false;
  for (TextBox& tb : m_textBoxes) {
    QRect rc = tb.m_rect;
    if (polyLasso.intersects(rc))  {
      tb.m_selected = true;
      hasSelected = true;
    } else {
      tb.m_selected = false;
    }
  }
  return hasSelected;
}
