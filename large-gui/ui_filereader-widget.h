/********************************************************************************
** Form generated from reading UI file 'filereader-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEREADER_2D_WIDGET_H
#define UI_FILEREADER_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_filereaderWidget
{
public:
    QFrame *frame;
    QLabel *label;
    QLabel *nameofFile;

    void setupUi(QWidget *filereaderWidget)
    {
        if (filereaderWidget->objectName().isEmpty())
            filereaderWidget->setObjectName(QStringLiteral("filereaderWidget"));
        filereaderWidget->resize(146, 122);
        frame = new QFrame(filereaderWidget);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(0, 0, 151, 231));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 20, 91, 21));
        nameofFile = new QLabel(frame);
        nameofFile->setObjectName(QStringLiteral("nameofFile"));
        nameofFile->setGeometry(QRect(10, 50, 131, 21));

        retranslateUi(filereaderWidget);

        QMetaObject::connectSlotsByName(filereaderWidget);
    } // setupUi

    void retranslateUi(QWidget *filereaderWidget)
    {
        filereaderWidget->setWindowTitle(QApplication::translate("filereaderWidget", "Form", 0));
        label->setText(QApplication::translate("filereaderWidget", "fileReader", 0));
        nameofFile->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class filereaderWidget: public Ui_filereaderWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEREADER_2D_WIDGET_H
