//
//
//

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4127)
#endif

#include <QtWidgets/QApplication>
#include <QtTest/QTest>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <cstdlib>
#include <cstdio>


#include "testitf.h"


// ************************************

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QObject *objText = new TestInterface;
  QTest::qExec(objText, argc, argv);
  return 0;
}