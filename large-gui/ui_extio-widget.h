/********************************************************************************
** Form generated from reading UI file 'extio-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXTIO_2D_WIDGET_H
#define UI_EXTIO_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_extioWidget
{
public:
    QFrame *frame;
    QLabel *label;
    QLabel *nameofdll;
    QLabel *status;
    QComboBox *theSelector;

    void setupUi(QWidget *extioWidget)
    {
        if (extioWidget->objectName().isEmpty())
            extioWidget->setObjectName(QStringLiteral("extioWidget"));
        extioWidget->resize(148, 170);
        frame = new QFrame(extioWidget);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(0, 0, 151, 231));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 100, 121, 21));
        nameofdll = new QLabel(frame);
        nameofdll->setObjectName(QStringLiteral("nameofdll"));
        nameofdll->setGeometry(QRect(10, 80, 131, 21));
        status = new QLabel(frame);
        status->setObjectName(QStringLiteral("status"));
        status->setGeometry(QRect(10, 40, 121, 31));
        theSelector = new QComboBox(frame);
        theSelector->setObjectName(QStringLiteral("theSelector"));
        theSelector->setGeometry(QRect(10, 10, 111, 29));

        retranslateUi(extioWidget);

        QMetaObject::connectSlotsByName(extioWidget);
    } // setupUi

    void retranslateUi(QWidget *extioWidget)
    {
        extioWidget->setWindowTitle(QApplication::translate("extioWidget", "Form", 0));
        label->setText(QApplication::translate("extioWidget", "extIO handler", 0));
        nameofdll->setText(QString());
        status->setText(QApplication::translate("extioWidget", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class extioWidget: public Ui_extioWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXTIO_2D_WIDGET_H
