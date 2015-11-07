/********************************************************************************
** Form generated from reading UI file 'sdrgui.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDRGUI_H
#define UI_SDRGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include "qwt_plot.h"
#include "qwt_scale_widget.h"
#include "qwt_text_label.h"

QT_BEGIN_NAMESPACE

class Ui_elektorSDR
{
public:
    QPushButton *add_one;
    QPushButton *add_two;
    QPushButton *add_four;
    QPushButton *add_five;
    QPushButton *add_six;
    QPushButton *add_seven;
    QPushButton *add_eight;
    QPushButton *add_nine;
    QPushButton *add_zero;
    QPushButton *dec_50;
    QPushButton *dec_5;
    QPushButton *inc_500;
    QPushButton *inc_50;
    QPushButton *inc_5;
    QLCDNumber *lcd_Frequency;
    QLCDNumber *attenuationLevelDisplay;
    QPushButton *dec_500;
    QPushButton *add_three;
    QPushButton *add_correct;
    QPushButton *add_clear;
    QComboBox *inputModeSelect;
    QComboBox *deviceSelector;
    QLabel *systemindicator;
    QLCDNumber *lcd_fmRate;
    QSlider *spectrumAmplitudeSlider;
    QComboBox *streamOutSelector;
    QLCDNumber *IQBalanceDisplay;
    QPushButton *khzSelector;
    QwtPlot *hfscope;
    QLabel *timeDisplay;
    QPushButton *quitButton;
    QPushButton *dumpButton;
    QLCDNumber *lcd_OutputRate;
    QStackedWidget *decoderWidget;
    QWidget *page;
    QLCDNumber *rdsPiDisplay;
    QLCDNumber *bitErrorRate;
    QLabel *label_6;
    QLabel *label_3;
    QLCDNumber *rdsAFDisplay;
    QLCDNumber *crcErrors;
    QLabel *label;
    QLCDNumber *syncErrors;
    QLabel *stationLabelTextBox;
    QLabel *label_2;
    QLabel *radioTextBox;
    QLabel *rdsSyncLabel;
    QComboBox *fmDeemphasisSelector;
    QScrollBar *fmStereoSlider;
    QComboBox *fmMode;
    QComboBox *fmChannelSelect;
    QComboBox *fmRdsSelector;
    QComboBox *fmDecoder;
    QComboBox *fmView;
    QLabel *speechLabel;
    QLabel *pll_isLocked;
    QLabel *decoderDisplay;
    QComboBox *fmLFcutoff;
    QLCDNumber *balanceDisplay;
    QComboBox *fmFilterSelect;
    QLCDNumber *pilotStrength;
    QLCDNumber *rdsStrength;
    QLCDNumber *noiseStrength;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QComboBox *logging;
    QPushButton *logSaving;
    QSpinBox *fmFilterDegree;
    QLCDNumber *dc_component;
    QLabel *label_11;
    QPushButton *fc_plus;
    QPushButton *fc_minus;
    QLabel *incrementingFlag;
    QPushButton *f_minus;
    QPushButton *f_plus;
    QSpinBox *fm_increment;
    QPushButton *pauseButton;
    QPushButton *HFAverageButton;
    QSpinBox *minimumSelect;
    QSpinBox *maximumSelect;
    QLCDNumber *lcd_inputRate;
    QComboBox *HFplotterView;
    QPushButton *audioDump;
    QSlider *squelchSlider;
    QPushButton *squelchButton;
    QSlider *attenuationSlider;
    QSlider *IQbalanceSlider;
    QSlider *volumeSlider;
    QPushButton *startButton;
    QComboBox *theSelector;

    void setupUi(QDialog *elektorSDR)
    {
        if (elektorSDR->objectName().isEmpty())
            elektorSDR->setObjectName(QStringLiteral("elektorSDR"));
        elektorSDR->resize(874, 489);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        elektorSDR->setPalette(palette);
        QFont font;
        font.setBold(false);
        font.setWeight(50);
        elektorSDR->setFont(font);
        add_one = new QPushButton(elektorSDR);
        add_one->setObjectName(QStringLiteral("add_one"));
        add_one->setGeometry(QRect(310, 220, 40, 31));
        QFont font1;
        font1.setBold(true);
        font1.setItalic(true);
        font1.setWeight(75);
        add_one->setFont(font1);
        add_one->setDefault(false);
        add_one->setFlat(false);
        add_two = new QPushButton(elektorSDR);
        add_two->setObjectName(QStringLiteral("add_two"));
        add_two->setGeometry(QRect(350, 220, 41, 31));
        add_two->setFont(font1);
        add_four = new QPushButton(elektorSDR);
        add_four->setObjectName(QStringLiteral("add_four"));
        add_four->setGeometry(QRect(310, 250, 40, 31));
        add_four->setFont(font1);
        add_five = new QPushButton(elektorSDR);
        add_five->setObjectName(QStringLiteral("add_five"));
        add_five->setGeometry(QRect(350, 250, 40, 31));
        add_five->setFont(font1);
        add_six = new QPushButton(elektorSDR);
        add_six->setObjectName(QStringLiteral("add_six"));
        add_six->setGeometry(QRect(390, 250, 40, 31));
        add_six->setFont(font1);
        add_seven = new QPushButton(elektorSDR);
        add_seven->setObjectName(QStringLiteral("add_seven"));
        add_seven->setGeometry(QRect(310, 280, 40, 31));
        add_seven->setFont(font1);
        add_eight = new QPushButton(elektorSDR);
        add_eight->setObjectName(QStringLiteral("add_eight"));
        add_eight->setGeometry(QRect(350, 280, 40, 31));
        add_eight->setFont(font1);
        add_nine = new QPushButton(elektorSDR);
        add_nine->setObjectName(QStringLiteral("add_nine"));
        add_nine->setGeometry(QRect(390, 280, 40, 31));
        QFont font2;
        font2.setBold(true);
        font2.setItalic(true);
        font2.setUnderline(false);
        font2.setWeight(75);
        add_nine->setFont(font2);
        add_zero = new QPushButton(elektorSDR);
        add_zero->setObjectName(QStringLiteral("add_zero"));
        add_zero->setGeometry(QRect(350, 310, 40, 31));
        add_zero->setFont(font1);
        dec_50 = new QPushButton(elektorSDR);
        dec_50->setObjectName(QStringLiteral("dec_50"));
        dec_50->setGeometry(QRect(260, 280, 51, 31));
        QFont font3;
        font3.setPointSize(8);
        dec_50->setFont(font3);
        dec_5 = new QPushButton(elektorSDR);
        dec_5->setObjectName(QStringLiteral("dec_5"));
        dec_5->setGeometry(QRect(260, 310, 51, 31));
        dec_5->setFont(font3);
        inc_500 = new QPushButton(elektorSDR);
        inc_500->setObjectName(QStringLiteral("inc_500"));
        inc_500->setGeometry(QRect(430, 250, 51, 31));
        inc_500->setFont(font3);
        inc_50 = new QPushButton(elektorSDR);
        inc_50->setObjectName(QStringLiteral("inc_50"));
        inc_50->setGeometry(QRect(430, 280, 50, 31));
        inc_50->setFont(font3);
        inc_5 = new QPushButton(elektorSDR);
        inc_5->setObjectName(QStringLiteral("inc_5"));
        inc_5->setGeometry(QRect(430, 310, 50, 31));
        inc_5->setFont(font3);
        lcd_Frequency = new QLCDNumber(elektorSDR);
        lcd_Frequency->setObjectName(QStringLiteral("lcd_Frequency"));
        lcd_Frequency->setGeometry(QRect(480, 280, 191, 31));
        QPalette palette1;
        QBrush brush1(QColor(255, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Button, brush1);
        QBrush brush2(QColor(255, 255, 0, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette1.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette1.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        lcd_Frequency->setPalette(palette1);
        QFont font4;
        font4.setPointSize(17);
        font4.setBold(true);
        font4.setItalic(true);
        font4.setWeight(75);
        lcd_Frequency->setFont(font4);
        lcd_Frequency->setAutoFillBackground(false);
        lcd_Frequency->setFrameShape(QFrame::NoFrame);
        lcd_Frequency->setFrameShadow(QFrame::Raised);
        lcd_Frequency->setLineWidth(2);
        lcd_Frequency->setDigitCount(10);
        lcd_Frequency->setSegmentStyle(QLCDNumber::Flat);
        attenuationLevelDisplay = new QLCDNumber(elektorSDR);
        attenuationLevelDisplay->setObjectName(QStringLiteral("attenuationLevelDisplay"));
        attenuationLevelDisplay->setGeometry(QRect(750, 240, 41, 21));
        QPalette palette2;
        palette2.setBrush(QPalette::Active, QPalette::Base, brush);
        palette2.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette2.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette2.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette2.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette2.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        attenuationLevelDisplay->setPalette(palette2);
        attenuationLevelDisplay->setAutoFillBackground(false);
        attenuationLevelDisplay->setFrameShape(QFrame::NoFrame);
        attenuationLevelDisplay->setFrameShadow(QFrame::Raised);
        attenuationLevelDisplay->setLineWidth(2);
        attenuationLevelDisplay->setDigitCount(3);
        attenuationLevelDisplay->setSegmentStyle(QLCDNumber::Flat);
        dec_500 = new QPushButton(elektorSDR);
        dec_500->setObjectName(QStringLiteral("dec_500"));
        dec_500->setGeometry(QRect(260, 250, 51, 31));
        dec_500->setFont(font3);
        add_three = new QPushButton(elektorSDR);
        add_three->setObjectName(QStringLiteral("add_three"));
        add_three->setGeometry(QRect(390, 220, 40, 31));
        add_three->setFont(font1);
        add_correct = new QPushButton(elektorSDR);
        add_correct->setObjectName(QStringLiteral("add_correct"));
        add_correct->setGeometry(QRect(260, 220, 51, 31));
        QFont font5;
        font5.setBold(true);
        font5.setWeight(75);
        add_correct->setFont(font5);
        add_clear = new QPushButton(elektorSDR);
        add_clear->setObjectName(QStringLiteral("add_clear"));
        add_clear->setGeometry(QRect(430, 220, 51, 31));
        add_clear->setFont(font5);
        inputModeSelect = new QComboBox(elektorSDR);
        inputModeSelect->setObjectName(QStringLiteral("inputModeSelect"));
        inputModeSelect->setGeometry(QRect(400, 200, 81, 21));
        QFont font6;
        font6.setFamily(QStringLiteral("Andale Mono"));
        font6.setPointSize(8);
        inputModeSelect->setFont(font6);
        deviceSelector = new QComboBox(elektorSDR);
        deviceSelector->setObjectName(QStringLiteral("deviceSelector"));
        deviceSelector->setGeometry(QRect(0, 200, 111, 21));
        QPalette palette3;
        palette3.setBrush(QPalette::Active, QPalette::BrightText, brush);
        palette3.setBrush(QPalette::Active, QPalette::Base, brush);
        palette3.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette3.setBrush(QPalette::Inactive, QPalette::BrightText, brush);
        palette3.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette3.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette3.setBrush(QPalette::Disabled, QPalette::BrightText, brush);
        palette3.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette3.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        deviceSelector->setPalette(palette3);
        deviceSelector->setFont(font6);
        deviceSelector->setAutoFillBackground(true);
        systemindicator = new QLabel(elektorSDR);
        systemindicator->setObjectName(QStringLiteral("systemindicator"));
        systemindicator->setGeometry(QRect(630, 220, 161, 21));
        QFont font7;
        font7.setPointSize(10);
        systemindicator->setFont(font7);
        systemindicator->setFrameShape(QFrame::Panel);
        systemindicator->setFrameShadow(QFrame::Raised);
        systemindicator->setLineWidth(2);
        lcd_fmRate = new QLCDNumber(elektorSDR);
        lcd_fmRate->setObjectName(QStringLiteral("lcd_fmRate"));
        lcd_fmRate->setGeometry(QRect(580, 190, 131, 31));
        QPalette palette4;
        palette4.setBrush(QPalette::Active, QPalette::Base, brush);
        palette4.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette4.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette4.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette4.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette4.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        lcd_fmRate->setPalette(palette4);
        lcd_fmRate->setAutoFillBackground(false);
        lcd_fmRate->setFrameShape(QFrame::NoFrame);
        lcd_fmRate->setFrameShadow(QFrame::Raised);
        lcd_fmRate->setLineWidth(2);
        lcd_fmRate->setDigitCount(7);
        lcd_fmRate->setSegmentStyle(QLCDNumber::Flat);
        spectrumAmplitudeSlider = new QSlider(elektorSDR);
        spectrumAmplitudeSlider->setObjectName(QStringLiteral("spectrumAmplitudeSlider"));
        spectrumAmplitudeSlider->setGeometry(QRect(10, 20, 20, 161));
        QPalette palette5;
        QBrush brush3(QColor(255, 85, 0, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::Button, brush3);
        palette5.setBrush(QPalette::Inactive, QPalette::Button, brush3);
        palette5.setBrush(QPalette::Disabled, QPalette::Button, brush3);
        spectrumAmplitudeSlider->setPalette(palette5);
        spectrumAmplitudeSlider->setMinimum(10);
        spectrumAmplitudeSlider->setMaximum(100);
        spectrumAmplitudeSlider->setSliderPosition(50);
        spectrumAmplitudeSlider->setOrientation(Qt::Vertical);
        spectrumAmplitudeSlider->setTickPosition(QSlider::TicksBothSides);
        spectrumAmplitudeSlider->setTickInterval(20);
        streamOutSelector = new QComboBox(elektorSDR);
        streamOutSelector->setObjectName(QStringLiteral("streamOutSelector"));
        streamOutSelector->setGeometry(QRect(490, 310, 171, 31));
        QFont font8;
        font8.setPointSize(8);
        font8.setBold(false);
        font8.setItalic(true);
        font8.setWeight(50);
        streamOutSelector->setFont(font8);
        IQBalanceDisplay = new QLCDNumber(elektorSDR);
        IQBalanceDisplay->setObjectName(QStringLiteral("IQBalanceDisplay"));
        IQBalanceDisplay->setGeometry(QRect(750, 260, 41, 21));
        QPalette palette6;
        palette6.setBrush(QPalette::Active, QPalette::Base, brush);
        palette6.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette6.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette6.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette6.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette6.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        IQBalanceDisplay->setPalette(palette6);
        IQBalanceDisplay->setAutoFillBackground(false);
        IQBalanceDisplay->setFrameShape(QFrame::NoFrame);
        IQBalanceDisplay->setLineWidth(2);
        IQBalanceDisplay->setDigitCount(3);
        IQBalanceDisplay->setSegmentStyle(QLCDNumber::Flat);
        khzSelector = new QPushButton(elektorSDR);
        khzSelector->setObjectName(QStringLiteral("khzSelector"));
        khzSelector->setGeometry(QRect(310, 310, 41, 31));
        khzSelector->setFont(font5);
        hfscope = new QwtPlot(elektorSDR);
        hfscope->setObjectName(QStringLiteral("hfscope"));
        hfscope->setGeometry(QRect(40, 10, 811, 171));
        hfscope->setAutoFillBackground(false);
        hfscope->setFrameShape(QFrame::Panel);
        hfscope->setFrameShadow(QFrame::Raised);
        hfscope->setLineWidth(3);
        timeDisplay = new QLabel(elektorSDR);
        timeDisplay->setObjectName(QStringLiteral("timeDisplay"));
        timeDisplay->setGeometry(QRect(480, 220, 151, 20));
        timeDisplay->setFrameShape(QFrame::Panel);
        timeDisplay->setFrameShadow(QFrame::Raised);
        timeDisplay->setLineWidth(2);
        quitButton = new QPushButton(elektorSDR);
        quitButton->setObjectName(QStringLiteral("quitButton"));
        quitButton->setGeometry(QRect(790, 260, 51, 21));
        QPalette palette7;
        palette7.setBrush(QPalette::Active, QPalette::Button, brush1);
        palette7.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette7.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        quitButton->setPalette(palette7);
        quitButton->setFont(font5);
        quitButton->setAutoFillBackground(true);
        dumpButton = new QPushButton(elektorSDR);
        dumpButton->setObjectName(QStringLiteral("dumpButton"));
        dumpButton->setGeometry(QRect(210, 200, 91, 21));
        QFont font9;
        font9.setFamily(QStringLiteral("Cantarell"));
        font9.setPointSize(11);
        dumpButton->setFont(font9);
        lcd_OutputRate = new QLCDNumber(elektorSDR);
        lcd_OutputRate->setObjectName(QStringLiteral("lcd_OutputRate"));
        lcd_OutputRate->setGeometry(QRect(480, 190, 101, 31));
        QPalette palette8;
        palette8.setBrush(QPalette::Active, QPalette::Base, brush);
        palette8.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette8.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette8.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette8.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette8.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        lcd_OutputRate->setPalette(palette8);
        lcd_OutputRate->setAutoFillBackground(false);
        lcd_OutputRate->setFrameShape(QFrame::NoFrame);
        lcd_OutputRate->setLineWidth(2);
        lcd_OutputRate->setDigitCount(6);
        lcd_OutputRate->setSegmentStyle(QLCDNumber::Flat);
        decoderWidget = new QStackedWidget(elektorSDR);
        decoderWidget->setObjectName(QStringLiteral("decoderWidget"));
        decoderWidget->setGeometry(QRect(10, 340, 861, 141));
        decoderWidget->setFrameShape(QFrame::Panel);
        decoderWidget->setFrameShadow(QFrame::Raised);
        decoderWidget->setLineWidth(2);
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        rdsPiDisplay = new QLCDNumber(page);
        rdsPiDisplay->setObjectName(QStringLiteral("rdsPiDisplay"));
        rdsPiDisplay->setGeometry(QRect(390, 100, 64, 23));
        QPalette palette9;
        palette9.setBrush(QPalette::Active, QPalette::Base, brush);
        palette9.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette9.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette9.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette9.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette9.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        rdsPiDisplay->setPalette(palette9);
        rdsPiDisplay->setAutoFillBackground(false);
        rdsPiDisplay->setFrameShape(QFrame::NoFrame);
        rdsPiDisplay->setMode(QLCDNumber::Hex);
        rdsPiDisplay->setSegmentStyle(QLCDNumber::Flat);
        bitErrorRate = new QLCDNumber(page);
        bitErrorRate->setObjectName(QStringLiteral("bitErrorRate"));
        bitErrorRate->setGeometry(QRect(590, 100, 64, 23));
        QPalette palette10;
        palette10.setBrush(QPalette::Active, QPalette::Base, brush);
        palette10.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette10.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette10.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette10.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette10.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        bitErrorRate->setPalette(palette10);
        bitErrorRate->setAutoFillBackground(false);
        bitErrorRate->setFrameShape(QFrame::NoFrame);
        bitErrorRate->setSegmentStyle(QLCDNumber::Flat);
        label_6 = new QLabel(page);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(320, 100, 51, 17));
        label_3 = new QLabel(page);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(480, 100, 91, 20));
        rdsAFDisplay = new QLCDNumber(page);
        rdsAFDisplay->setObjectName(QStringLiteral("rdsAFDisplay"));
        rdsAFDisplay->setGeometry(QRect(230, 100, 64, 23));
        QPalette palette11;
        palette11.setBrush(QPalette::Active, QPalette::Base, brush);
        palette11.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette11.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette11.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette11.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette11.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        rdsAFDisplay->setPalette(palette11);
        rdsAFDisplay->setAutoFillBackground(false);
        rdsAFDisplay->setFrameShape(QFrame::NoFrame);
        rdsAFDisplay->setSegmentStyle(QLCDNumber::Flat);
        crcErrors = new QLCDNumber(page);
        crcErrors->setObjectName(QStringLiteral("crcErrors"));
        crcErrors->setGeometry(QRect(620, 40, 64, 23));
        QPalette palette12;
        palette12.setBrush(QPalette::Active, QPalette::Base, brush);
        palette12.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette12.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette12.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette12.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette12.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        crcErrors->setPalette(palette12);
        crcErrors->setAutoFillBackground(false);
        crcErrors->setFrameShape(QFrame::NoFrame);
        crcErrors->setSegmentStyle(QLCDNumber::Flat);
        label = new QLabel(page);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(550, 40, 81, 20));
        syncErrors = new QLCDNumber(page);
        syncErrors->setObjectName(QStringLiteral("syncErrors"));
        syncErrors->setGeometry(QRect(100, 100, 64, 23));
        QPalette palette13;
        palette13.setBrush(QPalette::Active, QPalette::Base, brush);
        palette13.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette13.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette13.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette13.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette13.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        syncErrors->setPalette(palette13);
        syncErrors->setAutoFillBackground(false);
        syncErrors->setFrameShape(QFrame::NoFrame);
        syncErrors->setSegmentStyle(QLCDNumber::Flat);
        stationLabelTextBox = new QLabel(page);
        stationLabelTextBox->setObjectName(QStringLiteral("stationLabelTextBox"));
        stationLabelTextBox->setGeometry(QRect(420, 70, 161, 21));
        stationLabelTextBox->setFrameShape(QFrame::Panel);
        stationLabelTextBox->setFrameShadow(QFrame::Raised);
        stationLabelTextBox->setLineWidth(2);
        label_2 = new QLabel(page);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 100, 81, 20));
        radioTextBox = new QLabel(page);
        radioTextBox->setObjectName(QStringLiteral("radioTextBox"));
        radioTextBox->setGeometry(QRect(70, 70, 351, 21));
        radioTextBox->setFrameShape(QFrame::Panel);
        radioTextBox->setFrameShadow(QFrame::Raised);
        rdsSyncLabel = new QLabel(page);
        rdsSyncLabel->setObjectName(QStringLiteral("rdsSyncLabel"));
        rdsSyncLabel->setGeometry(QRect(120, 40, 41, 21));
        rdsSyncLabel->setFrameShape(QFrame::Panel);
        rdsSyncLabel->setFrameShadow(QFrame::Raised);
        rdsSyncLabel->setLineWidth(2);
        fmDeemphasisSelector = new QComboBox(page);
        fmDeemphasisSelector->setObjectName(QStringLiteral("fmDeemphasisSelector"));
        fmDeemphasisSelector->setGeometry(QRect(550, 10, 51, 21));
        fmStereoSlider = new QScrollBar(page);
        fmStereoSlider->setObjectName(QStringLiteral("fmStereoSlider"));
        fmStereoSlider->setGeometry(QRect(280, 40, 211, 21));
        fmStereoSlider->setMinimum(-50);
        fmStereoSlider->setMaximum(50);
        fmStereoSlider->setValue(0);
        fmStereoSlider->setOrientation(Qt::Horizontal);
        fmMode = new QComboBox(page);
        fmMode->setObjectName(QStringLiteral("fmMode"));
        fmMode->setGeometry(QRect(300, 10, 71, 21));
        fmChannelSelect = new QComboBox(page);
        fmChannelSelect->setObjectName(QStringLiteral("fmChannelSelect"));
        fmChannelSelect->setGeometry(QRect(370, 10, 85, 21));
        fmRdsSelector = new QComboBox(page);
        fmRdsSelector->setObjectName(QStringLiteral("fmRdsSelector"));
        fmRdsSelector->setGeometry(QRect(50, 40, 71, 21));
        fmDecoder = new QComboBox(page);
        fmDecoder->setObjectName(QStringLiteral("fmDecoder"));
        fmDecoder->setGeometry(QRect(70, 10, 111, 21));
        fmView = new QComboBox(page);
        fmView->setObjectName(QStringLiteral("fmView"));
        fmView->setGeometry(QRect(460, 10, 85, 21));
        speechLabel = new QLabel(page);
        speechLabel->setObjectName(QStringLiteral("speechLabel"));
        speechLabel->setGeometry(QRect(160, 40, 71, 21));
        speechLabel->setFrameShape(QFrame::Panel);
        speechLabel->setFrameShadow(QFrame::Raised);
        speechLabel->setLineWidth(2);
        pll_isLocked = new QLabel(page);
        pll_isLocked->setObjectName(QStringLiteral("pll_isLocked"));
        pll_isLocked->setGeometry(QRect(230, 40, 51, 21));
        pll_isLocked->setFrameShape(QFrame::Panel);
        pll_isLocked->setFrameShadow(QFrame::Raised);
        pll_isLocked->setLineWidth(2);
        decoderDisplay = new QLabel(page);
        decoderDisplay->setObjectName(QStringLiteral("decoderDisplay"));
        decoderDisplay->setGeometry(QRect(180, 10, 121, 20));
        QFont font10;
        font10.setPointSize(9);
        decoderDisplay->setFont(font10);
        decoderDisplay->setFrameShape(QFrame::Panel);
        decoderDisplay->setFrameShadow(QFrame::Raised);
        decoderDisplay->setLineWidth(2);
        fmLFcutoff = new QComboBox(page);
        fmLFcutoff->setObjectName(QStringLiteral("fmLFcutoff"));
        fmLFcutoff->setGeometry(QRect(604, 10, 71, 21));
        balanceDisplay = new QLCDNumber(page);
        balanceDisplay->setObjectName(QStringLiteral("balanceDisplay"));
        balanceDisplay->setGeometry(QRect(490, 40, 51, 21));
        QPalette palette14;
        palette14.setBrush(QPalette::Active, QPalette::Base, brush);
        palette14.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette14.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette14.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette14.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette14.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        balanceDisplay->setPalette(palette14);
        balanceDisplay->setAutoFillBackground(false);
        balanceDisplay->setLineWidth(2);
        balanceDisplay->setDigitCount(3);
        balanceDisplay->setSegmentStyle(QLCDNumber::Flat);
        fmFilterSelect = new QComboBox(page);
        fmFilterSelect->setObjectName(QStringLiteral("fmFilterSelect"));
        fmFilterSelect->setGeometry(QRect(10, 10, 61, 21));
        pilotStrength = new QLCDNumber(page);
        pilotStrength->setObjectName(QStringLiteral("pilotStrength"));
        pilotStrength->setGeometry(QRect(780, 40, 64, 23));
        QPalette palette15;
        palette15.setBrush(QPalette::Active, QPalette::Base, brush);
        palette15.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette15.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette15.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette15.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette15.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        pilotStrength->setPalette(palette15);
        pilotStrength->setAutoFillBackground(false);
        pilotStrength->setFrameShape(QFrame::NoFrame);
        pilotStrength->setLineWidth(2);
        pilotStrength->setSegmentStyle(QLCDNumber::Flat);
        rdsStrength = new QLCDNumber(page);
        rdsStrength->setObjectName(QStringLiteral("rdsStrength"));
        rdsStrength->setGeometry(QRect(780, 70, 64, 23));
        QPalette palette16;
        palette16.setBrush(QPalette::Active, QPalette::Base, brush);
        palette16.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette16.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette16.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette16.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette16.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        rdsStrength->setPalette(palette16);
        rdsStrength->setAutoFillBackground(false);
        rdsStrength->setFrameShape(QFrame::NoFrame);
        rdsStrength->setLineWidth(2);
        rdsStrength->setSegmentStyle(QLCDNumber::Flat);
        noiseStrength = new QLCDNumber(page);
        noiseStrength->setObjectName(QStringLiteral("noiseStrength"));
        noiseStrength->setGeometry(QRect(780, 100, 64, 23));
        QPalette palette17;
        palette17.setBrush(QPalette::Active, QPalette::Base, brush);
        palette17.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette17.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette17.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette17.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette17.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        noiseStrength->setPalette(palette17);
        noiseStrength->setAutoFillBackground(false);
        noiseStrength->setFrameShape(QFrame::NoFrame);
        noiseStrength->setLineWidth(2);
        noiseStrength->setSegmentStyle(QLCDNumber::Flat);
        label_8 = new QLabel(page);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(690, 40, 81, 17));
        label_9 = new QLabel(page);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(690, 70, 81, 17));
        label_10 = new QLabel(page);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(686, 100, 81, 20));
        logging = new QComboBox(page);
        logging->setObjectName(QStringLiteral("logging"));
        logging->setGeometry(QRect(680, 10, 81, 21));
        logSaving = new QPushButton(page);
        logSaving->setObjectName(QStringLiteral("logSaving"));
        logSaving->setGeometry(QRect(780, 10, 61, 21));
        fmFilterDegree = new QSpinBox(page);
        fmFilterDegree->setObjectName(QStringLiteral("fmFilterDegree"));
        fmFilterDegree->setGeometry(QRect(10, 70, 59, 21));
        fmFilterDegree->setMinimum(1);
        fmFilterDegree->setValue(15);
        dc_component = new QLCDNumber(page);
        dc_component->setObjectName(QStringLiteral("dc_component"));
        dc_component->setGeometry(QRect(620, 70, 64, 23));
        QPalette palette18;
        palette18.setBrush(QPalette::Active, QPalette::Base, brush);
        palette18.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette18.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette18.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette18.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette18.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        dc_component->setPalette(palette18);
        dc_component->setAutoFillBackground(false);
        dc_component->setFrameShape(QFrame::NoFrame);
        dc_component->setSegmentStyle(QLCDNumber::Flat);
        label_11 = new QLabel(page);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setGeometry(QRect(186, 100, 41, 21));
        decoderWidget->addWidget(page);
        fc_plus = new QPushButton(elektorSDR);
        fc_plus->setObjectName(QStringLiteral("fc_plus"));
        fc_plus->setGeometry(QRect(140, 220, 51, 21));
        fc_minus = new QPushButton(elektorSDR);
        fc_minus->setObjectName(QStringLiteral("fc_minus"));
        fc_minus->setGeometry(QRect(60, 220, 51, 21));
        incrementingFlag = new QLabel(elektorSDR);
        incrementingFlag->setObjectName(QStringLiteral("incrementingFlag"));
        incrementingFlag->setGeometry(QRect(110, 220, 31, 21));
        incrementingFlag->setFrameShape(QFrame::Box);
        incrementingFlag->setLineWidth(2);
        f_minus = new QPushButton(elektorSDR);
        f_minus->setObjectName(QStringLiteral("f_minus"));
        f_minus->setGeometry(QRect(0, 220, 61, 21));
        f_plus = new QPushButton(elektorSDR);
        f_plus->setObjectName(QStringLiteral("f_plus"));
        f_plus->setGeometry(QRect(190, 220, 71, 21));
        fm_increment = new QSpinBox(elektorSDR);
        fm_increment->setObjectName(QStringLiteral("fm_increment"));
        fm_increment->setGeometry(QRect(60, 240, 61, 21));
        fm_increment->setMaximum(1000);
        fm_increment->setSingleStep(100);
        fm_increment->setValue(100);
        pauseButton = new QPushButton(elektorSDR);
        pauseButton->setObjectName(QStringLiteral("pauseButton"));
        pauseButton->setGeometry(QRect(790, 280, 51, 31));
        HFAverageButton = new QPushButton(elektorSDR);
        HFAverageButton->setObjectName(QStringLiteral("HFAverageButton"));
        HFAverageButton->setGeometry(QRect(110, 200, 101, 21));
        HFAverageButton->setFont(font10);
        minimumSelect = new QSpinBox(elektorSDR);
        minimumSelect->setObjectName(QStringLiteral("minimumSelect"));
        minimumSelect->setGeometry(QRect(0, 240, 61, 21));
        minimumSelect->setFont(font10);
        minimumSelect->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        minimumSelect->setMaximum(1700);
        minimumSelect->setValue(86);
        maximumSelect = new QSpinBox(elektorSDR);
        maximumSelect->setObjectName(QStringLiteral("maximumSelect"));
        maximumSelect->setGeometry(QRect(120, 240, 61, 21));
        maximumSelect->setFont(font10);
        maximumSelect->setMaximum(1700);
        maximumSelect->setValue(110);
        lcd_inputRate = new QLCDNumber(elektorSDR);
        lcd_inputRate->setObjectName(QStringLiteral("lcd_inputRate"));
        lcd_inputRate->setGeometry(QRect(710, 190, 131, 31));
        QPalette palette19;
        palette19.setBrush(QPalette::Active, QPalette::Base, brush);
        palette19.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette19.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette19.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        palette19.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette19.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        lcd_inputRate->setPalette(palette19);
        lcd_inputRate->setAutoFillBackground(false);
        lcd_inputRate->setFrameShape(QFrame::NoFrame);
        lcd_inputRate->setLineWidth(2);
        lcd_inputRate->setDigitCount(8);
        lcd_inputRate->setSegmentStyle(QLCDNumber::Flat);
        HFplotterView = new QComboBox(elektorSDR);
        HFplotterView->setObjectName(QStringLiteral("HFplotterView"));
        HFplotterView->setGeometry(QRect(300, 200, 101, 21));
        HFplotterView->setFont(font10);
        audioDump = new QPushButton(elektorSDR);
        audioDump->setObjectName(QStringLiteral("audioDump"));
        audioDump->setGeometry(QRect(680, 280, 111, 31));
        squelchSlider = new QSlider(elektorSDR);
        squelchSlider->setObjectName(QStringLiteral("squelchSlider"));
        squelchSlider->setGeometry(QRect(0, 290, 191, 20));
        squelchSlider->setSliderPosition(50);
        squelchSlider->setOrientation(Qt::Horizontal);
        squelchButton = new QPushButton(elektorSDR);
        squelchButton->setObjectName(QStringLiteral("squelchButton"));
        squelchButton->setGeometry(QRect(190, 280, 71, 31));
        attenuationSlider = new QSlider(elektorSDR);
        attenuationSlider->setObjectName(QStringLiteral("attenuationSlider"));
        attenuationSlider->setGeometry(QRect(480, 240, 271, 19));
        QPalette palette20;
        palette20.setBrush(QPalette::Active, QPalette::WindowText, brush1);
        palette20.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
        QBrush brush4(QColor(143, 146, 147, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette20.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        attenuationSlider->setPalette(palette20);
        attenuationSlider->setAutoFillBackground(true);
        attenuationSlider->setMinimum(1);
        attenuationSlider->setMaximum(100);
        attenuationSlider->setOrientation(Qt::Horizontal);
        IQbalanceSlider = new QSlider(elektorSDR);
        IQbalanceSlider->setObjectName(QStringLiteral("IQbalanceSlider"));
        IQbalanceSlider->setGeometry(QRect(480, 260, 271, 19));
        IQbalanceSlider->setMinimum(-35);
        IQbalanceSlider->setMaximum(35);
        IQbalanceSlider->setOrientation(Qt::Horizontal);
        volumeSlider = new QSlider(elektorSDR);
        volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
        volumeSlider->setGeometry(QRect(670, 310, 171, 20));
        volumeSlider->setOrientation(Qt::Horizontal);
        startButton = new QPushButton(elektorSDR);
        startButton->setObjectName(QStringLiteral("startButton"));
        startButton->setGeometry(QRect(790, 220, 51, 41));
        startButton->setFont(font5);
        startButton->setAutoFillBackground(true);
        theSelector = new QComboBox(elektorSDR);
        theSelector->setObjectName(QStringLiteral("theSelector"));
        theSelector->setGeometry(QRect(0, 260, 181, 29));

        retranslateUi(elektorSDR);

        decoderWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(elektorSDR);
    } // setupUi

    void retranslateUi(QDialog *elektorSDR)
    {
        elektorSDR->setWindowTitle(QApplication::translate("elektorSDR", "fm-receiver", 0));
        elektorSDR->setWindowIconText(QApplication::translate("elektorSDR", "QUIT", 0));
        add_one->setText(QApplication::translate("elektorSDR", "1", 0));
        add_two->setText(QApplication::translate("elektorSDR", "2", 0));
        add_four->setText(QApplication::translate("elektorSDR", "4", 0));
        add_five->setText(QApplication::translate("elektorSDR", "5", 0));
        add_six->setText(QApplication::translate("elektorSDR", "6", 0));
        add_seven->setText(QApplication::translate("elektorSDR", "7", 0));
        add_eight->setText(QApplication::translate("elektorSDR", "8", 0));
        add_nine->setText(QApplication::translate("elektorSDR", "9", 0));
        add_zero->setText(QApplication::translate("elektorSDR", "0", 0));
        dec_50->setText(QApplication::translate("elektorSDR", "--", 0));
        dec_5->setText(QApplication::translate("elektorSDR", "-", 0));
        inc_500->setText(QApplication::translate("elektorSDR", "+++", 0));
        inc_50->setText(QApplication::translate("elektorSDR", "++", 0));
        inc_5->setText(QApplication::translate("elektorSDR", "+", 0));
        dec_500->setText(QApplication::translate("elektorSDR", "---", 0));
        add_three->setText(QApplication::translate("elektorSDR", "3", 0));
        add_correct->setText(QApplication::translate("elektorSDR", "Corr", 0));
        add_clear->setText(QApplication::translate("elektorSDR", "Clear", 0));
        inputModeSelect->clear();
        inputModeSelect->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "I and Q", 0)
         << QApplication::translate("elektorSDR", "I Only", 0)
         << QApplication::translate("elektorSDR", "Q and I", 0)
         << QApplication::translate("elektorSDR", "Q Only", 0)
         << QApplication::translate("elektorSDR", "FSK test", 0)
        );
        deviceSelector->clear();
        deviceSelector->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "no device", 0)
         << QApplication::translate("elektorSDR", "filereader", 0)
        );
        systemindicator->setText(QApplication::translate("elektorSDR", "JFF-ESDR V2.0 Portaudio", 0));
        streamOutSelector->clear();
        streamOutSelector->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "select output", 0)
        );
        khzSelector->setText(QApplication::translate("elektorSDR", "khz", 0));
        timeDisplay->setText(QApplication::translate("elektorSDR", "TextLabel", 0));
        quitButton->setText(QApplication::translate("elektorSDR", "QUIT", 0));
        dumpButton->setText(QApplication::translate("elektorSDR", "inputDump", 0));
        label_6->setText(QApplication::translate("elektorSDR", "PiCode", 0));
        label_3->setText(QApplication::translate("elektorSDR", "biterror rate", 0));
        label->setText(QApplication::translate("elektorSDR", "CRC errors", 0));
        stationLabelTextBox->setText(QApplication::translate("elektorSDR", "TextLabel", 0));
        label_2->setText(QApplication::translate("elektorSDR", "Sync errors", 0));
        radioTextBox->setText(QApplication::translate("elektorSDR", "TextLabel", 0));
        rdsSyncLabel->setText(QString());
        fmDeemphasisSelector->clear();
        fmDeemphasisSelector->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "50", 0)
         << QApplication::translate("elektorSDR", "none", 0)
         << QApplication::translate("elektorSDR", "75", 0)
        );
        fmMode->clear();
        fmMode->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "stereo", 0)
         << QApplication::translate("elektorSDR", "mono", 0)
        );
        fmChannelSelect->clear();
        fmChannelSelect->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "stereo", 0)
         << QApplication::translate("elektorSDR", "Left", 0)
         << QApplication::translate("elektorSDR", "Right", 0)
         << QApplication::translate("elektorSDR", "Left+Right", 0)
         << QApplication::translate("elektorSDR", "Left-Right", 0)
        );
        fmRdsSelector->clear();
        fmRdsSelector->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "no rds", 0)
         << QApplication::translate("elektorSDR", "rds 1", 0)
         << QApplication::translate("elektorSDR", "rds 2", 0)
        );
        fmDecoder->clear();
        fmDecoder->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "fm4decoder", 0)
         << QApplication::translate("elektorSDR", "fm1decoder", 0)
         << QApplication::translate("elektorSDR", "fm2decoder", 0)
         << QApplication::translate("elektorSDR", "fm3decoder", 0)
         << QApplication::translate("elektorSDR", "fm5decoder", 0)
        );
        fmView->clear();
        fmView->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "view 1", 0)
         << QApplication::translate("elektorSDR", "view 2", 0)
         << QApplication::translate("elektorSDR", "view 3", 0)
         << QApplication::translate("elektorSDR", "view 4", 0)
         << QApplication::translate("elektorSDR", "view 5", 0)
         << QApplication::translate("elektorSDR", "view 6", 0)
         << QApplication::translate("elektorSDR", "view 7", 0)
         << QApplication::translate("elektorSDR", "view 8", 0)
        );
        speechLabel->setText(QString());
        pll_isLocked->setText(QApplication::translate("elektorSDR", "pilot", 0));
        decoderDisplay->setText(QString());
        fmLFcutoff->clear();
        fmLFcutoff->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "12000", 0)
         << QApplication::translate("elektorSDR", "15000", 0)
         << QApplication::translate("elektorSDR", "9600", 0)
         << QApplication::translate("elektorSDR", "6000", 0)
         << QApplication::translate("elektorSDR", "20000", 0)
         << QApplication::translate("elektorSDR", "25000", 0)
         << QApplication::translate("elektorSDR", "off", 0)
        );
        fmFilterSelect->clear();
        fmFilterSelect->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "---", 0)
         << QApplication::translate("elektorSDR", "65", 0)
         << QApplication::translate("elektorSDR", "75", 0)
         << QApplication::translate("elektorSDR", "90", 0)
         << QApplication::translate("elektorSDR", "115", 0)
         << QApplication::translate("elektorSDR", "140", 0)
         << QApplication::translate("elektorSDR", "165", 0)
         << QApplication::translate("elektorSDR", "190", 0)
        );
        label_8->setText(QApplication::translate("elektorSDR", "Pilotstrength", 0));
        label_9->setText(QApplication::translate("elektorSDR", "rdsstrength", 0));
        label_10->setText(QApplication::translate("elektorSDR", "noisestrength", 0));
        logging->clear();
        logging->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "log off", 0)
         << QApplication::translate("elektorSDR", "log 1 sec", 0)
         << QApplication::translate("elektorSDR", "log 2 sec", 0)
         << QApplication::translate("elektorSDR", "log 3 sec", 0)
         << QApplication::translate("elektorSDR", "log 4 sec", 0)
         << QApplication::translate("elektorSDR", "log 5 sec", 0)
        );
        logSaving->setText(QApplication::translate("elektorSDR", "save", 0));
        label_11->setText(QApplication::translate("elektorSDR", "AF", 0));
        fc_plus->setText(QApplication::translate("elektorSDR", "fc+", 0));
        fc_minus->setText(QApplication::translate("elektorSDR", "fc-", 0));
        incrementingFlag->setText(QString());
        f_minus->setText(QApplication::translate("elektorSDR", "f-", 0));
        f_plus->setText(QApplication::translate("elektorSDR", "f+", 0));
        pauseButton->setText(QApplication::translate("elektorSDR", "Pause", 0));
        HFAverageButton->setText(QApplication::translate("elektorSDR", "HF Freeze", 0));
        HFplotterView->clear();
        HFplotterView->insertItems(0, QStringList()
         << QApplication::translate("elektorSDR", "spectrumview", 0)
         << QApplication::translate("elektorSDR", "waterfall", 0)
        );
        audioDump->setText(QApplication::translate("elektorSDR", "audioDump", 0));
        squelchButton->setText(QApplication::translate("elektorSDR", "squelchOff", 0));
        startButton->setText(QApplication::translate("elektorSDR", "START", 0));
    } // retranslateUi

};

namespace Ui {
    class elektorSDR: public Ui_elektorSDR {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDRGUI_H
