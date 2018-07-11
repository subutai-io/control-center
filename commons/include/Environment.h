#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include <QObject>
#include "OsBranchConsts.h"
#include <QDebug>

class Environment : QObject {
Q_OBJECT

public:
  static Environment* Instance();
  template <class OS>
  unsigned int ramSizeInternal();
  unsigned int ramSize();
  unsigned int numCpu();
};

#endif // ENVIRONMENT_H
