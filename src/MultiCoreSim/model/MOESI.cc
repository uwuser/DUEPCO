/*
 * File  :      MOESI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 5, 2020
 */

#include "MOESI.h"

namespace ns3 { 

    // override ns3 type
    TypeId MOESI::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::MOESI")
               .SetParent<Object > ();
        return tid;
    }

    MOESI::MOESI() {
        // default
    }

    // We don't do any dynamic allocations
    MOESI::~MOESI() {
    }

    void MOESI::SNOOPSharedEventProcessing (SNOOPSharedEventType       eventType, 
                                          bool                         Cache2Cache,
                                          SNOOPSharedEventList         eventList, 
                                          int                         &cacheState, 
                                          SNOOPSharedOwnerState       &ownerState,
                                          SNOOPSharedCtrlAction       &ctrlAction) {

       // current and next state variable
       SNOOP_MOESISharedCacheState cacheState_c, 
                                 cacheState_n; 
       SNOOPSharedOwnerState     ownerState_n; 

       cacheState_c = static_cast<SNOOP_MOESISharedCacheState>(cacheState);

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
               case SNOOP_MOESISharedCacheState::I: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendDataExclusive;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM;
                 break;
               case SNOOP_MOESISharedCacheState::S: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 break;
               case SNOOP_MOESISharedCacheState::EorM:
                 cacheState_n = SNOOP_MOESISharedCacheState::O;
                 break;
               case SNOOP_MOESISharedCacheState::O:
                 break;
               case SNOOP_MOESISharedCacheState::O_d_S:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::SendExclusiveRespStall;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_a:
               case SNOOP_MOESISharedCacheState::O_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MOESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::GetM:{
             switch (cacheState_c) {
               case SNOOP_MOESISharedCacheState::I: 
               case SNOOP_MOESISharedCacheState::S: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM;
                 break;
               case SNOOP_MOESISharedCacheState::EorM:
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 break;
               case SNOOP_MOESISharedCacheState::O:
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::SendExclusiveRespStall;
                 break;
               case SNOOP_MOESISharedCacheState::O_d_S:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_a:
               case SNOOP_MOESISharedCacheState::O_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MOESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OwnerPutM:{
             switch (cacheState_c) {
               case SNOOP_MOESISharedCacheState::I: 
               case SNOOP_MOESISharedCacheState::S: 
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::EorM:
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM_d_I;
                 break;
               case SNOOP_MOESISharedCacheState::O:
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 cacheState_n = SNOOP_MOESISharedCacheState::O_d_S;
                 break;
               case SNOOP_MOESISharedCacheState::O_d_S:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_a:
                 cacheState_n = SNOOP_MOESISharedCacheState::I;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MOESISharedCacheState::O_a:
                 cacheState_n = SNOOP_MOESISharedCacheState::S;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               default:
                 std::cout << "MOESI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OTherPutM: {
             switch (cacheState_c) { 
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::I: 
               case SNOOP_MOESISharedCacheState::S: 
               case SNOOP_MOESISharedCacheState::EorM: 
               case SNOOP_MOESISharedCacheState::O: 
               case SNOOP_MOESISharedCacheState::O_d_S: 
               case SNOOP_MOESISharedCacheState::EorM_d_I:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedNoAck;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_a:
               case SNOOP_MOESISharedCacheState::O_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               default:
                 std::cout << "MOESI-Shared::Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MOESISharedCacheState::I: 
               case SNOOP_MOESISharedCacheState::S:
               case SNOOP_MOESISharedCacheState::EorM_a: 
               case SNOOP_MOESISharedCacheState::O_a: 
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::EorM:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM_a;
                 break;
               case SNOOP_MOESISharedCacheState::O:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MOESISharedCacheState::O_a;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_d_I:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MOESISharedCacheState::I;
                 break;
               case SNOOP_MOESISharedCacheState::O_d_S:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MOESISharedCacheState::S;
                 break;
               default:
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPSharedRespBusEvent::DRAMDataResp:{
             if (cacheState_c == SNOOP_MOESISharedCacheState::DRAM_d) {
               ctrlAction   = SNOOPSharedCtrlAction::CopyDRAMIntoCache;
               cacheState_n = SNOOP_MOESISharedCacheState::I;
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
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MOESISharedCacheState::DRAM_d;
             }
             break;
           }                
           case SNOOPSharedCtrlEvent::DRAMGetS: {
             switch (cacheState_c) { 
               case SNOOP_MOESISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MOESISharedCacheState::DRAM_d;
             }
             break;
           }
           case SNOOPSharedCtrlEvent::Replacement: { 
             switch (cacheState_c) { 
               case SNOOP_MOESISharedCacheState::DRAM_d:
               case SNOOP_MOESISharedCacheState::EorM_a:
               case SNOOP_MOESISharedCacheState::O_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MOESISharedCacheState::I: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueDRAMWrite; 
                 break;
               case SNOOP_MOESISharedCacheState::S: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite; 
                 cacheState_n = SNOOP_MOESISharedCacheState::I;
                 break;
               case SNOOP_MOESISharedCacheState::EorM: 
                 ctrlAction   = SNOOPSharedCtrlAction::IssueCoreInv; 
                 cacheState_n = SNOOP_MOESISharedCacheState::EorM_d_I;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MOESISharedCacheState::O: 
                 ctrlAction   = SNOOPSharedCtrlAction::IssueCoreInv; 
                 cacheState_n = SNOOP_MOESISharedCacheState::O_d_S;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MOESISharedCacheState::EorM_d_I:
               case SNOOP_MOESISharedCacheState::O_d_S: 
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
               case SNOOP_MOESISharedCacheState::DRAM_d:
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
    } // MOESI::SNOOPSharedEventProcessing


    // MOESI Private Cache FSM
    void  MOESI::SNOOPPrivEventProcessing 
             (SNOOPPrivEventType  eventType, 
              SNOOPPrivEventList  eventList,
              int                 &cacheState,     
              SNOOPPrivCohTrans   &trans2Issue,
              SNOOPPrivCtrlAction &ctrlAction,
              bool                Cache2Cache) 
    {
       // current and next state variable
       SNOOP_MOESIPrivCacheState  cacheState_c, 
                                  cacheState_n; 

       // type case MOESI State variable
       cacheState_c = static_cast<SNOOP_MOESIPrivCacheState>(cacheState);

       // default assignments
       trans2Issue    = NullTrans;
       ctrlAction     = SNOOPPrivCtrlAction::NoAck;
       cacheState_n   = cacheState_c;
       
       if (Cache2Cache == false) {
         std::cout << "MOESI:: SNOOPPrivCoreEvent::Error Cannot Run MOESI Protocol without Cache2Cache Enable" << std::endl;
         ctrlAction   = SNOOPPrivCtrlAction::Fault;
         exit(0);
       }

       // process CoreEvent
       if (eventType == SNOOPPrivEventType::Core) {
         switch (eventList.cpuReqEvent) {
           case SNOOPPrivCoreEvent::Load:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::IS_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetSTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::S:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::EI_a:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::II_a:
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI:: SNOOPPrivCoreEvent::Load Error, uncovered condition detected" << std::endl;
                 std::cout << "Curr State = " << PrivStateName(cacheState) << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Store:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::S:
                 cacheState_n = SNOOP_MOESIPrivCacheState::SM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::E:
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MOESIPrivCacheState::O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::OM_a;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::MI_a:
                 ctrlAction  = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::II_a:  
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI:: SNOOPPrivCoreEvent::Store Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Replacement:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MOESIPrivCacheState::S: // self invalidate
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 break;
               case SNOOP_MOESIPrivCacheState::M:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::MI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::E:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::EI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::O:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::OI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::II_a:  
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI:: SNOOPPrivCoreEvent::Replc Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MOESIPrivCacheState::IS_ad: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::IS_d;
                 break;  
               case SNOOP_MOESIPrivCacheState::IS_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               case SNOOP_MOESIPrivCacheState::IE_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::E;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           } 
           case SNOOPPrivReqBusEvent::OwnGetM:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::IM_ad: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_d;
                 break;  
               case SNOOP_MOESIPrivCacheState::IM_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break; 
               case SNOOP_MOESIPrivCacheState::SM_ad: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::SM_d;
                 break;  
                case SNOOP_MOESIPrivCacheState::OM_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::Hit;
                 break;  
                case SNOOP_MOESIPrivCacheState::OM_a_I: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::HitSendMemOnly;
                 break;  
                case SNOOP_MOESIPrivCacheState::SM_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnPutM:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IM_ad:
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::S:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::OM_a_I: 
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
                 break;
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::EI_a: 
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::II_a: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetS:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::S:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::II_a:  
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IE_d_O;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_d_O;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::SM_d_O;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::M:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::O;
                 ctrlAction     = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::OI_a;
                 ctrlAction     = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
                 ctrlAction     = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetM:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::II_a:  
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IE_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::S: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::SM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_a:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_a;
                 break;
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::M:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::OM_a:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_ad;
                 ctrlAction   = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::II_a;
                 ctrlAction     = SNOOPPrivCtrlAction::SendCoreOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherPutM:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::II_a:  
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::IS_d:
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::SM_ad:
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::S:
               case SNOOP_MOESIPrivCacheState::SM_d:
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
                 break;
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnExclTrans:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IE_d;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnInvTrans:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_ad:
               case SNOOP_MOESIPrivCacheState::IE_d_I:
               case SNOOP_MOESIPrivCacheState::IS_d_I:
               case SNOOP_MOESIPrivCacheState::IM_ad:
               case SNOOP_MOESIPrivCacheState::IM_d_I:
               case SNOOP_MOESIPrivCacheState::SM_d_I:
               case SNOOP_MOESIPrivCacheState::II_a:  
                 break;
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
               case SNOOP_MOESIPrivCacheState::O_d_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MOESIPrivCacheState::OM_a:
                 cacheState_n = SNOOP_MOESIPrivCacheState::OM_a_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d:
               case SNOOP_MOESIPrivCacheState::IM_d:
               case SNOOP_MOESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I_EorM_d_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d_O:
               case SNOOP_MOESIPrivCacheState::IM_d_O:
               case SNOOP_MOESIPrivCacheState::SM_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::O_d_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::S: 
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::EorMorO_a_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
                 cacheState_n   = SNOOP_MOESIPrivCacheState::EorMorO_a_I;
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
               case SNOOP_MOESIPrivCacheState::I:
               case SNOOP_MOESIPrivCacheState::IS_a:
               case SNOOP_MOESIPrivCacheState::IE_a:
               case SNOOP_MOESIPrivCacheState::IM_a:
               case SNOOP_MOESIPrivCacheState::OM_a:
               case SNOOP_MOESIPrivCacheState::S:
               case SNOOP_MOESIPrivCacheState::SM_a:
               case SNOOP_MOESIPrivCacheState::E:
               case SNOOP_MOESIPrivCacheState::O:
               case SNOOP_MOESIPrivCacheState::M:
               case SNOOP_MOESIPrivCacheState::EI_a:
               case SNOOP_MOESIPrivCacheState::OI_a:
               case SNOOP_MOESIPrivCacheState::MI_a:
               case SNOOP_MOESIPrivCacheState::II_a:
               case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
               case SNOOP_MOESIPrivCacheState::OM_a_I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IS_a;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::E;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::O;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::IE_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IM_a;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::IM_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::O;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::SM_a;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::SM_d_O:
                 cacheState_n = SNOOP_MOESIPrivCacheState::O;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MOESIPrivCacheState::O_d_I:
                 cacheState_n = SNOOP_MOESIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendCoreMem;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MOESI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivRespBusEvent::OwnDataRespExclusive:{
             switch (cacheState_c) {
               case SNOOP_MOESIPrivCacheState::IS_ad:
                 cacheState_n = SNOOP_MOESIPrivCacheState::IE_a;
                 break;
               case SNOOP_MOESIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MOESIPrivCacheState::E;
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

    SNOOPPrivEventPriority MOESI::PrivCacheEventPriority (SNOOPPrivEventList eventList,SNOOPPrivEventsCacheInfo eventCacheInfoList ) {
       SNOOPPrivEventPriority eventPriority = SNOOPPrivEventPriority::WorkConserv;
       SNOOP_MOESIPrivCacheState cacheState_c;
       cacheState_c = static_cast<SNOOP_MOESIPrivCacheState>(eventCacheInfoList.cpuReqCacheLineInfo.state);
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
           if (eventCacheInfoList.cpuReqCacheLineInfo.IsValid == true && (cacheState_c == SNOOP_MOESIPrivCacheState::S || 
                                                                          cacheState_c == SNOOP_MOESIPrivCacheState::O)) {
             eventPriority = SNOOPPrivEventPriority::ReqBus;
           }
         }
       }
       return eventPriority;
    }

     bool MOESI::IsValidBlk (int s) {
       SNOOP_MOESIPrivCacheState state = static_cast<SNOOP_MOESIPrivCacheState>(s);
       switch (state) {
         case SNOOP_MOESIPrivCacheState::S:
         case SNOOP_MOESIPrivCacheState::SM_ad:
         case SNOOP_MOESIPrivCacheState::SM_d:
         case SNOOP_MOESIPrivCacheState::SM_a:
         case SNOOP_MOESIPrivCacheState::SM_d_I:
         case SNOOP_MOESIPrivCacheState::SM_d_O:
         case SNOOP_MOESIPrivCacheState::E:
         case SNOOP_MOESIPrivCacheState::O:
         case SNOOP_MOESIPrivCacheState::M:
         case SNOOP_MOESIPrivCacheState::EI_a:
         case SNOOP_MOESIPrivCacheState::OI_a:
         case SNOOP_MOESIPrivCacheState::OM_a:
         case SNOOP_MOESIPrivCacheState::MI_a:
           return true;
         default:
           return false;
       }
    }

     std::string MOESI::PrivStateName (int s)
     {

       SNOOP_MOESIPrivCacheState state = static_cast<SNOOP_MOESIPrivCacheState>(s);

       switch (state) {
         case SNOOP_MOESIPrivCacheState::I:
           return " I "; 
         case SNOOP_MOESIPrivCacheState::IS_ad:
           return " I_S_ad (waiting for GET(S) command to be issued on bus) ";
         case SNOOP_MOESIPrivCacheState::IS_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_MOESIPrivCacheState::IS_d:
           return " I_S_d (waiting data resp) ";
         case SNOOP_MOESIPrivCacheState::IE_d:
           return " IE_d (waiting Exclusive data resp) ";
         case SNOOP_MOESIPrivCacheState::IE_d_O:
           return " IE_d_O (waiting data resp), Then Hit, then move to O";
         case SNOOP_MOESIPrivCacheState::IE_d_I:
           return " IE_d_I (waiting data resp), Then Hit, then move to I";           
         case SNOOP_MOESIPrivCacheState::IS_d_I:
           return " IS_d_I (waiting for data, then do Hit, then move to I)";
         case SNOOP_MOESIPrivCacheState::IE_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_MOESIPrivCacheState::IM_ad:
           return " IM_ad (waiting for GET(M) command to be issued on bus) ";
         case SNOOP_MOESIPrivCacheState::IM_a:
           return " IM_a (waiting for GET(M) command to be issued on bus, data has been received) ";
         case SNOOP_MOESIPrivCacheState::IM_d:
           return " IM_d (waiting data resp) ";
         case SNOOP_MOESIPrivCacheState::IM_d_I:
           return " IM_d_I (waiting data resp), Then Hit, then move to I ";
         case SNOOP_MOESIPrivCacheState::IM_d_O:
           return " IM_d_O (waiting data resp), Then Hit, then move to O";
         case SNOOP_MOESIPrivCacheState::S:
           return " S ";
         case SNOOP_MOESIPrivCacheState::SM_ad:
           return " SM_ad (Waiting for GET(M) upgrade to be issued on the bus) " ;
         case SNOOP_MOESIPrivCacheState::SM_d:
           return " SM_d (Waiting for Data Response) " ;
         case SNOOP_MOESIPrivCacheState::SM_a:
           return " SM_a (Waiting for GET(M), data has been received) " ;
         case SNOOP_MOESIPrivCacheState::SM_d_I:
           return " SM_d_I (Waiting for Data Response, Then do Hit, then move to I) " ;
         case SNOOP_MOESIPrivCacheState::SM_d_O:
           return " SM_d_O (Waiting for Data Response, Then do Hit, then move to O) " ;
         case SNOOP_MOESIPrivCacheState::E:
           return " E ";
         case SNOOP_MOESIPrivCacheState::O:
           return " O ";
         case SNOOP_MOESIPrivCacheState::OM_a:
           return " OM_a ";
         case SNOOP_MOESIPrivCacheState::M:
           return " M ";
         case SNOOP_MOESIPrivCacheState::EI_a:
           return " EI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MOESIPrivCacheState::OI_a:
           return " OI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MOESIPrivCacheState::MI_a:
           return " MI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MOESIPrivCacheState::II_a:
           return " II_a (Waiting for Put(M) then move to I)";
         case SNOOP_MOESIPrivCacheState::EorMorO_a_I:
           return " EorMorO_a_I (Waiting for Put(M) then sendMem and move to I)";
         case SNOOP_MOESIPrivCacheState::I_EorM_d_I:
           return " I_EorM_d_I (Waiting for data then Hit, sendMem then move to I)";
         case SNOOP_MOESIPrivCacheState::O_d_I:
           return " O_d_I (Waiting for data then Hit, sendMem and Core then move to I)";
         case SNOOP_MOESIPrivCacheState::OM_a_I:
           return " OM_a_I (Waiting for Get(M) then Hit, then move to I)";
         default: 
           return " MOESI: Error Undefine State";
       }
     }
     
     std::string MOESI::SharedStateName (int s)
     {
       SNOOP_MOESISharedCacheState state = static_cast<SNOOP_MOESISharedCacheState>(s);
       switch (state) {
         case SNOOP_MOESISharedCacheState::I:
           return " I (Shared Mem is the only Owner) "; 
         case SNOOP_MOESISharedCacheState::S:
           return " S (zero to multiple sharers, clean) "; 
         case SNOOP_MOESISharedCacheState::EorM:
           return " EorM (zero to multiple sharers (dirty copy) ) ";
         case SNOOP_MOESISharedCacheState::O:
           return " O (zero to multiple sharers (dirty copy) ) ";
         case SNOOP_MOESISharedCacheState::EorM_d_I:
           return " EorM_d_I (Waiting for Data then Move to I) ";
         case SNOOP_MOESISharedCacheState::O_d_S:
           return " O_d_S (Waiting for Data then Move to S) ";
         case SNOOP_MOESISharedCacheState::EorM_a:
           return " EorM_a (Waiting for coherence msg then Move to I) ";
         case SNOOP_MOESISharedCacheState::O_a:
           return " O_a (Waiting for coherence msg then Move to S) ";
         case SNOOP_MOESISharedCacheState::DRAM_d:
           return " DRAM_d (Waiting for DRAM Memory response) ";
         default:
           return " MOESI: Error Undefine State (Shared Cache Controller)";
       }
     }
}
