/*
 * File  :      PrivateCacheCtrl.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 17, 2020
 */

#ifndef _PrivateCacheCtrl_H
#define _PrivateCacheCtrl_H

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "MemTemplate.h"
#include "SNOOPPrivCohProtocol.h"
#include "GenericCache.h"


#include <string>

namespace ns3 { 
  /**
   * brief PrivateCacheCtrl Implements Cache coherence protocol, it is
   * the main interface between CPU core and the bus arbiter, the cache
   * controller communicates with bus arbiter through FIFO bi-directional
   * channel. In addition, it has direct access to cpu private cache.  
  */
  
  class PrivateCacheCtrl : public ns3::Object {
  private:
     enum TransType {
       CoreOnly = 0,
       MemOnly,
       CorePlsMem
     };
     struct PendingMsg {
       CpuFIFO::ReqMsg cpuMsg;
       SNOOPPrivCoreEvent cpuReqEvent;
       bool IsProcessed;
       bool IsPending;
     };

     Ptr<GenericFIFO <PendingMsg >> m_cpuPendingFIFO;

     int      m_maxPendingReq;
     int      m_pendingCpuReq;

     // private cache parameters
     uint16_t m_cacheType;
     uint32_t m_cacheSize;
     uint32_t m_cacheBlkSize;
     uint32_t m_nways;
     uint32_t m_nsets;
     bool     m_duetto;
     int      m_coreId;
     int      m_sharedMemId;
     double   m_dt; 
     double   m_clkSkew; 
     int      m_reqWbRatio;
     uint64_t m_cacheCycle;
     bool     m_cach2Cache;
     int      m_prllActionCnt;
     bool     m_logFileGenEnable;
     bool     m_reza_log_private; 

     string m_mode;

     GenericDeque <BusIfFIFO::BusReqMsg> m_PendingWbFIFO;

     // A pointer to CPU Interface FIFO
     Ptr<CpuFIFO>   m_cpuFIFO;

     // A pointer to Bus Interface FIFO
     Ptr<BusIfFIFO> m_busIfFIFO;
    
    // pointer to global queue
    ns3::Ptr<ns3::GlobalQueue> m_GlobalQueue;         // Modified to Accomodate Multi Shared Cache

     // A pointer to Private cache
     Ptr<GenericCache> m_cache;

    // A pointer to Cache Coherence Protocol 
    Ptr<ns3::SNOOPPrivCohProtocol> m_cohProtocol;

    // Coherence Protocol Type (e.x. MSI, PMSI)
    CohProtType                              m_pType;

     // insert new Transaction into BusTxMsg FIFO 
     bool PushMsgInBusTxFIFO (uint64_t       msgId, 
                              uint16_t       reqCoreId, 
                              uint16_t       wbCoreId, 
                              uint16_t       transId, 
                              uint64_t       addr,
                              uint64_t       sharedcacheid,
                              bool           PendingWbBuf,                              
                              bool           NoGetMResp                              
                              );

     // execute write back command
     bool DoWriteBack (uint64_t addr, uint16_t wbCoreId, uint64_t msgId, bool dualTrans);

     // send pending WB to TxResp FIFO
     bool SendPendingWB  (GenericCacheMapFrmt recvTrans, TransType type);

     bool PendingCoreBufRemoveMsg  (uint64_t msgId, PendingMsg &removedMsg);
     
     bool MOESI_Modify_NoGetMResp_TxFIFO (uint64_t addr);

     // Called by static method to process step
     // to insert new request or remove response
     // from assoicatedBuffers.
     void CycleProcess  ();
     void CacheCtrlMain ();

  public:
    // Override TypeId.
    static TypeId GetTypeId(void);

    PrivateCacheCtrl(uint32_t cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO, 
                     Ptr<CpuFIFO  > associatedCpuFIFO,
                     ns3::Ptr<ns3::GlobalQueue>  globalqueue);

    ~PrivateCacheCtrl();
     
    void SetCacheSize (uint32_t cacheSize);

    uint32_t GetCacheSize ();

    void SetCacheBlkSize (uint32_t cacheBlkSize);

    uint32_t GetCacheBlkSize ();

    void SetCacheNways (uint32_t nways);

    uint32_t GetCacheNways ();

    void SetCacheNsets (uint32_t nsets);

    uint32_t GetCacheNsets ();

    void SetCacheType (uint16_t cacheType);

    uint16_t GetCacheType ();

    void SetCoreId (int coreId);

    void SetSharedMemId (int sharedMemId);

    int GetCoreId ();

    void SetDt (double dt);

    void SetIsDuetto (bool duetto);

    int GetDt ();

    void SetClkSkew (double clkSkew);

    void SetReqWbRatio (int reqWbRatio);

    void SetCache2Cache (bool cach2Cache);

    unsigned createMask(unsigned a, unsigned b);    

    unsigned int retrieveCacheFIFOID(uint64_t id);

    void SetLogFileGenEnable (bool logFileGenEnable);

    void SetMaxPendingReq (int maxPendingReq);

    void SetPendingCpuFIFODepth (int size);

    void SetProtocolType (CohProtType ptype);

    void init();
  
    /**
     * Run PrivateCacheCtrl every clock cycle to
     * update cache line states and generate coherence 
     * messages. This function does the scheduling
     */
     static void Step(Ptr<PrivateCacheCtrl> privateCacheCtrl);
  };
}

#endif /* _PrivateCacheCtrl_H */
