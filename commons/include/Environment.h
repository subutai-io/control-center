#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include <QObject>
#include "OsBranchConsts.h"
#include <QDebug>

class Environment : QObject {
Q_OBJECT

public:
  static Environment* Instance();
  unsigned int ramSize();
  unsigned int numCpu();
  unsigned int diskSize();
};

#endif // ENVIRONMENT_H
