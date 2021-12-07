#include "xinshengparse.h"
#include <QDebug>

#define XINSHENG_PROTOCOL_FRAME_STARTCHAR           0X68        // 新圣协议起始符
#define XINSHENG_PROTOCOL_FRAME_ENDCHAR             0X16        // 新圣协议结束符
#define XINSHENG_PROTOCOL_FRAME_BODY_LENGTH(x, y)      (sizeof(x), (sizeof(y) + PADDING_LENGTH(y)))

static const quint16 crc16Table[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

template<typename T>
QString FormatOutput(const QString &str, const T &data, bool endline = false)
{
    if (str.size() >= 6)
    {
        return QString().sprintf("%s\t：%0*x", str.toUtf8().data(), sizeof(data) * 2, data) + QString(endline ? "\n" : "");
    }
    else
    {
        return QString().sprintf("%s\t\t：%0*x", str.toUtf8().data(), sizeof(data) * 2, data) + QString(endline ? "\n" : "");
    }
}

QString FormatOutput(const QString &str, const uint8_t &data1, const uint8_t &data2)
{
    return QString().sprintf("%s\t\t：%0*x%0*x\n",str.toUtf8().data(), sizeof(data1) * 2, data1, sizeof(data2) * 2, data2);
}

QString FormatOutput(const QString &str, const uint8_t &data1, const uint8_t &data2, const uint8_t &data3, const uint8_t &data4)
{
    return QString().sprintf("%s\t\t：%0*x%0*x%0*x%0*x(V%c%c.%c%c)\n", str.toUtf8().data(), sizeof(data1) * 2, data1, sizeof(data2) * 2, data2, sizeof(data3) * 2, data3, sizeof(data4) * 2, data4, data1, data2, data3, data4);
}

QString FormatOutput(const QString &str, const uint8_t &year, const uint8_t &month, const uint8_t &day, const uint8_t &hour, const uint8_t &min, const uint8_t &sec)
{
    if (str.size() >= 6)
    {
        return QString().sprintf("%s\t：%0*x/%0*x/%0*x %0*x:%0*x:%0*x\n", str.toUtf8().data(), sizeof(year) * 2, year, sizeof(month) * 2, month, sizeof(day) * 2, day, sizeof(hour) * 2, hour, sizeof(min) * 2, min, sizeof(sec) * 2, sec);
    }
    else
    {
        return QString().sprintf("%s\t\t：%0*x/%0*x/%0*x %0*x:%0*x:%0*x\n", str.toUtf8().data(), sizeof(year) * 2, year, sizeof(month) * 2, month, sizeof(day) * 2, day, sizeof(hour) * 2, hour, sizeof(min) * 2, min, sizeof(sec) * 2, sec);
    }

}

XinShengParse::XinShengParse(QString &inputData, QObject *parent) : QObject(parent)
{
    this->m_frameData = QByteArray::fromHex(inputData.toLocal8Bit());
    this->m_frameHead = this->m_frameData.left(sizeof(XINSHENG_PROTOCOL_FRAME_HEADER));
    this->m_frameTail = this->m_frameData.right(sizeof(XINSHENG_PROTOCOL_FRAME_TAIL));

    this->m_useDefaultKeyFlag = true;   // 使用初始密钥

}

void XinShengParse::StartParse()
{
    // 校验协议头
    XINSHENG_PROTOCOL_FRAME_HEADER head;
    memcpy((uint8_t *)&head.StartChar, (uint8_t *)this->m_frameHead.data(), this->m_frameHead.size());
    // 判断起始符
    if (head.StartChar == XINSHENG_PROTOCOL_FRAME_STARTCHAR)
    {
        // 起始符正确
        qDebug() << "起始符正确!";
    }
    else
    {
        // 起始符错误
        qDebug() << "起始符错误!";
    }

    FRAME_TYPE frameType;
    TRANS_DIRECTION transDirection;
    XinShengParse::COMMAND_TYPE commandType = this->ParseHead(frameType, transDirection);

    // 校验协议尾
    XINSHENG_PROTOCOL_FRAME_TAIL tail;
    memcpy((uint8_t *)&tail.CRCCheck, (uint8_t *)this->m_frameTail.data(), this->m_frameTail.size());
    uint16_t crc = tail.CRCCheck;
    if (transDirection == XINSHENG_PROTOCOL_TRANS_DIRECTION_B2P)
    {
        crc = (tail.CRCCheck & 0xff) << 8;
        crc |= tail.CRCCheck >> 8;
    }


    // 需要校验的数据
    QByteArray needCRCData = this->m_frameData;
    needCRCData.chop(sizeof(XINSHENG_PROTOCOL_FRAME_TAIL));
    qDebug() << crc;
    qDebug() << crc16ForModbus(needCRCData);
    if (crc == crc16ForModbus(needCRCData))
    {
        // 校验码正确
        qDebug() << "校验码正确!";
    }
    else
    {
        // 校验码错误
        qDebug() << "校验码错误!";
    }
    // 判断结束符
    if (tail.EndChar == XINSHENG_PROTOCOL_FRAME_ENDCHAR)
    {
        // 结束符正确
        qDebug() << "结束符正确!";
    }
    else
    {
        // 结束符错误
        qDebug() << "结束符错误!";
    }


    switch(commandType)
    {
        case XinShengParse::XINSHENG_PROTOCOL_REPORT_SINGLE:
            if (frameType == XINSHENG_PROTOCOL_REQUEST)
            {
                qDebug() << "This is 7021 to PT";
                this->m_frameBody = this->m_frameData.mid(sizeof(XINSHENG_PROTOCOL_FRAME_HEADER), sizeof(XINSHENG_PROTOCOL_REPORT_SINGLE_DATA) + PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_DATA));
//                this->m_frameBody = this->m_frameData.mid(XINSHENG_PROTOCOL_FRAME_BODY_LENGTH(XINSHENG_PROTOCOL_FRAME_HEADER, XINSHENG_PROTOCOL_REPORT_SINGLE_DATA));
                this->ParseSingleReportBody();
            }
            else
            {
                qDebug() << "This is 7021 rsp from PT";
                this->m_frameBody = this->m_frameData.mid(sizeof(XINSHENG_PROTOCOL_FRAME_HEADER), sizeof(XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA) + PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA));
//                this->m_frameBody = this->m_frameData.mid(XINSHENG_PROTOCOL_FRAME_BODY_LENGTH(XINSHENG_PROTOCOL_FRAME_HEADER, XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA));
                this->ParseSingleReportRspBody();
            }

        break;

        case XinShengParse::XINSHENG_PROTOCOL_SET_KEY:
            if (frameType == XINSHENG_PROTOCOL_REQUEST)
            {
                qDebug() << "This is 7082 from PT";
                this->m_frameBody = this->m_frameData.mid(sizeof(XINSHENG_PROTOCOL_FRAME_HEADER), sizeof(XINSHENG_PROTOCOL_SET_KEY_DATA) + PADDING_LENGTH(XINSHENG_PROTOCOL_SET_KEY_DATA));
//                this->m_frameBody = this->m_frameData.mid(XINSHENG_PROTOCOL_FRAME_BODY_LENGTH(XINSHENG_PROTOCOL_FRAME_HEADER, XINSHENG_PROTOCOL_SET_KEY_DATA));
                this->ParseSetKey();
            }
            else
            {
                qDebug() << "This is 7082 rsp from BD";
            }
        break;

        default:
            qDebug() << "未识别的命令码";
        break;
    }
}

XinShengParse::COMMAND_TYPE XinShengParse::ParseHead(XinShengParse::FRAME_TYPE &frameType, XinShengParse::TRANS_DIRECTION &transDirection)
{
    QString temp = 0;

    this->m_defaultKey = "QW";
    qDebug() << "设置初始密钥";


    XINSHENG_PROTOCOL_FRAME_HEADER head;
    memcpy((uint8_t *)&head.StartChar, (uint8_t *)this->m_frameHead.data(), this->m_frameHead.size());

    temp += FormatOutput<uint8_t>("帧头", head.StartChar, true);
    temp += FormatOutput<uint16_t>("长度", head.FrameLength, true);
    temp += FormatOutput<uint8_t>("后续帧", head.HasMore) + QString().sprintf("(%s后续帧)\n", head.HasMore?"有":"没有");
    temp += FormatOutput<uint32_t>("报文ID", (uint32_t)head.FrameID, true);
    temp += FormatOutput<uint16_t>("协议版本号", head.ProtoclVersion, true);
    temp += FormatOutput<uint16_t>("命令码", head.CommodCode, true);

    temp += QString("表具编号\t\t：");
    for (int i = 0; i < 20; i++)
    {
        temp += QString().sprintf("%0*x", sizeof(head.MeterID[i]) * 2, head.MeterID[i]);

        if (i > 5)
        {
            this->m_defaultKey += head.MeterID[i] - '0';
        }
    }

    temp += QString("(0000QW");
    for (int i = 6; i < 20; i++)
    {
        temp += QString().sprintf("%x", head.MeterID[i] - '0');
    }
    temp += QString(")\n");

    temp += QString("ICCID号\t\t：");
    for (int i = 0; i < 20; i++)
    {
        temp += QString().sprintf("%0*x", sizeof(head.ICCID[i]) * 2, head.ICCID[i]);
    }
    temp += QString("(");
    for (int i = 0; i < 20; i++)
    {
        temp += QString().sprintf("%x", head.ICCID[i] - '0');
    }
    temp += QString(")\n");

    temp += FormatOutput<uint8_t>("传输方向", head.TransferDirection) + QString().sprintf("(表具%s平台)\n", head.TransferDirection? "-->" : "<--");
    if (head.TransferDirection)
    {
        transDirection = XINSHENG_PROTOCOL_TRANS_DIRECTION_B2P;
    }
    else
    {
        transDirection = XINSHENG_PROTOCOL_TRANS_DIRECTION_P2B;
    }

    temp += FormatOutput<uint8_t>("请求响应标志位", head.RequestOrRespond) + QString().sprintf("(%s)\n", head.RequestOrRespond ? "响应" : "请求");
    if (head.RequestOrRespond)
    {
        frameType = XINSHENG_PROTOCOL_RESPONSE;
    }
    else
    {
        frameType = XINSHENG_PROTOCOL_REQUEST;
    }
    temp += FormatOutput("数据保留位", head.Reserve[0], head.Reserve[1]);
    temp += FormatOutput("加密保护", head.Encryption[0], head.Encryption[1]);
    temp += FormatOutput<uint16_t>("数据域长度", head.DataAreaLength) + QString().sprintf("(%d)\n", head.DataAreaLength);

    this->m_parsedHead = temp;
    return (XinShengParse::COMMAND_TYPE)head.CommodCode;
}

QString XinShengParse::CheckAbnormalBit(int32_t WarningStatus)
{
    QString res = 0;

    if (WarningStatus & (0x01 << 0))
    {
        res += QString("泄漏事件 ");
    }
    if (WarningStatus & (0x01 << 1))
    {
        res += QString("压力低事件 ");
    }
    if (WarningStatus & (0x01 << 2))
    {
        res += QString("压力高事件 ");
    }
    if (WarningStatus & (0x01 << 3))
    {
        res += QString("温度异常事件 ");
    }
    if (WarningStatus & (0x01 << 4))
    {
        res += QString("过载流量事件 ");
    }
    if (WarningStatus & (0x01 << 5))
    {
        res += QString("异常小流量事件 ");
    }
    if (WarningStatus & (0x01 << 6))
    {
        res += QString("持续流量事件 ");
    }
    if (WarningStatus & (0x01 << 7))
    {
        res += QString("反向接气事件 ");
    }
    if (WarningStatus & (0x01 << 8))
    {
        res += QString("强磁干扰事件 ");
    }
    if (WarningStatus & (0x01 << 9))
    {
        res += QString("霍尔磁干扰事件 ");
    }
    if (WarningStatus & (0x01 << 10))
    {
        res += QString("直通事件 ");
    }
    if (WarningStatus & (0x01 << 11))
    {
        res += QString("阀门故障事件 ");
    }
    if (WarningStatus & (0x01 << 12))
    {
        res += QString("远程开阀事件 ");
    }
    if (WarningStatus & (0x01 << 13))
    {
        res += QString("远程普通关阀事件 ");
    }
    if (WarningStatus & (0x01 << 14))
    {
        res += QString("传感器故障事件 ");
    }
    if (WarningStatus & (0x01 << 15))
    {
        res += QString("RTC故障事件 ");
    }
    if (WarningStatus & (0x01 << 16))
    {
        res += QString("存储故障事件 ");
    }
    if (WarningStatus & (0x01 << 17))
    {
        res += QString("开盖事件 ");
    }
    if (WarningStatus & (0x01 << 18))
    {
        res += QString("失联事件 ");
    }
    if (WarningStatus & (0x01 << 19))
    {
        res += QString("余量低于透支额度事件 ");
    }
    if (WarningStatus & (0x01 << 20))
    {
        res += QString("余量低于一级额度事件 ");
    }
    if (WarningStatus & (0x01 << 21))
    {
        res += QString("内部电池小于一级报警事件 ");
    }
    if (WarningStatus & (0x01 << 22))
    {
        res += QString("内部电池小于二级报警事件 ");
    }
    if (WarningStatus & (0x01 << 23))
    {
        res += QString("外部电池小于一级报警事件 ");
    }
    if (WarningStatus & (0x01 << 24))
    {
        res += QString("外部电池小于二级报警事件 ");
    }
    if (WarningStatus & (0x01 << 25))
    {
        res += QString("掉电事件 ");
    }
    if (WarningStatus & (0x01 << 26))
    {
        res += QString("外部RTC故障事件 ");
    }
    if (WarningStatus & (0x01 << 27))
    {
        res += QString("地震事件 ");
    }
    if (WarningStatus & (0x01 << 27))
    {
        res += QString("长期未使用事件 ");
    }

    return res;
}



void XinShengParse::ParseSingleReportBody()
{
    QString temp = 0;
    XINSHENG_PROTOCOL_REPORT_SINGLE_DATA body;
    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB);

    qDebug() << this->m_defaultKey;
    qDebug() << this->GetParseKey();
    QByteArray decodedText = encryption.decode(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body.MeterType, pArray, this->m_frameBody.size() - PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_DATA));

    temp += FormatOutput<uint16_t>("表具类型", body.MeterType);
    switch (body.MeterType)
    {
        case 1:
        temp += QString("(皮膜表)\n");
        break;

        case 2:
        temp += QString("(罗茨/腰轮表)\n");
        break;

        case 3:
        temp += QString("(超声波表)\n");
        break;

        case 4:
        temp += QString("(涡轮表)\n");
        break;

        case 5:
        temp += QString("(其他类型表)\n");
        break;

        default:
        temp += QString("(数据未定义)\n");
        break;
    }

    temp += FormatOutput("抄表时间", body.MeterReadYear, body.MeterReadMonth, body.MeterReadDay, body.MeterReadHour, body.MeterReadMin, body.MeterReadSec);
    temp += FormatOutput<uint8_t>("上报类型", body.ReportType);
    switch(body.ReportType)
    {
        case 0:
        temp += QString("(单条上报)\n");
        break;

        case 1:
        temp += QString("(手动上报)\n");
        break;

        case 2:
        temp += QString("(报警上报)\n");
        break;

        default:
        temp += QString("(数据未定义)\n");
        break;
    }

    temp += FormatOutput<uint8_t>("阀门状态", body.ValveStaus);
    switch(body.ValveStaus)
    {
        case 0:
        temp += QString("(无阀门)\n");
        break;

        case 1:
        temp += QString("(阀门开)\n");
        break;

        case 2:
        temp += QString("(阀门临时关)\n");
        break;

        case 3:
        temp += QString("(阀门强制关)\n");
        break;

        case 4:
        temp += QString("(阀门状态未知)\n");
        break;

        case 5:
        temp += QString("(阀门异常)\n");
        break;

        default:
        temp += QString("(数据未定义)\n");
        break;
    }

    temp += FormatOutput<uint32_t>("工况总量", body.Total_Working_Condition, true);
    temp += FormatOutput<uint32_t>("标况总量", body.Stand_Working_Condition, true);
    temp += FormatOutput<uint32_t>("标况瞬时流量", body.Standard_Instant_Flow, true);
    qDebug() << QString("标况总量").size();
    qDebug() << QString("标况瞬时流量").size();
    temp += FormatOutput<uint16_t>("温度", (uint16_t)body.Temperature, true);
    temp += FormatOutput<uint16_t>("压力", (uint16_t)body.Pressure, true);
    temp += FormatOutput<uint32_t>("剩余金额", (uint32_t)body.MoneySurplus, true);
    temp += FormatOutput<uint32_t>("最新结算读数", body.Latest_Settle_Reading, true);
    temp += FormatOutput("最新结算时间", body.Latest_Settle_Timing[0], body.Latest_Settle_Timing[1], body.Latest_Settle_Timing[2], body.Latest_Settle_Timing[3], body.Latest_Settle_Timing[4], body.Latest_Settle_Timing[5]);
    temp += FormatOutput<uint32_t>("告警状态", (uint32_t)body.WarmingStatus) + "(" + CheckAbnormalBit(body.WarmingStatus) + ")\n";
    temp += FormatOutput<uint32_t>("告警状态保留位", (uint32_t)body.WarmingStatusReserveBit) + "(" + CheckAbnormalBit(body.WarmingStatusReserveBit) + ")\n";
    temp += FormatOutput<uint16_t>("干电池电量", (uint16_t)body.DryPower) + QString().sprintf("(%d.%dV)\n",  body.DryPower / 100, body.DryPower % 100);
    temp += FormatOutput<uint16_t>("锂电池电量", (uint16_t)body.LiPower) + QString().sprintf("(%d.%dV)\n",  body.LiPower / 100, body.LiPower % 100);
    temp += FormatOutput<uint16_t>("信号质量", (uint16_t)body.ModuleRSRP, true);
    temp += FormatOutput<uint16_t>("信噪比",(uint16_t) body.ModuleSNR, true);
    temp += FormatOutput<uint16_t>("频点", (uint16_t)body.ModuleEARFCN, true);
    temp += FormatOutput<uint16_t>("物理小区标识", (uint16_t)body.ModulePhysicalCellId, true);
    temp += FormatOutput<uint8_t>("覆盖等级", body.ModuleECL, true);
    temp += FormatOutput("固件版本号", body.SoftWareVersion[0], body.SoftWareVersion[1], body.SoftWareVersion[2], body.SoftWareVersion[3]);
    temp += FormatOutput<uint16_t>("保留位", body.Reserve, true);

    //temp += (QString("基站小区标识\t\t：%1").arg((body.ModuleEARFCN), sizeof(body.ModuleEARFCN) * 2, 16, QLatin1Char('0')));
    this->m_parsedBody = temp;
}

void XinShengParse::ParseSingleReportRspBody()
{
    QString temp = 0;
    XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA body;
    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB);

    qDebug() << this->m_defaultKey;
    QByteArray decodedText = encryption.decode(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body.RespondCode, pArray, this->m_frameBody.size() - PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA));

    temp += FormatOutput<uint16_t>("响应码", body.RespondCode, true);
    temp += FormatOutput("服务器时间", body.Servertime[0], body.Servertime[1], body.Servertime[2], body.Servertime[3], body.Servertime[4], body.Servertime[5]);
    temp += FormatOutput("保留位", body.reserve[0], body.reserve[1]);
    this->m_parsedBody = temp;
}

void XinShengParse::ParseSetKey()
{
    QString temp = 0;
    QString key = 0;
    XINSHENG_PROTOCOL_SET_KEY_DATA body;
    QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB);

    qDebug() << this->GetParseKey();
    qDebug() << this->m_defaultKey;
    QByteArray decodedText = encryption.decode(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    memcpy((uint8_t *)&body.securitykey[0], pArray, this->m_frameBody.size() - PADDING_LENGTH(XINSHENG_PROTOCOL_SET_KEY_DATA));

    temp += QString("密钥\t\t：");
    for (int i = 0; i < 16; i++)
    {
        temp += QString("%1").arg(body.securitykey[i], sizeof(body.securitykey[i]) * 2, 16, QLatin1Char('0'));
        key += QString().sprintf("%02x", body.securitykey[i]);
    }

    this->m_latestKey = QByteArray::fromHex(key.toLocal8Bit());            // 获取平台下发的最新密钥

    // 解析数据赋值给m_parsedBody
    this->m_parsedBody = temp;
}

uint16_t XinShengParse::crc16ForModbus(const QByteArray &data)
{
    quint8 buf;
    quint16 crc16 = 0xFFFF;

    for ( auto i = 0; i < data.size(); ++i )
    {
        buf = data.at( i ) ^ crc16;
        crc16 >>= 8;
        crc16 ^= crc16Table[ buf ];
    }
    return  crc16;
}

QString XinShengParse::GetLatestKey()
{
    return this->m_latestKey.toHex();
}

void XinShengParse::SetLatestKey(QByteArray inputKey)
{
    this->m_latestKey = inputKey;
}

QByteArray XinShengParse::GetParseKey()
{
    if (this->m_useDefaultKeyFlag)
    {
        qDebug() << "使用初始密钥";
        return this->m_defaultKey;
    }
    else
    {
        if (this->m_latestKey.isEmpty())
        {

            qDebug() << "应该使用平台下发密钥，可是为空，所以使用初始密钥";
            return this->m_defaultKey;
        }
        else
        {
            qDebug() << "使用平台下发的密钥";
            return this->m_latestKey;
        }
    }
}

void XinShengParse::setUseDefaultKey(bool res)
{

    this->m_useDefaultKeyFlag = res;

}
