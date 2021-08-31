/*
 * File  :      PMSI.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 18, 2020
 */

#include "PMSI.h"

namespace ns3 {

    // override ns3 type
    TypeId PMSI::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PMSI")
               .SetParent<Object > ();
        return tid;
    }

    PMSI::PMSI() {
        // default
    }

    // We don't do any dynamic allocations
    PMSI::~PMSI() {
    }


    // This function implements the PMSI
    // state machine according to table 1
    // in "Predictable Cache Coherence for
    // Multi-core Real Time System" paper
     void PMSI::SNOOPPrivEventProcessing 
          (SNOOPPrivEventType  eventType, 
           SNOOPPrivEventList  eventList,
           int                 &cacheState,       
           SNOOPPrivCohTrans   &trans2Issue,
           SNOOPPrivCtrlAction &ctrlAction,
           bool                cache2Cache
           )
      {
       // current and next state variable
       SNOOP_PMSIPrivCacheState cacheState_c, cacheState_n; 

       cacheState_c = static_cast<SNOOP_PMSIPrivCacheState>(cacheState);

       // default assignments
       trans2Issue    = NullTrans;
       ctrlAction     = SNOOPPrivCtrlAction::NoAck;
       cacheState_n   = cacheState_c;

       // process CoreEvent
       if (eventType == SNOOPPrivEventType::Core) {
         switch (eventList.cpuReqEvent) {
           case SNOOPPrivCoreEvent::Load:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IS_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetSTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::S:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_PMSIPrivCacheState::M:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::SM_w:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S: 
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Store:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::S:
                 //cacheState_n   = SNOOP_PMSIPrivCacheState::SM_w;
                 //trans2Issue    = UpgTrans;
                 //ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::M:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb:
                 ctrlAction  = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::SM_w:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S: 
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Replacement:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::SM_w:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S: 
                 ctrlAction   = SNOOPPrivCtrlAction::Stall;
                 break;
               case SNOOP_PMSIPrivCacheState::MI_wb:
                 ctrlAction   = SNOOPPrivCtrlAction::Stall;
                 break;
               case SNOOP_PMSIPrivCacheState::S: // self invalidate
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 break;
               case SNOOP_PMSIPrivCacheState::M:
                 cacheState_n   = SNOOP_PMSIPrivCacheState::MI_wb;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::MS_wb:
                 cacheState_n = SNOOP_PMSIPrivCacheState::MI_wb;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           default: // CoreNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPPrivCtrlAction::NullAck;
         }

       }
       // process BusEvent
       else if (eventType == SNOOPPrivEventType::ReqBus) {
         switch (eventList.busReqEvent) {
           case SNOOPPrivReqBusEvent::OwnGetS:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::IS_a_d: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IS_d;
                 break;  
               case SNOOP_PMSIPrivCacheState::IS_a: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnGetM:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::IM_a_d: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_d;
                 break;  
               case SNOOP_PMSIPrivCacheState::IM_a: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnPutM:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::M:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::SM_w:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::S: 
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S: 
                 break;
               case SNOOP_PMSIPrivCacheState::MI_wb:
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 //ctrlAction   = SNOOPPrivCtrlAction::WritBack;
                 ctrlAction     = cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_PMSIPrivCacheState::MS_wb:
                 cacheState_n = SNOOP_PMSIPrivCacheState::S;
                 //ctrlAction   = SNOOPPrivCtrlAction::WritBack;
                 ctrlAction     = cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnUpg:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::S: 
               case SNOOP_PMSIPrivCacheState::M:
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb: 
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_PMSIPrivCacheState::SM_w:
                 cacheState_n = SNOOP_PMSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::Hit;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetS:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::S:     
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb: 
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S:
                 break;
               case SNOOP_PMSIPrivCacheState::SM_w: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::ReissueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::M:
                 cacheState_n   = SNOOP_PMSIPrivCacheState::MS_wb;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_d_S;
                 trans2Issue  = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTransSaveWbId;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetM:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a:
                 break;
               case SNOOP_PMSIPrivCacheState::S: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 break;
               case SNOOP_PMSIPrivCacheState::M:
                 cacheState_n = SNOOP_PMSIPrivCacheState::MI_wb;
                 trans2Issue  = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_d_I;
                 trans2Issue  = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTransSaveWbId;
                 break;
               case SNOOP_PMSIPrivCacheState::SM_w:
                 //cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::ReissueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::MS_wb:
                 cacheState_n = SNOOP_PMSIPrivCacheState::MI_wb;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d_S: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_d_I;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherPutM:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S:
               case SNOOP_PMSIPrivCacheState::MI_wb:
                 break;
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::S: 
               case SNOOP_PMSIPrivCacheState::M:    
               case SNOOP_PMSIPrivCacheState::SM_w: 
               case SNOOP_PMSIPrivCacheState::MS_wb: 
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherUpg:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::IS_a_d:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IM_a_d:
                 break;
               case SNOOP_PMSIPrivCacheState::S: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 break;
               case SNOOP_PMSIPrivCacheState::SM_w: 
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a_d;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::ReissueTrans;
                 break;
               case SNOOP_PMSIPrivCacheState::M:  
               case SNOOP_PMSIPrivCacheState::IM_a:  
               case SNOOP_PMSIPrivCacheState::IM_d:
               case SNOOP_PMSIPrivCacheState::IS_d:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb: 
               case SNOOP_PMSIPrivCacheState::IM_d_I:
               case SNOOP_PMSIPrivCacheState::IS_d_I:
               case SNOOP_PMSIPrivCacheState::IM_d_S:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           default: // BusNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPPrivCtrlAction::NullAck;
         } // switch (reqbusEvent) {
       } // else if (eventType == SNOOPPrivEventType::ReqBus)
       else if(eventType == SNOOPPrivEventType::RespBus) {
         switch (eventList.busRespEvent) {
           case SNOOPPrivRespBusEvent::OwnDataResp:{
             switch (cacheState_c) {
               case SNOOP_PMSIPrivCacheState::I:
               case SNOOP_PMSIPrivCacheState::S:     
               case SNOOP_PMSIPrivCacheState::M:
               case SNOOP_PMSIPrivCacheState::IS_a:
               case SNOOP_PMSIPrivCacheState::IM_a:
               case SNOOP_PMSIPrivCacheState::MI_wb:
               case SNOOP_PMSIPrivCacheState::MS_wb: 
               case SNOOP_PMSIPrivCacheState::SM_w: 
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_a_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IS_a;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_a_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::IM_a;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_PMSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d_I:
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 //ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitWB;
                 ctrlAction   = cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_PMSIPrivCacheState::IS_d_I:
                 cacheState_n = SNOOP_PMSIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_PMSIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_PMSIPrivCacheState::S;
                 //ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitWB;
                 ctrlAction   = cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivRespBusEvent::OtherDataResp:{
             break;
           }
           default: // BusOtherDataRespEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPPrivCtrlAction::NullAck;
         }
       }// else if (eventType == RespBusEvent)
       cacheState = static_cast<int>(cacheState_n);
      }

    SNOOPPrivEventPriority PMSI::PrivCacheEventPriority (SNOOPPrivEventList eventList,SNOOPPrivEventsCacheInfo eventCacheInfoList ) {
       SNOOPPrivEventPriority eventPriority = SNOOPPrivEventPriority::WorkConserv;
       if (eventList.busRespEvent == SNOOPPrivRespBusEvent::OwnDataResp) {
         eventPriority = SNOOPPrivEventPriority::RespBus;
       }
       else if (eventList.busReqEvent != SNOOPPrivReqBusEvent::Null ) {
         eventPriority = SNOOPPrivEventPriority::ReqBus;
       }
       return eventPriority;
    }

    bool PMSI::IsValidBlk (int s) {
       SNOOP_PMSIPrivCacheState state = static_cast<SNOOP_PMSIPrivCacheState>(s);
       switch (state) {
         case SNOOP_PMSIPrivCacheState::S:
         case SNOOP_PMSIPrivCacheState::M:
         case SNOOP_PMSIPrivCacheState::MI_wb:
         case SNOOP_PMSIPrivCacheState::MS_wb:
           return true;
         default:
           return false;
       }
    }

     std::string PMSI::PrivStateName (int s)
     {
       SNOOP_PMSIPrivCacheState state = static_cast<SNOOP_PMSIPrivCacheState>(s);

       switch (state) {
         case SNOOP_PMSIPrivCacheState::I:
           return " I "; 
         case SNOOP_PMSIPrivCacheState::S:
           return " S ";
         case SNOOP_PMSIPrivCacheState::M:
           return " M ";
         case SNOOP_PMSIPrivCacheState::IS_a_d:
           return " I_S_a_d (waiting for GET(S) command to be issued on bus) ";
         case SNOOP_PMSIPrivCacheState::IS_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_PMSIPrivCacheState::IS_d:
           return " I_S_d (waiting data resp) ";
         case SNOOP_PMSIPrivCacheState::IM_a_d:
           return " I_M_a_d (waiting for GET(M) command to be issued on bus) ";
         case SNOOP_PMSIPrivCacheState::IM_a:
           return " I_M_a (waiting for GET(M) command to be issued on bus, data has been received) ";
         case SNOOP_PMSIPrivCacheState::IM_d:
           return " I_M_d (waiting data resp) ";
         case SNOOP_PMSIPrivCacheState::SM_w:
           return " S_M_w (waiting core slot) " ;
         case SNOOP_PMSIPrivCacheState::MI_wb:
           return " M_wb_I (waiting to WB) ";
         case SNOOP_PMSIPrivCacheState::MS_wb:
           return " M_wb_S (waiting to WB, then move to S) ";
         case SNOOP_PMSIPrivCacheState::IM_d_I:
           return " IM_d_I (waiting for data, then do Hit, then move to I";
         case SNOOP_PMSIPrivCacheState::IS_d_I:
           return " IS_d_I (waiting for data, then do Hit, then move to I)";
         default: //SNOOP_PMSIPrivCacheState::IM_d_S:
           return " IM_d_S (waiting for data, then do Hit, then move to S)";
       }
     }
     
     std::string PMSI::SharedStateName (int s)
     {
       SNOOP_PMSISharedCacheState state = static_cast<SNOOP_PMSISharedCacheState>(s);
       switch (state) {
         case SNOOP_PMSISharedCacheState::IorS:
           return " IorS (Shared Mem is the Owner) "; 
         case SNOOP_PMSISharedCacheState::M:
           return " M (Shared Mem content is dirty) ";
         case SNOOP_PMSISharedCacheState::M_d_IorS:
           return " M_d_IorS (Waiting for Data then Move to IorS) ";
         case SNOOP_PMSISharedCacheState::M_d_M:
           return " M_d_M (Waiting for Data then Move to M) ";
         case SNOOP_PMSISharedCacheState::IorSorM_a:
           return " IorSorM_a (Waiting for Data then Move to M or IorS) ";
         default:
           return " MSI: Error Undefine State";
       }
     }

    void PMSI::SNOOPSharedEventProcessing (SNOOPSharedEventType        eventType, 
                                          bool                         Cache2Cache,
                                          SNOOPSharedEventList         eventList, 
                                          int                         &cacheState, 
                                          SNOOPSharedOwnerState       &ownerState,
                                          SNOOPSharedCtrlAction       &ctrlAction) {

       // current and next state variable
       SNOOP_PMSISharedCacheState cacheState_c, 
                                  cacheState_n; 
       SNOOPSharedOwnerState      ownerState_n; 

       cacheState_c = static_cast<SNOOP_PMSISharedCacheState>(cacheState);

       // default assignments
       ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
       cacheState_n = cacheState_c;
       ownerState_n = SNOOPSharedOwnerState::SkipSet;

       // process ReqBusEvent
       if (eventType == SNOOPSharedEventType::ReqBus) {
         // process ReqBusEvent
         switch (eventList.busReqEvent) {
           case SNOOPSharedReqBusEvent::GetS:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 cacheState_n = SNOOP_PMSISharedCacheState::M_d_IorS;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorSorM_a:
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               default:
                 std::cout << "PMSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::GetM:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = Cache2Cache ? SNOOP_PMSISharedCacheState::M :
                                              SNOOP_PMSISharedCacheState::M_d_M;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorSorM_a:
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 break;
               default:
                 std::cout << "PMSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OwnerPutM:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 cacheState_n = SNOOP_PMSISharedCacheState::M_d_IorS;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorSorM_a:
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               default:
                 std::cout << "PMSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OTherPutM: {
             switch (cacheState_c) { 
               case SNOOP_PMSISharedCacheState::IorS: 
               case SNOOP_PMSISharedCacheState::M: 
               case SNOOP_PMSISharedCacheState::M_d_IorS: 
               case SNOOP_PMSISharedCacheState::M_d_M: 
               case SNOOP_PMSISharedCacheState::IorSorM_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedNoAck;
                 break;
               default:
                 std::cout << "PMSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           default: // BusNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPSharedCtrlAction::SharedNullAck;
         }
       }
       else if (eventType == SNOOPSharedEventType::RespBus) {
         switch (eventList.busRespEvent) {
           case SNOOPSharedRespBusEvent::OWnDataResp:{
             ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
             break;
           }
           case SNOOPSharedRespBusEvent::OTherDataResp:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
               case SNOOP_PMSISharedCacheState::M:
                 //ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorSorM_a;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 break;
               default:
                 std::cout << "PMSI::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           default: // RespNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPSharedCtrlAction::SharedNullAck;
         }
       } // else if (eventType == SNOOPSharedEventType::RespBus)
       cacheState = static_cast<int>(cacheState_n);
       ownerState = ownerState_n;
    } // PMSI::SharedCacheEventProcessing

/*
    void PMSI::SharedCacheEventProcessing (SNOOPSharedEventType         eventType, 
                                           bool                         Cache2Cache,
                                           SNOOPSharedReqBusEvent       reqbusEvent, 
                                           SNOOPSharedRespBusEvent      respbusEvent, 
                                           int                        & cacheState, 
                                           PMSIOwnerState   &      ownerState,
                                           PMSISharedAction &      ctrlAction) {

       // current and next state variable
       SNOOP_PMSISharedCacheState cacheState_c, cacheState_n; 
       PMSIOwnerState        ownerState_c, ownerState_n; 

       cacheState_c = static_cast<SNOOP_PMSISharedCacheState>(cacheState);

       ownerState_c = ownerState;

       // default assignments
       ctrlAction   = SharedNoAck;
       cacheState_n = cacheState_c;
       //ownerState_n = ownerState_c;
       ownerState_n = SkipSet;
       // process ReqBusEvent
       if (eventType == SNOOPSharedEventType::ReqBus) {
         // process ReqBusEvent
         switch (reqbusEvent) {
           case SNOOPSharedReqBusEvent::GetS:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction   = SendData;
                 ownerState_n = SharedMem;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ctrlAction   = Cache2Cache ? SharedNoAck : SaveReqCoreId;
                 ownerState_n = SharedMem;
                 cacheState_n = SNOOP_PMSISharedCacheState::M_d_IorS;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
               case SNOOP_PMSISharedCacheState::IorS_d: 
                 ctrlAction   = SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorS_A:
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 ownerState_n = SharedMem;
                 break;
               default:
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::GetM:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction   = SendData;
                 ownerState_n = OtherCore;
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ctrlAction   = Cache2Cache ? SharedNoAck : SaveReqCoreId;
                 ownerState_n = OtherCore;
                 cacheState_n = SNOOP_PMSISharedCacheState::M_d_M;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
               case SNOOP_PMSISharedCacheState::IorS_d: 
                 ctrlAction   = SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorS_A:
                 ctrlAction   = Cache2Cache ? SharedNoAck : SaveReqCoreId;
                 ownerState_n = OtherCore;
                 break;
               default:
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OwnerPutM:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
               case SNOOP_PMSISharedCacheState::IorS_d: 
                 ctrlAction  = SharedFault;
                 break;
               case SNOOP_PMSISharedCacheState::M:
                 ownerState_n = SharedMem;
                 ctrlAction   = SharedNoAck;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS_d;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = SharedStall;
                 break;
               case SNOOP_PMSISharedCacheState::IorS_A:
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 break;
               default:
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OTherPutM: {
             switch (cacheState_c) { 
               case SNOOP_PMSISharedCacheState::IorS_d: 
                 ctrlAction  = SharedFault;
                 break;
               case SNOOP_PMSISharedCacheState::IorS_A:
                 ctrlAction  = SendPendingData;
                 if (ownerState_c == SharedMem) {
                   cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 } 
                 else {
                   cacheState_n = SNOOP_PMSISharedCacheState::M;
                 }
                 break;
               default:
                 ctrlAction  = SharedNoAck;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::PutS: 
             ctrlAction   = SharedNoAck;
             break;
           case SNOOPSharedReqBusEvent::Upg:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS: 
                 ctrlAction   = SharedNoAck;
                 ownerState_n = OtherCore;
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 break;
               case SNOOP_PMSISharedCacheState::M:
               case SNOOP_PMSISharedCacheState::M_d_IorS:
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction  = SharedFault;
                 break;
               default:
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           default: // BusNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SharedNullAck;
         }
       }
       else if (eventType == SNOOPSharedEventType::RespBus) {
         switch (respbusEvent) {
           case SNOOPSharedRespBusEvent::OWnDataResp:{
             ctrlAction   = SharedNoAck;
             break;
           }
           case SNOOPSharedRespBusEvent::OTherDataResp:{
             switch (cacheState_c) {
               case SNOOP_PMSISharedCacheState::IorS_d: 
                 ctrlAction   = StoreDataOnly;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 break;
               case SNOOP_PMSISharedCacheState::IorS: 
               case SNOOP_PMSISharedCacheState::M:
                 //ctrlAction  = SharedFault;
                 ctrlAction   = StoreData;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS_A;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_IorS:
                 ctrlAction   = StoreData;
                 cacheState_n = SNOOP_PMSISharedCacheState::IorS;
                 break;
               case SNOOP_PMSISharedCacheState::M_d_M:
                 ctrlAction   = Cache2Cache ? StoreDataOnly : StoreData;
                 cacheState_n = SNOOP_PMSISharedCacheState::M;
                 break;
               default:
                 std::cout << "Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           default: // RespNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SharedNullAck;
         }
       } // else if (eventType == SNOOPSharedEventType::RespBus)
       cacheState = static_cast<int>(cacheState_n);
       ownerState = ownerState_n;
    } // PMSI::SharedCacheEventProcessing
*/
}







