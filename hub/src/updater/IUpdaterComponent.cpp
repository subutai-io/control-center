#include "updater/IUpdaterComponent.h"
using namespace update_system;

#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::P2P = "p2p";
#else
const QString IUpdaterComponent::P2P = "p2p.exe";
#endif
const QString IUpdaterComponent::TRAY = "tray";
const QString IUpdaterComponent::RH = "resource_host";
