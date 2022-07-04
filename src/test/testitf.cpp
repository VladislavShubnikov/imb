//
//
//

#include "testitf.h"
#include "FImage.h"
#include "FastMeanStd.h"


TestInterface::TestInterface(QObject *parent) {
  parent; // avoid compile warnings
}

void TestInterface::testFImage() {
  const int w = 16;
  const int h = 16;
  FImage *image = new FImage(w, h);
  QVERIFY(image != nullptr);
  delete image;
}

void TestInterface::testBinarize() {
  const int w = 16;
  const int h = 16;
  FImage imageSrc(w, h);
  float *pixels = imageSrc.getBits();
  const int numPixels = w * h;
  for (int i = 0; i < numPixels; i++) {
    pixels[i] = 255.0f;
  }

  int x = 8, y = 8;
  pixels[x + y * w] = 0.0F;

  // get mean image
  FImage imageMean = imageSrc.getWindowedMean(5);
  pixels = imageMean.getBits();
  float val;
  float valMatch;

  val = pixels[0];
  valMatch = 255.0f;
  QVERIFY(fabs(val - valMatch) < 1.0e-4F);
  
  val = pixels[x + y * w];
  valMatch = (255.0F  * 25 - 255) / 25.0F;
  QVERIFY(fabs(val  - valMatch) < 1.0e-4F);

  // get stddev image
  FImage imageStdDev = imageSrc.getWindowedStdDev(imageMean, 5);
  // get threshold
  FImage imageThr = imageMean.getSauvolaThreshold(imageStdDev, 0.25f);
  // get binarized
  FImage imageBina = imageSrc.applyThresholds(imageThr);

  pixels = imageBina.getBits();

  val = pixels[0];
  valMatch = 255.0f;
  QVERIFY(fabs(val - valMatch) < 1.0e-4F);

  val = pixels[x + y * w];
  valMatch = 0.0F;
  QVERIFY(fabs(val - valMatch) < 1.0e-4F);
}

void TestInterface::testGauss() {
  const int w = 16;
  const int h = 16;
  FImage imageSrc(w, h);
  const int numPixels = w * h;
  float *pixels = imageSrc.getBits();
  const float colBack = 255.0F;
  const float colObj = 0.0f;

  for (int i = 0; i < numPixels; i++) {
    pixels[i] = colBack;
  }  //
  const int xc = w / 2;
  const int yc = h / 2;
  pixels[xc + yc * w] = colObj;
  pixels[xc + yc * w + 1] = colObj;
  pixels[xc + yc * w - 1] = colObj;
  pixels[xc + yc * w + w] = colObj;
  pixels[xc + yc * w - w] = colObj;

  FImage imageKernel = getGaussianKernel(7, 7, 0.4F);
  FImage imageSmooth = imageSrc.getGaussSmooth(imageKernel);
  float *pixelsDst = imageSmooth.getBits();
  QVERIFY((int)pixelsDst[0] == (int)colBack);
  #ifdef DEEP_DEBUG
    QImage im = imageSmooth.getQImage();
    im.save("log/utest.png");
  #endif
}

void TestInterface::testIntegralSum3x3() {
  const int w = 3;
  const int h = 3;
  FImage imageSrc(w, h);
  const int numPixels = w * h;
  float *pixels = imageSrc.getBits();
  pixels[0] = 192;
  pixels[1] = 166;
  pixels[2] = 113;

  pixels[3] = 147;
  pixels[4] = 194;
  pixels[5] = 227;

  pixels[6] = 219;
  pixels[7] = 97;
  pixels[8] = 29;

  FImage imageIntSum = imageSrc.getIntegralImage();
  int val;
  int xMin, yMin, xMax, yMax;

  xMin = 1; yMin = 1;
  xMax = 2; yMax = 2;

  val = (int)imageIntSum.getSum(xMin, yMin, xMax, yMax);
  QVERIFY(val == 547);

  xMin = 1; yMin = 0;
  xMax = 2; yMax = 1;
  val = (int)imageIntSum.getSum(xMin, yMin, xMax, yMax);
  QVERIFY(val == 700);

  xMin = 0; yMin = 1;
  xMax = 1; yMax = 2;
  val = (int)imageIntSum.getSum(xMin, yMin, xMax, yMax);
  QVERIFY(val == 657);
}

void TestInterface::testIntegralSum() {
  const int w = 16;
  const int h = 16;
  FImage imageSrc(w, h);
  const int numPixels = w * h;
  float *pixels = imageSrc.getBits();
  for (int i = 0; i < numPixels; i++) {
    pixels[i] = 1.0F;
  } //
  FImage imageIntSum = imageSrc.getIntegralImage();
  pixels = imageIntSum.getBits();
  int val;

  val = (int)pixels[0];
  QVERIFY(val == 1);
  val = (int)pixels[1];
  QVERIFY(val == 2);
  val = (int)pixels[2];
  QVERIFY(val == 3);

  val = (int)pixels[0 + 1 * w];
  QVERIFY(val == 2);
  val = (int)pixels[0 + 2 * w];
  QVERIFY(val == 3);

  val = (int)pixels[1 + 1 * w];
  QVERIFY(val == 4);
  val = (int)pixels[2 + 1 * w];
  QVERIFY(val == 6);

  val = (int)imageIntSum.getSum(1, 1, 3, 3);
  QVERIFY(val == 9);
  val = (int)imageIntSum.getSum(1, 1, 4, 3);
  QVERIFY(val == 12);
}

void TestInterface::testIntegralSum2() {
  const int w = 16;
  const int h = 16;
  FImage imageSrc(w, h);
  const int numPixels = w * h;
  float *pixels = imageSrc.getBits();
  for (int i = 0; i < numPixels; i++) {
    pixels[i] = 2.0F;
  }  //
  FImage imageIntSum2 = imageSrc.getIntegralImage2();
  pixels = imageIntSum2.getBits();
  int val;

  val = (int)pixels[0];
  QVERIFY(val == 4);
  val = (int)pixels[1];
  QVERIFY(val == 8);
  val = (int)pixels[2];
  QVERIFY(val == 12);

  val = (int)pixels[0 + 1 * w];
  QVERIFY(val == 8);
  val = (int)pixels[0 + 2 * w];
  QVERIFY(val == 12);

  val = (int)pixels[1 + 1 * w];
  QVERIFY(val == 16);
  val = (int)pixels[2 + 1 * w];
  QVERIFY(val == 24);

  val = (int)imageIntSum2.getSum2(1, 1, 3, 3);
  QVERIFY(val == 36);
}

void TestInterface::testFastMean() {
  const int w = 8;
  const int h = 8;
  FImage imageSrc(w, h);
  const int numPixels = w * h;
  float *pixels = imageSrc.getBits();

  srand(0x78656);
  for (int i = 0; i < numPixels; i++) {
    const int val = rand() & 255;
    pixels[i] = (float)val;
  }  //

  FImage imageMeanFast(imageSrc);
  FImage imageStdFast(imageSrc);

  const int neibSize = 3;
  // perform fast calculation
  FastMeanStd::getFastMeanStd(imageSrc, imageMeanFast, imageStdFast, neibSize);

  // perform slow calculation
  FImage imageMeanSlow = imageSrc.getWindowedMean(neibSize);
  FImage imageStdSlow = imageSrc.getWindowedStdDev(imageMeanSlow, neibSize);

  const float *floatMeanSlow = imageMeanSlow.getBits();
  const float *floatMeanFast = imageMeanFast.getBits();
  float err = 0.0F;
  for (int i = 0; i < numPixels; i++) {
    float dx = floatMeanSlow[i] - floatMeanFast[i]; 
    dx = (dx >= 0) ? dx : -dx;
    err += dx;
  } // for i
  err = err / numPixels;
  QVERIFY(err < 1.0e-3F);

  err = 0.0F;
  const float *floatStdSlow = imageStdSlow.getBits();
  const float *floatStdFast = imageStdFast.getBits();
  for (int i = 0; i < numPixels; i++) {
    float dx = floatStdSlow[i] - floatStdFast[i];
    dx = (dx >= 0) ? dx : -dx;
    err += dx;
  }  // for i
  err = err / numPixels;
  QVERIFY(err < 1.0e-3F);
}
