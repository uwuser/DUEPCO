/*
 * File  :      LatencyLogger.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On March 17, 2020
 */

#include "LatencyLogger.h"
using namespace std;
namespace ns3 {

    // override ns3 type
    TypeId LatencyLogger::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::LatencyLogger")
               .SetParent<Object > ();
        return tid;
    }

    LatencyLogger::LatencyLogger (ns3::Ptr<ns3::InterConnectFIFO> interConnectFIFO,
                                  ns3::Ptr<ns3:: CpuFIFO        > associatedCpuFIFO,
                                  ns3::Ptr<ns3::BusIfFIFO       > privCacheBusIfFIFO)
    {
        // default
        m_coreId                = 0;
        m_sharedMemId           = 10;
        m_dt                    = 1;      
        m_clkSkew               = 0; 
        m_overSamplingRatio     = 2;
        m_wcl                   = 0;
        m_wclMinRepl            = 0;
        m_wc_req                = 0;
        m_wc_resp               = 0;
        m_wc_replc              = 0;
        m_exec_cnt              = 0;
        m_acc_ltncy             = 0;
        m_CycleCnt              = 0;
        m_logFileGenEnable      = false;
        m_interConnectFIFO      = interConnectFIFO;
        m_cpuFIFO               = associatedCpuFIFO;
        m_privCacheBusIfFIFO    = privCacheBusIfFIFO;
        m_latencyLogFIFO.SetFifoDepth(4*associatedCpuFIFO->m_txFIFO.GetFifoDepth());
        m_currMsgId             = 0;
        m_currRespId            = 0;
        m_currPrivBusReqId      = 0;
        m_currBusReqId          = 0;
        m_currBusRespCycle      = 0;
        m_privCacheBlkSize      = 64;
        m_privCacheNsets        = 256;
        m_prevFinishCycle       = 0;
        m_latTraceEnable        = false;
    }

    // We don't do any dynamic allocations
    LatencyLogger::~LatencyLogger() {
    }

    // set CoreId
    void LatencyLogger::SetCoreId (int coreId) {
      m_coreId = coreId;
    }

    // get core id
    int LatencyLogger::GetCoreId () {
      return m_coreId;
    }

    // set dt
    void LatencyLogger::SetDt (double dt) {
      m_dt = dt;
    }

    // get dt
    int LatencyLogger::GetDt () {
      return m_dt;
    }

    // set clk skew
    void LatencyLogger::SetClkSkew (double clkSkew) {
       m_clkSkew = clkSkew;
    }

    // set oversampling ratio
    void LatencyLogger::SetOverSamplingRatio (int overSamplingRatio) {
       m_overSamplingRatio = overSamplingRatio;
    }

    // set clk skew
    void LatencyLogger::SetSharedMemId (int sharedMemId) {
       m_sharedMemId = sharedMemId;
    }

    // set latency trace file name
    void LatencyLogger::SetLatencyTraceFile (std::string fileName) {
      m_LatencyTraceFileName = fileName;
    }

    // set latency Report file name
    void LatencyLogger::SetLatencyReprtFile (std::string fileName) {
      m_LatencyReprtFileName = fileName;
    }

    void LatencyLogger::SetPrivCacheNsets (uint32_t nsets) {
      m_privCacheNsets = nsets;
    }

    void LatencyLogger::SetPrivCacheBlkSize (uint32_t cacheBlkSize) {
      m_privCacheBlkSize = cacheBlkSize;
    }

    void LatencyLogger::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }

    // Map physical address to private cache format
    GenericCacheMapFrmt LatencyLogger::AddrMap (uint64_t phy_addr) {
      GenericCacheMapFrmt cacheLine;

      cacheLine.idx_set = (phy_addr >> (int) log2(m_privCacheBlkSize)) & (m_privCacheNsets-1);  
      cacheLine.ofst    = phy_addr & (m_privCacheBlkSize-1);  
      cacheLine.tag     = phy_addr >> ((int) log2(m_privCacheBlkSize)+(int) log2(m_privCacheNsets));
      return cacheLine;
    }

    // The init function starts the controller at the beginning 
    void LatencyLogger::init() {
        if (m_latTraceEnable) {
          m_TraceFileId.open(m_LatencyTraceFileName.c_str());
        }
        m_ReprtFileId.open(m_LatencyReprtFileName.c_str());
        m_ReprtFileId << "ReqId,ReqAddr,RefTraceCycle,ReqBusLatency,RespBusLatency,ReplcLatency,LatencyMinusRepl,TotLatency,WcReqLatency,WcRespLatency,WcReplcLatency,WcLatency,WcLatencyMinsRepl,AverageLatency" << std::endl;
        Simulator::Schedule(NanoSeconds(m_clkSkew), &LatencyLogger::Step, Ptr<LatencyLogger > (this));
    }


    void LatencyLogger::PushNewRequestorLatency () {
      //cout<<"In PushNewRequestorLatency "<<endl;     
      if (!m_cpuFIFO->m_txFIFO.IsEmpty()) {
       // cout<<"In PushNewRequestorLatency - not empty"<<endl;  
        CpuFIFO::ReqMsg cpuReqMsg = m_cpuFIFO->m_txFIFO.GetFrontElement();
        if (cpuReqMsg.msgId != m_currMsgId) {
          if (m_logFileGenEnable) {
            std::cout << "New Request Inserted In Cpu FIFO " << cpuReqMsg.reqCoreId << " MsgId = " << cpuReqMsg.msgId << " addr = " << cpuReqMsg.addr << std::endl;
          }
          m_currMsgId = cpuReqMsg.msgId;
          LatencyComponent newLatencyCandidate;
          newLatencyCandidate.msgId                      = cpuReqMsg.msgId;
          newLatencyCandidate.addr                       = cpuReqMsg.addr;
          newLatencyCandidate.reqType                    = cpuReqMsg.type;
          newLatencyCandidate.cpuReqCycle                = cpuReqMsg.cycle;
          newLatencyCandidate.cpuTxFIFOInsertionCycle    = cpuReqMsg.fifoInserionCycle;
          newLatencyCandidate.startProcessingCycle       = 0;
          newLatencyCandidate.busTxFIFOInsertionCycle    = 0;
          newLatencyCandidate.absArivalCycle             = 0;
          newLatencyCandidate.reqBusInsertionCycle       = 0;
          newLatencyCandidate.arbitLatency               = 0;
          newLatencyCandidate.respBusAccumlatedLatency   = 0;
          newLatencyCandidate.replacementLatency         = 0;
          newLatencyCandidate.totalLatency               = 0;
          newLatencyCandidate.cpuRxFIFOInsertionCycle    = 0;
          if (!m_latencyLogFIFO.IsFull()) {
            //cout<<"In PushNewRequestorLatency - 3"<<endl;
            m_latencyLogFIFO.InsertElement(newLatencyCandidate);
          }
          else {
            if (m_logFileGenEnable) {
              std::cout << "LatencyLogger " << m_coreId << " LatencyFIFO OverFlow Detected!" << std::endl;
            }
            exit(0);
          }
        }
      }
      //cout<<"In PushNewRequestorLatency - 4"<<endl;  
    }

    void LatencyLogger::DumpLatencyReport (LatencyComponent CandidateLatency) {
       m_exec_cnt++;
       if (CandidateLatency.reqBusInsertionCycle != 0){
         if ((CandidateLatency.reqBusInsertionCycle*m_overSamplingRatio) < (unsigned)m_prevFinishCycle){
           CandidateLatency.respBusAccumlatedLatency = CandidateLatency.respBusAccumlatedLatency - (m_prevFinishCycle/m_overSamplingRatio) + CandidateLatency.reqBusInsertionCycle;
           if (CandidateLatency.respBusAccumlatedLatency < 0) CandidateLatency.respBusAccumlatedLatency = 0;
         }
       }

       uint16_t totalLatency = CandidateLatency.arbitLatency + CandidateLatency.respBusAccumlatedLatency;
       uint16_t totalLatencyMinsRepl = totalLatency - CandidateLatency.replacementLatency;
       m_prevFinishCycle = m_CycleCnt;
       if (totalLatency == 0) {
          totalLatency = 1;
       }

      if (m_wcl < totalLatency) {
        m_wcl = totalLatency;
      }

      if (m_wclMinRepl < totalLatencyMinsRepl) {
        m_wclMinRepl = totalLatencyMinsRepl;
      }

      if (m_wc_req < CandidateLatency.arbitLatency) {
        m_wc_req = CandidateLatency.arbitLatency;
      }

      if (m_wc_resp < CandidateLatency.respBusAccumlatedLatency) {
        m_wc_resp = CandidateLatency.respBusAccumlatedLatency;
      }

      if (m_wc_replc < CandidateLatency.replacementLatency) {
        m_wc_replc = CandidateLatency.replacementLatency;
      }

      m_acc_ltncy += totalLatency;

      m_ReprtFileId << CandidateLatency.msgId << "," << CandidateLatency.addr << ","  << CandidateLatency.cpuReqCycle << ","  << CandidateLatency.arbitLatency << ","  << CandidateLatency.respBusAccumlatedLatency << ","  << CandidateLatency.replacementLatency << ","  <<  totalLatencyMinsRepl << ","  << totalLatency << "," << m_wc_req << "," << m_wc_resp << "," << m_wc_replc << "," << m_wcl << "," << m_wclMinRepl << "," << m_acc_ltncy/m_exec_cnt  << std::endl; 

      int pendingQueueSize = m_latencyLogFIFO.GetQueueSize();
      LatencyComponent pendingMsg ;
      for (int i = 0; i < pendingQueueSize ;i++) {
         pendingMsg = m_latencyLogFIFO.GetFrontElement();
         pendingMsg.replacementLatency = 0;
         m_latencyLogFIFO.PopElement();
         m_latencyLogFIFO.InsertElement(pendingMsg);
         
      }
    }

    void LatencyLogger::CoreHitEventChk () {
      if (!m_cpuFIFO->m_rxFIFO.IsEmpty()) {
        CpuFIFO::RespMsg cpuRespMsg = m_cpuFIFO->m_rxFIFO.GetFrontElement();
        if (cpuRespMsg.msgId != m_currRespId) {
          m_currRespId = cpuRespMsg.msgId;
          bool LatencyCandProcDone = false;
          int pendingQueueSize = m_latencyLogFIFO.GetQueueSize();
          LatencyComponent pendingMsg ;
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingMsg = m_latencyLogFIFO.GetFrontElement();
            // Remove message from the busReq buffer
            m_latencyLogFIFO.PopElement();
            if (m_logFileGenEnable) {
              std::cout << "LatencyLogger: Cpu " << m_coreId << " Received Ack curr respId = " << m_currRespId << " buffer Id " << pendingMsg.msgId << std::endl;
            }
            if (pendingMsg.msgId == m_currRespId) {
              pendingMsg.cpuRxFIFOInsertionCycle = cpuRespMsg.cycle;

              if (pendingMsg.busTxFIFOInsertionCycle == 0) {
                if (m_latTraceEnable) {
                  m_TraceFileId << "Cpu " << m_coreId << " New Request Id " << cpuRespMsg.msgId << " to address " << cpuRespMsg.addr << " and RefTraceCycle = " << pendingMsg.cpuReqCycle << std::endl;
                }
                pendingMsg.startProcessingCycle = cpuRespMsg.cycle;
              }
              // dump latency here
              DumpLatencyReport (pendingMsg);

              LatencyCandProcDone = true;
            }
            else {
              // Dequeue the data again into latency buffer
              m_latencyLogFIFO.InsertElement(pendingMsg);
            }
          }
          if (LatencyCandProcDone == false) {
            if (m_logFileGenEnable) {
              std::cout << "LatencyLogger: Cpu Received Ack to non-exist Request in the Latency FIFO!" << std::endl;
            }
            exit(0);
          }
 
          if (m_latTraceEnable) {
            m_TraceFileId << "\t|----- Cpu " << m_coreId << " receives Ack for Msg Id " << cpuRespMsg.msgId << " @ Cycle " << cpuRespMsg.cycle << std::endl << std::endl;
          }
        }
      }
    }

    void LatencyLogger::ChkPrivCacheReqBusEvent () {
      if (!m_privCacheBusIfFIFO->m_txMsgFIFO.IsEmpty()) {
        BusIfFIFO::BusReqMsg busReqMsg = m_privCacheBusIfFIFO->m_txMsgFIFO.GetFrontElement();
        if (busReqMsg.msgId != m_currPrivBusReqId && busReqMsg.reqCoreId == m_coreId) {
          m_currPrivBusReqId = busReqMsg.msgId;
          int pendingQueueSize = m_latencyLogFIFO.GetQueueSize();
          LatencyComponent pendingMsg ;
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingMsg = m_latencyLogFIFO.GetFrontElement();
            GenericCacheMapFrmt addrMap   = AddrMap(pendingMsg.addr);
            // Remove message from the busReq buffer
            m_latencyLogFIFO.PopElement();
            if (pendingMsg.msgId == busReqMsg.msgId) {
              if (m_latTraceEnable) {
                m_TraceFileId << "Cpu " << m_coreId << " New ReqId = " << pendingMsg.msgId << ", addr = " << pendingMsg.addr << ", CacheLine = " << addrMap.idx_set << ", Tag = " << addrMap.tag << ", RefTraceCycle = " << pendingMsg.cpuReqCycle << std::endl;
                m_TraceFileId << "\t|----- Msg Id " << busReqMsg.msgId << " GetM/S Inserted in PrivCache TxReq FIFO @ Cycle " << busReqMsg.cycle << std::endl;
              }

              pendingMsg.startProcessingCycle    = busReqMsg.cycle;
              pendingMsg.busTxFIFOInsertionCycle = busReqMsg.cycle;
              pendingMsg.absArivalCycle          = m_CycleCnt;
              
              if (m_latTraceEnable) {
                m_TraceFileId << "\t|----- Msg Id " << busReqMsg.msgId << " absArivalCycle " << pendingMsg.absArivalCycle << std::endl;
              }
            }
            // Dequeue the data again into latency buffer
            m_latencyLogFIFO.InsertElement(pendingMsg);
          }
        }
      }
    }

    void LatencyLogger::ChkReqBusLatency () {
      if (!m_interConnectFIFO->m_ReqMsgFIFO.IsEmpty()) {
        BusIfFIFO::BusReqMsg busReqMsg = m_interConnectFIFO->m_ReqMsgFIFO.GetFrontElement();
        if (busReqMsg.msgId != m_currBusReqId && busReqMsg.reqCoreId == m_coreId && busReqMsg.cohrMsgId != SNOOPPrivCohTrans::PutMTrans) {
          m_currBusReqId = busReqMsg.msgId;
          int pendingQueueSize = m_latencyLogFIFO.GetQueueSize();
          LatencyComponent pendingMsg ;
          int OOOCompansateFactor;
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingMsg = m_latencyLogFIFO.GetFrontElement();
            // Remove message from the busReq buffer
            m_latencyLogFIFO.PopElement();

            if (pendingMsg.msgId == busReqMsg.msgId) {
              OOOCompansateFactor = (m_prevFinishCycle - pendingMsg.absArivalCycle)/m_overSamplingRatio;
              if (OOOCompansateFactor < 0) OOOCompansateFactor = 0;

              if (pendingMsg.busTxFIFOInsertionCycle == 0) {
                pendingMsg.busTxFIFOInsertionCycle = busReqMsg.cycle;
                OOOCompansateFactor = 0;
              }
              pendingMsg.reqBusInsertionCycle    = busReqMsg.cycle;
              if ((pendingMsg.busTxFIFOInsertionCycle * m_overSamplingRatio) >= (unsigned)m_prevFinishCycle) {
                pendingMsg.arbitLatency            = pendingMsg.reqBusInsertionCycle - pendingMsg.busTxFIFOInsertionCycle;
              }
              else {
                pendingMsg.arbitLatency            =  pendingMsg.reqBusInsertionCycle*m_overSamplingRatio - m_prevFinishCycle;
              }

              if (m_latTraceEnable) {
                m_TraceFileId << "\t|----- Msg Id " << busReqMsg.msgId << " GetM/S appeared on Bus @ Cycle " << busReqMsg.cycle << ", arbit Latency = " << pendingMsg.arbitLatency << std::endl;
                m_TraceFileId << "\t|----- Msg Id " << busReqMsg.msgId << " pendingMsg.absArivalCycle " << pendingMsg.absArivalCycle << " pendingMsg.reqBusInsertionCycle " << pendingMsg.reqBusInsertionCycle << " pendingMsg.busTxFIFOInsertionCycle " << pendingMsg.busTxFIFOInsertionCycle << " OOOCompansateFactor " << OOOCompansateFactor << " m_prevFinishCycle " << m_prevFinishCycle << std::endl;
                PrintReqMsg (busReqMsg, 0);
              }
            }
            // Dequeue the data again into latency buffer
            m_latencyLogFIFO.InsertElement(pendingMsg);
          }
        }
        else if (busReqMsg.msgId != m_currBusReqId) {
          m_currBusReqId = busReqMsg.msgId;
          //PrintReqMsg (busReqMsg,1);

        }
      }
    }

    void LatencyLogger::PrintReqMsg (BusIfFIFO::BusReqMsg busReqMsg, int format = 0) {
        GenericCacheMapFrmt busAddrMap   = AddrMap(busReqMsg.addr);
        std::string printFormat = (format == 0) ? "\t|--- Req --- Cpu " : "\t|*** Req *** Cpu ";
          switch (busReqMsg.cohrMsgId) {
            case SNOOPPrivCohTrans::GetSTrans:
              m_TraceFileId << printFormat << busReqMsg.reqCoreId << ", ReqId = " << busReqMsg.msgId << " GetSTrans to CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busReqMsg.cycle << std::endl;
              break;
            case SNOOPPrivCohTrans::GetMTrans:
              m_TraceFileId << printFormat << busReqMsg.reqCoreId << ", ReqId = " << busReqMsg.msgId << " GetMTrans to CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busReqMsg.cycle << std::endl;
              break;
            case SNOOPPrivCohTrans::UpgTrans :
              m_TraceFileId << printFormat << busReqMsg.reqCoreId << ", ReqId = " << busReqMsg.msgId << " UpgTrans to CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busReqMsg.cycle << std::endl;
              break;
            case SNOOPPrivCohTrans::PutMTrans:
              m_TraceFileId << printFormat << busReqMsg.wbCoreId << ", ReqId = " << busReqMsg.msgId << " PutMTrans to CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busReqMsg.cycle << std::endl;
              break;
            default: // Invalid Transaction
              m_TraceFileId << printFormat << busReqMsg.wbCoreId << ", ReqId = " << busReqMsg.msgId << " PutSTrans to CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busReqMsg.cycle << std::endl;
           }
    }

    void LatencyLogger::ChkRespBusLatency () {
      if (!m_interConnectFIFO->m_RespMsgFIFO.IsEmpty()) {
        BusIfFIFO::BusRespMsg busRespMsg = m_interConnectFIFO->m_RespMsgFIFO.GetFrontElement();
        GenericCacheMapFrmt busAddrMap   = AddrMap(busRespMsg.addr);
        if (busRespMsg.cycle != m_currBusRespCycle) {
          m_currBusRespCycle = busRespMsg.cycle;
          int pendingQueueSize = m_latencyLogFIFO.GetQueueSize();
          LatencyComponent pendingMsg ;

          uint16_t respLantency, replcLatency;

          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingMsg   = m_latencyLogFIFO.GetFrontElement();
            // Remove message from the busReq buffer
            m_latencyLogFIFO.PopElement();
            if (pendingMsg.reqBusInsertionCycle != 0) {
              replcLatency = 0;
              respLantency = busRespMsg.cycle - pendingMsg.reqBusInsertionCycle;

              if (busRespMsg.msgId == 0) {
                replcLatency = respLantency - pendingMsg.respBusAccumlatedLatency;
              }

              pendingMsg.replacementLatency = pendingMsg.replacementLatency + replcLatency;
              pendingMsg.respBusAccumlatedLatency = respLantency;

            }

            if (m_latTraceEnable) {
              if (pendingMsg.msgId == busRespMsg.msgId && busRespMsg.respCoreId == m_sharedMemId) {
                m_TraceFileId << "\t|--- Res --- Cpu " << busRespMsg.respCoreId << " OwnDataResp,  ReqId  " << busRespMsg.msgId << " CacheLine = " << busAddrMap.idx_set << " appeared on Bus @ Cycle " << busRespMsg.cycle << ", AccumRespLatency = " <<  pendingMsg.respBusAccumlatedLatency << std::endl;
              }
              else if (busRespMsg.msgId == 0) {
                m_TraceFileId << "\t|--- Res --- Cpu " << busRespMsg.respCoreId << " ReplDataResp, ReqId " << busRespMsg.msgId  << " CacheLine = " << busAddrMap.idx_set<< " appeared on Bus @ Cycle " << busRespMsg.cycle <<  ", AccumReplcLatency = " <<  pendingMsg.replacementLatency << std::endl;
              }
              else if (busRespMsg.respCoreId == m_sharedMemId) {
                m_TraceFileId << "\t|--- Res --- SharedMem Data Tansfer (ReqId = " << busRespMsg.msgId << " ) to Cpu " << busRespMsg.reqCoreId << " CacheLine = " << busAddrMap.idx_set << " appeared on bus @ Cycle " << busRespMsg.cycle << ", AccumRespLatency = " <<  pendingMsg.respBusAccumlatedLatency << std::endl;
              }
              else {
                m_TraceFileId << "\t|--- Res --- Cpu " << busRespMsg.respCoreId << " WriteBack, ReqId " << busRespMsg.msgId << " CacheLine = " << busAddrMap.idx_set<< " appeared on Bus @ Cycle " << busRespMsg.cycle << ", AccumRespLatency = " <<  pendingMsg.respBusAccumlatedLatency << std::endl;
              }
            }

            // Dequeue the data again into latency buffer
            m_latencyLogFIFO.InsertElement(pendingMsg);
          }
        }
      }
    }

    void LatencyLogger::CycleProcess () {
      //cout<<"in Latency Logger CycleProcess"<<endl;
      // Push new request into LatencyLogger FIFO
      PushNewRequestorLatency ();

      // update latency number of requests
      ChkPrivCacheReqBusEvent ();

      ChkReqBusLatency ();

      ChkRespBusLatency ();

      // check for request removal and 
      // dump latency information in the trace file
      CoreHitEventChk ();

      Simulator::Schedule(NanoSeconds(m_dt), &LatencyLogger::Step, Ptr<LatencyLogger > (this));
      m_CycleCnt++;
    }

    /**
     * Runs one mobility Step for the given vehicle generator.
     * This function is called each interval dt
     */

    void LatencyLogger::Step(Ptr<LatencyLogger> latencyLogger) {
        latencyLogger->CycleProcess();
    }
}
