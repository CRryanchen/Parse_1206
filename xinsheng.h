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

/*----------------通用响应数据域-----------------*/
typedef struct{
    int16_t     RespondCode;
    uint8_t     reserve[2];
}XINSHENG_PROTOCOL_SET_REPORT_PERIOD_RSP_DATA,
XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_RSP_DATA,
XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_RSP_DATA,
XINSHENG_PROTOCOL_SET_KEY_RSP_DATA,
XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_RSP_DATA,
XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_RSP_DATA,
XINSHENG_PROTOCOL_GENERIC_RSP_DATA;             //4字节


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

/*------------------密钥设置-------------------*/
typedef struct{
    uint8_t securitykey[16];
    uint8_t reserve[4];
}XINSHENG_PROTOCOL_SET_KEY_DATA;  //修改密钥数据域(20字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_SET_KEY_DATA                      body;
    uint8_t												complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_KEY_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_SET_KEY_FRAME;//修改密钥报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_SET_KEY_RSP_DATA                  body;
    uint8_t												complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_KEY_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_SET_KEY_RSP_FRAME;//修改密钥响应报文

/*---------------远程阀门控制-----------------*/
typedef struct{
    uint16_t     valveCommand;				//开关阀命令
}XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_DATA; //远程阀控数据域(2字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                  head;
    XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_DATA		body;
    uint8_t											complement[PADDING_LENGTH(XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                    tail;
}XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_DATA_FRAME; // 远程阀控报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_RSP_DATA		body;
    uint8_t                                             complement[PADDING_LENGTH(XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_REMOTE_VALVE_CONTROL_DATA_RSP_FRAME; // 远程阀控响应报文

/*--------------设置总购和余额------------*/
typedef struct{
    uint32_t	TotoalMoney;	//充值总购金额
    int32_t		LeftMoney;		//剩余金额
    uint32_t	CurrentPrice;	//当前单价
    uint8_t		reserve[2];
}XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_DATA;		//修改总购余额数据域(14字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_DATA		body;
    uint8_t                                             complement[PADDING_LENGTH(XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_DATA_FRAME;  //修改总购余额报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_RSP_DATA	body;
    uint8_t                                             complement[PADDING_LENGTH(XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_MODIFY_PURCHASE_BALANCE_DATA_RSP_FRAME;  //修改总购余额响应报文
/*-------------通讯参数设置---------------*/
//平台->表端
typedef struct{
    uint8_t     IPAddress[3][4];
    uint16_t    Port[3];
    uint8_t     Communicate_Protocol;
    uint8_t     ResendTimes;
    uint8_t     reserve[2];
}XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_DATA;  //设置通讯参数数据域(22字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_DATA      body;
    uint8_t												complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_FRAME;// 设置通信参数报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                      head;
    XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_RSP_DATA  body;
    uint8_t												complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                        tail;
}XINSHENG_PROTOCOL_SET_COMMUNICATION_PARAM_RSP_FRAME;// 设置通信参数响应报文


/*----------------上报周期设置--------------*/
typedef struct{
    uint8_t     ReportyType;  //上报周期类型
    uint8_t    	ReportTime[3];
    uint16_t    CycleValue;
    uint16_t    DataInvert;
    uint8_t     reserve[2];
}XINSHENG_PROTOCOL_SET_REPORT_PERIOD_DATA;  //设置上报周期数据域（10字节）

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                  head;
    XINSHENG_PROTOCOL_SET_REPORT_PERIOD_DATA        body;
    uint8_t                                         complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_REPORT_PERIOD_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                    tail;
}XINSHENG_PROTOCOL_SET_REPORT_PERIOD_FRAME; // 设置上报周期报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                  head;
    XINSHENG_PROTOCOL_SET_REPORT_PERIOD_RSP_DATA	body;
    uint8_t                                         complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_REPORT_PERIOD_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                    tail;
}XINSHENG_PROTOCOL_SET_REPORT_PERIOD_RSP_FRAME; // 设置上报周期响应报文


/*----------------报警阈值设置-------------*/
typedef struct{
    uint32_t	MaskCode;       			//掩码
    uint32_t 	MaxSurplusMoney;			//最大剩余金额
    int32_t	    PriceSurplusValue;			//剩余金额一级限值
    int32_t	    PriceOverrunValue;			//剩余金额二级限值
    int16_t     SuperFlowTime;				//过载
    int16_t		LittleFlowTime;				//异常小
    int16_t		TempLowValue;				//温度低限值
    int16_t		TempHightValue;	   			//温度高限值
    int16_t     PressureLowValue;  			//压力下限
    int16_t     PressureHighValue; 			//压力上限
    int16_t     ContinuedFlowTime; 			//持续流量时间
    uint8_t     LongTimeUnused;    			//长时间未用
    int32_t     DirectGas;     	    		//直通气量
    int16_t     LiPower[2];  			    //锂电池报警限值
    int16_t     DryPower[2];        		//干电池报警限值
    uint8_t     Earthquake[2];        		//地震
    int16_t     LongTimeUnconnect; 			//长时间未连接通信
    uint8_t		Reserve[2];
}XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_DATA;  //设置报警阈值数据域(49字节)

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                  head;
    XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_DATA	body;
    uint8_t                                         complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                    tail;
}XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_FRAME;//设置报警阈值报文

typedef struct{
    XINSHENG_PROTOCOL_FRAME_HEADER                  head;
    XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_RSP_DATA	body;
    uint8_t                                         complement[PADDING_LENGTH(XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_RSP_DATA)];
    XINSHENG_PROTOCOL_FRAME_TAIL                    tail;
}XINSHENG_PROTOCOL_SET_WARNING_THRESEHOLD_RSP_FRAME;//设置报警阈值响应报文

#endif // XINSHENG_H
