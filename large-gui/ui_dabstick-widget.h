/********************************************************************************
** Form generated from reading UI file 'dabstick-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DABSTICK_2D_WIDGET_H
#define UI_DABSTICK_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_dabstickWidget
{
public:
    QFrame *contents;
    QLCDNumber *rateDisplay;
    QLabel *label;
    QSpinBox *externalGain;
    QSpinBox *f_correction;
    QSpinBox *KhzOffset;
    QSpinBox *HzOffset;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QComboBox *rateSelector;

    void setupUi(QWidget *dabstickWidget)
    {
        if (dabstickWidget->objectName().isEmpty())
            dabstickWidget->setObjectName(QStringLiteral("dabstickWidget"));
        dabstickWidget->resize(159, 226);
        contents = new QFrame(dabstickWidget);
        contents->setObjectName(QStringLiteral("contents"));
        contents->setGeometry(QRect(0, 0, 151, 191));
        contents->setFrameShape(QFrame::StyledPanel);
        contents->setFrameShadow(QFrame::Raised);
        rateDisplay = new QLCDNumber(contents);
        rateDisplay->setObjectName(QStringLiteral("rateDisplay"));
        rateDisplay->setGeometry(QRect(0, 80, 121, 21));
        rateDisplay->setDigitCount(7);
        rateDisplay->setSegmentStyle(QLCDNumber::Flat);
        label = new QLabel(contents);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 150, 101, 21));
        externalGain = new QSpinBox(contents);
        externalGain->setObjectName(QStringLiteral("externalGain"));
        externalGain->setGeometry(QRect(0, 0, 91, 21));
        externalGain->setMaximum(103);
        externalGain->setValue(10);
        f_correction = new QSpinBox(contents);
        f_correction->setObjectName(QStringLiteral("f_correction"));
        f_correction->setGeometry(QRect(0, 20, 91, 21));
        f_correction->setMinimum(-100);
        f_correction->setMaximum(100);
        KhzOffset = new QSpinBox(contents);
        KhzOffset->setObjectName(QStringLiteral("KhzOffset"));
        KhzOffset->setGeometry(QRect(0, 40, 91, 21));
        KhzOffset->setMaximum(1000000);
        HzOffset = new QSpinBox(contents);
        HzOffset->setObjectName(QStringLiteral("HzOffset"));
        HzOffset->setGeometry(QRect(0, 60, 91, 21));
        HzOffset->setMaximum(999);
        label_2 = new QLabel(contents);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(100, 60, 51, 21));
        label_3 = new QLabel(contents);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(100, 40, 41, 21));
        label_4 = new QLabel(contents);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(100, 20, 51, 21));
        label_5 = new QLabel(contents);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(100, 0, 41, 21));
        rateSelector = new QComboBox(contents);
        rateSelector->setObjectName(QStringLiteral("rateSelector"));
        rateSelector->setGeometry(QRect(0, 100, 121, 29));

        retranslateUi(dabstickWidget);

        QMetaObject::connectSlotsByName(dabstickWidget);
    } // setupUi

    void retranslateUi(QWidget *dabstickWidget)
    {
        dabstickWidget->setWindowTitle(QApplication::translate("dabstickWidget", "Form", 0));
        label->setText(QApplication::translate("dabstickWidget", "dabstick", 0));
        label_2->setText(QApplication::translate("dabstickWidget", "Hz", 0));
        label_3->setText(QApplication::translate("dabstickWidget", "KHz", 0));
        label_4->setText(QApplication::translate("dabstickWidget", "ppm", 0));
        label_5->setText(QApplication::translate("dabstickWidget", "gain", 0));
        rateSelector->clear();
        rateSelector->insertItems(0, QStringList()
         << QApplication::translate("dabstickWidget", "960", 0)
         << QApplication::translate("dabstickWidget", "1200", 0)
         << QApplication::translate("dabstickWidget", "1536", 0)
         << QApplication::translate("dabstickWidget", "2000", 0)
         << QApplication::translate("dabstickWidget", "2500", 0)
         << QApplication::translate("dabstickWidget", "3000", 0)
        );
    } // retranslateUi

};

namespace Ui {
    class dabstickWidget: public Ui_dabstickWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DABSTICK_2D_WIDGET_H
