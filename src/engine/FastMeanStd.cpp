 

 #include "FastMeanStd.h"

 void FastMeanStd::getFastMeanStd( const FImage& imageSrc,
                                  FImage& imageMean,
                                  FImage& imageStd,
                                  int neibSize) {
  FImage imageSum = imageSrc.getIntegralImage();
  FImage imageSum2 = imageSrc.getIntegralImage2();

  assert((neibSize & 1) == 1); // check this is odd value
  const int ns2 = neibSize / 2;

  const int w = imageSrc.width();
  const int h = imageSrc.height();

  float* pixMean = imageMean.getBits();
  float* pixStd = imageStd.getBits();

  int k = 0; // dest index
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int xMin = x - ns2;
      if (xMin < 0)
        xMin = 0;
      int xMax = x + ns2;
      if (xMax >= w)
        xMax = w - 1;
      int yMin = y - ns2;
      if (yMin < 0) 
        yMin = 0;
      int yMax = y + ns2;
      if (yMax >= h) 
        yMax = h - 1;
      float m = imageSum.getSum(xMin, yMin, xMax, yMax);
      const int numPixInNeib = (xMax - xMin + 1) * (yMax - yMin + 1);
      m = m / numPixInNeib;
      pixMean[k++] = m;
    } // for x
  } // for y

  k = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int xMin = x - ns2;
      if (xMin < 0) 
        xMin = 0;
      int xMax = x + ns2;
      if (xMax >= w) 
        xMax = w - 1;
      int yMin = y - ns2;
      if (yMin < 0) 
        yMin = 0;
      int yMax = y + ns2;
      if (yMax >= h) 
        yMax = h - 1;

      const int numPixInNeib = (xMax - xMin + 1) * (yMax - yMin + 1);

      float sumVal = imageSum.getSum(xMin, yMin, xMax, yMax);
      float sumVal2 = imageSum2.getSum2(xMin, yMin, xMax, yMax);
      float mean = pixMean[k];

      float std = sumVal2 - 2.0F * mean * sumVal + mean * mean * numPixInNeib;
      std = std / numPixInNeib;
      std = sqrtf(std);

      pixStd[k] = std;
      k++;

    } // for x
  } // for y


 }