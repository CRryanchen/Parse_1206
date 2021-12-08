#include "fengxianparse.h"
#include <QDebug>

#define FENGXIAN_PROTOCOL_FRAME_STARTCHAR           0X69            // 新圣协议起始符
#define FENGXIAN_PROTOCOL_FRAME_ENDCHAR             0X17            // 新圣协议结束符

template<typename T>
QString FengXianParse::FormatOutput(const QString &str, const T &data, bool endline)
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

QString FengXianParse::FormatOutput(const QString &str, const uint8_t &year, const uint8_t &month, const uint8_t &day, const uint8_t &hour, const uint8_t &min, const uint8_t &sec)
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

uint32_t FengXianParse::u32Convert(uint32_t val)
{
    uint32_t temp;
    temp = val;
    temp = ((val)>>24&0x000000ff)|(((val)>>8&0x0000ff00))|((val)<<24&0xff000000)|((val)<<8&0x00ff0000);

    return temp;
}

uint16_t FengXianParse::u16Convert(uint16_t val)
{
    uint16_t temp;
    temp = val;
    temp = ((val)>>8&0x00ff)|((val)<<8&0xff00);

    return temp;
}

QByteArray FengXianParse::GetParseKey()
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


FengXianParse::FengXianParse(QString &inputData, QObject *parent) : QObject(parent)
{
    this->m_frameData = QByteArray::fromHex(inputData.toLocal8Bit());
    this->m_frameHead = this->m_frameData.left(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER));
    this->m_frameTail = this->m_frameData.right(sizeof(FENGXIAN_PROTOCOL_FRAME_TAIL));

    this->m_useDefaultKeyFlag = true;   // 使用初始密钥
    this->m_defaultKey = QByteArray::fromHex("5348465852514753");
}

void FengXianParse::StartParse()
{
    // 校验协议头
    FENGXIAN_PROTOCOL_FRAME_HEADER head;
    memcpy((uint8_t *)&head.Start, (uint8_t *)this->m_frameHead.data(), this->m_frameHead.size());

    // 判断起始符
    if (head.Start == FENGXIAN_PROTOCOL_FRAME_STARTCHAR)
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
    FengXianParse::COMMAND_TYPE commandType = this->ParseHead(frameType, transDirection);

    // 校验协议尾
    FENGXIAN_PROTOCOL_FRAME_TAIL tail;
    memcpy((uint8_t *)&tail.cs, (uint8_t *)this->m_frameTail.data(), this->m_frameTail.size());

    uint8_t cs = tail.cs;
    // 需要校验的数据
    QByteArray needCRCData = this->m_frameData;
    needCRCData.chop(sizeof(FENGXIAN_PROTOCOL_FRAME_TAIL));
    qDebug() << cs;
    qDebug() << CalcCheckCode(needCRCData);
    if (cs == CalcCheckCode(needCRCData))
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
    if (tail.end == FENGXIAN_PROTOCOL_FRAME_ENDCHAR)
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
        case FENGXIAN_PROTOCOL_BUSINESS_REPORT:
            // 业务上报数据帧为8的整数倍，理应再加8个补码，但是从数据来看其中没有8个补码，所以这里去掉了补码的长度
            qDebug() << "This is 3801";
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_BUSINESS_REPORT_DATA));
            this->ParseBusinessReportBody();
        break;

        case FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE:
            qDebug() << "This is 3401";
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE_DATA));
            this->ParseBusinessDistributeBody();
        break;

        case FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM_DATA));
            this->ParseSetCommunicationParamBody();
        break;

        case FENGXIAN_PROTOCOL_SET_FUNCTION:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_SET_FUNCTION_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_FUNCTION_DATA));
            this->ParseSetFunctionBody();
        break;

        case FENGXIAN_PROTOCOL_SET_TOTAL_GAS:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_SET_TOTAL_GAS_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_TOTAL_GAS_DATA));
            this->ParseSetTotalGasBody();
        break;

        case FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL_DATA));
            this->ParseConstructionValveControlBody();
        break;

        case FENGXIAN_PROTOCOL_COMMUNICATION_END:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_COMMUNICATION_END_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_COMMUNICATION_END_DATA));
            this->ParseComuunicationEndBody();
        break;

        case FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL_DATA));
            this->ParseRemoteValveControlBody();
        break;

        case FENGXIAN_PROTOCOL_HEARTBEAT_PACK:
            this->m_frameBody = this->m_frameData.mid(sizeof(FENGXIAN_PROTOCOL_FRAME_HEADER), sizeof(FENGXIAN_PROTOCOL_HEARTBEAT_PACK_DATA) + PADDING_LENGTH(FENGXIAN_PROTOCOL_HEARTBEAT_PACK_DATA));
            this->ParseHeartbeatPackBody();
        break;

        case FENGXIAN_PROTOCOL_SET_KEY:
            // 奉贤目前没有修改过密钥，暂不做处理
        break;

        case FENGXIAN_PROTOCOL_EXTEND_SECURITY_FUNCTION:
            // 奉贤代码中都没有安全功能扩展的类型定义，所以不做处理
        break;

        default:
            qDebug() << "错误的命令码";
        break;
    }
}

FengXianParse::COMMAND_TYPE FengXianParse::ParseHead(FengXianParse::FRAME_TYPE &frameType, FengXianParse::TRANS_DIRECTION &transDirection)
{
    QString temp = 0;

    FengXianParse::COMMAND_TYPE commandType;
    FENGXIAN_PROTOCOL_FRAME_HEADER head;
    memcpy((uint8_t *)&head.Start, (uint8_t *)this->m_frameHead.data(), this->m_frameHead.size());

    temp += FormatOutput<uint8_t>("帧起始符", head.Start, true);
    temp += FormatOutput<uint16_t>("数据长度", head.RealDataLength, true);
    temp += FormatOutput<uint16_t>("协议版本", head.ProtocolVer, true);
    temp += FormatOutput<uint8_t>("厂商代码", head.VendoCode, true);
    temp += FormatOutput<uint8_t>("控制码", head.ContralArea, true);
    // 这里奉贤和新圣的不一样，奉贤平台不会下发对表端的响应报文，而是收到表端的响应之后下发下一个请求
    // 所以来自平台的都是请求，来自表端的都是响应
    if (head.ContralArea == 0x81 || head.ContralArea == 0x84)
    {
        transDirection = FENGXIAN_PROTOCOL_TRANS_DIRECTION_B2P;
        frameType = FENGXIAN_PROTOCOL_RESPONSE;
    }
    else if (head.ContralArea == 0x01 || head.ContralArea == 0x04)
    {
        transDirection = FENGXIAN_PROTOCOL_TRANS_DIRECTION_P2B;
        frameType = FENGXIAN_PROTOCOL_REQUEST;
    }

    commandType = (FengXianParse::COMMAND_TYPE)(head.DataIdent & 0xff);
    temp += FormatOutput<uint8_t>("数据标识", commandType, true);
    temp += FormatOutput<uint8_t>("安全模式", (head.DataIdent & 0xff00) >> 8, true);

    this->m_parsedHead = temp;
    return commandType;
}

uint8_t FengXianParse::CalcCheckCode(const QByteArray &data)
{
    uint8_t checkCode = 0;

    for (int i = 0; i < data.size(); ++i)
    {
        checkCode += data.at(i);
    }

    return checkCode;
}

void FengXianParse::ParseBusinessReportBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_BUSINESS_REPORT_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size());

    temp += FormatOutput<uint16_t>("连续通讯失败关阀天数", u16Convert(body.CommFailDay));
    temp += FormatOutput<uint8_t>("连续通讯失败关阀时间", body.CommFailHour);
    temp += FormatOutput<uint16_t>("抄表冻结时间", u16Convert(body.ReadStatTime.m_time));
    temp += FormatOutput<uint16_t>("安全功能码", u16Convert(body.SafeFunCode.SafeCode));
    temp += FormatOutput<uint16_t>("温度", u16Convert(body.Temperature));
    temp += FormatOutput<uint16_t>("压力", u16Convert(body.Pressure));
//    temp += FormatOutput<uint8_t>("工况累积量");
//    temp += FormatOutput<uint8_t>("当前累计量", body.CurrentTotalGas);
    for (int i = 0; i < 30; i++)
    {
        temp += QString().sprintf("%d天之前的累积量", i + 1) + FormatOutput<uint32_t>("", u32Convert(body.LastThirtyDayGas[i]));
    }

    temp += FormatOutput<uint16_t>("表具运行状态", u16Convert(body.MeterState.Stat_Value));
    temp += FormatOutput<uint16_t>("阀门状态", u16Convert(body.ValveNum.Valve_Value));
    temp += FormatOutput<uint8_t>("电压值", body.VoltageValue);
    temp += FormatOutput<uint8_t>("供电方式", body.SupplyMode);
    temp += FormatOutput<uint8_t>("信号强度", body.Rssi);
    temp += FormatOutput<uint16_t>("信噪比", u16Convert(body.Snr));
    temp += FormatOutput<uint8_t>("覆盖等级", body.Cover_Level);
    temp += FormatOutput<uint8_t>("基站编号", body.Cell_id[0]);
    temp += FormatOutput<uint16_t>("定时通讯开始时间", u16Convert(body.TimeCommStart.m_time));
    temp += FormatOutput<uint16_t>("定时通讯结束时间", u16Convert(body.TimeCommEnd.m_time));
    temp += FormatOutput<uint16_t>("定时通讯连接间隔", u16Convert(body.TimeCommInterval));
    temp += FormatOutput<uint8_t>("通讯超时时间", body.CommTimeOut);
    temp += FormatOutput<uint8_t>("重连次数", body.ReconnectNum);
    temp += FormatOutput<uint8_t>("重连延迟时间", body.ReconnectTime);
    temp += FormatOutput<uint8_t>("程序版本号", body.SoftVer);
    temp += FormatOutput<uint16_t>("上报原因", u16Convert(body.ReportWay.Upload_Way));
    temp += FormatOutput("表内时间", body.MeterTime.year, body.MeterTime.month, body.MeterTime.day, body.MeterTime.hour, body.MeterTime.minute, body.MeterTime.second);
    temp += FormatOutput<uint8_t>("工作模式", body.WorkMode);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseBusinessDistributeBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_BUSINESS_DISTRIBUTE_DATA));

    temp += FormatOutput<uint8_t>("功能码", body.FunValue.funcode);
    temp += FormatOutput<uint32_t>("账户余额", u32Convert((uint32_t)body.SurplusMoney));
    temp += FormatOutput<uint8_t>("余额状态", body.SurplusStat);
    temp += FormatOutput("表内时间", body.MeterTime.year, body.MeterTime.month, body.MeterTime.day, body.MeterTime.hour, body.MeterTime.minute, body.MeterTime.second);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseSetCommunicationParamBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_COMMUNICATION_PARAM_DATA));

    temp += FormatOutput<uint8_t>("功能码", body.ConnectCode.funcode);
    temp += FormatOutput<uint16_t>("定时通讯开始时间", u16Convert(body.StartTime.m_time));
    temp += FormatOutput<uint16_t>("定时通讯结束时间", u16Convert(body.EndTime.m_time));
    temp += FormatOutput<uint16_t>("定时通讯连接间隔", u16Convert(body.CommInterval));
    temp += FormatOutput<uint8_t>("通讯超时时间", body.CommTimeOut);
    temp += FormatOutput<uint8_t>("重连次数", body.ReCommNum);
    temp += FormatOutput<uint8_t>("重连延迟时间", body.ReCommDelay);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseSetFunctionBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_SET_FUNCTION_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_FUNCTION_DATA));

    temp += FormatOutput<uint8_t>("功能码", body.FunCode.funcode);
    temp += FormatOutput<uint8_t>("安全模式", body.SecureMode);
    temp += FormatOutput<uint8_t>("工作模式", body.WorkMode);
    temp += FormatOutput<uint16_t>("连续通讯失败关阀天数", u16Convert(body.CommFailedDay));
    temp += FormatOutput<uint8_t>("连续通讯失败关阀时间", body.CommFailedTime);
    temp += FormatOutput<uint16_t>("抄表冻结时间", u16Convert(body.ReadMeterTime.m_time));
    temp += FormatOutput<uint16_t>("安全功能", u16Convert(body.Safe_Func.SafeCode));
    temp += FormatOutput<uint8_t>("大流时间", body.OverLoadTime);
    temp += FormatOutput<uint16_t>("大流流量上限", body.OverLoadFlow);
    temp += FormatOutput<uint8_t>("小流时间", body.LittleTime);
    temp += FormatOutput<uint8_t>("小流流量上限", body.LittleFlow);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseSetTotalGasBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_SET_TOTAL_GAS_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_SET_TOTAL_GAS_DATA));

    for (int i = 0; i < 5; i++)
    {
        temp += FormatOutput<uint8_t>("累积量", body.TotalGas[i]);
    }

    this->m_parsedBody = temp;
}

void FengXianParse::ParseConstructionValveControlBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_CONSTRUCTION_VALVE_CONTROL_DATA));

    temp += FormatOutput<uint8_t>("阀门控制", body.valve_control);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseComuunicationEndBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_COMMUNICATION_END_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_COMMUNICATION_END_DATA));

    temp += FormatOutput("服务器时间", body.Server_Time.year, body.Server_Time.month, body.Server_Time.day, body.Server_Time.hour, body.Server_Time.minute, body.Server_Time.second);
    temp += FormatOutput<uint32_t>("年累计量", u32Convert(body.LadderSurplusGas));
    temp += FormatOutput<uint16_t>("当前气价", u16Convert(body.CurGasPrice));

    this->m_parsedBody = temp;
}

void FengXianParse::ParseRemoteValveControlBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_REMOTE_VALVE_CONTROL_DATA));

    temp += FormatOutput<uint8_t>("阀门控制命令", body.valve_cmd);

    this->m_parsedBody = temp;
}

void FengXianParse::ParseHeartbeatPackBody()
{
    QString temp = 0;
    FENGXIAN_PROTOCOL_HEARTBEAT_PACK_DATA body;
    QByteArray decodedText = decrypt(this->m_frameBody, this->GetParseKey());
    uint8_t *pArray = NULL;

    pArray = (uint8_t *)decodedText.data();
    // 将解密后的数据赋值给body，长度需要减去补码的长度
    memcpy((uint8_t *)&body, pArray, this->m_frameBody.size() - PADDING_LENGTH(FENGXIAN_PROTOCOL_HEARTBEAT_PACK_DATA));

    temp += FormatOutput("当前时间", body.current_time.year, body.current_time.month, body.current_time.day, body.current_time.hour, body.current_time.minute, body.current_time.second);

    this->m_parsedBody = temp;
}


QByteArray FengXianParse::decrypt(const QByteArray &src, const QByteArray &key)
{
    QByteArray res = 0;
    //异或加密计算
    for(int i=0; i < src.size(); ++i)  //8字节分组
    {
        res.append(src.at(i) ^ key.at(i % ENCRYPTION_LEN));
    }

    return res;
}






