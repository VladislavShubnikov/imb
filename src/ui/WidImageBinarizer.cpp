//
// Copyright 2022 Vlad
//
// Image binarizer sample project.
//

// *************************************
// includes
// *************************************

#pragma warning(push, 3)

#include <QtWidgets/QFileDialog>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtWidgets/QBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtCore/QTime>


#pragma warning(pop)

#include <cassert>

#include "tesseract/baseapi.h"
#include "tesseract/publictypes.h"


#include "WidImageBinarizer.h"
#include "WidRender.h"
#include "WidCompare.h"


#include "FImage.h"
#include "Bmp.h"
#include "ImageConv.h"
#include "ImageDif.h"
#include "FastMeanStd.h"


// *************************************
// funcs
// *************************************


WidImageBinarizer::WidImageBinarizer(QWidget* parent)
    : QMainWindow(parent) {
  m_ui.setupUi(this);

  QIcon iconApp(":/text_recognize.png");
  this->setWindowIcon(iconApp);

  m_widgetsRender.clear();
  m_recognitionResults.clear();
  m_timeClickButton = QTime::currentTime();

  m_algorithmType = AlgirithmBinType::ALGORITHM_SAUVOLA;
  m_numWidgets = 0;
  m_sauvilaNeibSize = 2;
  m_sauvolaFactor = 0.25F;

  // register connections
  connect(m_ui.m_pushButtonOpenImage, SIGNAL(pressed()), this, SLOT(onPushButtonOpen()));
  connect(m_ui.m_pushButtonBinarize, SIGNAL(pressed()), this,
          SLOT(onPushButtonBinarize()));

  connect(m_ui.m_sliderSauvolaRange, SIGNAL(valueChanged(int)), this, SLOT(onSliderSauvolaRange(int)));
  connect(m_ui.m_sliderSauvolaFactor, SIGNAL(valueChanged(int)), this,
          SLOT(onSliderSauvolaFactor(int)));
  


  connect(m_ui.m_radioFuncSauvola, SIGNAL(pressed()), this, SLOT(onPressedRadioSauvola()));
  connect(m_ui.m_radioFuncSauvolaFast, SIGNAL(pressed()), this,
          SLOT(onPressedRadioSauvolaFast()));
  connect(m_ui.m_radioFuncLeptonica, SIGNAL(pressed()), this,
          SLOT(onPressedRadioLeptonica()));
  connect(m_ui.m_radioFuncNone, SIGNAL(pressed()), this, SLOT(onPressedRadioNone()));


  connect(m_ui.m_checkBoxRenderBinarizedImage, SIGNAL(stateChanged(int)), this, 
          SLOT(onCheckRenderBinarizedImage(int)));
  connect(m_ui.m_checkBoxRenderRecogniedText, SIGNAL(stateChanged(int)), this,
          SLOT(onCheckRenderRecognizedText(int)));
  connect(m_ui.m_checkBoxRenderRectangles, SIGNAL(stateChanged(int)), this,
          SLOT(onCheckRenderRectangles(int)));

  connect(m_ui.m_buttonRemoveSelectedRectangles, SIGNAL(pressed()), this, 
          SLOT(onPressedRemoveSelectedRectangles()));

  connect(m_ui.m_buttonDown, SIGNAL(pressed()), this,
          SLOT(onPushButtonDown()));
  connect(m_ui.m_buttonUp, SIGNAL(pressed()), this, 
          SLOT(onPushButtonUp()));
  connect(m_ui.m_buttonPlus, SIGNAL(pressed()), this, SLOT(onPushButtonPlus()));
  connect(m_ui.m_buttonMinus, SIGNAL(pressed()), this, SLOT(onPushButtonMinus()));

  connect(m_ui.m_radioRotateNone, SIGNAL(pressed()), this,
          SLOT(onRadioRotateNone()));
  connect(m_ui.m_radioRotateLeft, SIGNAL(pressed()), this,
          SLOT(onRadioRotateLeft()));
  connect(m_ui.m_radioRotateRight, SIGNAL(pressed()), this,
          SLOT(oRadioRotateRight()));

  connect(m_ui.m_buttonCompareBinarized, SIGNAL(pressed()), this, SLOT(onPushButtonCompareBinarized()) );


  setSauvolaRange(m_sauvilaNeibSize);
  setSauvolaFactor(m_sauvolaFactor);

  m_ui.m_pushButtonOpenImage->setEnabled(true);
  m_ui.m_pushButtonBinarize->setEnabled(false);

  // remove initial tabs
  const int numTabs = m_ui.m_tabWidget->count();
  for (int i = numTabs - 1; i >= 0; i--) {
    m_ui.m_tabWidget->removeTab(i);
  }

  // switch off progress
  m_ui.m_progressRecognition->setVisible(false);

  // init leptonica LIB
  m_ctxFz = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
  fz_register_document_handlers(m_ctxFz);

  ocrInit();

  // init pdf load from mupdf
  m_doc = nullptr;
  m_docNumPages = 0;
  m_docPageIndex = 0;
  m_docScale = 2.0F;
  m_docRotate = 0.0F;
}


WidImageBinarizer::~WidImageBinarizer() {
  ocrDestroy();

  if (m_doc)
    fz_drop_document(m_ctxFz, m_doc);

  // close leptonica lib
  fz_drop_context(m_ctxFz);

  // destroy render widgets
  for (WidRender* wid: m_widgetsRender) {
    delete wid;
  }
  // destroy results
  for (RecognitionResult* res: m_recognitionResults) {
    delete res;
  }
}

QImage WidImageBinarizer::loadPdf(const char* fileName) {
  fz_pixmap*    pix;
  fz_matrix     ctm;
  
  m_doc = fz_open_document(m_ctxFz, fileName);
  m_docNumPages = fz_count_pages(m_ctxFz, m_doc);

  ctm = fz_scale(m_docScale, m_docScale);
  ctm = fz_pre_rotate(ctm, m_docRotate);

  pix = fz_new_pixmap_from_page_number(m_ctxFz, m_doc, m_docPageIndex, ctm,
                                       fz_device_rgb(m_ctxFz), 0);

  QImage imgPdf = BmpFzPixToQImage(pix);

  // clean up
  fz_drop_pixmap(m_ctxFz, pix);
  return imgPdf;
}
void WidImageBinarizer::loadCurrentPageFromDoc() {
  fz_pixmap* pix;
  fz_matrix ctm;

  ctm = fz_scale(m_docScale, m_docScale);
  ctm = fz_pre_rotate(ctm, m_docRotate);

  pix = fz_new_pixmap_from_page_number(m_ctxFz, m_doc, m_docPageIndex, ctm,
                                       fz_device_rgb(m_ctxFz), 0);

  m_imageSrc = BmpFzPixToQImage(pix);
  #ifdef DEEP_DEBUG
    m_imageSrc.save("log/pdf_src.png");
  #endif

  // clean up
  fz_drop_pixmap(m_ctxFz, pix);
  // send image to screen
  showImageSrc();
}

bool WidImageBinarizer::performOpenFile(QString& strFileName) {
  QString suf = strFileName.right(3);
  bool okLoad;
  if ((suf == "pdf") || (suf == "PDF")) {
    QByteArray ba = strFileName.toUtf8();
    const char *fn = ba.data();
    m_imageSrc = loadPdf(fn);
    okLoad = !m_imageSrc.isNull();
  } else {
    okLoad = m_imageSrc.load(strFileName);
    m_docNumPages = 1;
    m_docPageIndex = 0;
  }
  if (!okLoad) {
    return false;
  }
  showImageSrc();
  updatePagesUi();
  return true;
}

void WidImageBinarizer::updatePagesUi() {
  QString strText;

  QTextStream(&strText) << m_docScale;
  m_ui.m_lineScale->setText(strText);
  strText = "";

  QTextStream(&strText) << (m_docPageIndex + 1) << " / " << m_docNumPages;
  m_ui.m_linePage->setText(strText);
  if (m_docNumPages <= 1) {
    m_ui.m_buttonDown->setEnabled(false);
    m_ui.m_buttonUp->setEnabled(false);
    return;
  }
  const bool isDownEna = (m_docPageIndex < m_docNumPages - 1);
  m_ui.m_buttonDown->setEnabled(isDownEna);
  const bool isUpEna = (m_docPageIndex > 0);
  m_ui.m_buttonUp->setEnabled(isUpEna);
}

void WidImageBinarizer::onPushButtonPlus() {
  m_docScale = m_docScale * 2.0F;
  updatePagesUi();
  loadCurrentPageFromDoc();
}

void WidImageBinarizer::onPushButtonMinus() {
  if (m_docScale >= 2.0F) {
    m_docScale = m_docScale / 2.0F;
    updatePagesUi();
    loadCurrentPageFromDoc();
  }
}

void WidImageBinarizer::onRadioRotateNone() {
  m_docRotate = 0.0F;
  loadCurrentPageFromDoc();
}
void WidImageBinarizer::onRadioRotateLeft() {
  m_docRotate = -90.0F;
  loadCurrentPageFromDoc();
}
void WidImageBinarizer::oRadioRotateRight() {
  m_docRotate = +90.0F;
  loadCurrentPageFromDoc();
}



void WidImageBinarizer::onPushButtonDown() {
  if (m_docPageIndex >= m_docNumPages - 1)
    return;
  m_docPageIndex++;
  updatePagesUi();
  loadCurrentPageFromDoc();
}

void WidImageBinarizer::onPushButtonUp() {
  if (m_docPageIndex <= 0) 
    return;
  m_docPageIndex--;
  updatePagesUi();
  loadCurrentPageFromDoc();
}



void WidImageBinarizer::onPushButtonOpen() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Select source image to open", "",
      "Jpeg files (*.jpg);;Png files (*.png);;Pdf files (*.pdf);;All files (*.*)");
  if (fileName.length() < 2) 
    return;
  performOpenFile(fileName);
}

void WidImageBinarizer::enableButtonRemoveSelectedRectangles() {
  m_ui.m_buttonRemoveSelectedRectangles->setEnabled(true);
}

void WidImageBinarizer::onPressedRemoveSelectedRectangles() {
  const int indexTab = m_ui.m_tabWidget->currentIndex();
  if (indexTab >= 0) {
    RecognitionResult* res = m_recognitionResults[indexTab];
    res->removeSelected();
    WidRender* wid = m_widgetsRender[indexTab];
    wid->repaint();
    m_ui.m_buttonRemoveSelectedRectangles->setEnabled(false);
  }
}

void WidImageBinarizer::onPressedRadioSauvola() {
  m_algorithmType = AlgirithmBinType::ALGORITHM_SAUVOLA;
}
void WidImageBinarizer::onPressedRadioSauvolaFast() {
  m_algorithmType = AlgirithmBinType::ALGORITHM_SAUVOLA_FAST;
}
void WidImageBinarizer::onPressedRadioLeptonica() {
  m_algorithmType = AlgirithmBinType::ALGORITHM_LEPTONICA;
}
void WidImageBinarizer::onPressedRadioNone() {
  m_algorithmType = AlgirithmBinType::ALGORITHM_NONE;
}

void WidImageBinarizer::onCheckRenderBinarizedImage(int state) {
  const bool isCheck = (state == Qt::Checked);
  /*
  const int tabIndex = m_ui.m_tabWidget->currentIndex();
  if (tabIndex >= 0) {
    WidRender* wid = m_widgetsRender[tabIndex];
    wid->setRenderBinarizedBackgroundImage(isCheck);
  }
  */
  const int numTabs = m_ui.m_tabWidget->count();
  for (int i = 0; i < numTabs; i++) {
    WidRender* wid = m_widgetsRender[i];
    wid->setRenderBinarizedBackgroundImage(isCheck);
  }
} 
void WidImageBinarizer::onCheckRenderRecognizedText(int state) {
  const bool isCheck = (state == Qt::Checked);
  /*
  const int tabIndex = m_ui.m_tabWidget->currentIndex();
  if (tabIndex >= 0) {
    WidRender* wid = m_widgetsRender[tabIndex];
    wid->setRenderRecognizedText(isCheck);
  }
  */
  const int numTabs = m_ui.m_tabWidget->count();
  for (int i = 0; i < numTabs; i++) {
    WidRender* wid = m_widgetsRender[i];
    wid->setRenderRecognizedText(isCheck);
  }
}
void WidImageBinarizer::onCheckRenderRectangles(int state) {
  const bool isCheck = (state == Qt::Checked);
  const int numTabs = m_ui.m_tabWidget->count();
  for (int i = 0; i < numTabs; i++) {
    WidRender* wid = m_widgetsRender[i];
    wid->setRenderRectangles(isCheck);
  }
}

void WidImageBinarizer::setSauvolaRange(int range) { 
  m_ui.m_sliderSauvolaRange->setValue(range);
}

void WidImageBinarizer::setSauvolaFactor(float factor) {
  const int tick = (int)(factor * 10.0F);
  m_ui.m_sliderSauvolaFactor->setValue(tick);
}

void WidImageBinarizer::onSliderSauvolaRange(int val) {
  m_sauvilaNeibSize = val;
  QString strText;
  QTextStream(&strText) << "Sauvola range: " << val;
  m_ui.m_labelSauvolaRange->setText(strText);
  // onPushButtonBinarize();
}
void WidImageBinarizer::onSliderSauvolaFactor(int val) { 
  m_sauvolaFactor = (float)val * 0.1F;
  QString strText;
  QTextStream(&strText) << "Sauvola factor: " << m_sauvolaFactor;
  m_ui.m_labelSauvolaFactor->setText(strText);
}




void WidImageBinarizer::showImageSrc()
{
  // get label current size
  const QSize szLabel = m_ui.m_labelImageSrc->size();

  QSize szImage = m_imageSrc.size();
  const float aspect = (float)szImage.width() / szImage.height();
  QSize szTarget;

  int w = szLabel.width();
  int h = (int)(szLabel.width() / aspect);
  if (h > szLabel.height()) {
    h = szLabel.height();
    w = (int)(h * aspect);
    assert(w <= szLabel.width());
  }

  QImage imageScaled = m_imageSrc.scaled(QSize(w, h));

  QPixmap pixmap = QPixmap::fromImage(imageScaled);

  m_ui.m_labelImageSrc->setPixmap(pixmap);
  m_ui.m_pushButtonBinarize->setEnabled(true);
}

void WidImageBinarizer::renderBoxes(QPixmap& pixmap, float scale,
                                    std::vector<TextBox>& boxes) {
  QPainter painter(&pixmap);
  for (TextBox& tb: boxes) {
    QRect r = tb.m_rect;
    QString s = tb.m_text;
    const int x = (int)(r.x() * scale);
    const int y = (int)(r.y() * scale);
    const int w = (int)(r.width() * scale);
    const int h = (int)(r.height() * scale);

    painter.setPen(QPen(Qt::green, 4, Qt::SolidLine));
    painter.drawRect(x, y, w, h);

    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    painter.drawText(x, y, s);
  }
  painter.end();
}

void WidImageBinarizer::setStatusText(const QString& strText) { 
  m_ui.m_lineEditStatus->setText(strText);
}

void WidImageBinarizer::addResultToTab(RecognitionResult* res,
                                       const QString& strTabMsg) {
  QRect rcTab = m_ui.m_tabWidget->geometry();
  const int wRender = rcTab.width() - 4;
  const int hRender = rcTab.height() - 20;

  auto* wid = new WidRender();
  wid->resize(wRender, hRender);
  wid->setRecognitionResult(res);
  wid->setMainWindow(this);

  m_ui.m_tabWidget->addTab(wid, QString());
  // QString strTab = QString("Tab %1").arg(m_numStacked + 1);
  m_ui.m_tabWidget->setTabText(m_numWidgets, strTabMsg);
  m_widgetsRender.push_back(wid);
  m_recognitionResults.push_back(res);
  m_numWidgets++;

  if (m_numWidgets >= 2) {
    m_ui.m_buttonCompareBinarized->setEnabled(true);
  }
}


void WidImageBinarizer::onPushButtonBinarize()
{ 
  // prevent repeated click on this button
  QTime tm = QTime::currentTime();
  const int dt = m_timeClickButton.msecsTo(tm);
  if (dt < 500)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_ui.m_pushButtonBinarize->setEnabled(false);
  m_ui.m_pushButtonOpenImage->setEnabled(false);
  m_ui.m_sliderSauvolaRange->setEnabled(false);
  m_ui.m_sliderSauvolaFactor->setEnabled(false);

  QImage imageBin;

  if (m_algorithmType == AlgirithmBinType::ALGORITHM_SAUVOLA) {
    imageBin = createSauvola(m_imageSrc, m_sauvilaNeibSize, m_sauvolaFactor);
  }
  if (m_algorithmType == AlgirithmBinType::ALGORITHM_SAUVOLA_FAST) {
    imageBin = createSauvolaFast(m_imageSrc, m_sauvilaNeibSize, m_sauvolaFactor);
  }
  if (m_algorithmType == AlgirithmBinType::ALGORITHM_LEPTONICA) {
    imageBin = createLeptonicaBinarization(m_imageSrc, m_sauvilaNeibSize, m_sauvolaFactor);
  }
  if (m_algorithmType == AlgirithmBinType::ALGORITHM_NONE) {
    imageBin = m_imageSrc.copy();
  }

  std::vector<TextBox> boxes = applyTesseract(imageBin);

  QString strTab = QString("Binarized %1").arg(m_numWidgets + 1);

  auto* recRes = new RecognitionResult();
  recRes->m_image = imageBin;
  recRes->m_textBoxes = boxes;

  addResultToTab(recRes, strTab);

  m_ui.m_pushButtonOpenImage->setEnabled(true);
  m_ui.m_sliderSauvolaRange->setEnabled(true);
  m_ui.m_sliderSauvolaFactor->setEnabled(true);
  m_ui.m_pushButtonBinarize->setEnabled(true);

  QApplication::restoreOverrideCursor();

  // prevent dbl click
  m_timeClickButton = QTime::currentTime();
}


// #define USE_MUPDF

void WidImageBinarizer::ocrInit() {
  // Initialize tesseract-ocr with English, without specifying tessdata path
  //
  // trained models are taken from
  // https://github.com/tesseract-ocr/tessdata/raw/master/eng.traineddata
  //

  const char* dataPath = "data\\models\\";
  const char* lang = "rus";

  #ifdef USE_MUPDF
    m_ocrApi = ocr_init(m_ctxFz, lang, dataPath);
  #else
  set_leptonica_mem(m_ctxFz);


    tesseract::TessBaseAPI* api;
    m_ocrApi = api = new tesseract::TessBaseAPI();
    api->Init(dataPath, 
      0, 
      lang, 
      tesseract::OcrEngineMode::OEM_LSTM_ONLY,
      NULL, 0,
      NULL, NULL,
      false, // set_only_non_debug_params
      NULL);
  #endif
}

void WidImageBinarizer::ocrDestroy() { 
#ifdef USE_MUPDF
  ocr_fin(m_ctxFz, m_ocrApi); 
#else
  tesseract::TessBaseAPI* api = (tesseract::TessBaseAPI*)m_ocrApi;
  api->End();
  delete api;
  clear_leptonica_mem(m_ctxFz);
#endif
}


QImage WidImageBinarizer::createLeptonicaBinarization(QImage&       imageSrc,
                                                      const int     neibSize,
                                                      const float   factor) {

  PIX* pixSrc = nullptr;
  PIX* pixDest = nullptr;
  PIX* pixThr = nullptr;

  Bmp bmpMem;
  bmpMem.initFromQImage(imageSrc);

  auto* dataBits = (l_uint8*)bmpMem.bits();
  size_t sz = bmpMem.sizeInBytes();

  pixSrc = pixReadMem(dataBits, sz);
  assert(pixSrc);

  #ifdef DEEP_DEBUG
    pixWrite("log/src_for_lepto.png", pixSrc, IFF_PNG);
  #endif

  const int subdivTiles = 4;
  pixSauvolaBinarizeTiled(pixSrc, neibSize * 2 + 1, factor, subdivTiles, subdivTiles,
                          &pixThr, &pixDest);

  #ifdef DEEP_DEBUG
    pixWrite("log/bin_lepto.png", pixDest, IFF_PNG);
  #endif

  QImage imageBin; //  = imageFloatDest.getQImage();
  imageBin = BmpPixToQImage(pixDest);

  #ifdef DEEP_DEBUG
    imageBin.save("log/qimg_bina.png");
  #endif

  pixDestroy(&pixDest);
  pixDestroy(&pixThr);
  pixDestroy(&pixSrc);
  return imageBin;
}


QImage WidImageBinarizer::createSauvola(QImage& imageSrc, const int neibSize,
                                        const float factor) {
  //
  // According to https://craftofcoding.wordpress.com/2021/10/06/thresholding-algorithms-sauvola-local/
  // threshold value is calculated as:
  // t = M * (1 + k * ((S/128) - 1))
  // t: result threshold
  // M: media value
  // S: standard deviation value
  // k: factor in [0.2 .. 0.5]

  FImage imageFloatSrc(imageSrc);

  FImage imageFloatMean = imageFloatSrc.getWindowedMean(neibSize * 2 + 1);
  FImage imageFloatStdDev = imageFloatSrc.getWindowedStdDev(imageFloatMean, neibSize * 2 + 1);
  FImage imageFloatThresholds =
      imageFloatMean.getSauvolaThreshold(imageFloatStdDev, factor);

  FImage imageFloatDest = imageFloatSrc.applyThresholds(imageFloatThresholds);

  QImage imageBin = imageFloatDest.getQImage();
  return imageBin;
}

QImage WidImageBinarizer::createSauvolaFast(QImage& imageSrc, const int neibSize,
                                        const float factor) {
  FImage imageFloatSrc(imageSrc);
  FImage imageFloatMean(imageSrc);
  FImage imageFloatStdDev(imageSrc);

  FastMeanStd::getFastMeanStd(imageFloatSrc, imageFloatMean, imageFloatStdDev,
                              neibSize * 2 + 1);

  FImage imageFloatThresholds =
      imageFloatMean.getSauvolaThreshold(imageFloatStdDev, factor);

  FImage imageFloatDest = imageFloatSrc.applyThresholds(imageFloatThresholds);

  QImage imageBin = imageFloatDest.getQImage();
  return imageBin;
}


std::vector<TextBox> WidImageBinarizer::applyTesseract(QImage& image) {
  std::vector<TextBox> textBoxes;

  const uchar* bits = image.bits();
  const int w = image.width();
  const int h = image.height();
  const int bpp = 4;
  const int bpLine = image.bytesPerLine();

  auto* ocr = (tesseract::TessBaseAPI*)m_ocrApi;
  ocr->SetImage(bits, w, h, bpp, bpLine);


  Boxa* bounds = ocr->GetWords(nullptr);

  // prevent recognize too narrow (by height) text boxes
  const int hMax = h / 8;
  m_ui.m_progressRecognition->setVisible(true);
  m_ui.m_progressRecognition->setValue(0);

  for (int i = 0; i < bounds->n; ++i) {
    // progress rare update
    if ((i & 15) == 0) {
      const int valProgress = 100 * i / bounds->n;
      m_ui.m_progressRecognition->setValue(valProgress);
      QCoreApplication::processEvents();
    }

    Box* b = bounds->box[i];
    if (b->h > hMax) {
      continue;
    }

    ocr->SetRectangle(b->x, b->y, b->w, b->h);
    char* outText;
    outText = ocr->GetUTF8Text();
    QString strRecognized(outText);
    delete[] outText;

    if (strRecognized.length() == 0) {
      continue;
    }

    QRect rect(b->x, b->y, b->w, b->h);

    TextBox tBox;
    tBox.m_rect = rect;
    tBox.m_text = strRecognized;
    textBoxes.push_back(tBox);
  } // for i, all detected rects by ocr

  // free memory from detected by ocr boxes set
  boxaDestroy(&bounds);

  m_ui.m_progressRecognition->setVisible(false);
  return textBoxes;
}

void WidImageBinarizer::onPushButtonCompareBinarized() { 
  assert(m_numWidgets >= 2);
  RecognitionResult* resA = m_recognitionResults[0];
  RecognitionResult* resB = m_recognitionResults[1];

  FImage imageA(resA->m_image);
  FImage imageB(resB->m_image);
  FImage imageDiff(resA->m_image);
  const float distBar = 0.2F;
  ImageDiff::getDiff(imageA, imageB, distBar, imageDiff);
  QImage qimageDiff = imageDiff.getQImage();

  #ifdef DEEP_DEBUG
    qimageDiff.save("log/diff.png");
  #endif

  auto *dlg = new WidCompare(this);
  dlg->setImages(resA->m_image, resB->m_image, qimageDiff);
  int ok;
  ok = dlg->exec();
  dlg->hide();
  dlg->close();
  delete dlg;
}
