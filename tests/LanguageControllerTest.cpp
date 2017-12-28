#include "LanguageControllerTest.h"
#include "LanguageController.h"
#include <QTest>

void LanguageControllerTest::testLocaleTypeToVal() {
    QFETCH(int , locale_type);
    QFETCH(QString , expected_lang);
    QString returned_value = LanguageController::LocaleTypeToStr((LanguageController::LOCALE_TYPE)locale_type);
    QCOMPARE(returned_value , expected_lang);
}

void LanguageControllerTest::testLocaleTypeToVal_data() {
    QTest::addColumn<int> ("locale_type");
    QTest::addColumn<QString> ("expected_lang");
    // QTest::newRow("Russian") << (int) LanguageController::LOCALE_RU << "Русский";
    QTest::newRow("English") << (int) LanguageController::LOCALE_EN << "English";
}
