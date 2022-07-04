//
// Copyright 2022 Vlad
//
// Image binarizer sample project.
//

#pragma once


// *************************************
// includes
// *************************************

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <QtWidgets/QMainWindow>
#include <QtGui/QImage>
#include <vector>
#include <QtCore/QRect>
#include <QtCore/QTime>

#include "ui_WidImageBinarizer.h"

#include "RecogRes.h"
#include "WidRender.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4611)
#endif

#include "mupdf/tessocr.h"


#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// *************************************
// classes
// *************************************

enum class AlgirithmBinType {
  ALGORITHM_SAUVOLA = 0,
  ALGORITHM_SAUVOLA_FAST = 1,
  ALGORITHM_LEPTONICA = 2,
  ALGORITHM_NONE = 3,
};


class WidImageBinarizer: public QMainWindow {
  Q_OBJECT

public:
  explicit WidImageBinarizer(QWidget* parent = Q_NULLPTR);
  virtual ~WidImageBinarizer();

  bool performOpenFile(QString& strFileName);
  void setStatusText(const QString& strText);
  void enableButtonRemoveSelectedRectangles();

  void setSauvolaRange(int range);
  void setSauvolaFactor(float factor);



protected slots:
  void onPushButtonOpen();
  void onPushButtonBinarize();

  void onSliderSauvolaRange(int val);
  void onSliderSauvolaFactor(int val);

  void onPressedRadioSauvola();
  void onPressedRadioSauvolaFast();
  void onPressedRadioLeptonica();
  void onPressedRadioNone();

  void onCheckRenderBinarizedImage(int state);
  void onCheckRenderRecognizedText(int state);
  void onCheckRenderRectangles(int state);

  void onPressedRemoveSelectedRectangles();
  void onPushButtonDown();
  void onPushButtonUp();
  void onPushButtonPlus();
  void onPushButtonMinus();

  void onRadioRotateNone();
  void onRadioRotateLeft();
  void oRadioRotateRight();

  void onPushButtonCompareBinarized();



private:
  QImage                  createSauvola(QImage& imageSrc, int neibSize, float factor);
  QImage                  createSauvolaFast(QImage& imageSrc, int neibSize, float factor);
  QImage                  createLeptonicaBinarization(QImage& imageSrc, int neibSize,
                                float factor);

  std::vector<TextBox> applyTesseract(QImage& image);
  void                    showImageSrc();
  void                    renderBoxes(QPixmap& pixmap, float scale, std::vector<TextBox>& boxes);
  void                    addResultToTab(RecognitionResult* res,
                              const QString& strTabMsg = QString(""));
  QImage                  loadPdf(const char* fileName);
  void                    loadCurrentPageFromDoc();
  void                    updatePagesUi();
  void                    ocrInit();
  void                    ocrDestroy();


  // private data
  Ui::WidImageBinarizerClass      m_ui;

  // interface to tessocr
  void*                           m_ocrApi;
  // interface to leptonica / tesseract lib
  fz_context*                     m_ctxFz;

  // source pdf document
  fz_document*                    m_doc;
  // pdf document scale
  float                           m_docScale;
  // pdf rotate
  float                           m_docRotate;
  // num pages
  int                             m_docNumPages;
  // index cur page of dicument
  int                             m_docPageIndex;

  // source input image (without processing)
  QImage                          m_imageSrc;

  // prevent double click on  button
  QTime                           m_timeClickButton;

  // sauvola parameters
  int                             m_sauvilaNeibSize;
  float                           m_sauvolaFactor;

  // binarization algorithm type
  AlgirithmBinType                m_algorithmType;

  // recognized results
  std::vector<RecognitionResult*>  m_recognitionResults;

  // widget with results
  std::vector<WidRender*>         m_widgetsRender;

  // stacked widgets
  int                             m_numWidgets;
};
