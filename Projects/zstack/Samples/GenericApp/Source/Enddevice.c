/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  ��Enddevice
* ����    ��dengdawei
* �汾    ��V0.0.1
* ʱ��    ��2021/5/18
* ����    ���ն˽ڵ�
********************************************************************
* ����
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* ͷ�ļ� ----------------------------------------------------------------*/
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

endPointDesc_t GenericApp_epDesc;           //�ڵ�������
byte GenericApp_TaskID;                     //�������ȼ�
byte GenericApp_TransID;                    //���ݷ�������
devStates_t GenericApp_NwkState;            //����ڵ�״̬

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );   //��Ϣ������
void GenericApp_SendTheMessage ( void );                       //���ݷ��ͺ���
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* ������  ��GenericApp_Init
* ����    ��byte task_id
* ����    ��void
* ����    ��dengdawei
* ʱ��    ��2021/5/18
* ����    �������ʼ��
----------------------------------------------------------------*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID             = task_id;   //��ʼ�����������ȼ�(�������ȼ���Э��ջ�Ĳ���ϵͳOSAL����)
    GenericApp_NwkState           = DEV_INIT;  //���豸״̬��ʼ��ΪDEV_INIT����ʾ�ýڵ�û�����ӵ�ZigBee
    GenericApp_TransID            = 0;         //���������ݰ�����ų�ʼ��Ϊ0
    GenericApp_epDesc.endPoint    = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id     = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc  =
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq  = noLatencyReqs;   //49-53�Խڵ����������еĳ�ʼ��
    afRegister( &GenericApp_epDesc );          //ʹ��afRegister�������ڵ�����������ע��
}


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* ������  ��GenericApp_ProcessEvent
* ����    ��byte task_id,UINT16 events
* ����    ��UINT16
* ����    ��Zhengchaoyue
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
            case ZDO_STATE_CHANGE:          //�Խ��յ�����Ϣ�����ж�
                GenericApp_NwkState = ( devStates_t )(MSGpkt->hdr.status) ;
                if(GenericApp_NwkState == DEV_END_DEVICE)  //�Խڵ��豸���ͽ����жϣ�������ն˽ڵ㣨�豸������ΪDEV_END DEVICE)����ִ����һ�д��룬ʵ���������ݷ��͡�
                {
                    GenericApp_SendTheMessage();
                }
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
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* ������  ��GenericApp_SendTheMessage
* ����    ��void
* ����    ��void
* ����    ��Zhengchaoyue
* ʱ��    ��2021/5/18
* ����    �����ݷ���
----------------------------------------------------------------*/
void GenericApp_SendTheMessage ( void )
{
    unsigned char theMessageData[4] = "LED" ;   //������һ������theMessageData�����ڴ��Ҫ���͵����ݡ�
    afAddrType_t my_DstAddr;   //������һ��afAddrType_t���͵ı���my_DstAddr����Ϊ���ݷ��ͺ���AF_DataRequest�ĵ�һ�����������������͵ı�����
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;  //�����͵�ַģʽ����Ϊ������Addr16Bit��ʾ����)
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;    //��ʼ���˿ں�
    my_DstAddr.addr.shortAddr = 0x0000;           //ֱ��ָ��Э�����������ַ
    AF_DataRequest ( &my_DstAddr,&GenericApp_epDesc,    //�������ݷ��ͺ���AF_DataRequest�����������ݵķ���
                    GENERICAPP_CLUSTERID,
                    3,
                    theMessageData,
                    &GenericApp_TransID,
                    AF_DISCV_ROUTE,
                    AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500);       //����HalLedBlink������ʹ�ն˽ڵ��LED2��˸
}

