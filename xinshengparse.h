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
    QByteArray m_defaultKey;        // 获取初始密钥
    QByteArray m_latestKey;         // 7082解析出来的密钥
    bool       m_useDefaultKeyFlag; // 使用初始密钥标志位

    void GenericRsp();
public:
    enum COMMAND_TYPE
    {
        XINSHENG_PROTOCOL_REPORT_SINGLE           = 0x7021,  //单条上报数据
        XINSHENG_PROTOCOL_SET_KEY                 = 0x7082,  //设置密钥
        XINSHENG_PROTOCOL_REMORT_VALVE_CONTROL    = 0x7024,  //远程阀控
        XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE = 0x7025,  //修改总购余额
    };

    enum FRAME_TYPE
    {
        XINSHENG_PROTOCOL_REQUEST,                           // 请求报文
        XINSHENG_PROTOCOL_RESPONSE,                          // 响应报文
    };

    enum TRANS_DIRECTION
    {
        XINSHENG_PROTOCOL_TRANS_DIRECTION_B2P,               // 传输方向为表->平台
        XINSHENG_PROTOCOL_TRANS_DIRECTION_P2B,               // 传输方向为平台->表端
    };

    QString m_parsedHead;           // 解析后的报文头数据
    QString m_parsedBody;           // 解析后的报文体数据

public:
    XinShengParse(QString &inputData, QObject *parent = 0);
    void StartParse(void);
    XinShengParse::COMMAND_TYPE ParseHead(XinShengParse::FRAME_TYPE &frameType, XinShengParse::TRANS_DIRECTION &transDirection);
    QString CheckAbnormalBit(int32_t WarningStatus);
    void ParseSingleReportBody(void);
    void ParseSingleReportRspBody(void);
    void ParseSetKeyBody(void);
    void ParseSetKeyRspBody(void);
    void ParseSetRemoteValveBody(void);
    void ParseSetRemoteValveRspBody(void);
    void ParseSetTotalBalanceBody(void);
    void ParseSetTotalBalanceRspBody(void);

    uint16_t crc16ForModbus(const QByteArray &data);
    QString GetLatestKey(void);
    void SetLatestKey(QByteArray inputKey);
    QByteArray GetParseKey(void);
    void setUseDefaultKey(bool res);

signals:

public slots:
};

#endif // XINSHENGPARSE_H
