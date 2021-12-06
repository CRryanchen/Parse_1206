#include "widget.h"
#include "ui_widget.h"
#include "xinshengparse.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->btnStart, &QPushButton::clicked, [=](){
          QString inputString = ui->inputText->toPlainText();

          XinShengParse xs(inputString);
          ui->outputText->append(xs.m_parsedHead);
          ui->outputText->append(xs.m_parsedBody);

    });
}

Widget::~Widget()
{
    delete ui;
}
