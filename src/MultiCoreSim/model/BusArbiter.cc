/*
 * File  :      BusArbiter.cc
 * Author:      Reza Mirosanlou
 * Email :      rmirosan@uwaterloo.ca
 *
 * Created On April 02, 2021
 */

#include "BusArbiter.h"
#include <algorithm> // std::max
#include <math.h>    /* ceil */

using namespace std;

namespace ns3
{

  // override ns3 type
  TypeId BusArbiter::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::BusArbiter")
                            .SetParent<Object>();
    return tid;
  }

  BusArbiter::BusArbiter(std::list<ns3::Ptr<ns3::BusIfFIFO>> associatedPrivCacheBusIfFIFO,
                         std::list<ns3::Ptr<ns3::BusIfFIFO>> assoicateLLCBusIfFIFO,
                         ns3::Ptr<ns3::InterConnectFIFO> interConnectFIFO,
                         ns3::Ptr<ns3::GlobalQueue> globalQueue)
  { // Modified to Accomodate Multi Shared Cache std::list<ns3::Ptr<ns3::BusIfFIFO> > assoicateLLCBusIfFIFO,
    // default
    m_dt = (1.0); // cpu clock cycle period
    m_clkSkew = 0;
    m_cpuCore = 4;
    m_arbiCycle = 0;
    m_reqclks = 4;
    m_sharedbankclks = 20;
    m_respclks = 50;
    m_workconserv = false;
    m_duetto = false;
    m_reqCoreCnt = 0;
    m_respCoreCnt = 0;
    m_logFileGenEnable = 0;
    m_cacheBlkSize = 64;
    SwitchHPRT = 0;
    SwitchRTHP = 0;
    kreturn = 0;
    RTCycle = 0;
    HPCycle = 0;
    m_sharedCacheBusIfFIFO = assoicateLLCBusIfFIFO;
    m_busIfFIFO = associatedPrivCacheBusIfFIFO;
    m_interConnectFIFO = interConnectFIFO;
    m_GlobalQueue = globalQueue;
    m_bus_arb = BusARBType::PISCOT_ARB;
    m_cohProType = CohProtType::SNOOP_PMSI;
    m_bus_arch = "split";
    m_bus_arbiter = "PISCOT";
    m_reqbus_arb = "TDM";
    m_respbus_arb = "FCFS";
    m_maxPendingReq = 1;
    m_PndReq = false;
    m_PndResp = false;
    m_cach2Cache = false;
    m_TimeOut = 0;
    m_FcFsPndMemResp = false;
    m_PndPutMChk = false;
    m_DirectTransfer = false;
    m_IdleSlot = false;
    m_PndMemResp = false;
    m_PndWB = false;
    arb_req_mode = "HP";
    arb_resp_mode = "HP";
    for (int i = 0; i < 32; i++)
    {
      m_ReqWbFlag[i] = true;
    }
    m_stallDetectionEnable = true;
    m_stall_cnt = 0;
    m_reza_log = false;
    m_wcArbiter = 0;
    priorityInversion = 0;
  }
  BusArbiter::~BusArbiter()
  {
  }
  void BusArbiter::SetCacheBlkSize(uint32_t cacheBlkSize)
  {
    m_cacheBlkSize = cacheBlkSize;
  }
  void BusArbiter::SetDt(double dt)
  {
    m_dt = dt; // dt is cpuClkPeriod
  }

  int BusArbiter::GetDt()
  {
    return m_dt;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg &msg)
  {
    unsigned int WCL_0;
    unsigned int WCL_1;
    unsigned int WCL_2;
    // chose the bounds based on the types
    WCL_0 = 8888;
    WCL_1 = 8888;
    WCL_2 = 8888;
    if (msg.orderDetermined)
    {
      switch (msg.orderofArbitration)
      {
      case 0:
        msg.associateDeadline = WCL_0 - (m_arbiCycle - msg.becameOldest);
        break;
      case 1:
        msg.associateDeadline = WCL_1 - (m_arbiCycle - msg.becameOldest);
        break;
      case 2:
        msg.associateDeadline = WCL_2 - (m_arbiCycle - msg.becameOldest);
        break;
      }
      msg.associateDeadline_final = true;
    }
  }
  void BusArbiter::SetClkSkew(double clkSkew)
  {
    m_clkSkew = clkSkew;
  }
  void BusArbiter::SetIsWorkConserv(bool workConservFlag)
  {
    m_workconserv = workConservFlag;
  }
  void BusArbiter::SetIsDuetto(bool duetto)
  {
    m_duetto = duetto;
  }
  void BusArbiter::SetNumPrivCore(int nPrivCores)
  {
    m_cpuCore = nPrivCores; // since we have the same amount of cores as private caches
  }
  uint32_t BusArbiter::retrieveSharedBankCacheLine(uint64_t phy_addr)
  {
    uint32_t cacheLine;
    cacheLine = (phy_addr >> (int)log2(m_cacheBlkSize)) & (m_nsets - 1);
    return cacheLine;
  }
  unsigned BusArbiter::createMask(unsigned a, unsigned b)
  {
    unsigned r = 0;
    for (unsigned i = a; i <= b; i++)
      r |= 1 << i;
    return r;
  }
  void BusArbiter::SetCacheNsets(uint32_t nsets)
  {
    m_nsets = nsets;
  }
  void BusArbiter::SetSharedCacheLatency(uint32_t latency)
  {
    m_sharedbankclks = latency;
  }
  void BusArbiter::SetNumReqCycles(int ncycle)
  {
    m_reqclks = ncycle;
  }
  void BusArbiter::SetNumRespCycles(int ncycle)
  {
    m_respclks = ncycle;
  }
  void BusArbiter::SetCache2Cache(bool cach2Cache)
  {
    m_cach2Cache = cach2Cache;
  }
  void BusArbiter::SetBusArchitecture(string bus_arch)
  {
    m_bus_arch = bus_arch;
  }
  void BusArbiter::SetBusArbitration(string bus_arb)
  {
    m_bus_arbiter = bus_arb;
  }
  void BusArbiter::SetReqBusArb(string reqbus_arb)
  {
    m_reqbus_arb = reqbus_arb;
  }
  void BusArbiter::SetRespBusArb(string respbus_arb)
  {
    m_respbus_arb = respbus_arb;
  }
  void BusArbiter::SetCohProtType(CohProtType ptype)
  {
    m_cohProType = ptype;
  }
  void BusArbiter::SetMaxPendingReq(int maxPendingReq)
  {
    m_maxPendingReq = maxPendingReq;
  }
  void BusArbiter::SetLogFileGenEnable(bool logFileGenEnable)
  {
    m_logFileGenEnable = logFileGenEnable;
  }
  void BusArbiter::MSI_FcFsRespBus()
  {
    if (m_PndResp)
    {
      m_GlobalQueue->m_totalResp++;
      if (m_duetto)
      {
        bool terminateii = false;
        for (int ii1 = 0; ii1 < m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii1++)
        {
          BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if (tempS.msgId == m_PendResp.msgId && tempS.addr == m_PendResp.addr)
          {
            tempS.currStage = "RESPONSE";
            terminateii = true;
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
          else
          {
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
        }
      }
      m_PndResp = CheckPendingFCFSResp(m_PendResp, false);
      m_PndResp = false;

      if (m_PendResp.msgId != 0)
      {
        // the mask should be configured according to the architecture (banks#)
        unsigned int mask = createMask(6, 8);
        unsigned bank_num = mask & m_PendResp.addr;
        bank_num = bank_num >> 6;
        m_PendResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

        if (isFinal(m_PendResp.addr, m_PendResp.msgId, m_PendResp.sharedCacheAgent))
        {
          if (isOldest(m_PendResp.addr, m_PendResp.reqCoreId))
          {
            // 1- If YES, adjust the order
            for (unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size(); h++)
            {
              if (m_GlobalQueue->m_GlobalRRQueue.at(h) == m_PendResp.reqCoreId)
              {
                m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
              }
            }

            // 2- If YES, remove this oldest request from oldest queue

            removeFromOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
            removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);
            removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
            // 3- adjust the oldest request
            adjustOldest(m_PendResp.reqCoreId);
          }
          else
          {
            removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
            removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);
          }
        }
        else
        {
          //abort();
        }
      }
    }

    m_PndResp = CheckPendingFCFSResp(m_PendResp, true);
    if (m_PndResp)
    {
      m_GlobalQueue->m_respArbBlock = m_respclks;
      Simulator::Schedule(NanoSeconds(m_dt * m_respclks), &BusArbiter::RespStep, Ptr<BusArbiter>(this));
    }
    else
    { // wait one clock cycle and check again
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::RespStep, Ptr<BusArbiter>(this));
    }

  } //void BusArbiter::MSI_FcFsRespBus()

  void BusArbiter::MSI_FcFsReqBus()
  {

    if (m_PndReq)
    {
      m_PndReq = CheckPendingFCFSReq(m_ReqBusMsg, false);
      InsertOnReqBus(m_ReqBusMsg);
      m_PndReq = false;

      if (m_ReqBusMsg.NoGetMResp == false)
      {
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
      }
    }
    m_PndReq = CheckPendingFCFSReq(m_ReqBusMsg, true);
    // wait one Req-TDM slot
    if (m_PndReq)
    {
      m_GlobalQueue->m_busArbBlock = m_reqclks;
      Simulator::Schedule(NanoSeconds(m_dt * m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }
    else
    { // wait one clock cycle and check again
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }

  } //void BusArbiter::MSI_FcFsReqBus2()

  bool BusArbiter::isHigherPrio(unsigned int l, unsigned int h)
  {
    for (unsigned int jj = 0; jj < m_GlobalQueue->m_GlobalRRQueue.size(); jj++)
    {
      if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
      {
        m_respCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(jj);
        if (l == m_respCoreCnt)
          return true;
        else if (h == m_respCoreCnt)
          return false;
      }
    }
    return false;
  }

  void BusArbiter::deadlineProgress()
  {
    bool deadlineProgress_terminate = false;
    BusIfFIFO::BusReqMsg deadlineProgress_temp;
    BusIfFIFO::BusReqMsg deadlineProgress_temp_type;
    if (!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty())
    {
      for (int it11 = 0; it11 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it11++)
      {
        deadlineProgress_terminate = false;
        deadlineProgress_temp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        for (int it12 = 0; it12 < m_GlobalQueue->m_MsgType.GetQueueSize() && deadlineProgress_terminate == false; it12++)
        {
          deadlineProgress_temp_type = m_GlobalQueue->m_MsgType.GetFrontElement();
          if (deadlineProgress_temp.msgId == deadlineProgress_temp_type.msgId && deadlineProgress_temp.addr == deadlineProgress_temp_type.addr)
          {
            deadlineProgress_terminate = true;
            m_GlobalQueue->m_MsgType.PopElement();
            if (deadlineProgress_temp_type.associateDeadline > 0)
              deadlineProgress_temp_type.associateDeadline--;

            m_GlobalQueue->m_MsgType.InsertElement(deadlineProgress_temp_type);
          }
          else
          {
            m_GlobalQueue->m_MsgType.PopElement();
            m_GlobalQueue->m_MsgType.InsertElement(deadlineProgress_temp_type);
          }
        }
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(deadlineProgress_temp);
      }
    }
  }

  bool BusArbiter::WCLator()
  {

    /** check if HP can potentially issue a request that causes a state where is not desireble. In this case, we limit the number of requests (k) 
     * that target the same bank and same cache line. **/

    unsigned int k = 1;
    //cout<<"----------------------------------------------"<<endl;
    for (std::list<Ptr<BusIfFIFO>>::iterator itk = m_busIfFIFO.begin(); itk != m_busIfFIFO.end(); itk++)
    {
      unsigned iSameLine = 0;
      if ((*itk)->m_txMsgFIFO.IsEmpty() == false)
      {
        ns3::BusIfFIFO::BusReqMsg txMsg_k;
        txMsg_k = (*itk)->m_txMsgFIFO.GetFrontElement();

        for (int itkM = 0; itkM < m_GlobalQueue->m_MsgType.GetQueueSize(); itkM++)
        {
          BusIfFIFO::BusReqMsg tempk;
          tempk = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if (((retrieveSharedBankCacheLine(txMsg_k.addr) == retrieveSharedBankCacheLine(tempk.addr)) && (txMsg_k.sharedCacheAgent == tempk.sharedCacheAgent) && (tempk.msgId != 0) && (!isOldest(tempk.addr, tempk.reqCoreId))) ||
              ((retrieveSharedBankCacheLine(txMsg_k.addr) == retrieveSharedBankCacheLine(tempk.addr)) && (txMsg_k.sharedCacheAgent == tempk.sharedCacheAgent) && (tempk.msgId == 0) && (!isOldest(tempk.addr, tempk.wbCoreId))))
          {
            m_GlobalQueue->m_MsgType.InsertElement(tempk);

            iSameLine++;
            if ((txMsg_k.msgId == tempk.msgId) && (txMsg_k.addr == tempk.addr) && (txMsg_k.reqCoreId == tempk.reqCoreId))
            {
              iSameLine--;
            }

            if (iSameLine == k)
            {
              kreturn++;
              return false;
            }
          }
          else
          {
            m_GlobalQueue->m_MsgType.InsertElement(tempk);
          }
        }
      }
    }

    unsigned int latency, addedLatency, WCLatorReqID, currentOrder;
    unsigned int B_1, B_2, B_3;
    unsigned int k_a, k_b, k_c, k_d, k_e;

    B_1 = m_reqclks - 1;
    B_2 = m_sharedbankclks - 1;
    B_3 = m_respclks - 1;

    ns3::GenericFIFO<ns3::BusIfFIFO::BusReqMsg> tempFromOldest;
    tempFromOldest = m_GlobalQueue->m_GlobalOldestQueue;
    if (!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty())
    {
      for (int it5 = 0; it5 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it5++)
      {
        currentOrder = 100;
        WCLatorReqID = 100;
        k_a = 0;
        k_b = 0;
        k_c = 0;
        k_d = 0;
        k_e = 0;

        // Retrieve the reqID
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator;
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType;
        tempOldestMsgQueueWCLator_FromMemType.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType.associateDeadline_final = false;
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType_1;
        tempOldestMsgQueueWCLator_FromMemType_1.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType_1.associateDeadline_final = false;
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType_local;
        tempOldestMsgQueueWCLator_FromMemType_local.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType_local.associateDeadline_final = false;

        tempOldestMsgQueueWCLator = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();

        WCLatorReqID = tempOldestMsgQueueWCLator.msgId == 0 ? tempOldestMsgQueueWCLator.wbCoreId : tempOldestMsgQueueWCLator.reqCoreId;

        bool RR_Con = false;
        // Retrieve the RR Order
        for (unsigned int it6 = 0; it6 < m_GlobalQueue->m_GlobalRRQueue.size() && RR_Con == false; it6++)
        {
          if (WCLatorReqID == m_GlobalQueue->m_GlobalRRQueue.at(it6))
          {
            currentOrder = it6;
            RR_Con = true;
          }
        }

        // Retrieve the Deadline
        bool terminatei = false;
        for (int ii1 = 0; ii1 < m_GlobalQueue->m_MsgType.GetQueueSize() && terminatei == false; ii1++)
        {

          if (m_GlobalQueue->m_MsgType.GetFrontElement().msgId == tempOldestMsgQueueWCLator.msgId && m_GlobalQueue->m_MsgType.GetFrontElement().addr == tempOldestMsgQueueWCLator.addr && m_GlobalQueue->m_MsgType.GetFrontElement().reqCoreId == tempOldestMsgQueueWCLator.reqCoreId)
          {
            tempOldestMsgQueueWCLator_FromMemType = m_GlobalQueue->m_MsgType.GetFrontElement();
            m_GlobalQueue->m_MsgType.PopElement();
            terminatei = true;
            m_GlobalQueue->m_MsgType.InsertElement(tempOldestMsgQueueWCLator_FromMemType);
          }
          else
          {
            tempOldestMsgQueueWCLator_FromMemType_1 = m_GlobalQueue->m_MsgType.GetFrontElement();
            m_GlobalQueue->m_MsgType.PopElement();
            m_GlobalQueue->m_MsgType.InsertElement(tempOldestMsgQueueWCLator_FromMemType_1);
          }
        }

        // Note that the following is based on the pipeline version of WCLAtor estimation. Based on the discussion, it does not affect
        // the final decision of WCLator if we use the pipelining version or the usual one used in the paper - the pipeline version had
        // a modeling issue which made the proof undoable (a scenario that doesn'r happen but we had to include in the proof)
        // For the non pipeline version, the estimation is even easier
        // Extract the k_a, k_b, k_c, k_d, and k_e
        unsigned tempOrderFront;
        unsigned int GlobalQueueSize = m_GlobalQueue->m_GlobalRRQueue.size();
        for (unsigned int it8 = 0; it8 < GlobalQueueSize; it8++)
        {
          if (currentOrder > it8)
          {

            tempOrderFront = m_GlobalQueue->m_GlobalRRQueue.at(it8);

            bool nextInMemType = false;
            bool notDetermined = false;

            for (int it9 = 0; it9 < tempFromOldest.GetQueueSize() && nextInMemType == false; it9++)
            {
              tempOldestMsgQueueWCLator_FromMemType_local = tempFromOldest.GetFrontElement();
              tempFromOldest.PopElement();
              if (((tempOldestMsgQueueWCLator_FromMemType_local.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType_local.wbCoreId == tempOrderFront) || (tempOldestMsgQueueWCLator_FromMemType_local.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType_local.reqCoreId == tempOrderFront)))
              {
                tempFromOldest.InsertElement(tempOldestMsgQueueWCLator_FromMemType_local);
                for (int ii2 = 0; ii2 < m_GlobalQueue->m_MsgType.GetQueueSize() && nextInMemType == false; ii2++)
                {
                  BusIfFIFO::BusReqMsg tempMemType;
                  tempMemType = m_GlobalQueue->m_MsgType.GetFrontElement();
                  m_GlobalQueue->m_MsgType.PopElement();
                  if (tempOrderFront == WCLatorReqID)
                    notDetermined = true;
                  if (tempMemType.msgId == tempOldestMsgQueueWCLator_FromMemType_local.msgId && tempMemType.addr == tempOldestMsgQueueWCLator_FromMemType_local.addr && tempOrderFront != WCLatorReqID)
                  {

                    notDetermined = true;
                    if (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == tempMemType.orderofArbitration && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent)
                    {

                      k_a++;
                      nextInMemType = true;
                    }
                    else if (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == tempMemType.orderofArbitration && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent)
                    {

                      k_b++;
                      nextInMemType = true;
                    }
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 1 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent) || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 1 && tempMemType.orderofArbitration == 0 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent))
                    {

                      k_c++;
                      nextInMemType = true;
                    }
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 1 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent) || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 1 && tempMemType.orderofArbitration == 0 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent))
                    {

                      k_d++;
                      nextInMemType = true;
                    }
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 2) || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 2 && tempMemType.orderofArbitration == 0))
                    {

                      k_e++;
                      nextInMemType = true;
                    }
                    m_GlobalQueue->m_MsgType.InsertElement(tempMemType);
                  }
                  else
                  {
                    m_GlobalQueue->m_MsgType.InsertElement(tempMemType);
                  }
                  /** here it means that we went over the queue of MemType but we could not find the oldest request meaning that it is not determined yet. 
                   * hence, we need to consider the worst possible case until it can be determined **/
                  if ((ii2 == (m_GlobalQueue->m_MsgType.GetQueueSize() - 1)) && notDetermined == false)
                  {

                    k_c++;
                  }
                }
              }
              else
              {
                tempFromOldest.InsertElement(tempOldestMsgQueueWCLator_FromMemType_local);
              }
            }
          }
        }

        // check if HP can do something that adds to the latency - This can be any oldest that is lower priority or any non oldest from anyone
        addedLatency = 0;

        if (tempOldestMsgQueueWCLator_FromMemType.orderDetermined)
        {

          switch (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration)
          {
          case 0:
            /* REQ:BANK:RESPONSE */
            if (tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ")
            {
              // check if there is anything that HPA can process on same BANK from RxResp from lower priority
              std::list<Ptr<BusIfFIFO>>::iterator itInvert = m_sharedCacheBusIfFIFO.begin();

              std::advance(itInvert, retrieveSharedCacheID(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent));

              // check if there is anything that HPA can process on same BANK from RxResp from lower priority

              if ((((*itInvert)->m_rxRespFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 2)) ||
                  (((*itInvert)->m_rxRespFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 0)))
              {

                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg RxResp_HP;
                RxResp_HP = (*itInvert)->m_rxRespFIFO.GetFrontElement();
                if (RxResp_HP.msgId != 0 && isOldest(RxResp_HP.addr, RxResp_HP.reqCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxResp_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxResp_HP.reqCoreId))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (RxResp_HP.msgId == 0 && isOldest(RxResp_HP.addr, RxResp_HP.respCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxResp_HP.respCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxResp_HP.respCoreId)))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {
                  addedLatency = m_sharedbankclks;
                }
              }

              // check if there is anything that HPA can process on same BANK from m_localPendingRespTxBuffer from lower priority

              if ((((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 2)) ||
                  (((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 0)))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg Txlocal_HP;
                Txlocal_HP = (*itInvert)->m_localPendingRespTxBuffer.GetFrontElement();
                if (Txlocal_HP.msgId != 0 && isOldest(Txlocal_HP.addr, Txlocal_HP.reqCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == Txlocal_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == Txlocal_HP.reqCoreId))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (Txlocal_HP.msgId == 0 && isOldest(Txlocal_HP.addr, Txlocal_HP.respCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == Txlocal_HP.respCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == Txlocal_HP.respCoreId)))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {
                  addedLatency = m_sharedbankclks;
                }
              }
              // if there is any MSG from lower priority oldest Or from any priority non oldest, we need to add to the latency
              if ((((*itInvert)->m_rxMsgFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 2)) ||
                  (((*itInvert)->m_rxMsgFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 0)))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusReqMsg RxMsg_HP;
                RxMsg_HP = (*itInvert)->m_rxMsgFIFO.GetFrontElement();
                if (RxMsg_HP.msgId != 0 && isOldest(RxMsg_HP.addr, RxMsg_HP.reqCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxMsg_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxMsg_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxMsg_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxMsg_HP.reqCoreId))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (RxMsg_HP.msgId == 0 && isOldest(RxMsg_HP.addr, RxMsg_HP.wbCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxMsg_HP.wbCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxMsg_HP.wbCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxMsg_HP.wbCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxMsg_HP.wbCoreId)))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {

                  addedLatency = m_sharedbankclks;
                }
              }
            }
            else if ((tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK") && (m_GlobalQueue->m_respArbBlock == 0))
            {

              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
              // check if there is anything that HPA can send on RESPONSE bus from shared banks
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_sharedCacheBusIfFIFO.begin(); it13 != m_sharedCacheBusIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {
            }
            break;
          case 1:
            /* REQ:RESPONSE:BANK */
            if ((tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") && (m_GlobalQueue->m_respArbBlock == 0))
            {

              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
              // check if there is anything that HPA can send on RESPONSE bus from shared banks
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_sharedCacheBusIfFIFO.begin(); it13 != m_sharedCacheBusIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {

              std::list<Ptr<BusIfFIFO>>::iterator itInvert = m_sharedCacheBusIfFIFO.begin();
              std::advance(itInvert, retrieveSharedCacheID(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent));
              // check if there is anything that HPA can process on same BANK from RxResp from lower priority

              if ((*itInvert)->m_rxRespFIFO.IsEmpty() == false && ((*itInvert)->m_bankArbBlock == 2))
              {

                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg RxResp_HP;
                RxResp_HP = (*itInvert)->m_rxRespFIFO.GetFrontElement();
                if (RxResp_HP.msgId != 0 && isOldest(RxResp_HP.addr, RxResp_HP.reqCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxResp_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxResp_HP.reqCoreId))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (RxResp_HP.msgId == 0 && isOldest(RxResp_HP.addr, RxResp_HP.respCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxResp_HP.respCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxResp_HP.respCoreId)))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {
                  addedLatency = m_sharedbankclks;
                }
              }

              // check if there is anything that HPA can process on same BANK from m_localPendingRespTxBuffer from lower priority

              if ((((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 2)) ||
                  (((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 0)))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg Txlocal_HP;
                Txlocal_HP = (*itInvert)->m_localPendingRespTxBuffer.GetFrontElement();
                if (Txlocal_HP.msgId != 0 && isOldest(Txlocal_HP.addr, Txlocal_HP.reqCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == Txlocal_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == Txlocal_HP.reqCoreId))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (Txlocal_HP.msgId == 0 && isOldest(Txlocal_HP.addr, Txlocal_HP.respCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == Txlocal_HP.respCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == Txlocal_HP.respCoreId)))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {
                  addedLatency = m_sharedbankclks;
                }
              }

              // if there is any MSG from lower priority oldest Or from any priority non oldest, we need to add to the latency
              if ((((*itInvert)->m_rxMsgFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 2)) ||
                  (((*itInvert)->m_rxMsgFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 0)))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusReqMsg RxMsg_HP;
                RxMsg_HP = (*itInvert)->m_rxMsgFIFO.GetFrontElement();
                if (RxMsg_HP.msgId != 0 && isOldest(RxMsg_HP.addr, RxMsg_HP.reqCoreId))
                {
                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxMsg_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxMsg_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxMsg_HP.reqCoreId) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxMsg_HP.reqCoreId))
                  {
                    addedLatency = m_sharedbankclks;
                  }
                }
                else if (RxMsg_HP.msgId == 0 && isOldest(RxMsg_HP.addr, RxMsg_HP.wbCoreId))
                {

                  if ((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxMsg_HP.wbCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxMsg_HP.wbCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == RxMsg_HP.wbCoreId)) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == RxMsg_HP.wbCoreId)))
                  {

                    addedLatency = m_sharedbankclks;
                  }
                }
                else
                {
                  addedLatency = m_sharedbankclks;
                }
              }
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK")
            {
            }

            break;
          case 2:
            /* REQ:RESP */
            if ((tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") && (m_GlobalQueue->m_respArbBlock == 0))
            {

              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
              // check if there is anything that HPA can send on RESPONSE bus from shared banks
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_sharedCacheBusIfFIFO.begin(); it13 != m_sharedCacheBusIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr, txResp_HP.reqCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.reqCoreId) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.reqCoreId))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else if (txResp_HP.msgId == 0 && isOldest(txResp_HP.addr, txResp_HP.respCoreId))
                  {
                    if ((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && (tempOldestMsgQueueWCLator_FromMemType.reqCoreId == txResp_HP.respCoreId)) ||
                        (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && (tempOldestMsgQueueWCLator_FromMemType.wbCoreId == txResp_HP.respCoreId)))
                    {
                      addedLatency = m_respclks;
                    }
                  }
                  else
                  {
                    addedLatency = m_respclks;
                  }
                }
              }
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {
            }

            break;
          default:
            break;
          }
        }
        else
        {
          if (m_GlobalQueue->m_busArbBlock == 0)
          {
            for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
            {
              if ((*it13)->m_txMsgFIFO.IsEmpty() == false)
              {
                ns3::BusIfFIFO::BusReqMsg txMsg_HP;
                txMsg_HP = (*it13)->m_txMsgFIFO.GetFrontElement();
                if (tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != txMsg_HP.sharedCacheAgent)
                {
                  addedLatency = m_reqclks;
                }
                else
                {
                  // check if this is from the same cache line
                  if (retrieveSharedBankCacheLine(tempOldestMsgQueueWCLator_FromMemType.addr) == retrieveSharedBankCacheLine(txMsg_HP.addr))
                  {
                    k_a++;
                    if (!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty())
                    {
                      BusIfFIFO::BusReqMsg PrioInversion;
                      int size_glob = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
                      for (int it14 = 0; it14 < size_glob; it14++)
                      {
                        PrioInversion = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
                        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
                        if ((PrioInversion.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && !isHigherPrio(PrioInversion.wbCoreId, tempOldestMsgQueueWCLator_FromMemType.wbCoreId)) ||
                            (PrioInversion.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && !isHigherPrio(PrioInversion.reqCoreId, tempOldestMsgQueueWCLator_FromMemType.reqCoreId)) ||
                            (PrioInversion.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && !isHigherPrio(PrioInversion.wbCoreId, tempOldestMsgQueueWCLator_FromMemType.reqCoreId)) ||
                            (PrioInversion.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && !isHigherPrio(PrioInversion.reqCoreId, tempOldestMsgQueueWCLator_FromMemType.wbCoreId)))
                        {
                          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(PrioInversion);

                          k_a++;
                        }
                        else
                        {
                          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(PrioInversion);
                        }
                      }
                    }
                  }
                  else
                  {
                    addedLatency = m_reqclks;
                  }
                }
              }
            }
          }
        }

        latency = 0;
        // Estimate the latency
        if ((tempOldestMsgQueueWCLator_FromMemType.orderDetermined == true) && (tempOldestMsgQueueWCLator_FromMemType.associateDeadline_final == true))
        {

          std::list<Ptr<BusIfFIFO>>::iterator itt = m_sharedCacheBusIfFIFO.begin();
          std::advance(itt, retrieveSharedCacheID(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent));

          switch (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration)
          {
          case 0:
            /* REQ:BANK:RESPONSE */
            if (tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ")
            {
              // The request finished proceed REQ stage

              latency = addedLatency + (*itt)->m_bankArbBlock + B_3 + m_sharedbankclks + m_respclks + (k_a)*max(m_sharedbankclks, m_respclks) + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK")
            {

              // The request finished its second stage
              latency = addedLatency + m_GlobalQueue->m_respArbBlock + m_respclks + (k_a)*m_respclks + (k_b + k_d + k_e) * (m_respclks) + k_c * m_respclks;
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {

              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else
            {

              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + ((k_a)*max(m_reqclks, max(m_sharedbankclks, m_respclks))) + ((k_b + k_d + k_e) * (m_reqclks + m_respclks)) + (k_c * max(m_sharedbankclks, m_respclks));
            }
            break;
          case 1:
            /* REQ:RESPONSE:BANK */
            if (tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ")
            {

              // The request finished proceed REQ stage
              latency = addedLatency + m_GlobalQueue->m_respArbBlock + B_2 + m_sharedbankclks + m_respclks + (k_a)*max(m_sharedbankclks, m_respclks) + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {

              // The request finished its second last stage
              latency = addedLatency + (*itt)->m_bankArbBlock + m_sharedbankclks + (k_a)*m_sharedbankclks + k_c * m_sharedbankclks;
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK")
            {

              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else
            {

              // The request did not proceed into any stages
              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a)*max(m_reqclks, max(m_sharedbankclks, m_respclks)) + (k_b + k_d + k_e) * (m_reqclks + m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
            }
            break;
          case 2:
            /* REQ:RESP */
            if (tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ")
            {

              // The request finished proceed REQ stage
              latency = addedLatency + m_GlobalQueue->m_respArbBlock + m_sharedbankclks + m_respclks + (k_a)*max(m_sharedbankclks, m_respclks) + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
            }
            else if (tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE")
            {

              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else
            {

              // The request did not proceed into any stages
              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a)*max(m_reqclks, max(m_sharedbankclks, m_respclks)) + (k_b + k_d + k_e) * (m_reqclks + m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
            }
            break;
          default:
            break;
          }
        }
        else
        {

          // The request did not proceed into any stages
          latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a)*max(m_reqclks, max(m_sharedbankclks, m_respclks)) + (k_b + k_d + k_e) * (m_reqclks + m_respclks) + k_c * max(m_sharedbankclks, m_respclks);
        }

        // Determination
        if (latency >= tempOldestMsgQueueWCLator_FromMemType.associateDeadline)
        {
          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestMsgQueueWCLator);
          return false;
        }

        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestMsgQueueWCLator);
      }

      return true;
    }
    return true;
  }

  void BusArbiter::SendMemCohrMsg(BusIfFIFO::BusReqMsg msg, bool BroadCast = false)
  {
    // send coherence messages on the bus - it seems it only targets the private caches
    for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++)
    {
      if (msg.reqCoreId == (*it2)->m_fifo_id || BroadCast == true)
      {
        if (!(*it2)->m_rxMsgFIFO.IsFull())
        {
          (*it2)->m_rxMsgFIFO.InsertElement(msg);
        }
        else
        {
          if (m_logFileGenEnable)
          {
            std::cout << "BusArbiter cannot insert new messages into the buffers" << std::endl;
          }
          exit(0);
        }
      }
    }
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SendData(BusIfFIFO::BusRespMsg msg, AGENT agent)
  {

    if (agent == AGENT::CORE)
    {
      for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++)
      {
        if (msg.reqCoreId == (*it2)->m_fifo_id)
        {
          if (!(*it2)->m_rxRespFIFO.IsFull())
          {
            if (msg.respCoreId < 10)
            {
              m_GlobalQueue->m_Core2CoreTransfer++;
              if (msg.respCoreId == 0)
                m_GlobalQueue->m_Core2CoreTransfer_0++;
              else if (msg.respCoreId == 1)
                m_GlobalQueue->m_Core2CoreTransfer_1++;
              else if (msg.respCoreId == 2)
                m_GlobalQueue->m_Core2CoreTransfer_2++;
              else if (msg.respCoreId == 3)
                m_GlobalQueue->m_Core2CoreTransfer_3++;
              else if (msg.respCoreId == 4)
                m_GlobalQueue->m_Core2CoreTransfer_4++;
              else if (msg.respCoreId == 5)
                m_GlobalQueue->m_Core2CoreTransfer_5++;
              else if (msg.respCoreId == 6)
                m_GlobalQueue->m_Core2CoreTransfer_6++;
              else if (msg.respCoreId == 7)
                m_GlobalQueue->m_Core2CoreTransfer_7++;
            }
            else
            {
              m_GlobalQueue->m_SharedBankTransfer++;
              if (msg.reqCoreId == 0)
                m_GlobalQueue->m_SharedBankTransfer_0++;
              else if (msg.reqCoreId == 1)
                m_GlobalQueue->m_SharedBankTransfer_1++;
              else if (msg.reqCoreId == 2)
                m_GlobalQueue->m_SharedBankTransfer_2++;
              else if (msg.reqCoreId == 3)
                m_GlobalQueue->m_SharedBankTransfer_3++;
              else if (msg.reqCoreId == 4)
                m_GlobalQueue->m_SharedBankTransfer_4++;
              else if (msg.reqCoreId == 5)
                m_GlobalQueue->m_SharedBankTransfer_5++;
              else if (msg.reqCoreId == 6)
                m_GlobalQueue->m_SharedBankTransfer_6++;
              else if (msg.reqCoreId == 7)
                m_GlobalQueue->m_SharedBankTransfer_7++;
            }

            if (m_logFileGenEnable)
            {
              std::cout << "\nBusArbiter: Cpu/Mem Id " << msg.respCoreId << " Sent Data to Core " << msg.reqCoreId << "  the address is  " << msg.addr << " ============================================== " << m_arbiCycle << "\n\n";
            }
            (*it2)->m_rxRespFIFO.InsertElement(msg);
          }
          else
          {
            if (m_logFileGenEnable)
            {
              std::cout << "BusArbiter cannot insert new messages into the buffers" << std::endl;
            }
            exit(0);
          }
        }
      }
    }

    if (agent == AGENT::SHAREDMEM)
    {
      m_GlobalQueue->m_totalReplacement++;
      unsigned int mask = createMask(6, 8);
      unsigned bank_num = mask & msg.addr;
      bank_num = bank_num >> 6;
      msg.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

      std::list<Ptr<BusIfFIFO>>::iterator it3 = m_sharedCacheBusIfFIFO.begin();
      std::advance(it3, retrieveSharedCacheID(msg.sharedCacheAgent));

      if (!(*it3)->m_rxRespFIFO.IsFull())
      {
        (*it3)->m_rxRespFIFO.InsertElement(msg);
      }
      else
      {
        if (m_logFileGenEnable)
        {
          std::cout << "BusArbiter cannot insert new messages into LLC RxResp buffers" << std::endl;
        }
        exit(0);
      }
    }

    if (agent == AGENT::INTERCONNECT)
    {
      if (!m_interConnectFIFO->m_RespMsgFIFO.IsEmpty())
      {
        m_interConnectFIFO->m_RespMsgFIFO.PopElement();
      }
      msg.cycle = m_arbiCycle;
      m_interConnectFIFO->m_RespMsgFIFO.InsertElement(msg);
    }
  }
  bool BusArbiter::CheckPendingFCFSReq(BusIfFIFO::BusReqMsg &txMsg, bool ChkOnly = true)
  { /* checks if there is a  pending request in FCFS among all requestors not for specific core. */

    double arrivalTime = 0;
    uint16_t core_idx = 0;
    uint16_t TargetCore = 0;
    bool PendingTxReq = false;

    for (std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end(); it1++)
    {
      if ((*it1)->m_txMsgFIFO.IsEmpty() == false)
      {
        txMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
        if ((PendingTxReq == false) || arrivalTime > txMsg.timestamp)
        {
          arrivalTime = txMsg.timestamp;
          TargetCore = core_idx;
        }
        PendingTxReq = true;
      }
      core_idx++;
    }
    if (PendingTxReq)
    {

      std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin();
      std::advance(it2, TargetCore);
      txMsg = (*it2)->m_txMsgFIFO.GetFrontElement();

      // check if exists in the global oldest queue, then assign the became oldest timing
      for (int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it2++)
      {
        BusIfFIFO::BusReqMsg txMsgTemp;
        txMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        if ((txMsgTemp.msgId == txMsg.msgId) && (txMsgTemp.addr == txMsg.addr))
        {
          txMsg.becameOldest = txMsgTemp.becameOldest;
        }
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(txMsgTemp);
      }

      // Remove message from the busResp buffer

      if (ChkOnly == false)
      {

        (*it2)->m_txMsgFIFO.PopElement();
      }
    }
    return PendingTxReq;
  }

  unsigned int BusArbiter::retrieveSharedCacheID(uint64_t id)
  {
    if (id == 10)
      return 0;
    else if (id == 11)
      return 1;
    else if (id == 12)
      return 2;
    else if (id == 13)
      return 3;
    else if (id == 14)
      return 4;
    else if (id == 15)
      return 5;
    else if (id == 16)
      return 6;
    else if (id == 17)
      return 7;
    else if (id == 18)
      return 8;
    else if (id == 19)
      return 9;
    else if (id == 20)
      return 10;
    else if (id == 21)
      return 11;
    else if (id == 22)
      return 12;
    else if (id == 23)
      return 13;
    else if (id == 24)
      return 14;
    else if (id == 25)
      return 15;
    cout << "retrieveSharedCacheID cannot find " << id << " as a Shared Cache ID" << endl;
    abort();
    return 0;
  }

  unsigned int BusArbiter::retrieveCacheFIFOID(uint64_t id)
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

  bool BusArbiter::existServiceQueue(BusIfFIFO::BusReqMsg &tempMsgQueue)
  {

    for (int it4 = 0; it4 < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); it4++)
    {
      BusIfFIFO::BusReqMsg tempMsgQueue_service;
      tempMsgQueue_service = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();

      m_GlobalQueue->m_GlobalReqFIFO.PopElement();
      if (tempMsgQueue_service.msgId == tempMsgQueue.msgId)
      {
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempMsgQueue_service);

        return false;
      }
      else
      {
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempMsgQueue_service);
      }
    }
    return true;
  }

  bool BusArbiter::CheckPendingResp(BusIfFIFO::BusRespMsg &txResp, bool ChkOnly, bool oldest, int &mode)
  {
    std::list<Ptr<BusIfFIFO>>::iterator itx = m_sharedCacheBusIfFIFO.begin();
    bool selected = false;
    bool selectedPrioInvert = false;
    int selectedPrioInvertIndex = 0;
    mode = 0;
    bool PendingTxResp = false;
    bool PendingTxWBResp = false;

    unsigned int sharedCacheID;
    unsigned int GlobalQueueSize;
    BusIfFIFO::BusRespMsg pendingWbMsg_1;
    BusIfFIFO::BusRespMsg pendingWbMsg_2;
    BusIfFIFO::BusRespMsg pendingWbMsg_3;
    BusIfFIFO::BusReqMsg itgReq;
    BusIfFIFO::BusRespMsg issueTemp;

    if ((ChkOnly && (m_delaycycle == 2)) || ((ChkOnly) && (m_arbiCycle == m_delaycyclereg)))
    {

      m_delaycycle = 0;
      return false;
    }
    // the arbitration is non preemptive. If check only is not true, the arbiter must isssue the scheduled one even if the mode is changed (RT->HP/HP->RT)
    if (!ChkOnly)
    {
      if (m_delaycycle == 1)
      {
        m_delaycyclereg = m_arbiCycle;
        m_delaycycle = 2;
      }
      issueTemp = txResp;
      if (issueTemp.msgId == 0)
      {
        std::list<Ptr<BusIfFIFO>>::iterator itc = m_busIfFIFO.begin();
        std::advance(itc, issueTemp.respCoreId);
        int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
        for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
        {
          pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
          (*itc)->m_txRespFIFO.PopElement();
          if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
          {
            PendingTxWBResp = true;
          }
          else
          {
            (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
          }
        }
      }
      else
      {
        if (issueTemp.respCoreId < 10)
        {
          std::list<Ptr<BusIfFIFO>>::iterator itc = m_busIfFIFO.begin();
          std::advance(itc, issueTemp.respCoreId);
          int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
          for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
          {
            pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
            (*itc)->m_txRespFIFO.PopElement();
            if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
            {
              PendingTxWBResp = true;
            }
            else
            {
              (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
            }
          }
        }
        else if (issueTemp.respCoreId >= 10)
        {
          // if it is not a wirte back resp, we should check for the shared banks
          std::list<Ptr<BusIfFIFO>>::iterator itc = m_sharedCacheBusIfFIFO.begin();
          sharedCacheID = retrieveSharedCacheID(issueTemp.sharedCacheAgent);
          std::advance(itc, sharedCacheID);
          int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
          for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
          {
            pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
            (*itc)->m_txRespFIFO.PopElement();
            if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
            {
              PendingTxResp = true;
            }
            else
            {
              (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
            }
          }
        }
      }
    }

    else
    {

      // arbitrate the responses from shared banks
      for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && PendingTxResp == false; RR_iterator++)
      {
        if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
        {
          m_respCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
        }

        // check the oldest global queue
        if (oldest)
          GlobalQueueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
        else
          GlobalQueueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();

        for (unsigned int itr = 0; itr < GlobalQueueSize && PendingTxResp == false; itr++)
        {

          if (oldest)
          {
            m_ServQueueMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
            m_GlobalQueue->m_GlobalOldestQueue.PopElement();
          }
          else
          {
            m_ServQueueMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
            m_GlobalQueue->m_GlobalReqFIFO.PopElement();
          }
          if (m_ServQueueMsg.reqCoreId == m_respCoreCnt)
          {

            std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin();
            sharedCacheID = retrieveSharedCacheID(m_ServQueueMsg.sharedCacheAgent);
            std::advance(it1, sharedCacheID);
            int pendingQueueSize = (*it1)->m_txRespFIFO.GetQueueSize();
            for (int i = 0; i < pendingQueueSize && PendingTxResp == false; i++)
            {

              pendingWbMsg_1 = (*it1)->m_txRespFIFO.GetFrontElement();
              (*it1)->m_txRespFIFO.PopElement();
              if ((pendingWbMsg_1.reqCoreId == m_respCoreCnt) && (pendingWbMsg_1.addr == m_ServQueueMsg.addr))
              {
                if (ChkOnly == true)
                {
                  m_delaycycle = 1;
                  (*it1)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                }
                itx = it1;
                selected = true;
                mode = 1;
                PendingTxResp = true;
                txResp = pendingWbMsg_1;
              }
              else
              {
                (*it1)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
              }
            }

            if (((oldest) && (PendingTxResp == false) && (ChkOnly == true)) || ((!oldest) && (PendingTxResp == false) && (ChkOnly == false)))
            {
              bool PendingTxRespg = false;
              for (int itg = 0; itg < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() && PendingTxRespg == false; itg++)
              {
                itgReq = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
                m_GlobalQueue->m_GlobalReqFIFO.PopElement();
                // if(((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                //    (!isOldest(itgReq.addr,itgReq.reqCoreId)) && (itgReq.msgId != 0) && (itgReq.timestamp < m_ServQueueMsg.timestamp)) ||
                //    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                //    (!isOldest(itgReq.addr,itgReq.wbCoreId)) && (itgReq.msgId == 0) && (itgReq.timestamp < m_ServQueueMsg.timestamp)) ||
                //    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                //    (isOldest(itgReq.addr,itgReq.reqCoreId)) && (itgReq.msgId != 0) && (!isHigherPrio(itgReq.reqCoreId,m_ServQueueMsg.reqCoreId)) && (itgReq.timestamp < m_ServQueueMsg.timestamp)) ||
                //    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                //    (isOldest(itgReq.addr,itgReq.wbCoreId)) && (itgReq.msgId == 0)  && (!isHigherPrio(itgReq.wbCoreId,m_ServQueueMsg.reqCoreId)) && (itgReq.timestamp < m_ServQueueMsg.timestamp))) {
                if (((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                     (!isOldest(itgReq.addr, itgReq.reqCoreId)) && (itgReq.msgId != 0)) ||
                    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                     (!isOldest(itgReq.addr, itgReq.wbCoreId)) && (itgReq.msgId == 0)) ||
                    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                     (isOldest(itgReq.addr, itgReq.reqCoreId)) && (itgReq.msgId != 0) && (!isHigherPrio(itgReq.reqCoreId, m_ServQueueMsg.reqCoreId))) ||
                    ((retrieveSharedBankCacheLine(m_ServQueueMsg.addr) == retrieveSharedBankCacheLine(itgReq.addr)) && (m_ServQueueMsg.sharedCacheAgent == itgReq.sharedCacheAgent) &&
                     (isOldest(itgReq.addr, itgReq.wbCoreId)) && (itgReq.msgId == 0) && (!isHigherPrio(itgReq.wbCoreId, m_ServQueueMsg.reqCoreId))))
                {

                  // check the TX response buffer of the bank
                  std::list<Ptr<BusIfFIFO>>::iterator itg1 = m_sharedCacheBusIfFIFO.begin();
                  sharedCacheID = retrieveSharedCacheID(m_ServQueueMsg.sharedCacheAgent);
                  std::advance(itg1, sharedCacheID);
                  int pendingQueueSize = (*itg1)->m_txRespFIFO.GetQueueSize();
                  for (int i = 0; i < pendingQueueSize && PendingTxRespg == false; i++)
                  {
                    pendingWbMsg_1 = (*itg1)->m_txRespFIFO.GetFrontElement();
                    (*itg1)->m_txRespFIFO.PopElement();
                    if ((pendingWbMsg_1.msgId == itgReq.msgId) && (pendingWbMsg_1.addr == itgReq.addr))
                    {
                      PendingTxRespg = true;
                      PendingTxResp = true;
                      if (ChkOnly == true)
                      {
                        m_delaycycle = 1;

                        (*itg1)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                      }

                      itx = itg1;
                      mode = 1;
                      selected = true;
                      selectedPrioInvert = true;
                      selectedPrioInvertIndex = m_respCoreCnt;
                      txResp = pendingWbMsg_1;
                    }
                    else
                    {
                      (*itg1)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                    }
                  }
                  // check the other requestors TX response buffer as well
                  for (std::list<Ptr<BusIfFIFO>>::iterator itg2 = m_busIfFIFO.begin(); itg2 != m_busIfFIFO.end() && PendingTxRespg == false; itg2++)
                  {
                    int pendingQueueSize = (*itg2)->m_txRespFIFO.GetQueueSize();
                    for (int i = 0; i < pendingQueueSize && PendingTxRespg == false; i++)
                    {
                      pendingWbMsg_1 = (*itg2)->m_txRespFIFO.GetFrontElement();
                      (*itg2)->m_txRespFIFO.PopElement();
                      if ((pendingWbMsg_1.msgId == itgReq.msgId) && (pendingWbMsg_1.addr == itgReq.addr))
                      {

                        PendingTxRespg = true;
                        PendingTxResp = true;
                        if (ChkOnly == true)
                        {
                          m_delaycycle = 1;
                          (*itg2)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                        }

                        itx = itg2;
                        mode = 1;
                        selected = true;
                        selectedPrioInvert = true;
                        selectedPrioInvertIndex = m_respCoreCnt;
                        txResp = pendingWbMsg_1;
                      }
                      else
                      {
                        (*itg2)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                      }
                    }
                  }

                  m_GlobalQueue->m_GlobalReqFIFO.InsertElement(itgReq);
                }
                else
                {
                  m_GlobalQueue->m_GlobalReqFIFO.InsertElement(itgReq);
                }
              }
            }
          }
          if (oldest)
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(m_ServQueueMsg);
          else
            m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ServQueueMsg);
        }
      }

      // arbitrate the responses from cores
      for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && PendingTxWBResp == false; RR_iterator++)
      {
        if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
        {
          m_respCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
        }

        // get current size of the RxResp queue
        if (oldest)
          GlobalQueueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
        else
          GlobalQueueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();

        for (unsigned int itr = 0; itr < GlobalQueueSize && PendingTxWBResp == false; itr++)
        {
          if (oldest)
          {
            m_ServQueueMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
            m_GlobalQueue->m_GlobalOldestQueue.PopElement();
          }
          else
          {
            m_ServQueueMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
            m_GlobalQueue->m_GlobalReqFIFO.PopElement();
          }

          if (m_ServQueueMsg.msgId != 0 && m_ServQueueMsg.reqCoreId == m_respCoreCnt)
          {

            for (std::list<Ptr<BusIfFIFO>>::iterator it4 = m_busIfFIFO.begin(); it4 != m_busIfFIFO.end(); it4++)
            {
              int pendingQueueSize = (*it4)->m_txRespFIFO.GetQueueSize();
              for (int i = 0; i < pendingQueueSize && PendingTxWBResp == false; i++)
              {
                pendingWbMsg_2 = (*it4)->m_txRespFIFO.GetFrontElement();
                (*it4)->m_txRespFIFO.PopElement();

                if (((pendingWbMsg_2.addr >> (int)log2(m_cacheBlkSize)) == (m_ServQueueMsg.addr >> (int)log2(m_cacheBlkSize))) && PendingTxWBResp == false && pendingWbMsg_2.msgId == m_ServQueueMsg.msgId)
                {

                  if (selected)
                  {
                    if (selectedPrioInvert)
                    {
                      if (!(isHigherPrio(selectedPrioInvertIndex, pendingWbMsg_2.reqCoreId)))
                      {

                        if (ChkOnly == true)
                        {
                          m_delaycycle = 1;
                          (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                        }
                        else
                        {
                          if (selected)
                          {
                            (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                          }
                        }
                        mode = 2;
                        PendingTxWBResp = true;
                        txResp = pendingWbMsg_2;
                      }
                      else
                      {
                        (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                      }
                    }
                    else
                    {
                      if (!(isHigherPrio(pendingWbMsg_1.reqCoreId, pendingWbMsg_2.reqCoreId)))
                      {
                        if (ChkOnly == true)
                        {
                          m_delaycycle = 1;
                          (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                        }
                        else
                        {
                          if (selected)
                          {
                            (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                          }
                        }
                        mode = 2;
                        PendingTxWBResp = true;
                        txResp = pendingWbMsg_2;
                      }
                      else
                      {
                        (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                      }
                    }
                  }
                  else
                  {
                    if (ChkOnly == true)
                    {
                      (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                    }
                    mode = 2;
                    PendingTxWBResp = true;
                    txResp = pendingWbMsg_2;
                  }
                }
                else
                {
                  (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                }
              }
            }
          }
          else if (m_ServQueueMsg.msgId == 0 && m_ServQueueMsg.wbCoreId == m_respCoreCnt)
          {

            std::list<Ptr<BusIfFIFO>>::iterator it7 = m_busIfFIFO.begin();
            std::advance(it7, m_respCoreCnt);
            int pendingQueueSize = (*it7)->m_txRespFIFO.GetQueueSize();

            for (int i = 0; i < pendingQueueSize && PendingTxWBResp == false; i++)
            {

              pendingWbMsg_2 = (*it7)->m_txRespFIFO.GetFrontElement();

              (*it7)->m_txRespFIFO.PopElement();
              if (((pendingWbMsg_2.addr >> (int)log2(m_cacheBlkSize)) == (m_ServQueueMsg.addr >> (int)log2(m_cacheBlkSize))) && PendingTxWBResp == false && pendingWbMsg_2.respCoreId == m_respCoreCnt)
              {
                if (selected)
                {
                  if (selectedPrioInvert)
                  {
                    if (!(isHigherPrio(selectedPrioInvertIndex, pendingWbMsg_2.respCoreId)))
                    {

                      if (ChkOnly == true)
                      {
                        (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                      }
                      else
                      {
                        if (selected)
                        {
                          (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                        }
                      }
                      mode = 2;
                      PendingTxWBResp = true;
                      txResp = pendingWbMsg_2;
                    }
                    else
                    {
                      (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                    }
                  }
                  else
                  {
                    if (!(isHigherPrio(pendingWbMsg_1.reqCoreId, pendingWbMsg_2.respCoreId)))
                    {

                      if (ChkOnly == true)
                      {
                        (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                      }
                      else
                      {
                        if (selected)
                        {
                          (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                        }
                      }
                      mode = 2;
                      PendingTxWBResp = true;
                      txResp = pendingWbMsg_2;
                    }
                    else
                    {
                      (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                    }
                  }
                }
                else
                {
                  if (ChkOnly == true)
                  {
                    (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                  }
                  mode = 2;
                  PendingTxWBResp = true;
                  txResp = pendingWbMsg_2;
                }
              }
              else
              {
                (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
              }
            }
          }
          if (oldest)
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(m_ServQueueMsg);
          else
            m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ServQueueMsg);
        }
      }
    }

    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();

    if (PendingTxResp == true && PendingTxWBResp == false && ChkOnly == false)
    {
      if (m_logFileGenEnable)
      {
        std::cout << "BusArbiter: SharedMem granted TDM to send " << txResp.msgId << "response slot ============================================================ " << m_arbiCycle << "\n\n";
      }
      SendData(txResp, AGENT::CORE);
      SendData(txResp, AGENT::INTERCONNECT);
      return true;
    }
    else if (PendingTxWBResp == true && ChkOnly == false)
    {

      if (m_logFileGenEnable)
      {
        std::cout << "\nBusArbiter: Cpu " << (*it1)->m_fifo_id << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
      }
      SendData(txResp, AGENT::CORE);

      if (txResp.reqCoreId >= 10 || txResp.dualTrans == true)
      {
        if (m_logFileGenEnable)
        {
          std::cout << "\nBusArbiter: Cpu " << (*it1)->m_fifo_id << " Sent Data to Shared Mem =============================================================== " << m_arbiCycle << "\n\n";
        }
        if (txResp.reqCoreId >= 10)
        {

          unsigned int mask = createMask(6, 8);
          unsigned bank_num = mask & txResp.addr;
          bank_num = bank_num >> 6;
          txResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);
        }
        else
          txResp.sharedCacheAgent = 0;
        SendData(txResp, AGENT::SHAREDMEM);
      }
      SendData(txResp, AGENT::INTERCONNECT);
      return true;
    }
    return (PendingTxResp || PendingTxWBResp);
  }

  bool BusArbiter::CheckPendingFCFSResp(BusIfFIFO::BusRespMsg &txMsg, bool ChkOnly = true)
  {

    double arrivalTime  = 0;
    uint16_t core_idx   = 0;
    uint16_t TargetCore = 0;
    bool PendingTxReq   = false;
    BusIfFIFO::BusRespMsg pendingWbMsg_3;
    BusIfFIFO::BusReqMsg itgReq;
    BusIfFIFO::BusRespMsg issueTemp;
    unsigned int sharedCacheID;

    if ((ChkOnly && (m_delaycycle == 2)) || ((ChkOnly) && (m_arbiCycle == m_delaycyclereg)))
    {
      m_delaycycle = 0;
      return false;
    }
    if (!ChkOnly)
    {
      if (m_delaycycle == 1)
      {
        m_delaycyclereg = m_arbiCycle;
        m_delaycycle = 2;
      }
    }
    if (!ChkOnly)
    {
      issueTemp = txMsg;
      if (issueTemp.msgId == 0)
      {
        std::list<Ptr<BusIfFIFO>>::iterator itc = m_busIfFIFO.begin();
        std::advance(itc, issueTemp.respCoreId);
        int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
        for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
        {
          pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
          (*itc)->m_txRespFIFO.PopElement();
          if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
          {
            PendingTxReq = true;
          }
          else
          {
            (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
          }
        }
      }
      else
      {
        if (issueTemp.respCoreId < 10)
        {
          std::list<Ptr<BusIfFIFO>>::iterator itc = m_busIfFIFO.begin();
          std::advance(itc, issueTemp.respCoreId);
          int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
          for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
          {
            pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
            (*itc)->m_txRespFIFO.PopElement();
            if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
            {
              PendingTxReq = true;
            }
            else
            {
              (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
            }
          }
        }
        else if (issueTemp.respCoreId >= 10)
        {
          // if it is not a wirte back resp, we should check for the shared banks
          std::list<Ptr<BusIfFIFO>>::iterator itc = m_sharedCacheBusIfFIFO.begin();
          sharedCacheID = retrieveSharedCacheID(issueTemp.sharedCacheAgent);
          std::advance(itc, sharedCacheID);
          int pendingQueueSize = (*itc)->m_txRespFIFO.GetQueueSize();
          for (int itc1 = 0; itc1 < pendingQueueSize; itc1++)
          {
            pendingWbMsg_3 = (*itc)->m_txRespFIFO.GetFrontElement();
            (*itc)->m_txRespFIFO.PopElement();
            if (pendingWbMsg_3.msgId == issueTemp.msgId && pendingWbMsg_3.addr == issueTemp.addr)
            {
              PendingTxReq = true;
            }
            else
            {
              (*itc)->m_txRespFIFO.InsertElement(pendingWbMsg_3);
            }
          }
        }
      }
    }

    BusIfFIFO::BusRespMsg txMsgMem;

    if (ChkOnly)
    {
      for (std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end(); it1++)
      {
        if ((*it1)->m_txRespFIFO.IsEmpty() == false)
        {
          txMsg = (*it1)->m_txRespFIFO.GetFrontElement();

          if ((txMsg.msgId != 0 && txMsg.reqCoreId == 1) || (txMsg.msgId == 0 && txMsg.respCoreId == 1) || (txMsg.respCoreId < 10 && txMsg.reqCoreId < 10 && txMsg.respCoreId == 1) || (txMsg.respCoreId < 10 && txMsg.reqCoreId < 10 && txMsg.reqCoreId == 1))
          {

            txMsg.timestamp = 0;
          }
          if ((PendingTxReq == false) || arrivalTime > txMsg.timestamp)
          {
            arrivalTime = txMsg.timestamp;
            TargetCore = core_idx;
          }
          m_delaycycle = 1;
          PendingTxReq = true;
        }
        core_idx++;
      }

      for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_sharedCacheBusIfFIFO.begin(); it2 != m_sharedCacheBusIfFIFO.end(); it2++)
      {
        if ((*it2)->m_txRespFIFO.IsEmpty() == false)
        {
          BusIfFIFO::BusRespMsg txMsgMem_temp;
          BusIfFIFO::BusRespMsg txMsgMem_temp_1;

          // in this queue, not always the front element is oldest.... perhaps somewhere the order is changed. Find the oldest one

          BusIfFIFO::BusRespMsg txMsgMem_temp_2;
          if (!(*it2)->m_txRespFIFO.IsEmpty())
          {
            for (int i = 0; i < (*it2)->m_txRespFIFO.GetQueueSize(); i++)
            {
              txMsgMem_temp_2 = (*it2)->m_txRespFIFO.GetFrontElement();
              if ((txMsgMem_temp_2.msgId != 0 && txMsgMem_temp_2.reqCoreId == 1) || (txMsgMem_temp_2.msgId == 0 && txMsgMem_temp_2.respCoreId == 1))
              {
                txMsgMem_temp_2.timestamp = 0;
              }
              (*it2)->m_txRespFIFO.PopElement();
              (*it2)->m_txRespFIFO.InsertElement(txMsgMem_temp_2);
            }
          }

          txMsgMem_temp = (*it2)->m_txRespFIFO.GetFrontElement();

          if (!(*it2)->m_txRespFIFO.IsEmpty())
          {
            for (int i = 0; i < (*it2)->m_txRespFIFO.GetQueueSize(); i++)
            {

              txMsgMem_temp_1 = (*it2)->m_txRespFIFO.GetElement(i);

              if (txMsgMem_temp_1.timestamp < txMsgMem_temp.timestamp)
              {
                txMsgMem_temp = (*it2)->m_txRespFIFO.GetElement(i);
              }
            }
          }

          if ((PendingTxReq == false) || arrivalTime > txMsgMem_temp.timestamp)
          {
            txMsgMem = txMsgMem_temp;
            arrivalTime = txMsgMem.timestamp;
            TargetCore = (*it2)->m_fifo_id;
          }
          m_delaycycle = 1;
          PendingTxReq = true;
        }
      }

      if (TargetCore >= 10)
      {
        txMsg = txMsgMem;
      }
      else
      {

        std::list<Ptr<BusIfFIFO>>::iterator it4 = m_busIfFIFO.begin();
        std::advance(it4, TargetCore);
        txMsg = (*it4)->m_txRespFIFO.GetFrontElement();
      }
    }

    if (PendingTxReq)
    {
      if (ChkOnly == false)
      {
        if (m_logFileGenEnable)
        {
          if (TargetCore >= 10)
          {
            std::cout << "\nBusArbiter: SharedMem " << TargetCore << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
          }
          else
          {
            std::cout << "\nBusArbiter: Cpu " << TargetCore << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
          }
        }
        SendData(txMsg, AGENT::CORE);
        if (txMsg.reqCoreId >= 10 || txMsg.dualTrans == true)
        {
          if (m_logFileGenEnable)
          {
            std::cout << "\nBusArbiter: Cpu " << TargetCore << " Sent Data to Shared Mem =============================================================== " << m_arbiCycle << "\n\n";
          }
          SendData(txMsg, AGENT::SHAREDMEM);
        }

        SendData(txMsg, AGENT::INTERCONNECT);
      }
    }
    return PendingTxReq;
  }

  bool BusArbiter::CheckPendingReq(uint16_t core_idx, BusIfFIFO::BusReqMsg &txMsg, bool CheckOnly = false, bool oldest = true)
  {
    if (oldest)
    {
      PendingTxReq = false;
      std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
      std::advance(it1, core_idx);
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {

        txMsg = (*it1)->m_txMsgFIFO.GetFrontElement();

        for (int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && PendingTxReq == false; it2++)
        {
          BusIfFIFO::BusReqMsg tempMsgQueue;
          tempMsgQueue = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
          m_GlobalQueue->m_GlobalOldestQueue.PopElement();

          if (txMsg.msgId != 0 && txMsg.reqCoreId == tempMsgQueue.reqCoreId && txMsg.addr == tempMsgQueue.addr && txMsg.cohrMsgId == tempMsgQueue.cohrMsgId && existServiceQueue(tempMsgQueue))
          {

            txMsg.becameOldest = tempMsgQueue.becameOldest;
            PendingTxReq = true;
          }
          else if (txMsg.msgId == 0 && txMsg.wbCoreId == tempMsgQueue.wbCoreId && txMsg.addr == tempMsgQueue.addr && txMsg.cohrMsgId == tempMsgQueue.cohrMsgId)
          {
            txMsg.becameOldest = tempMsgQueue.becameOldest;
            PendingTxReq = true;
          }
          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempMsgQueue);
        }
        // in oldest, we are not checking the bank condition whatsoever since it has higher priority in any case
        if (PendingTxReq && CheckOnly == false)
        {

          (*it1)->m_txMsgFIFO.PopElement();
        }
      }
    }
    else
    {

      BusIfFIFO::BusReqMsg txResp;
      std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
      std::advance(it1, core_idx);
      bool candidate_chosen = false;

      int pendingQueueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {
        if (CheckOnly == true)
        {
          PendingTxReq = false;
          for (int iterator = 0; iterator < (*it1)->m_txMsgFIFO.GetQueueSize() && candidate_chosen == false; iterator++)
          {
            PendingTxReq = true;
            txMsg = (*it1)->m_txMsgFIFO.GetElement(iterator);
            for (int j = 0; (j < pendingQueueSize && PendingTxReq == true); j++)
            {
              txResp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
              m_GlobalQueue->m_GlobalOldestQueue.PopElement();

              if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId != 0 && CheckOnly == true) ||
                  (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId == 0 && CheckOnly == true) ||
                  (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) ||
                  (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
              {
                PendingTxReq = false;
              }
              m_GlobalQueue->m_GlobalOldestQueue.InsertElement(txResp);
            }

            if (PendingTxReq)
            {
              for (int j = 0; (j < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() && PendingTxReq == true); j++)
              {
                txResp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
                m_GlobalQueue->m_GlobalReqFIFO.PopElement();

                if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId != 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
                {
                  PendingTxReq = false;
                }
                m_GlobalQueue->m_GlobalReqFIFO.InsertElement(txResp);
              }
            }

            if (PendingTxReq && CheckOnly == true)
            {
              candidateID = iterator;
              candidate_chosen = true;
            }
          }
        }
        else
        {
          txMsg = (*it1)->m_txMsgFIFO.GetElement(candidateID);

          if (PendingTxReq)
          {

            (*it1)->m_txMsgFIFO.EraseElement(candidateID);
            PendingTxReq = false;
          }
        }
      }
    }
    return PendingTxReq;
  }

  bool BusArbiter::InsertOnReqBus(BusIfFIFO::BusReqMsg txMsg)
  { // it takes a coherence message that is picked by the requestBus Arbiter and broadcasts it.
    m_GlobalQueue->m_totalMsG++;

    if (m_logFileGenEnable)
    {
      if (txMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans)
      {
        std::cout << "\nBusArbiter: Cpu " << txMsg.wbCoreId << " granted TDM request slot =============================================================== " << m_arbiCycle << std::endl;
      }
      else
      {
        std::cout << "\nBusArbiter: Cpu " << txMsg.reqCoreId << " granted TDM request slot =============================================================== " << m_arbiCycle << std::endl;
      }
    }
    // broadcast requests to all cores (snooping based)
    for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++)
    {
      if (!(*it2)->m_rxMsgFIFO.IsFull())
      {

        (*it2)->m_rxMsgFIFO.InsertElement(txMsg);
      }
      else
      {
        if (m_logFileGenEnable)
        {
          std::cout << "BusArbiter: cannot insert new messages into PrivCache Ctrl buffers" << std::endl;
        }
        exit(0);
      }
    }
    // send request to Shared Mem controller as well

    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin();
    std::advance(it1, retrieveSharedCacheID(txMsg.sharedCacheAgent));
    if (!(*it1)->m_rxMsgFIFO.IsFull())
    {

      (*it1)->m_rxMsgFIFO.InsertElement(txMsg);
    }
    else
    {
      if (m_logFileGenEnable)
      {
        std::cout << "BusArbiter: cannot insert new messages into SharedMem RxMsg buffers" << std::endl;
      }
      exit(0);
    }
    // send message to Interconnect FIFO
    if (!m_interConnectFIFO->m_ReqMsgFIFO.IsEmpty())
    {
      m_interConnectFIFO->m_ReqMsgFIFO.PopElement();
    }
    txMsg.cycle = m_arbiCycle;
    m_interConnectFIFO->m_ReqMsgFIFO.InsertElement(txMsg);
    return true;
  }

  void BusArbiter::RR_RT_ReqBus()
  {
    /*** Request Bus Arbiter for the RT Controller ***/

    bool next_arb_level = true;

    // insert message on the request bus (broadcast)
    // in the slot, if there was a pending request chosen before, now its time to broadcast. and finally advance the TDM
    if (m_PndReq)
    {

      if (m_ReqBusMsg.msgId == 0)
        m_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, false, isOldest(m_ReqBusMsg.addr, m_ReqBusMsg.wbCoreId));
      else
        m_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, false, isOldest(m_ReqBusMsg.addr, m_ReqBusMsg.reqCoreId));

      m_PndReq = false;
      // insert message on request bus

      InsertOnReqBus(m_ReqBusMsg);

      if (m_ReqBusMsg.NoGetMResp == false)
      {
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
      }
    }

    bool temp_PndReq = false;
    /********************* First Level of Arbitration  ************************/
    for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && m_PndReq == false; RR_iterator++)
    {

      if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
      {
        m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);

        temp_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, true, true); // CheckPendingReq returns true the front element of the buffer

        m_PndReq = temp_PndReq;
      }
    }
    // if a Request is chosen, no need for second level of arbitration
    if (m_PndReq)
    {
      m_PndOld = true;
      next_arb_level = false;
    } /* if a msg is picked at first level, there is no need for the second level of arbiteration. */

    /********************* Second Level of Arbitration  ************************/

    if (next_arb_level)
    {
      bool temp_PndReq = false;
      for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && m_PndReq == false; RR_iterator++)
      {

        // take the order based on the RR queue
        if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
        {
          m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
          temp_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, true, false);

          m_PndReq = temp_PndReq;
          if (m_PndReq)
            m_PndOld = false;
        }
      }
    }

    // wait one TDM Request slot, if there is any request
    if (m_PndReq)
    {
      m_GlobalQueue->m_busArbBlock = m_reqclks;

      Simulator::Schedule(NanoSeconds(m_dt * m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }
    else
    { // wait one clock cycle and check again
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }
  }

  bool BusArbiter::isOldest(uint64_t adr, unsigned int coreIndex)
  {

    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg;
    int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
      m_GlobalQueue->m_GlobalOldestQueue.PopElement();
      if ((tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex) || ((tempOldestReqMsg.addr == adr && tempOldestReqMsg.wbCoreId == coreIndex) && tempOldestReqMsg.msgId == 0))
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

  bool BusArbiter::removeFromM_Type(uint64_t adr, unsigned int coreIndex, bool replacement)
  {
    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg_removeFromMtype;
    int queueSize = m_GlobalQueue->m_MsgType.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg_removeFromMtype = m_GlobalQueue->m_MsgType.GetFrontElement();
      m_GlobalQueue->m_MsgType.PopElement();
      if ((replacement == false && tempOldestReqMsg_removeFromMtype.addr == adr && tempOldestReqMsg_removeFromMtype.reqCoreId == coreIndex) ||
          (replacement == true && tempOldestReqMsg_removeFromMtype.addr == adr && tempOldestReqMsg_removeFromMtype.wbCoreId == coreIndex))
      {

        success = true;
        return true;
      }
      m_GlobalQueue->m_MsgType.InsertElement(tempOldestReqMsg_removeFromMtype);
    }
    return success;
  }

  bool BusArbiter::removeFromOldest(uint64_t adr, unsigned int coreIndex, bool replacement)
  {
    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg;
    int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
      m_GlobalQueue->m_GlobalOldestQueue.PopElement();
      if ((replacement == false && tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex) ||
          (replacement == true && tempOldestReqMsg.addr == adr && tempOldestReqMsg.wbCoreId == coreIndex))
      {
        success = true;
        return true;
      }
      m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
    }
    return success;
  }

  void BusArbiter::adjustOldest(unsigned int coreIndex)
  {

    bool temporal = false;

    double arrivalTime = 0;
    bool PendingTxReq = false;
    unsigned int queueSize = 0;

    BusIfFIFO::BusReqMsg tempFCFSMsg;
    BusIfFIFO::BusReqMsg tempCandidate;

    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();

    std::advance(it1, coreIndex);

    // First take the oldest REQ from IF FIFO request buffer
    if ((*it1)->m_txMsgFIFO.IsEmpty() == false)
    {
      tempFCFSMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
      arrivalTime = tempFCFSMsg.timestamp;
      PendingTxReq = true;
      tempCandidate = tempFCFSMsg;
    }

    // Second parse the non oldest requests that already was sent into the banks
    queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();

    for (unsigned int itr = 0; itr < queueSize; itr++)
    {
      if (m_GlobalQueue->m_GlobalReqFIFO.IsEmpty() == false)
      {
        tempFCFSMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();

        m_GlobalQueue->m_GlobalReqFIFO.PopElement();
        if ((tempFCFSMsg.msgId != 0 && tempFCFSMsg.reqCoreId == coreIndex) || (tempFCFSMsg.msgId == 0 && tempFCFSMsg.wbCoreId == coreIndex))
        {
          if ((PendingTxReq == false) || arrivalTime > tempFCFSMsg.timestamp)
          {
            temporal = true;
            PendingTxReq = true;
            arrivalTime = tempFCFSMsg.timestamp;
            tempCandidate = tempFCFSMsg;
          }
        }
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempFCFSMsg);
      }
    }
    if (PendingTxReq == true)
    {
      tempCandidate.becameOldest = m_arbiCycle;
      m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempCandidate);
      m_GlobalQueue->m_GlobalRRQueue.push_back(coreIndex);

      if (temporal)
      {
        BusIfFIFO::BusReqMsg tempitr7;
        for (int itr7 = 0; itr7 < m_GlobalQueue->m_MsgType.GetQueueSize(); itr7++)
        {
          tempitr7 = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if (tempitr7.msgId == tempCandidate.msgId && tempitr7.addr == tempCandidate.addr)
          {
            tempitr7.becameOldest = m_arbiCycle;
            //cout<<"It is being oldest at  "<<tempitr7.becameOldest<<endl;
            assignDeadlineAfterDetermination(tempitr7);
            m_GlobalQueue->m_MsgType.InsertElement(tempitr7);
          }
          else
          {
            m_GlobalQueue->m_MsgType.InsertElement(tempitr7);
          }
        }
      }
    }
  }

  bool BusArbiter::removeFromNonOldest(uint64_t adr, unsigned int coreIndex, bool replacement)
  {
    bool success = false;
    BusIfFIFO::BusReqMsg tempNonOldestReqMsg;
    int queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempNonOldestReqMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
      m_GlobalQueue->m_GlobalReqFIFO.PopElement();
      if ((replacement == false && tempNonOldestReqMsg.addr == adr && tempNonOldestReqMsg.reqCoreId == coreIndex) ||
          (replacement == true && tempNonOldestReqMsg.addr == adr && tempNonOldestReqMsg.wbCoreId == coreIndex))
      {

        success = true;
        return true;
      }
      m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsg);
    }
    return success;
  }

  bool BusArbiter::isFinal(uint64_t adr, unsigned int messageID, uint64_t sharedID)
  {

    BusIfFIFO::BusRespMsg BusRespMsg_RXtemp;
    std::list<Ptr<BusIfFIFO>>::iterator it7 = m_sharedCacheBusIfFIFO.begin();
    std::advance(it7, retrieveSharedCacheID(sharedID));
    if (!(*it7)->m_rxRespFIFO.IsEmpty())
    {
      for (int itr_temp = 0; itr_temp < (*it7)->m_rxRespFIFO.GetQueueSize(); itr_temp++)
      {
        BusRespMsg_RXtemp = (*it7)->m_rxRespFIFO.GetFrontElement();
        (*it7)->m_rxRespFIFO.PopElement();
        if (BusRespMsg_RXtemp.addr == adr && BusRespMsg_RXtemp.msgId == messageID)
        {
          (*it7)->m_rxRespFIFO.InsertElement(BusRespMsg_RXtemp);
          return false;
        }
        (*it7)->m_rxRespFIFO.InsertElement(BusRespMsg_RXtemp);
      }
      return true;
    }
    return true;
  }

  void BusArbiter::RR_RT_RespBus()
  {

    if (m_PndResp)
    {
      m_GlobalQueue->m_totalResp++;

      if (m_PendResp.msgId == 0)
        CheckPendingResp(m_PendResp, false, isOldest(m_PendResp.addr, m_PendResp.respCoreId), mode);
      else
        CheckPendingResp(m_PendResp, false, isOldest(m_PendResp.addr, m_PendResp.reqCoreId), mode);

      if (m_duetto)
      {
        bool terminateii = false;
        for (int ii1 = 0; ii1 < m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii1++)
        {
          BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if (tempS.msgId == m_PendResp.msgId && tempS.addr == m_PendResp.addr)
          {
            tempS.currStage = "RESPONSE";
            terminateii = true;
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
          else
          {
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
        }
      }

      if (m_PendResp.msgId != 0)
      {
        unsigned int mask = createMask(6, 8);
        unsigned bank_num = mask & m_PendResp.addr;
        bank_num = bank_num >> 6;
        m_PendResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

        if (isFinal(m_PendResp.addr, m_PendResp.msgId, m_PendResp.sharedCacheAgent))
        {
          if (isOldest(m_PendResp.addr, m_PendResp.reqCoreId))
          {

            for (unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size(); h++)
            {
              if (m_GlobalQueue->m_GlobalRRQueue.at(h) == m_PendResp.reqCoreId)
              {
                m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
              }
            }
            removeFromOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);

            removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);

            removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
            adjustOldest(m_PendResp.reqCoreId);
          }
          else
          {
            removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);
            removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
          }
        }
        else
        {

          //abort();
        }
      }
    }

    oldest_determine = true;
    m_PndResp = false;
    m_PndResp = CheckPendingResp(m_PendResp, true, true, mode);
    if (m_PndResp == false)
    {
      m_PndResp = CheckPendingResp(m_PendResp, true, false, mode);
    }

    if (m_PndResp == true)
    {
      m_GlobalQueue->m_respArbBlock = m_respclks;

      Simulator::Schedule(NanoSeconds(m_dt * m_respclks), &BusArbiter::RespStep, Ptr<BusArbiter>(this));
    }
    else
    { // wait one clock cycle and check again
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::RespStep, Ptr<BusArbiter>(this));
    }
  }

  void BusArbiter::init()
  {
    BusArbDecode();
    Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::RespStep, Ptr<BusArbiter>(this));
    Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::Step, Ptr<BusArbiter>(this));
    Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::L2CohrStep, Ptr<BusArbiter>(this));
  }
  void BusArbiter::L2CohrMsgHandle()
  {
    BusIfFIFO::BusReqMsg txreqMsg;

    for (std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin(); it1 != m_sharedCacheBusIfFIFO.end(); it1++)
    {
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {
        txreqMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
        if (txreqMsg.cohrMsgId == SNOOPPrivCohTrans::ExclTrans)
        {
          SendMemCohrMsg(txreqMsg, false);
        }
        else
        {
          SendMemCohrMsg(txreqMsg, true);
        }
        (*it1)->m_txMsgFIFO.PopElement();
      }
    }
    Simulator::Schedule(NanoSeconds(m_dt / 4), &BusArbiter::L2CohrStep, Ptr<BusArbiter>(this));
  }

  void BusArbiter::ReqFncCall()
  {
    if (m_duetto)
    {
      /**** Logic to determine the arbiter****/
      if (arb_req_mode == "RT")
      {
        RR_RT_ReqBus();
      }
      else if (arb_req_mode == "HP")
      {
        MSI_FcFsReqBus();
      }
    }
    else
    {
      if (m_bus_arb == BusARBType::UNIFIED_TDM_ARB)
      {
        //Unified_TDM_PMSI_Bus2();
      }
      else if (m_bus_arb == BusARBType::PISCOT_ARB)
      {
        if (m_cohProType == CohProtType::SNOOP_MSI ||
            m_cohProType == CohProtType::SNOOP_MESI ||
            m_cohProType == CohProtType::SNOOP_MOESI)
        {
          //PISCOT_OOO_MSI_TDM_ReqBus();
        }
        else if (m_cohProType == CohProtType::SNOOP_PMSI)
        {
          // PMSI_OOO_TDM_ReqBus ();
        }
        else
        {
          std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl;
          exit(0);
        }
      }
      else if (m_bus_arb == BusARBType::FCFS_ARB)
      {
        if (m_cohProType == CohProtType::SNOOP_MSI || m_cohProType == CohProtType::SNOOP_PMSI || m_cohProType == CohProtType::SNOOP_MESI || m_cohProType == CohProtType::SNOOP_MOESI)
        {
          MSI_FcFsReqBus();
        }
        else
        {
          std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl;
          exit(0);
        }
      }
      else if (m_bus_arb == BusARBType::RT_ARB)
      {
        RR_RT_ReqBus();
      }
    }
  }
  void BusArbiter::RespFncCall()
  {

    if (m_duetto)
    {
      /**** Logic to determine the arbiter****/
      if (arb_resp_mode == "RT")
      {
        RR_RT_RespBus();
      }
      else if (arb_resp_mode == "HP")
      {
        MSI_FcFsRespBus();
      }
    }
    else
    {
      if (m_bus_arb == BusARBType::UNIFIED_TDM_ARB)
      {
        std::cout << "BusArbiter: [Info] Unified TDM Bus response scheduling is handled using Unified_TDM_PMSI_Bus function call" << std::endl;
      }
      else if (m_bus_arb == BusARBType::PISCOT_ARB ||
               m_bus_arb == BusARBType::FCFS_ARB)
      {
        if (m_cohProType == CohProtType::SNOOP_MSI ||
            m_cohProType == CohProtType::SNOOP_MESI ||
            m_cohProType == CohProtType::SNOOP_MOESI)
        {
          if (m_bus_arb == BusARBType::PISCOT_ARB)
          {
            // PISCOT_MSI_FcFsResBus();
          }
          else
          {
            MSI_FcFsRespBus();
          }
        }
        else if (m_cohProType == CohProtType::SNOOP_PMSI)
        {
          //PMSI_FcFsRespBus();
        }
        else
        {
          std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl;
          exit(0);
        }
      }
      else if (m_bus_arb == BusARBType::RT_ARB)
      {
        RR_RT_RespBus();
      }
    }
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::BusArbDecode()
  {
    if (m_bus_arch == "unified")
    {
      if (m_cohProType != CohProtType::SNOOP_PMSI)
      {
        exit(0);
      }

      if (m_bus_arbiter == "PMSI" || (m_bus_arbiter == "CUSTOM" && m_reqbus_arb == "TDM" && m_respbus_arb == "TDM"))
      {
        m_bus_arb = BusARBType::UNIFIED_TDM_ARB;
      }
      else
      {
        exit(0);
      }

      if (m_cach2Cache == true && m_bus_arbiter == "PMSI")
      {
        exit(0);
      }
    }
    else if (m_bus_arch == "split")
    {
      if (m_bus_arbiter == "PISCOT")
      {
        m_bus_arb = BusARBType::PISCOT_ARB;
      }
      else if (m_bus_arbiter == "FCFS")
      {
        m_bus_arb = BusARBType::FCFS_ARB;
      }
      else if (m_bus_arbiter == "RT")
      {
        m_bus_arb = BusARBType::RT_ARB;
      }
      else
      {
        exit(0);
      }
      if (!(m_cohProType == CohProtType::SNOOP_PMSI || m_cohProType == CohProtType::SNOOP_MSI || m_cohProType == CohProtType::SNOOP_MESI || m_cohProType == CohProtType::SNOOP_MOESI))
      {
        exit(0);
      }
    }
  }
  /**
    * Call request/response bus functions
    * These function is called each interval dt
    */
  void BusArbiter::ReqStep(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->ReqFncCall();
  }
  void BusArbiter::RespStep(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->RespFncCall();
  }
  void BusArbiter::L2CohrStep(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->L2CohrMsgHandle();
  }
  // Schedule the next run
  void BusArbiter::Step(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->deadlineProgress();
    busArbiter->CycleAdvance();

    busArbiter->m_GlobalQueue->bank_mode = busArbiter->arb_req_mode;

    if (busArbiter->m_duetto)
    {
      /**** Logic to determine the arbiter****/
      if (busArbiter->WCLator())
      {
        busArbiter->HPCycle++;
        if (busArbiter->m_reza_log)
          cout << "---------------------------------------------------------------------------------------------------------------------------------------------- REQ Arbiter HP Mode" << endl;
        if (busArbiter->arb_req_mode == "RT")
        {
          busArbiter->SwitchRTHP++;
          busArbiter->m_PndReq = false;
          if (busArbiter->m_reza_log)
            cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from RT to HP  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        }
        busArbiter->arb_req_mode = "HP";
        if (busArbiter->m_reza_log)
          cout << "---------------------------------------------------------------------------------------------------------------------------------------------- RESP Arbiter HP Mode" << endl;
        if (busArbiter->arb_resp_mode == "RT")
        {
          if (busArbiter->m_reza_log)
            cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from RT to HP  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        }
        busArbiter->arb_resp_mode = "HP";
      }
      else
      {
        busArbiter->RTCycle++;
        if (busArbiter->m_reza_log)
          cout << "----------------------------------------------------------------------------------------------------------------------------------------------  REQ Arbiter RT Mode " << endl;
        if (busArbiter->arb_req_mode == "HP")
        {
          busArbiter->SwitchHPRT++;
          busArbiter->m_PndReq = false;
          if (busArbiter->m_reza_log)
            cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from HP to RT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        }
        busArbiter->arb_req_mode = "RT";

        if (busArbiter->m_reza_log)
          cout << "---------------------------------------------------------------------------------------------------------------------------------------------- RESP Arbiter RT Mode " << endl;
        if (busArbiter->arb_resp_mode == "HP")
        {
          if (busArbiter->m_reza_log)
            cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from HP to RT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        }
        busArbiter->arb_resp_mode = "RT";
      }
    }
  }
  void BusArbiter::CycleAdvance()
  {

    if (m_GlobalQueue->m_busArbBlock > 0)
      m_GlobalQueue->m_busArbBlock--;
    if (m_GlobalQueue->m_respArbBlock > 0)
      m_GlobalQueue->m_respArbBlock--;

    if (m_stallDetectionEnable)
    {
      m_stall_cnt = (m_PndReq) ? 0 : (m_stall_cnt + 1);

      if (m_stall_cnt >= Stall_CNT_LIMIT)
      {
        exit(0);
      }
    }

    m_arbiCycle++;

    if (m_arbiCycle % 100000000 == 0)
    {
      cout << "********************************************************************************************************************************************************" << endl;
      cout << "********************************************************************************************************************************************************" << endl;
      cout << "HeartBeat     :: " << m_arbiCycle << " C2C Transfer :: " << m_GlobalQueue->m_Core2CoreTransfer << endl;
      cout << "Bank Transfer :: " << m_GlobalQueue->m_SharedBankTransfer << endl;
      cout << "Total Msg Transfered :: " << m_GlobalQueue->m_totalMsG << endl;
      cout << "Total Resp Transfered :: " << m_GlobalQueue->m_totalResp << endl;
      cout << "Total Replaccement :: " << m_GlobalQueue->m_totalReplacement << endl;
      cout << "Total L1 Hits " << m_GlobalQueue->m_totalL1Hit << endl;
      cout << "RT Cycle " << RTCycle << endl;
      cout << "HP Cycle " << HPCycle << endl;
      cout << "SW to HP " << SwitchRTHP << endl;
      cout << "SW to RT " << SwitchHPRT << endl;
      cout << "Kreturn  " << kreturn << endl;
      cout << "Core 0 L1 Hit " << m_GlobalQueue->m_L1Hit_0 << endl;
      cout << "Core 1 L1 Hit " << m_GlobalQueue->m_L1Hit_1 << endl;
      cout << "Core 2 L1 Hit " << m_GlobalQueue->m_L1Hit_2 << endl;
      cout << "Core 3 L1 Hit " << m_GlobalQueue->m_L1Hit_3 << endl;
      cout << "Core 4 L1 Hit " << m_GlobalQueue->m_L1Hit_4 << endl;
      cout << "Core 5 L1 Hit " << m_GlobalQueue->m_L1Hit_5 << endl;
      cout << "Core 6 L1 Hit " << m_GlobalQueue->m_L1Hit_6 << endl;
      cout << "Core 7 L1 Hit " << m_GlobalQueue->m_L1Hit_7 << endl;
      cout << "Core2Core from 0 " << m_GlobalQueue->m_Core2CoreTransfer_0 << endl;
      cout << "Core2Core from 1 " << m_GlobalQueue->m_Core2CoreTransfer_1 << endl;
      cout << "Core2Core from 2 " << m_GlobalQueue->m_Core2CoreTransfer_2 << endl;
      cout << "Core2Core from 3 " << m_GlobalQueue->m_Core2CoreTransfer_3 << endl;
      cout << "Core2Core from 4 " << m_GlobalQueue->m_Core2CoreTransfer_4 << endl;
      cout << "Core2Core from 5 " << m_GlobalQueue->m_Core2CoreTransfer_5 << endl;
      cout << "Core2Core from 6 " << m_GlobalQueue->m_Core2CoreTransfer_6 << endl;
      cout << "Core2Core from 7 " << m_GlobalQueue->m_Core2CoreTransfer_7 << endl;
      cout << "B2Core to 0 " << m_GlobalQueue->m_SharedBankTransfer_0 << endl;
      cout << "B2Core to 1 " << m_GlobalQueue->m_SharedBankTransfer_1 << endl;
      cout << "B2Core to 2 " << m_GlobalQueue->m_SharedBankTransfer_2 << endl;
      cout << "B2Core to 3 " << m_GlobalQueue->m_SharedBankTransfer_3 << endl;
      cout << "B2Core to 4 " << m_GlobalQueue->m_SharedBankTransfer_4 << endl;
      cout << "B2Core to 5 " << m_GlobalQueue->m_SharedBankTransfer_5 << endl;
      cout << "B2Core to 6 " << m_GlobalQueue->m_SharedBankTransfer_6 << endl;
      cout << "B2Core to 7 " << m_GlobalQueue->m_SharedBankTransfer_7 << endl;
      cout << "Priority Inversion " << priorityInversion << endl;
      cout << "********************************************************************************************************************************************************" << endl;
      cout << "********************************************************************************************************************************************************" << endl;
    }

    Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::Step, Ptr<BusArbiter>(this));
  }
}
