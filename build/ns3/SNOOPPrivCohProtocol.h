/*
 * File  :      SNOOPPrivCohProtocol.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On May 29, 2020
 */

#ifndef _SNOOPPrivCohProtocol_H
#define _SNOOPPrivCohProtocol_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "CohProtocolCommon.h"

namespace ns3 { 

  enum CacheField {
       State = 0,
       Tag,
       Data,
       Line
  };

  class SNOOPPrivCohProtocol: public ns3::Object {
  private:
    CohProtType                                 m_pType;
    int                                         m_coreId;
    int                                         m_sharedMemId;
    bool                                        m_cache2Cache;
    Ptr<GenericCache>                           m_privCache;
    Ptr<CpuFIFO>                                m_cpuFIFO;
    Ptr<BusIfFIFO>                              m_busIfFIFO;
    SNOOPPrivEventList                          m_eventList;
    SNOOPPrivMsgList                            m_msgList;
    SNOOPPrivEventsCacheInfo                    m_eventCacheInfoList;
    int                                         m_currEventCurrState;
    int                                         m_currEventNextState;
    SNOOPPrivCohTrans                           m_currEventTrans2Issue;
    SNOOPPrivEventType                          m_processEvent;
    SNOOPPrivCtrlAction                         m_ctrlAction;
    int                                         m_prllActionCnt;
    int                                         m_reqWbRatio;
    ReplcPolicy                                 m_replcPolicy;
    Ptr<UniformRandomVariable>                  uRnd1;
    bool                                        m_logFileGenEnable;
    bool                                        m_reza_log_snoop;
  public:
    static TypeId GetTypeId(void);

    SNOOPPrivEventType GetCurrProcEvent (); 

    SNOOPPrivCohProtocol();

    ~SNOOPPrivCohProtocol();
     
    void SetLogFileGenEnable (bool logFileGenEnable);

    void SetCoreId (int coreId);

    void SetSharedMemId (int sharedMemId);

    void SetProtocolType (CohProtType ptype);

    CohProtType GetProtocolType ();    

    void SetPrivCachePtr (Ptr<GenericCache> privCache);

    void SetCpuFIFOPtr (Ptr<CpuFIFO> cpuFIFO);

    void SetBusFIFOPtr (Ptr<BusIfFIFO> busFIFO);

    void SetCache2Cache (bool cache2Cache);

    void SetReqWbRatio (int reqWbRatio);

    CpuFIFO::ReqMsg GetCpuReqMsg ();

    void SetCpuReqMsg (CpuFIFO::ReqMsg cpuReqMsg);

    BusIfFIFO::BusReqMsg GetBusReqMsg ();

    BusIfFIFO::BusRespMsg GetBusRespMsg ();

    SNOOPPrivCoreEvent GetCpuReqEvent ();

    void SetCpuReqEvent (SNOOPPrivCoreEvent cpuReqEvent);

    SNOOPPrivReqBusEvent GetBusReqEvent ();

    SNOOPPrivRespBusEvent GetBusRespEvent ();

    void SetCurrEventCtrlAction (SNOOPPrivCtrlAction currEvent);

    SNOOPPrivCtrlAction GetCurrEventCtrlAction ();

    int GetCurrEventCacheCurrState ();

    int GetCurrEventCacheNextState ();

    GenericCache::CacheLineInfo GetCurrEventCacheLineInfo ();

    void UpdateCacheLine (CacheField       field, 
                          GenericCacheFrmt cacheLineInfo, 
                          uint32_t         set_idx, 
                          uint32_t         way_idx);

    SNOOPPrivCohTrans GetCurrEventCohrTrans ();

    int InvalidCacheState ();
    
    void InitializeCacheStates ();

    void ChkCoreEvent ();

    void ChkReqBusEvent();

    void ChkRespBusEvent ();

    void ChkCohEvents ();

    void GetEventsCacheInfo ();
    
    void CohEventsSerialize ();

    void ProcessCoreEvent ();

    void ProcessBusEvents ();

    void CohProtocolFSMProcessing ();

    SNOOPPrivCohTrans GetCohTrans ();

    bool IsCacheBlkValid (int s);

    SNOOPPrivEventPriority PrivEventPriorityBinding (IFCohProtocol *obj);

    void CohProtocolFSMBinding (IFCohProtocol *obj);

    std::string PrintPrivStateName (int state);

    void PrintEventInfo ();

    std::string PrintPrivEventType  (SNOOPPrivEventType event);

    std::string PrintPrivActionName (SNOOPPrivCtrlAction action);

    std::string PrintTransName (SNOOPPrivCohTrans trans);

    std::string PrivReqBusEventName (SNOOPPrivReqBusEvent event);

    std::string PrivRespBusEventName (SNOOPPrivRespBusEvent event);

  }; // class CohProtocol
}

#endif /* _SNOOPPrivCohProtocol_H */

