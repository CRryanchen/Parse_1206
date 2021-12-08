#ifndef FENGXIANPARSE_H
#define FENGXIANPARSE_H

#include <QObject>
#include "fengxian.h"
#include <QByteArray>
#include <QString>

class FengXianParse : public QObject
{
    Q_OBJECT
    QByteArray m_frameData;                             // 获取接收的报文数据
    QByteArray m_frameHead;                             // 获取报文头数据
    QByteArray m_frameBody;                             // 获取报文体数据
    QByteArray m_frameTail;                             // 获取报文尾数据
    QByteArray m_defaultKey;                            // 获取初始密钥
    QByteArray m_latestKey;                             // 平台下发的新密钥
    bool       m_useDefaultKeyFlag;                     // 使用初始密钥标志位

public:
    enum COMMAND_TYPE
    {
        FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM = 0X32,   // 通讯参数设置
        FENGXIAN_PROTOCOL_SET_FUNCTION = 0X33,              // 功能设置
        FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE = 0X34,       // 业务下发
        FENGXIAN_PROTOCOL_SET_TOTAL_GAS = 0X35,             // 累积量设置
        FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL = 0X36,// 施工开阀
        FENGXIAN_PROTOCOL_COMMUNICATION_END = 0X37,         // 通信结束
        FENGXIAN_PROTOCOL_BUSINESS_REPORT = 0X38,           // 业务上报
        FENGXIAN_PROTOCOL_SET_KEY = 0X39,                   // 更新密钥
        FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL = 0X3C,      // 开关阀
        FENGXIAN_PROTOCOL_EXTEND_SECURITY_FUNCTION = 0X3D,  // 安全功能扩展
        FENGXIAN_PROTOCOL_HEARTBEAT_PACK = 0X40,            // 心跳包
    };

    enum FRAME_TYPE
    {
        FENGXIAN_PROTOCOL_REQUEST,                      // 请求报文
        FENGXIAN_PROTOCOL_RESPONSE,                     // 响应报文
    };

    enum TRANS_DIRECTION
    {
        FENGXIAN_PROTOCOL_TRANS_DIRECTION_B2P,          // 传输方向为表->平台
        FENGXIAN_PROTOCOL_TRANS_DIRECTION_P2B,          // 传输方向为平台->表端
    };

    QString m_parsedHead;                               // 解析后的报文头数据
    QString m_parsedBody;                               // 解析后的报文体数据

public:
    FengXianParse(QString &inputData, QObject *parent = 0);
    void StartParse(void);
    FengXianParse::COMMAND_TYPE ParseHead(FengXianParse::FRAME_TYPE &frameType, FengXianParse::TRANS_DIRECTION &transDirection);
    uint8_t CalcCheckCode(const QByteArray &data);
    void ParseBusinessReportBody(void);
    void ParseBusinessDistributeBody(void);
    void ParseSetCommunicationParamBody(void);
    void ParseSetFunctionBody(void);
    void ParseSetTotalGasBody(void);
    void ParseConstructionValveControlBody(void);
    void ParseComuunicationEndBody(void);
    void ParseRemoteValveControlBody(void);
    void ParseHeartbeatPackBody(void);


private:
    template<typename T>
    QString FormatOutput(const QString &str, const T &data, bool endline = true);
    QString FormatOutput(const QString &str, const uint8_t &year, const uint8_t &month, const uint8_t &day, const uint8_t &hour, const uint8_t &min, const uint8_t &sec);
    QByteArray decrypt(const QByteArray &src, const QByteArray &key);
    uint32_t u32Convert(uint32_t val);
    uint16_t u16Convert(uint16_t val);
    QByteArray GetParseKey(void);

signals:

public slots:
};

#endif // FENGXIANPARSE_H
