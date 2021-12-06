#ifndef XINSHENG_H
#define XINSHENG_H

#pragma pack(1)

#define     PKCS7_LENGTH                (16)         //PKCS7Padding补码基准值
#define     PADDING_LENGTH(x)           (PKCS7_LENGTH - (sizeof(x) % PKCS7_LENGTH))

/*----------------新圣协议报文头-----------------*/
typedef struct{
    uint8_t  	StartChar;                  //起始符
    uint16_t 	FrameLength;                //报文长度
    uint8_t	 	HasMore;                    //是否有后续帧
    int32_t  	FrameID;                    //报文ID
    uint16_t  	ProtoclVersion;             //协议版本号
    uint16_t 	CommodCode;                 //命令码
    uint8_t		MeterID[20];                //表具编号
    uint8_t		ICCID[20];                  //ICCID号
    uint8_t     TransferDirection;          //传输方向
    uint8_t     RequestOrRespond;           //请求响应标志位
    uint8_t     Reserve[2];                 //数据保留位 2字节
    uint8_t		Encryption[2];              //加密保护
    uint16_t    DataAreaLength;             //数据与长度
}XINSHENG_PROTOCOL_FRAME_HEADER;

/*----------------新圣协议报文尾-----------------*/
typedef struct{
    uint16_t     CRCCheck;                  //CRC校验
    uint8_t      EndChar;                   //数据与长度
}XINSHENG_PROTOCOL_FRAME_TAIL;

/*----------------单条数据上报-----------------*/
//表端->平台
typedef struct{
    uint16_t	MeterType;                  //表具类型
    uint8_t		MeterReadYear;              //抄表时间
    uint8_t 	MeterReadMonth;
    uint8_t 	MeterReadDay;
    uint8_t		MeterReadHour;
    uint8_t		MeterReadMin;
    uint8_t		MeterReadSec;
    uint8_t		ReportType;                 //上报类型
    uint8_t		ValveStaus;                 //阀门状态
    uint32_t	Total_Working_Condition;    //工况总量,保留1位小数,传输时扩大10倍
    uint32_t	Stand_Working_Condition;    //标况总量,保留1位小数,传输时扩大10倍
    uint32_t	Standard_Instant_Flow;      //标况瞬时流量
    int16_t		Temperature;                //温度
    int16_t		Pressure;                   //压力
    int32_t		MoneySurplus;               //剩余金额,保留2位小数，传输时扩大100倍
    uint32_t	Latest_Settle_Reading;      //最新结算读数(表端预付费才上传)
    uint8_t		Latest_Settle_Timing[6];    //最新结算时间(表端预付费才上传)
    int32_t		WarmingStatus;              //告警状态
    int32_t		WarmingStatusReserveBit;    //告警状态保留位
    int16_t		DryPower;                   //干电池电量,保留2位小数,扩大100倍
    int16_t		LiPower;                    //锂电池电量,保留2位小数,扩大100倍
    int16_t		ModuleRSRP;                 //信号质量，保留1位小数，扩大10倍
    int16_t 	ModuleSNR;                  //信噪比，保留1位小数，扩大10倍
    int16_t		ModuleEARFCN;               //频点
    int8_t		ModuleCellId[6];            //基站小区标识
    int16_t		ModulePhysicalCellId;       //物理小区标识
    uint8_t		ModuleECL;                  //覆盖等级
    uint8_t		SoftWareVersion[4];         //固件版本号
    uint16_t	Reserve;                    //保留位
}XINSHENG_PROTOCOL_REPORT_SINGLE_DATA;      //单条数据上报数据域(73字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER              head;
    XINSHENG_PROTOCOL_REPORT_SINGLE_DATA		body;
    uint8_t 									complement[PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                tail;
}XINSHENG_PROTOCOL_REPORT_SINGLE_FRAME;     //单条数据上报报文

typedef struct{
    int16_t     RespondCode;                //响应码
    uint8_t     Servertime[6];              //服务器时间
    uint8_t     reserve[2];                 //保留位
}XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA;  //单条上报响应数据域(10字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER              head;
    XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA    body;
    uint8_t 									complement[PADDING_LENGTH(XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                tail;
}XINSHENG_PROTOCOL_REPORT_SINGLE_RSP_FRAME; //单条数据上报响应报文


#endif // XINSHENG_H
