#ifndef TRAYSKINCONTROLLER_H
#define TRAYSKINCONTROLLER_H

#include <QObject>

class TraySkinController : public QObject
{
  Q_OBJECT
public:
  enum TRAY_SKINS {
    DEFAULT_SKIN = 0,
    DARK_SKIN,
    ORANGE_DARK_SKIN,
  };
  static const int SKIN_NUMBER = 3;

  static TraySkinController &Instance() {
    static TraySkinController ret;
    return ret;
  }
  static QString tray_skin_to_str(TRAY_SKINS skin) {
    static QString ret[] = {
      "Light (default)",
      "Dark",
      "Orange Dark",
    };
    return ret[(size_t) skin];
  }


  TRAY_SKINS tray_skin;
  void set_tray_skin(TRAY_SKINS current_skin);


private:
  QString tray_skin_to_stylesheet(TRAY_SKINS skin) {
    static QString ret[] = {
      "default",
      ":skins/dark.css",
      ":skins/orange-dark.css"
    };
    return ret[(size_t) skin];
  }
};

#endif // TRAYSKINCONTROLLER_H
