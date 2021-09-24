/*
 * File  :      MemTemplate.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 16, 2020
 */

#ifndef _MemTemplate_H
#define _MemTemplate_H

using namespace std;

#include <queue>

#include <string>

namespace ns3 {
  // Generic FIFO Interface
  template <typename T>
  class GenericFIFO : public ns3::Object {
  private:
     std::queue<T> m_FIFO;
     uint16_t m_fifoDepth;
  public:
    void SetFifoDepth (int fifoDepth) {
      m_fifoDepth = fifoDepth;
    }

    int GetFifoDepth () {
      return m_fifoDepth;
    }
/* Data to be added. This is a typical %queue operation. The function creates an element at the end of the %queue and assigns the given data to it. The time complexity of the operation depends on the underlying sequence. */
    void InsertElement (T msg) {
      m_FIFO.push(msg); 
    }
/*Removes first element. This is a typical %queue operation. It shrinks the %queue by one. The time complexity of the operation depends on the underlying sequence. Note that no data is returned, and if the first element's data is needed, it should be retrieved before pop() is called.*/
    void PopElement () {
      m_FIFO.pop();
    }
  
    T GetFrontElement () {
      
      return m_FIFO.front();
    }

    void UpdateFrontElement (T msg) {
      m_FIFO.front() = msg;
    }

    void InsertElementFront (T msg) {
      m_FIFO.push_front(msg);
    }

    int GetQueueSize () {
      return m_FIFO.size();
    }

    bool IsEmpty () {
      //cout<<"the size is  "<<m_FIFO.size()<<endl;
      return m_FIFO.empty();
    }

    bool IsFull () {
      return (m_FIFO.size() == m_fifoDepth) ? true : false;
    }
  };

  // Generic FIFO Interface
  template <typename T>
  class GenericDeque : public ns3::Object {
  private:
     std::deque<T> m_FIFO;
     uint16_t m_fifoDepth;
  public:
    void SetFifoDepth (int fifoDepth) {
      m_fifoDepth = fifoDepth;
    }

    int GetFifoDepth () {
      return m_fifoDepth;
    }

    void InsertElement (T msg) {
      m_FIFO.push_back(msg);
    }

    void InsertElementFront (T msg) {
      m_FIFO.push_front(msg);
    }

    void PopElement () {
      m_FIFO.pop_front();
    }
  
    T GetFrontElement () {
      return m_FIFO.front();
    }

    void EraseElement (int index) {      
      m_FIFO.erase(m_FIFO.begin()+index);
    }

    T GetElement (int index) {
      return m_FIFO.at(index);
    }

    void UpdateFrontElement (T msg) {
      m_FIFO.front() = msg;
    }

    T GetBackElement () {
      return m_FIFO.back();
    }

    void PopBackElement () {
      m_FIFO.pop_back();
    }

    int GetQueueSize () {
      return m_FIFO.size();
    }

    bool IsEmpty () {
      return m_FIFO.empty();
    }

    bool IsFull () {
      return (m_FIFO.size() == m_fifoDepth) ? true : false;
    }
  };

  // CPU FIFO Interface
  class CpuFIFO : public ns3::Object {
  public:
    enum REQTYPE {
      READ    = 0,
      WRITE   = 1,
      REPLACE = 2
    };
    // A request  contains information on 
    // its own memory request, type, and cycle.
    struct ReqMsg {
      uint64_t msgId;
      uint16_t reqCoreId;
      uint64_t addr;
      uint64_t cycle;
      uint64_t fifoInserionCycle;
      REQTYPE  type;
      uint8_t  data[8];
      uint64_t sharedCacheAgent;
    };

    struct RespMsg {
      uint64_t msgId;
      uint64_t addr;
      uint64_t reqcycle;
      uint64_t cycle;
    };

    GenericDeque <ReqMsg > m_txFIFO;
    GenericFIFO <RespMsg> m_rxFIFO;

    //GenericDeque <ReqMsg> m_txDeque;

    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }

  };

  // Bus interface FIFO 
  class BusIfFIFO : public ns3::Object {
  public:
    // A request  contains information on 
    // its own memory request, type, and cycle.
    struct BusReqMsg {
      uint64_t addr;
      uint64_t msgId;
      uint16_t reqCoreId;
      uint16_t wbCoreId;
      uint16_t cohrMsgId;
      double   timestamp;
      uint64_t cycle;
      bool     NoGetMResp;
      uint64_t sharedCacheAgent;
      uint64_t becameOldest;
      uint64_t orderofArbitration; // 0: REQ:BANK:RESPONSE, 1: REQ:RESPONSE:BANK, 2: REQ:RESPONSE
      bool     orderDetermined; 
      uint64_t associateDeadline;
      bool associateDeadline_final;
      string currStage;  // "REQ", "BANK", "RESPONSE"
    };

    struct BusRespMsg {
      uint64_t addr;
      uint64_t msgId;
      bool     dualTrans;
      uint16_t reqCoreId;
      uint16_t respCoreId;
      double   timestamp;
      uint64_t cycle;
      uint8_t  data[8];
      uint64_t sharedCacheAgent;
    };

    

    GenericDeque <BusReqMsg  > m_txMsgFIFO; // used as a local buffer when it used in SharedMem to save pending requests 
    GenericDeque <BusRespMsg > m_txRespFIFO;
   // GenericDeque  <BusReqMsg  > m_rxMsgFIFO;                          // Modified to Accomodate Multi Shared Cache   chaged from generic fifo to dequeue
   // GenericDeque  <BusRespMsg > m_rxRespFIFO;                        // Modified to Accomodate Multi Shared Cache    chaged from generic fifo to dequeue
    GenericFIFO <BusReqMsg  > m_rxMsgFIFO;
    GenericFIFO <BusRespMsg > m_rxRespFIFO;

    GenericFIFO <BusRespMsg> m_localPendingRespTxBuffer;

    uint16_t m_bankArbBlock;
     
    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }

  };
  

  class GlobalQueue : public ns3::Object {
    public:

    struct BusMsgType {
      uint64_t addr;
      uint64_t msgId;
      uint16_t reqCoreId;
      uint16_t respCoreId;
    };
    // Global Round Robin Queue                                      // Modified to Accomodate Multi Shared Cache
    std::vector <unsigned int> m_GlobalRRQueue;                            // Modified to Accomodate Multi Shared Cache

    // Oldest Requestor Repo                                         // Modified to Accomodate Multi Shared Cache    
    GenericFIFO <BusIfFIFO::BusReqMsg> m_GlobalOldestQueue;          // Modified to Accomodate Multi Shared Cache

    // Global Queue                                               // Modified to Accomodate Multi Shared Cache    
    GenericDeque <BusIfFIFO::BusReqMsg> m_GlobalReqFIFO;         // Modified to Accomodate Multi Shared Cache

    GenericFIFO <BusIfFIFO::BusReqMsg> m_MsgType;
  
    unsigned int m_Core2CoreTransfer;
    unsigned int m_Core2CoreTransfer_0;
    unsigned int m_Core2CoreTransfer_1;
    unsigned int m_Core2CoreTransfer_2;
    unsigned int m_Core2CoreTransfer_3;
    unsigned int m_Core2CoreTransfer_4;
    unsigned int m_Core2CoreTransfer_5;
    unsigned int m_Core2CoreTransfer_6;
    unsigned int m_Core2CoreTransfer_7;              
    unsigned int m_SharedBankTransfer;
    unsigned int m_SharedBankTransfer_0;
    unsigned int m_SharedBankTransfer_1;
    unsigned int m_SharedBankTransfer_2;
    unsigned int m_SharedBankTransfer_3;
    unsigned int m_SharedBankTransfer_4;
    unsigned int m_SharedBankTransfer_5;
    unsigned int m_SharedBankTransfer_6;
    unsigned int m_SharedBankTransfer_7;
    unsigned int m_totalMsG;
    unsigned int m_totalResp;
    unsigned int m_totalReplacement;
    unsigned int m_totalL1Hit;
    unsigned int m_L1Hit_0;
    unsigned int m_L1Hit_1;
    unsigned int m_L1Hit_2;
    unsigned int m_L1Hit_3;
    unsigned int m_L1Hit_4;
    unsigned int m_L1Hit_5;
    unsigned int m_L1Hit_6;
    unsigned int m_L1Hit_7;
    uint16_t m_busArbBlock;
    uint16_t m_respArbBlock;

    string bank_mode;
    
  };

  class DRAMIfFIFO : public ns3::Object {
  public:
    enum DRAM_REQ {
      DRAM_READ    = 0,
      DRAM_WRITE   = 1
    };
    struct DRAMReqMsg {
      uint64_t msgId;
      uint64_t addr;
      DRAM_REQ type;
      uint8_t  data[8];
      uint16_t reqAgentId;
      uint64_t cycle;
      uint64_t dramFetchcycle;
    };
    
    struct DRAMRespMsg {
      uint64_t msgId;
      uint64_t addr;
      uint16_t wbAgentId;
      uint8_t  data[8];
      uint64_t cycle;
      uint64_t sharedCacheAgent;                                      // Modified to Accomodate Multi Shared Cache
    };

    GenericFIFO <DRAMReqMsg  > m_txReqFIFO;
    GenericFIFO <DRAMRespMsg > m_rxRespFIFO;

    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }
  };

  // Bus interface FIFO 
  class InterConnectFIFO : public ns3::Object {
  public:
    GenericFIFO <BusIfFIFO::BusReqMsg  > m_ReqMsgFIFO;
    GenericFIFO <BusIfFIFO::BusRespMsg > m_RespMsgFIFO;
  };

}



#endif /* _MemTemplate */

