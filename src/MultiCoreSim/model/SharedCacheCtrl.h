/*
 * File  :      SharedCacheCtrl.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 20, 2020
 */

#ifndef _SharedCacheCtrl_H
#define _SharedCacheCtrl_H

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "MemTemplate.h"
#include "GenericCache.h"
#include "PMSI.h"
#include "MSI.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CohProtocolCommon.h"

namespace ns3 { 
  /**
   * brief SharedCacheCtrl Implements Cache coherence protocol on the 
   * shared memory side, it is the main interface between LLC and the 
   * bus arbiter, the cache controller communicates with bus arbiter 
   * through FIFO bi-directional channel. 
  */
  
  class SharedCacheCtrl : public ns3::Object {
  private:
     enum CacheField {
       State = 0,
       Tag,
       Data,
       Line
     };
     
     enum SendDataType {
       DataOnly = 0,
       ExclOnly,
       DataPlsExcl,
       CoreInv
     };
     
     // shared cache parameters
     uint16_t   m_cacheType;
     uint32_t   m_cacheSize;
     uint32_t   m_cacheBlkSize;
     uint32_t   m_victimCacheSize;
     uint32_t   m_nways;
     uint32_t   m_nsets;
     int        m_coreId;
     double     m_dt; 
     double     m_clkSkew; 
     uint64_t   m_cacheCycle;
     bool       m_cache2Cache;
     bool       m_duetto;
     int        m_nPrivCores;
     bool       m_logFileGenEnable;
     bool       m_l2CachePreloadFlag;
     bool       m_dramSimEnable;
     uint32_t   m_dramLatcy;
     uint32_t   m_dramOutstandReq;
     uint32_t   m_reqclks;                
     uint32_t   m_respclks;
     uint32_t   m_sharedcachelatency;     // Modified to Accomodate Multi Shared Cache
     uint32_t   m_sharedcachelatencyCounter;     // Modified to Accomodate Multi Shared Cache
     bool       m_sharedCacheBusy;     // Modified to Accomodate Multi Shared Cache
     uint64_t   m_Nreqs;
     uint64_t   m_Nmiss;
     string     m_mode;
     uint64_t   m_wcShared;
     bool       m_txexist;
     



     std::string m_dramModle;
     
     ReplcPolicy m_replcPolicy;

     bool not_busy;

     SNOOPSharedMsgList        m_CurrEventMsg;
     SNOOPSharedEventList      m_CurrEventList;
     SNOOPSharedActionList     m_CurrEventAction;  
     SNOOPSharedStateList      m_CurrEventCurrState,
                               m_CurrEventNextState;
     SNOOPSharedOwnerList      m_CurrEventOwnerCurrState,
                               m_CurrEventOwnerNextState;
     
     CohProtType m_cohrProt;

     std::string m_bmsPath;

     uint16_t * m_ownerCoreId;
     
     uint16_t * m_victimOwnerCoreId;

     Ptr<BusIfFIFO> m_busIfFIFO;

     bool m_direction;
      bool m_reza_log;
     bool m_msgBypass;

     unsigned int m_msgBypass_msgID, m_msgBypass_reqID, m_msgBypass_respID;

     bool m_reza_log_shared;

     // A list of Cache Ctrl Bus interface buffers
     std::list<ns3::Ptr<ns3::BusIfFIFO> > m_busIfFIFOPrivate;  // Modified to Accomodate Multi Shared Cache

     Ptr<DRAMIfFIFO> m_dramBusIfFIFO;

     // pointer to global queue
     ns3::Ptr<ns3::GlobalQueue> m_GlobalQueue;         // Modified to Accomodate Multi Shared Cache
    
     GenericFIFO <BusIfFIFO::BusReqMsg> m_PndWBFIFO;

     GenericFIFO <BusIfFIFO::BusRespMsg> m_localPendingRespTxBuffer;
     
     GenericFIFO <uint64_t> m_DRAMOutStandingExclMsg;

     Ptr<GenericCache> m_cache;
     
     Ptr<VictimCache> m_victimCache;

    // SNOOPSharedReqBusEvent  ChkBusRxReqEvent  (BusIfFIFO::BusReqMsg &  busReqMsg,  bool & not_busy, bool bypass);
     SNOOPSharedReqBusEvent  ChkBusRxReqEvent  (BusIfFIFO::BusReqMsg &  busReqMsg);

     SNOOPSharedRespBusEvent ChkBusRxRespEvent (BusIfFIFO::BusRespMsg & busRespMsg, BusIfFIFO::BusRespMsg busRespMsg_temp);  
     
     SNOOPSharedCtrlEvent ChkDRAMReqEvent (SNOOPSharedReqBusEvent busReqEvent);

     void UpdateSharedCache                     (CacheField field, 
                                                 uint64_t addr, 
                                                 int state, 
                                                 uint8_t * data,
                                                 bool UpdateAccessCnt);
                                                 
     void VictimCacheLineEvict (uint32_t victimWayIdx);
     
     void SendExclRespEarly ();
     
     bool RemoveExclRespAddr ();

     void assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg & msg);

     bool terminateBank(unsigned int coreID, uint64_t adr, unsigned int msg );                           // Modified to Accomodate Multi Shared Cache

     bool isOldest(uint64_t adr, unsigned int coreIndex);              // Modified to Accomodate Multi Shared Cache

     bool sameCacheLineRX(BusIfFIFO::BusRespMsg busRespMsg);

     bool sameCacheLineTX(BusIfFIFO::BusRespMsg busRespMsg);

     bool removeFromOldest(uint64_t adr, unsigned int coreIndex);      // Modified to Accomodate Multi Shared Cache

     bool removeFromNonOldest(uint64_t adr, unsigned int coreIndex);     // Modified to Accomodate Multi Shared Cache

     bool removeFromM_Type(uint64_t adr, unsigned int coreIndex); 

     void adjustOldest(unsigned int coreIndex);                   // Modified to Accomodate Multi Shared Cache

     bool FetchLine (uint64_t addr, GenericCacheFrmt & cacheLine, uint32_t & LineWayIdx);

     bool DoWriteBack (uint64_t cl_idx, uint16_t wbCoreId, uint64_t msgId, double timestamp , SendDataType type);

     bool PushMsgInBusTxFIFO  (uint64_t       msgId, 
                                                uint16_t       reqCoreId, 
                                                uint16_t       wbCoreId, 
                                                uint64_t       addr);

     void SendPendingReqData  (GenericCacheMapFrmt recvTrans );

     void CycleProcess  ();
     
     void CacheCtrlMain ();
     
     void CacheInitialize();

     bool Bank_WCLator();
     
    void CohProtocolFSMProcessing (SNOOPSharedEventType eventType, int state, SNOOPSharedOwnerState owner);
    
    int GetWaitDRAMRespState();
    
    void InitCacheStates ();
    
    int ResetCacheState ();
        
    bool SendDRAMReq (uint64_t msgId, uint64_t addr, DRAMIfFIFO::DRAM_REQ type);
    
    std::string PrintSharedActionName (SNOOPSharedCtrlAction action);
    
    std::string PrintSharedRespBusEventName (SNOOPSharedRespBusEvent event);
    
    std::string PrintSharedReqBusEventName (SNOOPSharedReqBusEvent event);
    
    std::string PrintSharedStateName (int state);

    bool CheckBypass (int state);

    uint16_t m_reqCoreCnt;

  public:
    static TypeId GetTypeId(void);

    SharedCacheCtrl (std::list<ns3::Ptr<ns3::BusIfFIFO> > associatedPrivCacheBusIfFIFO,
                     uint32_t       cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO,
                     Ptr<DRAMIfFIFO> associatedDRAMBusIfFifo,
                     ns3::Ptr<ns3::GlobalQueue>  globalqueue);


    ~SharedCacheCtrl();
     
    void SetCacheSize (uint32_t cacheSize);

    uint32_t GetCacheSize ();

    void SetCacheBlkSize (uint32_t cacheBlkSize);

    uint32_t GetCacheBlkSize ();

    void SetCacheNways (uint32_t nways);

    uint32_t GetCacheNways ();

    void SetCacheNsets (uint32_t nsets);

    uint32_t GetCacheNsets ();

    void SetCacheType (uint16_t cacheType);
    
    void SetVictCacheSize (uint32_t cacheSize);

    void SetNumReqCycles(int ncycle);  

    void SetNumRespCycles(int ncycle);

    uint16_t GetCacheType ();

    void SetCoreId (int coreId);
    
    void SetIsDuetto (bool duetto);

    int GetCoreId ();

    void SetDt (double dt);

    int GetDt ();

    void SetClkSkew (double clkSkew);

    void SetCache2Cache (bool cache2Cache);
    
    void SetReplcPolicy (ReplcPolicy replcPolicy);

    void SetBMsPath  (std::string bmsPath);

    void SetNumPrivCore (int nPrivCores);
    
    void SetProtocolType (CohProtType ptype);

    void SetLogFileGenEnable (bool logFileGenEnable);
    
    void SetCachePreLoad (bool l2CachePreload);
    
    void SetDramSimEnable (bool dramSim_en );
    
    void SetDramFxdLatcy (uint32_t dramLatcy );
    
    void SetDramModel (std::string dramModel );
    
    void SetDramOutstandReq (uint32_t dramOutstandReqs);

    void SetSharedCacheLatency(uint32_t latency);           // Modified to Accomodate Multi Shared Cache
    
    uint64_t GetShareCacheMisses ();
    
    uint64_t GetShareCacheNReqs ();
    
    void init();

     static void Step(Ptr<SharedCacheCtrl> sharedCacheCtrl);

  };
}

#endif /* _SharedCacheCtrl_H */

