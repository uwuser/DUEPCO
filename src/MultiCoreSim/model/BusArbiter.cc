/*
 * File  :      BusArbiter.cc
 * Author:      Reza Mirosanlou
 * Email :      rmirosan@uwaterloo.ca
 *
 * Created On April 02, 2021
 */

#include "BusArbiter.h"
#include <algorithm>    // std::max
#include <math.h>       /* ceil */

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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
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
    m_reza_log = true;
    m_wcArbiter = 0;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  BusArbiter::~BusArbiter()
  {
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetCacheBlkSize(uint32_t cacheBlkSize)
  {
    m_cacheBlkSize = cacheBlkSize;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetDt(double dt)
  {
    m_dt = dt; // dt is cpuClkPeriod
  }

  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  int BusArbiter::GetDt()
  {
    return m_dt;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg & msg) {       
    if (m_reza_log) cout<<"in BusArbiter::assignDeadlineAfterDetermination "<<endl;
    unsigned int WCL_0;
    unsigned int WCL_1;
    unsigned int WCL_2;
    WCL_0 = 156;
    WCL_1 = 156;
    WCL_2 = 156; 
    if(msg.orderDetermined) {
      if (m_reza_log) cout<<"order determined"<<endl;
      switch (msg.orderofArbitration) {
        case 0: 
          msg.associateDeadline = WCL_0 - (m_arbiCycle - msg.becameOldest);
          if (m_reza_log) cout<<"msg.associateDeadline "<<msg.associateDeadline<<endl;
          break;
        case 1: 
          msg.associateDeadline = WCL_1 - (m_arbiCycle - msg.becameOldest);
          if (m_reza_log) cout<<"msg.associateDeadline "<<msg.associateDeadline<<endl;
          break;
        case 2: 
          msg.associateDeadline = WCL_2 - (m_arbiCycle - msg.becameOldest);
          if (m_reza_log) cout<<"msg.associateDeadline "<<msg.associateDeadline<<endl;
          break;
      }
      msg.associateDeadline_final = true;
    }      
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetClkSkew(double clkSkew)
  {
    m_clkSkew = clkSkew;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetIsWorkConserv(bool workConservFlag)
  {
    m_workconserv = workConservFlag;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetIsDuetto(bool duetto)
  {
    m_duetto = duetto;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetNumPrivCore(int nPrivCores)
  {
    m_cpuCore = nPrivCores; // since we have the same amount of cores as private caches
  }

  uint32_t BusArbiter::retrieveSharedBankCacheLine (uint64_t phy_addr) {          
    uint32_t cacheLine; 
    cacheLine = (phy_addr >> (int) log2(m_cacheBlkSize)) & (m_nsets-1);  
    return cacheLine;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  unsigned BusArbiter::createMask(unsigned a, unsigned b)
  {
    unsigned r = 0;
    for (unsigned i = a; i <= b; i++)
      r |= 1 << i;
    return r;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetCacheNsets (uint32_t nsets) {
    m_nsets = nsets;
  }
  void BusArbiter::SetSharedCacheLatency (uint32_t latency)  {    // Modified to Accomodate Multi Shared Cache
    m_sharedbankclks = latency;                                    // Modified to Accomodate Multi Shared Cache
  }  

  void BusArbiter::SetNumReqCycles(int ncycle)
  {
    m_reqclks = ncycle;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetNumRespCycles(int ncycle)
  {
    m_respclks = ncycle;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetCache2Cache(bool cach2Cache)
  {
    m_cach2Cache = cach2Cache;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetBusArchitecture(string bus_arch)
  {
    m_bus_arch = bus_arch;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetBusArbitration(string bus_arb)
  {
    m_bus_arbiter = bus_arb;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetReqBusArb(string reqbus_arb)
  {
    m_reqbus_arb = reqbus_arb;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetRespBusArb(string respbus_arb)
  {
    m_respbus_arb = respbus_arb;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetCohProtType(CohProtType ptype)
  {
    m_cohProType = ptype;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetMaxPendingReq(int maxPendingReq)
  {
    m_maxPendingReq = maxPendingReq;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::SetLogFileGenEnable(bool logFileGenEnable)
  {
    m_logFileGenEnable = logFileGenEnable;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::MSI_FcFsRespBus()
  {
    if (m_reza_log) cout<<"In MSI_FcFsRespBus "<<endl;
    if (m_PndResp)
    {
     if (m_reza_log) cout<<"In MSI_FcFsRespBus 1"<<endl;
     if(m_duetto) {
       if (m_reza_log) cout<<"In MSI_FcFsRespBus 2"<<endl;
        bool terminateii = false;
        for(int ii1=0; ii1<m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii1++){
          BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if(tempS.msgId == m_PendResp.msgId && tempS.addr == m_PendResp.addr) {
            cout<<"FCFS Stage is changed for "<<tempS.msgId<<" to Response "<<endl;
            tempS.currStage = "RESPONSE";
            if (m_reza_log) cout<<"In MSI_FcFsRespBus 3"<<endl;
            terminateii = true;
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
          else{
            m_GlobalQueue->m_MsgType.InsertElement(tempS);
          }
        }
      } 
      if (m_reza_log) cout<<"In MSI_FcFsRespBus 4"<<endl;
      m_PndResp = CheckPendingFCFSResp(m_PendResp, false);
      m_PndResp = false;











        if (m_PendResp.msgId != 0)
        {
          if (m_reza_log) cout<<"In MSI_FcFsRespBus 6"<<endl;
          unsigned int mask = createMask(6, 6);
          unsigned bank_num = mask & m_PendResp.addr;
          bank_num = bank_num >> 6;
          m_PendResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

          // m_PendResp.sharedCacheAgent = 10;

          if (isFinal(m_PendResp.addr, m_PendResp.msgId, m_PendResp.sharedCacheAgent))
          {
            if (m_reza_log)
              cout << "It is Final 1" << endl;
            if (isOldest(m_PendResp.addr, m_PendResp.reqCoreId))
            {
              // 0- check first if it is the oldest of its requestor
              if (m_reza_log)
                cout << "It is oldest the order now is " << m_GlobalQueue->m_GlobalRRQueue.at(0) << endl;
              // 1- If YES, adjust the order
              for (unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size(); h++)
              {
                if (m_GlobalQueue->m_GlobalRRQueue.at(h) == m_PendResp.reqCoreId)
                {
                  m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                }
              }
              //if(m_GlobalQueue->m_GlobalRRQueue.size() >0) cout<<"Order is adjusted and now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<"  the oldest queue size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<endl;
              // 2- If YES, remove this oldest request from oldest queue

              if (m_reza_log)
                cout << " the oldest queue size " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
              removeFromOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              if (m_reza_log)
                cout << " the oldest queue size " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
              removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);   

              if (m_reza_log)
                cout << " the non oldest queue size " << m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() << endl;
              removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              if (m_reza_log)
                cout << " the non oldest queue size after removing " << m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() << endl;
              if (m_reza_log)
                cout << " it should be removed from oldest queue and now the size is  " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
              // 3- adjust the oldest request
              adjustOldest(m_PendResp.reqCoreId);
              if (m_reza_log)
                cout << " the oldest shold be inserted if exist and now the size is  " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << " the Order queue size is " << m_GlobalQueue->m_GlobalRRQueue.size() << endl;
            }
            else
            {
              //cout<<" the request is non oldest so remove from the non oldest queue the size is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
              removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false); 
              //cout<<" the request is non oldest so remove from the non oldest queue the size after removing is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
            }
          }
          else
          {
            cout << "ERROR: how is it possible to have a response from shared memory but it is not the final response? Check the trace and extract the scenario" << endl;
            abort();
          }
        }
        // else
        // {
        //   cout << "ERROR: how is it possible to have a replacement response from shared memory? Check the trace and extract the scenario" << endl;
        //   abort();
        // }
        /***********************************  function to adjust the oldest request **********************************************/
      }      
 

    m_PndResp = CheckPendingFCFSResp(m_PendResp, true);
    if (m_PndResp)
    {
      if (m_reza_log)
        cout << "|||||||||||||||||||||||||||| HP RESP SCHEDULED TO SENT IN NEXT DELTA ||||||||||||||||||||||||||||||| "<< m_PendResp.msgId << endl;
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
    // insert message on the request bus
    // it takes the oldest no matter which core
    // it does not have any TDM slot so not advancing
    // if (m_reza_log)
    //   cout << "-------------------------------REQ BUS ARBITER FCFS ----------------------------------" << endl;

    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
    //     tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
    //     m_GlobalQueue->m_GlobalOldestQueue.PopElement();
    //     cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
    //   }
    // }
    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
    //     tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
    //     m_GlobalQueue->m_GlobalReqFIFO.PopElement();
    //     cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
    //   }
    // }

    // for (unsigned int i = 0; i < m_GlobalQueue->m_GlobalRRQueue.size(); i++)
    // {
    //   if (m_reza_log)
    //     cout << " Order is: " << m_GlobalQueue->m_GlobalRRQueue.at(i) << endl;
    // }
    
    

    if (m_PndReq)
    {
      m_PndReq = CheckPendingFCFSReq(m_ReqBusMsg, false);
      InsertOnReqBus(m_ReqBusMsg);
      m_PndReq = false;

      if (m_ReqBusMsg.NoGetMResp == false)
      { /// what is NoGetMResp?
        // cout<<"NoGetMResp"<<endl;
        // abort();
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

  bool BusArbiter::isHigherPrio(unsigned int l, unsigned int h) {    
    for(unsigned int jj = 0; jj < m_GlobalQueue->m_GlobalRRQueue.size(); jj++){
      if (m_GlobalQueue->m_GlobalRRQueue.size() != 0) {
        m_respCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(jj);
        if(l == m_respCoreCnt)
          return true;        
        else if (h == m_respCoreCnt)
          return false;
      }
    }
    return false;
  }

  void BusArbiter::deadlineProgress() {    
    if (m_reza_log) cout<<"deadline progress"<<endl;
    bool deadlineProgress_terminate = false;
    BusIfFIFO::BusReqMsg deadlineProgress_temp;
    BusIfFIFO::BusReqMsg deadlineProgress_temp_type;
    if(!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty()){       
      for (int it11 = 0; it11 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it11++) {
        deadlineProgress_terminate = false;
        deadlineProgress_temp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();        
        for(int it12 = 0; it12 < m_GlobalQueue->m_MsgType.GetQueueSize() && deadlineProgress_terminate == false; it12++) {
          deadlineProgress_temp_type = m_GlobalQueue->m_MsgType.GetFrontElement(); 
          if(deadlineProgress_temp.msgId == deadlineProgress_temp_type.msgId && deadlineProgress_temp.addr == deadlineProgress_temp_type.addr) {
            deadlineProgress_terminate = true;
            m_GlobalQueue->m_MsgType.PopElement();
            if(deadlineProgress_temp_type.associateDeadline > 0) deadlineProgress_temp_type.associateDeadline--;
            if(deadlineProgress_temp.msgId == 0)
              cout<<"Deadline of "<<deadlineProgress_temp_type.wbCoreId<<" msgID "<<deadlineProgress_temp_type.msgId<<" is "<<deadlineProgress_temp_type.associateDeadline<<endl;
            else 
              cout<<"Deadline of "<<deadlineProgress_temp_type.reqCoreId<<" msgID "<<deadlineProgress_temp_type.msgId<<" is "<<deadlineProgress_temp_type.associateDeadline<<endl;  
            m_GlobalQueue->m_MsgType.InsertElement(deadlineProgress_temp_type);
          }
          else{
            m_GlobalQueue->m_MsgType.PopElement();
            m_GlobalQueue->m_MsgType.InsertElement(deadlineProgress_temp_type);
          }
        }
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(deadlineProgress_temp);
      }
    }    
  }

  bool BusArbiter::WCLator() {
    if (m_reza_log) cout<<"------------------------------------------------------WCLator in Arbiter------------------------------------------------------"<<endl;

    if (m_reza_log && m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() >0)
     cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following WCLATOR  ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    if (m_reza_log)
    {
      for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
      {
        BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
        tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        cout << "AT " << u << " Oldest ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
      }
    }

    
    if (m_reza_log)
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    if (m_reza_log)
    {
      for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
      {
        BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
        tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
        m_GlobalQueue->m_GlobalReqFIFO.PopElement();
        cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << 
            tempNonOldestReqMsgTemp.sharedCacheAgent << " Cache line " <<retrieveSharedBankCacheLine(tempNonOldestReqMsgTemp.addr) <<endl;
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
      }
    }
    
    if (m_reza_log && m_GlobalQueue->m_MsgType.GetQueueSize() > 0 )
    {
      cout<<"*********************** Content of the M_Type******************* Begning of WCLATOR  the size is "<<m_GlobalQueue->m_MsgType.GetQueueSize()<<endl;  
      for (int u = 0; u < m_GlobalQueue->m_MsgType.GetQueueSize(); u++)
      {
        BusIfFIFO::BusReqMsg tempOldestReqMsgTemp_mtype;
        tempOldestReqMsgTemp_mtype = m_GlobalQueue->m_MsgType.GetFrontElement();
        m_GlobalQueue->m_MsgType.PopElement();
        cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp_mtype.addr << "  ReqID  " << tempOldestReqMsgTemp_mtype.reqCoreId << "  wbID  " << tempOldestReqMsgTemp_mtype.wbCoreId << "  MSGID " << tempOldestReqMsgTemp_mtype.msgId << "  AGENT  " <<
             tempOldestReqMsgTemp_mtype.sharedCacheAgent << " curr stage "<< tempOldestReqMsgTemp_mtype.currStage<< " Cache line " <<retrieveSharedBankCacheLine(tempOldestReqMsgTemp_mtype.addr) <<endl;
        m_GlobalQueue->m_MsgType.InsertElement(tempOldestReqMsgTemp_mtype);
      }
      cout<<"*************************************************************"<<endl; 
    }

    for (unsigned int i = 0; i < m_GlobalQueue->m_GlobalRRQueue.size(); i++)
    {
      if (m_reza_log)
        cout << " Order is: " << m_GlobalQueue->m_GlobalRRQueue.at(i) << endl;
    }
    


    unsigned int latency, addedLatency, WCLatorReqID, currentOrder;
    unsigned int B_1, B_2, B_3;
    unsigned int k_a, k_b, k_c, k_d, k_e;    

    B_1 = m_reqclks - 1;
    B_2 = m_sharedbankclks - 1;
    B_3 = m_respclks - 1;

    ns3::GenericFIFO<ns3::BusIfFIFO::BusReqMsg> tempFromOldest;
    tempFromOldest = m_GlobalQueue->m_GlobalOldestQueue;
    unsigned int deadline;
    if(!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty()){    
      for (int it5 = 0; it5 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it5++) {      
        currentOrder = 100;
        WCLatorReqID = 100;
        deadline = 100;
        k_a = 0;
        k_b = 0;
        k_c = 0;
        k_d = 0;
        k_e = 0;
        
        // Retrieve the reqID
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator;                   // the transaction that we are calculating the latency for
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType;       // the transaction that we are calculating the latency for but we got from MemType
        tempOldestMsgQueueWCLator_FromMemType.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType.associateDeadline_final = false;
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType_1;     // Just to handle the orders
        tempOldestMsgQueueWCLator_FromMemType_1.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType_1.associateDeadline_final = false;
        BusIfFIFO::BusReqMsg tempOldestMsgQueueWCLator_FromMemType_local; // the transaction from oldest queue to check against the tua
        tempOldestMsgQueueWCLator_FromMemType_local.orderDetermined = false;
        tempOldestMsgQueueWCLator_FromMemType_local.associateDeadline_final = false;

        tempOldestMsgQueueWCLator = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        //cout<<"tempOldestMsgQueueWCLator id is "<<tempOldestMsgQueueWCLator.msgId<<endl;
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();

        WCLatorReqID = tempOldestMsgQueueWCLator.msgId == 0 ? tempOldestMsgQueueWCLator.wbCoreId : tempOldestMsgQueueWCLator.reqCoreId;        

        bool RR_Con = false;
        // Retrieve the RR Order
        for (unsigned int it6 = 0; it6 < m_GlobalQueue->m_GlobalRRQueue.size() && RR_Con == false; it6++) {
          if(WCLatorReqID == m_GlobalQueue->m_GlobalRRQueue.at(it6)) {
            currentOrder = it6;
            RR_Con = true;
          }
        }
        
        
        // Retrieve the Deadline
        bool terminatei = false;        
        for(int ii1=0; ii1<m_GlobalQueue->m_MsgType.GetQueueSize() && terminatei == false; ii1++) {  
          
          //cout<<"tempOldestMsgQueueWCLator "<<tempOldestMsgQueueWCLator.msgId<<" adr "<<tempOldestMsgQueueWCLator.addr<<endl;
          //cout<<"m_GlobalQueue->m_MsgType.GetFrontElement().msgId "<<m_GlobalQueue->m_MsgType.GetFrontElement().msgId<<" adr "<<m_GlobalQueue->m_MsgType.GetFrontElement().addr<<endl;
          if(m_GlobalQueue->m_MsgType.GetFrontElement().msgId == tempOldestMsgQueueWCLator.msgId && m_GlobalQueue->m_MsgType.GetFrontElement().addr == tempOldestMsgQueueWCLator.addr
            && m_GlobalQueue->m_MsgType.GetFrontElement().reqCoreId == tempOldestMsgQueueWCLator.reqCoreId) {   
            tempOldestMsgQueueWCLator_FromMemType = m_GlobalQueue->m_MsgType.GetFrontElement(); 
            m_GlobalQueue->m_MsgType.PopElement();           
            deadline = tempOldestMsgQueueWCLator_FromMemType.associateDeadline;                      
            terminatei = true;
            m_GlobalQueue->m_MsgType.InsertElement(tempOldestMsgQueueWCLator_FromMemType);
          }
          else{          
            tempOldestMsgQueueWCLator_FromMemType_1 = m_GlobalQueue->m_MsgType.GetFrontElement(); 
            m_GlobalQueue->m_MsgType.PopElement();    
            m_GlobalQueue->m_MsgType.InsertElement(tempOldestMsgQueueWCLator_FromMemType_1);
          }
        }


       
      
        cout<<"For ReqID "<<WCLatorReqID<<" and msgID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<" the RR order is "<<currentOrder<<" order of arbitration "<<tempOldestMsgQueueWCLator_FromMemType.orderofArbitration<<" deadline "<<deadline<<endl;
        
        // Extract the k_a, k_b, k_c, k_d, and k_e
        unsigned tempOrderFront;
        unsigned int GlobalQueueSize = m_GlobalQueue->m_GlobalRRQueue.size();
        for (unsigned int it8 = 0; it8 < GlobalQueueSize; it8++) {
          if(currentOrder > it8){    
                
            //cout<<"1 WCLator in Arbiter Done with Ka "<<k_a<<" Kb "<<k_b<<" Kc "<<k_c<<" Kd "<<k_d<<" Ke "<<k_e<<endl;
            tempOrderFront = m_GlobalQueue->m_GlobalRRQueue.at(it8);
           
            bool nextInMemType = false;
            bool notDetermined = false;

         
            //cout<<"size of global "<<tempFromOldest.GetQueueSize()<<endl;
            for (int it9 = 0; it9 < tempFromOldest.GetQueueSize() && nextInMemType == false; it9++) {
              tempOldestMsgQueueWCLator_FromMemType_local = tempFromOldest.GetFrontElement();
              //cout<<"in the inner loopp "<<tempOldestMsgQueueWCLator_FromMemType_local.msgId<<" popped "<<endl;
              tempFromOldest.PopElement();              
              if(((tempOldestMsgQueueWCLator_FromMemType_local.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType_local.wbCoreId == tempOrderFront) 
                || (tempOldestMsgQueueWCLator_FromMemType_local.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType_local.reqCoreId == tempOrderFront)) ) {                  
                tempFromOldest.InsertElement(tempOldestMsgQueueWCLator_FromMemType_local);         
                //cout<<"in the inner loopp "<<tempOldestMsgQueueWCLator_FromMemType_local.msgId<<" added back "<<endl;         
                for(int ii2=0; ii2<m_GlobalQueue->m_MsgType.GetQueueSize() && nextInMemType == false; ii2++) {               
                  BusIfFIFO::BusReqMsg tempMemType;
                  tempMemType = m_GlobalQueue->m_MsgType.GetFrontElement();
                  m_GlobalQueue->m_MsgType.PopElement();
                  if(tempOrderFront == WCLatorReqID) notDetermined = true;
                  // cout<<"found in the memtype  from the oldest order of arbitration "<<tempOldestMsgQueueWCLator_FromMemType.orderofArbitration<<" bank agent "<<tempOldestMsgQueueWCLator_FromMemType_local.sharedCacheAgent<<"  id is "<<tempOldestMsgQueueWCLator_FromMemType_local.msgId<<endl;
                  // cout<<"found in the memtype  from the in front of order of arbitration "<<tempMemType.orderofArbitration<<" bank agent "<<tempMemType.sharedCacheAgent<<"  id is "<<tempMemType.msgId<<endl;                     
                  if(tempMemType.msgId == tempOldestMsgQueueWCLator_FromMemType_local.msgId && tempMemType.addr == tempOldestMsgQueueWCLator_FromMemType_local.addr && tempOrderFront != WCLatorReqID) {
             
                    notDetermined = true;
                    if(tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == tempMemType.orderofArbitration && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent){
                      // Extract the k_a
                      
                      //abort();
                      k_a++;
                       //cout<<"k_a incremented "<<k_a<<endl;
                      nextInMemType = true;
                    }
                    else if (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == tempMemType.orderofArbitration && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent) {
                      // Extract the k_b
                      //cout<<"Check the sharedcacheagent is known for both of these temporary request "<<tempMemType.orderofArbitration<<endl;
                      //abort();
                      k_b++;
                      //cout<<"k_b incremented "<<k_b<<endl;
                      nextInMemType = true;
                    }   
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 1 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent)
                          || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 1 && tempMemType.orderofArbitration == 0 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent == tempMemType.sharedCacheAgent)) {
                      // Extract the k_c
                      //cout<<"Check the sharedcacheagent is known for both of these temporary request "<<tempMemType.orderofArbitration<<endl;
                      //abort();
                      k_c++;
                      //cout<<"k_c incremented "<<k_c<<endl;
                      nextInMemType = true;
                    }  
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 1 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent)
                          || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 1 && tempMemType.orderofArbitration == 0 && tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != tempMemType.sharedCacheAgent)) {
                      // Extract the k_d      
                      //cout<<"Check the sharedcacheagent is known for both of these temporary request "<<tempMemType.orderofArbitration<<endl;
                      //abort();
                      k_d++;
                      //cout<<"k_d incremented "<<k_d<<endl;
                      nextInMemType = true;
                    }  
                    else if ((tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 0 && tempMemType.orderofArbitration == 3) || (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration == 3 && tempMemType.orderofArbitration == 0)){
                      // Extract the k_e
                      //cout<<"Check the sharedcacheagent is known for both of these temporary request  "<<tempMemType.orderofArbitration<<endl;
                      //abort();
                      k_e++;
                      //cout<<"k_e incremented "<<k_e<<endl;
                      nextInMemType = true;
                    }
                    //cout<<"2- WCLator in Arbiter Done with Ka "<<k_a<<" Kb "<<k_b<<" Kc "<<k_c<<" Kd "<<k_d<<" Ke "<<k_e<<endl;                
                    m_GlobalQueue->m_MsgType.InsertElement(tempMemType);
                  }
                  else{
                    m_GlobalQueue->m_MsgType.InsertElement(tempMemType);
                  }
                  /** here it means that we went over the queue of MemType but we could not find the oldest request meaning that it is not determined yet. 
                   * hence, we need to consider the worst possible case until it can be determined **/                  
                  if((ii2 == (m_GlobalQueue->m_MsgType.GetQueueSize()-1)) && notDetermined == false) {
                    //cout<<"the request oldest id "<<tempOldestMsgQueueWCLator_FromMemType_local.msgId<<" is not determined yet hence adding one to kc to resemble the worst case "<<endl;
                    k_c++;
                  }
                }
              }
              else{
                //cout<<"in the inner loopp "<<tempOldestMsgQueueWCLator_FromMemType_local.msgId<<" added back "<<endl;
                tempFromOldest.InsertElement(tempOldestMsgQueueWCLator_FromMemType_local);
              }
            }
          }
          //cout<<"WCLator in Arbiter Done with Ka "<<k_a<<" Kb "<<k_b<<" Kc "<<k_c<<" Kd "<<k_d<<" Ke "<<k_e<<endl;
        }
        
       

        cout<<"Final WCLator in Arbiter Done with Ka "<<k_a<<" Kb "<<k_b<<" Kc "<<k_c<<" Kd "<<k_d<<" Ke "<<k_e<<endl;
        
        // check if HP can do something that adds to the latency - This can be any oldest that is lower priority or any non oldest from anyone
        addedLatency = 0;
        if(tempOldestMsgQueueWCLator_FromMemType.orderDetermined){
          switch (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration)
          {          
          case 0:
            /* REQ:BANK:RESPONSE */            
            if(tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") {            
              // check if there is anything that HPA can process on same BANK from RxResp from lower priority      
              std::list<Ptr<BusIfFIFO>>::iterator itInvert = m_sharedCacheBusIfFIFO.begin();
              std::advance(itInvert, retrieveSharedCacheID(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent));            
              // check if there is anything that HPA can process on same BANK from RxResp from lower priority   
            
              if (((*itInvert)->m_rxRespFIFO.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 1 ))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg RxResp_HP;
                RxResp_HP = (*itInvert)->m_rxRespFIFO.GetFrontElement();
                if (RxResp_HP.msgId != 0 && isOldest(RxResp_HP.addr,RxResp_HP.reqCoreId)) {
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.reqCoreId) == true) ||
                    (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.reqCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else if(RxResp_HP.msgId == 0 && isOldest(RxResp_HP.addr,RxResp_HP.respCoreId)){
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.respCoreId) == true) ||
                    (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.respCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else {
                  addedLatency =  m_sharedbankclks;
                }                  
              }                
              
              // check if there is anything that HPA can process on same BANK from m_localPendingRespTxBuffer from lower priority
              
              if (((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 1 ))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg Txlocal_HP;
                Txlocal_HP = (*itInvert)->m_localPendingRespTxBuffer.GetFrontElement();
                if (Txlocal_HP.msgId != 0 && isOldest(Txlocal_HP.addr,Txlocal_HP.reqCoreId)) {
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.reqCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else if(Txlocal_HP.msgId == 0 && isOldest(Txlocal_HP.addr,Txlocal_HP.respCoreId)){
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.respCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else {
                  addedLatency =  m_sharedbankclks;
                }                  
              }
                
                
            }            
            else if((tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK") && (m_GlobalQueue->m_respArbBlock == 1)){
              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true)){
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true)){
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
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
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
                  }                  
                }                
              } 
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              // no effect?
            }
            break;
          case 1:
            /* REQ:RESPONSE:BANK */
            if((tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") && (m_GlobalQueue->m_respArbBlock == 1)) {
              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true))  {
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true))  {
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
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
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
                  }                  
                }                
              }
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              std::list<Ptr<BusIfFIFO>>::iterator itInvert = m_sharedCacheBusIfFIFO.begin();
              std::advance(itInvert, retrieveSharedCacheID(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent));            
              // check if there is anything that HPA can process on same BANK from RxResp from lower priority   

              if ((*itInvert)->m_rxRespFIFO.IsEmpty() == false && ((*itInvert)->m_bankArbBlock == 1 ))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg RxResp_HP;
                RxResp_HP = (*itInvert)->m_rxRespFIFO.GetFrontElement();
                if (RxResp_HP.msgId != 0 && isOldest(RxResp_HP.addr,RxResp_HP.reqCoreId)) {
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.reqCoreId) == true)) {
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else if(RxResp_HP.msgId == 0 && isOldest(RxResp_HP.addr,RxResp_HP.respCoreId)){
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, RxResp_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 &&  isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, RxResp_HP.respCoreId) == true)) {
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else {
                  addedLatency =  m_sharedbankclks;
                }                  
              }
                
               
              // check if there is anything that HPA can process on same BANK from m_localPendingRespTxBuffer from lower priority
              
              if (((*itInvert)->m_localPendingRespTxBuffer.IsEmpty() == false) && ((*itInvert)->m_bankArbBlock == 1 ))
              {
                // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                ns3::BusIfFIFO::BusRespMsg Txlocal_HP;
                Txlocal_HP = (*itInvert)->m_localPendingRespTxBuffer.GetFrontElement();
                if (Txlocal_HP.msgId != 0 && isOldest(Txlocal_HP.addr,Txlocal_HP.reqCoreId)) {
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.reqCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.reqCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else if(Txlocal_HP.msgId == 0 && isOldest(Txlocal_HP.addr,Txlocal_HP.respCoreId)){
                  if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, Txlocal_HP.respCoreId) == true) ||
                      (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, Txlocal_HP.respCoreId) == true)){
                    addedLatency =  m_sharedbankclks;
                  }
                }
                else {
                  addedLatency =  m_sharedbankclks;
                }                  
              }                
              
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK") {
              // no effect?
            }
            
            break;
          case 2:
            /* REQ:RESP */
            if((tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") && (m_GlobalQueue->m_respArbBlock == 1)) {
              // check if there is anything that HPA can send on RESPONSE bus from lower priority cores
              for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
              {
                if ((*it13)->m_txRespFIFO.IsEmpty() == false)
                {
                  // if there is any response from lower priority oldest Or from any priority non oldest, we need to add to the latency
                  ns3::BusIfFIFO::BusRespMsg txResp_HP;
                  txResp_HP = (*it13)->m_txRespFIFO.GetFrontElement();
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true)){
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true)){
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
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
                  if (txResp_HP.msgId != 0 && isOldest(txResp_HP.addr,txResp_HP.reqCoreId)) {
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.reqCoreId) == true) ||          
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.reqCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else if(txResp_HP.msgId == 0 && isOldest(txResp_HP.addr,txResp_HP.respCoreId)){
                    if((tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.wbCoreId, txResp_HP.respCoreId) == true) ||          
                       (tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && isHigherPrio(tempOldestMsgQueueWCLator_FromMemType.reqCoreId, txResp_HP.respCoreId) == true)) {
                      addedLatency =  m_respclks;
                    }
                  }
                  else{
                    addedLatency =  m_respclks;
                  }                  
                }                
              }
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              // no effect?
            }
            
            break;  
          default:
            break;
          }
        }
        else {
          // it is waiting for REQ
          if(m_GlobalQueue->m_busArbBlock == 1) 
          { 
            for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_busIfFIFO.begin(); it13 != m_busIfFIFO.end(); it13++)
            {
              if ((*it13)->m_txMsgFIFO.IsEmpty() == false)
              {
                ns3::BusIfFIFO::BusReqMsg txMsg_HP;
                txMsg_HP = (*it13)->m_txMsgFIFO.GetFrontElement();
                if(tempOldestMsgQueueWCLator_FromMemType.sharedCacheAgent != txMsg_HP.sharedCacheAgent) {
                  addedLatency = m_reqclks;
                }
                else{
                  // check if this is from the same cache line  
                  if(retrieveSharedBankCacheLine(tempOldestMsgQueueWCLator_FromMemType.addr) == retrieveSharedBankCacheLine(txMsg_HP.addr)) {
                    k_a++;
                    if(!m_GlobalQueue->m_GlobalOldestQueue.IsEmpty()){    
                      BusIfFIFO::BusReqMsg PrioInversion;
                      int size_glob = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
                      for (int it14 = 0; it14 < size_glob; it14++) {
                        PrioInversion = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
                        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
                        if((PrioInversion.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && !isHigherPrio(PrioInversion.wbCoreId,tempOldestMsgQueueWCLator_FromMemType.wbCoreId))   ||
                          (PrioInversion.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && !isHigherPrio(PrioInversion.reqCoreId,tempOldestMsgQueueWCLator_FromMemType.reqCoreId)) ||
                          (PrioInversion.msgId == 0 && tempOldestMsgQueueWCLator_FromMemType.msgId != 0 && !isHigherPrio(PrioInversion.wbCoreId,tempOldestMsgQueueWCLator_FromMemType.reqCoreId))  ||
                          (PrioInversion.msgId != 0 && tempOldestMsgQueueWCLator_FromMemType.msgId == 0 && !isHigherPrio(PrioInversion.reqCoreId,tempOldestMsgQueueWCLator_FromMemType.wbCoreId))) {
                            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(PrioInversion);
                        
                            k_a++;
                          }
                          else{
                            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(PrioInversion);
                
                          }      
                      }
                    }
                  }
                  else {
                    addedLatency = m_reqclks;
                  }
                }              
              }            
            } 
          }          
        }
        
        cout<<"After retaliation "<<k_a<<" added latency "<<addedLatency<<endl;
        latency = 0;
        // Estimate the latency           
        if((tempOldestMsgQueueWCLator_FromMemType.orderDetermined == true) && (tempOldestMsgQueueWCLator_FromMemType.associateDeadline_final==true)){
          //cout<<"4"<<endl;
          switch (tempOldestMsgQueueWCLator_FromMemType.orderofArbitration)
          {            
          case 0:
            /* REQ:BANK:RESPONSE */            
            if(tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") {
              // The request finished proceed REQ stage
              cout<<"Latency Est. 1 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              latency = addedLatency + B_2 + B_3 + m_sharedbankclks + m_respclks + (k_a) * max(m_sharedbankclks,m_respclks) 
                  + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks,m_respclks); 
                  
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK"){
              cout<<"Latency Est. 2 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished its second stage
              latency = addedLatency + B_3 + m_respclks + (k_a) * m_respclks 
                  + (k_b + k_d + k_e) * (m_respclks) + k_c * m_respclks; 
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              cout<<"Latency Est. 3 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else {    
              cout<<"the current is "<<tempOldestMsgQueueWCLator_FromMemType.currStage<<endl;          
              cout<<"Latency Est. 4 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;              
              abort();
              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + ((k_a) * max(m_reqclks,max(m_sharedbankclks,m_respclks))) 
                  + ((k_b + k_d + k_e) * (m_reqclks + m_respclks)) + (k_c * max(m_sharedbankclks,m_respclks)); 
            }
            break;
          case 1:
            /* REQ:RESPONSE:BANK */
            if(tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") {
              cout<<"Latency Est. 5 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished proceed REQ stage
              latency = addedLatency +  B_2 + B_3 + m_sharedbankclks + m_respclks + (k_a) * max(m_sharedbankclks,m_respclks) 
                  + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks,m_respclks); 
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              cout<<"Latency Est. 6 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished its second last stage
              latency = addedLatency + B_2 + m_sharedbankclks + (k_a) * m_sharedbankclks 
                   + k_c * m_sharedbankclks; 
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "BANK"){
              cout<<"Latency Est. 7 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else {
              cout<<"Latency Est. 8 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              abort();
              // The request did not proceed into any stages
              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a) * max(m_reqclks,max(m_sharedbankclks,m_respclks))
                  + (k_b + k_d + k_e) * (m_reqclks + m_respclks) + k_c * max(m_sharedbankclks,m_respclks); 
            }
            break;
          case 2:
            /* REQ:RESP */
            if(tempOldestMsgQueueWCLator_FromMemType.currStage == "REQ") {
              cout<<"Latency Est. 9 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;;
              // The request finished proceed REQ stage
              latency = addedLatency + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a) * max(m_sharedbankclks,m_respclks) 
                  + (k_b + k_d + k_e) * (m_respclks) + k_c * max(m_sharedbankclks,m_respclks);
            }
            else if(tempOldestMsgQueueWCLator_FromMemType.currStage == "RESPONSE"){
              cout<<"Latency Est. 10 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              // The request finished its last stage
              latency = addedLatency + 1;
            }
            else{
              cout<<"Latency Est. 11 for ID "<<tempOldestMsgQueueWCLator_FromMemType.msgId<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
              abort();
              // The request did not proceed into any stages
              latency = addedLatency + B_1 + B_2 + B_3 + m_reqclks + m_sharedbankclks + m_respclks + (k_a) * max(m_reqclks,max(m_sharedbankclks,m_respclks))
                  + (k_b + k_d + k_e) * (m_reqclks + m_respclks) + k_c * max(m_sharedbankclks,m_respclks); 

            }
            break;  
          default:
            break;
          }
        }
        else {
          //cout<<"6"<<endl;
          cout<<"order is not determined at all. let s give it a hard time"<<endl;
          latency  = addedLatency + 0 ;
        }

        cout<<"############### "<<WCLatorReqID<<"  WCLator in Arbiter Done with Latency "<<latency<<" and the assosicate deadline is "<<tempOldestMsgQueueWCLator_FromMemType.associateDeadline<<endl;
        //cout<<"to be added is "<<tempOldestMsgQueueWCLator.msgId<<" adress "<<tempOldestMsgQueueWCLator.addr<<"  Addr  "<<tempOldestMsgQueueWCLator_FromMemType.addr<<endl;
        // Determination
        if(latency >= tempOldestMsgQueueWCLator_FromMemType.associateDeadline){
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
          //cout<<"HERE SOMETHING INSERTED"<<endl;
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
  { // agent is destination that data needs to be sent to and it could be: core, interconnect, shared mem
    // send data to core

    if (agent == AGENT::CORE)
    {
      for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++)
      {
        if (msg.reqCoreId == (*it2)->m_fifo_id)
        {
          if (!(*it2)->m_rxRespFIFO.IsFull())
          {
            if(msg.respCoreId < 10)
              m_GlobalQueue->m_Core2CoreTransfer++;
            else
              m_GlobalQueue->m_SharedBankTransfer++;  
              
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

      unsigned int mask = createMask(6, 6);
      unsigned bank_num = mask & msg.addr;
      bank_num = bank_num >> 6;
      msg.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

      //  msg.sharedCacheAgent = 10;

      if (m_reza_log)
      {
        cout << " in the send data when agent is sharedmem address is  " << msg.addr << "  and the agent is  " << msg.sharedCacheAgent << endl;
        cout << "reqCoreID " << msg.reqCoreId << "  respcoreID " << msg.respCoreId << endl;
      }
      std::list<Ptr<BusIfFIFO>>::iterator it3 = m_sharedCacheBusIfFIFO.begin();
      std::advance(it3, retrieveSharedCacheID(msg.sharedCacheAgent));

      if (!(*it3)->m_rxRespFIFO.IsFull())
      {
        (*it3)->m_rxRespFIFO.InsertElement(msg);
        if (m_reza_log)
        {
          cout << "size of the resp of shred cache is " << (*it3)->m_rxRespFIFO.GetQueueSize() << endl;
        }
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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//   where is this interconnect conceptually?
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
      for (int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it2++) {
        BusIfFIFO::BusReqMsg txMsgTemp;
        txMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        if ((txMsgTemp.msgId == txMsg.msgId) && (txMsgTemp.addr == txMsg.addr)) {
          cout<<"the become oldes time from the oldest queue is "<<txMsgTemp.becameOldest<<"  for msgID "<<txMsgTemp.msgId<<endl;
          txMsg.becameOldest = txMsgTemp.becameOldest;
        }
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(txMsgTemp);
      }

      // Remove message from the busResp buffer
      if (ChkOnly == true) cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ REQUEST SCHEDULED TO SEND IN NEXT INTERVAL FROM REQ Arbiter FCFS - MSGID  "<<txMsg.msgId <<"  ADDRESS "<<txMsg.addr<<endl;
      if (ChkOnly == false)
      {
        cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ REQUEST BROADCASTED FROM REQ Arbiter FCFS - MSGID  "<<txMsg.msgId<<"  ADDRESS "<<txMsg.addr<<endl;
        (*it2)->m_txMsgFIFO.PopElement();
      }
    }     
    return PendingTxReq;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//

  /*** For the FCFS: we will use MSI_FcFsReqBus2 ();  as the request bus arbiter and MSI_FcFsRespBus(); as the response bus arbiter ***/

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
    mode = 0;
    bool PendingTxResp = false;
    bool PendingTxWBResp = false;
    unsigned int sharedCacheID;
    unsigned int GlobalQueueSize;
    BusIfFIFO::BusRespMsg pendingWbMsg_1;
    BusIfFIFO::BusRespMsg pendingWbMsg_2;

    /***   if in global oldest there is a msg whcih is not PutM that and its reqcoreID 
     *** equals to RR order requestor then go to the resp buffer and find it   ***/
    /*** if not go check the non oldest **/
    // if (m_reza_log)
    //   cout << " ----------------------------------In checkPendingResp-------------oldest:  " << oldest << endl;

    for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && PendingTxResp == false; RR_iterator++)
    {
      if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
      {
        m_respCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
      }

      // if (m_reza_log)
      //   cout << "check the order " << m_respCoreCnt << endl;

      // check the oldest global queue
      if (oldest)
        GlobalQueueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
      else
        GlobalQueueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();

      for (unsigned int itr = 0; itr < GlobalQueueSize && PendingTxResp == false; itr++)
      {
        //if(m_reza_log)  cout<<"in loop in the global queue  "<<GlobalQueueSize<<endl;
        if (oldest)
        {
          m_ServQueueMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
          m_GlobalQueue->m_GlobalOldestQueue.PopElement();
          // if (m_reza_log)
          //   cout << "oldest  " << m_ServQueueMsg.addr << endl;
        }
        else
        {
          m_ServQueueMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
          m_GlobalQueue->m_GlobalReqFIFO.PopElement();
          // if (m_reza_log)
          //   cout << "Non oldest  " << m_ServQueueMsg.addr << endl;
        }
        //cout<<"m_ServQueueMsg.reqCoreId "<<m_ServQueueMsg.reqCoreId<<"  m_respcount "<<m_respCoreCnt<<endl;
        //if(m_ServQueueMsg.cohrMsgId != SNOOPPrivCohTrans::PutMTrans && m_ServQueueMsg.reqCoreId == m_respCoreCnt) {
        if (m_ServQueueMsg.reqCoreId == m_respCoreCnt)
        {

          std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin();
          // if (m_reza_log)
          //   cout << " the request address is  " << m_ServQueueMsg.addr << "  and timestamp " << m_ServQueueMsg.timestamp << endl;
          sharedCacheID = retrieveSharedCacheID(m_ServQueueMsg.sharedCacheAgent);
          // if (m_reza_log)
          //   cout << "shared cache ID " << sharedCacheID << endl;
          std::advance(it1, sharedCacheID);
          // if (m_reza_log)
          //   cout << " advance shared cache ID " << (*it1)->m_fifo_id << endl;
          int pendingQueueSize = (*it1)->m_txRespFIFO.GetQueueSize();
          // if (m_reza_log)
          //   cout << "check the TX Response of the shared cache the size is " << pendingQueueSize << endl;
          for (int i = 0; i < pendingQueueSize && PendingTxResp == false; i++)
          {

            pendingWbMsg_1 = (*it1)->m_txRespFIFO.GetFrontElement();
            (*it1)->m_txRespFIFO.PopElement();
            // if (m_reza_log)
            //   cout << "------------In TX RESP of shared Cache------------m_respCoreCnt " << m_respCoreCnt << " address  " << m_ServQueueMsg.addr << endl;
            // if (m_reza_log)
            //   cout << " REQ ID " << pendingWbMsg_1.reqCoreId << "  addr  " << pendingWbMsg_1.addr << "   RESP ID " << pendingWbMsg_1.respCoreId << endl;
            if (pendingWbMsg_1.reqCoreId == m_respCoreCnt && (pendingWbMsg_1.addr == m_ServQueueMsg.addr))
            {
              // if (m_reza_log)
              //   cout << "pendingWbMsg.reqCoreId == m_respCoreCnt && (pendingWbMsg.addr == m_ServQueueMsg.addr" << endl;
              if (ChkOnly == true)
              {
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
        }
        if (oldest)
          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(m_ServQueueMsg);
        else
          m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ServQueueMsg);
      }
    }

    














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

      // if (m_reza_log)
      // {
      //   if (oldest)
      //     cout << "In CheckPendingWriteBackResp-----------The Order is " << m_respCoreCnt << "  The size of the global oldest     queue is " << GlobalQueueSize << endl;
      //   else
      //     cout << "In CheckPendingWriteBackResp-----------The Order is " << m_respCoreCnt << "  The size of the global non-oldest queue is " << GlobalQueueSize << endl;
      // }

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
        //cout<<"In the Global queue the order is "<<m_respCoreCnt<<"  reqID "<<m_ServQueueMsg.reqCoreId<<"  wbID "<<m_ServQueueMsg.wbCoreId<<" msgID "<<m_ServQueueMsg.msgId<<endl;
        /*** Here, as we find out that the RR order has something in the Global Service Queue(oldest or not), we go over ALL TX RESP of the 
         * other cores and see if there is any response related to this specific request (with msgID) ***/

        /*** need to add the logic for choosing the locally oldest ans send the globally oldest request  ***/

        if (m_ServQueueMsg.msgId != 0 && m_ServQueueMsg.reqCoreId == m_respCoreCnt)
        {
          //if (m_reza_log)
           // cout << "In m_ServQueueMsg.msgId != 0 CheckPendingWriteBackResp----------- Found one m_ServQueueMsg.reqCoreId " << m_ServQueueMsg.reqCoreId << " m_ServQueueMsg.wbCoreID  " << m_ServQueueMsg.wbCoreId << endl;
          for (std::list<Ptr<BusIfFIFO>>::iterator it4 = m_busIfFIFO.begin(); it4 != m_busIfFIFO.end(); it4++)
          {
            //cout<<"in the loop"<<endl;
            int pendingQueueSize = (*it4)->m_txRespFIFO.GetQueueSize();
            //if (m_reza_log) cout<<"Check in "<<(*it4)->m_fifo_id<<"  size is "<<pendingQueueSize<<endl;
            for (int i = 0; i < pendingQueueSize && PendingTxWBResp == false; i++)
            {
              pendingWbMsg_2 = (*it4)->m_txRespFIFO.GetFrontElement();
              (*it4)->m_txRespFIFO.PopElement();
              //if (m_reza_log) cout<<"in the TX resp the reqID "<<pendingWbMsg_2.reqCoreId<<"  respID "<<pendingWbMsg_2.respCoreId<<"  msgID  "<<pendingWbMsg_2.msgId<<endl;

              if (((pendingWbMsg_2.addr >> (int)log2(m_cacheBlkSize)) == (m_ServQueueMsg.addr >> (int)log2(m_cacheBlkSize))) && PendingTxWBResp == false && pendingWbMsg_2.msgId == m_ServQueueMsg.msgId)
              {
                //if (m_reza_log) cout<<"there is some core to answer you"<<endl;
                if(selected){
                  if(!(isHigherPrio(pendingWbMsg_1.reqCoreId,pendingWbMsg_2.reqCoreId))){
                    //if (m_reza_log) cout<<"well this is higher priority"<<endl;
                    if (ChkOnly == true) {
                      (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                    }
                    else{
                      //if (m_reza_log) cout<<"check only false"<<endl;
                      if(selected){
                        //if (m_reza_log) cout<<"even selected adding back to "<<(*itx)->m_fifo_id<<endl;
                        (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                      }
                    }
                    mode = 2;
                    PendingTxWBResp = true;
                    txResp = pendingWbMsg_2;
                  }
                  else{
                    //if (m_reza_log) cout<<"it is not higher priority"<<endl;
                    (*it4)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                  }
                }
                else {
                  if (ChkOnly == true) {
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

          /*** Here, as we find out that the RR order has replacement in the Global Service Queue(oldest or not), we go over TX RESP of the 
         * this specific core and see if there is any response related to this specific request (with msgID) ***/

          // if (m_reza_log)
          //   cout << "In m_ServQueueMsg.msgId == 0 CheckPendingWriteBackResp----------- Found one m_ServQueueMsg.reqCoreId " << m_ServQueueMsg.reqCoreId << "m_ServQueueMsg.wbCoreID  " << m_ServQueueMsg.wbCoreId << endl;
          std::list<Ptr<BusIfFIFO>>::iterator it7 = m_busIfFIFO.begin();
          std::advance(it7, m_respCoreCnt);

          //if((*it7)->m_txRespFIFO.IsEmpty() ==  true) cout<<"It is really empty"<<endl;
          //cout<<"the id is "<<(*it7)->m_fifo_id<<endl;
          int pendingQueueSize = (*it7)->m_txRespFIFO.GetQueueSize();

          // if (m_reza_log)
          //   cout << "pendingQueueSize is " << pendingQueueSize << "  for the response queue of  " << (*it7)->m_fifo_id << endl;

          for (int i = 0; i < pendingQueueSize && PendingTxWBResp == false; i++)
          {

            pendingWbMsg_2 = (*it7)->m_txRespFIFO.GetFrontElement();
    
            // if (m_reza_log)
            //   cout << "the write back msg itself has reqCoreID " << pendingWbMsg_2.reqCoreId << "  and respCoreID  " << pendingWbMsg_2.respCoreId << endl;
            // Remove message from the busResp buffer
            (*it7)->m_txRespFIFO.PopElement();
            //cout<<"1"<<endl;
            if (((pendingWbMsg_2.addr >> (int)log2(m_cacheBlkSize)) == (m_ServQueueMsg.addr >> (int)log2(m_cacheBlkSize))) && PendingTxWBResp == false && pendingWbMsg_2.respCoreId == m_respCoreCnt)
            {
              if(selected){
                if(!(isHigherPrio(pendingWbMsg_1.respCoreId,pendingWbMsg_2.respCoreId))){
        
                  if (ChkOnly == true) {
                    (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);
                  }
                  else{
                    //cout<<"check only false"<<endl;
                    if(selected){
                      //cout<<"even selected adding back to "<<(*itx)->m_fifo_id<<endl;
                      (*itx)->m_txRespFIFO.InsertElement(pendingWbMsg_1);
                    }
                  }
                  //cout<<"5"<<endl;
                  mode = 2;
                  PendingTxWBResp = true;
                  txResp = pendingWbMsg_2;
                }
                else{
                  //cout<<"it is not higher priorityyy"<<endl;                
                  (*it7)->m_txRespFIFO.InsertElement(pendingWbMsg_2);               
                }
              }
              else {              
                if (ChkOnly == true) {
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


    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();


    if (PendingTxResp == true && PendingTxWBResp == false && ChkOnly == false)
    {
      if (m_logFileGenEnable)
      {
        std::cout << "BusArbiter: SharedMem granted TDM response slot ============================================================ " << m_arbiCycle << "\n\n";
      }
      SendData(txResp, AGENT::CORE);
      SendData(txResp, AGENT::INTERCONNECT);
      return true;
    }
    else if(PendingTxWBResp == true && ChkOnly == false)
    {
      if (m_reza_log)
        cout << "The reqCore ID is  " << txResp.reqCoreId << "  and the respCoreID  " << txResp.respCoreId << " the address is " << txResp.addr << endl;
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

          unsigned int mask = createMask(6, 6);
          unsigned bank_num = mask & txResp.addr;
          bank_num = bank_num >> 6;
          txResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

           //txResp.sharedCacheAgent = 10;

          //txResp.sharedCacheAgent = txResp.reqCoreId;  /**************** SHOULD BE MODIFIED BASED ON THE SHARED CACHE AGENT ********************/
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
    cout<<"CheckPendingFCFSResp "<<endl;
    double arrivalTime = 0;
    uint16_t core_idx = 0;
    //uint16_t shared_idx   = 10;
    uint16_t TargetCore = 0;
    bool PendingTxReq = false;

    for (std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end(); it1++)
    {
      if ((*it1)->m_txRespFIFO.IsEmpty() == false)
      {
        txMsg = (*it1)->m_txRespFIFO.GetFrontElement();
        if ((PendingTxReq == false) || arrivalTime > txMsg.timestamp)
        {
          //cout<<"1 the address is "<<txMsg.addr<<"  and arrival is  "<<txMsg.timestamp<<endl;
          cout<<"CheckPendingFCFSResp 1"<<endl;
          arrivalTime = txMsg.timestamp;
          TargetCore = core_idx;
        }
        PendingTxReq = true;
      }
      core_idx++;
    }

    BusIfFIFO::BusRespMsg txMsgMem;
    BusIfFIFO::BusRespMsg txMsgMem_temp;
    for (std::list<Ptr<BusIfFIFO>>::iterator it2 = m_sharedCacheBusIfFIFO.begin(); it2 != m_sharedCacheBusIfFIFO.end(); it2++)
    {
      if ((*it2)->m_txRespFIFO.IsEmpty() == false)
      {
        txMsgMem_temp = (*it2)->m_txRespFIFO.GetFrontElement();
        if ((PendingTxReq == false) || arrivalTime > txMsgMem_temp.timestamp)
        {
          cout<<"CheckPendingFCFSResp 2"<<endl;
          txMsgMem = txMsgMem_temp;
          arrivalTime = txMsgMem.timestamp;
          TargetCore = (*it2)->m_fifo_id;
          cout<<"2 the address is "<<txMsgMem.addr<<"  and arrival is  "<<txMsgMem.timestamp<<"  and the target core is "<<TargetCore<<endl;
        }
        PendingTxReq = true;
      }
    }

    if (PendingTxReq)
    {
      cout<<"CheckPendingFCFSResp 3"<<endl;
      if (TargetCore >= 10)
      {
        txMsg = txMsgMem;
        if (ChkOnly == false)
        {
          cout<<"CheckPendingFCFSResp 4"<<endl;
          std::list<Ptr<BusIfFIFO>>::iterator it3 = m_sharedCacheBusIfFIFO.begin();
          std::advance(it3, retrieveSharedCacheID(TargetCore));
          (*it3)->m_txRespFIFO.PopElement();
        }
      }
      else
      {
        cout<<"CheckPendingFCFSResp 5"<<endl;
        //cout<<"here 1"<<endl;
        std::list<Ptr<BusIfFIFO>>::iterator it4 = m_busIfFIFO.begin();
        std::advance(it4, TargetCore);
        txMsg = (*it4)->m_txRespFIFO.GetFrontElement();
        // Remove message from the busResp buffer
        if (ChkOnly == false)
        {
          (*it4)->m_txRespFIFO.PopElement();
        }
      }

      if (ChkOnly == false)
      {
        cout<<"CheckPendingFCFSResp 6"<<endl;
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
        //cout<<"444 address "<<txMsg.addr<<"  shared agent "<<txMsg.sharedCacheAgent<<endl;
        SendData(txMsg, AGENT::CORE);
        //cout<<"5555 address "<<txMsg.addr<<"  shared agent "<<txMsg.sharedCacheAgent<<endl;
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

  // it seems for the response, we should check both the BusIFFIFO and also sharedCacheBusIfFIFO. --> why?  Because pending data can come either from cores (in case of cache to cache) or from shared cache itself.

  // what is the reason of line 360 to 371? we have the same scenario in line 224 to 232 : To my understanding, each of them sends the response to a different destination based on the designated message’s desintation.

  bool BusArbiter::CheckPendingReq(uint16_t core_idx, BusIfFIFO::BusReqMsg &txMsg, bool CheckOnly = false, bool oldest = true)
  {
    if (oldest)
    {
      PendingTxReq = false;
      // if (m_reza_log)
      //   cout << "**********************In CheckPendingReq - Oldest for core " << core_idx << endl;
      std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
      std::advance(it1, core_idx);
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {
        // if (m_reza_log)
        //   cout << "**********************In CheckPendingReq - The queue for CPU  " << core_idx << " is not empty" << endl;
        txMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
        // if (m_reza_log)
        //   cout << "**********************In CheckPendingReq - Global Oldest Queue size is  " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
        
        for (int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && PendingTxReq == false; it2++)
        {
          BusIfFIFO::BusReqMsg tempMsgQueue;
          tempMsgQueue = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
          m_GlobalQueue->m_GlobalOldestQueue.PopElement();
          // if (m_reza_log)
          //   cout << "The address msg in the oldest buffer at " << it2 << " address is  " << tempMsgQueue.addr << " reqCoreID " << tempMsgQueue.reqCoreId << endl;
          if (txMsg.msgId != 0 && txMsg.reqCoreId == tempMsgQueue.reqCoreId && txMsg.addr == tempMsgQueue.addr && txMsg.cohrMsgId == tempMsgQueue.cohrMsgId && existServiceQueue(tempMsgQueue))
          {
            // if (m_reza_log)
            //   cout << "**********************In CheckPendingReq - It does exists in the oldest queue" << endl;
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
          // if (m_reza_log)
          //   cout << "**********************In CheckPendingReq - remove from the CPU buffer  the address is " << txMsg.addr <<"  became oldest at "<<txMsg.becameOldest<< endl;
          // if (m_reza_log)
          //   cout << "size of the txMsg is " << (*it1)->m_txMsgFIFO.GetQueueSize() << endl;
          // Remove message from the busResp buffer
          (*it1)->m_txMsgFIFO.PopElement();
          // if (m_reza_log)
          //   cout << "size of the txMsg after is " << (*it1)->m_txMsgFIFO.GetQueueSize() << endl;
        }
      }
    }
    else
    {

      // if (m_reza_log)
      //   cout << "**********************In CheckPendingReq - Non Oldest" << endl;
      BusIfFIFO::BusReqMsg txResp;
      std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
      std::advance(it1, core_idx);
      bool candidate_chosen = false;

      int pendingQueueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {
        // if (m_reza_log)
        //   cout << "**********************In CheckPendingReq - The NON OLDEST queue for CPU  " << core_idx << " is not empty" << endl;
        if (CheckOnly == true)
        {
          PendingTxReq = false;
          for (int iterator = 0; iterator < (*it1)->m_txMsgFIFO.GetQueueSize() && candidate_chosen == false; iterator++)
          {
            PendingTxReq = true;
            // if (m_reza_log)
            //   cout << "the iterator for the TX of queue is  " << iterator << endl;
            txMsg = (*it1)->m_txMsgFIFO.GetElement(iterator);
            // if (m_reza_log)
            //   cout << "the address of the current i TXMsg is " << txMsg.addr << " shared cache id  " << txMsg.sharedCacheAgent << "  reqCoreID  " << txMsg.reqCoreId << endl;
            for (int j = 0; (j < pendingQueueSize && PendingTxReq == true); j++)
            {
              // if (m_reza_log)
              //   cout << "the iterator inside the global oldest queue " << j << endl;
              txResp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
              m_GlobalQueue->m_GlobalOldestQueue.PopElement();
              // if (m_reza_log)
              //   cout << "checkonly  " << CheckOnly << endl;
              // if (m_reza_log)
              //   cout << "txMsg from the TX adress  " << txMsg.addr << "  agent " << txMsg.sharedCacheAgent << "  reqCoreID " << txMsg.reqCoreId << " wbcoreID " << txMsg.wbCoreId << " msgID " << txMsg.msgId << endl;
              // if (m_reza_log)
              //   cout << "txResp from the oldest address  " << txResp.addr << "  agent " << txResp.sharedCacheAgent << "  reqCoreID " << txResp.reqCoreId << " wbcoreID " << txResp.wbCoreId << " msgID " << txResp.msgId << endl;
              //if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.msgId != 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.wbCoreId == txResp.reqCoreId && txMsg.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
              if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId != 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
              {
                PendingTxReq = false;
                // if (m_reza_log)
                //   cout << "It is false here so no scheduling" << endl;
              }
              m_GlobalQueue->m_GlobalOldestQueue.InsertElement(txResp);
            }

            /******* THE FOLLOWING: Avoid sending a non oldest request to a bank that has a pending request  *************/
            if (PendingTxReq)
            {
              for (int j = 0; (j < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() && PendingTxReq == true); j++)
              {
                // if (m_reza_log)
                //   cout << "the iterator inside the global service queue " << j << endl;
                txResp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
                m_GlobalQueue->m_GlobalReqFIFO.PopElement();
                // if (m_reza_log)
                //   cout << "checkonly  " << CheckOnly << endl;
                // if (m_reza_log)
                //   cout << "txMsg from the TX adress  " << txMsg.addr << "  agent " << txMsg.sharedCacheAgent << "  reqCoreID " << txMsg.reqCoreId << " wbcoreID " << txMsg.wbCoreId << " msgID " << txMsg.msgId << endl;
                // if (m_reza_log)
                //   cout << "txResp from the oldest address  " << txResp.addr << "  agent " << txResp.sharedCacheAgent << "  reqCoreID " << txResp.reqCoreId << " wbcoreID " << txResp.wbCoreId << " msgID " << txResp.msgId << endl;
                //if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.msgId != 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.wbCoreId == txResp.reqCoreId && txMsg.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
                if ((txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId != 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.wbCoreId && txResp.msgId == 0 && CheckOnly == true) || (txMsg.sharedCacheAgent == txResp.sharedCacheAgent && txMsg.reqCoreId == txResp.reqCoreId && txMsg.wbCoreId == txResp.wbCoreId && txMsg.msgId == 0 && txResp.msgId == 0 && CheckOnly == true))
                { 
                  /*************** NEEDS MODIFICATION SINCE NOW WE HAVE MORE BANKS! HENCE, WE SHOULD CHECK IF THAT RESPONSE IS FOR THAT SPECIFIC BANK OR NOT? CORRECT? ***************/
                  PendingTxReq = false;
                  // if (m_reza_log)
                  //   cout << "It is false here so no scheduling" << endl;
                }
                m_GlobalQueue->m_GlobalReqFIFO.InsertElement(txResp);
              }
            }

            if (PendingTxReq && CheckOnly == true)
            {
              if (m_reza_log)
                cout << "candidate chosen here non oldest address " << txMsg.addr << endl;
              candidateID = iterator;
              candidate_chosen = true;
            }
          }
        }
        else
        {
          txMsg = (*it1)->m_txMsgFIFO.GetElement(candidateID);
          // if (m_reza_log)
          //   cout << "Actual Sending the Request from the non oldest REQ arbiter check only false add: " << txMsg.addr << " ID " << txMsg.msgId << endl;
          // if (m_reza_log)
          //   cout << "candidate id is " << candidateID << " and it is removed  CheckOnly  " << CheckOnly << "  PendingTxReq " << PendingTxReq << endl;
          if (PendingTxReq)
          {
            // if (m_reza_log)
            //   cout << "################ candidate id is " << candidateID << " and it is removed" << endl;
            // Remove message from the busResp buffer
            //cout<<" the id is "<<(*it1)->m_txMsgFIFO.GetFrontElement().msgId<<endl;
            (*it1)->m_txMsgFIFO.EraseElement(candidateID);
            PendingTxReq = false;
          }
        }
      }
    }
    return PendingTxReq;
  }
  // the same question as  in  FcFsWriteBackCheckInsert. why m_txRespFIFO? so it is
  // in txResp of the bus.. hummm check it later
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  bool BusArbiter::InsertOnReqBus(BusIfFIFO::BusReqMsg txMsg)
  { // it takes a coherence message that is picked by the requestBus Arbiter and broadcasts it.
    if (m_reza_log)
      cout << " in InsertOnReqBus  msgID "<<txMsg.msgId << endl;
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
        if (m_reza_log)
          cout << " in InsertOnReqBus - Broadcasting to the cores" << endl;
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

    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin(); // Modified to Accomodate Multi Shared Cache
    std::advance(it1, retrieveSharedCacheID(txMsg.sharedCacheAgent));         // Modified to Accomodate Multi Shared Cache
    if (!(*it1)->m_rxMsgFIFO.IsFull())
    { // Modified to Accomodate Multi Shared Cache
      if (m_reza_log)
        cout << "NOW PUSHING TO THE RX MSG OF SHARED CACHE " << txMsg.reqCoreId << "  Adress  " << txMsg.addr << "  agent is " << txMsg.sharedCacheAgent << endl;
      (*it1)->m_rxMsgFIFO.InsertElement(txMsg); // Modified to Accomodate Multi Shared Cache
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
    /*
    if (m_reza_log)
      if (m_PndReq == true)
        cout << "+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||+||" << endl;
    if (m_reza_log)
      cout << "-------------------------------REQ BUS ARBITER RT----------------------------------" << endl;

    if (m_reza_log)
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    if (m_reza_log)
    {
      for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
      {
        BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
        tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
      }
    }
    if (m_reza_log)
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    if (m_reza_log)
    {
      for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
      {
        BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
        tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
        m_GlobalQueue->m_GlobalReqFIFO.PopElement();
        cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
      }
    }

    */


    bool next_arb_level = true;
    // insert message on the request bus (broadcast)
    // in the slot, if there was a pending request chosen before, now its time to broadcast. and finally advance the TDM
    if (m_PndReq)
    {
      if (m_reza_log)
        cout << " In RR_RT_ReqBus  - Broadcast Request  addr  " << m_ReqBusMsg.addr << " become oldest "<<m_ReqBusMsg.becameOldest<< endl;

      if (m_ReqBusMsg.msgId == 0)
        m_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, false, isOldest(m_ReqBusMsg.addr, m_ReqBusMsg.wbCoreId));
      else
        m_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, false, isOldest(m_ReqBusMsg.addr, m_ReqBusMsg.reqCoreId));

      m_PndReq = false;
      // insert message on request bus

      InsertOnReqBus(m_ReqBusMsg);

      if (m_ReqBusMsg.NoGetMResp == false)
      { /// what is NoGetMResp?
        // cout<<"NoGetMResp"<<endl;
        // abort();
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
      }
    }

    /* Now, the chosen REQ is issued. We should check for the next transaction (according to the RR)
       * either if there was no pending requst or it was broadcasted, we should check for the next candidate
       * First check if the RR candidate has anything in its queue
       * Then advance RR order if didn't find anything, advance until you find something  */
    //cout<<" In RR_RT_ReqBus - the RR size is  "<<m_GlobalQueue->m_GlobalRRQueue.size()<<endl;

    
    for (unsigned int i = 0; i < m_GlobalQueue->m_GlobalRRQueue.size(); i++)
    {
      if (m_reza_log)
        cout << " Order is: " << m_GlobalQueue->m_GlobalRRQueue.at(i) << endl;
    }
    
    bool temp_PndReq = false;
    /********************* First Level of Arbitration  ************************/
    for (unsigned int RR_iterator = 0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && m_PndReq == false; RR_iterator++)
    {

      if (m_GlobalQueue->m_GlobalRRQueue.size() != 0)
      {
        m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);

        temp_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, true, true); // CheckPendingReq returns true the front element of the buffer
        if (temp_PndReq)
        {
          if (m_reza_log)
          {
            cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
            cout << "THE REQUEST BUS SCHEDULED A REQUEST TO BE BROADCASTED IN THE NEXT DELTA INTERVAL" << endl;
            cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
          }
        }
        m_PndReq = temp_PndReq;
      }
    }
    // if a Request is chosen, no need for second level of arbitration
    if (m_PndReq)
    {
      m_PndOld = true;
      next_arb_level = false;
    } /* if a msg is picked at first level, there is no need for the second level of arbiteration. */
    //cout<<" In RR_RT_ReqBus Before Second Level"<<endl;
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
          //cout<<"check if requestor "<<m_reqCoreCnt<<" has any eligible in Non oldest queue"<<endl;
          temp_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, true, false);
          if (temp_PndReq)
          {
            if (m_reza_log)
            {
              cout << "1^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
              cout << "THE REQUEST BUS SCHEDULED A REQUEST TO BE BROADCASTED IN THE NEXT DELTA INTERVAL" << endl;
              cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
            }
          }
          m_PndReq = temp_PndReq;
          if (m_PndReq)
            m_PndOld = false;
        }
      }
    }

    // if (m_reza_log)
    //   cout << "-------------------------------REQ BUS ARBITER 22 ----------------------------------" << endl;

    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
    //     tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
    //     m_GlobalQueue->m_GlobalOldestQueue.PopElement();
    //     cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  WBID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
    //   }
    // }
    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
    //     tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
    //     m_GlobalQueue->m_GlobalReqFIFO.PopElement();
    //     if (m_reza_log)
    //       cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  WBID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
    //   }
    // }
    //cout<<" In RR_RT_ReqBus Before Scheduling the next transaction"<<endl;
    // wait one TDM Request slot, if there is any request
    if (m_PndReq)
    {
      m_GlobalQueue->m_busArbBlock = m_reqclks;
      if (m_reza_log)
        cout << " ************************************************************************************************************************************************************In RR_RT_ReqBus a candidate is chosen to schedule" << endl;
      Simulator::Schedule(NanoSeconds(m_dt * m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }
    else
    { // wait one clock cycle and check again
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter>(this));
    }
  }

  bool BusArbiter::isOldest(uint64_t adr, unsigned int coreIndex)
  {
    //cout<<" ------------------------In Oldest Function------------------------- "<<endl;
    
    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg;
    int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
      m_GlobalQueue->m_GlobalOldestQueue.PopElement();
      if ((tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex) || ((tempOldestReqMsg.addr == adr && tempOldestReqMsg.wbCoreId == coreIndex) && tempOldestReqMsg.msgId == 0))
      {
        if (m_reza_log)
          cout << "IT does exists in OLDEST Queeue" << endl;
        success = true;
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
      }
      else {
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
      }      
    }
    //cout<<" ------------------------------------------------------------------- "<<endl;
    return success;
  }

  bool BusArbiter::removeFromM_Type(uint64_t adr, unsigned int coreIndex, bool replacement) {
    bool success = false;
    BusIfFIFO::BusReqMsg tempOldestReqMsg_removeFromMtype;
    int queueSize = m_GlobalQueue->m_MsgType.GetQueueSize();
    //cout<<"the size of removeFromM_Type is  "<<queueSize<<endl; 
    for (int itr = 0; itr < queueSize; itr++)
    {
      tempOldestReqMsg_removeFromMtype = m_GlobalQueue->m_MsgType.GetFrontElement();
      m_GlobalQueue->m_MsgType.PopElement();
      if ((replacement == false && tempOldestReqMsg_removeFromMtype.addr == adr && tempOldestReqMsg_removeFromMtype.reqCoreId == coreIndex) ||
          (replacement == true && tempOldestReqMsg_removeFromMtype.addr == adr && tempOldestReqMsg_removeFromMtype.wbCoreId == coreIndex))
      {
                
        if (m_reza_log)
          cout << "the request address " << tempOldestReqMsg_removeFromMtype.addr << "  msgID  " << tempOldestReqMsg_removeFromMtype.msgId << "   is removed from Oldest" << endl;
        success = true;
        //cout<<"the size of removeFromM_Type after deletion is  "<<m_GlobalQueue->m_MsgType.GetQueueSize()<<endl;
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
        //if (m_reza_log){
          if(m_wcArbiter < m_arbiCycle - tempOldestReqMsg.becameOldest)
            m_wcArbiter = m_arbiCycle - tempOldestReqMsg.becameOldest;
          cout<<"The Oldest MsgID "<<tempOldestReqMsg.msgId<<" reQID "<<coreIndex<<" wbID "<<tempOldestReqMsg.wbCoreId<<" Terminated @ "<<m_arbiCycle<<" Arrival @ "<<tempOldestReqMsg.becameOldest<<" Latency: "<<
                m_arbiCycle - tempOldestReqMsg.becameOldest<<"  The WC is "<<m_wcArbiter<<endl; 
        //}
        // if(m_wcArbiter >= 180)
        //   abort();
 
        if (m_reza_log)
          cout << "the request address " << tempOldestReqMsg.addr << "  msgID  " << tempOldestReqMsg.msgId << "   is removed from Oldest" << endl;
        success = true;
        return true;
      }
      m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
    }
    return success;
  }

  void BusArbiter::adjustOldest(unsigned int coreIndex)
  {
    
    cout<<"*************************************************************"<<endl; 
    bool temporal = false;
    // if (m_reza_log)
    //   cout << "-------------------------------REQ BUS ARBITER 00 ----------------------------------" << endl;

    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
    //     tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
    //     m_GlobalQueue->m_GlobalOldestQueue.PopElement();
    //     cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
    //   }
    // }
    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
    //     tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
    //     m_GlobalQueue->m_GlobalReqFIFO.PopElement();
    //     cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
    //   }
    // }




    double arrivalTime = 0;
    bool PendingTxReq = false;
    unsigned int queueSize = 0;

    BusIfFIFO::BusReqMsg tempFCFSMsg;
    BusIfFIFO::BusReqMsg tempCandidate;

    //if(m_reza_log) cout<<"****************************************** Adjust Oldest *************************************************  "<<coreIndex<<endl;
    std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();

    std::advance(it1, coreIndex);
    //cout<<"the id buffer is "<<(*it1)->m_fifo_id<<endl;
    if (m_reza_log)
      cout << "****************************************** Adjust Oldest ************************************************* arbiter  " << coreIndex << endl;
    // First take the oldest REQ from IF FIFO request buffer
    if ((*it1)->m_txMsgFIFO.IsEmpty() == false)
    {
      tempFCFSMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
      arrivalTime = tempFCFSMsg.timestamp;
      PendingTxReq = true;
      tempCandidate = tempFCFSMsg;
      if (m_reza_log)
        cout << " Requestor  " << coreIndex << "  Buffer not empty and the addre " << tempCandidate.addr << " msgID " << tempCandidate.msgId << "  req  " << tempCandidate.reqCoreId << endl;
    }
   
    // Second parse the non oldest requests that already was sent into the banks
    queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
    //if(m_reza_log) cout<<" service  queue size "<<queueSize<<endl;
    for (unsigned int itr = 0; itr < queueSize; itr++)
    {
      if (m_GlobalQueue->m_GlobalReqFIFO.IsEmpty() == false)
      {
        //if(m_reza_log) cout<<" not empty"<<queueSize<<endl;
        tempFCFSMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
        //cout<<"The address msg in the oldest buffer at "<<itr<<" address is  "<<tempFCFSMsg.addr<<" reqCoreID "<<tempFCFSMsg.reqCoreId<<" arrive time "<<tempFCFSMsg.timestamp<<"  while the arrive time is "<<arrivalTime<<endl;
        m_GlobalQueue->m_GlobalReqFIFO.PopElement();
        if ((tempFCFSMsg.msgId != 0 && tempFCFSMsg.reqCoreId == coreIndex) || (tempFCFSMsg.msgId == 0 && tempFCFSMsg.wbCoreId == coreIndex))
        {
          if ((PendingTxReq == false) || arrivalTime > tempFCFSMsg.timestamp)
          {
            //cout<<"found arlier addre "<<tempFCFSMsg.addr<<endl;
            temporal = true;
            PendingTxReq = true;
            arrivalTime = tempFCFSMsg.timestamp;
            tempCandidate = tempFCFSMsg;
          }
        }
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempFCFSMsg);
      }
    }
    // cout<<"-----------------------------------------------------Remove From the Oldest---------------------------------------------------------"<<endl;
    // for(int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it2++) {
    //   BusIfFIFO::BusReqMsg xxx;
    //   xxx = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
    //   m_GlobalQueue->m_GlobalOldestQueue.PopElement();
    //   //cout<<"The address msg in the oldest buffer at "<<it2<<" address is  "<<xxx.addr<<" reqCoreID "<<xxx.reqCoreId<<endl;
    //   m_GlobalQueue->m_GlobalOldestQueue.InsertElement(xxx);
    // }
    // cout<<"------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    // now it should move to the oldest queue
    if (PendingTxReq == true)
    {
      tempCandidate.becameOldest = m_arbiCycle;
      m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempCandidate);
      m_GlobalQueue->m_GlobalRRQueue.push_back(coreIndex);      

      if(temporal) {
        BusIfFIFO::BusReqMsg tempitr7;
        for(int itr7= 0; itr7 < m_GlobalQueue->m_MsgType.GetQueueSize(); itr7++){
          tempitr7 = m_GlobalQueue->m_MsgType.GetFrontElement();
          m_GlobalQueue->m_MsgType.PopElement();
          if(tempitr7.msgId == tempCandidate.msgId && tempitr7.addr == tempCandidate.addr){
            tempitr7.becameOldest = m_arbiCycle;
            cout<<"It is being oldest at  "<<tempitr7.becameOldest<<endl;
            assignDeadlineAfterDetermination(tempitr7);  
            m_GlobalQueue->m_MsgType.InsertElement(tempitr7);
          }
          else{
            m_GlobalQueue->m_MsgType.InsertElement(tempitr7);
          }          
        }        
      } 
          
      
      // if (m_reza_log)
      //   cout << "-------------------------------REQ BUS ARBITER 80----------------------------------" << endl;

      // if (m_reza_log)
      //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      // if (m_reza_log)
      // {
      //   for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
      //   {
      //     BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
      //     tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
      //     m_GlobalQueue->m_GlobalOldestQueue.PopElement();
      //     cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
      //     m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
      //   }
      // }
      // if (m_reza_log)
      //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      // if (m_reza_log)
      // {
      //   for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
      //   {
      //     BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
      //     tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
      //     m_GlobalQueue->m_GlobalReqFIFO.PopElement();
      //     cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
      //     m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
      //   }
      // }
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
        if (m_reza_log)
          cout << "the request address " << tempNonOldestReqMsg.addr << "  msgID  " << tempNonOldestReqMsg.msgId << "   is removed from NON OLdest" << endl;
        success = true;
        return true;
      }
      m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsg);
    }
    return success;
  }

  bool BusArbiter::isFinal(uint64_t adr, unsigned int messageID, uint64_t sharedID)
  {
    //cout<<"sharedIDDD "<<sharedID<<endl;
    BusIfFIFO::BusRespMsg BusRespMsg_RXtemp;
    std::list<Ptr<BusIfFIFO>>::iterator it7 = m_sharedCacheBusIfFIFO.begin(); // Modified to Accomodate Multi Shared Cache
    std::advance(it7, retrieveSharedCacheID(sharedID));                       // Modified to Accomodate Multi Shared Cache
    if (!(*it7)->m_rxRespFIFO.IsEmpty())
    {
      for (int itr_temp = 0; itr_temp < (*it7)->m_rxRespFIFO.GetQueueSize(); itr_temp++)
      {
        BusRespMsg_RXtemp = (*it7)->m_rxRespFIFO.GetFrontElement();
        (*it7)->m_rxRespFIFO.PopElement();
        if (BusRespMsg_RXtemp.addr == adr && BusRespMsg_RXtemp.msgId == messageID)
        {
          if (m_reza_log)
            cout << "It is not final thus should be removed from the shared memory" << endl;
          (*it7)->m_rxRespFIFO.InsertElement(BusRespMsg_RXtemp); // Modified to Accomodate Multi Shared Cache
          return false;
        }
        (*it7)->m_rxRespFIFO.InsertElement(BusRespMsg_RXtemp); // Modified to Accomodate Multi Shared Cache
      }
      return true;
    }
    return true;
  }

  void BusArbiter::RR_RT_RespBus()
  {
    
    
    // if (m_reza_log)
    //   cout << "-------------------------------RESPONSE BUS ARBITER ----------------------------------" << endl;

    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Oldest Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempOldestReqMsgTemp;
    //     tempOldestReqMsgTemp = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
    //     m_GlobalQueue->m_GlobalOldestQueue.PopElement();
    //     cout << "AT " << u << " ADDR " << tempOldestReqMsgTemp.addr << "  ReqID  " << tempOldestReqMsgTemp.reqCoreId << "  wbID  " << tempOldestReqMsgTemp.wbCoreId << "  MSGID " << tempOldestReqMsgTemp.msgId << "  AGENT  " << tempOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsgTemp);
    //   }
    // }
    // if (m_reza_log)
    //   cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Global Service Queue Contains the Following ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    // if (m_reza_log)
    // {
    //   for (int u = 0; u < m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize(); u++)
    //   {
    //     BusIfFIFO::BusReqMsg tempNonOldestReqMsgTemp;
    //     tempNonOldestReqMsgTemp = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
    //     m_GlobalQueue->m_GlobalReqFIFO.PopElement();
    //     cout << "AT " << u << " ADDR " << tempNonOldestReqMsgTemp.addr << "  ReqID  " << tempNonOldestReqMsgTemp.reqCoreId << "  wbID  " << tempNonOldestReqMsgTemp.wbCoreId << "  MSGID " << tempNonOldestReqMsgTemp.msgId << "  AGENT  " << tempNonOldestReqMsgTemp.sharedCacheAgent << endl;
    //     m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsgTemp);
    //   }
    // }

      
      if (m_PndMemResp)
      {
        // a response from memory is sent
        if (m_reza_log)
          cout << "||||||||||||||| In Response Arbiter  - Sending the Response |||||||||||||||   addr  " << m_PendResp.addr << endl;
        if(m_duetto) {
          bool terminateii = false;
          for(int ii1=0; ii1<m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii1++){
            BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
            m_GlobalQueue->m_MsgType.PopElement();
            if(tempS.msgId == m_PendResp.msgId && tempS.addr == m_PendResp.addr) {
              cout<<"RT Stage is changed for "<<tempS.msgId<<" to Response "<<endl;
              tempS.currStage = "RESPONSE";
              terminateii = true;
              m_GlobalQueue->m_MsgType.InsertElement(tempS);
            }
            else{
              m_GlobalQueue->m_MsgType.InsertElement(tempS);
            }
          }
        } 
        if(mode == 1){      
          if (m_reza_log) cout<<"mode is 1"<<endl;
          CheckPendingResp(m_PendResp, false, isOldest(m_PendResp.addr, m_PendResp.reqCoreId), mode);
          
        }
        else if (mode == 2){
          if (m_reza_log) cout<<"mode is 2"<<endl;
          CheckPendingResp(m_PendResp, false, isOldest(m_PendResp.addr,m_PendResp.reqCoreId), mode);
        }
        
        /* the transaction is READ and is considered to be finished now
           * 0- check first if it is the oldest of its requestor
           * 1- If YES, adjust the order 
           * 2- If YES, remove this oldest request from oldest queue
           * 3- adjust the oldest request
           * 4- If NOT, remove the request from the global queue
           * 
           * Revision:
           * 1- The replacement will never terminate here at the bus; it always terminates at the bank sinc the 
           * sequence is always: REQ:RESPONSE:BANK
           * 2- For any other requests, we need to make sure if there is any response related to the current response in the bank
           * if such transaction exists, the request will not terminate until the transaction in the bank is finished. For example
           * another core can respond both the the core and shared cache and the response reach the requesting core. However there 
           * still is a transaction in the bank queue to commit.  
           */

        if (m_PendResp.msgId != 0)
        {
          unsigned int mask = createMask(6, 6);
          unsigned bank_num = mask & m_PendResp.addr;
          bank_num = bank_num >> 6;
          m_PendResp.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

          // m_PendResp.sharedCacheAgent = 10;

          if (isFinal(m_PendResp.addr, m_PendResp.msgId, m_PendResp.sharedCacheAgent))
          {
            if (m_reza_log)
              cout << "It is Final 1" << endl;
            if (isOldest(m_PendResp.addr, m_PendResp.reqCoreId))
            {
              // 0- check first if it is the oldest of its requestor
              if (m_reza_log)
                cout << "It is oldest the order now is " << m_GlobalQueue->m_GlobalRRQueue.at(0) << endl;
              // 1- If YES, adjust the order
              for (unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size(); h++)
              {
                if (m_GlobalQueue->m_GlobalRRQueue.at(h) == m_PendResp.reqCoreId)
                {
                  m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                }
              }
              //if(m_GlobalQueue->m_GlobalRRQueue.size() >0) cout<<"Order is adjusted and now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<"  the oldest queue size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<endl;
              // 2- If YES, remove this oldest request from oldest queue

              if (m_reza_log)
                cout << " the oldest queue size " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
              removeFromOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              if (m_reza_log)
                cout << " the oldest queue size " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;

              removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false);  

              if (m_reza_log)
                cout << " the non oldest queue size " << m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() << endl;
              removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              if (m_reza_log)
                cout << " the non oldest queue size after removing " << m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize() << endl;
              if (m_reza_log)
                cout << " it should be removed from oldest queue and now the size is  " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << endl;
              // 3- adjust the oldest request
              adjustOldest(m_PendResp.reqCoreId);
              if (m_reza_log)
                cout << " the oldest shold be inserted if exist and now the size is  " << m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() << " the Order queue size is " << m_GlobalQueue->m_GlobalRRQueue.size() << endl;
              
      
            }
            else
            {
              //cout<<" the request is non oldest so remove from the non oldest queue the size is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
              removeFromM_Type(m_PendResp.addr, m_PendResp.reqCoreId, false); 
              removeFromNonOldest(m_PendResp.addr, m_PendResp.reqCoreId, false);
              //cout<<" the request is non oldest so remove from the non oldest queue the size after removing is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
            }
          }
          else
          {
            cout << "ERROR: how is it possible to have a response from shared memory but it is not the final response? Check the trace and extract the scenario" << endl;
            abort();
          }
        }
        // else
        // {
        //   cout << "ERROR: how is it possible to have a replacement response from shared memory? Check the trace and extract the scenario" << endl;
        //   abort();
        // }
        /***********************************  function to adjust the oldest request **********************************************/
      }      
    
    oldest_determine = true;
    m_PndResp = false;
    m_PndMemResp = false;

    if (m_reza_log)
      cout << "Before going over the response arbiteration" << endl;
    m_PndMemResp = CheckPendingResp(m_PendResp, true, true, mode);
    if (m_PndMemResp == false)
    {
      if (m_reza_log)
          cout << " CheckPendingResp - no oldest exists " << endl;
        m_PndMemResp = CheckPendingResp(m_PendResp, true, false, mode); 
        if(m_PndMemResp == false ){
          if (m_reza_log)
            cout << " CheckPendingResp - no non oldest exists " << endl;
        }
        else {
          if (m_reza_log)
            cout << " ((((((((((((((((((((((((((((((((((((((((((((((((RESPONSE ARBITRATED From NON OLDET))))))))))))))))))))))))))))))))))))))))))))))))))))))adr "<<m_PendResp.addr<<" ID "<<m_PendResp.msgId << endl;    
        }
    }
    else
    {
      if (m_reza_log)
        cout << " ((((((((((((((((((((((((((((((((((((((((((((((((RESPONSE ARBITRATED From OLDET))))))))))))))))))))))))))))))))))))))))))))))))))adr "<<m_PendResp.addr<<" ID "<<m_PendResp.msgId<< endl;
    }

    
    

    // check service queue
    // wait Resp-TDM response slot
    if (m_PndMemResp == true)
    {
      m_GlobalQueue->m_respArbBlock = m_respclks;
      if (m_reza_log)
        cout << "|||||||||||||||||||||||||||| RT RESP SCHEDULED TO SENT IN NEXT DELTA |||||||||||||||||||||||||||||||  " << m_PendResp.msgId<< endl;
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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::L2CohrMsgHandle()
  {
    //cout<<"In L2CohrMsgHandle"<<endl;
    BusIfFIFO::BusReqMsg txreqMsg;

    for (std::list<Ptr<BusIfFIFO>>::iterator it1 = m_sharedCacheBusIfFIFO.begin(); it1 != m_sharedCacheBusIfFIFO.end(); it1++)
    { // Modified to Accomodate Multi Shared Cache
      if (!(*it1)->m_txMsgFIFO.IsEmpty())
      {                                                   // Modified to Accomodate Multi Shared Cache
        txreqMsg = (*it1)->m_txMsgFIFO.GetFrontElement(); // Modified to Accomodate Multi Shared Cache
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
    Simulator::Schedule(NanoSeconds(m_dt / 4), &BusArbiter::L2CohrStep, Ptr<BusArbiter>(this)); // Modified to Accomodate Multi Shared Cache
  }

  void BusArbiter::ReqFncCall()
  {
    //cout<<"In ReqFncCall"<<endl;
    if(m_duetto){
      /**** Logic to determine the arbiter****/             
      if(arb_req_mode == "RT") {
        RR_RT_ReqBus();
      }
      else if (arb_req_mode == "HP") { 
        MSI_FcFsReqBus();
      }          
    }
    else {
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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::RespFncCall()
  {
    //cout<<"In RespFncCall"<<endl;

    if(m_duetto){
      /**** Logic to determine the arbiter****/   
      if(arb_resp_mode == "RT") {
        RR_RT_RespBus();      
      }
      else if(arb_resp_mode == "HP") {
        MSI_FcFsRespBus();      
      } 
    }
    else {
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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  /**
    * Call request/response bus functions
    * These function is called each interval dt
    */
  void BusArbiter::ReqStep(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->ReqFncCall();
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::RespStep(Ptr<BusArbiter> busArbiter)
  {
    busArbiter->RespFncCall();
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
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
      cout<<"before logic to dtermine "<<endl;
      /**** Logic to determine the arbiter****/
      if (busArbiter->WCLator())
      {
          if (busArbiter->m_reza_log)
              cout << "---------------------------------------------------------------------------------------------------------------------------------------------- REQ Arbiter HP Mode" << endl;
          if (busArbiter->arb_req_mode == "RT")
          {
              busArbiter->m_PndReq = false;
              cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from RT to HP  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
          }
          busArbiter->arb_req_mode = "HP";
          if (busArbiter->m_reza_log)
              cout << "---------------------------------------------------------------------------------------------------------------------------------------------- RESP Arbiter HP Mode" << endl;
          if (busArbiter->arb_resp_mode == "RT")
          {
              busArbiter->m_PndMemResp = false;
              cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from RT to HP  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
          }
          busArbiter->arb_resp_mode = "HP";
      }
      else
      {
          if (busArbiter->m_reza_log)
              cout << "----------------------------------------------------------------------------------------------------------------------------------------------  REQ Arbiter RT Mode " << endl;
          if (busArbiter->arb_req_mode == "HP")
          {
              busArbiter->m_PndReq = false;
              cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from HP to RT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
          }
          busArbiter->arb_req_mode = "RT";

          if (busArbiter->m_reza_log)
              cout << "---------------------------------------------------------------------------------------------------------------------------------------------- RESP Arbiter RT Mode " << endl;
          if (busArbiter->arb_resp_mode == "HP")
          {
              busArbiter->m_PndMemResp = false;
              cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from HP to RT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
          }
          busArbiter->arb_resp_mode = "RT";
      }
    }

  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  void BusArbiter::CycleAdvance()
  {
    cout << "In BusArbiter::CycleAdvance   " << m_arbiCycle <<" REQ Block "<<m_GlobalQueue->m_busArbBlock<<" RESPONSE Block "<<m_GlobalQueue->m_respArbBlock<< endl;            
    for (std::list<Ptr<BusIfFIFO>>::iterator it13 = m_sharedCacheBusIfFIFO.begin(); it13 != m_sharedCacheBusIfFIFO.end(); it13++)
    {
      cout << "For Bank "<<(*it13)->m_fifo_id<<" Bank Block "<<(*it13)->m_bankArbBlock<<endl; 
    }    
    if(m_GlobalQueue->m_busArbBlock > 0 ) m_GlobalQueue->m_busArbBlock--; 
    if(m_GlobalQueue->m_respArbBlock > 0 ) m_GlobalQueue->m_respArbBlock--; 

    if (m_stallDetectionEnable)
    {
      m_stall_cnt = (m_PndReq) ? 0 : (m_stall_cnt + 1);

      if (m_stall_cnt >= Stall_CNT_LIMIT)
      {
        exit(0);
      }
    }

    m_arbiCycle++;

    if (m_arbiCycle % 1000000 == 0){      
      cout << "HeartBeat ::" << m_arbiCycle <<" C2C Transfer :: "<<m_GlobalQueue->m_Core2CoreTransfer<< " Bank Transfer :: "<<m_GlobalQueue->m_SharedBankTransfer<< endl;
    }
    if (m_reza_log)
      cout << "-----------------------------------------------------------------CLOCK: " << m_arbiCycle << "----------------------------------------------------------" << endl;
    //  if(m_arbiCycle > 1 ){
    //    //m_reza_log = true;
     if(m_arbiCycle == 100000)
      abort();
    //  }

    Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::Step, Ptr<BusArbiter>(this));
    //cout<<"Finish In BusArbiter::CycleAdvance"<<endl;
  }
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
}
