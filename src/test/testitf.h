//
//
//


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4127)
#endif

#include <QtCore/QObject>
#include <QtTest/QTest>


#if defined(_MSC_VER)
#pragma warning(pop)
#endif


class TestInterface: public QObject {
  Q_OBJECT
public:
  explicit TestInterface(QObject *parent = nullptr);

private slots:
  void testFImage();
  void testBinarize();
  void testGauss();
  void testGaussThreads();
  void testIntegralSum3x3();
  void testIntegralSum();
  void testIntegralSum2();
  void testFastMean();
};
