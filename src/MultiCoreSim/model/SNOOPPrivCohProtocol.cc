/*
 * File  :      SNOOPPrivCohProtocol.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On May 30, 2020
 */

#include "SNOOPPrivCohProtocol.h"
using namespace std;
namespace ns3 {

    // override ns3 type
    TypeId SNOOPPrivCohProtocol::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::SNOOPPrivCohProtocol")
               .SetParent<Object > ();
        return tid;
    }

    SNOOPPrivCohProtocol::SNOOPPrivCohProtocol() {
        // default
        m_pType            = CohProtType::SNOOP_PMSI;
        m_processEvent     = SNOOPPrivEventType::Core;
        m_coreId           = 1;
        m_sharedMemId      = 10;
        m_cache2Cache      = false;
        m_logFileGenEnable = false;         
        m_prllActionCnt    = 0;
        m_reqWbRatio       = 0;
        m_currEventTrans2Issue = NullTrans;
        m_replcPolicy      = ReplcPolicy::RANDOM;
        uRnd1 = CreateObject<UniformRandomVariable> ();
        uRnd1-> SetAttribute ("Min", DoubleValue (0));
        uRnd1-> SetAttribute ("Max", DoubleValue (100));
        m_reza_log_snoop   = false;
    }

    // We don't do any dynamic allocations
    SNOOPPrivCohProtocol::~SNOOPPrivCohProtocol() {
    }

    void SNOOPPrivCohProtocol::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }

    void SNOOPPrivCohProtocol::SetCoreId (int coreId) {
      m_coreId = coreId;
    }

    void SNOOPPrivCohProtocol::SetSharedMemId (int sharedMemId) {
      m_sharedMemId = sharedMemId;
    }

    void SNOOPPrivCohProtocol::SetReqWbRatio (int reqWbRatio) {
      m_reqWbRatio = reqWbRatio;
    }

    void SNOOPPrivCohProtocol::SetProtocolType (CohProtType ptype) {
      m_pType = ptype;
    }

    CohProtType SNOOPPrivCohProtocol::GetProtocolType () {
      return m_pType;
    } 

    void SNOOPPrivCohProtocol::SetPrivCachePtr (Ptr<GenericCache> privCache) {
      m_privCache = privCache;
    }

    void SNOOPPrivCohProtocol::SetCpuFIFOPtr (Ptr<CpuFIFO> cpuFIFO) {
      m_cpuFIFO = cpuFIFO;
    }

    void SNOOPPrivCohProtocol::SetBusFIFOPtr (Ptr<BusIfFIFO> busFIFO) {
      m_busIfFIFO = busFIFO;
    }

    void SNOOPPrivCohProtocol::SetCache2Cache (bool cache2Cache) {
      m_cache2Cache = cache2Cache;
    }

    CpuFIFO::ReqMsg SNOOPPrivCohProtocol::GetCpuReqMsg () {
      return m_msgList.cpuReqMsg;
    }

    void SNOOPPrivCohProtocol::SetCpuReqMsg (CpuFIFO::ReqMsg cpuReqMsg) {
      m_msgList.cpuReqMsg = cpuReqMsg;
    }

    BusIfFIFO::BusReqMsg SNOOPPrivCohProtocol::GetBusReqMsg () {
      return m_msgList.busReqMsg;
    }

    BusIfFIFO::BusRespMsg SNOOPPrivCohProtocol::GetBusRespMsg () {
      return m_msgList.busRespMsg;
    }

    SNOOPPrivCoreEvent SNOOPPrivCohProtocol::GetCpuReqEvent () {
      return m_eventList.cpuReqEvent;
    }

    void SNOOPPrivCohProtocol::SetCpuReqEvent (SNOOPPrivCoreEvent cpuReqEvent) {
       m_eventList.cpuReqEvent = cpuReqEvent;
    }

    SNOOPPrivReqBusEvent SNOOPPrivCohProtocol::GetBusReqEvent () {
      return m_eventList.busReqEvent;
    }

    SNOOPPrivRespBusEvent SNOOPPrivCohProtocol::GetBusRespEvent () {
      return m_eventList.busRespEvent;
    }

    SNOOPPrivEventType SNOOPPrivCohProtocol::GetCurrProcEvent () {
      return m_processEvent;
    }

    SNOOPPrivCtrlAction SNOOPPrivCohProtocol::GetCurrEventCtrlAction () { 
      return m_ctrlAction;
    }

    void SNOOPPrivCohProtocol::SetCurrEventCtrlAction (SNOOPPrivCtrlAction currEvent) {
      m_ctrlAction = currEvent;
    }

    int SNOOPPrivCohProtocol::GetCurrEventCacheNextState () {
      return m_currEventNextState;
    }

    int SNOOPPrivCohProtocol::GetCurrEventCacheCurrState () {
      return m_currEventCurrState;
    }

    SNOOPPrivCohTrans SNOOPPrivCohProtocol::GetCurrEventCohrTrans () {
      return m_currEventTrans2Issue;
    }

    void SNOOPPrivCohProtocol::ChkCoreEvent () {
      //std::cout<<" in ChkCoreEvent"<<endl;
       if (!m_cpuFIFO->m_txFIFO.IsEmpty()) {
         //std::cout<<" There is an EVENT on Core"<<endl;
         m_msgList.cpuReqMsg = m_cpuFIFO->m_txFIFO.GetFrontElement();
         if (m_msgList.cpuReqMsg.type == CpuFIFO::REQTYPE::READ) {
           m_eventList.cpuReqEvent = SNOOPPrivCoreEvent::Load;
         }
         else {
           m_eventList.cpuReqEvent = SNOOPPrivCoreEvent::Store;
         }
       }
       else {
         //cout<<"it is nll here"<<endl;
         m_eventList.cpuReqEvent = SNOOPPrivCoreEvent::Null;
       }
    }

    void SNOOPPrivCohProtocol::ChkReqBusEvent() {
       //std::cout<<" in ChkReqBusEvent "<<endl;
       if (!m_busIfFIFO->m_rxMsgFIFO.IsEmpty()) {
         //std::cout<<" There is an EVENT is Request"<<endl;
         m_msgList.busReqMsg = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();
         //std::cout<<" There is an EVENT is Request  got the front one"<<endl;
         int reqCoreId = m_msgList.busReqMsg.reqCoreId;
         //std::cout<<" There is an EVENT is Request the reqID is "<<reqCoreId<<endl;
         int wbCoreId = m_msgList.busReqMsg.wbCoreId;
         //std::cout<<" There is an EVENT is WbID the reqID is "<<wbCoreId<<endl;

         switch (m_msgList.busReqMsg.cohrMsgId) {
           case SNOOPPrivCohTrans::GetSTrans:
            // cout<<"GetSTrans"<<endl;
             m_eventList.busReqEvent = (reqCoreId == m_coreId) ? SNOOPPrivReqBusEvent::OwnGetS  : 
                                                                 SNOOPPrivReqBusEvent::OtherGetS;
             break;
           case SNOOPPrivCohTrans::GetMTrans:
           //cout<<"GetMTrans"<<endl;
             m_eventList.busReqEvent = (reqCoreId == m_coreId) ? SNOOPPrivReqBusEvent::OwnGetM  : 
                                                                 SNOOPPrivReqBusEvent::OtherGetM;
             break;
           case SNOOPPrivCohTrans::UpgTrans :
          // cout<<"UPGRADE"<<endl;
             m_eventList.busReqEvent = (reqCoreId == m_coreId) ? SNOOPPrivReqBusEvent::OwnUpg  : 
                                                                 SNOOPPrivReqBusEvent::OtherUpg;
             break;
           case SNOOPPrivCohTrans::PutMTrans:
          // cout<<"PUTM"<<endl;
             m_eventList.busReqEvent = (wbCoreId == m_coreId ) ? SNOOPPrivReqBusEvent::OwnPutM  : 
                                                                 SNOOPPrivReqBusEvent::OtherPutM;
             break;
           case SNOOPPrivCohTrans::PutSTrans:
          // cout<<"PUTS"<<endl;
             m_eventList.busReqEvent = (wbCoreId == m_coreId ) ? SNOOPPrivReqBusEvent::OwnPutS  : 
                                                                 SNOOPPrivReqBusEvent::OtherPutS;
             break;
           case SNOOPPrivCohTrans::ExclTrans:     
          // cout<<"EXCLUDE"<<endl;
             m_eventList.busReqEvent = (reqCoreId == m_coreId) ? SNOOPPrivReqBusEvent::OwnExclTrans  : 
                                                                 SNOOPPrivReqBusEvent::OtherExclTrans;
             break;
           case SNOOPPrivCohTrans::InvTrans: 
           //cout<<"Invalid"<<endl;
             m_eventList.busReqEvent = SNOOPPrivReqBusEvent::OwnInvTrans;
             break;
           default: // Invalid Transaction
             std::cout << " SNOOPPrivCohProtocol: Invalid Transaction detected on the Bus" << std::endl;
             exit(0);
           }
       }
       else {
       //  cout<<"m_eventList.busReqEvent is empty"<<endl;
         m_eventList.busReqEvent = SNOOPPrivReqBusEvent::Null;
       }
       if(m_eventList.busReqEvent != SNOOPPrivReqBusEvent::Null){
        // cout<<"m_eventList.busReqEvent is NOT Null"<<endl;
       }
    }

    void SNOOPPrivCohProtocol::ChkRespBusEvent () {
       //std::cout<<" in ChkRespBusEvent "<<endl;
       if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
        // std::cout<<" There is an EVENT is Response"<<endl;
         m_msgList.busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();


        // if (m_cache2Cache == false) {
        // m_eventList.busRespEvent =  (m_msgList.busRespMsg.reqCoreId  == m_coreId && 
        //                                         m_msgList.busRespMsg.respCoreId == m_sharedMemId) ? SNOOPPrivRespBusEvent::OwnDataResp : 
        //                                          SNOOPPrivRespBusEvent::OtherDataResp;
        // }
         if (m_cache2Cache == false) {
           m_eventList.busRespEvent =  (m_msgList.busRespMsg.reqCoreId  == m_coreId && 
                                        m_msgList.busRespMsg.respCoreId == m_msgList.busRespMsg.sharedCacheAgent) ? SNOOPPrivRespBusEvent::OwnDataResp : 
                                                                                            SNOOPPrivRespBusEvent::OtherDataResp;
         }
         else {
           m_eventList.busRespEvent = (m_msgList.busRespMsg.reqCoreId == m_coreId) ? SNOOPPrivRespBusEvent::OwnDataResp : 
                                                                                     SNOOPPrivRespBusEvent::OtherDataResp;
         }
       }
       else {
         //std::cout<<" in ChkRespBusEvent 3"<<endl;
         m_eventList.busRespEvent = SNOOPPrivRespBusEvent::Null;
       }
    }

    void SNOOPPrivCohProtocol::ChkCohEvents () {
    //  std::cout<<" in ChkCohEvents "<<endl;
       ChkCoreEvent ();  // is there any event on the core (LOAD or STORE or NULL  -- MSG is READ or WRITE)? if there is, it takes it
       ChkReqBusEvent(); // is there any event on the RX MSG buffer of the core?
       ChkRespBusEvent();// is there any event on the RX RESP buffer of the core?
       //std::cout<<" Finish ChkCohEvents "<<endl;
       
    }

    void SNOOPPrivCohProtocol::ProcessCoreEvent () {
      uint32_t replcWayIdx;
      int      emptyWayIdx;
      GenericCacheFrmt CacheLine;

      if (m_eventCacheInfoList.cpuReqCacheLineInfo.IsExist == false){
        if (m_eventCacheInfoList.cpuReqCacheLineInfo.IsSetFull == true) {
          m_eventList.cpuReqEvent = SNOOPPrivCoreEvent::Replacement;  
          replcWayIdx = m_privCache->GetReplacementLine(m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx, m_replcPolicy);
          CacheLine  = m_privCache->ReadCacheLine(m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx, replcWayIdx);
          m_eventCacheInfoList.cpuReqCacheLineInfo.way_idx = replcWayIdx;
          m_eventCacheInfoList.cpuReqCacheLineInfo.state   = CacheLine.state;
          m_msgList.cpuReqMsg.addr  = m_privCache->CpuPhyAddr(m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx, replcWayIdx);
          m_msgList.cpuReqMsg.msgId = 0; 
        }
        else {
          emptyWayIdx = m_privCache->GetEmptyCacheLine(m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx);
          if (emptyWayIdx == -1 ) {
            std::cout << "SNOOPPrivCohProtocol: Fault non-empty Set condition !!!" << std::endl;
            exit(0);
          }
          CacheLine  = m_privCache->ReadCacheLine(m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx, emptyWayIdx);
          m_eventCacheInfoList.cpuReqCacheLineInfo.way_idx = emptyWayIdx;
          m_eventCacheInfoList.cpuReqCacheLineInfo.state  = CacheLine.state;   
        }
      }
      m_currEventCurrState = m_eventCacheInfoList.cpuReqCacheLineInfo.state;
      m_currEventNextState = m_eventCacheInfoList.cpuReqCacheLineInfo.state;
      CohProtocolFSMProcessing ();
    }

    void SNOOPPrivCohProtocol::ProcessBusEvents (){
     // cout<<"void SNOOPPrivCohProtocol::ProcessBusEvents (){"<<endl;
      if (m_processEvent == SNOOPPrivEventType::ReqBus) {
       // cout<<"void SNOOPPrivCohProtocol::ProcessBusEvents (){  21"<<endl;
        m_currEventCurrState = m_eventCacheInfoList.busReqCacheLineInfo.state;
        m_currEventNextState = m_eventCacheInfoList.busReqCacheLineInfo.state;
      }
      else {
        m_currEventCurrState = m_eventCacheInfoList.busRespCacheLineInfo.state;
        m_currEventNextState = m_eventCacheInfoList.busRespCacheLineInfo.state;
      }

      if ((m_eventCacheInfoList.busReqCacheLineInfo.IsExist  == false && m_processEvent == SNOOPPrivEventType::ReqBus) ||
          (m_eventCacheInfoList.busRespCacheLineInfo.IsExist == false && m_processEvent == SNOOPPrivEventType::RespBus)) {
        m_currEventTrans2Issue = SNOOPPrivCohTrans::NullTrans;
        m_ctrlAction = SNOOPPrivCtrlAction::NoAck;
      }
      else {
        CohProtocolFSMProcessing ();
      }
    }

    void SNOOPPrivCohProtocol::CohProtocolFSMProcessing () {
      IFCohProtocol *ptr;

      switch (m_pType) {
        case CohProtType::SNOOP_PMSI: 
          ptr = new PMSI; 
          break;
        case CohProtType::SNOOP_MSI:
          ptr = new MSI; 
          break;
        case CohProtType::SNOOP_MESI:
          ptr = new MESI; 
          break;
        case CohProtType::SNOOP_MOESI:
          ptr = new MOESI; 
          break;
        default:
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
        
        CohProtocolFSMBinding(ptr);
        delete ptr;
    }

    void SNOOPPrivCohProtocol::CohProtocolFSMBinding (IFCohProtocol *obj){
      return obj->SNOOPPrivEventProcessing 
                    (m_processEvent, 
                     m_eventList, 
                     m_currEventNextState, 
                     m_currEventTrans2Issue, 
                     m_ctrlAction, 
                     m_cache2Cache    
                    );
    }

    int SNOOPPrivCohProtocol::InvalidCacheState () {
      switch (m_pType) {
        case CohProtType::SNOOP_PMSI: 
          return static_cast<int>(SNOOP_PMSIPrivCacheState::I);
        case CohProtType::SNOOP_MSI:
          return static_cast<int>(SNOOP_MSIPrivCacheState::I);
        case CohProtType::SNOOP_MESI:
          return static_cast<int>(SNOOP_MESIPrivCacheState::I);
        case CohProtType::SNOOP_MOESI:
          return static_cast<int>(SNOOP_MOESIPrivCacheState::I);
        default:
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
    }

    void SNOOPPrivCohProtocol::InitializeCacheStates () {
       int initState = InvalidCacheState();
        m_privCache->InitalizeCacheStates(initState);
    }

    void SNOOPPrivCohProtocol::GetEventsCacheInfo () {
      if (m_eventList.cpuReqEvent != SNOOPPrivCoreEvent::Null) {
        m_eventCacheInfoList.cpuReqCacheLineInfo  = m_privCache->GetCacheLineInfo(m_msgList.cpuReqMsg.addr);
      }
      if (m_eventList.busReqEvent != SNOOPPrivReqBusEvent::Null) {
        m_eventCacheInfoList.busReqCacheLineInfo  = m_privCache->GetCacheLineInfo(m_msgList.busReqMsg.addr);
      }
      if (m_eventList.busRespEvent != SNOOPPrivRespBusEvent::Null) {
        m_eventCacheInfoList.busRespCacheLineInfo  = m_privCache->GetCacheLineInfo(m_msgList.busRespMsg.addr);
      }
    }

    SNOOPPrivEventPriority SNOOPPrivCohProtocol::PrivEventPriorityBinding (IFCohProtocol *obj){
      return obj->PrivCacheEventPriority 
                    (m_eventList, 
                     m_eventCacheInfoList 
                    );
    }

    void SNOOPPrivCohProtocol::CohEventsSerialize () {
      SNOOPPrivEventPriority eventPriority;
      IFCohProtocol *ptr;
      switch (m_pType) {
        case CohProtType::SNOOP_PMSI: 
          ptr = new PMSI; 
          break;
        case CohProtType::SNOOP_MSI:
          ptr = new MSI; 
          break;
        case CohProtType::SNOOP_MESI:
          ptr = new MESI; 
          break;
        case CohProtType::SNOOP_MOESI:
          ptr = new MOESI; 
          break;                    
        default:
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      
      eventPriority = PrivEventPriorityBinding(ptr); 
      if(m_reza_log_snoop) {
        if(eventPriority == SNOOPPrivEventPriority::ReqBus) cout<<"priority req"<<endl;
        else if (eventPriority == SNOOPPrivEventPriority::RespBus) cout<<"priority resp"<<endl;
        else cout<<"priority work conserving"<<endl;
      }
      delete ptr;
      if(m_reza_log_snoop) {
        if(m_eventList.busReqEvent != SNOOPPrivReqBusEvent::Null) cout<<"There is a ReqBus event list"<<endl;
        if (m_eventList.busRespEvent != SNOOPPrivRespBusEvent::Null) cout<<"There is a RespBus event list"<<endl;
        if (m_eventList.cpuReqEvent != SNOOPPrivCoreEvent::Null) cout<<"There is a CPU event list"<<endl;         
      }
      /** If there are multiple events on the same core, they will be serilize and then issue on cycles after cycles. ****/

      m_processEvent = SNOOPPrivEventType::Null;
    //cout<<"1"<<endl;
      if (m_eventList.busReqEvent  != SNOOPPrivReqBusEvent::Null && 
          m_eventList.busRespEvent != SNOOPPrivRespBusEvent::Null){
           if(m_reza_log_snoop)  cout<<"22222222222222222222222222222222222222222"<<endl;

        //m_processEvent = (uRnd1->GetValue() <= 50) ? SNOOPPrivEventType::ReqBus : SNOOPPrivEventType::RespBus;
        m_processEvent = SNOOPPrivEventType::ReqBus;
      }
      else if (m_eventList.busReqEvent  != SNOOPPrivReqBusEvent::Null) {
        //cout<<"3"<<endl;
        m_processEvent = SNOOPPrivEventType::ReqBus;
      }
      else if (m_eventList.busRespEvent  != SNOOPPrivRespBusEvent::Null) {
       //cout<<"4"<<endl;
        m_processEvent = SNOOPPrivEventType::RespBus;
      }
      //cout<<"5"<<endl;

      if (eventPriority == SNOOPPrivEventPriority::ReqBus) {
       // cout<<"6"<<endl;
        m_processEvent = SNOOPPrivEventType::ReqBus;
      }
      else if (eventPriority == SNOOPPrivEventPriority::RespBus) {
        //cout<<"7"<<endl;
        m_processEvent = SNOOPPrivEventType::RespBus;
      }
      else if (eventPriority == SNOOPPrivEventPriority::WorkConserv) {
        //cout<<"8"<<endl;
        if (m_eventList.cpuReqEvent != SNOOPPrivCoreEvent::Null){
         // cout<<"9"<<endl;
          if (m_eventList.busReqEvent != SNOOPPrivReqBusEvent::Null || 
              m_eventList.busRespEvent != SNOOPPrivRespBusEvent::Null) {
            m_prllActionCnt++;
            if (m_prllActionCnt  > m_reqWbRatio) {
              m_prllActionCnt = 0;
            }
            else {
              //cout<<"10"<<endl;
              m_processEvent = SNOOPPrivEventType::Core;
            }
          }
          else {
           // cout<<"11"<<endl;
            m_processEvent = SNOOPPrivEventType::Core;
          }
        }
      }  

    }

    GenericCache::CacheLineInfo SNOOPPrivCohProtocol::GetCurrEventCacheLineInfo () {
      switch (m_processEvent) {
        case SNOOPPrivEventType::Core:
          return m_eventCacheInfoList.cpuReqCacheLineInfo;
        case SNOOPPrivEventType::ReqBus:
          return m_eventCacheInfoList.busReqCacheLineInfo;
        case SNOOPPrivEventType::RespBus:
          return m_eventCacheInfoList.busRespCacheLineInfo;
        case SNOOPPrivEventType::Null:
          return m_eventCacheInfoList.cpuReqCacheLineInfo;
        default:
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Event" << std::endl;
          exit(0);
      }
    }
    
    void SNOOPPrivCohProtocol::UpdateCacheLine (CacheField       field, 
                                                GenericCacheFrmt cacheLineInfo, 
                                                uint32_t         set_idx, 
                                                uint32_t         way_idx) {
      if (field == State) {
        m_privCache->SetCacheLineState (set_idx, way_idx, cacheLineInfo);
      }
      else if (field == Line) {
        m_privCache->WriteCacheLine(set_idx, way_idx,cacheLineInfo);
      }
    }

    bool SNOOPPrivCohProtocol::IsCacheBlkValid (int state) {
      IFCohProtocol *obj;
      bool VldBlk = false;
      switch (m_pType) {
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
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      VldBlk = obj->IsValidBlk (state);   
      delete obj;
      return VldBlk;
    }

    void SNOOPPrivCohProtocol::PrintEventInfo () {
      if (m_processEvent == SNOOPPrivEventType::Core) {
        std::cout << "\nSNOOPPrivCohProtocol: CoreId = " << m_coreId << " has Cpu ReqAction" << std::endl;
        std::cout << "\t\t Cpu Req Addr = " << m_msgList.cpuReqMsg.addr << " Type(0:Read) = " << m_msgList.cpuReqMsg.type << " CacheLine = " << m_eventCacheInfoList.cpuReqCacheLineInfo.cl_idx << std::endl;
        std::cout << "\t\t CacheLine Info: \n\t\t\t IsExist         =  " << m_eventCacheInfoList.cpuReqCacheLineInfo.IsExist << " \n\t\t\t IsValid         =  " << m_eventCacheInfoList.cpuReqCacheLineInfo.IsValid << " \n\t\t\t IsSetFull       =  " << m_eventCacheInfoList.cpuReqCacheLineInfo.IsSetFull << " \n\t\t\t Set_Idx         =  " << m_eventCacheInfoList.cpuReqCacheLineInfo.set_idx << " \n\t\t\t Way_Idx         =  " << m_eventCacheInfoList.cpuReqCacheLineInfo.way_idx <<  std::endl;
        std::cout << "\t\t\t CurrState       = " << PrintPrivStateName(m_currEventCurrState) << std::endl;
        std::cout << "\t\t\t NextState       = " << PrintPrivStateName(m_currEventNextState) << std::endl;
        std::cout << "\t\t\t Ctrl ReqAction  = " << PrintPrivActionName(m_ctrlAction) << std::endl;
        std::cout << "\t\t\t Ctrl ReqTrans   = " << PrintTransName(m_currEventTrans2Issue) << std::endl;
      }

      if (m_processEvent == SNOOPPrivEventType::ReqBus) {
        std::cout << "\nSNOOPPrivCohProtocol: CoreId = " << m_coreId << " has Msg on the RxReq Bus" << std::endl;
        std::cout << "\t\t BusEventName        = " << PrivReqBusEventName(m_eventList.busReqEvent) << std::endl;
        std::cout << "\t\t ReqCoreId           = " << m_msgList.busReqMsg.reqCoreId << ", RespCoreId = " << m_msgList.busReqMsg.wbCoreId << ", Agent = " << m_msgList.busReqMsg.sharedCacheAgent <<   ", ReqMsgId = " << m_msgList.busReqMsg.msgId << ", Req Addr  = " << m_msgList.busReqMsg.addr << " CacheLine = " << m_eventCacheInfoList.busReqCacheLineInfo.cl_idx << std::endl;
        std::cout << "\t\t CacheLine Info: \n\t\t\t IsExist         =  " << m_eventCacheInfoList.busReqCacheLineInfo.IsExist << " \n\t\t\t IsValid         =  " << m_eventCacheInfoList.busReqCacheLineInfo.IsValid << " \n\t\t\t Set_Idx         =  " << m_eventCacheInfoList.busReqCacheLineInfo.set_idx << " \n\t\t\t Way_Idx         =  " << m_eventCacheInfoList.busReqCacheLineInfo.way_idx << std::endl;
        std::cout << "\t\t\t CurrState       = " << PrintPrivStateName(m_currEventCurrState) << std::endl;
        std::cout << "\t\t\t NextState       = " << PrintPrivStateName(m_currEventNextState) << std::endl;
        std::cout << "\t\t\t Ctrl ReqAction  = " << PrintPrivActionName(m_ctrlAction) << std::endl;
        std::cout << "\t\t\t Ctrl ReqTrans   = " << PrintTransName(m_currEventTrans2Issue) << std::endl;
      }

      if (m_processEvent == SNOOPPrivEventType::RespBus) {
        std::cout << "\nSNOOPPrivCohProtocol: CoreId = " << m_coreId << " has Msg on the RxResp Bus" << std::endl;
        std::cout << "\t\t BusEventName        = " << PrivRespBusEventName(m_eventList.busRespEvent) << std::endl;
        if(m_msgList.busRespMsg.respCoreId >= 10 )
          std::cout << "\t\t ReqCoreId           = " << m_msgList.busRespMsg.reqCoreId << ", RespCoreId = " << m_msgList.busRespMsg.respCoreId << ", Agent = " << m_msgList.busRespMsg.sharedCacheAgent << ", ReqMsgId = " << m_msgList.busRespMsg.msgId << ", Req Addr  = " << m_msgList.busRespMsg.addr << " CacheLine = " << m_eventCacheInfoList.busRespCacheLineInfo.cl_idx << std::endl;
        else 
          std::cout << "\t\t ReqCoreId           = " << m_msgList.busRespMsg.reqCoreId << ", RespCoreId = " << m_msgList.busRespMsg.respCoreId << ", Agent = " << m_msgList.busRespMsg.respCoreId << ", ReqMsgId = " << m_msgList.busRespMsg.msgId << ", Req Addr  = " << m_msgList.busRespMsg.addr << " CacheLine = " << m_eventCacheInfoList.busRespCacheLineInfo.cl_idx << std::endl;
        std::cout << "\t\t CacheLine Info: \n\t\t\t IsExist         =  " << m_eventCacheInfoList.busRespCacheLineInfo.IsExist << " \n\t\t\t IsValid         =  " << m_eventCacheInfoList.busRespCacheLineInfo.IsValid <<  " \n\t\t\t Set_Idx         =  " << m_eventCacheInfoList.busRespCacheLineInfo.set_idx << " \n\t\t\t Way_Idx         =  " << m_eventCacheInfoList.busRespCacheLineInfo.way_idx << std::endl;
        std::cout << "\t\t\t CurrState       = " << PrintPrivStateName(m_currEventCurrState) << std::endl;
        std::cout << "\t\t\t NextState       = " << PrintPrivStateName(m_currEventNextState) << std::endl;
        std::cout << "\t\t\t Ctrl ReqAction  = " << PrintPrivActionName(m_ctrlAction) << std::endl;
        std::cout << "\t\t\t Ctrl ReqTrans   = " << PrintTransName(m_currEventTrans2Issue) << std::endl;
      }
    }

    std::string SNOOPPrivCohProtocol::PrintPrivStateName (int state) {
      IFCohProtocol *obj;
      std::string sName;
      switch (m_pType) {
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
          std::cout << "SNOOPPrivCohProtocol: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      sName = obj->PrivStateName (state);
      delete obj;
      return sName;
    }

    std::string SNOOPPrivCohProtocol::PrintPrivEventType (SNOOPPrivEventType event) {
       switch (event) {
         case SNOOPPrivEventType::Core:
           return " SNOOPPrivCohProtocol: Core Event is currently processed "; 
         case SNOOPPrivEventType::ReqBus:
           return " SNOOPPrivCohProtocol: ReqBus Event is currently processed "; 
         case SNOOPPrivEventType::RespBus:
           return " SNOOPPrivCohProtocol: RespBus Event is currently processed "; 
         case SNOOPPrivEventType::Null:
           return " SNOOPPrivCohProtocol: No Events currently processed "; 
         default:
           return " SNOOPPrivCohProtocol: Unknown Event!!!!";
       }
     }

     std::string SNOOPPrivCohProtocol::PrintPrivActionName (SNOOPPrivCtrlAction action)
     {
       switch (action) {
         case SNOOPPrivCtrlAction::Stall:
           return " Stall"; 
         case SNOOPPrivCtrlAction::Hit:
           return " Hit";
         case SNOOPPrivCtrlAction::issueTrans:
           return " issueTrans";
         case SNOOPPrivCtrlAction::ReissueTrans:
           return " ReissueTrans";
         case SNOOPPrivCtrlAction::issueTransSaveWbId:
           return " issueTransSaveWbId";
         case SNOOPPrivCtrlAction::WritBack:
           return " WritBack";
         case SNOOPPrivCtrlAction::CopyThenHit:
           return " CopyThenHit" ;
         case SNOOPPrivCtrlAction::CopyThenHitWB:
           return " CopyThenHitWB" ;
         case SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly:
           return " CopyThenHitSendCoreOnly" ;
         case SNOOPPrivCtrlAction::CopyThenHitSendMemOnly:
           return " CopyThenHitSendMemOnly" ;
         case SNOOPPrivCtrlAction::CopyThenHitSendCoreMem:
           return " CopyThenHitSendCoreMem" ;
         case SNOOPPrivCtrlAction::SaveWbCoreId:
           return " SaveWbCoreId" ;
         case SNOOPPrivCtrlAction::HitSendMemOnly:
           return " HitSendMemOnly" ;
         case SNOOPPrivCtrlAction::SendMemOnly:
           return " SendMemOnly" ;
         case SNOOPPrivCtrlAction::SendCoreOnly:
           return " SendCoreOnly" ;
         case SNOOPPrivCtrlAction::SendCoreMem:
           return " SendCoreMem" ;
         case SNOOPPrivCtrlAction::Fault:
           return " Fault" ;
         case SNOOPPrivCtrlAction::NoAck:
           return " NoAck";
         case SNOOPPrivCtrlAction::NullAck:
           return " NullAck";
         case SNOOPPrivCtrlAction::ProcessedAck:
           return " ProcessedAck";
         default: 
           return " Unknow Action";
       }
     }

     std::string SNOOPPrivCohProtocol::PrintTransName (SNOOPPrivCohTrans trans)
     {
       switch (trans) {
         case GetSTrans:
           return " GetSTrans "; 
         case GetMTrans:
           return " GetMTrans ";
         case UpgTrans:
           return " UpgTrans ";
         case PutMTrans:
           return " PutMTrans ";
         case PutSTrans:
           return " PutSTrans " ;
         case NullTrans:
           return " NullTrans ";
         default: // NullTrans:
           return " Unknow Trans";
       }
     }

     std::string SNOOPPrivCohProtocol::PrivReqBusEventName (SNOOPPrivReqBusEvent event)
     {
       switch (event) {
         case SNOOPPrivReqBusEvent::OwnGetS:
           return " OwnGetSEvent "; 
         case SNOOPPrivReqBusEvent::OwnGetM:
           return " OwnGetMEvent ";
         case SNOOPPrivReqBusEvent::OwnPutM:
           return " OwnPutMEvent ";
         case SNOOPPrivReqBusEvent::OwnPutS:
           return " OwnPutSEvent ";
         case SNOOPPrivReqBusEvent::OwnUpg:
           return " OwnUpgEvent ";
         case SNOOPPrivReqBusEvent::OtherGetS:
           return " OtherGetSEvent " ;
         case SNOOPPrivReqBusEvent::OtherGetM:
           return " OtherGetMEvent ";
         case SNOOPPrivReqBusEvent::OtherPutS:
           return " OtherPutSEvent ";
         case SNOOPPrivReqBusEvent::OtherPutM:
           return " OtherPutMEvent ";
         case SNOOPPrivReqBusEvent::OtherUpg:
           return " OtherUpgEvent ";
         case SNOOPPrivReqBusEvent::OwnExclTrans:
           return " OwnExclTrans ";  
         case SNOOPPrivReqBusEvent::OtherExclTrans:
           return " OtherExclTrans ";  
         case SNOOPPrivReqBusEvent::OwnInvTrans:
           return " OwnInvTrans ";  
         case SNOOPPrivReqBusEvent::Null:
           return " NullEvent ";
         default:
           return " Unknown !!!!";
       }
     }

     std::string SNOOPPrivCohProtocol::PrivRespBusEventName (SNOOPPrivRespBusEvent event)
     {
       switch (event) {
         case SNOOPPrivRespBusEvent::OwnDataResp:
           return " OwnDataRespEvent "; 
         case SNOOPPrivRespBusEvent::OwnDataRespExclusive:
           return " OwnDataRespExclusive "; 
         case SNOOPPrivRespBusEvent::OtherDataResp:
           return " OtherDataRespEvent ";
         case SNOOPPrivRespBusEvent::Null:
           return " NullRespEvent ";
         default:
           return " Unknown !!!";
       }
     }

    SNOOPPrivCohTrans SNOOPPrivCohProtocol::GetCohTrans () { return m_currEventTrans2Issue;};
}
