/*
 * File  :      LatencyLogger.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On March 16, 2020
 */
#ifndef _LatencyLogger_H
#define _LatencyLogger_H

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "MemTemplate.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "GenericCache.h"
#include "PMSI.h"

namespace ns3 { 
  /**
   * brief LatencyLogger log latency components for each CPU request .
   *
  */
  class LatencyLogger : public ns3::Object {
  public:
    struct LatencyComponent {
      uint64_t msgId;
      uint64_t addr;
      CpuFIFO::REQTYPE reqType;
      uint64_t cpuReqCycle;
      uint64_t cpuTxFIFOInsertionCycle;
      uint64_t startProcessingCycle;
      uint64_t busTxFIFOInsertionCycle;
      uint64_t reqBusInsertionCycle;
      uint16_t arbitLatency;
      uint16_t busArbitLatency;
      int      respBusAccumlatedLatency;
      int      replacementLatency;
      uint16_t totalLatency;
      uint64_t cpuRxFIFOInsertionCycle;
      uint64_t absArivalCycle;
    };

  private:
    int         m_coreId;
    int         m_sharedMemId;
    double      m_dt;      
    double      m_clkSkew; 
    int         m_overSamplingRatio;
    uint32_t    m_privCacheBlkSize;
    uint32_t    m_privCacheNsets;
    bool        m_logFileGenEnable;
    uint16_t    m_wcl;
    uint16_t    m_wclMinRepl;
    uint16_t    m_wc_req;
    uint16_t    m_wc_resp;
    uint16_t    m_wc_replc;
    uint64_t    m_exec_cnt;
    uint64_t    m_acc_ltncy;
    uint64_t    m_CycleCnt;
    uint64_t    m_currMsgId,
                m_currRespId,
                m_currPrivBusReqId,
                m_currBusReqId,
                m_currBusRespCycle;

    int         m_prevFinishCycle;
    
    bool        m_latTraceEnable;

    std::string m_LatencyTraceFileName,
                m_LatencyReprtFileName;

    GenericFIFO <LatencyComponent  > m_latencyLogFIFO;

    // The output stream for cpu ack tracing
    std::ofstream m_TraceFileId,
                  m_ReprtFileId;

     // A pointer to CPU Interface FIFO
     ns3::Ptr<CpuFIFO> m_cpuFIFO;

     // A pointer to Cache Controller FIFO
     ns3::Ptr<BusIfFIFO> m_privCacheBusIfFIFO;

     // A pointer to Interconnect FIFO
     ns3::Ptr<ns3::InterConnectFIFO> m_interConnectFIFO;

    // Called by static method to process step
    // to insert new request or remove response
    // from assoicatedBuffers.
    void CycleProcess();

    void PushNewRequestorLatency ();
    void CoreHitEventChk ();
    void ChkReqBusLatency ();
    void ChkRespBusLatency ();
    void ChkPrivCacheReqBusEvent ();
    void DumpLatencyReport (LatencyComponent CandidateLatency);
    GenericCacheMapFrmt AddrMap (uint64_t phy_addr);
    void PrintReqMsg (BusIfFIFO::BusReqMsg busReqMsg, int format);
  public:
    // Override TypeId.
    static TypeId GetTypeId(void);

    // LatencyLogger constructor must associated with all buffers
    // Bus FIFOs and one Cpu FIFO to calculate latency components

    LatencyLogger (ns3::Ptr<ns3::InterConnectFIFO> interConnectFIFO,
                   ns3::Ptr<ns3:: CpuFIFO        > associatedCpuFIFO,
                   ns3::Ptr<ns3::BusIfFIFO       > privCacheBusIfFIFO);

    ~LatencyLogger();

   // set cpu dump file name
    void SetLatencyTraceFile (std::string fileName);
    void SetLatencyReprtFile (std::string fileName);

    // set CoreId
    void SetCoreId (int coreId);

    // get core id
    int GetCoreId ();

    // set dt
    void SetDt (double dt);

    // get dt
    int GetDt ();

    // set clk skew
    void SetClkSkew (double clkSkew);

    // set oversampling ratio
    void SetOverSamplingRatio (int overSamplingRatio);
  
    // set shared memory Id
    void SetSharedMemId (int sharedMemId);

    void SetPrivCacheNsets (uint32_t nsets);

    void SetPrivCacheBlkSize (uint32_t cacheBlkSize);

    void SetLogFileGenEnable (bool logFileGenEnable);

    // Initialize core generator
    void init();
  
    /**
     * Run LatencyLogger every clock cycle to check
     * latency component by monitoring the FIFOs
     */
     static void Step(Ptr<LatencyLogger> latencyLogger);
  };

}

#endif /* _LatencyLogger_H */
