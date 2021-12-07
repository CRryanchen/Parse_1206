#include "widget.h"
#include "ui_widget.h"
#include "xinshengparse.h"
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->btnStart, &QPushButton::clicked, [=](){
          QString inputString = ui->inputText->toPlainText();
          XinShengParse xs(inputString);

          ui->outputText->clear();
          if (ui->lineEdit->text() != NULL)
          {
              // 则使用lineEdit里面的数据作为密钥
              xs.setUseDefaultKey(false);
              xs.SetLatestKey(QByteArray::fromHex(ui->lineEdit->text().toLocal8Bit()));
              qDebug() << ui->lineEdit->text();
              qDebug() << "我要使用新密钥";
          }

          xs.StartParse();
          ui->outputText->append(xs.m_parsedHead);
          ui->outputText->append(xs.m_parsedBody);
          ui->lineEdit->setText(xs.GetLatestKey());
    });
}

Widget::~Widget()
{
    delete ui;
}
