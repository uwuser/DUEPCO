/*
 * File  :      MESI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 5, 2020
 */

#include "MESI.h"

namespace ns3 { 

    // override ns3 type
    TypeId MESI::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::MESI")
               .SetParent<Object > ();
        return tid;
    }

    MESI::MESI() {
        // default
    }

    // We don't do any dynamic allocations
    MESI::~MESI() {
    }
    
    void MESI::SNOOPSharedEventProcessing (SNOOPSharedEventType         eventType, 
                                          bool                         Cache2Cache,
                                          SNOOPSharedEventList         eventList, 
                                          int                         &cacheState, 
                                          SNOOPSharedOwnerState       &ownerState,
                                          SNOOPSharedCtrlAction       &ctrlAction) {

       // current and next state variable
       SNOOP_MESISharedCacheState cacheState_c, 
                                 cacheState_n; 
       SNOOPSharedOwnerState     ownerState_n; 

       cacheState_c = static_cast<SNOOP_MESISharedCacheState>(cacheState);

       // default assignments
       ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
       cacheState_n = cacheState_c;
       ownerState_n = SNOOPSharedOwnerState::SkipSet;
    
       // process ReqBusEvent
       if (eventType == SNOOPSharedEventType::ReqBus) {
         // process ReqBusEvent
         switch (eventList.busReqEvent) {
           case SNOOPSharedReqBusEvent::GetS:{
             //std::cout<<"GetS in MESI"<<endl;
             switch (cacheState_c) {
               case SNOOP_MESISharedCacheState::I: 
                //std:: cout<<"GetS in MESI - I"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SendDataExclusive;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MESISharedCacheState::EorM;
                 break;
               case SNOOP_MESISharedCacheState::S: 
                 //std::cout<<"GetS in MESI - S"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MESISharedCacheState::EorM:
                 //std::cout<<"GetS in MESI - EorM"<<endl;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 cacheState_n = SNOOP_MESISharedCacheState::EorM_d_S;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_I:
                 //std::cout<<"GetS in MESI - EorM_d_I"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SendExclusiveRespStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_S:
                 //std::cout<<"GetS in MESI - EorM_d_S"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
                 //std::cout<<"GetS in MESI - EorM_d_EorM"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_a:
                 //std::cout<<"GetS in MESI - EorM_a"<<endl;
                 cacheState_n = SNOOP_MESISharedCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MESISharedCacheState::DRAM_d:
                 //std::cout<<"GetS in MESI - DRAM"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;

             }
             break;
           }
           case SNOOPSharedReqBusEvent::GetM:{
             //cout<<"GetM in MESI"<<endl;
             switch (cacheState_c) {
               case SNOOP_MESISharedCacheState::I: 
               case SNOOP_MESISharedCacheState::S: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MESISharedCacheState::EorM;
                 break;
               case SNOOP_MESISharedCacheState::EorM:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = Cache2Cache ? SNOOP_MESISharedCacheState::EorM :
                                              SNOOP_MESISharedCacheState::EorM_d_EorM;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_S:
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::SendExclusiveRespStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_a:
                 cacheState_n = SNOOP_MESISharedCacheState::EorM;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 break;
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OwnerPutM:{
             //cout<<"OwnerPutM in MESI"<<endl;
             switch (cacheState_c) {
               case SNOOP_MESISharedCacheState::I: 
               case SNOOP_MESISharedCacheState::S: 
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MESISharedCacheState::EorM:
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 cacheState_n = SNOOP_MESISharedCacheState::EorM_d_I;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_S:
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
               case SNOOP_MESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MESISharedCacheState::EorM_a:
                 cacheState_n = SNOOP_MESISharedCacheState::I;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               default:
                 std::cout << "MESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OTherPutM: {
             //cout<<"OtherPutM in MESI"<<endl;
             switch (cacheState_c) { 
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MESISharedCacheState::I: 
               case SNOOP_MESISharedCacheState::S: 
               case SNOOP_MESISharedCacheState::EorM: 
               case SNOOP_MESISharedCacheState::EorM_d_S:
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
               case SNOOP_MESISharedCacheState::EorM_d_I:
               case SNOOP_MESISharedCacheState::EorM_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedNoAck;
                 break;
               default:
                 std::cout << "MESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           default: // BusNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPSharedCtrlAction::SharedNullAck;
         }
       } // if (eventType == SNOOPSharedEventType::ReqBus) {                  
       else if (eventType == SNOOPSharedEventType::RespBus) {
         switch (eventList.busRespEvent) {
           case SNOOPSharedRespBusEvent::OWnDataResp:{
             ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
             break;
           }
           case SNOOPSharedRespBusEvent::OTherDataResp:{
             switch (cacheState_c) {
               case SNOOP_MESISharedCacheState::I: 
               case SNOOP_MESISharedCacheState::S: 
               case SNOOP_MESISharedCacheState::EorM_a:
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MESISharedCacheState::EorM:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MESISharedCacheState::EorM_a;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_S:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_MESISharedCacheState::S;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreData; 
                 cacheState_n = SNOOP_MESISharedCacheState::EorM;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_I:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_MESISharedCacheState::I;
                 break;
               default:
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedRespBusEvent::DRAMDataResp:{
             if (cacheState_c == SNOOP_MESISharedCacheState::DRAM_d) {
               ctrlAction   = SNOOPSharedCtrlAction::CopyDRAMIntoCache;
               cacheState_n = SNOOP_MESISharedCacheState::I;
               ownerState_n = SNOOPSharedOwnerState::SharedMem;
             }
             else {
               ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           default: // RespNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPSharedCtrlAction::SharedNullAck;
         }
       } // else if (eventType == SNOOPSharedEventType::RespBus)
       else if (eventType == SNOOPSharedEventType::CacheCtrl) {
         switch (eventList.CtrlEvent) {
           case SNOOPSharedCtrlEvent::DRAMGetM: {
             switch (cacheState_c) { 
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MESISharedCacheState::DRAM_d;
             }
             break;
           }                
           case SNOOPSharedCtrlEvent::DRAMGetS: {
             switch (cacheState_c) { 
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MESISharedCacheState::DRAM_d;
             }
             break;
           }
           case SNOOPSharedCtrlEvent::Replacement: { 
             switch (cacheState_c) { 
               case SNOOP_MESISharedCacheState::DRAM_d:
               case SNOOP_MESISharedCacheState::EorM_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MESISharedCacheState::I: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueDRAMWrite; 
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MESISharedCacheState::S: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite; 
                 cacheState_n = SNOOP_MESISharedCacheState::I;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MESISharedCacheState::EorM: 
                 ctrlAction   = SNOOPSharedCtrlAction::IssueCoreInv; 
                 cacheState_n = SNOOP_MESISharedCacheState::EorM_d_I;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MESISharedCacheState::EorM_d_I:
               case SNOOP_MESISharedCacheState::EorM_d_S:
               case SNOOP_MESISharedCacheState::EorM_d_EorM:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedCtrlEvent::VictCacheSwap: {
             switch (cacheState_c) { 
               case SNOOP_MESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SWAPPING;
             }
             break;
           }
           default: // CacheCtrlNullEvent
             cacheState_n = cacheState_c;
             ctrlAction   = SNOOPSharedCtrlAction::SharedNullAck;
         }
       } // else if (eventType == SNOOPSharedEventType::CacheCtrl) {
       
       cacheState = static_cast<int>(cacheState_n);
       ownerState = ownerState_n;
    } // MESI::SNOOPSharedEventProcessing


    // MSEI Private Cache FSM
    void  MESI::SNOOPPrivEventProcessing 
             (SNOOPPrivEventType  eventType, 
              SNOOPPrivEventList  eventList,
              int                 &cacheState,     
              SNOOPPrivCohTrans   &trans2Issue,
              SNOOPPrivCtrlAction &ctrlAction,
              bool                Cache2Cache) 
    {
       // current and next state variable
       SNOOP_MESIPrivCacheState  cacheState_c, 
                                cacheState_n; 

       // type case MESI State variable
       cacheState_c = static_cast<SNOOP_MESIPrivCacheState>(cacheState);

       // default assignments
       trans2Issue    = NullTrans;
       ctrlAction     = SNOOPPrivCtrlAction::NoAck;
       cacheState_n   = cacheState_c;

       // process CoreEvent
       if (eventType == SNOOPPrivEventType::Core) {
         switch (eventList.cpuReqEvent) {
           case SNOOPPrivCoreEvent::Load:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I: 
                 cacheState_n = SNOOP_MESIPrivCacheState::IS_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetSTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::S:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::SM_d:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
               case SNOOP_MESIPrivCacheState::MI_a:
               case SNOOP_MESIPrivCacheState::EI_a:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::II_a:  
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI:: SNOOPPrivCoreEvent::Load Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Store:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::S:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::E:
                 cacheState_n = SNOOP_MESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MESIPrivCacheState::M:
               case SNOOP_MESIPrivCacheState::MI_a:
                 ctrlAction  = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::SM_d:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::II_a:  
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI:: SNOOPPrivCoreEvent::Store Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Replacement:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MESIPrivCacheState::S: // self invalidate
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 break;
               case SNOOP_MESIPrivCacheState::M:
                 cacheState_n   = SNOOP_MESIPrivCacheState::MI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::E:
                 cacheState_n   = SNOOP_MESIPrivCacheState::EI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::SM_d:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
               case SNOOP_MESIPrivCacheState::II_a:
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I: 
                 ctrlAction   = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI:: SNOOPPrivCoreEvent::Replc Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MESIPrivCacheState::IS_ad: 
                 cacheState_n = SNOOP_MESIPrivCacheState::IS_d;
                 break;  
               case SNOOP_MESIPrivCacheState::IS_a: 
                 cacheState_n = SNOOP_MESIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               case SNOOP_MESIPrivCacheState::IE_a: 
                 cacheState_n = SNOOP_MESIPrivCacheState::E;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           } 
           case SNOOPPrivReqBusEvent::OwnGetM:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::IM_ad: 
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_d;
                 break;  
               case SNOOP_MESIPrivCacheState::IM_a: 
                 cacheState_n = SNOOP_MESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break; 
               case SNOOP_MESIPrivCacheState::SM_ad: 
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_d;
                 break;  
                case SNOOP_MESIPrivCacheState::SM_a: 
                 cacheState_n = SNOOP_MESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnPutM:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IM_ad:
                 break;
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::S:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::SM_d:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::M:
               case SNOOP_MESIPrivCacheState::E:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 break;
               case SNOOP_MESIPrivCacheState::MI_a:
               case SNOOP_MESIPrivCacheState::EI_a: 
               case SNOOP_MESIPrivCacheState::EorM_a_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::II_a: 
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetS:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::S:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::II_a:  
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 break;
               case SNOOP_MESIPrivCacheState::IE_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_d_S;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_d_S;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_d_S;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
                 cacheState_n   = SNOOP_MESIPrivCacheState::S;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MESIPrivCacheState::II_a;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetM:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::II_a:  
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 break;
               case SNOOP_MESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_d_SI;
                 break;  
               case SNOOP_MESIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_d_SI;
                 break;
               case SNOOP_MESIPrivCacheState::S: 
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 break;
               case SNOOP_MESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MESIPrivCacheState::SM_a:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_a;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_d_SI;
                 break;
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreOnly : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MESIPrivCacheState::II_a;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreOnly : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherPutM:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::II_a:  
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::IS_d:
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IE_d_S:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::SM_ad:
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
               case SNOOP_MESIPrivCacheState::S:
               case SNOOP_MESIPrivCacheState::SM_d:
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_S:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_S:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
                 break;
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnExclTrans:{
             //std::cout<<"in OwnExclTrans"<<endl;
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::IS_d:
               //std::cout<<"in OwnExclTrans: IS_d"<<endl;
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_d;
                 break;
              //  case SNOOP_MESIPrivCacheState::IS_d_I:
              //  std::cout<<"in OwnExclTrans: IS_d_I"<<endl;
              //    cacheState_n = SNOOP_MESIPrivCacheState::IS_d_I;
              //    break;  
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnInvTrans:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_ad:
               case SNOOP_MESIPrivCacheState::IE_d_SI:
               case SNOOP_MESIPrivCacheState::IE_d_I:
               case SNOOP_MESIPrivCacheState::IS_d_I:
               case SNOOP_MESIPrivCacheState::IM_ad:
               case SNOOP_MESIPrivCacheState::IM_d_I:
               case SNOOP_MESIPrivCacheState::IM_d_SI:
               case SNOOP_MESIPrivCacheState::SM_d_I:
               case SNOOP_MESIPrivCacheState::SM_d_SI:
               case SNOOP_MESIPrivCacheState::II_a:  
                 break;
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::EorM_a_I:
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d:
               case SNOOP_MESIPrivCacheState::IM_d:
               case SNOOP_MESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::I_EorM_d_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_d_SI;
                 break;  
               case SNOOP_MESIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_d_SI;
                 break;
               case SNOOP_MESIPrivCacheState::S: 
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 break;
               case SNOOP_MESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_d_SI;
                 break;
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
                 cacheState_n = SNOOP_MESIPrivCacheState::EorM_a_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MESIPrivCacheState::EorM_a_I;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MESIPrivCacheState::I:
               case SNOOP_MESIPrivCacheState::IS_a:
               case SNOOP_MESIPrivCacheState::IE_a:
               case SNOOP_MESIPrivCacheState::IM_a:
               case SNOOP_MESIPrivCacheState::S:
               case SNOOP_MESIPrivCacheState::SM_a:
               case SNOOP_MESIPrivCacheState::E:
               case SNOOP_MESIPrivCacheState::M:
               case SNOOP_MESIPrivCacheState::EI_a:
               case SNOOP_MESIPrivCacheState::MI_a:
               case SNOOP_MESIPrivCacheState::II_a:
               case SNOOP_MESIPrivCacheState::EorM_a_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MESIPrivCacheState::I_EorM_d_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IS_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::IS_a;
                 break;
               case SNOOP_MESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::E;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d_SI:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IE_d_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IS_d_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MESIPrivCacheState::IM_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::IM_a;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::IM_d_SI:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::SM_a;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d_I:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MESIPrivCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MESIPrivCacheState::SM_d_SI:
                 cacheState_n = SNOOP_MESIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivRespBusEvent::OwnDataRespExclusive:{
             switch (cacheState_c) {
               case SNOOP_MESIPrivCacheState::IS_ad:
                 cacheState_n = SNOOP_MESIPrivCacheState::IE_a;
                 break;
               case SNOOP_MESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MESIPrivCacheState::E;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
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

    SNOOPPrivEventPriority MESI::PrivCacheEventPriority (SNOOPPrivEventList eventList,SNOOPPrivEventsCacheInfo eventCacheInfoList ) {
       SNOOPPrivEventPriority eventPriority = SNOOPPrivEventPriority::WorkConserv;
       SNOOP_MESIPrivCacheState cacheState_c;
       cacheState_c = static_cast<SNOOP_MESIPrivCacheState>(eventCacheInfoList.cpuReqCacheLineInfo.state);
       if (eventList.busRespEvent == SNOOPPrivRespBusEvent::OwnDataResp) {
         eventPriority = SNOOPPrivEventPriority::RespBus;
       }
       else if (eventList.cpuReqEvent != SNOOPPrivCoreEvent::Null && 
                eventList.busReqEvent != SNOOPPrivReqBusEvent::Null ) {
         if (eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnGetS || 
             eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnGetM ||
             eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnPutM) {
             eventPriority = SNOOPPrivEventPriority::ReqBus;
         }
         else if (eventCacheInfoList.cpuReqCacheLineInfo.cl_idx == eventCacheInfoList.busReqCacheLineInfo.cl_idx) {
           if (eventCacheInfoList.cpuReqCacheLineInfo.IsValid == true && (cacheState_c == SNOOP_MESIPrivCacheState::S || 
                                                                          cacheState_c == SNOOP_MESIPrivCacheState::E)) {
             eventPriority = SNOOPPrivEventPriority::ReqBus;
           }
         }
       }
       return eventPriority;
    }

     bool MESI::IsValidBlk (int s) {
       SNOOP_MESIPrivCacheState state = static_cast<SNOOP_MESIPrivCacheState>(s);
       switch (state) {
         case SNOOP_MESIPrivCacheState::S:
         case SNOOP_MESIPrivCacheState::SM_ad:
         case SNOOP_MESIPrivCacheState::SM_d:
         case SNOOP_MESIPrivCacheState::SM_a:
         case SNOOP_MESIPrivCacheState::SM_d_I:
         case SNOOP_MESIPrivCacheState::SM_d_S:
         case SNOOP_MESIPrivCacheState::SM_d_SI:
         case SNOOP_MESIPrivCacheState::E:
         case SNOOP_MESIPrivCacheState::M:
         case SNOOP_MESIPrivCacheState::EI_a:
         case SNOOP_MESIPrivCacheState::MI_a:
           return true;
         default:
           return false;
       }
    }
    
    int MESI::SNOOPSharedCacheWaitDRAMState () {
      return static_cast<int>(SNOOP_MESISharedCacheState::DRAM_d);
    }
    

     std::string MESI::PrivStateName (int s)
     {

       SNOOP_MESIPrivCacheState state = static_cast<SNOOP_MESIPrivCacheState>(s);

       switch (state) {
         case SNOOP_MESIPrivCacheState::I:
           return " I "; 
         case SNOOP_MESIPrivCacheState::IS_ad:
           return " I_S_ad (waiting for GET(S) command to be issued on bus) ";
         case SNOOP_MESIPrivCacheState::IS_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_MESIPrivCacheState::IS_d:
           return " I_S_d (waiting data resp) ";
         case SNOOP_MESIPrivCacheState::IE_d:
           return " IE_d (waiting Exclusive data resp) ";
         case SNOOP_MESIPrivCacheState::IE_d_S:
           return " IE_d_S (waiting data resp), Then Hit, then move to S";
         case SNOOP_MESIPrivCacheState::IE_d_SI:
           return " IE_d_SI (waiting data resp), Then Hit, then move to I";
         case SNOOP_MESIPrivCacheState::IE_d_I:
           return " IE_d_I (waiting data resp), Then Hit, then move to I";           
         case SNOOP_MESIPrivCacheState::IS_d_I:
           return " IS_d_I (waiting for data, then do Hit, then move to I)";
         case SNOOP_MESIPrivCacheState::IE_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_MESIPrivCacheState::IM_ad:
           return " IM_ad (waiting for GET(M) command to be issued on bus) ";
         case SNOOP_MESIPrivCacheState::IM_a:
           return " IM_a (waiting for GET(M) command to be issued on bus, data has been received) ";
         case SNOOP_MESIPrivCacheState::IM_d:
           return " IM_d (waiting data resp) ";
         case SNOOP_MESIPrivCacheState::IM_d_I:
           return " IM_d_I (waiting data resp), Then Hit, then move to I ";
         case SNOOP_MESIPrivCacheState::IM_d_S:
           return " IM_d_S (waiting data resp), Then Hit, then move to S";
         case SNOOP_MESIPrivCacheState::IM_d_SI:
           return " IM_d_SI (waiting data resp), Then Hit, then move to I";
         case SNOOP_MESIPrivCacheState::S:
           return " S ";
         case SNOOP_MESIPrivCacheState::SM_ad:
           return " SM_ad (Waiting for GET(M) upgrade to be issued on the bus) " ;
         case SNOOP_MESIPrivCacheState::SM_d:
           return " SM_d (Waiting for Data Response) " ;
         case SNOOP_MESIPrivCacheState::SM_a:
           return " SM_a (Waiting for GET(M), data has been received) " ;
         case SNOOP_MESIPrivCacheState::SM_d_I:
           return " SM_d_I (Waiting for Data Response, Then do Hit, then move to I) " ;
         case SNOOP_MESIPrivCacheState::SM_d_S:
           return " SM_d_S (Waiting for Data Response, Then do Hit, then move to S) " ;
         case SNOOP_MESIPrivCacheState::SM_d_SI:
           return " SM_d_SI (Waiting for Data Response, Then do Hit, then move to I) " ;
         case SNOOP_MESIPrivCacheState::E:
           return " E ";
         case SNOOP_MESIPrivCacheState::M:
           return " M ";
         case SNOOP_MESIPrivCacheState::EI_a:
           return " EI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MESIPrivCacheState::MI_a:
           return " MI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MESIPrivCacheState::II_a:
           return " II_a (Waiting for Put(M) then move to I)";
         case SNOOP_MESIPrivCacheState::EorM_a_I:
           return " EorM_a_I (Waiting for Put(M) then SendMem and move to I)";
         case SNOOP_MESIPrivCacheState::I_EorM_d_I:
           return " I_EorM_d_I (Waiting for data then hit, SendMem, and move to I)";
         default: 
           return " MESI: Error Undefine State";
       }
     }
     
     std::string MESI::SharedStateName (int s)
     {
       SNOOP_MESISharedCacheState state = static_cast<SNOOP_MESISharedCacheState>(s);
       //std::cout << "CohProtType::SNOOP_MESI " << CohProtType::SNOOP_MESI << " = " << s << std::endl;
       switch (state) {
         case SNOOP_MESISharedCacheState::I:
           return " I (Shared Mem is the only Owner) "; 
         case SNOOP_MESISharedCacheState::S:
           return " S (zero to multiple sharers (clean copy) ) ";
         case SNOOP_MESISharedCacheState::EorM:
           return " EorM (zero to multiple sharers (dirty copy) ) ";
         case SNOOP_MESISharedCacheState::EorM_d_S:
           return " EorM_d_S (Waiting for Data then Move to S) ";
         case SNOOP_MESISharedCacheState::EorM_d_EorM:
           return " EorM_d_EorM (Waiting for Data then Move to M) ";
         case SNOOP_MESISharedCacheState::EorM_d_I:
           return " EorM_d_I (Waiting for Data then Move to I) ";
         case SNOOP_MESISharedCacheState::EorM_a:
           return " EorM_a (Waiting for coherence msg then Move to M, I or S) ";
         case SNOOP_MESISharedCacheState::DRAM_d:
           return " DRAM_d (Waiting for DRAM Memory response) ";
         default:
           return " MESI: Error Undefine State (Shared Cache Controller)";
       }
     }

     bool MESI::ByPassMESI (int s)
     {
       SNOOP_MESISharedCacheState state = static_cast<SNOOP_MESISharedCacheState>(s);
       //std::cout << "CohProtType::SNOOP_MESI " << CohProtType::SNOOP_MESI << " = " << s << std::endl;
       switch (state) {
         case SNOOP_MESISharedCacheState::EorM_a:
           return true;
         default:
           return false;
       }
     }
               
}

