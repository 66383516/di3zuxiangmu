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


