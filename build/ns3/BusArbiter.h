/*
 * File  :      BusArbiter.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 21, 2020
 */

#ifndef _BusArbiter_H
#define _BusArbiter_H

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "MemTemplate.h"
#include "SNOOPProtocolCommon.h"
#include <string.h>

//#define Stall_CNT_LIMIT 200000
#define Stall_CNT_LIMIT 20000
using namespace std;

namespace ns3 { 
  /**
   * brief BusArbiter manage the scheduling of the different core
   * message on the shared interconnect
  */
  
  enum BusARBType {
    PISCOT_ARB       = 0x000,  
    UNIFIED_TDM_ARB  = 0x100, 
    FCFS_ARB         = 0x200,
    RT_ARB           = 0x300
  };
  
  enum AGENT {
    CORE = 0,
    SHAREDMEM,
    INTERCONNECT
  };
  
  class BusArbiter : public ns3::Object {
  private:
    double   m_dt; 
    double   m_clkSkew; 
    uint16_t m_cpuCore;
    uint16_t m_reqclks;

    uint16_t m_respclks;
    uint32_t m_cacheBlkSize;
    uint32_t m_nsets;
    bool     m_workconserv;
    bool     m_duetto;
    uint16_t m_reqCoreCnt;
    uint16_t m_respCoreCnt;
    uint64_t m_arbiCycle;
    bool     m_logFileGenEnable;
    bool     m_cach2Cache;
    bool     m_PndReq,
             m_PndOld,
             m_PndResp;
    int      m_TimeOut;
    int      m_delaycycle;
    uint64_t      m_delaycyclereg;
    bool     m_FcFsPndMemResp,
             m_PndPutMChk,
             m_DirectTransfer,
             m_IdleSlot,
             m_PndMemResp,
             oldest_determine,
             m_PndWB;
    bool     m_ReqWbFlag[32];
    unsigned int candidateID;
    bool PendingTxReq;
    uint16_t   m_sharedbankclks;
    int mode;
    string arb_req_mode;
    string arb_resp_mode;
    long long HPCycle;
    long long RTCycle;
    long long SwitchHPRT;
    long long SwitchRTHP;
    long long kreturn;
    long long priorityInversion;
    

    bool m_reza_log;

    bool     m_stallDetectionEnable;
    uint64_t m_stall_cnt;
    uint32_t m_wcArbiter;

    // Bus Arbiteration Type
    BusARBType  m_bus_arb;
    
    string m_bus_arch;
    
    string m_bus_arbiter;
    
    string m_reqbus_arb;
    
    string m_respbus_arb;
    
    CohProtType m_cohProType;
    
    int         m_maxPendingReq;
    
    // request bus interface message
    BusIfFIFO::BusReqMsg m_ReqBusMsg;
    BusIfFIFO::BusReqMsg m_ServQueueMsg;
    BusIfFIFO::BusRespMsg m_PendWBMsg;
    BusIfFIFO::BusRespMsg m_PendResp;

    // service queue                                            // Modified to Accomodate Multi Shared Cache
    // GenericDeque <BusIfFIFO::BusReqMsg> m_GlobalReqFIFO;       // Modified to Accomodate Multi Shared Cache

    
    // pointer to global queue
    ns3::Ptr<ns3::GlobalQueue> m_GlobalQueue;         // Modified to Accomodate Multi Shared Cache

    // A list of Cache Ctrl Bus interface buffers
    std::list<ns3::Ptr<ns3::BusIfFIFO> > m_busIfFIFO;

    // A pointer to shared cache Bus IF buffers
    std::list<ns3::Ptr<ns3::BusIfFIFO> > m_sharedCacheBusIfFIFO;   // Modified to Accomodate Multi Shared Cache

     // A pointer to Inteconnect FIFOs
    ns3::Ptr<ns3::InterConnectFIFO>  m_interConnectFIFO;
    
    void BusArbDecode();
    
    void CycleAdvance ();
    
    void ReqFncCall ();
    
    void RespFncCall ();
    
    void L2CohrMsgHandle(); 

    unsigned int retrieveSharedCacheID(uint64_t id);        // Modified to Accomodate Multi Shared Cache

    unsigned int retrieveCacheFIFOID(uint64_t id);   

    bool isOldest(uint64_t adr, unsigned int coreIndex);              // Modified to Accomodate Multi Shared Cache

    bool isFinal(uint64_t adr, unsigned int coreIndex, uint64_t sharedbank);              // Modified to Accomodate Multi Shared Cache

    bool removeFromOldest(uint64_t adr, unsigned int coreIndex, bool replacement);      // Modified to Accomodate Multi Shared Cache

    bool removeFromM_Type(uint64_t adr, unsigned int coreIndex, bool replacement);

    uint32_t retrieveSharedBankCacheLine (uint64_t phy_addr);

    bool removeFromNonOldest(uint64_t adr, unsigned int coreIndex, bool replacement);     // Modified to Accomodate Multi Shared Cache

    bool existServiceQueue(BusIfFIFO::BusReqMsg & tempMsgQueue);

    void assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg & msg);

    void adjustOldest(unsigned int coreIndex);                   // Modified to Accomodate Multi Shared Cache
    
    void SendData (BusIfFIFO::BusRespMsg msg, AGENT agent);  
    
    bool WCLator();

    void SendMemCohrMsg (BusIfFIFO::BusReqMsg msg, bool BroadCast);
       
    bool CheckPendingWB (uint16_t core_idx, BusIfFIFO::BusRespMsg & wbMsg, bool CheckOnly);

    void deadlineProgress();

    bool CheckPendingPutM (BusIfFIFO::BusReqMsg reqMsg, BusIfFIFO::BusReqMsg & putmReq);
    
    bool CheckPendingInvPutM (uint64_t addr, BusIfFIFO::BusReqMsg & putmReq);    

    bool CheckPendingReq  (uint16_t core_idx, BusIfFIFO::BusReqMsg & txMsg, bool CheckOnly, bool oldest);

    bool InsertOnReqBus (BusIfFIFO::BusReqMsg txMsg);

    bool existinTxResp(BusIfFIFO::BusReqMsg Msg);

    bool isHigherPrio(unsigned int l, unsigned int h);

    bool FcFsMemCheckInsert (uint16_t coreId,uint64_t addr, bool CheckOnly, bool SkipAddrCheck);

    bool CheckPendingResp (BusIfFIFO::BusRespMsg & txMsg, bool ChkOnly, bool oldest, int &mode);  

    bool CheckPendingWriteBackResp (bool CheckOnly, BusIfFIFO::BusRespMsg & txResp, bool oldest);

    bool FcFsWriteBackCheckInsert (uint16_t core_idx, uint64_t addr, bool CheckOnly, BusIfFIFO::BusRespMsg & txResp);

   // void PMSI_TDM_ReqBus();

    //void PMSI_OOO_TDM_ReqBus();

    //void PMSI_FcFsRespBus();

    //void PISCOT_MSI_TDM_ReqBus(); 

    //void PISCOT_OOO_MSI_TDM_ReqBus();

    //void MSI_FcFsReqBus();

    //void PISCOT_MSI_FcFsResBus();

    bool CheckPendingFCFSReq (BusIfFIFO::BusReqMsg & txMsg, bool ChkOnly); 
    
    bool CheckPendingFCFSResp (BusIfFIFO::BusRespMsg & txMsg, bool ChkOnly);
    
    void MSI_FcFsRespBus();
    
    void MSI_FcFsReqBus();
    
    //void Unified_TDM_PMSI_Bus ();
    
    //void Unified_TDM_PMSI_Bus2 ();

    void RR_RT_ReqBus();

    void RR_RT_RespBus();















  public:
    static TypeId GetTypeId(void);

    BusArbiter(std::list<ns3::Ptr<ns3::BusIfFIFO> > associatedPrivCacheBusIfFIFO,
               std::list<ns3::Ptr<ns3::BusIfFIFO> > assoicateLLCBusIfFIFO,
               ns3::Ptr<ns3::InterConnectFIFO>  interConnectFIFO,
               ns3::Ptr<ns3::GlobalQueue>  globalqueue);  // Modified to Accomodate Multi Shared Cache 


    ~BusArbiter();

    void SetDt (double dt);

    int GetDt ();

    void SetClkSkew (double clkSkew);

    void SetCacheBlkSize (uint32_t cacheBlkSize);

    void SetSharedCacheLatency (uint32_t latency) ;
    
    void SetNumPrivCore (int nPrivCores);

    unsigned createMask(unsigned a, unsigned b);

    void SetLogFileGenEnable (bool logFileGenEnable);

    void SetNumReqCycles (int ncycle);

    void SetNumRespCycles (int ncycle);
 
    void SetCacheNsets (uint32_t nsets);

    void SetIsWorkConserv (bool workConservFlag);

    void SetIsDuetto(bool duetto);

    void SetCache2Cache (bool cach2Cache);
    
    void SetBusArchitecture (string bus_arch);
    
    void SetBusArbitration (string bus_arb);
    
    void SetReqBusArb (string reqbus_arb); 
    
    void SetRespBusArb (string respbus_arb);               
    
    void SetCohProtType (CohProtType ptype);
    
    void SetMaxPendingReq (int maxPendingReq);
   
    void init();

    /**
     * Run Shared Bus instances every clock cycle to
     * update cache line states and generate coherence 
     * messages. This function does the scheduling
     */
     static void ReqStep(Ptr<BusArbiter> busArbiter);

     static void RespStep(Ptr<BusArbiter> busArbiter);

     static void L2CohrStep(Ptr<BusArbiter> busArbiter);
     
     static void Step(Ptr<BusArbiter> busArbiter);
  };
}

#endif /* _BusArbiter_H */
