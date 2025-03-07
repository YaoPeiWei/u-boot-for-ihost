#include <common.h>
#include <command.h>
#include <ihost_uart3_common.h>
#include <debug_uart.h>
#include <linux/delay.h>
#include <linux/ctype.h>



#define FRAME_HEADER       0xFE
#define RESPONSE_FRAME_TYPE 0x40
#define CMD_CODE_SUCCESS    0x03
#define TIMEOUT_MS         20000  // 20秒超时

enum {
    RESULT_ERRO_TIMEOUT = -2,
    RESULT_ERRO_LENTH,
    RESULT_ERRO_READ,
    RESULT_SUCCESS,
};

/****************************Info********************************************** 
 * Name:    InvertUint8 
 * Note:     把字节颠倒过来，如0x12变成0x48
            0x12: 0001 0010
            0x48: 0100 1000
 *****************************************************************************/
 void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf)
 {
     int i;
     unsigned char tmp[4]={0};
  
     for(i=0;i< 8;i++)
     {
         if(srcBuf[0]& (1 << i))
         tmp[0]|=1<<(7-i);
     }
     dBuf[0] = tmp[0];
     
 }

 void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf)
 {
     int i;
     unsigned short tmp[4]={0};
  
     for(i=0;i< 16;i++)
     {
         if(srcBuf[0]& (1 << i))
         tmp[0]|=1<<(15 - i);
     }
     dBuf[0] = tmp[0];
 }


/****************************Info********************************************** 
 * Name:    CRC-16/CCITT        x16+x12+x5+1 
 * Width:    16
 * Poly:    0x1021 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Alias:   CRC-CCITT,CRC-16/CCITT-TRUE,CRC-16/KERMIT 
 *****************************************************************************/ 
 unsigned short CRC16_CCITT(unsigned char *data, unsigned int datalen)
 {
     unsigned short wCRCin = 0x0000;
     unsigned short wCPoly = 0x1021;
     
     InvertUint16(&wCPoly,&wCPoly);
     while (datalen--)     
     {
         wCRCin ^= *(data++);
         for(int i = 0;i < 8;i++)
         {
             if(wCRCin & 0x01)
                 wCRCin = (wCRCin >> 1) ^ wCPoly;
             else
                 wCRCin = wCRCin >> 1;
         }
     }
     return (wCRCin);
 }

/****************************Info********************************************** 
 * Name:    listen_uart3_data
 * Parameter:    void
 * Comment: handel mcu messages
 *****************************************************************************/ 
int listen_uart3_data(void) {
    uint8_t buffer[256];
    uint32_t start_time;
    uint16_t crc_received, crc_calculated;
    printf("Listening on UART3...\n");
    start_time = get_timer(0);

    while (get_timer(start_time) < TIMEOUT_MS) {
        if (uart3_tstc()) {
            uint8_t byte = uart3_getc();

            // find frame header FE
            if (byte == FRAME_HEADER) {
                buffer[0] = byte; //save FE
                int idx = 1;

                // read frame lenth（2 byte）
                while (idx < 3) {
                    if (get_timer(start_time) >= TIMEOUT_MS) {
                        printf("Timeout waiting for frame length\n");
                        return RESULT_ERRO_LENTH;
                    }
                    if (uart3_tstc()) {
                        buffer[idx++] = uart3_getc(); //save Len(2)
                    }
                }

                // parsing frame length（big endian）
                uint16_t frame_len = (buffer[1] << 8) | buffer[2];
                if (frame_len > sizeof(buffer)) {  // FE + Len(2) + Type + Cmd + Seq + Err + CRC(2)
                    printf("Frame too long: %d\n", frame_len);
                    continue;
                }

                // read remaining bytes（Type, Cmd, Seq, Err, CRC）
                while (idx < frame_len) {
                    if (get_timer(start_time) >= TIMEOUT_MS) {
                        printf("Timeout waiting for frame data\n");
                        return RESULT_ERRO_READ;
                    }
                    if (uart3_tstc()) {
                        buffer[idx++] = uart3_getc();
                    }
                }
                //printf frame

                // int i = 0;
                // while (i < frame_len) {
                //     uart3_printhex2(buffer[i]);
                //     i++;
                // }
                // uart3_printascii("\n");
                
                // extract CRC (last 2 bytes)
                crc_received = (buffer[frame_len - 2] << 8) | buffer[frame_len - 1];


                // Calculate CRC (from FE to error code)
                crc_calculated = CRC16_CCITT(buffer, frame_len - 2);

                // Check CRC
                if (crc_received != crc_calculated) {
                    printf("CRC error: received 0x%04X, calculated 0x%04X\n", crc_received, crc_calculated);
                    continue;
                }

                // Check if it is a key 3 command
                if (buffer[4] == CMD_CODE_SUCCESS) {
                    printf("Success: Received valid response frame\n");
                    return RESULT_SUCCESS;
                }
            }
        }

        udelay(1000);  // Reduce CPU usage
    }

    printf("Timeout: No valid frame received in %ds\n",TIMEOUT_MS);
    return RESULT_ERRO_TIMEOUT;
}

int do_uart3_mcu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{

    //uart3_printascii("do_uart3_test waitting key 3 press...\n");

    int ret = CMD_RET_FAILURE;

    switch(listen_uart3_data()){
        case RESULT_SUCCESS :
            printf("KEY press ok!\n");
            ret = CMD_RET_SUCCESS;
           break;
        case RESULT_ERRO_TIMEOUT:
            printf("KEY press ERRO_TIMEOUT!!!\n");
           break;
        case RESULT_ERRO_READ :
            printf("KEY press ERRO_READ!!!\n");
           break;
        case RESULT_ERRO_LENTH :
            printf("KEY press ERRO_LENTH!!!\n");
           break; 
        default :
            printf("KEY press ERRO!!!\n");
    }

    return ret;
}
