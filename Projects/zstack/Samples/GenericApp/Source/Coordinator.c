/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ：Coordiantor
* 作者    ：chenming
* 版本    ：V0.0.1
* 时间    ：2021/5/18
* 描述    ：协调器
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
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
    GENERICAPP_ENDPOINT,                 //  int Endpoint 
    GENERICAPP_PROFID,                   //  unit16 AppProfId[2]
    GENERICAPP_DEVICEID,                 //  unit16 AppDeviceId[2]
    GENERICAPP_DEVICE_VERSION,           //  int  AppDevVer:4
    GENERICAPP_FLAGS,                    //  int  AppFlags:4
    GENERICAPP_MAX_CLUSTERS,             //  byte AppNumInClusters
    (cId_t *)GenericApp_ClusterList,
    0,
    (cId_t *)NULL
};

endPointDesc_t GenericApp_epDesc;        // 节点描述符
byte GenericApp_TaskID;                  // 任务优先级
byte GenericApp_TransID;                 // 数据发送序列


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_Init
* 参数    ：byte task_id
* 返回    ：void
* 作者    ：chenming
* 时间    ：2021/5/18
* 描述    ：任务初始化
----------------------------------------------------------------*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID             = task_id;   //初始化了任务优先级(任务优先级有协议栈的操作系统OSAL分配)
    GenericApp_TransID            = 0;         //将发送数据包的序号初始化为0
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;   //48-52对节点描述符进行的初始化
    afRegister( &GenericApp_epDesc );          //使用afRegister函数将节点描述符进行注册
}


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 函数名  ：GenericApp_ProcessEvent
* 参数    ：byte task_id,UINT16 events
* 返回    ：UINT16
* 作者    ：chenming
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
            case AF_INCOMING_MSG_CMD:          //对接收到的消息进行判断
                GenericApp_MessageMSGCB ( MSGpkt ) ;
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
