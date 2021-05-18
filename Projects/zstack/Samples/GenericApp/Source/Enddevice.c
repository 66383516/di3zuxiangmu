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


