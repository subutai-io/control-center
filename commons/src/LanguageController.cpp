#include <LanguageController.h>
#include <SettingsManager.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

LanguageController::LanguageController() {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

LanguageController::~LanguageController() {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

LanguageController* LanguageController::Instance() {
  static LanguageController instance;
  return &instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const QString& LanguageController::LocaleTypeToStr(LOCALE_TYPE lt) {
  static QString lt_str[] = {tr("English"), tr("Brazilian Portuguese")};
  return lt_str[lt];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const QString& LanguageController::LocaleTypeToVal(LOCALE_TYPE lt) {
  static QString lt_val[] = {"en_US", "pt_BR"};
  return lt_val[lt];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

QString LanguageController::CurrentLocale() {
  QString locale = LanguageController::LocaleTypeToVal(LanguageController::LOCALE_EN);

  if (LanguageController::LAST >= CSettingsManager::Instance().locale()) {
    locale = LanguageController::LocaleTypeToVal(
             (LanguageController::LOCALE_TYPE)CSettingsManager::Instance().locale());
  }

  return locale;
}
