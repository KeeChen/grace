/*
**             Copyright 2012-2016 by Kvaser AB, Molndal, Sweden
**                        http://www.kvaser.com
**
** This software is dual licensed under the following two licenses:
** BSD-new and GPLv2. You may use either one. See the included
** COPYING file for details.
**
** License: BSD-new
** ===============================================================================
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the <organization> nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** License: GPLv2
** ===============================================================================
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** ---------------------------------------------------------------------------
**/

/*  Kvaser Linux Canlib VCan layer functions */

#include <stdint.h>
#include "vcan_ioctl.h"
#include "kcan_ioctl.h"
#include "canIfData.h"
#include "canlib_data.h"
#include "vcanevt.h"
#include "dlc.h"

#   include <canlib.h>
#   include <stdio.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <sys/ioctl.h>
#   include <unistd.h>
#   include <stdio.h>
#   include <errno.h>
#   include <signal.h>
#   include <pthread.h>
#   include <string.h>
#   include <sys/stat.h>


#include "osif_functions_user.h"
#include "VCanFunctions.h"
#include "debug.h"


#   if DEBUG
#      define DEBUGPRINT(args) printf args
#   else
#      define DEBUGPRINT(args)
#   endif

// Standard resolution for the time stamps and canReadTimer is
// 1 ms, i.e. 100 VCAND ticks.
#define DEFAULT_TIMER_FACTOR 100


static uint32_t capabilities_table[][2] = {
  {VCAN_CHANNEL_CAP_EXTENDED_CAN,        canCHANNEL_CAP_EXTENDED_CAN},
  {VCAN_CHANNEL_CAP_BUSLOAD_CALCULATION, canCHANNEL_CAP_BUS_STATISTICS},
  {VCAN_CHANNEL_CAP_ERROR_COUNTERS,      canCHANNEL_CAP_ERROR_COUNTERS},
  {VCAN_CHANNEL_CAP_CAN_DIAGNOSTICS,     canCHANNEL_CAP_CAN_DIAGNOSTICS},
  {VCAN_CHANNEL_CAP_SEND_ERROR_FRAMES,   canCHANNEL_CAP_GENERATE_ERROR},
  {VCAN_CHANNEL_CAP_TXREQUEST,           canCHANNEL_CAP_TXREQUEST},
  {VCAN_CHANNEL_CAP_TXACKNOWLEDGE,       canCHANNEL_CAP_TXACKNOWLEDGE},
  {VCAN_CHANNEL_CAP_VIRTUAL,             canCHANNEL_CAP_VIRTUAL},
  {VCAN_CHANNEL_CAP_SIMULATED,           canCHANNEL_CAP_SIMULATED},
  {VCAN_CHANNEL_CAP_REMOTE,              canCHANNEL_CAP_REMOTE},
  {VCAN_CHANNEL_CAP_CANFD,               canCHANNEL_CAP_CAN_FD},
  {VCAN_CHANNEL_CAP_CANFD_NONISO,        canCHANNEL_CAP_CAN_FD_NONISO},
  {VCAN_CHANNEL_CAP_SILENTMODE,          canCHANNEL_CAP_SILENT_MODE},
  {VCAN_CHANNEL_CAP_SINGLE_SHOT,         canCHANNEL_CAP_SINGLE_SHOT}
};


// If there are more handles than this, the rest will be
// handled by a linked list.
#define MAX_ARRAY_HANDLES 64

static HandleData  *handleArray[MAX_ARRAY_HANDLES];
static HandleList  *handleList;
static CanHandle   handleMax   = MAX_ARRAY_HANDLES;
static OS_IF_MUTEX handleMutex = OS_IF_MUTEX_INITIALIZER;

static uint32_t get_capabilities (uint32_t cap);

#define ERROR_WHEN_NEQ 0
#define ERROR_WHEN_LT  1
static int check_args (void* buf, uint32_t buf_len, uint32_t limit, uint32_t method);

//******************************************************
// Compare handles
//******************************************************
static int hndCmp (const void *hData1, const void *hData2)
{
  return ((HandleData *)(hData1))->handle ==
         ((HandleData *)(hData2))->handle;
}


//******************************************************
// Find handle in list
//******************************************************
HandleData * findHandle (CanHandle hnd)
{
  HandleData dummyHandleData, *found;
  dummyHandleData.handle = hnd;

  if (hnd < 0) {
    return NULL;
  }

  os_if_mutex_lock(&handleMutex);
  if (hnd < MAX_ARRAY_HANDLES) {
    found = handleArray[hnd];
  } else {
    found = listFind(&handleList, &dummyHandleData, &hndCmp);
  }
  os_if_mutex_unlock(&handleMutex);

  return found;
}


//******************************************************
// Remove handle from list
//******************************************************
HandleData * removeHandle (CanHandle hnd)
{
  HandleData dummyHandleData, *found;
  dummyHandleData.handle = hnd;

  if (hnd < 0) {
    return NULL;
  }

  os_if_mutex_lock(&handleMutex);
  if (hnd < MAX_ARRAY_HANDLES) {
    found = handleArray[hnd];
    handleArray[hnd] = NULL;
  } else {
    found = listRemove(&handleList, &dummyHandleData, &hndCmp);
  }
  os_if_mutex_unlock(&handleMutex);

  return found;
}

//******************************************************
// Insert handle in list
//******************************************************
CanHandle insertHandle (HandleData *hData)
{
  CanHandle hnd = -1;
  int i;

  os_if_mutex_lock(&handleMutex);

  for(i = 0; i < MAX_ARRAY_HANDLES; i++) {
    if (!handleArray[i]) {
      hData->handle = hnd = (CanHandle)i;
      handleArray[i] = hData;
      break;
    }
  }

  if (i == MAX_ARRAY_HANDLES) {
    if (listInsertFirst(&handleList, hData) == 0) {
      hData->handle = hnd = handleMax++;
    }
  }

  os_if_mutex_unlock(&handleMutex);

  return hnd;
}

static canStatus toStatus (int error)
{
  switch (error) {
  case 0:
    return canOK;
  case EINVAL:
    return canERR_PARAM;
  case ENOMEM:
    return canERR_NOMEM;
  case EAGAIN:
    return canERR_NOMSG;      // Sometimes overridden
  case EIO:
    return canERR_NOTFOUND;   // Not so good
  case ENODEV:
    return canERR_NOTFOUND;
  case EINTR:
    return canERR_INTERRUPTED;
  case EBADMSG:
    return canERR_PARAM;      // Used?
  case EACCES:
    return canERR_NO_ACCESS;
  case ETIMEDOUT:
    return canERR_TIMEOUT;
  default:
    return canERR_INTERNAL;   // Not so good
  }
}

static void test_cancel (void)
{
  pthread_testcancel();
}

static void notify (HandleData *hData)
{
  canNotifyData *notifyData = &hData->notifyData;

  if (hData->callback) {
    hData->callback(notifyData);
  } else if (hData->callback2) {
    hData->callback2(hData->handle, notifyData->tag, notifyData->eventType);
  }
}

//======================================================================
// Set CAN FD mode
//======================================================================
static canStatus kCanSetCanFD (HandleData *hData)
{
  int ret;
  KCAN_IOCTL_CANFD_T fd_data;

  memset(&fd_data, 0, sizeof(KCAN_IOCTL_CANFD_T));
  fd_data.fd = hData->fdMode;
  fd_data.action = CAN_CANFD_SET;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_CANFD, &fd_data, sizeof(KCAN_IOCTL_CANFD_T));
  if (ret != 0) {
    DEBUGPRINT((TXT("kCanSetCanFD failed!\n")));
    return toStatus(errno);
  }

  if (fd_data.status != CAN_CANFD_SUCCESS) {
    DEBUGPRINT((TXT("CAN FD / CAN Mismatch -> can't open channel. fd status %d\n"), fd_data.status));

    return canERR_NOTFOUND;
  }

  return canOK;
}


//======================================================================
// Notification thread
//======================================================================
static void *vCanNotifyThread (void *arg)
{
  VCAN_EVENT msg;

  HandleData    *hData      = (HandleData *)arg;
  canNotifyData *notifyData = &hData->notifyData;
  int           ret;

  // Get time to start with
  while (1) {
    test_cancel();    // Allow cancellation here

    ret = os_if_ioctl_read(hData->notifyFd, VCAN_IOC_RECVMSG,
                           &msg, sizeof(VCAN_EVENT));
    if (ret != 0 && errno == EAGAIN) {
      continue;            // Retry when no message received
    }

    // When this thread is cancelled, ioctl will be interrupted by a signal.
    if (ret != 0) {
      OS_IF_EXIT_THREAD(0);
    }

    if (msg.tag == V_CHIP_STATE) {
      struct s_vcan_chip_state *chipState    = &msg.tagData.chipState;
      notifyData->eventType                  = canEVENT_STATUS;
      notifyData->info.status.busStatus      = chipState->busStatus;
      notifyData->info.status.txErrorCounter = chipState->txErrorCounter;
      notifyData->info.status.rxErrorCounter = chipState->rxErrorCounter;
      notifyData->info.status.time           = (msg.timeStamp * 10UL) / (hData->timerResolution) ;
      notify(hData);
    } else if (msg.tag == V_RECEIVE_MSG) {
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_ERROR_FRAME) {
        if (hData->notifyFlags & canNOTIFY_ERROR) {
          notifyData->eventType = canEVENT_ERROR;
          notifyData->info.busErr.time = (msg.timeStamp * 10UL) / (hData->timerResolution) ;
          notify(hData);
        }
      } else if (msg.tagData.msg.flags & VCAN_MSG_FLAG_TXACK) {
        if (hData->notifyFlags & canNOTIFY_TX) {
          notifyData->eventType    = canEVENT_TX;
          notifyData->info.tx.id   = msg.tagData.msg.id;
          notifyData->info.tx.time = (msg.timeStamp * 10UL) / (hData->timerResolution) ;
          notify(hData);
        }
      } else {
        if (hData->notifyFlags & canNOTIFY_RX) {
          notifyData->eventType    = canEVENT_RX;
          notifyData->info.rx.id   = msg.tagData.msg.id;
          notifyData->info.tx.time = (msg.timeStamp * 10UL) / (hData->timerResolution) ;
          notify(hData);
        }
      }
    }
  }
}


//======================================================================
// vCanSetNotify
//======================================================================
static canStatus vCanSetNotify (HandleData *hData,
                                void (*callback) (canNotifyData *),
                                kvCallback_t callback2,
                                unsigned int notifyFlags)
{
  int one = 1;
  int ret;
  unsigned char newThread = 0;
  VCanMsgFilter filter;
  int timeout = 100; // Timeout in ms

  if (hData->notifyFd == OS_IF_INVALID_HANDLE) {
    newThread = 1;
    // Open an fd to read events from
    hData->notifyFd = os_if_open(hData->deviceName);
    if (hData->notifyFd != OS_IF_INVALID_HANDLE) {
      int ch = hData->channelNr;
      if (os_if_ioctl_write(hData->notifyFd, VCAN_IOC_OPEN_TRANSP,
                            &ch, sizeof(ch))) {
        OS_IF_CLOSE_HANDLE(hData->notifyFd);
        hData->notifyFd = OS_IF_INVALID_HANDLE;
      }
    }
    if (hData->notifyFd == OS_IF_INVALID_HANDLE) {
      goto error_open;
    }

    // Set blocking fileop
    os_if_ioctl_write(hData->notifyFd, VCAN_IOC_SET_READ_BLOCK,
                      &one, sizeof(one));

    // Set timeout in milliseconds
    os_if_ioctl_write(hData->notifyFd, VCAN_IOC_SET_READ_TIMEOUT,
                      &timeout, sizeof(timeout));

  }

  hData->notifyFlags = notifyFlags;

  // Set filters
  memset(&filter, 0, sizeof(VCanMsgFilter));
  filter.eventMask = 0;

  if ((notifyFlags & canNOTIFY_RX) ||
      (notifyFlags & canNOTIFY_TX) ||
      (notifyFlags & canNOTIFY_ERROR)) {
    filter.eventMask |= V_RECEIVE_MSG;
  }

  if (notifyFlags & canNOTIFY_STATUS) {
    filter.eventMask |= V_CHIP_STATE;
  }

  ret = os_if_ioctl_write(hData->notifyFd, VCAN_IOC_SET_MSG_FILTER,
                          &filter, sizeof(VCanMsgFilter));
  if (ret != 0) {
    goto error_ioc;
  }

  if (notifyFlags & canNOTIFY_TX) {
    int par = 1;
    ret = os_if_ioctl_write(hData->notifyFd, VCAN_IOC_SET_TXACK,
                            &par, sizeof(par));

    if (ret != 0) {
      goto error_ioc;
    }
  }

  if (newThread) {
    ret = os_if_ioctl_write(hData->fd, VCAN_IOC_FLUSH_RCVBUFFER, NULL, 0);
    if (ret != 0) {
      goto error_ioc;
    }

    ret = pthread_create(&hData->notifyThread, NULL, vCanNotifyThread, hData);
    if (ret != 0) {
      goto error_thread;
    }
  }

  hData->callback  = callback;
  hData->callback2 = callback2;
  return canOK;

error_thread:
error_ioc:
  OS_IF_CLOSE_HANDLE(hData->notifyFd);
  hData->notifyFd = OS_IF_INVALID_HANDLE;
error_open:
  return canERR_NOTFOUND;
}


//======================================================================
// vCanOpenChannel
//======================================================================
static canStatus vCanOpenChannel (HandleData *hData)
{
  int ret;
  VCanMsgFilter filter;
  uint32_t capability;

  hData->fd = os_if_open(hData->deviceName);
  if (hData->fd == OS_IF_INVALID_HANDLE) {
    return canERR_NOTFOUND;
  }

  if (hData->wantExclusive) {
    ret = os_if_ioctl_write(hData->fd, VCAN_IOC_OPEN_EXCL,
                            &hData->channelNr, sizeof(hData->channelNr));
  }
  else {
    ret = os_if_ioctl_write(hData->fd, VCAN_IOC_OPEN_CHAN,
                            &hData->channelNr, sizeof(hData->channelNr));
  }

  if (ret) {
    OS_IF_CLOSE_HANDLE(hData->fd);
    return canERR_NOTFOUND;
  }

  // VCAN_IOC_OPEN_CHAN sets channelNr to -1 if it fails
  if (hData->channelNr < 0) {
    OS_IF_CLOSE_HANDLE(hData->fd);
    return canERR_NOCHANNELS;
  }

  ret = os_if_ioctl_read(hData->fd, VCAN_IOC_GET_CHAN_CAP,
                           &capability, sizeof(capability));

  if (ret) {
    OS_IF_CLOSE_HANDLE(hData->fd);
    return canERR_NOTFOUND;
  }

  // save capabilities, this is a channel property rather than a handle proprty
  hData->capabilities = capability;

  if (!hData->acceptVirtual) {
    if (capability & VCAN_CHANNEL_CAP_VIRTUAL) {
      OS_IF_CLOSE_HANDLE(hData->fd);
      return canERR_NOTFOUND;
    }
  }

  if (capability & VCAN_CHANNEL_CAP_CANFD) {
    if ( canOK != kCanSetCanFD(hData) ) {
      OS_IF_CLOSE_HANDLE(hData->fd);
      return canERR_NOTFOUND;
    }
  }

  memset(&filter, 0, sizeof(VCanMsgFilter));
  // Read only CAN messages
  filter.eventMask = V_RECEIVE_MSG | V_TRANSMIT_MSG;
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_SET_MSG_FILTER,
                          &filter, sizeof(VCanMsgFilter));

  hData->timerScale = 1.0 / DEFAULT_TIMER_FACTOR;
  hData->timerResolution = (unsigned int)(10.0 / hData->timerScale);

  return canOK;
}

//======================================================================
// vCanBusOn
//======================================================================
static canStatus vCanBusOn (HandleData *hData)
{
  int ret;
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_BUS_ON, NULL, 0);
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


//======================================================================
// vCanBusOff
//======================================================================
static canStatus vCanBusOff (HandleData *hData)
{
  int ret;

  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_BUS_OFF, NULL, 0);
  if (ret != 0) {
    return canERR_INVHANDLE;
  }

  return canOK;
}

//======================================================================
// vCanSetBusparams
//======================================================================
static canStatus vCanSetBusParams (HandleData *hData,
                                   long freq,
                                   unsigned int tseg1,
                                   unsigned int tseg2,
                                   unsigned int sjw,
                                   unsigned int noSamp,
                                   long freq_brs,
                                   unsigned int tseg1_brs,
                                   unsigned int tseg2_brs,
                                   unsigned int sjw_brs,
                                   unsigned int syncmode)
{
  VCanBusParams busParams;
  int ret;

  (void)syncmode; // Unused.

  busParams.freq    = (signed long)freq;
  busParams.sjw     = sjw;
  busParams.tseg1   = tseg1;
  busParams.tseg2   = tseg2;
  busParams.samp3   = noSamp;   // This variable is # of samples inspite of name!

  busParams.freq_brs  = (signed long)freq_brs;
  busParams.sjw_brs   = sjw_brs;
  busParams.tseg1_brs = tseg1_brs;
  busParams.tseg2_brs = tseg2_brs;

  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_SET_BITRATE,
                          &busParams, sizeof(VCanBusParams));

  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


//======================================================================
// vCanGetBusParams
//======================================================================
static canStatus vCanGetBusParams(HandleData *hData,
                                  long *freq,
                                  unsigned int *tseg1,
                                  unsigned int *tseg2,
                                  unsigned int *sjw,
                                  unsigned int *noSamp,
                                  long *freq_brs,
                                  unsigned int *tseg1_brs,
                                  unsigned int *tseg2_brs,
                                  unsigned int *sjw_brs,
                                  unsigned int *syncmode)
{
  VCanBusParams busParams;
  int ret;

  ret = os_if_ioctl_read(hData->fd, VCAN_IOC_GET_BITRATE,
                         &busParams, sizeof(VCanBusParams));

  if (ret != 0) {
    return toStatus(errno);
  }

  if (freq)     *freq     = busParams.freq;
  if (sjw)      *sjw      = busParams.sjw;
  if (tseg1)    *tseg1    = busParams.tseg1;
  if (tseg2)    *tseg2    = busParams.tseg2;
  if (noSamp)   *noSamp   = busParams.samp3;

  if (freq_brs)  *freq_brs  = busParams.freq_brs;
  if (sjw_brs)   *sjw_brs   = busParams.sjw_brs;
  if (tseg1_brs) *tseg1_brs = busParams.tseg1_brs;
  if (tseg2_brs) *tseg2_brs = busParams.tseg2_brs;

  if (syncmode) *syncmode = 0;

  return canOK;
}

//======================================================================
// vCanReqBusStats
//======================================================================
static canStatus vCanReqBusStats(HandleData *hData)
{
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_REQ_BUS_STATS, NULL, 0)) {
    return toStatus(errno);
  }
  return canOK;
}

//======================================================================
// vCanGetBusStats
//======================================================================
static canStatus vCanGetBusStats(HandleData *hData, canBusStatistics *stat)
{
  VCanBusStatistics tstat;
  memset(stat, 0, sizeof(canBusStatistics));
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_BUS_STATS, &tstat, sizeof(VCanBusStatistics))) {
    return toStatus(errno);
  }

  stat->stdData = tstat.stdData;
  stat->stdRemote = tstat.stdRemote;
  stat->extData = tstat.extData;
  stat->extRemote = tstat.extRemote;
  stat->errFrame = tstat.errFrame;

  stat->busLoad = tstat.busLoad;
  stat->overruns = tstat.overruns;

  return canOK;
}


//======================================================================
// vCanReadInternal
//======================================================================
static canStatus vCanReadInternal (HandleData *hData, unsigned int iotcl_cmd,
                                   long *id,
                                   void *msgPtr, unsigned int *dlc,
                                   unsigned int *flag, unsigned long *time)
{
  int i;
  int ret;
  VCAN_EVENT msg;

  while (1) {
    ret = os_if_ioctl_read(hData->fd, iotcl_cmd, &msg, sizeof(VCAN_EVENT));
    if (ret != 0) {
      return toStatus(errno);
    }
    // Receive CAN message
    if (msg.tag == V_RECEIVE_MSG) {
      unsigned int flags;
      int count = 0;

      if (msg.tagData.msg.id & EXT_MSG) {
        flags = canMSG_EXT;
      } else {
        flags = canMSG_STD;
      }
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_ERROR_FRAME)
        flags = canMSG_ERROR_FRAME;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_FDF)
        flags |= canFDMSG_FDF;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_BRS)
        flags |= canFDMSG_BRS;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_ESI)
        flags |= canFDMSG_ESI;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_OVERRUN)
        flags |= canMSGERR_HW_OVERRUN | canMSGERR_SW_OVERRUN;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_REMOTE_FRAME)
        flags |= canMSG_RTR;
      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_TX_START)
        flags |= canMSG_TXRQ;

      if (flags & canFDMSG_FDF) {
        count = dlc_dlc_to_bytes_fd (msg.tagData.msg.dlc);
      } else {
        count = dlc_dlc_to_bytes_classic (msg.tagData.msg.dlc);
      }

      if (msg.tagData.msg.flags & VCAN_MSG_FLAG_SSM_NACK) {
        flags |= canMSG_TXNACK;
      } else if (msg.tagData.msg.flags & VCAN_MSG_FLAG_SSM_NACK_ABL) {
        flags |= canMSG_TXNACK;
        flags |= canMSG_ABL;
      } else {
        if (msg.tagData.msg.flags & VCAN_MSG_FLAG_TXACK) {
          flags |= canMSG_TXACK;
        }
      }


      // Copy data
      if (msgPtr) {
        for (i = 0; i < count; i++)
          ((unsigned char *)msgPtr)[i] = msg.tagData.msg.data[i];
      }

      // MSb is extended flag
      if (id)   *id   = msg.tagData.msg.id & ~EXT_MSG;
      if (dlc) {
        if (hData->acceptLargeDlc && !(flags & canFDMSG_FDF)) {
          *dlc = msg.tagData.msg.dlc;
        }
        else {
          *dlc  = count;
        }
      }
      if (time) *time = (msg.timeStamp * 10UL) / (hData->timerResolution) ;
      if (flag) *flag = flags;

      break;
    }
  }

  return canOK;
}


//======================================================================
// vCanRead
//======================================================================
static canStatus vCanRead (HandleData    *hData,
                           long          *id,
                           void          *msgPtr,
                           unsigned int  *dlc,
                           unsigned int  *flag,
                           unsigned long *time)
{
  int zero = 0;

  // Set non blocking fileop
  if (hData->readIsBlock) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_BLOCK, &zero, sizeof(zero));
    hData->readIsBlock = 0;
  }

  return vCanReadInternal(hData, VCAN_IOC_RECVMSG, id, msgPtr, dlc, flag, time);
}

//======================================================================
// vCanReadSpecific
//======================================================================
static canStatus vCanReadSpecific (HandleData    *hData,
                                   long          id,
                                   void          *msgPtr,
                                   unsigned int  *dlc,
                                   unsigned int  *flag,
                                   unsigned long *time)
{
  VCanReadSpecific cmd;

  cmd.skip    = READ_SPECIFIC_SKIP_MATCHING;
  cmd.id      = id;
  cmd.timeout = 0;

  os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_SPECIFIC, &cmd, sizeof(cmd));

  return vCanReadInternal(hData, VCAN_IOC_RECVMSG_SPECIFIC, NULL, msgPtr, dlc, flag, time);
}

//======================================================================
// vCanReadSpecificSkip
//======================================================================
static canStatus vCanReadSpecificSkip (HandleData    *hData,
                                       long          id,
                                       void          *msgPtr,
                                       unsigned int  *dlc,
                                       unsigned int  *flag,
                                       unsigned long *time)
{
  VCanReadSpecific cmd;

  cmd.skip    = READ_SPECIFIC_SKIP_PRECEEDING;
  cmd.id      = id;
  cmd.timeout = 0;

  os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_SPECIFIC, &cmd, sizeof(cmd));

  return vCanReadInternal(hData, VCAN_IOC_RECVMSG_SPECIFIC, NULL, msgPtr, dlc, flag, time);
}

//======================================================================
// vCanReaSyncdSpecific
//======================================================================
static canStatus vCanReadSyncSpecific (HandleData    *hData,
                                       long          id,
                                       unsigned long timeout)
{
 VCanReadSpecific cmd;

  cmd.skip    = READ_SPECIFIC_NO_SKIP;
  cmd.id      = id;
  cmd.timeout = timeout;

  os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_SPECIFIC, &cmd, sizeof(cmd));

  return vCanReadInternal(hData, VCAN_IOC_RECVMSG_SPECIFIC, NULL, NULL, NULL, NULL, NULL);
}

//======================================================================
// vCanReadWait
//======================================================================
static canStatus vCanReadWait (HandleData    *hData,
                               long          *id,
                               void          *msgPtr,
                               unsigned int  *dlc,
                               unsigned int  *flag,
                               unsigned long *time,
                               long          timeout)
{
  int one = 1;
  int timeout_int = timeout;

  if (timeout_int == 0) {
    return vCanRead(hData, id, msgPtr, dlc, flag, time);
  }

  // Set blocking fileop
  if (!hData->readIsBlock) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_BLOCK, &one, sizeof(one));
    hData->readIsBlock = 1;
  }

  // Set timeout in milliseconds
  if (hData->readTimeout != timeout_int) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_READ_TIMEOUT,
                      &timeout_int, sizeof(timeout_int));
    hData->readTimeout = timeout_int;
  }

  return vCanReadInternal(hData, VCAN_IOC_RECVMSG, id, msgPtr, dlc, flag, time);
}


//======================================================================
// vCanSetBusOutputControl
//======================================================================
static canStatus vCanSetBusOutputControl (HandleData *hData,
                                          unsigned int drivertype)
{
  int silent;
  int ret;

  switch (drivertype) {
  case canDRIVER_NORMAL:
    silent = 0;
    break;
  case canDRIVER_SILENT:
    silent = 1;
    break;
  default:
    return canERR_PARAM;
  }
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_SET_OUTPUT_MODE,
                          &silent, sizeof(silent));

  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


//======================================================================
// vCanGetBusOutputControl
//======================================================================
static canStatus vCanGetBusOutputControl (HandleData *hData,
                                          unsigned int *drivertype)
{
  int silent;
  int ret;

  ret = os_if_ioctl_read(hData->fd, VCAN_IOC_GET_OUTPUT_MODE,
                         &silent, sizeof(int));

  if (ret != 0) {
    return toStatus(errno);
  }

  switch (silent) {
  case 0:
    *drivertype = canDRIVER_NORMAL;
    break;
  case 1:
    *drivertype = canDRIVER_SILENT;
    break;
  default:
    break;
  }

  return canOK;
}


//======================================================================
// vCanAccept
//======================================================================
static canStatus vCanAccept(HandleData *hData,
                            const long envelope,
                            const unsigned int flag)
{
  VCanMsgFilter filter;
  int ret;

  // ret = ioctl(hData->fd, VCAN_IOC_GET_MSG_FILTER, &filter);
  ret = os_if_ioctl_read(hData->fd, VCAN_IOC_GET_MSG_FILTER,
                         &filter, sizeof(VCanMsgFilter));
  if (ret != 0) {
    return toStatus(errno);
  }

  switch (flag) {
  case canFILTER_SET_CODE_STD:
    filter.stdId   = envelope & ((1 << 11) - 1);
    break;
  case canFILTER_SET_MASK_STD:
    filter.stdMask = envelope & ((1 << 11) - 1);
    break;
  case canFILTER_SET_CODE_EXT:
    filter.extId   = envelope & ((1 << 29) - 1);
    break;
  case canFILTER_SET_MASK_EXT:
    filter.extMask = envelope & ((1 << 29) - 1);
    break;
  default:
    return canERR_PARAM;
  }
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_SET_MSG_FILTER,
                          &filter, sizeof(VCanMsgFilter));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


//======================================================================
// vCanWriteInternal
//======================================================================
static canStatus vCanWriteInternal(HandleData *hData, long id, void *msgPtr,
                                   unsigned int dlc, unsigned int flag)
{
  CAN_MSG msg;

  int ret;
  unsigned char sendExtended;
  unsigned int nbytes;
  unsigned int dlcFD;
  int msgFlags = 0;

  if      (flag & canMSG_STD) sendExtended = 0;
  else if (flag & canMSG_EXT) sendExtended = 1;
  else                        sendExtended = hData->isExtended;

  if  (( sendExtended && (id >= (1 << 29))) ||
       (!sendExtended && (id >= (1 << 11)))) {
    DEBUGPRINT((TXT("canERR_PARAM on line %d\n"), __LINE__));  // Was 3,
    return canERR_PARAM;
  }

  if (!dlc_is_dlc_ok (hData->acceptLargeDlc, (flag & canFDMSG_FDF), &dlc)) {
    return canERR_PARAM;
  }

  if (flag & canFDMSG_BRS && !(flag & canFDMSG_FDF))
  {
    // BRS is only allowed in CAN FD
    return canERR_PARAM;
  }

  if ((flag & canFDMSG_ESI) || (flag & canMSG_TXNACK) || (flag & canMSG_ABL)) {
    // ESI can only be received, not transmitted
    return canERR_PARAM;
  }

  if (flag & canFDMSG_FDF) {
    if (hData->fdMode) {
      msgFlags |= VCAN_MSG_FLAG_FDF;
    }
    else {
      return canERR_PARAM;
    }
  }

  if (flag & canMSG_SINGLE_SHOT) {
    if (! (hData->capabilities & VCAN_CHANNEL_CAP_SINGLE_SHOT)) {
      return canERR_NOT_SUPPORTED;
    }
    else {
      msgFlags |= VCAN_MSG_FLAG_SINGLE_SHOT;
    }
  }

  if (flag & canFDMSG_FDF) {
    dlcFD  = dlc_bytes_to_dlc_fd (dlc);
    nbytes = dlc_dlc_to_bytes_fd (dlcFD);
  }
  else {
    nbytes = dlc > 8 ? 8 : dlc;
    dlcFD = dlc;
  }

  if (sendExtended) {
    id |= EXT_MSG;
  }

  msg.id     = id;
  msg.length = dlcFD;
  msg.flags  = msgFlags;
  if (flag & canMSG_ERROR_FRAME) msg.flags |= VCAN_MSG_FLAG_ERROR_FRAME;
  if (flag & canMSG_RTR)         msg.flags |= VCAN_MSG_FLAG_REMOTE_FRAME;

  if (flag & canFDMSG_FDF)       msg.flags |= VCAN_MSG_FLAG_FDF;

  if (flag & canFDMSG_BRS)       msg.flags |= VCAN_MSG_FLAG_BRS;

  if (msgPtr) {
    memcpy(msg.data, msgPtr, nbytes);
  }

  // ret = ioctl(hData->fd, VCAN_IOC_SENDMSG, &msg);
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_SENDMSG, &msg, sizeof(CAN_MSG));

#if DEBUG
  if (ret == 0) {
    ;
  } else if (errno == EAGAIN) {
    DEBUGPRINT((TXT("VCAN_IOC_SENDMSG canERR_TXBUFOFL\n")));
  } else if (errno == EBADMSG) {
    DEBUGPRINT((TXT("VCAN_IOC_SENDMSG canERR_PARAM\n")));
  } else if (errno == EINTR) {
    DEBUGPRINT((TXT("VCAN_IOC_SENDMSG canERR_INTERRUPTED\n")));
  } else {
    DEBUGPRINT((TXT("VCAN_IOC_SENDMSG ERROR: %d\n"), errno));
  }
#endif

  if      (ret   == 0)       return canOK;
  else if (errno == EAGAIN)  return canERR_TXBUFOFL;
  else                       return toStatus(errno);
}


//======================================================================
// vCanWrite
//======================================================================
static canStatus vCanWrite (HandleData *hData, long id, void *msgPtr,
                            unsigned int dlc, unsigned int flag)
{
  int zero = 0;

  // Set non blocking fileop
  if (hData->writeIsBlock) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_WRITE_BLOCK, &zero, sizeof(zero));
  }
  hData->writeIsBlock = 0;

  return vCanWriteInternal(hData, id, msgPtr, dlc, flag);
}


//======================================================================
// vCanWriteWait
//======================================================================
static canStatus vCanWriteWait (HandleData *hData, long id, void *msgPtr,
                                unsigned int dlc, unsigned int flag,
                                long timeout)
{
  int one = 1;
  int timeout_int = timeout;

  if (timeout == 0) {
    return vCanWrite(hData, id, msgPtr, dlc, flag);
  }

  // Set blocking fileop
  if (!hData->writeIsBlock) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_WRITE_BLOCK, &one, sizeof(one));
  }
  hData->writeIsBlock = 1;

  // Set timeout in milliseconds
  if (hData->writeTimeout != timeout_int) {
    os_if_ioctl_write(hData->fd, VCAN_IOC_SET_WRITE_TIMEOUT,
                      &timeout_int, sizeof(timeout_int));
  }
  hData->writeTimeout = timeout_int;

  return vCanWriteInternal(hData, id, msgPtr, dlc, flag);
}


//======================================================================
// vCanWriteSync
//======================================================================
static canStatus vCanWriteSync (HandleData *hData, unsigned long timeout)
{
  int ret;
  ret = os_if_ioctl_write(hData->fd, VCAN_IOC_WAIT_EMPTY,
                          &timeout, sizeof(unsigned long));

  if      (ret   == 0)       return canOK;
  else if (errno == EAGAIN)  return canERR_TIMEOUT;
  else                       return toStatus(errno);
}


//======================================================================
// vCanReadTimer
//======================================================================
static canStatus vCanReadTimer (HandleData *hData, unsigned long *time)
{
  unsigned long tmpTime;

  if (!time) {
    return canERR_PARAM;
  }

  if (os_if_ioctl_read(hData->fd, VCAN_IOC_READ_TIMER,
                       &tmpTime, sizeof(unsigned long))) {
    return toStatus(errno);
  }
  *time = (tmpTime * 10UL) / (hData->timerResolution) ;

  return canOK;
}


//======================================================================
// vCanReadErrorCounters
//======================================================================
static canStatus vCanReadErrorCounters (HandleData *hData, unsigned int *txErr,
                                        unsigned int *rxErr, unsigned int *ovErr)
{
  if (txErr != NULL) {
    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_TX_ERR,
                         txErr, sizeof(unsigned int))) {
      goto ioc_error;
    }
  }
  if (rxErr != NULL) {
    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_RX_ERR,
                         rxErr, sizeof(unsigned int))) {
      goto ioc_error;
    }
  }
  if (ovErr != NULL) {
    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_OVER_ERR,
                         ovErr, sizeof(unsigned int))) {
      goto ioc_error;
    }
  }

  return canOK;

ioc_error:
  return toStatus(errno);
}


//======================================================================
// vCanReadStatus
//======================================================================
static canStatus vCanReadStatus (HandleData *hData, unsigned long *flags)
{
  int reply;

  if (flags == NULL) {
    return canERR_PARAM;
  }

  *flags = 0;

  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_CHIP_STATE, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply & CHIPSTAT_BUSOFF)        *flags  |= canSTAT_BUS_OFF;
  if (reply & CHIPSTAT_ERROR_PASSIVE) *flags  |= canSTAT_ERROR_PASSIVE;
  if (reply & CHIPSTAT_ERROR_WARNING) *flags  |= canSTAT_ERROR_WARNING;
  if (reply & CHIPSTAT_ERROR_ACTIVE)  *flags  |= canSTAT_ERROR_ACTIVE;

  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_TX_ERR, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply) {
    *flags |= canSTAT_TXERR;
  }
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_RX_ERR, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply) {
    *flags |= canSTAT_RXERR;
  }
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_OVER_ERR, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply) {
    *flags |= canSTAT_SW_OVERRUN | canSTAT_HW_OVERRUN;
  }
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_RX_QUEUE_LEVEL, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply) {
    *flags |= canSTAT_RX_PENDING;
  }
  if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_TX_QUEUE_LEVEL, &reply, sizeof(int))) {
    goto ioctl_error;
  }
  if (reply) {
    *flags |= canSTAT_TX_PENDING;
  }

  return canOK;

ioctl_error:
  return toStatus(errno);
}

//======================================================================
// vCanRequestChipStatus
//======================================================================
static canStatus vCanRequestChipStatus (HandleData *hData)
{
  // canRequestChipStatus is not needed in Linux, but in order to make code
  // portable between Windows and Linux, this dummy function was added.
  (void) hData;
  return canOK;
}

//======================================================================
// vCanGetChannelData
//======================================================================
static canStatus vCanGetChannelData (char *deviceName, int item,
                                     void *buffer, size_t bufsize)
{
  OS_IF_FILE_HANDLE fd;
  int err = 1;

  fd = os_if_open(deviceName);
  if (fd == OS_IF_INVALID_HANDLE) {
    DEBUGPRINT((TXT("Unable to open %s\n"), deviceName));
    return canERR_NOTFOUND;
  }

  switch (item) {
  case canCHANNELDATA_CARD_SERIAL_NO:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_SERIAL, buffer, bufsize);
    break;

  case canCHANNELDATA_CARD_UPC_NO:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_EAN, buffer, bufsize);
    break;

  case canCHANNELDATA_CARD_FIRMWARE_REV:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_FIRMWARE_REV, buffer, bufsize);
    break;

  case canCHANNELDATA_CARD_HARDWARE_REV:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_HARDWARE_REV, buffer, bufsize);
    break;

  case canCHANNELDATA_CHANNEL_CAP:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_CHAN_CAP, buffer, bufsize);
    if (!err) {
      *(uint32_t *)buffer = get_capabilities (*(uint32_t *)buffer);
    }
    break;

  case canCHANNELDATA_CHANNEL_CAP_MASK:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_CHAN_CAP_MASK, buffer, bufsize);
    if (!err) {
      *(uint32_t *)buffer = get_capabilities (*(uint32_t *)buffer);
    }
    break;

  case canCHANNELDATA_CARD_TYPE:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_CARD_TYPE, buffer, bufsize);
    break;

  case canCHANNELDATA_MAX_BITRATE:
    err = os_if_ioctl_read(fd, VCAN_IOC_GET_MAX_BITRATE, buffer, bufsize);
    break;

  case canCHANNELDATA_CUST_CHANNEL_NAME:
    {
      KCAN_IOCTL_GET_CUST_CHANNEL_NAME_T custChannelName;
      unsigned int maxCopySize;

      if (bufsize == 0) {
        return canERR_PARAM;
      }

      memset(buffer, 0, bufsize);
      maxCopySize = bufsize - 1; // Assure null termination.
      if (maxCopySize == 0) {
        return canOK;
      }

      if (maxCopySize > sizeof(custChannelName.data)) {
        maxCopySize = sizeof(custChannelName.data);
      }

      memset(&custChannelName, 0, sizeof(custChannelName));
      err = os_if_ioctl_read(fd, KCAN_IOCTL_GET_CUST_CHANNEL_NAME, &custChannelName, sizeof(custChannelName));
      if (!err) {
        memcpy(buffer, custChannelName.data, maxCopySize);
      }
    }
    break;

  default:
    OS_IF_CLOSE_HANDLE(fd);
    return canERR_PARAM;
  }

  OS_IF_CLOSE_HANDLE(fd);

  if (err) {
    DEBUGPRINT((TXT("Error on ioctl: %d / %d\n"), err, errno));
    return toStatus(errno);
  }

  return canOK;
}


//======================================================================
// vCanIoCtl
//======================================================================
static canStatus vCanIoCtl(HandleData *hData, unsigned int func,
                           void *buf, size_t buflen)
{
  switch(func) {
  case canIOCTL_GET_RX_BUFFER_LEVEL:
    // buf points at a uint32_t which receives the current RX queue level.
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_RX_QUEUE_LEVEL, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_GET_TX_BUFFER_LEVEL:
    // buf points at a uint32_t which receives the current TX queue level.
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_TX_QUEUE_LEVEL, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_FLUSH_RX_BUFFER:
    // Discard the current contents of the RX queue.
    if (os_if_ioctl_read(hData->fd, VCAN_IOC_FLUSH_RCVBUFFER, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_FLUSH_TX_BUFFER:
    //  Discard the current contents of the TX queue.
    if (os_if_ioctl_read(hData->fd, VCAN_IOC_FLUSH_SENDBUFFER, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_SET_TXACK:
    // buf points at a uint32_t which contains 0/1 to turn TXACKs on/ff
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_write(hData->fd, VCAN_IOC_SET_TXACK, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_GET_TXACK:
    // buf points at a uint32_t which receives current TXACKs setting
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_read(hData->fd, VCAN_IOC_GET_TXACK, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_SET_TXRQ:
    // buf points at a uint32_t which contains 0/1 to turn TXRQs on/ff
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_write(hData->fd, VCAN_IOC_SET_TXRQ, buf, buflen)) {
      return toStatus(errno);
    }
    break;
  case canIOCTL_SET_LOCAL_TXECHO:
    // buf points at an unsigned char which contains 0/1 to turn TXECHO on/ff
    if (check_args (buf, buflen, sizeof (uint8_t), ERROR_WHEN_LT)) {
      return canERR_PARAM;
    }

    if (os_if_ioctl_write(hData->fd, VCAN_IOC_SET_TXECHO, buf, buflen)) {
      return toStatus(errno);
    }
    break;

  case canIOCTL_RESET_OVERRUN_COUNT:
    if (os_if_ioctl_write(hData->fd, VCAN_IOC_RESET_OVERRUN_COUNT, NULL, 0)) {
      return toStatus(errno);
    }
    break;

  case canIOCTL_SET_TIMER_SCALE:
    {
      uint32_t t;
      //
      // t is the desired resolution in microseconds.
      // VCAN uses 10 us ticks, so we scale by 10 here.
      //
      if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
        return canERR_PARAM;
      }

      t = *(uint32_t *)buf;
      if (t == 0) {
        t = DEFAULT_TIMER_FACTOR * 10;
      }
      hData->timerScale = 10.0 / t;
      hData->timerResolution = t;
      break;
    }
  case canIOCTL_GET_TIMER_SCALE:
    //
    // Report the used resolution in microseconds.
    //
    if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
      return canERR_PARAM;
    }

    *(unsigned int *)buf = hData->timerResolution;
    break;


    case canIOCTL_TX_INTERVAL:
      if (check_args (buf, buflen, sizeof (uint32_t), ERROR_WHEN_NEQ)) {
        return canERR_PARAM;
      }

      if (os_if_ioctl_write(hData->fd, KCAN_IOCTL_TX_INTERVAL, buf, buflen)) {
        return toStatus(errno);
      }
      break;

    case canIOCTL_SET_BRLIMIT :
      if (check_args (buf, buflen, sizeof (long), ERROR_WHEN_NEQ)) {
        return canERR_PARAM;
      }

      if (os_if_ioctl_write(hData->fd, KCAN_IOCTL_SET_BRLIMIT, buf, buflen)) {
        return toStatus(errno);
      }

      break;
  default:
    return canERR_PARAM;
  }

  return canOK;
}


static canStatus kCanObjbufFreeAll (HandleData *hData)
{
  int ret;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_FREE_ALL, NULL, 0);
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}

static canStatus kCanObjbufAllocate (HandleData *hData, int type, int *number)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.type = type;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_ALLOCATE,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  *number = ioc.buffer_number;

  return canOK;
}


static canStatus kCanObjbufFree (HandleData *hData, int idx)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_FREE,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufWrite (HandleData *hData, int idx, int id, void* msg,
                                  unsigned int dlc, unsigned int flags)
{
  int                  retval;
  KCanObjbufBufferData ioc;

  if (flags & canFDMSG_BRS && !(flags & canFDMSG_FDF))
  {
    // BRS is only allowed in CAN FD
    return canERR_PARAM;
  }

  if (flags & canFDMSG_FDF) {
    if (!hData->fdMode) {
      return canERR_PARAM;
    }

    if (flags & canMSG_RTR) {
      return canERR_PARAM;
    }
  }

  if (!dlc_is_dlc_ok (hData->acceptLargeDlc, (flags & canFDMSG_FDF), &dlc)) {
    return canERR_PARAM;
  }

  if (flags & canMSG_EXT)  id |= EXT_MSG;

  ioc.flags = 0;
  if (flags & canMSG_RTR)    ioc.flags |= VCAN_AUTOTX_MSG_FLAG_REMOTE_FRAME;
  if (flags & canFDMSG_FDF)  ioc.flags |= VCAN_AUTOTX_MSG_FLAG_FDF;
  if (flags & canFDMSG_BRS ) ioc.flags |= VCAN_AUTOTX_MSG_FLAG_BRS;
  if (flags & canMSG_SINGLE_SHOT) {
    if (! (hData->capabilities & VCAN_CHANNEL_CAP_SINGLE_SHOT)) {
      return canERR_NOT_SUPPORTED;
    }
    else {
      ioc.flags |= VCAN_MSG_FLAG_SINGLE_SHOT;
    }
  }

  ioc.buffer_number = idx;
  ioc.id            = id;
  ioc.dlc           = dlc;
  if (msg) {
    memcpy(ioc.data, msg, sizeof(ioc.data));
  }

  retval = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_WRITE, &ioc, sizeof(ioc));

  if (retval != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufSetFilter (HandleData *hData, int idx,
                                      unsigned int code, unsigned int mask)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;
  ioc.acc_code      = code;
  ioc.acc_mask      = mask;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_SET_FILTER,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufSetFlags (HandleData *hData, int idx, unsigned int flags)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;
  ioc.flags         = flags;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_SET_FLAGS,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufSetPeriod (HandleData *hData, int idx, unsigned int period)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;
  ioc.period        = period;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_SET_PERIOD,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufSetMsgCount (HandleData *hData, int idx, unsigned int count)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;
  ioc.period        = count;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_SET_MSG_COUNT,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufSendBurst (HandleData *hData, int idx, unsigned int burstLen)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;
  ioc.period        = burstLen;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_SEND_BURST,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufEnable (HandleData *hData, int idx)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_ENABLE,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}


static canStatus kCanObjbufDisable (HandleData *hData, int idx)
{
  int ret;
  KCanObjbufAdminData ioc;

  ioc.buffer_number = idx;

  ret = os_if_ioctl_write(hData->fd, KCAN_IOCTL_OBJBUF_DISABLE,
                          &ioc, sizeof(ioc));
  if (ret != 0) {
    return toStatus(errno);
  }

  return canOK;
}

static uint32_t get_capabilities (uint32_t cap) {
  uint32_t i;
  uint32_t retval = 0;

  for(i = 0;
      i < sizeof(capabilities_table) / sizeof(capabilities_table[0]);
      i++) {
    if (cap & capabilities_table[i][0]) {
      retval |= capabilities_table[i][1];
    }
  }

  return retval;
}

static int check_args (void*buf, uint32_t buf_len, uint32_t limit, uint32_t method)
{
  if (!buf) {
    return -1;
  }

  switch (method) {
    case ERROR_WHEN_NEQ: if (buf_len != limit) return canERR_PARAM; break;
    case ERROR_WHEN_LT:  if (buf_len < limit) return canERR_PARAM; break;
    default: return canERR_PARAM; break;
  }
  return 0;
}



CANOps vCanOps = {
  // VCan Functions
  .setNotify           = vCanSetNotify,
  .openChannel         = vCanOpenChannel,
  .busOn               = vCanBusOn,
  .busOff              = vCanBusOff,
  .setBusParams        = vCanSetBusParams,
  .getBusParams        = vCanGetBusParams,
  .reqBusStats         = vCanReqBusStats,
  .getBusStats         = vCanGetBusStats,
  .read                = vCanRead,
  .readWait            = vCanReadWait,
  .readSpecific        = vCanReadSpecific,
  .readSpecificSkip    = vCanReadSpecificSkip,
  .readSyncSpecific    = vCanReadSyncSpecific,
  .setBusOutputControl = vCanSetBusOutputControl,
  .getBusOutputControl = vCanGetBusOutputControl,
  .accept              = vCanAccept,
  .write               = vCanWrite,
  .writeWait           = vCanWriteWait,
  .writeSync           = vCanWriteSync,
  .readTimer           = vCanReadTimer,
  .readErrorCounters   = vCanReadErrorCounters,
  .readStatus          = vCanReadStatus,
  .requestChipStatus   = vCanRequestChipStatus,
  .getChannelData      = vCanGetChannelData,
  .ioCtl               = vCanIoCtl,
  .objbufFreeAll       = kCanObjbufFreeAll,
  .objbufAllocate      = kCanObjbufAllocate,
  .objbufFree          = kCanObjbufFree,
  .objbufWrite         = kCanObjbufWrite,
  .objbufSetFilter     = kCanObjbufSetFilter,
  .objbufSetFlags      = kCanObjbufSetFlags,
  .objbufSetPeriod     = kCanObjbufSetPeriod,
  .objbufSetMsgCount   = kCanObjbufSetMsgCount,
  .objbufSendBurst     = kCanObjbufSendBurst,
  .objbufEnable        = kCanObjbufEnable,
  .objbufDisable       = kCanObjbufDisable,
};
