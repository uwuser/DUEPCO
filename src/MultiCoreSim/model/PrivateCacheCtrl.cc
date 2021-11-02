/*
 * File  :      MCoreSimProjectXml.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 17, 2020
 */

#include "PrivateCacheCtrl.h"
using namespace std;

namespace ns3
{

  // override ns3 type
  TypeId PrivateCacheCtrl::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::PrivateCacheCtrl")
                            .SetParent<Object>();
    return tid;
  }

  // private controller constructor
  PrivateCacheCtrl::PrivateCacheCtrl(uint32_t cachLines,
                                     Ptr<BusIfFIFO> assoicateBusIfFIFO,
                                     Ptr<CpuFIFO> associatedCpuFIFO,
                                     ns3::Ptr<ns3::GlobalQueue> globalqueue)
  {
    // default
    m_cacheType = 0;
    m_cacheSize = 64 * 256;
    m_cacheBlkSize = 64;
    m_nways = 1;
    m_duetto = false;
    m_nsets = 256;
    m_coreId = 1;
    m_sharedMemId = 10;
    m_cacheCycle = 1;
    m_dt = (1.0 / 1000000);
    m_clkSkew = 0;
    m_prllActionCnt = 0;
    m_reqWbRatio = 0;
    m_cach2Cache = false;
    m_logFileGenEnable = false;
    m_pType = CohProtType::SNOOP_MSI;
    m_maxPendingReq = 1;
    m_pendingCpuReq = 0;
    m_cache = CreateObject<GenericCache>(cachLines);
    m_GlobalQueue = globalqueue;
    m_busIfFIFO = assoicateBusIfFIFO;
    m_cpuFIFO = associatedCpuFIFO;
    m_cohProtocol = CreateObject<SNOOPPrivCohProtocol>();
    m_cpuPendingFIFO = CreateObject<GenericFIFO<PendingMsg>>();
    m_PendingWbFIFO.SetFifoDepth(assoicateBusIfFIFO->m_txMsgFIFO.GetFifoDepth());
    m_mode = "RT";
    m_reza_log_private = false;
  }

  // We don't do any dynamic allocations
  PrivateCacheCtrl::~PrivateCacheCtrl()
  {
  }

  // Set Maximum number of Pending CPU Request (OOO)
  void PrivateCacheCtrl::SetMaxPendingReq(int maxPendingReq)
  {
    m_maxPendingReq = maxPendingReq;
  }

  // Set Pending CPU FIFI depth
  void PrivateCacheCtrl::SetPendingCpuFIFODepth(int size)
  {
    m_cpuPendingFIFO->SetFifoDepth(size);
  }

  void PrivateCacheCtrl::SetCacheSize(uint32_t cacheSize)
  {
    m_cacheSize = cacheSize;
    m_cache->SetCacheSize(cacheSize);
  }

  uint32_t PrivateCacheCtrl::GetCacheSize()
  {
    return m_cacheSize;
  }

  unsigned PrivateCacheCtrl::createMask(unsigned a, unsigned b)
  {
    unsigned r = 0;
    for (unsigned i = a; i <= b; i++)
      r |= 1 << i;
    return r;
  }

  void PrivateCacheCtrl::SetCacheBlkSize(uint32_t cacheBlkSize)
  {
    m_cacheBlkSize = cacheBlkSize;
    m_cache->SetCacheBlkSize(cacheBlkSize);
  }

  uint32_t PrivateCacheCtrl::GetCacheBlkSize()
  {
    return m_cacheBlkSize;
  }

  void PrivateCacheCtrl::SetCacheNways(uint32_t nways)
  {
    m_nways = nways;
    m_cache->SetCacheNways(nways);
  }

  uint32_t PrivateCacheCtrl::GetCacheNways()
  {
    return m_nways;
  }

  bool PrivateCacheCtrl::isOldest(uint64_t adr, unsigned int coreIndex)
  {
    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg;
    int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
      m_GlobalQueue->m_GlobalOldestQueue.PopElement();
      if ((tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex && tempOldestReqMsg.msgId != 0) ||
          (tempOldestReqMsg.addr == adr && tempOldestReqMsg.wbCoreId == coreIndex && tempOldestReqMsg.msgId == 0))
      {
        success = true;
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
      }
      else
      {
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
      }
    }
    return success;
  }

  void PrivateCacheCtrl::SetCacheNsets(uint32_t nsets)
  {
    m_nsets = nsets;
    m_cache->SetCacheNsets(nsets);
  }

  uint32_t PrivateCacheCtrl::GetCacheNsets()
  {
    return m_nsets;
  }

  void PrivateCacheCtrl::SetCacheType(uint16_t cacheType)
  {
    m_cacheType = cacheType;
    m_cache->SetCacheType(cacheType);
  }

  uint16_t PrivateCacheCtrl::GetCacheType()
  {
    return m_cacheType;
  }

  void PrivateCacheCtrl::SetCoreId(int coreId)
  {
    m_coreId = coreId;
  }

  void PrivateCacheCtrl::SetSharedMemId(int sharedMemId)
  {
    m_sharedMemId = sharedMemId;
  }

  void PrivateCacheCtrl::SetIsDuetto(bool duetto)
  {
    m_duetto = duetto;
  }

  int PrivateCacheCtrl::GetCoreId()
  {
    return m_coreId;
  }

  void PrivateCacheCtrl::SetDt(double dt)
  {
    m_dt = dt;
  }

  void PrivateCacheCtrl::assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg &msg)
  {

    unsigned int WCL_0;
    unsigned int WCL_1;
    unsigned int WCL_2;
    WCL_0 = 473;
    WCL_1 = 473;
    WCL_2 = 473;
    if (msg.orderDetermined)
    {
      switch (msg.orderofArbitration)
      {
      case 0:
        msg.associateDeadline = WCL_0 - (m_cacheCycle - msg.becameOldest);
        break;
      case 1:
        msg.associateDeadline = WCL_1 - (m_cacheCycle - msg.becameOldest);
        break;
      case 2:
        msg.associateDeadline = WCL_2 - (m_cacheCycle - msg.becameOldest);
        break;
      }
      msg.associateDeadline_final = true;
    }
  }

  int PrivateCacheCtrl::GetDt()
  {
    return m_dt;
  }

  void PrivateCacheCtrl::SetClkSkew(double clkSkew)
  {
    m_clkSkew = clkSkew;
  }

  void PrivateCacheCtrl::SetReqWbRatio(int reqWbRatio)
  {
    m_reqWbRatio = reqWbRatio;
  }

  unsigned int PrivateCacheCtrl::retrieveCacheFIFOID(uint64_t id)
  {
    if (id == 0)
      return 10;
    else if (id == 1)
      return 11;
    else if (id == 2)
      return 12;
    else if (id == 3)
      return 13;
    else if (id == 4)
      return 14;
    else if (id == 5)
      return 15;
    else if (id == 6)
      return 16;
    else if (id == 7)
      return 17;
    else if (id == 8)
      return 18;
    else if (id == 9)
      return 19;
    else if (id == 10)
      return 20;
    else if (id == 11)
      return 21;
    else if (id == 12)
      return 22;
    else if (id == 13)
      return 23;
    else if (id == 14)
      return 24;
    else if (id == 15)
      return 25;
    cout << "retrieveCacheFIFOID cannot find  " << id << " as Shared Cache FIFO ID" << endl;
    abort();
    return 0;
  }

  void associateDeadlinef(ns3::BusIfFIFO::BusReqMsg msg)
  {
    if (msg.msgId == 0)
    {

      unsigned smallestWC = 200;
      msg.associateDeadline = smallestWC;
      msg.associateDeadline_final = false;
    }
    else
    {
      unsigned smallestWC = 200;
      msg.associateDeadline = smallestWC;
      msg.associateDeadline_final = false;
      //abort();
    }
  }

  void PrivateCacheCtrl::SetCache2Cache(bool cach2Cache)
  {
    m_cach2Cache = cach2Cache;
  }

  // Set Coherence Protocol Type
  void PrivateCacheCtrl::SetProtocolType(CohProtType ptype)
  {
    m_pType = ptype;
  }

  void PrivateCacheCtrl::SetLogFileGenEnable(bool logFileGenEnable)
  {
    m_logFileGenEnable = logFileGenEnable;
  }

  // insert new Transaction into BusTxMsg FIFO
  bool PrivateCacheCtrl::PushMsgInBusTxFIFO(uint64_t msgId,
                                            uint16_t reqCoreId,
                                            uint16_t wbCoreId,
                                            uint16_t transId,
                                            uint64_t addr,
                                            uint64_t sharedcacheid, // Modified to Accomodate Multi Shared Cache
                                            bool PendingWbBuf = false,
                                            bool NoGetMResp = false)
  {

    BusIfFIFO::BusReqMsg tempBusReqMsg;
    tempBusReqMsg.msgId = msgId;
    tempBusReqMsg.reqCoreId = reqCoreId;
    tempBusReqMsg.wbCoreId = wbCoreId;
    tempBusReqMsg.cohrMsgId = transId;
    tempBusReqMsg.sharedCacheAgent = sharedcacheid; // Modified to Accomodate Multi Shared Cache
    tempBusReqMsg.addr = addr;
    tempBusReqMsg.timestamp = m_cacheCycle * m_dt;
    tempBusReqMsg.cycle = m_cacheCycle;
    tempBusReqMsg.NoGetMResp = NoGetMResp;
    bool m_replacement = false;
    /**************** VERY IMPORTANT:: IF THE MSGid IS ZERO, IT MEANS THAT THIS TRANSACTION IS A REPLACEMENT ********************/
    if ((tempBusReqMsg.msgId != 0 && tempBusReqMsg.reqCoreId == 1) || (tempBusReqMsg.msgId == 0 && tempBusReqMsg.wbCoreId == 1))
      tempBusReqMsg.timestamp = 0;

    if (!m_busIfFIFO->m_txMsgFIFO.IsFull() && PendingWbBuf == false)
    {
      if (!m_duetto && m_mode == "FCFS")
      {
        m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
        return true;
      }
      else if ((!m_duetto && m_mode == "RT") || m_duetto == true)
      {

        // push the requestor in the RR global queue                                                 // Modified to Accomodate Multi Shared Cache
        if (msgId == 0)
        {
          m_replacement = true;
        }
        if (m_busIfFIFO->m_txMsgFIFO.IsEmpty())
        {

          bool check = true;
          bool con = false;
          if (m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() == 0)
          {
            check = false;
          }
          for (int itr1 = 0; itr1 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && con == false; itr1++)
          { // Modified to Accomodate Multi Shared Cache

            BusIfFIFO::BusReqMsg tempReq;
            tempReq = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement(); // Modified to Accomodate Multi Shared Cache
            m_GlobalQueue->m_GlobalOldestQueue.PopElement();                // Modified to Accomodate Multi Shared Cache
            if (!m_replacement)
            {
              if ((tempReq.msgId != 0 && tempReq.reqCoreId == reqCoreId) || (tempReq.msgId == 0 && tempReq.wbCoreId == reqCoreId))
              { // Modified to Accomodate Multi Shared Cache

                check = true; // Modified to Accomodate Multi Shared Cache
                con = true;
              }
              else
              {

                check = false;
              }
            }
            else
            {
              check = false;

              if ((tempReq.reqCoreId == wbCoreId && tempReq.msgId != 0) || (tempReq.wbCoreId == wbCoreId && tempReq.msgId == 0))
              {
                check = true;
                con = true;
              }
            }

            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempReq); // Modified to Accomodate Multi Shared Cache
          }

          if (!check)
          { // Modified to Accomodate Multi Shared Cache

            tempBusReqMsg.becameOldest = m_cacheCycle;
            associateDeadlinef(tempBusReqMsg);
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempBusReqMsg); // Modified to Accomodate Multi Shared Cache
            if (m_replacement)
            {
              m_GlobalQueue->m_GlobalRRQueue.push_back(wbCoreId); // Modified to Accomodate Multi Shared Cache
            }
            else
            {
              m_GlobalQueue->m_GlobalRRQueue.push_back(reqCoreId); // Modified to Accomodate Multi Shared Cache
            }
          }
        }
        else
        {
          //abort();
        }
        // push message into BusTxMsg FIFO

        m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
        return true;
      }
    }
    else if (!m_PendingWbFIFO.IsFull() && PendingWbBuf == true)
    {
      if (!m_duetto && m_mode == "FCFS")
      {
        m_PendingWbFIFO.InsertElement(tempBusReqMsg);
        return true;
      }
      else if ((!m_duetto && m_mode == "RT") || m_duetto == true)
      {
        if (m_replacement == true)
        {

          abort();
        }

        bool check = true;
        bool con = false;
        // push the requestor in the RR global queue                                                 // Modified to Accomodate Multi Shared Cache
        if (m_busIfFIFO->m_txRespFIFO.IsEmpty())
        {
          if (m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() == 0)
            check = false;
          for (int itr1 = 0; itr1 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && con == false; itr1++)
          { // Modified to Accomodate Multi Shared Cache
            BusIfFIFO::BusReqMsg tempReq;
            tempReq = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement(); // Modified to Accomodate Multi Shared Cache
            m_GlobalQueue->m_GlobalOldestQueue.PopElement();                // Modified to Accomodate Multi Shared Cache
            if (tempReq.wbCoreId == reqCoreId || tempReq.reqCoreId == reqCoreId)
            {               // Modified to Accomodate Multi Shared Cache
              check = true; // Modified to Accomodate Multi Shared Cache
              con = true;
            }
            else
            {
              check = false;
            }
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempReq); // Modified to Accomodate Multi Shared Cache
          }
          if (!check)
          { // Modified to Accomodate Multi Shared Cache

            tempBusReqMsg.becameOldest = m_cacheCycle;
            associateDeadlinef(tempBusReqMsg);
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempBusReqMsg); // Modified to Accomodate Multi Shared Cache
            m_GlobalQueue->m_GlobalRRQueue.push_back(reqCoreId);             // Modified to Accomodate Multi Shared Cache

          } // Modified to Accomodate Multi Shared Cache
        }
        // push message into BusTxMsg FIFO
        m_PendingWbFIFO.InsertElement(tempBusReqMsg);
        return true;
      }
    }
    else
    {
      if (m_logFileGenEnable)
      {
        std::cout << "Info: Cannot insert the Msg into BusTxMsg FIFO, FIFO is Full" << std::endl;
      }
      return false;
    }

    abort();
    return false;
  }

  bool PrivateCacheCtrl::MOESI_Modify_NoGetMResp_TxFIFO(uint64_t addr)
  {
    int pendingQueueSize = m_busIfFIFO->m_txMsgFIFO.GetQueueSize();
    bool ModifyDone = false;
    GenericCacheMapFrmt addrMap, TxBufAddrMap;
    addrMap = m_cache->CpuAddrMap(addr);
    BusIfFIFO::BusReqMsg pendingTxMsg;
    for (int i = 0; i < pendingQueueSize; i++)
    {
      pendingTxMsg = m_busIfFIFO->m_txMsgFIFO.GetFrontElement();
      m_busIfFIFO->m_txMsgFIFO.PopElement();
      TxBufAddrMap = m_cache->CpuAddrMap(pendingTxMsg.addr);
      if (addrMap.idx_set == TxBufAddrMap.idx_set && addrMap.tag == TxBufAddrMap.tag && pendingTxMsg.cohrMsgId == SNOOPPrivCohTrans::GetMTrans)
      {
        pendingTxMsg.NoGetMResp = false;
        ModifyDone = true;
      }
      m_busIfFIFO->m_txMsgFIFO.InsertElement(pendingTxMsg);
    }
    return ModifyDone;
  }

  // execute write back command
  bool PrivateCacheCtrl::DoWriteBack(uint64_t addr, uint16_t wbCoreId, uint64_t msgId, bool dualTrans = false)
  {
    if (!m_busIfFIFO->m_txRespFIFO.IsFull())
    {
      GenericCacheMapFrmt addrMap = m_cache->CpuAddrMap(addr);
      GenericCacheFrmt wbLine = m_cache->ReadCacheLine(addrMap.idx_set);
      BusIfFIFO::BusRespMsg wbMsg;
      wbMsg.reqCoreId = wbCoreId;
      wbMsg.respCoreId = m_coreId;
      wbMsg.addr = addr;
      wbMsg.timestamp = m_cacheCycle * m_dt;
      wbMsg.cycle = m_cacheCycle;
      wbMsg.msgId = msgId;
      wbMsg.dualTrans = dualTrans;
      if ((wbMsg.msgId != 0 && wbMsg.reqCoreId == 1) || (wbMsg.msgId == 0 && wbMsg.respCoreId == 1))
        wbMsg.timestamp = 0;
      if (m_logFileGenEnable)
      {
        std::cout << "DoWriteBack:: coreId = " << m_coreId << " requested Core = " << wbCoreId << "  dual: " << dualTrans << std::endl;
      }
      for (int i = 0; i < 8; i++)
        wbMsg.data[i] = wbLine.data[i];
      // push message into BusTxMsg FIFO
      m_busIfFIFO->m_txRespFIFO.InsertElement(wbMsg);

      return true;
    }
    else
    {
      if (m_logFileGenEnable)
      {
        std::cout << "Info: Cannot insert the Msg into BusTxResp FIFO, FIFO is Full" << std::endl;
        std::cout << "TxResp Buffer Size = " << m_busIfFIFO->m_txRespFIFO.GetQueueSize() << std::endl;
      }
      return false;
    }
  }

  // process pending buffer
  bool PrivateCacheCtrl::SendPendingWB(GenericCacheMapFrmt recvTrans, TransType type = MemOnly)
  {
    bool ReqSentFlag = false;

    // check if there is a pending write-back to this line in the pending buffer
    if (!m_PendingWbFIFO.IsEmpty())
    {
      BusIfFIFO::BusReqMsg pendingWbMsg;
      GenericCacheMapFrmt pendingWbAddrMap;
      int pendingQueueSize = m_PendingWbFIFO.GetQueueSize();
      for (int i = 0; i < pendingQueueSize; i++)
      {
        pendingWbMsg = m_PendingWbFIFO.GetFrontElement();
        pendingWbAddrMap = m_cache->CpuAddrMap(pendingWbMsg.addr);
        // Remove message from the busReq buffer
        m_PendingWbFIFO.PopElement();
        if (recvTrans.idx_set == pendingWbAddrMap.idx_set &&
            recvTrans.tag == pendingWbAddrMap.tag)
        {
          // b.3.2)send data to requestors
          //uint16_t reqCoreId = (type == CoreOnly || type == CorePlsMem) ? pendingWbMsg.reqCoreId : m_sharedMemId;
          uint16_t reqCoreId = (type == CoreOnly || type == CorePlsMem) ? pendingWbMsg.reqCoreId : pendingWbMsg.sharedCacheAgent;
          bool dualTrans = (type == CorePlsMem);

          // if this msg is not in the MType, meaning that someone take the responsibility, we should insert it assuming that it is oldest
          BusIfFIFO::BusReqMsg tempitrmt;
          bool existsmt = false;
          for (int itrmt = 0; itrmt < m_GlobalQueue->m_MsgType.GetQueueSize() && existsmt == false; itrmt++)
          {
            tempitrmt = m_GlobalQueue->m_MsgType.GetFrontElement();
            m_GlobalQueue->m_MsgType.PopElement();
            if (tempitrmt.msgId == pendingWbMsg.msgId && tempitrmt.addr == pendingWbMsg.addr && tempitrmt.reqCoreId == pendingWbMsg.reqCoreId)
            {
              existsmt = true;
              //m_GlobalQueue->m_MsgType.InsertElement(tempitrmt);
            }
            else
            {
              m_GlobalQueue->m_MsgType.InsertElement(tempitrmt);
            }
          }
          if (existsmt)
          {

            tempitrmt.orderDetermined = true;
            if (type == CoreOnly)
              tempitrmt.orderofArbitration = 2; // it was assumed to be WC, now we can relax
            // if((tempitrmt.msgId == 0 && isOldest(tempitrmt.addr,tempitrmt.wbCoreId)) || (tempitrmt.msgId != 0 && isOldest(tempitrmt.addr,tempitrmt.reqCoreId)))
            //   assignDeadlineAfterDetermination(tempitrmt);
            m_GlobalQueue->m_MsgType.InsertElement(tempitrmt);
          }

          if (!DoWriteBack(pendingWbMsg.addr, reqCoreId, pendingWbMsg.msgId, dualTrans))
          {
            std::cout << "PrivCache " << m_coreId << " TxResp FIFO is Full!" << std::endl;
            exit(0);
          }
          ReqSentFlag = true;
        }
        else
        {
          // b.3.1) Dequeue the data again into pending buffer
          m_PendingWbFIFO.InsertElement(pendingWbMsg);
        }
      }
    }
    return ReqSentFlag;
  }

  bool PrivateCacheCtrl::PendingCoreBufRemoveMsg(uint64_t msgId, PendingMsg &removedMsg)
  {
    bool removalFlg = false;
    int pendingBufSize = m_cpuPendingFIFO->GetQueueSize();
    PendingMsg cpuPendingMsg;
    for (int i = 0; i < pendingBufSize; i++)
    {
      cpuPendingMsg = m_cpuPendingFIFO->GetFrontElement();
      m_cpuPendingFIFO->PopElement();
      if (m_logFileGenEnable)
      {
        std::cout << "PendingCpuBuffer: Entery = " << i << ", core Id = " << m_coreId << " cpuMsg.msgId = " << cpuPendingMsg.cpuMsg.msgId << " request.msgId = " << msgId << ", addr = " << cpuPendingMsg.cpuMsg.addr << " IsProcessed = " << cpuPendingMsg.IsProcessed << std::endl;
      }
      // Remove message that has same Id from the pending buffer
      if (cpuPendingMsg.cpuMsg.msgId == msgId)
      {
        removalFlg = true;
        removedMsg = cpuPendingMsg;
        if (m_logFileGenEnable)
        {
          std::cout << "PendingCpuBuffer: Core Id = " << m_coreId << " New message get removed from the Pending Buffer, Pending Cnt " << m_pendingCpuReq << std::endl;
        }
      }
      else
      {
        m_cpuPendingFIFO->InsertElement(cpuPendingMsg); // dequeue
      }
    }
    return removalFlg;
  }

  // This function does most of the functionality.
  void PrivateCacheCtrl::CacheCtrlMain()
  {

    PendingMsg cpuPendingMsg; // Message pushed into CPU's pending buffer

    // Check Coherence Protocol Events (i.e. Core, Request-Bus, and Response-Bus)
    m_cohProtocol->ChkCohEvents();

    // Insert Core's requests into pending buffer
    if (m_cohProtocol->GetCpuReqEvent() != SNOOPPrivCoreEvent::Null && m_pendingCpuReq < m_maxPendingReq)
    {

      if (!m_cpuPendingFIFO->IsFull())
      {

        // remove request from CpuReq FIFO
        m_cpuFIFO->m_txFIFO.PopElement();
        // insert message into pending-queue
        cpuPendingMsg.IsProcessed = false;
        cpuPendingMsg.IsPending = false;
        cpuPendingMsg.cpuMsg = m_cohProtocol->GetCpuReqMsg();
        cpuPendingMsg.cpuReqEvent = m_cohProtocol->GetCpuReqEvent();
        m_cpuPendingFIFO->InsertElement(cpuPendingMsg);
        m_pendingCpuReq++;
        if (m_logFileGenEnable)
        {
          std::cout << "Core Id = " << m_coreId << " New CPU Request get inserted into pending buffer, number of pending requests  = " << m_pendingCpuReq << std::endl;
        }
      }
    }

    // fetch a new request from pending buffer to process it
    m_cohProtocol->SetCpuReqEvent(SNOOPPrivCoreEvent::Null);

    int pendingBufSize = m_cpuPendingFIFO->GetQueueSize();
    for (int i = 0; i < pendingBufSize; i++)
    {

      cpuPendingMsg = m_cpuPendingFIFO->GetFrontElement();

      if (cpuPendingMsg.IsProcessed == false)
      {

        m_cohProtocol->SetCpuReqMsg(cpuPendingMsg.cpuMsg);
        m_cohProtocol->SetCpuReqEvent(cpuPendingMsg.cpuReqEvent);
        break;
      }

      // dequeuing message
      m_cpuPendingFIFO->PopElement();
      m_cpuPendingFIFO->InsertElement(cpuPendingMsg);
    }

    // Get Cache line information for all event
    m_cohProtocol->GetEventsCacheInfo();

    // Check Which Event to be executed first
    SNOOPPrivEventType currProcEvent;
    m_cohProtocol->CohEventsSerialize();
    currProcEvent = m_cohProtocol->GetCurrProcEvent();

    // Run Coherence Protocol FSM for the current Event
    if (currProcEvent == SNOOPPrivEventType::Core)
    {

      m_cohProtocol->ProcessCoreEvent();
    }
    else if (currProcEvent == SNOOPPrivEventType::ReqBus ||
             currProcEvent == SNOOPPrivEventType::RespBus)
    {

      m_cohProtocol->ProcessBusEvents();
    }

    // Get current processed event action
    SNOOPPrivCtrlAction currEventCtrlAction;
    currEventCtrlAction = m_cohProtocol->GetCurrEventCtrlAction();

    // Get current processed event transaction
    SNOOPPrivCohTrans currEventCohrTrans;
    currEventCohrTrans = m_cohProtocol->GetCurrEventCohrTrans();
    // Get next state of the current process event
    int currEventCacheNextState;
    currEventCacheNextState = m_cohProtocol->GetCurrEventCacheNextState();

    // Get cache line information of the current process event
    GenericCache::CacheLineInfo currEventCacheLineInfo;
    currEventCacheLineInfo = m_cohProtocol->GetCurrEventCacheLineInfo();

    // Get received message of the current process event
    CpuFIFO ::ReqMsg cpuReqMsg = {};
    BusIfFIFO::BusReqMsg busReqMsg = {};
    BusIfFIFO::BusRespMsg busRespMsg = {};

    cpuReqMsg = m_cohProtocol->GetCpuReqMsg();
    busReqMsg = m_cohProtocol->GetBusReqMsg();
    busRespMsg = m_cohProtocol->GetBusRespMsg();

    // received cache line infomration
    GenericCacheFrmt cacheLineInfo;
    GenericCacheMapFrmt addrMap;

    // cpu RxResp buffer message
    CpuFIFO::RespMsg CpuResp;

    // cpuReq Event
    SNOOPPrivCoreEvent cpuReqEvent;

    cpuReqEvent = m_cohProtocol->GetCpuReqEvent();

    // Print Cache Line information
    if (m_logFileGenEnable)
    {
      if (!(currProcEvent == SNOOPPrivEventType::Core && currEventCtrlAction == SNOOPPrivCtrlAction::Stall))
      {
        if (currProcEvent != SNOOPPrivEventType::Null)
          std::cout << "SNOOPPrivCohProtocol: CoreId = " << m_coreId << " Tick ================================================================== " << m_cacheCycle << std::endl;
        m_cohProtocol->PrintEventInfo();
      }
    }

    /************************
        * Process RespBus Event
        ***********************/
    if (currProcEvent == SNOOPPrivEventType::RespBus)
    {

      /* a) CopyThenHit ACK Processing
          */
      if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHit ||
          currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitWB ||
          currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly ||
          currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly ||
          currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem)
      {
        // So far, we start by processing the RESP event and then here based on the coherency protocl the action is determined.
        // Since all of these are data hit meaning that data came from the interface to this core, first they need to be stored
        // in this private cache.

        // 1) Remove message from the busResp buffer
        m_busIfFIFO->m_rxRespFIFO.PopElement();

        // 2) Update Private Cache Line
        addrMap = m_cache->CpuAddrMap(busRespMsg.addr);
        cacheLineInfo.state = currEventCacheNextState;
        cacheLineInfo.valid = m_cohProtocol->IsCacheBlkValid(currEventCacheNextState);
        cacheLineInfo.tag = addrMap.tag;
        for (int i = 0; i < 8; i++)
          cacheLineInfo.data[i] = busRespMsg.data[i];

        m_cohProtocol->UpdateCacheLine(Line,
                                       cacheLineInfo,
                                       currEventCacheLineInfo.set_idx,
                                       currEventCacheLineInfo.way_idx);

        // 3) Remove message from core's pending-buffer
        // Now, we need to check the pending core buffer for the request that asked for this data. This requet is finihsed
        // at this point so it should be removed from the buffer
        if (PendingCoreBufRemoveMsg(busRespMsg.msgId, cpuPendingMsg) == true)
        {

          CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
          CpuResp.cycle = m_cacheCycle;
          CpuResp.msgId = cpuPendingMsg.cpuMsg.msgId;
          CpuResp.addr = cpuPendingMsg.cpuMsg.addr;
          m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
          m_pendingCpuReq--;
        }
        else
        {
          std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
          exit(0);
        }

        if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitWB ||
            currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly ||
            currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly ||
            currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem)
        {
          // the type of the write back response is Data  therefore resp. CorePlusMem is for dual trans. The requCoreID of
          // all of these types is this core and the respCoreID could be shared memory or core depending on the type where
          // the write back is going to (mem or core)
          // CopyThenHitSendCoreOnly which means that send core only and the requested core: this only happens when you have
          // core2core capabilty. Notice the pendingWbMsg.reqCoreId could be "any" core that is waiting for this data
          TransType type = (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly) ? CoreOnly : (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly) ? MemOnly
                                                                                                          : (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem)   ? CorePlsMem
                                                                                                                                                                                   : CorePlsMem;

          if (!SendPendingWB(addrMap, type))
          {
            if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly)
            {
              // Do write back to memory

              if (!DoWriteBack(busRespMsg.addr, busRespMsg.sharedCacheAgent, busRespMsg.msgId, false))
              {

                std::cout << "3This is will cause stall in the PMSI state machine !!!!" << std::endl;
                exit(0);
              }
            }
            else
            {
              std::cout << "PrivCache: No data send to core !!!!" << std::endl;
              exit(0);
            }
          }
        }
      }
      /*
          * b) No Action Processing
          */
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck)
      {
        // b.1) Remove "other" DataResp Msgs
        m_busIfFIFO->m_rxRespFIFO.PopElement();
      }
      /*
          * c) Fault Action Processing
          */
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault)
      {
        if (m_logFileGenEnable)
        {
          std::cout << "DataResp occur in illegal state!" << std::endl;
        }
        exit(0);
      }
      /*
          * c) Check For Uncovered scenario
          */
      else if (currEventCtrlAction != SNOOPPrivCtrlAction::NullAck)
      {
        if (m_logFileGenEnable)
        {
          std::cout << "DataResp uncovered Events occured!" << std::endl;
        }
        exit(0);
      }
      // mark busResp Action as processed
      currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
    }

    /*
        * Process CpuReq action
        */
    if (currProcEvent == SNOOPPrivEventType::Core)
    {

      if (currEventCtrlAction == SNOOPPrivCtrlAction::Hit)
      {
        m_GlobalQueue->m_totalL1Hit++;
        if (m_coreId == 0)
          m_GlobalQueue->m_L1Hit_0++;
        else if (m_coreId == 1)
          m_GlobalQueue->m_L1Hit_1++;
        else if (m_coreId == 2)
          m_GlobalQueue->m_L1Hit_2++;
        else if (m_coreId == 3)
          m_GlobalQueue->m_L1Hit_3++;
        else if (m_coreId == 4)
          m_GlobalQueue->m_L1Hit_4++;
        else if (m_coreId == 5)
          m_GlobalQueue->m_L1Hit_5++;
        else if (m_coreId == 6)
          m_GlobalQueue->m_L1Hit_6++;
        else if (m_coreId == 7)
          m_GlobalQueue->m_L1Hit_7++;

        // Remove message from core's pending-buffer
        // If it is a hit request to this private cache then it should be removed from the pending buffer of this core and then
        // send it to the core itself (core RX interface FIFO)

        if (PendingCoreBufRemoveMsg(cpuReqMsg.msgId, cpuPendingMsg) == true)
        {

          m_pendingCpuReq--;
          CpuResp.reqcycle = cpuReqMsg.cycle;
          CpuResp.addr = cpuReqMsg.addr;
          CpuResp.cycle = m_cacheCycle;
          CpuResp.msgId = cpuReqMsg.msgId;
          // Push a new response msg into CpuRx FIFO
          m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
        }
        else
        {
          std::cout << "Core Id = " << m_coreId << " Hit Response for CPU Request not found in the Pending Buffer" << std::endl;
          exit(0);
        }
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTrans)
      {

        // Cannot remove Cpu request unless hit or data response happen
        if (cpuReqEvent != SNOOPPrivCoreEvent::Replacement)
        {

          cpuPendingMsg.IsProcessed = true;
          m_cpuPendingFIFO->UpdateFrontElement(cpuPendingMsg);
        }

        unsigned int mask = createMask(6, 8);
        unsigned bank_num = mask & cpuReqMsg.addr;
        bank_num = bank_num >> 6;
        cpuReqMsg.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

        uint16_t wbCoreId = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? m_coreId : cpuReqMsg.sharedCacheAgent;
        uint16_t reqCoreId = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? cpuReqMsg.sharedCacheAgent : m_coreId;
        uint64_t msgId = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? 0 : cpuReqMsg.msgId;

        bool NoGetMResp = false;
        if (m_pType == CohProtType::SNOOP_MOESI && currEventCohrTrans == SNOOPPrivCohTrans::GetMTrans)
        {

          NoGetMResp = (static_cast<SNOOP_MOESIPrivCacheState>(m_cohProtocol->GetCurrEventCacheCurrState()) == SNOOP_MOESIPrivCacheState::O);
        }

        if (!PushMsgInBusTxFIFO(msgId, reqCoreId, wbCoreId, currEventCohrTrans, cpuReqMsg.addr, cpuReqMsg.sharedCacheAgent, false, NoGetMResp))
        {

          std::cout << "PrivCache " << m_coreId << " TxReq FIFO is Full!" << std::endl;
          exit(0);
        }
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::Stall)
      {
        /* No action is required          
            */
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck)
      {

        /* No action is required, just 
            * update state
            */
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault)
      {

        // illegal Event at this state
        std::cout << "PrivCache " << m_coreId << " CoreEvent occur in illegal state!" << std::endl;
        exit(0);
      }
      // Check For Uncovered actions
      else
      {
        std::cout << "PrivCache " << m_coreId << " uncovered Events occured!" << std::endl;
        exit(0);
      }

      // Update cache line state
      if (currEventCtrlAction != SNOOPPrivCtrlAction::Stall)
      {

        // 2) Update Private Cache Line
        addrMap = m_cache->CpuAddrMap(cpuReqMsg.addr);
        cacheLineInfo.state = currEventCacheNextState;
        cacheLineInfo.valid = m_cohProtocol->IsCacheBlkValid(currEventCacheNextState);
        cacheLineInfo.tag = addrMap.tag;
        for (int i = 0; i < 8; i++)
          cacheLineInfo.data[i] = cpuReqMsg.data[i];

        m_cohProtocol->UpdateCacheLine(Line,
                                       cacheLineInfo,
                                       currEventCacheLineInfo.set_idx,
                                       currEventCacheLineInfo.way_idx);
      }
      // Mark Cpu action as processed
      currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
    }

    /*
        * Process BusReq action
        */
    if (currProcEvent == SNOOPPrivEventType::ReqBus)
    {

      if (currEventCtrlAction == SNOOPPrivCtrlAction::Hit)
      {
        // Remove message from the busReq buffer
        m_busIfFIFO->m_rxMsgFIFO.PopElement();

        // Remove message from core's pending-buffer
        if (PendingCoreBufRemoveMsg(busReqMsg.msgId, cpuPendingMsg) == true)
        {
          CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
          CpuResp.cycle = m_cacheCycle;
          CpuResp.msgId = cpuPendingMsg.cpuMsg.msgId;
          CpuResp.addr = cpuPendingMsg.cpuMsg.addr;
          m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
          m_pendingCpuReq--;
        }
        else
        {
          std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
          exit(0);
        }
      }
      // This condition happens in write back states
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::WritBack ||
               currEventCtrlAction == SNOOPPrivCtrlAction::SendMemOnly ||
               currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreOnly ||
               currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem ||
               currEventCtrlAction == SNOOPPrivCtrlAction::HitSendMemOnly)
      {

        if (currEventCtrlAction == SNOOPPrivCtrlAction::SendMemOnly)
        {

          busReqMsg.orderDetermined = true;
          busReqMsg.orderofArbitration = 1;
          busReqMsg.currStage = "REQ";
          assignDeadlineAfterDetermination(busReqMsg);
          m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
        }
        else if (currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreOnly)
        {

          busReqMsg.orderDetermined = true;
          busReqMsg.orderofArbitration = 2;
          busReqMsg.currStage = "REQ";
          assignDeadlineAfterDetermination(busReqMsg);
          m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
        }
        else if (currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem)
        {

          busReqMsg.orderDetermined = true;
          busReqMsg.orderofArbitration = 1;
          busReqMsg.currStage = "REQ";
          assignDeadlineAfterDetermination(busReqMsg);
          m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
        }

        if (currEventCtrlAction == SNOOPPrivCtrlAction::HitSendMemOnly)
        { // do hit
          // Remove message from core's pending-buffer
          if (PendingCoreBufRemoveMsg(busReqMsg.msgId, cpuPendingMsg) == true)
          {
            CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
            CpuResp.cycle = m_cacheCycle;
            CpuResp.msgId = cpuPendingMsg.cpuMsg.msgId;
            CpuResp.addr = cpuPendingMsg.cpuMsg.addr;
            m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
            m_pendingCpuReq--;
          }
          else
          {
            std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
            exit(0);
          }
        }

        // // Remove message from the busReq buffer
        // m_busIfFIFO->m_rxMsgFIFO.PopElement();

        // Do write back  -- m_sharedMemId
        uint16_t reqCoreId = (currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreOnly ||
                              currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem)
                                 ? busReqMsg.reqCoreId
                                 : busReqMsg.sharedCacheAgent;
        bool dualTrans = (currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem);

        //  if(currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem) {
        //    abort();
        //  }
        if (!DoWriteBack(busReqMsg.addr, reqCoreId, busReqMsg.msgId, dualTrans))
        {
          std::cout << "2This is will cause stall in the PMSI state machine !!!!" << std::endl;
          //exit(0);
        }
        else
        {
          // Remove message from the busReq buffer
          m_busIfFIFO->m_rxMsgFIFO.PopElement();
        }

        if (m_pType == CohProtType::SNOOP_MOESI && m_cohProtocol->GetBusReqEvent() == SNOOPPrivReqBusEvent::OtherGetM)
        {
          if (static_cast<SNOOP_MOESIPrivCacheState>(m_cohProtocol->GetCurrEventCacheCurrState()) == SNOOP_MOESIPrivCacheState::OM_a)
          {
            if (MOESI_Modify_NoGetMResp_TxFIFO(busReqMsg.addr) == false)
            {
              std::cout << "PrivateCache: MOESI Fatal Error!!!!" << std::endl;
              exit(0);
            }
          }
        }
      }
      // This condition occur in PutM()
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTrans ||
               currEventCtrlAction == SNOOPPrivCtrlAction::issueTransSaveWbId)
      {
        // Remove message from the busReq buffer
        m_busIfFIFO->m_rxMsgFIFO.PopElement();
        if (currEventCohrTrans != SNOOPPrivCohTrans::PutMTrans)
        {
          std::cout << "Error This should be PutM() transaction, coreId =  " << m_coreId << " transaction = " << currEventCohrTrans << std::endl;
          exit(0);
        }

        //uint16_t reqCoreId = (m_cohProtocol->GetBusReqEvent () == SNOOPPrivReqBusEvent::OwnInvTrans) ? m_sharedMemId : busReqMsg.reqCoreId;
        uint16_t reqCoreId = (m_cohProtocol->GetBusReqEvent() == SNOOPPrivReqBusEvent::OwnInvTrans) ? busReqMsg.sharedCacheAgent : busReqMsg.reqCoreId;
        if (!PushMsgInBusTxFIFO(busReqMsg.msgId, reqCoreId, m_coreId, currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, false))
        {
          std::cout << "1This is will cause stall in the PMSI state machine !!!!" << std::endl;
          exit(0);
        }
        if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTransSaveWbId)
        {
          if (!PushMsgInBusTxFIFO(busReqMsg.msgId, busReqMsg.reqCoreId, m_coreId, currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, true))
          {
            std::cout << "PrivCache: Pending Wb buffer is full !!!!" << std::endl;
            exit(0);
          }
        }
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::SaveWbCoreId)
      {
        m_busIfFIFO->m_rxMsgFIFO.PopElement();
        // save pending write back coreId & address

        busReqMsg.orderDetermined = true;
        busReqMsg.orderofArbitration = 1; // since we don't know what will be the arbitration when sending the write back, let's consider it as "1". Then
        // after WB is specified, we can keep it as is or change it back to "2" which is a more relax consitraint.
        busReqMsg.currStage = "REQ";

        assignDeadlineAfterDetermination(busReqMsg);

        m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);

        if (!PushMsgInBusTxFIFO(busReqMsg.msgId, busReqMsg.reqCoreId, m_coreId, currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, true))
        {
          std::cout << "PrivCache: Pending Wb buffer is full !!!!" << std::endl;
          exit(0);
        }
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck)
      {
        if (!m_duetto && m_mode == "FCFS")
        {
          // remove no-action event
          m_busIfFIFO->m_rxMsgFIFO.PopElement();
        }
        else if ((!m_duetto && m_mode == "RT") || m_duetto == true)
        {

          // remove no-action event

          /*** if the req msg is replacement and there is no Ack, it means that there is no need
              * for replacement anymore, this, you should remove the request from the global oldest
              * and global queue ***/
          BusIfFIFO::BusReqMsg m_replacement_temp;
          m_replacement_temp = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();
          m_busIfFIFO->m_rxMsgFIFO.PopElement();
          if (m_replacement_temp.msgId == 0 && m_replacement_temp.wbCoreId == m_busIfFIFO->m_fifo_id)
          { /*** va own bashe na other**/

            // Remove from Oldest queue
            bool success = false;
            BusIfFIFO::BusReqMsg tempOldestReqMsg;
            int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
            for (int itr = 0; itr < queueSize && success == false; itr++)
            {
              tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
              m_GlobalQueue->m_GlobalOldestQueue.PopElement();

              if (tempOldestReqMsg.msgId == 0 && tempOldestReqMsg.addr == m_replacement_temp.addr && tempOldestReqMsg.wbCoreId == m_replacement_temp.wbCoreId)
              {
                success = true;
                // remove from the RR order list //
                /** make sure if you dont need to adjust the oldest queue **/
                for (unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size(); h++)
                {
                  if (m_GlobalQueue->m_GlobalRRQueue.at(h) == m_replacement_temp.wbCoreId)
                  {
                    m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                  }
                }
              }
              if (success == false)
                m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
            }

            // remove from non oldest global queue
            success = false;
            BusIfFIFO::BusReqMsg tempNonOldestReqMsg;
            queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
            for (int itr = 0; itr < queueSize && success == false; itr++)
            {
              tempNonOldestReqMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
              m_GlobalQueue->m_GlobalReqFIFO.PopElement();
              if (tempNonOldestReqMsg.msgId == 0 && tempNonOldestReqMsg.addr == m_replacement_temp.addr && tempNonOldestReqMsg.wbCoreId == m_replacement_temp.wbCoreId)
              {
                success = true;
              }
              if (success == false)
                m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsg);
            }
          }
        }
      }
      else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault)
      {
        // genenerate error message
        std::cout << "BusReqEvent occur in illegal state!" << std::endl;
        exit(0);
      }
      // Check For Uncovered scenario
      else
      {
        std::cout << "busReqEvent " << m_coreId << " uncovered Events occured!" << std::endl;
        exit(0);
      }

      // Update Private Cache Line
      if (currEventCacheLineInfo.IsExist == true)
      {
        cacheLineInfo.state = currEventCacheNextState;
        cacheLineInfo.valid = m_cohProtocol->IsCacheBlkValid(currEventCacheNextState);
        m_cohProtocol->UpdateCacheLine(State,
                                       cacheLineInfo,
                                       currEventCacheLineInfo.set_idx,
                                       currEventCacheLineInfo.way_idx);
      }
      // mark action as processed
      currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
    }

  } // PrivateCacheCtrl::CacheCtrlMain ()

  void PrivateCacheCtrl::CycleProcess()
  {

    CacheCtrlMain();
    // Schedule the next run
    Simulator::Schedule(NanoSeconds(m_dt), &PrivateCacheCtrl::Step, Ptr<PrivateCacheCtrl>(this));
    m_cacheCycle++;
  }

  // The init function starts the controller at the beginning
  void PrivateCacheCtrl::init()
  {

    // Initialized Cache Coherence Protocol
    m_cohProtocol->SetProtocolType(m_pType);
    m_cohProtocol->SetLogFileGenEnable(m_logFileGenEnable);
    m_cohProtocol->SetCoreId(m_coreId);
    m_cohProtocol->SetSharedMemId(m_sharedMemId);
    m_cohProtocol->SetPrivCachePtr(m_cache);
    m_cohProtocol->SetCpuFIFOPtr(m_cpuFIFO);
    m_cohProtocol->SetBusFIFOPtr(m_busIfFIFO);
    m_cohProtocol->SetCache2Cache(m_cach2Cache);
    m_cohProtocol->SetReqWbRatio(m_reqWbRatio);
    m_cohProtocol->InitializeCacheStates();
    Simulator::Schedule(NanoSeconds(m_clkSkew), &PrivateCacheCtrl::Step, Ptr<PrivateCacheCtrl>(this));
  }

  /**
     * Runs one mobility Step for the given vehicle generator.
     * This function is called each interval dt
     */
  void PrivateCacheCtrl::Step(Ptr<PrivateCacheCtrl> privateCacheCtrl)
  {

    privateCacheCtrl->CycleProcess();
  }
}
