#ifndef XINSHENGPARSE_H
#define XINSHENGPARSE_H

#include <QObject>
#include <xinsheng.h>
#include <QByteArray>
#include "qaesencryption.h"

class XinShengParse : public QObject
{
    Q_OBJECT
    QByteArray m_frameData;         // 获取接收的报文数据
    QByteArray m_frameHead;         // 获取报文头数据
    QByteArray m_frameBody;         // 获取报文体数据
    QByteArray m_frameTail;         // 获取报文尾数据
    QByteArray m_key;               // 获取初始密钥
public:
    enum COMMAND_TYPE
    {
        XINSHENG_PROTOCOL_REPORT_SINGLE           = 0x7021,  //单条上报数据
    };

    enum FRAME_TYPE
    {
        XINSHENG_PROTOCOL_REQUEST,                           // 请求报文
        XINSHENG_PROTOCOL_RESPONSE,                          // 响应报文
    };

    QString m_parsedHead;           // 解析后的报文头数据
    QString m_parsedBody;           // 解析后的报文体数据

public:
    XinShengParse(QString &inputData, QObject *parent = 0);
    XinShengParse::COMMAND_TYPE ParseHead(XinShengParse::FRAME_TYPE &frameType);
    QString CheckAbnormalBit(int32_t WarningStatus);
    void ParseSingleReportBody(void);
    void ParseSingleReportRspBody(void);
    uint16_t crc16ForModbus(const QByteArray &data);

signals:

public slots:
};

#endif // XINSHENGPARSE_H
