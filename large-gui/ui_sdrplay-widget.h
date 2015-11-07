/********************************************************************************
** Form generated from reading UI file 'sdrplay-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDRPLAY_2D_WIDGET_H
#define UI_SDRPLAY_2D_WIDGET_H

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

class Ui_sdrplayWidget
{
public:
    QFrame *frame;
    QLabel *label;
    QSpinBox *externalGain;
    QSpinBox *f_correction;
    QSpinBox *KhzOffset;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *statusLabel;
    QComboBox *rateSelector;
    QLCDNumber *api_version;

    void setupUi(QWidget *sdrplayWidget)
    {
        if (sdrplayWidget->objectName().isEmpty())
            sdrplayWidget->setObjectName(QStringLiteral("sdrplayWidget"));
        sdrplayWidget->resize(274, 175);
        frame = new QFrame(sdrplayWidget);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(0, 0, 231, 221));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 140, 101, 21));
        externalGain = new QSpinBox(frame);
        externalGain->setObjectName(QStringLiteral("externalGain"));
        externalGain->setGeometry(QRect(0, 0, 91, 21));
        externalGain->setMaximum(103);
        externalGain->setValue(55);
        f_correction = new QSpinBox(frame);
        f_correction->setObjectName(QStringLiteral("f_correction"));
        f_correction->setGeometry(QRect(0, 20, 91, 21));
        f_correction->setMinimum(-100);
        f_correction->setMaximum(100);
        KhzOffset = new QSpinBox(frame);
        KhzOffset->setObjectName(QStringLiteral("KhzOffset"));
        KhzOffset->setGeometry(QRect(0, 40, 91, 21));
        KhzOffset->setMaximum(1000000);
        label_3 = new QLabel(frame);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(100, 40, 41, 21));
        label_4 = new QLabel(frame);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(100, 20, 51, 21));
        label_5 = new QLabel(frame);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(100, 0, 111, 21));
        statusLabel = new QLabel(frame);
        statusLabel->setObjectName(QStringLiteral("statusLabel"));
        statusLabel->setGeometry(QRect(16, 180, 121, 21));
        rateSelector = new QComboBox(frame);
        rateSelector->setObjectName(QStringLiteral("rateSelector"));
        rateSelector->setGeometry(QRect(0, 60, 121, 29));
        api_version = new QLCDNumber(frame);
        api_version->setObjectName(QStringLiteral("api_version"));
        api_version->setGeometry(QRect(0, 90, 91, 16));
        api_version->setLineWidth(0);
        api_version->setSegmentStyle(QLCDNumber::Flat);

        retranslateUi(sdrplayWidget);

        QMetaObject::connectSlotsByName(sdrplayWidget);
    } // setupUi

    void retranslateUi(QWidget *sdrplayWidget)
    {
        sdrplayWidget->setWindowTitle(QApplication::translate("sdrplayWidget", "Form", 0));
        label->setText(QApplication::translate("sdrplayWidget", "mirics-sdrPlay", 0));
        label_3->setText(QApplication::translate("sdrplayWidget", "KHz", 0));
        label_4->setText(QApplication::translate("sdrplayWidget", "ppm", 0));
        label_5->setText(QApplication::translate("sdrplayWidget", "gain reduction", 0));
        statusLabel->setText(QString());
        rateSelector->clear();
        rateSelector->insertItems(0, QStringList()
         << QApplication::translate("sdrplayWidget", "1536", 0)
         << QApplication::translate("sdrplayWidget", "2000", 0)
         << QApplication::translate("sdrplayWidget", "2500", 0)
         << QApplication::translate("sdrplayWidget", "3000", 0)
         << QApplication::translate("sdrplayWidget", "5000", 0)
         << QApplication::translate("sdrplayWidget", "6000", 0)
         << QApplication::translate("sdrplayWidget", "8000", 0)
        );
    } // retranslateUi

};

namespace Ui {
    class sdrplayWidget: public Ui_sdrplayWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDRPLAY_2D_WIDGET_H
