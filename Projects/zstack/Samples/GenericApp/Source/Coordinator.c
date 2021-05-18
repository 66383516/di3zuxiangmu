/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  ��Coordiantor
* ����    ��chenming
* �汾    ��V0.0.1
* ʱ��    ��2021/5/18
* ����    ��Э����
********************************************************************
* ����
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

endPointDesc_t GenericApp_epDesc;        // �ڵ�������
byte GenericApp_TaskID;                  // �������ȼ�
byte GenericApp_TransID;                 // ���ݷ�������


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* ������  ��GenericApp_Init
* ����    ��byte task_id
* ����    ��void
* ����    ��chenming
* ʱ��    ��2021/5/18
* ����    �������ʼ��
----------------------------------------------------------------*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID             = task_id;   //��ʼ�����������ȼ�(�������ȼ���Э��ջ�Ĳ���ϵͳOSAL����)
    GenericApp_TransID            = 0;         //���������ݰ�����ų�ʼ��Ϊ0
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;   //48-52�Խڵ����������еĳ�ʼ��
    afRegister( &GenericApp_epDesc );          //ʹ��afRegister�������ڵ�����������ע��
}


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* ������  ��GenericApp_ProcessEvent
* ����    ��byte task_id,UINT16 events
* ����    ��UINT16
* ����    ��chenming
* ʱ��    ��2021/5/18
* ����    ����Ϣ����
----------------------------------------------------------------*/

UINT16 GenericApp_ProcessEvent ( byte task_id,UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;             //������һ��ָ�������Ϣ�ṹ���ָ��MSGpkt
    if (events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive (GenericApp_TaskID ) ;  //ʹ��osal_msg _receive��������Ϣ�����Ͻ�����Ϣ
        while ( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD:          //�Խ��յ�����Ϣ�����ж�
                GenericApp_MessageMSGCB ( MSGpkt ) ;
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt ); //����osal_msg_deallocate��������ռ�ݵĶ��ڴ��ͷ�
            MSGpkt = (afIncomingMSGPacket_t *) osal_msg_receive   //������һ����Ϣ���ٴ���Ϣ�����������Ϣ��Ȼ����������Ӧ�Ĵ���ֱ��������Ϣ��������Ϊֹ
                ( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG) ;
    }
    return 0;
}
