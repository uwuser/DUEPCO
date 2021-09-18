/*
 * File  :      SharedCacheCtrl.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 20, 2020
 */

#include "SharedCacheCtrl.h"
using namespace std;

namespace ns3 {
    TypeId SharedCacheCtrl::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::SharedCacheCtrl")
               .SetParent<Object > ();
        return tid;
    }

    SharedCacheCtrl::SharedCacheCtrl(std::list<ns3::Ptr<ns3::BusIfFIFO> > associatedPrivCacheBusIfFIFO,
                     uint32_t cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO,
                     Ptr<DRAMIfFIFO> associatedDRAMBusIfFifo,
                     ns3::Ptr<ns3::GlobalQueue> globalQueue) {
        // default
        m_cacheType        = 0;
        m_cacheSize        = 64*256;
        m_duetto           = false; 
        m_cacheBlkSize     = 64;
        m_victimCacheSize  = 64*4;
        m_nways            = 1;
        m_nsets            = 256;
        m_coreId           = 1;
        m_reqclks = 4;
        m_sharedcachelatency = 20;        
        m_respclks = 50;
        m_dt               = (1.0/1000000);
        m_clkSkew          = 0;
        m_cacheCycle       = 1;
        m_cache2Cache      = false;
        m_nPrivCores       = 4;
        m_replcPolicy      = ReplcPolicy::RANDOM;
        m_logFileGenEnable = false;
        m_cache            = CreateObject<GenericCache> (cachLines);
        m_busIfFIFOPrivate = associatedPrivCacheBusIfFIFO;        
        m_busIfFIFO        = assoicateBusIfFIFO;
        m_dramBusIfFIFO    = associatedDRAMBusIfFifo;
        m_GlobalQueue      = globalQueue;
        m_ownerCoreId      = new uint16_t[cachLines];
        m_PndWBFIFO.SetFifoDepth(30);
        //m_localPendingRespTxBuffer.SetFifoDepth(30);
        m_DRAMOutStandingExclMsg.SetFifoDepth(30);
        m_l2CachePreloadFlag = false;
        m_dramSimEnable   = false;
        m_dramLatcy       = 100;
        m_dramOutstandReq = 6;
        m_dramModle       = "FIXEDLat";
        m_Nreqs           = 0;
        m_Nmiss           = 0;
        m_sharedCacheBusy = false;
        m_direction       = false;
        m_reza_log_shared = true;
        m_mode            = "HP"; 
        m_wcShared        = 0;
        
    }

    // We don't do any dynamic allocations
    SharedCacheCtrl::~SharedCacheCtrl() {
    }

    void SharedCacheCtrl::SetCacheSize (uint32_t cacheSize) {
      m_cacheSize = cacheSize;
      m_cache->SetCacheSize(cacheSize);
    }

    uint32_t SharedCacheCtrl::GetCacheSize () {
      return m_cacheSize;
    }    

    void SharedCacheCtrl::SetCacheBlkSize (uint32_t cacheBlkSize) {
      m_cacheBlkSize = cacheBlkSize;
      m_cache->SetCacheBlkSize(cacheBlkSize);
    }

    uint32_t SharedCacheCtrl::GetCacheBlkSize () {
      return m_cacheBlkSize;
    }

    void SharedCacheCtrl::SetCacheNways (uint32_t nways) {
      m_nways = nways;
      m_cache->SetCacheNways(nways);
    }

    uint32_t SharedCacheCtrl::GetCacheNways () {
      return m_nways;
    }  

    void SharedCacheCtrl::assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg & msg) {
      if(m_reza_log_shared) cout<<"In  shared cache the assignDeadlineAfterDetermination"<<endl;
      //abort();
      unsigned int WCL_0;
      unsigned int WCL_1;
      unsigned int WCL_2;
      WCL_0 = 158;
      WCL_1 = 158;
      WCL_2 = 158;       
      cout<<"become oldest "<<msg.becameOldest<<endl;
      if(msg.orderDetermined) {
        switch (msg.orderofArbitration) {
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

    void SharedCacheCtrl::SetCacheNsets (uint32_t nsets) {
      m_nsets = nsets;
      m_cache->SetCacheNsets(nsets);
    }

    uint32_t SharedCacheCtrl::GetCacheNsets () {
      return m_nsets;
    }

    void SharedCacheCtrl::SetCacheType (uint16_t cacheType) {
      m_cacheType = cacheType;
      m_cache->SetCacheType(cacheType);
    }

    void SharedCacheCtrl::SetVictCacheSize (uint32_t cacheSize) {
      m_victimCacheSize = cacheSize;
    }
    
    uint16_t SharedCacheCtrl::GetCacheType () {
      return m_cacheType;
    }

    void SharedCacheCtrl::SetNumReqCycles(int ncycle)
    {
      m_reqclks = ncycle;
    }

    void SharedCacheCtrl::SetNumRespCycles(int ncycle)
    {
      m_respclks = ncycle;
    }
    void SharedCacheCtrl::SetReplcPolicy (ReplcPolicy replcPolicy) {
      m_replcPolicy = replcPolicy;
    }

    void SharedCacheCtrl::SetCoreId (int coreId) {
      m_coreId = coreId;
    }

    void SharedCacheCtrl::SetIsDuetto (bool duetto) {
      m_duetto = duetto;
    }

    int SharedCacheCtrl::GetCoreId () {
      return m_coreId;
    }

    void SharedCacheCtrl::SetDt (double dt) {
      m_dt = dt;
    }

    int SharedCacheCtrl::GetDt () {
      return m_dt;
    }

    void SharedCacheCtrl::SetClkSkew (double clkSkew) {
       m_clkSkew = clkSkew;
    }

    void SharedCacheCtrl::SetCache2Cache (bool cache2Cache) {
       m_cache2Cache = cache2Cache;
    }

    void SharedCacheCtrl::SetBMsPath  (std::string bmsPath) {
      m_bmsPath = bmsPath;
    }

    void SharedCacheCtrl::SetNumPrivCore (int nPrivCores) {
      m_nPrivCores = nPrivCores;
    }

    void SharedCacheCtrl::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }

    void SharedCacheCtrl::SetDramSimEnable (bool dramSim_en ) {
      m_dramSimEnable = dramSim_en;
    }
    
    void SharedCacheCtrl::SetDramFxdLatcy (uint32_t dramLatcy ) {
      m_dramLatcy = dramLatcy;
    }
    
    void SharedCacheCtrl::SetDramModel (std::string dramModel ) {
      m_dramModle = dramModel;
    }
    
    void SharedCacheCtrl::SetDramOutstandReq (uint32_t dramOutstandReqs) {        
      m_dramOutstandReq = dramOutstandReqs;
    }

    void SharedCacheCtrl::SetSharedCacheLatency (uint32_t latency)  {    // Modified to Accomodate Multi Shared Cache
      m_sharedcachelatency = latency;                                    // Modified to Accomodate Multi Shared Cache
    }                                                                    // Modified to Accomodate Multi Shared Cache

    void SharedCacheCtrl::SetCachePreLoad (bool l2CachePreload) {
      m_l2CachePreloadFlag = l2CachePreload;
    }
    
    uint64_t SharedCacheCtrl::GetShareCacheMisses () {
      return m_Nmiss;
    }
    
    uint64_t SharedCacheCtrl::GetShareCacheNReqs () {
      return m_Nreqs;
    }

    

    

    bool SharedCacheCtrl::terminateBank(unsigned int coreID, uint64_t adr, unsigned int msg ){
      bool terminate;
      if(!m_cache2Cache) {
      //  if(m_reza_log_shared)  cout<<" there is no cache2cache so no need for checking the local one"<<endl;
        return false;
      }
      else {
        //if(m_reza_log_shared) cout<<"In terminateBank "<<endl;
        terminate = true;
        std::list<Ptr<BusIfFIFO>>::iterator it3 = m_busIfFIFOPrivate.begin();
        std::advance(it3, coreID);
        //if(m_reza_log_shared) cout<<"In terminateBank 1"<<endl;
        BusIfFIFO::BusRespMsg temp_resp;
        if ((*it3)->m_rxRespFIFO.IsEmpty() == false) { 
          //if(m_reza_log_shared) cout<<"In terminateBank 2"<<endl;
          for(int i = 0; i< (*it3)->m_rxRespFIFO.GetQueueSize(); i++) {
            //if(m_reza_log_shared) cout<<"In terminateBank 4"<<endl;
            temp_resp = (*it3)->m_rxRespFIFO.GetFrontElement();
            (*it3)->m_rxRespFIFO.PopElement();
            if(temp_resp.addr == adr && temp_resp.msgId == msg){
              (*it3)->m_rxRespFIFO.InsertElement(temp_resp);
              //if(m_reza_log_shared) cout<<"Should not terminate "<<endl;
              terminate = false;
            }
            else{
              (*it3)->m_rxRespFIFO.InsertElement(temp_resp);
            }
          }        
        }
      }      
      //if(m_reza_log_shared) cout<<"In terminateBank Done"<<endl;
      return terminate;
    }

    SNOOPSharedCtrlEvent SharedCacheCtrl::ChkDRAMReqEvent (SNOOPSharedReqBusEvent busReqEvent) {
      SNOOPSharedCtrlEvent DRAMReqEvent;
      
      if (busReqEvent == SNOOPSharedReqBusEvent::GetM) {
        DRAMReqEvent = SNOOPSharedCtrlEvent::DRAMGetM;
      }
      else if (busReqEvent == SNOOPSharedReqBusEvent::GetS) {
        DRAMReqEvent = SNOOPSharedCtrlEvent::DRAMGetS;
      }
      else if (busReqEvent == SNOOPSharedReqBusEvent::Null) {
        DRAMReqEvent = SNOOPSharedCtrlEvent::Null;
      }
      else if (busReqEvent == SNOOPSharedReqBusEvent::Upg || 
              busReqEvent == SNOOPSharedReqBusEvent::OwnerPutM ||
              busReqEvent == SNOOPSharedReqBusEvent::OTherPutM ||
              busReqEvent == SNOOPSharedReqBusEvent::PutS ) {
        std::cout << "SharedMem: [Error] invalid Transaction for DRAM Request" << std::endl;
        exit(0);
      }
      else {
        std::cout << "SharedMem: [Error] uncovered ReqBus Event" << std::endl;
        exit(0);
      }
      return DRAMReqEvent;
    }
     
/**** Original Code ****/

    SNOOPSharedReqBusEvent SharedCacheCtrl::ChkBusRxReqEvent  (BusIfFIFO::BusReqMsg &  busReqMsg) {  
      if(m_reza_log_shared) cout<<" In  SharedCacheCtrl::ChkBusRxReqEvent "<<endl;
      if (!m_busIfFIFO->m_rxMsgFIFO.IsEmpty()) {

        SNOOPSharedReqBusEvent reqBusEvent;
        busReqMsg = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();

        uint16_t wbCoreId   = busReqMsg.wbCoreId;
        uint32_t  busReqCacheWayIdx;
        uint16_t ownerCoreId;
        GenericCache::CacheLineInfo CacheLineInfo;
        VictimCache ::CacheLineInfo VictimCacheLineInfo;
        
        CacheLineInfo  = m_cache->GetCacheLineInfo(busReqMsg.addr);
        VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(busReqMsg.addr); 
        
        if (CacheLineInfo.IsValid) {
          busReqCacheWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;             
          ownerCoreId = m_ownerCoreId[busReqCacheWayIdx];
        }
        else {
          ownerCoreId = m_victimOwnerCoreId[VictimCacheLineInfo.line_idx];
        }
          
        switch (busReqMsg.cohrMsgId) {
          case SNOOPPrivCohTrans::GetSTrans:
            reqBusEvent = SNOOPSharedReqBusEvent::GetS;
            break;
          case SNOOPPrivCohTrans::GetMTrans:
            reqBusEvent = SNOOPSharedReqBusEvent::GetM;
            break;
          case SNOOPPrivCohTrans::UpgTrans :
            reqBusEvent = SNOOPSharedReqBusEvent::Upg; 
            break;
          case SNOOPPrivCohTrans::PutMTrans:
            reqBusEvent = (wbCoreId == ownerCoreId) ? 
                          SNOOPSharedReqBusEvent::OwnerPutM : 
                          SNOOPSharedReqBusEvent::OTherPutM ;
            break;
          case SNOOPPrivCohTrans::PutSTrans:
            reqBusEvent = SNOOPSharedReqBusEvent::PutS;
            break;
          default: // Invalid Transaction
            if (m_logFileGenEnable) {
              std::cout << "SharedMem: [Error] invalid Transaction detected on the Bus" << std::endl;
            }
            exit(0);
          }
        return reqBusEvent;
      }
      else {// CoreNullEvent
        return SNOOPSharedReqBusEvent::Null;
      }
    }



    SNOOPSharedRespBusEvent SharedCacheCtrl::ChkBusRxRespEvent (BusIfFIFO::BusRespMsg & busRespMsg, BusIfFIFO::BusRespMsg busRespMsg_temp) { 
      
      if(!m_duetto) {
        if(m_reza_log_shared) cout<<"No Duetto enabled"<<endl;
        // here I implement the standalone FCFS and RT
        if(m_mode == "FCFS") {
          if(m_reza_log_shared) cout<<"In FCFS ChkBusRxRespEvent "<<endl;
          if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
            busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
            if(m_txexist){
              if(busRespMsg.timestamp < busRespMsg_temp.timestamp){
                uint16_t respCoreId   = busRespMsg.respCoreId;
                if(respCoreId == m_coreId) {
                  return SNOOPSharedRespBusEvent::OWnDataResp;
                }
                else  
                  return SNOOPSharedRespBusEvent::OTherDataResp;
              }
              else {
                if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {        
                  m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
                  m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);              
                  if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                    m_sharedCacheBusy = true;
                    m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
                  }                  
                }
                return SNOOPSharedRespBusEvent::NUll;
              }
            }
            else {
              uint16_t respCoreId   = busRespMsg.respCoreId;
              if(respCoreId == m_coreId) {
                return SNOOPSharedRespBusEvent::OWnDataResp;
              }
              else  
                return SNOOPSharedRespBusEvent::OTherDataResp;
            }           
          }
          else if (m_txexist) {
            if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {        
              m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
              m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);              
              if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                m_sharedCacheBusy = true;
                m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
              }                  
            }
            return SNOOPSharedRespBusEvent::NUll;
          }
          else {
            return SNOOPSharedRespBusEvent::NUll;
          }
        }
        else if (m_mode == "RT") {
          if(m_reza_log_shared) cout<<"In RT ChkBusRxRespEvent "<<endl;
          if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
            //if(m_reza_log_shared) cout<<"Check there is a response coming to the cache: Is not empty "<<endl;
            for(unsigned int RR_iterator=0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size(); RR_iterator++) { 
            //cout<<"1"<<endl;         
            m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);	

            for(int iteratori=0; iteratori < m_busIfFIFO->m_rxRespFIFO.GetQueueSize(); iteratori++) { 
              busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
              // if(m_reza_log_shared) cout<<"In RespShared Cache the Order is "<<m_reqCoreCnt<<"  recCoreID is "<<busRespMsg.reqCoreId<<" and respCoreID is "<<busRespMsg.respCoreId
              //         <<"  agent is  "<<busRespMsg.sharedCacheAgent<<" msgID  "<<busRespMsg.msgId<<" address  "<<busRespMsg.addr<<endl;     
              if(busRespMsg.reqCoreId < 10 && busRespMsg.respCoreId < 10){
                if(busRespMsg.reqCoreId == m_reqCoreCnt && sameCacheLineRX(busRespMsg)) {
                //if(busRespMsg.reqCoreId == m_reqCoreCnt) {
                  int temp =  busRespMsg.reqCoreId;
                  busRespMsg.reqCoreId = busRespMsg.respCoreId;
                  busRespMsg.respCoreId = temp;
                  // if(m_reza_log_shared) cout<<"222 In RespShared Cache the Order is "<<m_reqCoreCnt<<"  recCoreID is "<<busRespMsg.reqCoreId<<" and respCoreID is "<<busRespMsg.respCoreId<<endl;
                  return SNOOPSharedRespBusEvent::OTherDataResp;
                }
                else {
                  //cout<<"2"<<endl;
                  m_busIfFIFO->m_rxRespFIFO.PopElement();
                  m_busIfFIFO->m_rxRespFIFO.InsertElement(busRespMsg);
                }  
              }
              else {
                //cout<<"3"<<endl;
                if(busRespMsg.respCoreId == m_reqCoreCnt && sameCacheLineRX(busRespMsg)) {
                //if(busRespMsg.respCoreId == m_reqCoreCnt) {
                  //cout<<"4"<<endl;
                  uint16_t respCoreId   = busRespMsg.respCoreId;
                  return (respCoreId == m_coreId) ? SNOOPSharedRespBusEvent::OWnDataResp :
                                                    SNOOPSharedRespBusEvent::OTherDataResp;
                }
                else {
                  //cout<<"5"<<endl;
                  m_busIfFIFO->m_rxRespFIFO.PopElement();
                  m_busIfFIFO->m_rxRespFIFO.InsertElement(busRespMsg);
                }  
              }              
            }                             
          }
          busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
          uint16_t respCoreId   = busRespMsg.respCoreId;
          return (respCoreId == m_coreId) ? SNOOPSharedRespBusEvent::OWnDataResp :
                                            SNOOPSharedRespBusEvent::OTherDataResp;
        }
        else {
          return SNOOPSharedRespBusEvent::NUll;
        }
       }
      }
      else {
        
      /**** Logic to determine the arbiter****/ 
        if((m_duetto) && (m_GlobalQueue->bank_mode == "HP")) {
          if(m_reza_log_shared) cout<<"In Duetto FCFS ChkBusRxRespEvent "<<endl;
          if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
            busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
            if(m_txexist){
              if(busRespMsg.timestamp < busRespMsg_temp.timestamp){
                uint16_t respCoreId   = busRespMsg.respCoreId;
                if(respCoreId == m_coreId) {
                  return SNOOPSharedRespBusEvent::OWnDataResp;
                }
                else  
                  return SNOOPSharedRespBusEvent::OTherDataResp;
              }
              else {
                if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {        
                  m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
                  m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);              
                  if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                    m_sharedCacheBusy = true;
                    m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
                  }                  
                }
                return SNOOPSharedRespBusEvent::NUll;
              }
            }
            else {
              uint16_t respCoreId   = busRespMsg.respCoreId;
              if(respCoreId == m_coreId) {
                return SNOOPSharedRespBusEvent::OWnDataResp;
              }
              else  
                return SNOOPSharedRespBusEvent::OTherDataResp;
            }           
          }
          else if (m_txexist) {
            if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {        
              m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();

              bool terminateii = false;
              for(int ii=0; ii<m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii++){
                BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
                m_GlobalQueue->m_MsgType.PopElement();
                if(tempS.msgId == busRespMsg_temp.msgId && tempS.addr == busRespMsg_temp.addr) {
                  if(m_reza_log_shared) cout<<"Bank is done and push to tx buffer"<<endl;
                  tempS.currStage = "BANK";
                  terminateii = true;
                  m_GlobalQueue->m_MsgType.InsertElement(tempS);
                }
                else{
                  m_GlobalQueue->m_MsgType.InsertElement(tempS);
                }
              }               
              m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);              
              if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                m_sharedCacheBusy = true;
                m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
              }                  
            }
            return SNOOPSharedRespBusEvent::NUll;
          }
          else {
            return SNOOPSharedRespBusEvent::NUll;
          }
        }
        else if((m_duetto) && (m_GlobalQueue->bank_mode == "RT")) {
          if(m_reza_log_shared) cout<<"In Duetto RT ChkBusRxRespEvent "<<endl;
          if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
            // if(m_reza_log_shared) cout<<"Check there is a response coming to the cache: Is not empty "<<endl;
            for(unsigned int RR_iterator=0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size(); RR_iterator++) { 
              //cout<<"1"<<endl;         
              m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);	

              for(int iteratori=0; iteratori < m_busIfFIFO->m_rxRespFIFO.GetQueueSize(); iteratori++) { 
                busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
                // if(m_reza_log_shared) cout<<"In RespShared Cache the Order is "<<m_reqCoreCnt<<"  recCoreID is "<<busRespMsg.reqCoreId<<" and respCoreID is "<<busRespMsg.respCoreId
                //         <<"  agent is  "<<busRespMsg.sharedCacheAgent<<" msgID  "<<busRespMsg.msgId<<" address  "<<busRespMsg.addr<<endl;     
                if(busRespMsg.reqCoreId < 10 && busRespMsg.respCoreId < 10){
                  if(busRespMsg.reqCoreId == m_reqCoreCnt && sameCacheLineRX(busRespMsg)) {
                  //if(busRespMsg.reqCoreId == m_reqCoreCnt) {
                    int temp =  busRespMsg.reqCoreId;
                    busRespMsg.reqCoreId = busRespMsg.respCoreId;
                    busRespMsg.respCoreId = temp;
                    // if(m_reza_log_shared) cout<<"222 In RespShared Cache the Order is "<<m_reqCoreCnt<<"  recCoreID is "<<busRespMsg.reqCoreId<<" and respCoreID is "<<busRespMsg.respCoreId<<endl;
                    return SNOOPSharedRespBusEvent::OTherDataResp;
                  }
                  else {
                    //cout<<"2"<<endl;
                    m_busIfFIFO->m_rxRespFIFO.PopElement();
                    m_busIfFIFO->m_rxRespFIFO.InsertElement(busRespMsg);
                  }  
                }
                else {
                  //cout<<"3"<<endl;
                  if(busRespMsg.respCoreId == m_reqCoreCnt && sameCacheLineRX(busRespMsg)) {
                  //if(busRespMsg.respCoreId == m_reqCoreCnt) {
                    //cout<<"4"<<endl;
                    uint16_t respCoreId   = busRespMsg.respCoreId;
                    return (respCoreId == m_coreId) ? SNOOPSharedRespBusEvent::OWnDataResp :
                                                      SNOOPSharedRespBusEvent::OTherDataResp;
                  }
                  else {
                    //cout<<"5"<<endl;
                    m_busIfFIFO->m_rxRespFIFO.PopElement();
                    m_busIfFIFO->m_rxRespFIFO.InsertElement(busRespMsg);
                  }  
                }              
              }                             
            }
            busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
            uint16_t respCoreId   = busRespMsg.respCoreId;
            return (respCoreId == m_coreId) ? SNOOPSharedRespBusEvent::OWnDataResp :
                                              SNOOPSharedRespBusEvent::OTherDataResp;
          }
          else {
            return SNOOPSharedRespBusEvent::NUll;
          }
        } 
    
        if(m_reza_log_shared) cout<<"Here shared need to be implemented for duetto mode"<<endl;
        abort();
        return SNOOPSharedRespBusEvent::NUll;
      }
      if(m_reza_log_shared) cout<<"Should not be here"<<endl;
      abort();
      return SNOOPSharedRespBusEvent::NUll;
     }

    void SharedCacheCtrl::UpdateSharedCache   (CacheField field, 
                                            uint64_t addr, 
                                            int state, 
                                            uint8_t * data,
                                            bool UpdateAccessCnt = false) {

    GenericCacheFrmt cacheLine;
    uint32_t LineWayIdx;
    if (FetchLine(addr, cacheLine, LineWayIdx)) {
    
      if (UpdateAccessCnt) {
        cacheLine.accessCounter = cacheLine.accessCounter + 1;
        cacheLine.accessCycle   = m_cacheCycle;
      }
      
      if (field == State) {
        cacheLine.state  = state;
        m_cache->WriteCacheLine(cacheLine, LineWayIdx);
      }
      else if (field == Line) {
        cacheLine.state  = state;
        cacheLine.tag    = m_cache->CpuAddrMap (addr).tag;
        for (int i = 0; i < 8; i++)
          cacheLine.data[i] = data[i];

        m_cache->WriteCacheLine(cacheLine, LineWayIdx);
      }
    }
    else {
      if (m_logFileGenEnable) {
        std::cout << "SharedMem: [Warning] Cannot find the block in shared cache!" << std::endl;
      }
      exit(0);
    }
  }

     bool SharedCacheCtrl::FetchLine (uint64_t addr, GenericCacheFrmt  & cacheLine, uint32_t & LineWayIdx) {
       GenericCacheMapFrmt addrMap   = m_cache->CpuAddrMap (addr);
       uint32_t setIdx = addrMap.idx_set;
       uint32_t nWays  = m_cache -> GetCacheNways();
       uint32_t setOfst = setIdx * nWays;

       for (uint32_t wayIdx = setOfst; wayIdx < setOfst+nWays;wayIdx++) {
         cacheLine    = m_cache->ReadCacheLine(wayIdx);
         LineWayIdx   = wayIdx;
         if (cacheLine.valid == true && cacheLine.tag == addrMap.tag) {
           return true;
         }
       }
       return false;
     }

     // execute write back or sent command
     bool SharedCacheCtrl::DoWriteBack (uint64_t addr, uint16_t wbCoreId, uint64_t msgId, double timestamp, SendDataType type = DataOnly) {      
       if(m_reza_log_shared) cout<<"in DoWriteBack"<<endl;
       BusIfFIFO::BusReqMsg tempBusReqMsg;
       BusIfFIFO::BusRespMsg  wbMsg;
       
       tempBusReqMsg.msgId        = msgId;
       tempBusReqMsg.reqCoreId    = wbCoreId;
       tempBusReqMsg.wbCoreId     = m_coreId;
       tempBusReqMsg.cohrMsgId    = SNOOPPrivCohTrans::ExclTrans;
       tempBusReqMsg.addr         = addr;
       tempBusReqMsg.timestamp    = timestamp;
       tempBusReqMsg.cycle        = m_cacheCycle;
       tempBusReqMsg.cohrMsgId    = (type == CoreInv) ? SNOOPPrivCohTrans::InvTrans : SNOOPPrivCohTrans::ExclTrans;
       tempBusReqMsg.sharedCacheAgent = m_coreId;
       bool TxReqMsgInsertFlag    = false;
       bool TxRespMsgInsertFlag   = false;
       
       
       if (type == CoreInv || type == DataPlsExcl || type == ExclOnly) {
         if (!m_busIfFIFO->m_txMsgFIFO.IsFull()) {
           m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
           TxReqMsgInsertFlag = true;
         }
       }
       else {
         TxReqMsgInsertFlag = true;
       }
       if (type == DataOnly || type == DataPlsExcl) {
         wbMsg.reqCoreId    = wbCoreId;
         wbMsg.respCoreId   = m_coreId;
         wbMsg.addr         = addr;
         wbMsg.msgId        = msgId;
         wbMsg.timestamp    = timestamp;
         wbMsg.dualTrans    = false;
         wbMsg.cycle        = m_cacheCycle;
         wbMsg.sharedCacheAgent = m_coreId;
         
         for (int i = 0; i < 8; i++)
           wbMsg.data[i] = 0;

         //if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {
         //m_busIfFIFO->m_txRespFIFO.InsertElement(wbMsg);       
         if (!m_busIfFIFO->m_localPendingRespTxBuffer.IsFull()) {
           if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() == 0){
             m_sharedCacheBusy = true;
             m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;
           }
           m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(wbMsg);
           TxRespMsgInsertFlag = true;
         }
         //}
         
         if (m_logFileGenEnable) {
           std::cout << "Sent data to core = " << wbMsg.reqCoreId << ", addr " << addr << std::endl;
         }
         
       }
       else {
         TxRespMsgInsertFlag = true;
       }
       return (TxReqMsgInsertFlag && TxRespMsgInsertFlag);    
     }

     // push to pending WB buffer  
     bool SharedCacheCtrl::PushMsgInBusTxFIFO  (uint64_t       msgId, 
                                                uint16_t       reqCoreId, 
                                                uint16_t       wbCoreId, 
                                                uint64_t       addr) {
       if (!m_PndWBFIFO.IsFull()) {
         BusIfFIFO::BusReqMsg tempBusReqMsg;
         tempBusReqMsg.msgId        = msgId;
         tempBusReqMsg.reqCoreId    = reqCoreId;
         tempBusReqMsg.wbCoreId     = wbCoreId;
         tempBusReqMsg.addr         = addr;
         tempBusReqMsg.timestamp    = m_cacheCycle*m_dt;
         // push message into BusTxMsg FIFO
         m_PndWBFIFO.InsertElement(tempBusReqMsg);
         return true;
       }
       else {
         if (m_logFileGenEnable) {
           std::cout << "SharedMem: [Warning] Cannot insert the Msg in BusTxMsg FIFO, FIFO is Full!" << std::endl;
         }
         return false;
       }
     }

    // send memory request to DRAM
    bool SharedCacheCtrl::SendDRAMReq (uint64_t msgId, uint64_t addr, DRAMIfFIFO::DRAM_REQ type) {
      DRAMIfFIFO::DRAMReqMsg dramReqMsg;
      dramReqMsg.msgId       = msgId;
      dramReqMsg.addr        = addr;
      dramReqMsg.type        = type;
      dramReqMsg.reqAgentId  = m_coreId;
      dramReqMsg.cycle       = m_cacheCycle;

      bool InsertFlag = false;
      if (!m_dramBusIfFIFO->m_txReqFIFO.IsFull()) {
        m_dramBusIfFIFO->m_txReqFIFO.InsertElement(dramReqMsg);
        InsertFlag = true;
      }
      return InsertFlag;
    }
    
    // Send the Data from the PendingWB buffer to the Bus interface
    void SharedCacheCtrl::SendPendingReqData  (GenericCacheMapFrmt recvTrans ) {
      if(m_reza_log_shared) cout<<"In sendPendingData"<<endl;
       if (!m_PndWBFIFO.IsEmpty()) {
          BusIfFIFO::BusReqMsg pendingWbMsg ;
          GenericCacheMapFrmt  pendingWbAddrMap;
          int pendingQueueSize = m_PndWBFIFO.GetQueueSize();
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingWbMsg = m_PndWBFIFO.GetFrontElement();
            pendingWbAddrMap = m_cache->CpuAddrMap (pendingWbMsg.addr);
            m_PndWBFIFO.PopElement();
            if (recvTrans.idx_set == pendingWbAddrMap.idx_set &&
                recvTrans.tag == pendingWbAddrMap.tag) {
              if (!DoWriteBack (pendingWbMsg.addr,pendingWbMsg.reqCoreId,pendingWbMsg.msgId,pendingWbMsg.timestamp)) {
                if (m_logFileGenEnable){
                  std::cout << "111SharedMem: Cannot Send Data On the Bus !!!!" << std::endl;
                }
                exit(0);
              }
            }
            else {
              m_PndWBFIFO.InsertElement(pendingWbMsg);
            }
          }
        } 
     }  
      
     void SharedCacheCtrl::VictimCacheLineEvict (uint32_t victimWayIdx) {
       uint64_t              ReplcAddr;
       GenericCacheFrmt      ReplcCacheLine ;
       SNOOPSharedOwnerState CacheLineOwner;
       
       ReplcCacheLine = m_victimCache->ReadCacheLine(victimWayIdx);
       ReplcAddr      = ReplcCacheLine.tag;                     
                   
       if (ReplcCacheLine.valid) { 
         m_CurrEventList.CtrlEvent = SNOOPSharedCtrlEvent::Replacement;
         CacheLineOwner = (m_victimOwnerCoreId[victimWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore ;

         CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                 
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInv) { 
           if (!DoWriteBack (ReplcAddr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, CoreInv)) {
             std::cout << "SharedMem: Cannot send Inv() on request Bus !!!!" << std::endl;
             exit(0);
           }
         }
                     
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueDRAMWrite) {
           if (!SendDRAMReq (m_CurrEventMsg.busReqMsg.msgId, ReplcAddr, DRAMIfFIFO::DRAM_REQ::DRAM_WRITE)) {
             std::cout << "SharedMem: Error: Cannot Direct Write Request to DRAM " << std::endl;
             exit(0);
           }
         }
            
         ReplcCacheLine.evicted   = false;
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueDRAMWrite) {
           ReplcCacheLine.evicted   = true;
         }
         ReplcCacheLine.state   = m_CurrEventNextState.CtrlEventState;
         m_victimCache->WriteCacheLine(victimWayIdx, ReplcCacheLine);    
         
         if (m_CurrEventOwnerNextState.CtrlOwnerState == SNOOPSharedOwnerState::SharedMem) {
            m_victimOwnerCoreId[victimWayIdx] = m_coreId;
         }          
       } // if (!ReplcCacheLine.valid) { 
     }
     

     void SharedCacheCtrl::SendExclRespEarly () {
       bool OutStandingExclFlag = false;  // default
       int ExclQueueSize;
       uint64_t OutstandingExclAddr;
       if ((m_cohrProt == CohProtType::SNOOP_MESI || 
            m_cohrProt == CohProtType::SNOOP_MOESI)) {
                   
         if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS ||
             m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
           ExclQueueSize = m_DRAMOutStandingExclMsg.GetQueueSize();
           for (int j = 0; j < ExclQueueSize; j++ ) {
             OutstandingExclAddr = m_DRAMOutStandingExclMsg.GetFrontElement();
             m_DRAMOutStandingExclMsg.PopElement();
             if ((OutstandingExclAddr >> (int) log2(m_cacheBlkSize))  == 
                 (m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)))  {
               OutStandingExclFlag = true;
             }
             m_DRAMOutStandingExclMsg.InsertElement(OutstandingExclAddr);  
           }
                   
           if (OutStandingExclFlag == false) {
             m_DRAMOutStandingExclMsg.InsertElement(m_CurrEventMsg.busReqMsg.addr);
             if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS) {
               if (!DoWriteBack (m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, ExclOnly)) {
                 std::cout << "SharedMem: Cannot Send Exclusive Msg on the bus !!!!" << std::endl;
                 exit(0);
               }
             }
           }       
         }
       }
     }
          
     bool SharedCacheCtrl::RemoveExclRespAddr () {
          int ExclQueueSize = m_DRAMOutStandingExclMsg.GetQueueSize();
          bool OutStandingExclFlag = false;  // default
          uint64_t OutstandingExclAddr;
          for (int j = 0; j < ExclQueueSize; j++ ) {
            OutstandingExclAddr = m_DRAMOutStandingExclMsg.GetFrontElement();
            m_DRAMOutStandingExclMsg.PopElement();
            if ((OutstandingExclAddr >> (int) log2(m_cacheBlkSize))  == 
                (m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)) && OutStandingExclFlag == false)  {
              OutStandingExclFlag = true;
            }
            else {
              m_DRAMOutStandingExclMsg.InsertElement(OutstandingExclAddr);  // dequeue
            }
          }
          return OutStandingExclFlag;
     }
                               
     
     
    bool SharedCacheCtrl::isOldest(uint64_t adr, unsigned int coreIndex) {
      bool success = false;
      BusIfFIFO::BusReqMsg tempOldestReqMsg;
      int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
      for(int itr = 0; itr < queueSize; itr ++) {
        tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();       
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        if(tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex) {
          success = true;    
          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);                                     
        }
        else{
          m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);
        }                
      }
      return success;
    }


    bool SharedCacheCtrl::sameCacheLineRX(BusIfFIFO::BusRespMsg busRespMsgLineCheck) {
      bool sameRX = true;
      BusIfFIFO::BusRespMsg checkRespLineInBuffer;
      GenericCacheMapFrmt CacheLineInfoCheck, CacheLineInfoCheckInBuffer; 
      CacheLineInfoCheck = m_cache->CpuAddrMap (busRespMsgLineCheck.addr); 
      if(!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
        for(int iteratori_1=0; iteratori_1 < m_busIfFIFO->m_rxRespFIFO.GetQueueSize() && sameRX == true; iteratori_1++) { 
         checkRespLineInBuffer = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();     
         m_busIfFIFO->m_rxRespFIFO.PopElement();
         if(checkRespLineInBuffer.msgId != busRespMsgLineCheck.msgId){
          CacheLineInfoCheckInBuffer = m_cache->CpuAddrMap (checkRespLineInBuffer.addr);
          if(CacheLineInfoCheck.idx_set == CacheLineInfoCheckInBuffer.idx_set && checkRespLineInBuffer.timestamp < busRespMsgLineCheck.timestamp) {
            sameRX = false;
            //abort();
            m_busIfFIFO->m_rxRespFIFO.InsertElement(checkRespLineInBuffer);
          }
          else {
            m_busIfFIFO->m_rxRespFIFO.InsertElement(checkRespLineInBuffer);
          }
         }
         else{
           m_busIfFIFO->m_rxRespFIFO.InsertElement(checkRespLineInBuffer);
         }
        }
      }       
      //if(m_reza_log_shared) cout<<"Same Cache Line RX 9  "<<sameRX<<endl;
      return sameRX;
    }

    bool SharedCacheCtrl::sameCacheLineTX(BusIfFIFO::BusRespMsg busRespMsgLineCheck) {
      //if(m_reza_log_shared) cout<<"Same Cache Line TX"<<endl;
      bool sameTX = true;
      BusIfFIFO::BusRespMsg checkRespLineInBuffer;
      GenericCacheMapFrmt CacheLineInfoCheck, CacheLineInfoCheckInBuffer; 
      CacheLineInfoCheck = m_cache->CpuAddrMap (busRespMsgLineCheck.addr); 
      //if(m_reza_log_shared) cout<<"Same Cache Line TX 1"<<endl;
      if(!m_busIfFIFO->m_localPendingRespTxBuffer.IsEmpty()){
        for(int iteratori_2=0; iteratori_2 < m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() && sameTX == true; iteratori_2++) { 
         //if(m_reza_log_shared) cout<<"Same Cache Line TX 2"<<endl;
         checkRespLineInBuffer = m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement();        
         //if(m_reza_log_shared) cout<<"Same Cache Line TX 3"<<endl;
         m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
         //if(m_reza_log_shared) cout<<"Same Cache Line TX 4"<<endl;
         if(checkRespLineInBuffer.msgId != busRespMsgLineCheck.msgId){
          //if(m_reza_log_shared) cout<<"Same Cache Line TX 5"<<endl;
          CacheLineInfoCheckInBuffer = m_cache->CpuAddrMap (checkRespLineInBuffer.addr);
          //if(m_reza_log_shared) cout<<"Same Cache Line TX 6"<<endl;
          if(CacheLineInfoCheck.idx_set == CacheLineInfoCheckInBuffer.idx_set && checkRespLineInBuffer.timestamp < busRespMsgLineCheck.timestamp) {
            //if(m_reza_log_shared) cout<<"Same Cache Line TX 7"<<endl;
            sameTX = false;
            //abort();
            m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(checkRespLineInBuffer);
          }
          else {
            //if(m_reza_log_shared) cout<<"Same Cache Line TX 8"<<endl;
            m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(checkRespLineInBuffer);
          }
        }
        else {
          m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(checkRespLineInBuffer);
        }
       }
      }       
       //if(m_reza_log_shared) cout<<"Same Cache Line TX 9  "<<sameTX<<endl;
       return sameTX;
    }

    bool SharedCacheCtrl::removeFromM_Type(uint64_t adr, unsigned int coreIndex) {
      
      // if(m_reza_log_shared) cout<<"in removeFromM_Type "<<endl;
      bool success = false;
      BusIfFIFO::BusReqMsg tempOldestReqMsg_removeFromM_Type;
      int queueSize = m_GlobalQueue->m_MsgType.GetQueueSize();
      // cout<<"the size of removeFromM_Type is Shared Bank "<<queueSize<<endl;
      for(int itr = 0; itr < queueSize; itr ++) {
        tempOldestReqMsg_removeFromM_Type = m_GlobalQueue->m_MsgType.GetFrontElement();
        // if(m_reza_log_shared) cout<<"reqID "<<tempOldestReqMsg_removeFromM_Type.reqCoreId<<" wbID "<<tempOldestReqMsg_removeFromM_Type.wbCoreId<<" mgID "<<tempOldestReqMsg_removeFromM_Type.msgId<<endl;
        m_GlobalQueue->m_MsgType.PopElement();
        if(tempOldestReqMsg_removeFromM_Type.addr == adr && tempOldestReqMsg_removeFromM_Type.reqCoreId == coreIndex) {                               
          // if(m_reza_log_shared) cout<<"removeFromOldest done"<<endl;
          success = true;
          // cout<<"the size of removeFromM_Type removeFromM_Type is Shared Bank after deletion is  "<<m_GlobalQueue->m_MsgType.GetQueueSize()<<endl;
          return true;                                                   
        }
        m_GlobalQueue->m_MsgType.InsertElement(tempOldestReqMsg_removeFromM_Type);        
      }
      return success;
    }
    bool SharedCacheCtrl::removeFromOldest(uint64_t adr, unsigned int coreIndex) {
      // if(m_reza_log_shared) cout<<"in removeOldest "<<endl;
      bool success = false;
      BusIfFIFO::BusReqMsg tempOldestReqMsg;
      int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
      for(int itr = 0; itr < queueSize; itr ++) {
        tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
        // if(m_reza_log_shared) cout<<"reqID "<<tempOldestReqMsg.reqCoreId<<" wbID "<<tempOldestReqMsg.wbCoreId<<" mgID "<<tempOldestReqMsg.msgId<<endl;
        m_GlobalQueue->m_GlobalOldestQueue.PopElement();
        if(tempOldestReqMsg.addr == adr && tempOldestReqMsg.reqCoreId == coreIndex) {          
         // if(m_reza_log_shared) {
            if(m_wcShared < m_cacheCycle - tempOldestReqMsg.becameOldest + m_sharedcachelatency)
              m_wcShared = m_cacheCycle - tempOldestReqMsg.becameOldest + m_sharedcachelatency;
             cout<<"The Oldest MsgID "<<tempOldestReqMsg.msgId<<" From "<<coreIndex<<" Terminated @ "<<m_cacheCycle<<" Arrival @ "<< tempOldestReqMsg.becameOldest <<" In the Bank: "<<m_coreId<<" Latency: "<<
                m_cacheCycle - tempOldestReqMsg.becameOldest + m_sharedcachelatency<<" The WC is "<<m_wcShared <<endl; 
          //}
            
          // if(m_reza_log_shared) cout<<"removeFromOldest done"<<endl;
          success = true;
          return true;   
                                                
        }
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);        
      }
      return success;
    }

    void SharedCacheCtrl::adjustOldest(unsigned int coreIndex) {
      if(m_reza_log_shared) cout<<"****************************************** Adjust Oldest ************************************************* "<<coreIndex <<"  mcoreID "<<m_coreId<<endl;      
      double arrivalTime        = 0;
      bool PendingTxReq         = false;
      unsigned int queueSize    = 0;
      bool tem = false;

      BusIfFIFO::BusReqMsg tempFCFSMsg, tempCandidate;      

      std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFOPrivate.begin();
      std::advance(it1, coreIndex);
      
      // First take the oldest REQ from IF FIFO request buffer
      if ((*it1)->m_txMsgFIFO.IsEmpty() == false) {        
        tempFCFSMsg = (*it1)->m_txMsgFIFO.GetFrontElement();      
        arrivalTime = tempFCFSMsg.timestamp;
        PendingTxReq = true;
        tempCandidate = tempFCFSMsg;

        if(m_reza_log_shared) cout<<" Requestor  "<< coreIndex<<"  Buffer not empty address  "<<tempCandidate.addr<<" arrival is "<<arrivalTime<<" agent bank "<< tempCandidate.sharedCacheAgent <<endl;
      }
     
      // Second parse the service queue for already was sent into the banks
      queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
      for(unsigned int itr = 0; itr < queueSize; itr++) {       
        if(m_GlobalQueue->m_GlobalReqFIFO.IsEmpty() == false)  {                   
          tempFCFSMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();                  
          m_GlobalQueue->m_GlobalReqFIFO.PopElement();  
          if(m_reza_log_shared) cout<<"In global queue reqID  "<<tempFCFSMsg.reqCoreId<<" wbID "<<tempFCFSMsg.wbCoreId<<" address "<<tempFCFSMsg.addr<<"  arrival "<<tempFCFSMsg.timestamp<<" coreIndex "<<coreIndex<<" arrival "<<tempFCFSMsg.timestamp<<endl;       
          if((tempFCFSMsg.msgId !=0 && tempFCFSMsg.reqCoreId == coreIndex) || (tempFCFSMsg.msgId ==0 && tempFCFSMsg.wbCoreId == coreIndex)) {                                 
            if ((PendingTxReq == false) || ((PendingTxReq == true) && (arrivalTime > tempFCFSMsg.timestamp))){   
              PendingTxReq = true;                          
              arrivalTime = tempFCFSMsg.timestamp;
              tempCandidate = tempFCFSMsg;    
              tem = true;      
            }
          }
          m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempFCFSMsg);   
        }        
      }

      if(m_reza_log_shared){    
        for(int it2 = 0; it2 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize(); it2++) {       
          BusIfFIFO::BusReqMsg xxx;
          xxx = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();  
          m_GlobalQueue->m_GlobalOldestQueue.PopElement(); 
          if(m_reza_log_shared) cout<<"The address msg in the oldest buffer at "<<it2<<" address is  "<<xxx.addr<<" reqCoreID "<<xxx.reqCoreId<<endl;          
            m_GlobalQueue->m_GlobalOldestQueue.InsertElement(xxx);
        }      
      }


    


      // now it should move to the oldest queue
      if(PendingTxReq == true){
        if(m_reza_log_shared) cout<<"Adding to the queue addr "<<tempCandidate.addr<<"  reqCoreID  "<<tempCandidate.reqCoreId<<endl; 
        tempCandidate.becameOldest = m_cacheCycle;        
        m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempCandidate);
        m_GlobalQueue->m_GlobalRRQueue.push_back(coreIndex);
        
        if(tem) {
          BusIfFIFO::BusReqMsg tempitr8;
          for(int itr7= 0; itr7 < m_GlobalQueue->m_MsgType.GetQueueSize(); itr7++){
            tempitr8 = m_GlobalQueue->m_MsgType.GetFrontElement();
            m_GlobalQueue->m_MsgType.PopElement();
            if(tempitr8.msgId == tempCandidate.msgId && tempitr8.addr == tempCandidate.addr){
              tempitr8.becameOldest = m_cacheCycle;
              assignDeadlineAfterDetermination(tempitr8);  
              m_GlobalQueue->m_MsgType.InsertElement(tempitr8);
            }
            else {
              m_GlobalQueue->m_MsgType.InsertElement(tempitr8);
            }            
          }        
        }       
      }
    }
   
    bool SharedCacheCtrl::removeFromNonOldest(uint64_t adr, unsigned int coreIndex)  {
      if(m_reza_log_shared) cout<<"in remove Non Oldest "<<endl;
      bool success = false;
      BusIfFIFO::BusReqMsg tempNonOldestReqMsg;
      int queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
      for(int itr = 0; itr < queueSize; itr ++) {
        tempNonOldestReqMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
        m_GlobalQueue->m_GlobalReqFIFO.PopElement();
        if(m_reza_log_shared) cout<<"reqID "<<tempNonOldestReqMsg.reqCoreId<<" wbID "<<tempNonOldestReqMsg.wbCoreId<<" mgID "<<tempNonOldestReqMsg.msgId<<endl;
        if(tempNonOldestReqMsg.addr == adr && tempNonOldestReqMsg.reqCoreId == coreIndex) {
          if(m_reza_log_shared) cout<<"Removed from Service Queue   "<<endl;
          success = true;
          return true;                                         
        }
        m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsg);        
      }
      return success;
    }
     
     
     // This function does most of the functionality.
     void SharedCacheCtrl::CacheCtrlMain () {
      if(m_reza_log_shared) cout<<"****************In SharedCacheCtrl****************** Cahce                                                                   "<<m_coreId<<" is busy for next "<<m_busIfFIFO->m_bankArbBlock<<" cycles"<<endl; 
      
      if(m_reza_log_shared) cout<<"1---------------- Content of the MSG Buffer in the Shared Cache: "<<m_busIfFIFO->m_fifo_id<<"  The size is  "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;
      if (!m_busIfFIFO->m_rxMsgFIFO.IsEmpty()) {    
        ns3::BusIfFIFO::BusReqMsg busReqMsg_temp_1;        
        for(int itr= 0 ; itr < m_busIfFIFO->m_rxMsgFIFO.GetQueueSize(); itr++) {
           busReqMsg_temp_1 = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();           
           m_busIfFIFO->m_rxMsgFIFO.PopElement();
            if(m_reza_log_shared) cout<<"Address: "<<busReqMsg_temp_1.addr<<" reqCoreID "<<busReqMsg_temp_1.reqCoreId<<"  respCoreID  "<<busReqMsg_temp_1.wbCoreId<<"  msgID is  "<<busReqMsg_temp_1.msgId<<endl;           
           m_busIfFIFO->m_rxMsgFIFO.InsertElement(busReqMsg_temp_1);
        }
      }
      if(m_reza_log_shared) cout<<"1---------------- Content of the RX RESP Buffer in the Shared Cache: "<<m_busIfFIFO->m_fifo_id<<"  The size is  "<<m_busIfFIFO->m_rxRespFIFO.GetQueueSize()<<endl;       
      if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {      
        ns3::BusIfFIFO::BusRespMsg busResp_temp_1;         
        for(int itr1= 0 ; itr1 < m_busIfFIFO->m_rxRespFIFO.GetQueueSize(); itr1++) {
           busResp_temp_1 = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();           
           m_busIfFIFO->m_rxRespFIFO.PopElement();
            if(m_reza_log_shared) cout<<"Address: "<<busResp_temp_1.addr<<" reqCoreID "<<busResp_temp_1.reqCoreId<<"  respCoreID  "<<busResp_temp_1.respCoreId<<"  msgID is  "<<busResp_temp_1.msgId<<endl;          
           m_busIfFIFO->m_rxRespFIFO.InsertElement(busResp_temp_1);
        }
      }
      if(m_reza_log_shared) cout<<"1---------------- Content of the TX RESP Buffer in the Shared Cache: "<<m_busIfFIFO->m_fifo_id<<"  The size is  "<<m_busIfFIFO->m_txRespFIFO.GetQueueSize()<<endl;       
      if (!m_busIfFIFO->m_txRespFIFO.IsEmpty()) {
        ns3::BusIfFIFO::BusRespMsg busResp_temp_1;         
        for(int itr1= 0 ; itr1 < m_busIfFIFO->m_txRespFIFO.GetQueueSize(); itr1++) {
           busResp_temp_1 = m_busIfFIFO->m_txRespFIFO.GetFrontElement();           
           m_busIfFIFO->m_txRespFIFO.PopElement();
            if(m_reza_log_shared) cout<<"Address: "<<busResp_temp_1.addr<<" reqCoreID "<<busResp_temp_1.reqCoreId<<"  respCoreID  "<<busResp_temp_1.respCoreId<<"  msgID is  "<<busResp_temp_1.msgId<<endl;           
           m_busIfFIFO->m_txRespFIFO.InsertElement(busResp_temp_1);
        }
      }
      if (!m_busIfFIFO->m_localPendingRespTxBuffer.IsEmpty()) { 
            if(m_reza_log_shared) cout<<"Size of the m_localPendingRespTxBuffer is "<<m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize()<<" front address is "<<m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement().addr<<endl;
        
      }
      // cout<<"1------------------------------------------------------------------------------Queue size is  "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;
     
     
      if(m_busIfFIFO->m_bankArbBlock > 0) m_busIfFIFO->m_bankArbBlock--;     
      if(m_busIfFIFO->m_bankArbBlock == 0) m_sharedCacheBusy = false;
       


       GenericCacheFrmt            TempCacheline, 
                                   ReplcCacheLine;
                                   
       uint32_t  SclWayIdx;
              

       GenericCache::CacheLineInfo CacheLineInfo;
       VictimCache ::CacheLineInfo VictimCacheLineInfo;
       
       SNOOPSharedOwnerState       CacheLineOwner;
       int QueueSize;
       bool NewFetchDone;
       bool ProcFromVictimCache;

       // take the ready response from the DRAM interface  
       // This part we do not care. At this point, we consider our model to be the perfect cache without DRAM interferences. 
       QueueSize = m_dramBusIfFIFO->m_rxRespFIFO.GetQueueSize();
       NewFetchDone = false; 
       DRAMIfFIFO::DRAMRespMsg dramBusRespMsg; 

       for (int i = 0; i < QueueSize; i++ ) { 
         if(m_reza_log_shared) cout<<"It came to the DRAM "<<endl;
         //exit(0);         
         dramBusRespMsg = m_dramBusIfFIFO->m_rxRespFIFO.GetFrontElement();
         m_dramBusIfFIFO->m_rxRespFIFO.PopElement();
          /***  if(dramBusRespMsg.sharedCacheAgent == " is it is equal to this specific shared cache. If not insert and take the next one. You can retrieve the shared cache by checking the address and how you map to the shared caches.") {    }       ****/
          if (NewFetchDone == false) {
            CacheLineInfo = m_cache->GetCacheLineInfo(dramBusRespMsg.addr); 
            if (CacheLineInfo.IsValid == true) { 
              NewFetchDone = true;
              m_CurrEventList.busRespEvent = SNOOPSharedRespBusEvent::DRAMDataResp;
              CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, CacheLineInfo.state, SNOOPSharedOwnerState::SharedMem);
              
              if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::SharedFault) {
                if(m_reza_log_shared) std::cout << "SharedMem: DRAM DataResp occur in illegal state!" << std::endl; 
                exit(0);
              }
              else {
                TempCacheline = m_cache->ReadCacheLine(CacheLineInfo.set_idx, CacheLineInfo.way_idx);
                TempCacheline.state = m_CurrEventNextState.busRespEventState;
                m_cache->WriteCacheLine (CacheLineInfo.set_idx, CacheLineInfo.way_idx, TempCacheline);
                
                SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;
                if (m_CurrEventOwnerNextState.busRespOwnerState == SNOOPSharedOwnerState::SharedMem) {
                  m_ownerCoreId[SclWayIdx] = m_coreId;
                }
                else {
                  std::cout << "SharedCache: DRAM DataResp illegal Owner Set!" << std::endl; 
                  exit(0);
                }      
              }
            }
            else { 
              m_dramBusIfFIFO->m_rxRespFIFO.InsertElement(dramBusRespMsg);
            }
          }
          else {
            m_dramBusIfFIFO->m_rxRespFIFO.InsertElement(dramBusRespMsg);
          }
       } // for (int i = 0; i < QueueSize; i++ ) { 
       
       BusIfFIFO::BusRespMsg busRespMsg_temp;  
       // Process responses in the local buffer to send on the bus TXRESP
       if(!m_sharedCacheBusy){ 
        m_txexist = false;
        if (!m_busIfFIFO->m_localPendingRespTxBuffer.IsEmpty()) {
          if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Check the m_localPendingRespTxBuffer path"<<endl;          
          if(m_reza_log_shared) cout<<"there is at least one response that needs to send to the cores "<<endl;
          
          if(!m_duetto && m_mode =="RT") {
            if(m_reza_log_shared) cout<<"Process TX RT"<<endl;
            for(unsigned int RR_iterator=0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && !m_sharedCacheBusy ; RR_iterator++) {          
              m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
              //cout<<"m_reqCoreCnt is "<<m_reqCoreCnt<<endl;
              for(int RR_iterator_1=0; RR_iterator_1 < m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() && !m_sharedCacheBusy ; RR_iterator_1++) {          
                busRespMsg_temp = m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement();    
                if(m_reza_log_shared) cout<<"In RespShared Cache TX local buffer the recCoreID is "<<busRespMsg_temp.reqCoreId<<" and respCoreID is "<<busRespMsg_temp.respCoreId<<endl;                          
                if(busRespMsg_temp.reqCoreId == m_reqCoreCnt) {
                  //cout<<"inside puishing to TX"<<endl;
                  // Now push it to the TX Response interface
                  if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {
                    if(sameCacheLineTX(busRespMsg_temp)) {
                      if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Pushed to the TX Resp of the shared cache"<<endl;         
                      m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();                   
                      m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);    
                      if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                        m_sharedCacheBusy = true;
                        m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
                      }
                    }
                  }
                  else{
                    if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Faild to Push to the TX Resp of the shared cache"<<endl; 
                  }
                }
                else {
                  m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
                  m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(busRespMsg_temp);
                }
              }                      
            }
          }
          else if(!m_duetto && m_mode =="HP") {
            if(m_reza_log_shared) cout<<"Process TX FCFS"<<endl;
            busRespMsg_temp = m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement();
            m_txexist = true;
          }          
          else if(m_duetto){
            if(m_GlobalQueue->bank_mode == "RT") {
              if(m_reza_log_shared) cout<<"Process TX Duetto RT"<<endl;
              for(unsigned int RR_iterator=0; RR_iterator < m_GlobalQueue->m_GlobalRRQueue.size() && !m_sharedCacheBusy ; RR_iterator++) {          
                m_reqCoreCnt = m_GlobalQueue->m_GlobalRRQueue.at(RR_iterator);
                //cout<<"m_reqCoreCnt is "<<m_reqCoreCnt<<endl;
                for(int RR_iterator_1=0; RR_iterator_1 < m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() && !m_sharedCacheBusy ; RR_iterator_1++) {          
                  busRespMsg_temp = m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement();    
                  if(m_reza_log_shared) cout<<"In RespShared Cache TX local buffer the recCoreID is "<<busRespMsg_temp.reqCoreId<<" and respCoreID is "<<busRespMsg_temp.respCoreId<<endl;                          
                  if(busRespMsg_temp.reqCoreId == m_reqCoreCnt) {
                    //cout<<"inside puishing to TX"<<endl;
                    // Now push it to the TX Response interface
                    if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {
                      if(sameCacheLineTX(busRespMsg_temp)) {
                        if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Pushed to the TX Resp of the shared cache"<<endl;         
                        m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
                        
                        bool terminatei = false;
                        for(int ii=0; ii<m_GlobalQueue->m_MsgType.GetQueueSize() && terminatei == false; ii++){
                          BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
                          m_GlobalQueue->m_MsgType.PopElement();
                          if(tempS.msgId == busRespMsg_temp.msgId && tempS.addr == busRespMsg_temp.addr) {
                            if(m_reza_log_shared) cout<<"Bank is done and push to tx buffer"<<endl;
                            tempS.currStage = "BANK";
                            terminatei = true;
                            m_GlobalQueue->m_MsgType.InsertElement(tempS);
                          }
                          else{
                            m_GlobalQueue->m_MsgType.InsertElement(tempS);
                          }
                        }
                        
                        m_busIfFIFO->m_txRespFIFO.InsertElement(busRespMsg_temp);    
                        if(m_busIfFIFO->m_localPendingRespTxBuffer.GetQueueSize() > 0) {
                          m_sharedCacheBusy = true;
                          m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;                            
                        }
                      }
                    }
                    else{
                      if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Faild to Push to the TX Resp of the shared cache"<<endl; 
                    }
                  }
                  else {
                    m_busIfFIFO->m_localPendingRespTxBuffer.PopElement();
                    m_busIfFIFO->m_localPendingRespTxBuffer.InsertElement(busRespMsg_temp);
                  }
                }                      
              }
            }
            if(m_GlobalQueue->bank_mode =="HP") {              
              if(m_reza_log_shared) cout<<"Process TX Duetto FCFS"<<endl;
              busRespMsg_temp = m_busIfFIFO->m_localPendingRespTxBuffer.GetFrontElement();
              m_txexist = true;
            }
          }
        }
      }
      
      // Process the incoming data from the bus    
      if(!m_sharedCacheBusy){
       if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Check the RX RESP path"<<endl;
       // take the response from the RX Path of the shared cache interface FIFO and then process. Either it could be in the shared cache itself or in the victim cache  
       m_CurrEventList.busRespEvent      = ChkBusRxRespEvent (m_CurrEventMsg.busRespMsg, busRespMsg_temp);  
       GenericCacheMapFrmt respAddrMap   = m_cache->CpuAddrMap (m_CurrEventMsg.busRespMsg.addr); 
       ProcFromVictimCache = false;
       
       if (m_CurrEventList.busRespEvent != SNOOPSharedRespBusEvent::NUll) {
         if(m_duetto){
            bool terminateii = false;
            for(int ii1=0; ii1<m_GlobalQueue->m_MsgType.GetQueueSize() && terminateii == false; ii1++){
              BusIfFIFO::BusReqMsg tempS = m_GlobalQueue->m_MsgType.GetFrontElement();
              m_GlobalQueue->m_MsgType.PopElement();
              if(tempS.msgId == m_CurrEventMsg.busRespMsg.msgId && tempS.addr == m_CurrEventMsg.busRespMsg.addr) {
                if(m_reza_log_shared) cout<<"bank is done for incoming data"<<endl;
                tempS.currStage = "BANK";
                terminateii = true;
                m_GlobalQueue->m_MsgType.InsertElement(tempS);
              }
              else{
                m_GlobalQueue->m_MsgType.InsertElement(tempS);
              }
            }
         }
         
         if(m_reza_log_shared) cout<<"there is somehting here actually in resp of the shared cache"<<endl;
         m_sharedCacheBusy = true;
         m_busIfFIFO->m_bankArbBlock = m_sharedcachelatency;

         CacheLineInfo       = m_cache->GetCacheLineInfo(m_CurrEventMsg.busRespMsg.addr);
         VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(m_CurrEventMsg.busRespMsg.addr); 
       
         if (CacheLineInfo.IsValid == false) {
           if (VictimCacheLineInfo.IsValid == false) { 
             std::cout << "SharedMem: Error Response message is not in L2 Cache or Victim Cache" << std::endl;
             exit(0);
           }
           else {
             ProcFromVictimCache = true;
             CacheLineOwner = (m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
             CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, VictimCacheLineInfo.state, CacheLineOwner); // process RespBus event
             
             if (m_logFileGenEnable){ // debug message
               std::cout << "SharedMem: Process L2 response message from Victim Cache, wayIdx = " << VictimCacheLineInfo.line_idx << std::endl;
             }
           }
         }
         else {
           SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;
           CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
           CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, CacheLineInfo.state, CacheLineOwner);
         }
         
         if (m_logFileGenEnable){ // debug message
           if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::SharedNullAck) {
             std::cout << "\nSharedMem: " << m_coreId << " has Msg in the RxResp Bus" << std::endl;
             std::cout << "\t\t BusEventName        = " << PrintSharedRespBusEventName(m_CurrEventList.busRespEvent) << std::endl;
             std::cout << "\t\t ReqCoreId           = " << m_CurrEventMsg.busRespMsg.reqCoreId << " RespCoreId = " << m_CurrEventMsg.busRespMsg.respCoreId 
                       << " Resp Addr  = " << m_CurrEventMsg.busRespMsg.addr << " CacheLine = " << respAddrMap.idx_set << std::endl;
             std::cout << "\t\t CacheLine CurrState = " << PrintSharedStateName(m_CurrEventCurrState.busRespEventState) << std::endl;
             std::cout << "\t\t CacheLine NextState = " << PrintSharedStateName(m_CurrEventNextState.busRespEventState) << std::endl;
             std::cout << "\t\t Ctrl ReqAction      = " << PrintSharedActionName(m_CurrEventAction.busRespAction) << std::endl;
           }
         }
      
         if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::SharedNullAck) {
           if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::StoreData || m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::StoreDataOnly) {
            if((!m_duetto && m_mode =="RT") || m_duetto == true) {
               // Now we need to declare the write request to be finished and adjust the oldest request and the order if required        
               /********************* HERE THE WRITE is PROCESSING  ****************************/
               for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
               if(m_reza_log_shared) cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared)  cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared)  cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared)  cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared)  cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared) cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
               if(m_reza_log_shared) cout<<"The response is processed in shared cache, the addr is "<<m_CurrEventMsg.busRespMsg.addr<<" the reqCoreID is "<<m_CurrEventMsg.busRespMsg.reqCoreId<<
                                                                                                                " the respCoreID is "<<m_CurrEventMsg.busRespMsg.respCoreId<<
                                                                                                                " msgID is "<<m_CurrEventMsg.busRespMsg.msgId<<
                                                                                                                " m_reqCoreCnt is "<<m_reqCoreCnt<<endl;
               for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 1 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}

               if(m_CurrEventMsg.busRespMsg.respCoreId == m_coreId || m_CurrEventMsg.busRespMsg.msgId == 0) { 
                
                if(isOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId)) {  // 0- check first if it is the oldest of its requestor
                  //cout<<"It is oldest the order now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<endl;
                  // 1- If YES, adjust the order 
                  for(unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size() ; h++) {
                    if(m_GlobalQueue->m_GlobalRRQueue.at(h) == m_reqCoreCnt) {
                      m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                    }
                  }
                                  
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 2 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  //cout<<"Order is adjusted and now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<"  the oldest queue size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<endl; 
                  // 2- If YES, remove this oldest request from oldest queue            
                  removeFromOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId);
                  removeFromM_Type(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 3 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  // cout<<" the non oldest queue size "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl; 
                  removeFromNonOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 4 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  //cout<<" the non oldest queue size after removing "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl; 
                  // cout<<" it should be removed from oldest queue and now the size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<endl; 
                  // 3- adjust the oldest request
                  if( m_CurrEventMsg.busRespMsg.msgId != 0)
                    adjustOldest(m_CurrEventMsg.busRespMsg.reqCoreId);
                  else  
                    adjustOldest(m_CurrEventMsg.busRespMsg.respCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<"5 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  //  cout<<" the oldest shold be inserted if exist and now the size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<" the Order queue size is "<<m_GlobalQueue->m_GlobalRRQueue.size()<<endl; 
                  /*********************** shouldnt you remove it from the global request queue as well? ***************/            

                }
                else {
                  // cout<<" the request is non oldest so remove from the non oldest queue the size is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl; 
                  removeFromM_Type(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId);
                  removeFromNonOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.reqCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<"6 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  // cout<<" the request is non oldest so remove from the non oldest queue the size after removing is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
                }
              }
              else if(terminateBank(m_CurrEventMsg.busRespMsg.reqCoreId,m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.msgId)){
                if(m_reza_log_shared) cout<<"need to manipulate "<<endl;
                if(isOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId)) {  // 0- check first if it is the oldest of its requestor
                  cout<<"It is oldest the order now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<endl;
                  // 1- If YES, adjust the order 
                  for(unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size() ; h++) {
                    if(m_GlobalQueue->m_GlobalRRQueue.at(h) == m_reqCoreCnt) {
                      m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                    }
                  }
                  if(m_reza_log_shared) cout<<"after order  "<<endl;                
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 2 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  //cout<<"Order is adjusted and now is "<<m_GlobalQueue->m_GlobalRRQueue.at(0)<<"  the oldest queue size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<endl; 
                  // 2- If YES, remove this oldest request from oldest queue            
                  removeFromM_Type(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId);
                  removeFromOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 3 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  // cout<<" the non oldest queue size "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl; 
                  removeFromNonOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<" 4 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}                             
                  adjustOldest(m_CurrEventMsg.busRespMsg.respCoreId);                
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<"5 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  //  cout<<" the oldest shold be inserted if exist and now the size is  "<<m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize()<<" the Order queue size is "<<m_GlobalQueue->m_GlobalRRQueue.size()<<endl; 
                  /*********************** shouldnt you remove it from the global request queue as well? ***************/            

                }
                else {
                  // cout<<" the request is non oldest so remove from the non oldest queue the size is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl; 
                  removeFromM_Type(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId);
                  removeFromNonOldest(m_CurrEventMsg.busRespMsg.addr,m_CurrEventMsg.busRespMsg.respCoreId);
                  for(unsigned int i=0; i< m_GlobalQueue->m_GlobalRRQueue.size();i++){ if(m_reza_log_shared) cout<<"6 Order is: "<<m_GlobalQueue->m_GlobalRRQueue.at(i)<<endl;}
                  // cout<<" the request is non oldest so remove from the non oldest queue the size after removing is "<<m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize()<<endl;
                }
              }
            }
                 

            m_busIfFIFO->m_rxRespFIFO.PopElement();
             
            if (ProcFromVictimCache) {
              TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx); 
              TempCacheline.state = m_CurrEventNextState.busRespEventState;
              m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, TempCacheline);
            }
            else {
              UpdateSharedCache (Line, m_CurrEventMsg.busRespMsg.addr, m_CurrEventNextState.busRespEventState, m_CurrEventMsg.busRespMsg.data);
            }
      
            if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::StoreDataOnly) {        /********************** WHY it should send the data to the TX RESP of shared cache after storing the data to the cache itself? ****************/
              SendPendingReqData(respAddrMap);
            }
           }
           else if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::SharedNoAck) {
             /******************** We should not do anything here right? ******************************/
             cout<<"Error:: Make sure that if this happens, you should not initate processing for the bank. Add if statement above"<<endl;
             abort();
             m_busIfFIFO->m_rxRespFIFO.PopElement();
           }
           else { 
             if (m_logFileGenEnable){
               std::cout << "SharedCache: DataResp occur in illegal state!" << std::endl; 
             }   
             exit(0);     
           }

           m_CurrEventAction.busRespAction = SNOOPSharedCtrlAction::SharedProcessedAck;
         }
       }  // if (m_CurrEventList.busRespEvent != SNOOPSharedRespBusEvent::NUll) {
      }

      

      if(m_reza_log_shared) cout<<"In  SharedCacheCtrl:: Check the RX MSG path"<<endl;

      bool EvictDoneFlag = false;
      bool OutStandingExclFlag;
      GenericCacheFrmt busReqCacheLine;
      GenericCacheMapFrmt reqbusAddrMap;
                          
      BusIfFIFO::BusReqMsg        BusReqMsgTemp = {};
      SNOOPSharedReqBusEvent      BusReqEventTemp;
                                  
      int emptyWay,
          replcWayIdx,
          victimWayIdx;
          
      uint16_t tmpVictimOwnerCoreId;
      
      SendDataType sendType;
    
      
      SNOOPSharedCtrlEvent DRAMReqEvent;
      
      bool ItrSkipSwappingChk;
      NewFetchDone = false;
      ItrSkipSwappingChk = false;

      QueueSize = m_busIfFIFO->m_rxMsgFIFO.GetQueueSize();
      // cout<<"Queue size is  "<<QueueSize<<endl;
      for (int i = 0; i < QueueSize; i++ ) {
        //cout<<"In  SharedCacheCtrl:: going through the RR: "<<m_reqCoreCnt<<endl;        
        m_CurrEventList.busReqEvent  = ChkBusRxReqEvent  (m_CurrEventMsg.busReqMsg);
        if(m_reza_log_shared) cout<<"The size of the m_busIfFIFO->m_rxMsgFIFO "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;

        m_busIfFIFO->m_rxMsgFIFO.PopElement();

        if(m_reza_log_shared) cout<<"The size of the m_busIfFIFO->m_rxMsgFIFO "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;

        if (NewFetchDone == false) {
          if(m_reza_log_shared) cout<<"NewFetchDone == false"<<endl;
          ProcFromVictimCache = false;
          BusReqMsgTemp   = m_CurrEventMsg.busReqMsg;
          BusReqEventTemp = m_CurrEventList.busReqEvent;
          CacheLineInfo  = m_cache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr);
          VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr); 
          reqbusAddrMap         = m_cache->CpuAddrMap (m_CurrEventMsg.busReqMsg.addr);
          

          if (CacheLineInfo.IsValid == false) { 
            // The follwing condition gets never false since we assume perfect cache... No DRAM 
            if (VictimCacheLineInfo.IsValid == false) { 
              if(m_reza_log_shared) cout<<"ERROR Check: VictimCacheLineInfo.IsValid FALSE"<<endl;
              abort();
              DRAMReqEvent = ChkDRAMReqEvent (m_CurrEventList.busReqEvent);
              m_CurrEventList.CtrlEvent = DRAMReqEvent; 
              SendExclRespEarly();

              TempCacheline.valid   = true;
              TempCacheline.evicted = false;
              TempCacheline.state   = GetWaitDRAMRespState();
              TempCacheline.mru_bit = false;
              TempCacheline.tag     = reqbusAddrMap.tag;
              TempCacheline.insertCycle = m_cacheCycle;
              TempCacheline.accessCounter = 0;
              TempCacheline.accessCycle   = m_cacheCycle;
              
              if (CacheLineInfo.IsSetFull == false) { 
                emptyWay = m_cache->GetEmptyCacheLine(CacheLineInfo.set_idx);
                if (emptyWay == -1) {
                  std::cout << "SharedMem: Fatal non-empty Condition" << std::endl;
                  exit(0);
                }
                else {
                  m_cache->WriteCacheLine (CacheLineInfo.set_idx, emptyWay, TempCacheline);
                  EvictDoneFlag = true; 
                }
              }
              else { 
                if (!VictimCacheLineInfo.IsSetFull) { 
                  replcWayIdx     = m_cache->GetReplacementLine(CacheLineInfo.set_idx, m_replcPolicy);  
                  ReplcCacheLine  = m_cache->ReadCacheLine(CacheLineInfo.set_idx, replcWayIdx);
                  SclWayIdx       = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + replcWayIdx;
                  
                  CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
                  CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                    
                  if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::SendVictimCache) {                   
                    victimWayIdx   = m_victimCache->GetNextWritePtr();
                    m_victimOwnerCoreId[victimWayIdx] = m_ownerCoreId[SclWayIdx];
                    ReplcCacheLine.tag = m_cache->CpuPhyAddr (CacheLineInfo.set_idx, replcWayIdx); 
                    ReplcCacheLine.insertCycle = m_cacheCycle;
                    m_victimCache->WriteCacheLine(victimWayIdx,ReplcCacheLine);
                    
                    m_cache->WriteCacheLine (CacheLineInfo.set_idx, replcWayIdx, TempCacheline);
                    EvictDoneFlag = true;    
                  }
                  else {
                    m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
                  }   
                } // if (!VictimCacheLineInfo.IsSetFull) { 
                else { 
                  m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
                }
              } 
            }
            else { 
              // This whole part is for taking something from the victim cache and: send it early OR replace it with the shared cache
              tmpVictimOwnerCoreId = m_victimOwnerCoreId[VictimCacheLineInfo.line_idx];
              CacheLineOwner = (tmpVictimOwnerCoreId == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
              TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx); 
              CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, VictimCacheLineInfo.state, CacheLineOwner); 
              if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedNoAck) { 
             
                ProcFromVictimCache = true;
              }
              else { 
                replcWayIdx     = m_cache->GetReplacementLine(CacheLineInfo.set_idx, m_replcPolicy);
                ReplcCacheLine  = m_cache->ReadCacheLine(CacheLineInfo.set_idx, replcWayIdx);
                SclWayIdx       = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + replcWayIdx;
                
                m_CurrEventList.CtrlEvent = SNOOPSharedCtrlEvent::VictCacheSwap;
                CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                  
                if (m_CurrEventAction.CtrlAction != SNOOPSharedCtrlAction::SWAPPING || ItrSkipSwappingChk == true) { 
                  ItrSkipSwappingChk = true;
                  CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, VictimCacheLineInfo.state, CacheLineOwner); 
                  if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveRespStall ||
                      m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive || 
                      m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
                    SendExclRespEarly();
                  }
                  m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;              
                }
                else { 
                  TempCacheline.tag           = reqbusAddrMap.tag;  
                  TempCacheline.insertCycle   = m_cacheCycle;
                  TempCacheline.accessCounter = 0;
                  TempCacheline.accessCycle   = m_cacheCycle;
                  TempCacheline.evicted       = false;
                  
                  ReplcCacheLine.tag = m_cache->CpuPhyAddr (CacheLineInfo.set_idx, replcWayIdx);
                  ReplcCacheLine.insertCycle = m_cacheCycle; 
                  m_cache->WriteCacheLine (CacheLineInfo.set_idx, replcWayIdx, TempCacheline);
                  m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, ReplcCacheLine);
                  m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_ownerCoreId[SclWayIdx];
                  
                  m_ownerCoreId[SclWayIdx] = tmpVictimOwnerCoreId;
                  CacheLineInfo  = m_cache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr);
              
                  if (CacheLineInfo.IsValid == false) {
                    std::cout << "SharedMem: Fatal Error after swapping victim cache with L2 Cache" << std::endl;
                    exit(0);               
                  }
                }
              }
            }
          } // if (CacheLineInfo.IsValid == false) {    
          if (CacheLineInfo.IsValid) {
            if(m_reza_log_shared) cout<<"Cache Line is Valid"<<endl;

            SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;             
            CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore ;
            CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, CacheLineInfo.state, CacheLineOwner); // process ReqBus event
            
            /************** This condition also should not be true when we have perfect cache I believe *************/
            
            if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveRespStall) {
              // cout<<"check here"<<endl;
              // abort();
              m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
              SendExclRespEarly();
            }
          } 
          if (EvictDoneFlag) {
            if(m_reza_log_shared) cout<<"Evict Done Flage is True"<<endl;
            m_Nmiss++;
            m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
            if (!SendDRAMReq (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.addr, DRAMIfFIFO::DRAM_REQ::DRAM_READ)) {
              std::cout << "SharedMem: Error: Cannot Direct Request to DRAM " << std::endl;
              exit(0);
            }
          }
          if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
            m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
          }
          if (EvictDoneFlag || m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedStall) {
            NewFetchDone = true;
          }   
        }
        else {
          m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
        }
      }  // for (int i = 0; i < QueueSize; i++ ) {

      if(m_reza_log_shared) cout<<"5------------------------------------------------------------------------------Queue size is  "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;

      m_CurrEventMsg.busReqMsg = BusReqMsgTemp;
      m_CurrEventList.busReqEvent = BusReqEventTemp;
      
      reqbusAddrMap   = m_cache->CpuAddrMap (m_CurrEventMsg.busReqMsg.addr);

      if (QueueSize == 0 || m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
        m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedNullAck;
      }
      
      if (m_logFileGenEnable){
        if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck) {
          std::cout << "\nSharedMem: " << m_coreId << " has Msg in the RxReq Bus" << std::endl;
          std::cout << "\t\t BusEventName        = " << PrintSharedReqBusEventName(m_CurrEventList.busReqEvent) << std::endl;
          std::cout << "\t\t ReqCoreId           = " << m_CurrEventMsg.busReqMsg.reqCoreId << " RespCoreId = " << m_CurrEventMsg.busReqMsg.wbCoreId 
                    << " Req Addr  = " << m_CurrEventMsg.busReqMsg.addr << " CacheLine = " << reqbusAddrMap.idx_set << std::endl;
          std::cout << "\t\t CacheLine CurrState = " << PrintSharedStateName(m_CurrEventCurrState.busReqEventState) << std::endl;
          std::cout << "\t\t CacheLine NextState = " << PrintSharedStateName(m_CurrEventNextState.busReqEventState) << std::endl;
          std::cout << "\t\t Ctrl ReqAction      = " << PrintSharedActionName(m_CurrEventAction.busReqAction) << " ==================== " << m_cacheCycle << std::endl;
        }
      }    
      if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck) {
        if(m_reza_log_shared) cout<<"!SharedNullAck"<<endl;      
        if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS || 
            m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
              if(m_reza_log_shared) cout<<"GetM/GetS"<<endl;      
          m_Nreqs++;
        }
        
        if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
          OutStandingExclFlag = RemoveExclRespAddr(); 
        }
        
        if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedFault) {
          std::cout << "SharedCache: DataResp occur in illegal state!" << std::endl;    
          exit(0);     
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedNoAck) {
          if(m_reza_log_shared) cout<<"SharedNoAck"<<endl;      
          // do nothing     
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
          if(m_reza_log_shared) cout<<"SharedStall"<<endl;      
          // do nothing
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendData || 
                m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive ||
                m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveResp) {
                  if(m_reza_log_shared) cout<<"SendData/SendDataExclusive/SendExclusiveResp"<<endl; 
            
          if(m_reza_log_shared) cout<<"From "<<" Shared Cache "<<m_coreId<<" Send Data "<<endl;   
          m_CurrEventMsg.busReqMsg.orderDetermined = true;
          m_CurrEventMsg.busReqMsg.orderofArbitration = 0;         
          m_CurrEventMsg.busReqMsg.currStage = "REQ";
          if((m_CurrEventMsg.busReqMsg.msgId == 0 && isOldest(m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.wbCoreId)) || (m_CurrEventMsg.busReqMsg.msgId != 0 && isOldest(m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.reqCoreId))) 
            assignDeadlineAfterDetermination(m_CurrEventMsg.busReqMsg);
          m_GlobalQueue->m_MsgType.InsertElement(m_CurrEventMsg.busReqMsg);  
          


          OutStandingExclFlag = RemoveExclRespAddr();      
          
          sendType = (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendData         ) ? DataOnly    :
                    (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive) ? DataPlsExcl : 
                                                                                                    ExclOnly;
          if (OutStandingExclFlag) { 
            if (sendType == ExclOnly) {
              std::cout << "SharedMem: Fatal Excl processing Error" << std::endl;
              exit(0);
            }
            sendType = DataOnly;
          }
        
          if(m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveResp) {
            // save pending write back
            if (!PushMsgInBusTxFIFO (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.reqCoreId, m_coreId, m_CurrEventMsg.busReqMsg.addr )) {
              if (m_logFileGenEnable){
                std::cout << "SharedMem: Pending buffer is full !!!!" << std::endl;
              }
              exit(0);
            }
          }
                                    
          if (!DoWriteBack (m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, sendType)) {
            if (m_logFileGenEnable){
              std::cout << "SharedMem: Cannot Send Data on the bus !!!!" << std::endl;
            }
            exit(0);
          }
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SaveReqCoreId) {
          if(m_reza_log_shared) cout<<"SaveReqCoreID"<<endl;      
          // save pending write back
          if (!PushMsgInBusTxFIFO (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.reqCoreId, m_coreId, m_CurrEventMsg.busReqMsg.addr )) {
            if (m_logFileGenEnable){
              std::cout << "SharedMem: Pending buffer is full !!!!" << std::endl;
            }
            exit(0);
          }
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendPendingData) {
          if(m_reza_log_shared) cout<<"SendPendingData"<<endl;      
          SendPendingReqData(reqbusAddrMap);
        }
        else { // unknown action
          std::cout << "SharedMem: BusReq undefine action occur!" << std::endl;    
          exit(0);     
        }

        // update state into LLC cache 
        if (ProcFromVictimCache) {
          TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx);
          TempCacheline.state = m_CurrEventNextState.busReqEventState;
          m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, TempCacheline);
        }
        else { 
          bool UpdateAccessCnt;
          UpdateAccessCnt = (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS || 
                            m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM);
          UpdateSharedCache (State, m_CurrEventMsg.busReqMsg.addr, m_CurrEventNextState.busReqEventState, NULL, UpdateAccessCnt);
        }
        // update owner core
        if (m_CurrEventOwnerNextState.busReqOwnerState == SNOOPSharedOwnerState::SharedMem) {
          if (ProcFromVictimCache) {
            m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_coreId;
          }
          else {
            m_ownerCoreId[SclWayIdx] = m_coreId;
          }
        }
        else if (m_CurrEventOwnerNextState.busReqOwnerState == SNOOPSharedOwnerState::OtherCore) {
          if (ProcFromVictimCache) {
            m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_CurrEventMsg.busReqMsg.reqCoreId;
          }
          else {
            m_ownerCoreId[SclWayIdx] = m_CurrEventMsg.busReqMsg.reqCoreId;
          }
        }
        m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedProcessedAck;
      } // if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck)
         
      
      
      bool SkipEviction;
      uint64_t ReplcAddr;
      for (uint32_t i = 0; i < m_victimCacheSize / m_cacheBlkSize ; i++){
        ReplcCacheLine = m_victimCache->ReadCacheLine(i);
        ReplcAddr      = ReplcCacheLine.tag;         
        if (ReplcCacheLine.valid && ReplcCacheLine.evicted == false) {
          QueueSize = m_busIfFIFO->m_rxMsgFIFO.GetQueueSize();
          SkipEviction = false;
          for (int j = 0; j < QueueSize; j++ ) {
            m_CurrEventMsg.busReqMsg  = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();
            m_busIfFIFO->m_rxMsgFIFO.PopElement();
            m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
            if ((m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)) == (ReplcAddr >> (int) log2(m_cacheBlkSize))) {
              SkipEviction = true;
            }
          } 
          if (SkipEviction == false) {
            VictimCacheLineEvict(i);
          }
        }
      }
      //cout<<"6------------------------------------------------------------------------------Queue size is  "<<m_busIfFIFO->m_rxMsgFIFO.GetQueueSize()<<endl;
      //cout<<"Finish SharedCacheCtrl::CacheCtrlMain"<<endl;
    }
    
    void SharedCacheCtrl::CacheInitialize () {
    
      uint32_t victimCacheLines = m_victimCacheSize / m_cacheBlkSize;
      m_victimCache = CreateObject<VictimCache> (victimCacheLines);
      m_victimCache->SetCacheSize   ( m_victimCacheSize );
      m_victimCache->SetCacheBlkSize( m_cacheBlkSize    );
      m_victimCache->SetCacheLines  ( victimCacheLines  );
    
      if (m_logFileGenEnable){
        std::cout << "\nSharedMem: [Info] Cache State, Owner Initialization" << std::endl; 
      }
       // initialize shared cache states
      InitCacheStates();
      
      // initialize core owner
      for (uint32_t i = 0; i < m_cacheSize/m_cacheBlkSize; i++) {
        m_ownerCoreId[i] = m_coreId;
      }
      
      m_victimOwnerCoreId = new uint16_t[victimCacheLines];
      for (uint32_t i = 0; i < victimCacheLines; i++) {
        m_victimOwnerCoreId[i] = m_coreId;
      }
      
      // load memory content from benchmark files 
      if (m_dramSimEnable == false || m_l2CachePreloadFlag == true) {
        GenericCacheFrmt cacheLine;
        std::stringstream ss;
        std::string fline;
        std::ifstream m_bmTrace;
        size_t pos;
        std::string s;
        uint64_t addr;
        uint32_t setIdx;
        uint32_t nWays;
        uint32_t setOfst;
        bool     filled;
        uint32_t n_miss = 0;
        for (int i = 0; i < m_nPrivCores; i++) {
          ss << m_bmsPath <<"/trace_C" << i << ".trc.shared";
          if (m_logFileGenEnable){
            std::cout << "\nSharedMem: [Info] Load benchmark file\n\t" << ss.str() << std::endl; 
          }

          m_bmTrace.open(ss.str());
          if(m_bmTrace.is_open()) {
            while (getline(m_bmTrace,fline)){
              pos = fline.find(" ");
              s = fline.substr(0, pos); 
              // convert hex string address to decimal 
              addr = (uint64_t) strtol(s.c_str(), NULL, 16);

              GenericCacheMapFrmt addrMap   = m_cache->CpuAddrMap (addr);
              setIdx = addrMap.idx_set;
              nWays  = m_cache -> GetCacheNways();
              setOfst = setIdx * nWays;
              filled = false;
              uint32_t wayIdx;
              //std::cout << "Load Shared Mem addr = " << addr << " setIdx = " << setIdx << " nways = " << nWays <<" setOfst = " <<  setOfst << std::endl;
              for (wayIdx = setOfst; wayIdx < setOfst+nWays;wayIdx++) {
                cacheLine    = m_cache->ReadCacheLine(wayIdx);
                if (cacheLine.valid == true && cacheLine.tag == addrMap.tag) {
                  //std::cout << "SharedMem: [Info] Shared Cache Line exist\n"; 
                  filled = true;
                  //std::cout << "Load Shared Mem addr = " << addr << " tag = " << addrMap.tag << " setIdx = " << setIdx << " nways = " << nWays <<" wayIdx = " <<  wayIdx << std::endl;
                  break;
                }
                else if (cacheLine.valid == false) {
                  //std::cout << "Load Shared Mem addr = " << addr << " tag = " << addrMap.tag << " setIdx = " << setIdx << " nways = " << nWays <<" wayIdx = " <<  wayIdx << std::endl;
                  cacheLine.valid = true;
                  cacheLine.tag   = addrMap.tag;
                  filled = true;
                  m_cache->WriteCacheLine(cacheLine,wayIdx);
                  break;
                }
              }
              if (filled == false) {
                n_miss++;
              }
           }
            m_bmTrace.close();
            ss.str(std::string());
          }
          else {
            if (m_logFileGenEnable){
              std::cout<<"SharedMem: [Error] Benchmark file name "<< m_bmsPath << " is not exist!" << std::endl;
            }
            exit(0); // The program is terminated here
          } 
        } // for (int i = 0; i < m_nPrivCores; i++)
        
        if (n_miss != 0 && m_dramSimEnable == false) {
          std::cout << "SharedMem: [Error] Shared Line cannot placed in cache, increase L2 Cache Size to able to simulate with perfect L2Cache, n_miss = " <<  n_miss << std::endl;
          exit(0);
        }
        else if (n_miss == 0) {
          std::cout << "All data loaded in L2 Cache" << std::endl;
        }
      } // if (m_dramSimEnable == false) {      
    }
    
             
    // Cohr FSM Function Call
    void SharedCacheCtrl::CohProtocolFSMProcessing (SNOOPSharedEventType eventType, int state, SNOOPSharedOwnerState owner) {
    
      int state_n;
      SNOOPSharedOwnerState owner_n;
      SNOOPSharedCtrlAction action;
      
      owner_n = owner;
      state_n = state;
      
      IFCohProtocol *ptr;
      
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          ptr = new PMSI; break;
        case CohProtType::SNOOP_MSI:
          ptr = new MSI; break;
        case CohProtType::SNOOP_MESI:
          ptr = new MESI; break;
        case CohProtType::SNOOP_MOESI:
          ptr = new MOESI; break;
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
        
        ptr->SNOOPSharedEventProcessing (eventType, m_cache2Cache, m_CurrEventList, state_n, owner_n, action);
        
        if (eventType == SNOOPSharedEventType::ReqBus) {
          m_CurrEventCurrState.busReqEventState      = state;
          m_CurrEventNextState.busReqEventState      = state_n;
          m_CurrEventOwnerCurrState.busReqOwnerState = owner;
          m_CurrEventOwnerNextState.busReqOwnerState = owner_n;
          m_CurrEventAction.busReqAction             = action;
        }
        else if (eventType == SNOOPSharedEventType::RespBus) {
          m_CurrEventCurrState.busRespEventState      = state;
          m_CurrEventNextState.busRespEventState      = state_n;
          m_CurrEventOwnerCurrState.busRespOwnerState = owner;
          m_CurrEventOwnerNextState.busRespOwnerState = owner_n;
          m_CurrEventAction.busRespAction             = action;
        }
        else if (eventType == SNOOPSharedEventType::CacheCtrl) {
          m_CurrEventCurrState.CtrlEventState      = state;
          m_CurrEventNextState.CtrlEventState      = state_n;
          m_CurrEventOwnerCurrState.CtrlOwnerState = owner;
          m_CurrEventOwnerNextState.CtrlOwnerState = owner_n;
          m_CurrEventAction.CtrlAction             = action;
        }
        else {
          std::cout << "SharedMem: Error: Known Processing Event" << std::endl;
          exit(0);
        }
        
        delete ptr;
    }
     
    int SharedCacheCtrl::GetWaitDRAMRespState() { 
      switch (m_cohrProt) {
        case CohProtType::SNOOP_MESI:
          return static_cast<int>(SNOOP_MESISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_MSI:
          return static_cast<int>(SNOOP_MSISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_MOESI:
          return static_cast<int>(SNOOP_MOESISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_PMSI: 
          std::cout << "SharedMem: PMSI currently not support DRAM ACQ" << std::endl;
          exit(0);
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
    } 
       
    // Initialize Cache Internal States
    void SharedCacheCtrl::InitCacheStates () {
       int initState = ResetCacheState();
        m_cache->InitalizeCacheStates(initState);
        m_victimCache->InitalizeCacheStates(initState);
    }
    
    int SharedCacheCtrl::ResetCacheState () {
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          return static_cast<int>(SNOOP_PMSISharedCacheState::IorS);
        case CohProtType::SNOOP_MSI:
          return static_cast<int>(SNOOP_MSISharedCacheState::IorS);
        case CohProtType::SNOOP_MESI:
          return static_cast<int>(SNOOP_MESISharedCacheState::I);
        case CohProtType::SNOOP_MOESI:
          return static_cast<int>(SNOOP_MOESISharedCacheState::I);
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
    }
    
    std::string SharedCacheCtrl::PrintSharedStateName (int state) {
      IFCohProtocol *obj;
      std::string sName;
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          obj = new(PMSI);
          break;
        case CohProtType::SNOOP_MSI:
          obj = new(MSI);
          break;
        case CohProtType::SNOOP_MESI:
          obj = new(MESI);
          break;
        case CohProtType::SNOOP_MOESI:
          obj = new(MOESI);
          break;
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      sName = obj->SharedStateName (state);
      delete obj;
      return sName;
    }

    bool SharedCacheCtrl::CheckBypass (int state) {
      IFCohProtocol *obj;
      bool check;
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          return false;
          break;
        case CohProtType::SNOOP_MSI:
          return false;
          break;
        case CohProtType::SNOOP_MESI:
          obj = new(MESI);
          break;
        case CohProtType::SNOOP_MOESI:
          return false;
          break;
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      check = obj->ByPassMESI (state);
      delete obj;
      return check;
    }
       
     std::string SharedCacheCtrl::PrintSharedActionName (SNOOPSharedCtrlAction action)
     {
       switch (action) {
         case SNOOPSharedCtrlAction::SharedNoAck:
           return " No-Action"; 
         case SNOOPSharedCtrlAction::SharedStall:
           return " Stall (Waiting For Data Resp)";
         case SNOOPSharedCtrlAction::SendData:
           return " SendData to core";
         case SNOOPSharedCtrlAction::SendDataExclusive:
           return " Send Exclusive Data to core";
         case SNOOPSharedCtrlAction::SendExclusiveResp:
           return " SendExclusiveResp: Send Exclusive Response to core";
         case SNOOPSharedCtrlAction::StoreData:
           return " StoreData into Memory and Check Pending WB Buffer";
         case SNOOPSharedCtrlAction::StoreDataOnly:
           return " StoreData Only into Memory";
         case SNOOPSharedCtrlAction::SaveReqCoreId:
           return " SaveReqCoreId Msg in Pending WB buffer";
         case SNOOPSharedCtrlAction::SendPendingData:
           return " SendPending Data to core";
         case SNOOPSharedCtrlAction::SendVictimCache:
           return " SendVictimCache";           
         case SNOOPSharedCtrlAction::CopyDRAMIntoCache:
           return " CopyDRAMIntoCache ";          
         case SNOOPSharedCtrlAction::IssueDRAMWrite:
           return " IssueDRAMWrite";
         case SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite:
           return " IssueCoreInvDRAMWrite";
         case SNOOPSharedCtrlAction::IssueCoreInv:
           return " IssueCoreInv";
         case SNOOPSharedCtrlAction::SendExclusiveRespStall:
           return " SendExclusiveRespStall";
         case SNOOPSharedCtrlAction::SharedFault:
           return " Fault" ;
         case SNOOPSharedCtrlAction::SharedNullAck:
           return " No Pending Request";
         case SNOOPSharedCtrlAction::SharedProcessedAck:
           return " Actioned is Processsed";
         default:
           return " Unknown !!!!";
       }
     }

     
     std::string SharedCacheCtrl::PrintSharedRespBusEventName (SNOOPSharedRespBusEvent event)
     {
       switch (event) {
         case SNOOPSharedRespBusEvent::OWnDataResp:
           return " OwnDataRespEvent "; 
         case SNOOPSharedRespBusEvent::OTherDataResp:
           return " OtherDataRespEvent "; 
         case SNOOPSharedRespBusEvent::DRAMDataResp:
           return " DRAMDataResp "; 
         case SNOOPSharedRespBusEvent::NUll:
           return " NullRespEvent ";
         default:
           return "Unknown !!!!";
       }
     }

     std::string SharedCacheCtrl::PrintSharedReqBusEventName (SNOOPSharedReqBusEvent event)
     {
       switch (event) {
         case SNOOPSharedReqBusEvent::GetS:
           return " GetSEvent "; 
         case SNOOPSharedReqBusEvent::GetM:
           return " GetMEvent ";
         case SNOOPSharedReqBusEvent::Upg:
           return " UpgEvent ";
         case SNOOPSharedReqBusEvent::OwnerPutM:
           return " OwnerPutMEvent ";
         case SNOOPSharedReqBusEvent::OTherPutM:
           return " OTherPutMEvent " ;
         case SNOOPSharedReqBusEvent::PutS:
           return " PutSEvent ";
         case SNOOPSharedReqBusEvent::Null:
           return " NullReqEvent ";
         default:            
           return "Unknown !!!!";
       }
     }
     
    void SharedCacheCtrl::SetProtocolType (CohProtType ptype) {
      m_cohrProt = ptype;
    }
    
    void SharedCacheCtrl::CycleProcess() {
      if(m_reza_log_shared) cout<<"Shared Cache Controller Cycle Process"<<endl;
      // if(m_cacheCycle > 1 ){
      //    m_reza_log_shared = true;
      //  }
       CacheCtrlMain();
      // Schedule the next run
      if (m_duetto)
      {
        /**** Logic to determine the arbiter****/
        // if (m_GlobalQueue->bank_mode)
        // {
        //     if (m_reza_log)
        //         cout << "---------------------------------------------------------------------------------------------------------------------------------------------- REQ Arbiter HP Mode" << endl;
        //     if (m_mode == "RT")
        //     {
        //         cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from RT to HP  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        //     }
        //     m_mode = "HP";
        // }
        // else
        // {
        //     if (m_reza_log)
        //         cout << "----------------------------------------------------------------------------------------------------------------------------------------------  REQ Arbiter RT Mode " << endl;
        //     if (m_mode == "HP")
        //     {                
        //         cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  switch from HP to RT  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
        //     }
        //     m_mode = "RT";
        // }
      }
      Simulator::Schedule(NanoSeconds(m_dt), &SharedCacheCtrl::Step, Ptr<SharedCacheCtrl > (this));
      m_cacheCycle++;
    }

    // The init function starts the controller at the beginning 
    void SharedCacheCtrl::init() {
     // cout<<"In Shared Cache Controller Init"<<endl;
        CacheInitialize ();
        Simulator::Schedule(NanoSeconds(m_clkSkew), &SharedCacheCtrl::Step, Ptr<SharedCacheCtrl > (this));
    }

    /**
     * Runs one mobility Step for the given vehicle generator.
     * This function is called each interval dt
     */

    void SharedCacheCtrl::Step(Ptr<SharedCacheCtrl> sharedCacheCtrl) {
      //cout<<"In Shared Cache Controller Step"<<endl;
        sharedCacheCtrl->CycleProcess();
    }

}
