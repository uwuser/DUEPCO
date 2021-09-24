/*
 * File  :      MSI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On June 18, 2020
 */

#include "MSI.h"

namespace ns3 { 

    // override ns3 type
    TypeId MSI::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::MSI")
               .SetParent<Object > ();
        return tid;
    }

    MSI::MSI() {
        // default
    }

    // We don't do any dynamic allocations
    MSI::~MSI() {
    }

    void MSI::SNOOPSharedEventProcessing (SNOOPSharedEventType         eventType, 
                                          bool                         Cache2Cache,
                                          SNOOPSharedEventList         eventList, 
                                          int                         &cacheState, 
                                          SNOOPSharedOwnerState       &ownerState,
                                          SNOOPSharedCtrlAction       &ctrlAction) {

       // current and next state variable
       SNOOP_MSISharedCacheState cacheState_c, 
                                 cacheState_n; 
       SNOOPSharedOwnerState     ownerState_n; 

       cacheState_c = static_cast<SNOOP_MSISharedCacheState>(cacheState);

       // default assignments
       ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
       cacheState_n = cacheState_c;
       ownerState_n = SNOOPSharedOwnerState::SkipSet;

       // process ReqBusEvent
       if (eventType == SNOOPSharedEventType::ReqBus) {
         // process ReqBusEvent
         switch (eventList.busReqEvent) {
           case SNOOPSharedReqBusEvent::GetS:{
             //cout<<"It is GetS"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSISharedCacheState::IorS: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MSISharedCacheState::M:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 cacheState_n = SNOOP_MSISharedCacheState::M_d_IorS;
                 break;
               case SNOOP_MSISharedCacheState::M_d_IorS:
               case SNOOP_MSISharedCacheState::M_d_M:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MSISharedCacheState::IorSorM_a:
                 cacheState_n = SNOOP_MSISharedCacheState::IorS;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MSI-Shared::Error, uncovered condition detected0" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;

             }
             break;
           }
           case SNOOPSharedReqBusEvent::GetM:{
             //cout<<"It is GetM"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSISharedCacheState::IorS: 
                 ctrlAction   = SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = SNOOP_MSISharedCacheState::M;
                // cout<<"It is assigned here"<<endl;
                 break;
               case SNOOP_MSISharedCacheState::M:
                 //cout<<"1 - It is assigned here"<<endl;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SaveReqCoreId;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 cacheState_n = Cache2Cache ? SNOOP_MSISharedCacheState::M :
                                              SNOOP_MSISharedCacheState::M_d_M;
                 break;
               case SNOOP_MSISharedCacheState::M_d_IorS:
               case SNOOP_MSISharedCacheState::M_d_M:
                 // cout<<"2 - It is assigned here"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MSISharedCacheState::IorSorM_a:
                  //cout<<"3 - It is assigned here"<<endl;
                 cacheState_n = SNOOP_MSISharedCacheState::M;
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::SharedNoAck : SNOOPSharedCtrlAction::SendData;
                 ownerState_n = SNOOPSharedOwnerState::OtherCore;
                 break;
               case SNOOP_MSISharedCacheState::DRAM_d:
               //cout<<"4 - It is assigned here"<<endl;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:
                 std::cout << "MSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OwnerPutM:{
             //cout<<"It is OwnerPutM"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSISharedCacheState::IorS: 
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MSISharedCacheState::M:
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 cacheState_n = SNOOP_MSISharedCacheState::M_d_IorS;
                 break;
               case SNOOP_MSISharedCacheState::M_d_IorS:
               case SNOOP_MSISharedCacheState::M_d_M:
                 ctrlAction   = SNOOPSharedCtrlAction::SharedStall;
                 break;
               case SNOOP_MSISharedCacheState::IorSorM_a:
                 cacheState_n = SNOOP_MSISharedCacheState::IorS;
                 ctrlAction   = SNOOPSharedCtrlAction::SharedNoAck;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               default:
                 std::cout << "MSI-Shared::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedReqBusEvent::OTherPutM: {
             //cout<<"It is OtherPutM"<<endl;
             switch (cacheState_c) { 
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MSISharedCacheState::IorS: 
               case SNOOP_MSISharedCacheState::M: 
               case SNOOP_MSISharedCacheState::M_d_IorS: 
               case SNOOP_MSISharedCacheState::M_d_M: 
               case SNOOP_MSISharedCacheState::IorSorM_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedNoAck;
                 break;
               default:
                 std::cout << "MSI-Shared::Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MSISharedCacheState::IorS: 
               case SNOOP_MSISharedCacheState::M:
                 ctrlAction   = SNOOPSharedCtrlAction::StoreDataOnly;
                 cacheState_n = SNOOP_MSISharedCacheState::IorSorM_a;
                 break;
               case SNOOP_MSISharedCacheState::M_d_IorS:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_MSISharedCacheState::IorS;
                 break;
               case SNOOP_MSISharedCacheState::M_d_M:
                 ctrlAction   = Cache2Cache ? SNOOPSharedCtrlAction::StoreDataOnly : SNOOPSharedCtrlAction::StoreData;
                 cacheState_n = SNOOP_MSISharedCacheState::M;
                 break;
               default:
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
             }
             break;
           }
           case SNOOPSharedRespBusEvent::DRAMDataResp:{
             if (cacheState_c == SNOOP_MSISharedCacheState::DRAM_d) {
               ctrlAction   = SNOOPSharedCtrlAction::CopyDRAMIntoCache;
               cacheState_n = SNOOP_MSISharedCacheState::IorS;
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
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MSISharedCacheState::DRAM_d;
             }
             break;
           }                
           case SNOOPSharedCtrlEvent::DRAMGetS: {
             switch (cacheState_c) { 
               case SNOOP_MSISharedCacheState::DRAM_d:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedStall;
                 break;
               default:  
                 ctrlAction  = SNOOPSharedCtrlAction::SendVictimCache;
                 cacheState_n = SNOOP_MSISharedCacheState::DRAM_d;
             }
             break;
           }
           case SNOOPSharedCtrlEvent::Replacement: { // apply to victim cache only
             switch (cacheState_c) { 
               case SNOOP_MSISharedCacheState::DRAM_d:
               case SNOOP_MSISharedCacheState::IorSorM_a:
                 ctrlAction  = SNOOPSharedCtrlAction::SharedFault;
                 break;
               case SNOOP_MSISharedCacheState::IorS: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite; 
                 cacheState_n = SNOOP_MSISharedCacheState::IorS;
                 break;
               case SNOOP_MSISharedCacheState::M: 
                 ctrlAction  = SNOOPSharedCtrlAction::IssueCoreInv; 
                 cacheState_n = SNOOP_MSISharedCacheState::M_d_IorS;
                 ownerState_n = SNOOPSharedOwnerState::SharedMem;
                 break;
               case SNOOP_MSISharedCacheState::M_d_M:
               case SNOOP_MSISharedCacheState::M_d_IorS:
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
               case SNOOP_MSISharedCacheState::DRAM_d:
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
    } // MSI::SNOOPSharedEventProcessing


    // MSI Private Cache FSM
    void  MSI::SNOOPPrivEventProcessing 
             (SNOOPPrivEventType  eventType, 
              SNOOPPrivEventList  eventList,
              int                 &cacheState,     
              SNOOPPrivCohTrans   &trans2Issue,
              SNOOPPrivCtrlAction &ctrlAction,
              bool                Cache2Cache) 
    {
       // current and next state variable
       SNOOP_MSIPrivCacheState  cacheState_c, 
                                cacheState_n; 

       // type case MSI State variable
       cacheState_c = static_cast<SNOOP_MSIPrivCacheState>(cacheState);

       // default assignments
       trans2Issue    = NullTrans;
       ctrlAction     = SNOOPPrivCtrlAction::NoAck;
       cacheState_n   = cacheState_c;

       // process CoreEvent
       if (eventType == SNOOPPrivEventType::Core) {
         switch (eventList.cpuReqEvent) {
           case SNOOPPrivCoreEvent::Load:{
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I: 
                 cacheState_n = SNOOP_MSIPrivCacheState::IS_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetSTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::S:
               case SNOOP_MSIPrivCacheState::SM_ad:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::M:
               case SNOOP_MSIPrivCacheState::MI_a:
                 ctrlAction     = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::II_a:  
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI:: SNOOPPrivCoreEvent::Load Error, uncovered condition detected" << std::endl;
                 std::cout << "Curr State = " << PrivStateName(cacheState) << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Store:{
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::S:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_ad;
                 trans2Issue  = SNOOPPrivCohTrans::GetMTrans;
                 ctrlAction   = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::M:
               case SNOOP_MSIPrivCacheState::MI_a:
                 ctrlAction  = SNOOPPrivCtrlAction::Hit;
                 break;
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::SM_ad:
               case SNOOP_MSIPrivCacheState::SM_d:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::II_a: 
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a: 
                 ctrlAction  = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI:: SNOOPPrivCoreEvent::Store Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivCoreEvent::Replacement:{
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MSIPrivCacheState::S: // self invalidate
                 //cout<<"self invalidate"<<endl; 
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 break;
               case SNOOP_MSIPrivCacheState::M:
                 cacheState_n   = SNOOP_MSIPrivCacheState::MI_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::SM_ad:
               case SNOOP_MSIPrivCacheState::SM_d:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::MI_a:
               case SNOOP_MSIPrivCacheState::II_a: 
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 ctrlAction   = SNOOPPrivCtrlAction::Stall;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI:: SNOOPPrivCoreEvent::Replc Error, uncovered condition detected" << std::endl;
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
         //cout<<"req Bus"<<endl;
         switch (eventList.busReqEvent) {
           case SNOOPPrivReqBusEvent::OwnGetS:{
             //cout<<"req Bus 0"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::IS_ad: 
                 cacheState_n = SNOOP_MSIPrivCacheState::IS_d;
                 break;  
               case SNOOP_MSIPrivCacheState::IS_a: 
                 cacheState_n = SNOOP_MSIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           } 
           case SNOOPPrivReqBusEvent::OwnGetM:{
             //cout<<"req Bus 1"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::IM_ad: 
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_d;
                 break;  
               case SNOOP_MSIPrivCacheState::IM_a: 
                 cacheState_n = SNOOP_MSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break; 
               case SNOOP_MSIPrivCacheState::SM_ad: 
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_d;
                 break;  
                case SNOOP_MSIPrivCacheState::SM_a: 
                 cacheState_n = SNOOP_MSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;  
               default:     
                 ctrlAction  = SNOOPPrivCtrlAction::Fault;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnPutM:{
             //cout<<"req Bus 2"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IM_ad:
                 break;
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::S:
               case SNOOP_MSIPrivCacheState::SM_ad:
               case SNOOP_MSIPrivCacheState::SM_d:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::M:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MSIPrivCacheState::I_M_d_I:
                 break;
               case SNOOP_MSIPrivCacheState::III_a:
               case SNOOP_MSIPrivCacheState::MI_a:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::II_a:  
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetS:{
             //cout<<"req Bus 3"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::S:
               case SNOOP_MSIPrivCacheState::SM_ad:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::II_a:  
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 break;
               case SNOOP_MSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_d_S;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_d_S;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MSIPrivCacheState::M:
                 cacheState_n   = SNOOP_MSIPrivCacheState::S;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MSIPrivCacheState::II_a;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreMem : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherGetM:{
             //cout<<"req Bus 4"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::II_a: 
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 break;
               case SNOOP_MSIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_d_SI;
                 break;
               case SNOOP_MSIPrivCacheState::S: 
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 break;
               case SNOOP_MSIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_d_I;
                 ctrlAction   = SNOOPPrivCtrlAction::SaveWbCoreId;
                 break;
               case SNOOP_MSIPrivCacheState::SM_a:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_a;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_d_SI;
                 break;
               case SNOOP_MSIPrivCacheState::M:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreOnly : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MSIPrivCacheState::II_a;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreOnly : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OtherPutM:{
             //cout<<"req Bus 5"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::II_a:  
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::IS_d:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::SM_ad:
               case SNOOP_MSIPrivCacheState::M:
               case SNOOP_MSIPrivCacheState::S:
               case SNOOP_MSIPrivCacheState::SM_d:
               case SNOOP_MSIPrivCacheState::MI_a:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_S:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_S:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 break;
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::SM_a:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
             }
             break;
           }
           case SNOOPPrivReqBusEvent::OwnInvTrans:{
             //cout<<"req Bus 6"<<endl;
             switch (cacheState_c) {
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_ad:
               case SNOOP_MSIPrivCacheState::IS_d_I:
               case SNOOP_MSIPrivCacheState::IM_ad:
               case SNOOP_MSIPrivCacheState::IM_d_I:
               case SNOOP_MSIPrivCacheState::IM_d_SI:
               case SNOOP_MSIPrivCacheState::SM_d_I:
               case SNOOP_MSIPrivCacheState::SM_d_SI:
               case SNOOP_MSIPrivCacheState::II_a:  
                 break;
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::I_M_d_I:
               case SNOOP_MSIPrivCacheState::III_a:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MSIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::IS_d_I;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d:
               case SNOOP_MSIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::I_M_d_I;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_d_SI;
                 break;
               case SNOOP_MSIPrivCacheState::S: 
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 break;
               case SNOOP_MSIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_ad;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_d_SI;
                 break;
               case SNOOP_MSIPrivCacheState::M:
                 cacheState_n = SNOOP_MSIPrivCacheState::III_a;
                 trans2Issue    = SNOOPPrivCohTrans::PutMTrans;
                 ctrlAction     = SNOOPPrivCtrlAction::issueTrans;
                 break;
               case SNOOP_MSIPrivCacheState::MI_a:
                 cacheState_n   = SNOOP_MSIPrivCacheState::II_a;
                 ctrlAction     = Cache2Cache ? SNOOPPrivCtrlAction::SendCoreOnly : SNOOPPrivCtrlAction::SendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
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
               case SNOOP_MSIPrivCacheState::I:
               case SNOOP_MSIPrivCacheState::IS_a:
               case SNOOP_MSIPrivCacheState::IM_a:
               case SNOOP_MSIPrivCacheState::S:
               case SNOOP_MSIPrivCacheState::SM_a:
               case SNOOP_MSIPrivCacheState::M:
               case SNOOP_MSIPrivCacheState::MI_a:
               case SNOOP_MSIPrivCacheState::II_a:
               case SNOOP_MSIPrivCacheState::III_a:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 break;
               case SNOOP_MSIPrivCacheState::IS_ad:
                 cacheState_n = SNOOP_MSIPrivCacheState::IS_a;
                 break;
               case SNOOP_MSIPrivCacheState::IS_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::S;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MSIPrivCacheState::IS_d_I:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MSIPrivCacheState::IM_ad:
                 cacheState_n = SNOOP_MSIPrivCacheState::IM_a;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MSIPrivCacheState::I_M_d_I:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d_I:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::IM_d_SI:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::SM_ad:
                 cacheState_n = SNOOP_MSIPrivCacheState::SM_a;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d:
                 cacheState_n = SNOOP_MSIPrivCacheState::M;
                 ctrlAction   = SNOOPPrivCtrlAction::CopyThenHit;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d_I:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d_S:
                 cacheState_n = SNOOP_MSIPrivCacheState::S;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               case SNOOP_MSIPrivCacheState::SM_d_SI:
                 cacheState_n = SNOOP_MSIPrivCacheState::I;
                 ctrlAction   = Cache2Cache ? SNOOPPrivCtrlAction::CopyThenHitSendCoreMem : SNOOPPrivCtrlAction::CopyThenHitSendMemOnly;
                 break;
               default:
                 ctrlAction   = SNOOPPrivCtrlAction::Fault;
                 std::cout << "MSI::Error, uncovered condition detected" << std::endl;
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

    SNOOPPrivEventPriority MSI::PrivCacheEventPriority (SNOOPPrivEventList eventList,SNOOPPrivEventsCacheInfo eventCacheInfoList ) {
     // cout<<"in SNOOPPrivEventPriority MSI::PrivCacheEventPriority"<<endl;
       SNOOPPrivEventPriority eventPriority = SNOOPPrivEventPriority::WorkConserv;
       SNOOP_MSIPrivCacheState cacheState_c;
       cacheState_c = static_cast<SNOOP_MSIPrivCacheState>(eventCacheInfoList.cpuReqCacheLineInfo.state);
       if (eventList.busRespEvent == SNOOPPrivRespBusEvent::OwnDataResp) {
         //cout<<"in SNOOPPrivEventPriority MSI::PrivCacheEventPriority Bus RESP Event"<<endl;
         eventPriority = SNOOPPrivEventPriority::RespBus;
       }
       else if (eventList.cpuReqEvent != SNOOPPrivCoreEvent::Null && 
                eventList.busReqEvent != SNOOPPrivReqBusEvent::Null ) {
                 // cout<<"in SNOOPPrivEventPriority MSI::PrivCacheEventPriority  BUS AND REQ event are not null"<<endl;
         if (eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnGetS || 
             eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnGetM ||
             eventList.busReqEvent == SNOOPPrivReqBusEvent::OwnPutM) {
              // cout<<"in SNOOPPrivEventPriority MSI::PrivCacheEventPriority REQ BUS"<<endl;
             eventPriority = SNOOPPrivEventPriority::ReqBus;
         }
         else if (eventCacheInfoList.cpuReqCacheLineInfo.cl_idx == eventCacheInfoList.busReqCacheLineInfo.cl_idx) {
           if (eventCacheInfoList.cpuReqCacheLineInfo.IsValid == true && cacheState_c == SNOOP_MSIPrivCacheState::S) {
            // cout<<"in SNOOPPrivEventPriority MSI::PrivCacheEventPriority 6666"<<endl;
             eventPriority = SNOOPPrivEventPriority::ReqBus;
           }
         }
       }
       return eventPriority;
    }

     bool MSI::IsValidBlk (int s) {
       SNOOP_MSIPrivCacheState state = static_cast<SNOOP_MSIPrivCacheState>(s);
       switch (state) {
         case SNOOP_MSIPrivCacheState::S:
         case SNOOP_MSIPrivCacheState::SM_ad:
         case SNOOP_MSIPrivCacheState::SM_d:
         case SNOOP_MSIPrivCacheState::SM_a:
         case SNOOP_MSIPrivCacheState::SM_d_I:
         case SNOOP_MSIPrivCacheState::SM_d_S:
         case SNOOP_MSIPrivCacheState::SM_d_SI:
         case SNOOP_MSIPrivCacheState::M:
         case SNOOP_MSIPrivCacheState::MI_a:
           return true;
         default:
           return false;
       }
    }

     std::string MSI::PrivStateName (int s)
     {

       SNOOP_MSIPrivCacheState state = static_cast<SNOOP_MSIPrivCacheState>(s);

       switch (state) {
         case SNOOP_MSIPrivCacheState::I:
           return " I "; 
         case SNOOP_MSIPrivCacheState::IS_ad:
           return " I_S_ad (waiting for GET(S) command to be issued on bus) ";
         case SNOOP_MSIPrivCacheState::IS_a:
           return " I_S_a (waiting for GET(S) command to be issued on bus, data has been received) ";
         case SNOOP_MSIPrivCacheState::IS_d:
           return " I_S_d (waiting data resp) ";
         case SNOOP_MSIPrivCacheState::IS_d_I:
           return " IS_d_I (waiting for data, then do Hit, then move to I)";
         case SNOOP_MSIPrivCacheState::IM_ad:
           return " IM_ad (waiting for GET(M) command to be issued on bus) ";
         case SNOOP_MSIPrivCacheState::IM_a:
           return " IM_a (waiting for GET(M) command to be issued on bus, data has been received) ";
         case SNOOP_MSIPrivCacheState::IM_d:
           return " IM_d (waiting data resp) ";
         case SNOOP_MSIPrivCacheState::IM_d_I:
           return " IM_d_I (waiting data resp), Then Hit, then move to I ";
         case SNOOP_MSIPrivCacheState::IM_d_S:
           return " IM_d_S (waiting data resp), Then Hit, then move to S";
         case SNOOP_MSIPrivCacheState::IM_d_SI:
           return " IM_d_SI (waiting data resp), Then Hit, then move to I";
         case SNOOP_MSIPrivCacheState::S:
           return " S ";
         case SNOOP_MSIPrivCacheState::SM_ad:
           return " SM_ad (Waiting for GET(M) upgrade to be issued on the bus) " ;
         case SNOOP_MSIPrivCacheState::SM_d:
           return " SM_d (Waiting for Data Response) " ;
         case SNOOP_MSIPrivCacheState::SM_a:
           return " SM_a (Waiting for GET(M), data has been received) " ;
         case SNOOP_MSIPrivCacheState::SM_d_I:
           return " SM_d_I (Waiting for Data Response, Then do Hit, then move to I) " ;
         case SNOOP_MSIPrivCacheState::SM_d_S:
           return " SM_d_S (Waiting for Data Response, Then do Hit, then move to S) " ;
         case SNOOP_MSIPrivCacheState::SM_d_SI:
           return " SM_d_SI (Waiting for Data Response, Then do Hit, then move to I) " ;
         case SNOOP_MSIPrivCacheState::M:
           return " M ";
         case SNOOP_MSIPrivCacheState::MI_a:
           return " MI_a (Waiting for Put(M) then move to I)";
         case SNOOP_MSIPrivCacheState::II_a:
           return " II_a (Waiting for Put(M) then move to I)";
         case SNOOP_MSIPrivCacheState::I_M_d_I:
           return " I_M_d_I (Waiting for data then send memory move to I)";
         case SNOOP_MSIPrivCacheState::III_a:
           return " III_a (Waiting for Put(M) then send data then move to I)";
         default: 
           return " MSI: Error Undefine State";
       }
     }
     
     std::string MSI::SharedStateName (int s)
     {
       SNOOP_MSISharedCacheState state = static_cast<SNOOP_MSISharedCacheState>(s);
       switch (state) {
         case SNOOP_MSISharedCacheState::IorS:
           return " IorS (Shared Mem is the Owner) "; 
         case SNOOP_MSISharedCacheState::M:
           return " M (Shared Mem content is dirty) ";
         case SNOOP_MSISharedCacheState::M_d_IorS:
           return " M_d_IorS (Waiting for Data then Move to IorS) ";
         case SNOOP_MSISharedCacheState::M_d_M:
           return " M_d_M (Waiting for Data then Move to M) ";
         case SNOOP_MSISharedCacheState::IorSorM_a:
           return " IorSorM_a (Waiting for Data then Move to M or IorS) ";
         case SNOOP_MSISharedCacheState::DRAM_d:
           return " DRAM_d (Waiting for DRAM Memory response) ";
         default:
           return " MSI: Error Undefine State";
       }
     }

}

