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
  bool isCpuSupport(); // Checks CPU supports hardware virtualization
};

#endif // ENVIRONMENT_H
