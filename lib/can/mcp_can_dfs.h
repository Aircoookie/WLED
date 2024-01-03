/*
  mcp_can_dfs.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.
  2017 Copyright (c) Cory J. Fowler  All Rights Reserved.

  Author:Loovee
  Contributor: Cory J. Fowler
  2017-09-25
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/
#ifndef _MCP2515DFS_H_
#define _MCP2515DFS_H_

#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

#ifndef INT32U
#define INT32U unsigned long
#endif

#ifndef INT8U
#define INT8U byte
#endif

// if print debug information
#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

/*
 *   Begin mt
 */
#define TIMEOUTVALUE    2500                                           /* In Microseconds, May need changed depending on application and baud rate */
#define MCP_SIDH        0
#define MCP_SIDL        1
#define MCP_EID8        2
#define MCP_EID0        3

#define MCP_TXB_EXIDE_M     0x08                                        /* In TXBnSIDL                  */
#define MCP_DLC_MASK        0x0F                                        /* 4 LSBits                     */
#define MCP_RTR_MASK        0x40                                        /* (1<<6) Bit 6                 */

#define MCP_RXB_RX_ANY      0x60
#define MCP_RXB_RX_EXT      0x40
#define MCP_RXB_RX_STD      0x20
#define MCP_RXB_RX_STDEXT   0x00
#define MCP_RXB_RX_MASK     0x60
#define MCP_RXB_BUKT_MASK   (1<<2)

/*
** Bits in the TXBnCTRL registers.
*/
#define MCP_TXB_TXBUFE_M    0x80
#define MCP_TXB_ABTF_M      0x40
#define MCP_TXB_MLOA_M      0x20
#define MCP_TXB_TXERR_M     0x10
#define MCP_TXB_TXREQ_M     0x08
#define MCP_TXB_TXIE_M      0x04
#define MCP_TXB_TXP10_M     0x03

#define MCP_TXB_RTR_M       0x40                                        /* In TXBnDLC                   */
#define MCP_RXB_IDE_M       0x08                                        /* In RXBnSIDL                  */
#define MCP_RXB_RTR_M       0x40                                        /* In RXBnDLC                   */

#define MCP_STAT_RXIF_MASK   (0x03)
#define MCP_STAT_RX0IF       (1<<0)
#define MCP_STAT_RX1IF       (1<<1)

#define MCP_EFLG_RX1OVR     (1<<7)
#define MCP_EFLG_RX0OVR     (1<<6)
#define MCP_EFLG_TXBO       (1<<5)
#define MCP_EFLG_TXEP       (1<<4)
#define MCP_EFLG_RXEP       (1<<3)
#define MCP_EFLG_TXWAR      (1<<2)
#define MCP_EFLG_RXWAR      (1<<1)
#define MCP_EFLG_EWARN      (1<<0)
#define MCP_EFLG_ERRORMASK  (0xF8)                                      /* 5 MS-Bits                    */

#define MCP_BxBFS_MASK    0x30
#define MCP_BxBFE_MASK    0x0C
#define MCP_BxBFM_MASK    0x03

#define MCP_BxRTS_MASK    0x38
#define MCP_BxRTSM_MASK   0x07

/*
 *   Define MCP2515 register addresses
 */
#define MCP_RXF0SIDH    0x00
#define MCP_RXF0SIDL    0x01
#define MCP_RXF0EID8    0x02
#define MCP_RXF0EID0    0x03
#define MCP_RXF1SIDH    0x04
#define MCP_RXF1SIDL    0x05
#define MCP_RXF1EID8    0x06
#define MCP_RXF1EID0    0x07
#define MCP_RXF2SIDH    0x08
#define MCP_RXF2SIDL    0x09
#define MCP_RXF2EID8    0x0A
#define MCP_RXF2EID0    0x0B
#define MCP_BFPCTRL     0x0C
#define MCP_TXRTSCTRL   0x0D
#define MCP_CANSTAT     0x0E
#define MCP_CANCTRL     0x0F
#define MCP_RXF3SIDH    0x10
#define MCP_RXF3SIDL    0x11
#define MCP_RXF3EID8    0x12
#define MCP_RXF3EID0    0x13
#define MCP_RXF4SIDH    0x14
#define MCP_RXF4SIDL    0x15
#define MCP_RXF4EID8    0x16
#define MCP_RXF4EID0    0x17
#define MCP_RXF5SIDH    0x18
#define MCP_RXF5SIDL    0x19
#define MCP_RXF5EID8    0x1A
#define MCP_RXF5EID0    0x1B
#define MCP_TEC            0x1C
#define MCP_REC            0x1D
#define MCP_RXM0SIDH    0x20
#define MCP_RXM0SIDL    0x21
#define MCP_RXM0EID8    0x22
#define MCP_RXM0EID0    0x23
#define MCP_RXM1SIDH    0x24
#define MCP_RXM1SIDL    0x25
#define MCP_RXM1EID8    0x26
#define MCP_RXM1EID0    0x27
#define MCP_CNF3        0x28
#define MCP_CNF2        0x29
#define MCP_CNF1        0x2A
#define MCP_CANINTE        0x2B
#define MCP_CANINTF        0x2C
#define MCP_EFLG        0x2D
#define MCP_TXB0CTRL    0x30
#define MCP_TXB1CTRL    0x40
#define MCP_TXB2CTRL    0x50
#define MCP_RXB0CTRL    0x60
#define MCP_RXB0SIDH    0x61
#define MCP_RXB1CTRL    0x70
#define MCP_RXB1SIDH    0x71


#define MCP_TX_INT          0x1C                                    /* Enable all transmit interrupts  */
#define MCP_TX01_INT        0x0C                                    /* Enable TXB0 and TXB1 interrupts */
#define MCP_RX_INT          0x03                                    /* Enable receive interrupts        */
#define MCP_NO_INT          0x00                                    /* Disable all interrupts           */

#define MCP_TX01_MASK       0x14
#define MCP_TX_MASK        0x54

/*
 *   Define SPI Instruction Set
 */
#define MCP_WRITE           0x02

#define MCP_READ            0x03

#define MCP_BITMOD          0x05

#define MCP_LOAD_TX0        0x40
#define MCP_LOAD_TX1        0x42
#define MCP_LOAD_TX2        0x44

#define MCP_RTS_TX0         0x81
#define MCP_RTS_TX1         0x82
#define MCP_RTS_TX2         0x84
#define MCP_RTS_ALL         0x87

#define MCP_READ_RX0        0x90
#define MCP_READ_RX1        0x94

#define MCP_READ_STATUS     0xA0

#define MCP_RX_STATUS       0xB0

#define MCP_RESET           0xC0


/*
 *   CANCTRL Register Values
 */
#define MCP_NORMAL     0x00
#define MCP_SLEEP      0x20
#define MCP_LOOPBACK   0x40
#define MCP_LISTENONLY 0x60
#define MODE_CONFIG     0x80
#define MODE_POWERUP    0xE0
#define MODE_MASK       0xE0
#define ABORT_TX        0x10
#define MODE_ONESHOT    0x08
#define CLKOUT_ENABLE   0x04
#define CLKOUT_DISABLE  0x00
#define CLKOUT_PS1      0x00
#define CLKOUT_PS2      0x01
#define CLKOUT_PS4      0x02
#define CLKOUT_PS8      0x03


/*
 *   CNF1 Register Values
 */
#define SJW1            0x00
#define SJW2            0x40
#define SJW3            0x80
#define SJW4            0xC0


/*
 *   CNF2 Register Values
 */
#define BTLMODE         0x80
#define SAMPLE_1X       0x00
#define SAMPLE_3X       0x40


/*
 *   CNF3 Register Values
 */
#define SOF_ENABLE      0x80
#define SOF_DISABLE     0x00
#define WAKFIL_ENABLE   0x40
#define WAKFIL_DISABLE  0x00


/*
 *   CANINTF Register Bits
 */
#define MCP_RX0IF       0x01
#define MCP_RX1IF       0x02
#define MCP_TX0IF       0x04
#define MCP_TX1IF       0x08
#define MCP_TX2IF       0x10
#define MCP_ERRIF       0x20
#define MCP_WAKIF       0x40
#define MCP_MERRF       0x80


/*
 *  Speed 8M
 */
#define MCP_8MHz_1000kBPS_CFG1 (0x00)  
#define MCP_8MHz_1000kBPS_CFG2 (0xC0)  /* Enabled SAM bit     */
#define MCP_8MHz_1000kBPS_CFG3 (0x80)  /* Sample point at 75% */

#define MCP_8MHz_500kBPS_CFG1 (0x00)
#define MCP_8MHz_500kBPS_CFG2 (0xD1)   /* Enabled SAM bit     */
#define MCP_8MHz_500kBPS_CFG3 (0x81)   /* Sample point at 75% */

#define MCP_8MHz_250kBPS_CFG1 (0x80)   /* Increased SJW       */
#define MCP_8MHz_250kBPS_CFG2 (0xE5)   /* Enabled SAM bit     */
#define MCP_8MHz_250kBPS_CFG3 (0x83)   /* Sample point at 75% */

#define MCP_8MHz_200kBPS_CFG1 (0x80)   /* Increased SJW       */
#define MCP_8MHz_200kBPS_CFG2 (0xF6)   /* Enabled SAM bit     */
#define MCP_8MHz_200kBPS_CFG3 (0x84)   /* Sample point at 75% */

#define MCP_8MHz_125kBPS_CFG1 (0x81)   /* Increased SJW       */
#define MCP_8MHz_125kBPS_CFG2 (0xE5)   /* Enabled SAM bit     */
#define MCP_8MHz_125kBPS_CFG3 (0x83)   /* Sample point at 75% */

#define MCP_8MHz_100kBPS_CFG1 (0x81)   /* Increased SJW       */
#define MCP_8MHz_100kBPS_CFG2 (0xF6)   /* Enabled SAM bit     */
#define MCP_8MHz_100kBPS_CFG3 (0x84)   /* Sample point at 75% */

#define MCP_8MHz_80kBPS_CFG1 (0x84)    /* Increased SJW       */
#define MCP_8MHz_80kBPS_CFG2 (0xD3)    /* Enabled SAM bit     */
#define MCP_8MHz_80kBPS_CFG3 (0x81)    /* Sample point at 75% */

#define MCP_8MHz_50kBPS_CFG1 (0x84)    /* Increased SJW       */
#define MCP_8MHz_50kBPS_CFG2 (0xE5)    /* Enabled SAM bit     */
#define MCP_8MHz_50kBPS_CFG3 (0x83)    /* Sample point at 75% */

#define MCP_8MHz_40kBPS_CFG1 (0x84)    /* Increased SJW       */
#define MCP_8MHz_40kBPS_CFG2 (0xF6)    /* Enabled SAM bit     */
#define MCP_8MHz_40kBPS_CFG3 (0x84)    /* Sample point at 75% */

#define MCP_8MHz_33k3BPS_CFG1 (0x85)   /* Increased SJW       */
#define MCP_8MHz_33k3BPS_CFG2 (0xF6)   /* Enabled SAM bit     */
#define MCP_8MHz_33k3BPS_CFG3 (0x84)   /* Sample point at 75% */

#define MCP_8MHz_31k25BPS_CFG1 (0x87)  /* Increased SJW       */
#define MCP_8MHz_31k25BPS_CFG2 (0xE5)  /* Enabled SAM bit     */
#define MCP_8MHz_31k25BPS_CFG3 (0x83)  /* Sample point at 75% */

#define MCP_8MHz_20kBPS_CFG1 (0x89)    /* Increased SJW       */
#define MCP_8MHz_20kBPS_CFG2 (0xF6)    /* Enabled SAM bit     */
#define MCP_8MHz_20kBPS_CFG3 (0x84)    /* Sample point at 75% */

#define MCP_8MHz_10kBPS_CFG1 (0x93)    /* Increased SJW       */
#define MCP_8MHz_10kBPS_CFG2 (0xF6)    /* Enabled SAM bit     */
#define MCP_8MHz_10kBPS_CFG3 (0x84)    /* Sample point at 75% */

#define MCP_8MHz_5kBPS_CFG1 (0xA7)     /* Increased SJW       */
#define MCP_8MHz_5kBPS_CFG2 (0xF6)     /* Enabled SAM bit     */
#define MCP_8MHz_5kBPS_CFG3 (0x84)     /* Sample point at 75% */

/*
 *  speed 16M
 */
#define MCP_16MHz_1000kBPS_CFG1 (0x00)
#define MCP_16MHz_1000kBPS_CFG2 (0xCA)
#define MCP_16MHz_1000kBPS_CFG3 (0x81)    /* Sample point at 75% */

#define MCP_16MHz_500kBPS_CFG1 (0x40)     /* Increased SJW       */
#define MCP_16MHz_500kBPS_CFG2 (0xE5)
#define MCP_16MHz_500kBPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_16MHz_250kBPS_CFG1 (0x41)
#define MCP_16MHz_250kBPS_CFG2 (0xE5)
#define MCP_16MHz_250kBPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_16MHz_200kBPS_CFG1 (0x41)     /* Increased SJW       */
#define MCP_16MHz_200kBPS_CFG2 (0xF6)
#define MCP_16MHz_200kBPS_CFG3 (0x84)     /* Sample point at 75% */

#define MCP_16MHz_125kBPS_CFG1 (0x43)     /* Increased SJW       */
#define MCP_16MHz_125kBPS_CFG2 (0xE5)
#define MCP_16MHz_125kBPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_16MHz_100kBPS_CFG1 (0x44)     /* Increased SJW       */
#define MCP_16MHz_100kBPS_CFG2 (0xE5)
#define MCP_16MHz_100kBPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_16MHz_80kBPS_CFG1 (0x44)      /* Increased SJW       */
#define MCP_16MHz_80kBPS_CFG2 (0xF6)
#define MCP_16MHz_80kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_16MHz_50kBPS_CFG1 (0x47)      /* Increased SJW       */
#define MCP_16MHz_50kBPS_CFG2 (0xF6)
#define MCP_16MHz_50kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_16MHz_40kBPS_CFG1 (0x49)      /* Increased SJW       */
#define MCP_16MHz_40kBPS_CFG2 (0xF6)
#define MCP_16MHz_40kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_16MHz_33k3BPS_CFG1 (0x4E)
#define MCP_16MHz_33k3BPS_CFG2 (0xE5)
#define MCP_16MHz_33k3BPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_16MHz_20kBPS_CFG1 (0x53)      /* Increased SJW       */
#define MCP_16MHz_20kBPS_CFG2 (0xF6)
#define MCP_16MHz_20kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_16MHz_10kBPS_CFG1 (0x67)      /* Increased SJW       */
#define MCP_16MHz_10kBPS_CFG2 (0xF6)
#define MCP_16MHz_10kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_16MHz_5kBPS_CFG1 (0x3F)
#define MCP_16MHz_5kBPS_CFG2 (0xFF)
#define MCP_16MHz_5kBPS_CFG3 (0x87)       /* Sample point at 68% */

/*
 *  speed 20M
 */
#define MCP_20MHz_1000kBPS_CFG1 (0x00)
#define MCP_20MHz_1000kBPS_CFG2 (0xD9)
#define MCP_20MHz_1000kBPS_CFG3 (0x82)     /* Sample point at 80% */

#define MCP_20MHz_500kBPS_CFG1 (0x40)     /* Increased SJW       */
#define MCP_20MHz_500kBPS_CFG2 (0xF6)
#define MCP_20MHz_500kBPS_CFG3 (0x84)     /* Sample point at 75% */

#define MCP_20MHz_250kBPS_CFG1 (0x41)     /* Increased SJW       */
#define MCP_20MHz_250kBPS_CFG2 (0xF6)
#define MCP_20MHz_250kBPS_CFG3 (0x84)     /* Sample point at 75% */

#define MCP_20MHz_200kBPS_CFG1 (0x44)     /* Increased SJW       */
#define MCP_20MHz_200kBPS_CFG2 (0xD3)
#define MCP_20MHz_200kBPS_CFG3 (0x81)     /* Sample point at 80% */

#define MCP_20MHz_125kBPS_CFG1 (0x44)     /* Increased SJW       */
#define MCP_20MHz_125kBPS_CFG2 (0xE5)
#define MCP_20MHz_125kBPS_CFG3 (0x83)     /* Sample point at 75% */

#define MCP_20MHz_100kBPS_CFG1 (0x44)     /* Increased SJW       */
#define MCP_20MHz_100kBPS_CFG2 (0xF6)
#define MCP_20MHz_100kBPS_CFG3 (0x84)     /* Sample point at 75% */

#define MCP_20MHz_80kBPS_CFG1 (0xC4)      /* Increased SJW       */
#define MCP_20MHz_80kBPS_CFG2 (0xFF)
#define MCP_20MHz_80kBPS_CFG3 (0x87)      /* Sample point at 68% */

#define MCP_20MHz_50kBPS_CFG1 (0x49)      /* Increased SJW       */
#define MCP_20MHz_50kBPS_CFG2 (0xF6)
#define MCP_20MHz_50kBPS_CFG3 (0x84)      /* Sample point at 75% */

#define MCP_20MHz_40kBPS_CFG1 (0x18)
#define MCP_20MHz_40kBPS_CFG2 (0xD3)
#define MCP_20MHz_40kBPS_CFG3 (0x81)      /* Sample point at 80% */


#define MCPDEBUG        (0)
#define MCPDEBUG_TXBUF  (0)
#define MCP_N_TXBUFFERS (3)

#define MCP_RXBUF_0 (MCP_RXB0SIDH)
#define MCP_RXBUF_1 (MCP_RXB1SIDH)

#define MCP2515_SELECT()   digitalWrite(MCPCS, LOW)
#define MCP2515_UNSELECT() digitalWrite(MCPCS, HIGH)

#define MCP2515_OK         (0)
#define MCP2515_FAIL       (1)
#define MCP_ALLTXBUSY      (2)

#define CANDEBUG   1

#define CANUSELOOP 0

#define CANSENDTIMEOUT (200)                                            /* milliseconds                 */

/*
 *   initial value of gCANAutoProcess
 */
#define CANAUTOPROCESS (1)
#define CANAUTOON  (1)
#define CANAUTOOFF (0)

#define CAN_STDID (0)
#define CAN_EXTID (1)

#define CANDEFAULTIDENT    (0x55CC)
#define CANDEFAULTIDENTEXT (CAN_EXTID)

#define MCP_STDEXT   0                                                  /* Standard and Extended        */
#define MCP_STD      1                                                  /* Standard IDs ONLY            */
#define MCP_EXT      2                                                  /* Extended IDs ONLY            */
#define MCP_ANY      3                                                  /* Disables Masks and Filters   */

#define MCP_20MHZ    0
#define MCP_16MHZ    1
#define MCP_8MHZ     2
#define MCP_CLOCK_SELECT 3
#define MCP_CLKOUT_ENABLE 4


#define CAN_4K096BPS 0
#define CAN_5KBPS    1
#define CAN_10KBPS   2
#define CAN_20KBPS   3
#define CAN_31K25BPS 4
#define CAN_33K3BPS  5
#define CAN_40KBPS   6
#define CAN_50KBPS   7
#define CAN_80KBPS   8
#define CAN_100KBPS  9
#define CAN_125KBPS  10
#define CAN_200KBPS  11
#define CAN_250KBPS  12
#define CAN_500KBPS  13
#define CAN_1000KBPS 14

#define CAN_OK             (0)
#define CAN_FAILINIT       (1)
#define CAN_FAILTX         (2)
#define CAN_MSGAVAIL       (3)
#define CAN_NOMSG          (4)
#define CAN_CTRLERROR      (5)
#define CAN_GETTXBFTIMEOUT (6)
#define CAN_SENDMSGTIMEOUT (7)
#define CAN_FAIL       (0xff)

#define CAN_MAX_CHAR_IN_MESSAGE (8)

#define CAN_IS_EXTENDED       0x80000000
#define CAN_IS_REMOTE_REQUEST 0x40000000
#define CAN_EXTENDED_ID       0x1FFFFFFF

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
