/*
 * File  :      SNOOPProtocolCommon.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On May 29, 2020
 */


#ifndef _SNOOPProtocolCommon_H
#define _SNOOPProtocolCommon_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "MemTemplate.h"
#include "GenericCache.h"

namespace ns3 { 
  enum CohProtType {
    SNOOP_PMSI  = 0x000, 
    SNOOP_MSI   = 0x100,
    SNOOP_MESI  = 0x200,
    SNOOP_MOESI = 0x300
  };
  
  enum SNOOPPrivCohTrans {
    GetSTrans = 0,
    GetMTrans,
    PutMTrans,
    PutSTrans,
    UpgTrans,
    ExclTrans,
    InvTrans,
    NullTrans
  };

  enum class SNOOPPrivEventType {
    Core    = 0,
    ReqBus,
    RespBus,
    Null
  };

  enum class SNOOPPrivCoreEvent {
    Load = 0,
    Store,
    Replacement,
    Null 
  };

  enum class SNOOPPrivReqBusEvent {
    OwnGetS = 0,
    OwnGetM,
    OwnPutM,
    OwnPutS,
    OwnUpg,
    OtherGetS,
    OtherPutS,
    OtherGetM,
    OtherPutM, 
    OtherUpg,  
    OwnExclTrans,
    OtherExclTrans,   
    OwnInvTrans,  
    Null
  };

  enum class SNOOPPrivRespBusEvent {
    OwnDataResp = 0, 
    OwnDataRespExclusive,           
    OtherDataResp,
    Null
  };

   enum class SNOOPPrivCtrlAction {
       Stall = 0,                    
       Hit,                          
       issueTrans,                
       ReissueTrans,            
       issueTransSaveWbId,    
       WritBack,                     
       CopyThenHit,                          
       CopyThenHitWB,               
       CopyThenHitSendCoreOnly,
       CopyThenHitSendMemOnly,       
       CopyThenHitSendCoreMem, 
       SaveWbCoreId,                 
       HitSendMemOnly,              
       SendMemOnly,                 
       SendCoreOnly,                
       SendCoreMem,                 
       Fault,                       
       NoAck,                      
       NullAck,                     
       ProcessedAck                 
   };

  struct SNOOPPrivEventList {
    SNOOPPrivCoreEvent    cpuReqEvent;
    SNOOPPrivReqBusEvent  busReqEvent;
    SNOOPPrivRespBusEvent busRespEvent;
  };

  struct SNOOPPrivEventsCacheInfo {
    GenericCache::CacheLineInfo cpuReqCacheLineInfo;
    GenericCache::CacheLineInfo busReqCacheLineInfo;
    GenericCache::CacheLineInfo busRespCacheLineInfo;
  };

  struct SNOOPPrivStateList {
    int  cpuEventState;
    int  busReqEventState;
    int  busRespEventState;
  };

  enum class SNOOPPrivEventPriority {
    ReqBus,
    RespBus,
    WorkConserv,
  };

  struct SNOOPPrivMsgList {
    CpuFIFO  ::ReqMsg      cpuReqMsg = {};
    BusIfFIFO::BusReqMsg   busReqMsg = {};
    BusIfFIFO::BusRespMsg  busRespMsg= {};
  };

  enum class SNOOPSharedReqBusEvent {
    GetS = 0,
    GetM,
    Upg,
    OwnerPutM,
    OTherPutM,
    PutS,
    Null  
  };

  enum class SNOOPSharedCtrlEvent {
    DRAMGetM,
    DRAMGetS,
    VictCacheSwap,
    Replacement,
    Null  
  };
  
  enum class SNOOPSharedRespBusEvent {
    OWnDataResp = 0,      
    OTherDataResp,
    DRAMDataResp,
    NUll
  };

   enum class SNOOPSharedEventType {
     ReqBus = 0,
     RespBus,
     CacheCtrl
   };

  struct SNOOPSharedEventList {
    SNOOPSharedCtrlEvent    CtrlEvent;
    SNOOPSharedReqBusEvent  busReqEvent;
    SNOOPSharedRespBusEvent busRespEvent;
  };

  struct SNOOPSharedEventsCacheInfo {
    GenericCache::CacheLineInfo busReqCacheLineInfo;
    GenericCache::CacheLineInfo busRespCacheLineInfo;
  };

  struct SNOOPSharedStateList {
    int  CtrlEventState;
    int  busReqEventState;
    int  busRespEventState;
  };
          
  struct SNOOPSharedMsgList {
    BusIfFIFO::BusReqMsg   busReqMsg = {};
    BusIfFIFO::BusRespMsg  busRespMsg= {};
  };

  enum class SNOOPSharedCtrlAction {
    SharedNoAck = 0,
    SharedStall,
    SendData,
    SendDataExclusive,
    SendExclusiveResp,
    StoreData,
    StoreDataOnly,
    SaveReqCoreId,
    SendPendingData,
    SendVictimCache,
    CopyDRAMIntoCache,
    IssueDRAMWrite,
    IssueCoreInvDRAMWrite,
    IssueCoreInv,
    SendExclusiveRespStall,
    SWAPPING,
    SharedFault,
    SharedNullAck,
    SharedProcessedAck
  };
  
  enum class SNOOPSharedOwnerState{
    SharedMem = 0,
    OtherCore,
    SkipSet
  };
  
  struct SNOOPSharedOwnerList {
    SNOOPSharedOwnerState CtrlOwnerState;
    SNOOPSharedOwnerState busReqOwnerState;
    SNOOPSharedOwnerState busRespOwnerState;
  };

  struct SNOOPSharedActionList {
    SNOOPSharedCtrlAction CtrlAction;
    SNOOPSharedCtrlAction busReqAction;
    SNOOPSharedCtrlAction busRespAction;
  };
  

}

#endif /* _SNOOPProtocolCommon_H */
