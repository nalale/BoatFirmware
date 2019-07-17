#ifndef _CAN_FUNC_H_
#define _CAN_FUNC_H_

// Размер кольцевого буфера входящих сообщений CAN
#define RX_BUF_CAN_LEN			10

// Размер кольцевого буфера входящих сообщений CAN
#define TX_BUF_CAN_LEN         10

typedef struct {    
    uint32_t ID;
    uint8_t data[8];
    uint8_t Ext;
    uint8_t DLC;
} CanMsg;


void Can_Init(uint8_t CanChannel1, uint8_t CanChannel2);
uint8_t CanReceiveThread(uint8_t CanChannel, CanMsg *Msg);
uint8_t SendCanMsg(uint8_t channel, CanMsg *msg);


typedef struct
{
    CanMsg List[RX_BUF_CAN_LEN];
    int Head;
    int Tail;
} rxBufCAN_t;

typedef struct
{
    CanMsg List[TX_BUF_CAN_LEN];
    int Head;
    int Tail;
} txBufCAN_t;

CanMsg* canRxQueue_GetWriteMsg(int32_t buf_num);
int canRxQueue_ReadMsg(int32_t buf_num, CanMsg* msg);
uint8_t ecanGetTxMsg(CanMsg* msg, int32_t buf_num);
CanMsg* ecanGetEmptyTxMsg(int32_t buf_num);
uint32_t GetBufferAvailable(uint32_t channel);
#endif
