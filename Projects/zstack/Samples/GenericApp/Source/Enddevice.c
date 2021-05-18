/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ：Enddevice
* 作者    ：dengdawei
* 版本    ：V0.0.1
* 时间    ：2021/5/18
* 描述    ：终端节点
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* 头文件 ----------------------------------------------------------------*/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>

#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
#include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
    GENERICAPP_CLUSTERID
};
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,                    //  int Endpoint 
    GENERICAPP_PROFID,                      //  unit16 AppProfId[2]
    GENERICAPP_DEVICEID,                    //  unit16 AppDeviceId[2]
    GENERICAPP_DEVICE_VERSION,              //  int  AppDevVer:4
    GENERICAPP_FLAGS,                       //  int  AppFlags:4
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,                //  byte AppNumInClusters
    (cId_t *)GenericApp_ClusterList
};

endPointDesc_t GenericApp_epDesc;           //节点描述符
byte GenericApp_TaskID;                     //任务优先级
byte GenericApp_TransID;                    //数据发送序列
devStates_t GenericApp_NwkState;            //保存节点状态

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );   //消息处理函数
void GenericApp_SendTheMessage ( void );                       //数据发送函数
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_Init
* 参数    ：byte task_id
* 返回    ：void
* 作者    ：dengdawei
* 时间    ：2021/5/18
* 描述    ：任务初始化
----------------------------------------------------------------*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID             = task_id;   //初始化了任务优先级(任务优先级有协议栈的操作系统OSAL分配)
    GenericApp_NwkState           = DEV_INIT;  //将设备状态初始化为DEV_INIT，表示该节点没有连接到ZigBee
    GenericApp_TransID            = 0;         //将发送数据包的序号初始化为0
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;   //49-53对节点描述符进行的初始化
    afRegister( &GenericApp_epDesc );          //使用afRegister函数将节点描述符进行注册
}


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_ProcessEvent
* 参数    ：byte task_id,UINT16 events
* 返回    ：UINT16
* 作者    ：Zhengchaoyue
* 时间    ：2021/5/18
* 描述    ：消息处理
----------------------------------------------------------------*/

UINT16 GenericApp_ProcessEvent ( byte task_id,UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;             //定义了一个指向接收消息结构体的指针MSGpkt
    if (events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive (GenericApp_TaskID ) ;  //使用osal_msg _receive函数从消息队列上接收消息
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:          //对接收到的消息进行判断
                GenericApp_NwkState = ( devStates_t )(MSGpkt->hdr.status) ;
                if(GenericApp_NwkState == DEV_END_DEVICE)  //对节点设备类型进行判断，如果是终端节点（设备类型码为DEV_END DEVICE)，再执行下一行代码，实现无线数据发送。
                {
                    GenericApp_SendTheMessage();
                }
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt ); //调用osal_msg_deallocate函数将其占据的堆内存释放
            MSGpkt = (afIncomingMSGPacket_t *) osal_msg_receive   //处理完一个消息后，再从消息队列里接收消息，然后对其进行相应的处理，直到所有消息都处理完为止
                ( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG) ;
    }
    return 0;
}
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_SendTheMessage
* 参数    ：void
* 返回    ：void
* 作者    ：Zhengchaoyue
* 时间    ：2021/5/18
* 描述    ：数据发送
----------------------------------------------------------------*/
void GenericApp_SendTheMessage ( void )
{
    unsigned char theMessageData[4] = "LED" ;   //定义了一个数组theMessageData，用于存放要发送的数据。
    afAddrType_t my_DstAddr;   //定义了一个afAddrType_t类型的变量my_DstAddr，因为数据发送函数AF_DataRequest的第一个参数就是这种类型的变量。
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;  //将发送地址模式设置为单播（Addr16Bit表示单播)
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;    //初始化端口号
    my_DstAddr.addr.shortAddr = 0x0000;           //直接指定协调器的网络地址
    AF_DataRequest ( &my_DstAddr,&GenericApp_epDesc,    //调用数据发送函数AF_DataRequest进行无线数据的发送
                    GENERICAPP_CLUSTERID,
                    3,
                    theMessageData,
                    &GenericApp_TransID,
                    AF_DISCV_ROUTE,
                    AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500);       //调用HalLedBlink函数，使终端节点的LED2闪烁
}

