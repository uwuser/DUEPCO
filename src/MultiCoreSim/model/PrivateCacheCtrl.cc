/*
 * File  :      MCoreSimProjectXml.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 17, 2020
 */

#include "PrivateCacheCtrl.h"
using namespace std;

namespace ns3 {

    // override ns3 type
    TypeId PrivateCacheCtrl::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PrivateCacheCtrl")
               .SetParent<Object > ();
        return tid;
    }

    // private controller constructor
    PrivateCacheCtrl::PrivateCacheCtrl(uint32_t cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO, 
                     Ptr<CpuFIFO  > associatedCpuFIFO,
                     ns3::Ptr<ns3::GlobalQueue>  globalqueue) {
        // default
        m_cacheType        = 0;
        m_cacheSize        = 64*256;
        m_cacheBlkSize     = 64;
        m_nways            = 1;
        m_duetto           = false;
        m_nsets            = 256;
        m_coreId           = 1;
        m_sharedMemId      = 10;
        m_cacheCycle       = 1;
        m_dt               = (1.0/1000000);
        m_clkSkew          = 0;
        m_prllActionCnt    = 0;
        m_reqWbRatio       = 0;
        m_cach2Cache       = false;
        m_logFileGenEnable = false;
        m_pType            = CohProtType::SNOOP_MSI;
        m_maxPendingReq    = 1;
        m_pendingCpuReq    = 0;
        m_cache            = CreateObject<GenericCache> (cachLines);
        m_GlobalQueue      = globalqueue;
        m_busIfFIFO        = assoicateBusIfFIFO;
        m_cpuFIFO          = associatedCpuFIFO;
        m_cohProtocol      = CreateObject<SNOOPPrivCohProtocol>();
        m_cpuPendingFIFO   = CreateObject<GenericFIFO <PendingMsg >> ();
        m_PendingWbFIFO.SetFifoDepth(assoicateBusIfFIFO->m_txMsgFIFO.GetFifoDepth());
        m_mode             = "RT";
        //m_mode             = "RT";
        m_reza_log_private = false; 
    }

    // We don't do any dynamic allocations
    PrivateCacheCtrl::~PrivateCacheCtrl() {
    }

    // Set Maximum number of Pending CPU Request (OOO)
    void PrivateCacheCtrl::SetMaxPendingReq (int maxPendingReq) {
      m_maxPendingReq = maxPendingReq;
    }

    // Set Pending CPU FIFI depth
    void PrivateCacheCtrl::SetPendingCpuFIFODepth (int size) {
      m_cpuPendingFIFO->SetFifoDepth(size);
    }

    void PrivateCacheCtrl::SetCacheSize (uint32_t cacheSize) {
      m_cacheSize = cacheSize;
      m_cache->SetCacheSize(cacheSize);
    }

    uint32_t PrivateCacheCtrl::GetCacheSize () {
      return m_cacheSize;
    }

    unsigned PrivateCacheCtrl::createMask(unsigned a, unsigned b)
    {
      unsigned r = 0;
      for (unsigned i=a; i<=b; i++)
          r |= 1 << i;
      return r;    
    }

    void PrivateCacheCtrl::SetCacheBlkSize (uint32_t cacheBlkSize) {
      m_cacheBlkSize = cacheBlkSize;
      m_cache->SetCacheBlkSize(cacheBlkSize);
    }

    uint32_t PrivateCacheCtrl::GetCacheBlkSize () {
      return m_cacheBlkSize;
    }

    void PrivateCacheCtrl::SetCacheNways (uint32_t nways) {
      m_nways = nways;
      m_cache->SetCacheNways(nways);
    }

    uint32_t PrivateCacheCtrl::GetCacheNways () {
      return m_nways;
    }

    void PrivateCacheCtrl::SetCacheNsets (uint32_t nsets) {
      m_nsets = nsets;
      m_cache->SetCacheNsets(nsets);
    }

    uint32_t PrivateCacheCtrl::GetCacheNsets () {
      return m_nsets;
    }

    void PrivateCacheCtrl::SetCacheType (uint16_t cacheType) {
      m_cacheType = cacheType;
      m_cache->SetCacheType(cacheType);
    }

    uint16_t PrivateCacheCtrl::GetCacheType () {
      return m_cacheType;
    }

    void PrivateCacheCtrl::SetCoreId (int coreId) {
      m_coreId = coreId;
    }

    void PrivateCacheCtrl::SetSharedMemId (int sharedMemId) {
      m_sharedMemId = sharedMemId;
    }

    void PrivateCacheCtrl::SetIsDuetto (bool duetto) {
      m_duetto = duetto;
    }

    int PrivateCacheCtrl::GetCoreId () {
      return m_coreId;
    }

    void PrivateCacheCtrl::SetDt (double dt) {
      m_dt = dt;
    }
    
    void PrivateCacheCtrl::assignDeadlineAfterDetermination(ns3::BusIfFIFO::BusReqMsg &msg) {
      //cout<<"In the assignDeadlineAfterDetermination"<<endl;
      //abort();
      unsigned int WCL_0;
      unsigned int WCL_1;
      unsigned int WCL_2;
      WCL_0 = 287;
      WCL_1 = 287;
      WCL_2 = 287; 
      if(msg.orderDetermined) {
        //cout<<"determined become "<<msg.becameOldest<<endl;
        //cout<<"before deadline assigned "<<msg.associateDeadline<<endl;
        switch (msg.orderofArbitration) {
          case 0: 
            msg.associateDeadline = WCL_0 - (m_cacheCycle - msg.becameOldest);          
          break;
          case 1: 
           msg.associateDeadline = WCL_1 - (m_cacheCycle - msg.becameOldest);
           break;
          case 2: 
            msg.associateDeadline = WCL_2 - (m_cacheCycle - msg.becameOldest);
            break;
        }
        //cout<<"after deadline assigned "<<msg.associateDeadline<<endl;
        msg.associateDeadline_final = true;
      }      
    }

    int PrivateCacheCtrl::GetDt () {
      return m_dt;
    }

    void PrivateCacheCtrl::SetClkSkew (double clkSkew) {
       m_clkSkew = clkSkew;
    }

    void PrivateCacheCtrl::SetReqWbRatio (int reqWbRatio) {
      m_reqWbRatio = reqWbRatio;
    }

    unsigned int PrivateCacheCtrl::retrieveCacheFIFOID(uint64_t id) {
      if(id == 0)      return 10;
      else if(id == 1) return 11;
      else if(id == 2) return 12;
      else if(id == 3) return 13;
      else if(id == 4) return 14;
      else if(id == 5) return 15;
      else if(id == 6) return 16;
      else if(id == 7) return 17;
      cout<<"retrieveCacheFIFOID cannot find  "<<id<<" as Shared Cache FIFO ID"<<endl;
      abort();
      return 0;
    }

    void associateDeadlinef (ns3::BusIfFIFO::BusReqMsg msg){
      if(msg.msgId == 0 ){ 
        //assign the correct one from here
        //cout<<"determine the tightest WC in PrivateCacheCtrl"<<endl;
        // abort();
        unsigned smallestWC = 200;
        msg.associateDeadline = smallestWC;
        msg.associateDeadline_final = false;
      }
      else{
        unsigned smallestWC = 200;
        msg.associateDeadline = smallestWC;
        //cout<<"determine the tightest WC in PrivateCacheCtrl"<<endl;
        msg.associateDeadline_final = false;
        //abort();
      }      
    }

    void PrivateCacheCtrl::SetCache2Cache (bool cach2Cache) {
       m_cach2Cache = cach2Cache;
    }

    // Set Coherence Protocol Type
    void PrivateCacheCtrl::SetProtocolType (CohProtType ptype) {
      m_pType = ptype;
    }

    void PrivateCacheCtrl::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }

    // insert new Transaction into BusTxMsg FIFO 
    bool PrivateCacheCtrl::PushMsgInBusTxFIFO (uint64_t       msgId, 
                                               uint16_t       reqCoreId, 
                                               uint16_t       wbCoreId, 
                                               uint16_t       transId, 
                                               uint64_t       addr,
                                               uint64_t       sharedcacheid,             // Modified to Accomodate Multi Shared Cache
                                               bool PendingWbBuf = false,                                              
                                               bool NoGetMResp = false)  {         
       if(m_reza_log_private)  cout<<" PushMsgInBusTxFIFO  "<<msgId<<endl;
       BusIfFIFO::BusReqMsg tempBusReqMsg;
       tempBusReqMsg.msgId        = msgId;
       tempBusReqMsg.reqCoreId    = reqCoreId;
       tempBusReqMsg.wbCoreId     = wbCoreId;
       tempBusReqMsg.cohrMsgId        = transId;
       tempBusReqMsg.sharedCacheAgent = sharedcacheid;                      // Modified to Accomodate Multi Shared Cache
       tempBusReqMsg.addr         = addr;
       tempBusReqMsg.timestamp    = m_cacheCycle*m_dt;
       tempBusReqMsg.cycle        = m_cacheCycle;
       tempBusReqMsg.NoGetMResp   = NoGetMResp;
       bool m_replacement         = false;         
      /**************** VERY IMPORTANT:: IF THE MSGid IS ZERO, IT MEANS THAT THIS TRANSACTION IS A REPLACEMENT ********************/
       
       if (!m_busIfFIFO->m_txMsgFIFO.IsFull() && PendingWbBuf == false) {
         if(!m_duetto && m_mode == "FCFS") {
            m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
            return true;
         }
         else if((!m_duetto && m_mode == "RT") || m_duetto == true){
           if(m_reza_log_private) cout<<" PushMsgInBusTxFIFO  1   and the timestamp  &&&&&& "<<tempBusReqMsg.timestamp<<
              " buffer IFFIFO size "<<m_busIfFIFO->m_txMsgFIFO.GetQueueSize()<<" address "<<addr<<"   reqID  "<<reqCoreId<<endl;
           if(m_reza_log_private && m_duetto == true) cout<<"PushMsgInBusTxFIFO  1 in m_duetto "<<endl;   
         // push the requestor in the RR global queue                                                 // Modified to Accomodate Multi Shared Cache       
         if(msgId == 0) {
           m_replacement = true; 
         }
         if(m_busIfFIFO->m_txMsgFIFO.IsEmpty()) {           
          // cout<<" 3  "<<endl;
           bool check = true;
           bool con = false;       
           if(m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() == 0){           
             check = false;
           }
           for(int itr1 = 0; itr1 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && con == false; itr1++ )  {                                            // Modified to Accomodate Multi Shared Cache           
           if(m_reza_log_private) cout<<"check if is empty  "<<endl;
             BusIfFIFO::BusReqMsg tempReq;
             tempReq = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();  // Modified to Accomodate Multi Shared Cache
             m_GlobalQueue->m_GlobalOldestQueue.PopElement();  // Modified to Accomodate Multi Shared Cache
             if(!m_replacement) {
              if((tempReq.msgId != 0 && tempReq.reqCoreId == reqCoreId) || (tempReq.msgId == 0 && tempReq.wbCoreId == reqCoreId)) {  // Modified to Accomodate Multi Shared Cache
               if(m_reza_log_private) cout<<"It is empty 3"<<endl;
               check = true;   // Modified to Accomodate Multi Shared Cache
               con = true;
              }              
              else{
                 if(m_reza_log_private) cout<<"It is empty 4"<<endl;
                check = false;
              }
             }
             else {
               //cout<<" 5  "<<endl;
               check = false;
                if(m_reza_log_private) cout<<"================= Replacement ==================== reqCoreID "<<reqCoreId<<" wbCoreId "<<wbCoreId<<"  the address is "<<addr<<endl;
               if(m_reza_log_private) cout<<"This is replacement and msgID is zero  tempReq.reqCoreId "<<tempReq.reqCoreId<<" wbCoreId "<<wbCoreId<<"  the address is "<<tempReq.addr<<endl;
                if ((tempReq.reqCoreId == wbCoreId && tempReq.msgId != 0) || (tempReq.wbCoreId == wbCoreId && tempReq.msgId == 0)){                  
                  check = true;
                  con = true;
              }
             }             
              if(m_reza_log_private) cout<<"It is empty 5"<<endl;
              m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempReq);   // Modified to Accomodate Multi Shared Cache
            }   

            //cout<<" 6  "<<endl;
            if(!check) {  // Modified to Accomodate Multi Shared Cache
              //cout<<"It is empty 6"<<endl;
              if(m_reza_log_private) cout<<" PUSHEDDDDDDDDDDDDDDDDDDDDDDD PUSHEDDDDDDDDDDDDDDDDDDD toldest address "<<addr<<"  reqID  "<<reqCoreId<<" wbID  "<<wbCoreId<<" msgId "<<msgId<<endl;
              tempBusReqMsg.becameOldest = m_cacheCycle;
              associateDeadlinef(tempBusReqMsg);
              m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempBusReqMsg);       // Modified to Accomodate Multi Shared Cache              
              if(m_replacement) {
                m_GlobalQueue->m_GlobalRRQueue.push_back(wbCoreId);            // Modified to Accomodate Multi Shared Cache
                if(m_reza_log_private) cout<<"the wbcoreID "<<wbCoreId<<" added to the RR order"<<endl;
              }
              else { 
                //cout<<" 7  "<<endl;
                m_GlobalQueue->m_GlobalRRQueue.push_back(reqCoreId);            // Modified to Accomodate Multi Shared Cache  
              }
            }
          }
          else{
            //cout<<" 8  "<<endl;
            //abort();
          }
         // push message into BusTxMsg FIFO
         if(m_reza_log_private) cout<<" PUSHEDDDDDDDDDDDDDDDDDDDDDDD to the TX "<<addr<<"  reqID  "<<reqCoreId<<" wbID  "<<wbCoreId<<" msgId "<<msgId<<endl;
         m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
         return true;
         }         
        }
        else if (!m_PendingWbFIFO.IsFull() && PendingWbBuf == true) {
          if(!m_duetto && m_mode == "FCFS") { 
            m_PendingWbFIFO.InsertElement(tempBusReqMsg);
            return true;
          }
          else if ((!m_duetto && m_mode == "RT") || m_duetto == true) {
            if(m_replacement == true) {
              if(m_reza_log_private) cout<<"in pending there is a replacement "<<endl;
              abort();
            }
            if(m_reza_log_private && m_duetto == true) cout<<"PushMsgInBusTxFIFO  2 in m_duetto "<<endl;   
            if(m_reza_log_private) cout<<" PushMsgInBusTxFIFO  2   and the timestamp  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& address "<<addr<<"  reqID  "<<reqCoreId<<endl;
            bool check = true;    
            bool con = false;   
           // push the requestor in the RR global queue                                                 // Modified to Accomodate Multi Shared Cache
           if(m_busIfFIFO->m_txRespFIFO.IsEmpty()) {  
            if(m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() == 0)
              check = false;
            for(int itr1 = 0; itr1 < m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize() && con == false; itr1++ )  {                                            // Modified to Accomodate Multi Shared Cache           
              BusIfFIFO::BusReqMsg tempReq;
              tempReq = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();  // Modified to Accomodate Multi Shared Cache
              m_GlobalQueue->m_GlobalOldestQueue.PopElement();  // Modified to Accomodate Multi Shared Cache
              if(tempReq.wbCoreId == reqCoreId || tempReq.reqCoreId == reqCoreId) {  // Modified to Accomodate Multi Shared Cache
                check = true;   // Modified to Accomodate Multi Shared Cache
                con = true;
              }
              else{
                check = false;
              }
                m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempReq);   // Modified to Accomodate Multi Shared Cache
              }   
            if(!check) {  // Modified to Accomodate Multi Shared Cache
            if(m_reza_log_private) cout<<" PUSHEDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD PUSHEDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD "<<reqCoreId<<endl;
                tempBusReqMsg.becameOldest = m_cacheCycle;
                associateDeadlinef(tempBusReqMsg);
                m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempBusReqMsg);                                              // Modified to Accomodate Multi Shared Cache
                m_GlobalQueue->m_GlobalRRQueue.push_back(reqCoreId);            // Modified to Accomodate Multi Shared Cache
                
              }                                   // Modified to Accomodate Multi Shared Cache
            }
           // push message into BusTxMsg FIFO
            m_PendingWbFIFO.InsertElement(tempBusReqMsg);
            return true;
          }    
        }
       else {
         if (m_logFileGenEnable){
           std::cout << "Info: Cannot insert the Msg into BusTxMsg FIFO, FIFO is Full" << std::endl;
         }
         return false;
       }
       cout<<"In private return null"<<endl;
       abort();
       return false;
     }
     
     bool PrivateCacheCtrl::MOESI_Modify_NoGetMResp_TxFIFO (uint64_t addr) {
       int pendingQueueSize = m_busIfFIFO->m_txMsgFIFO.GetQueueSize();
       bool ModifyDone = false;
       GenericCacheMapFrmt addrMap, TxBufAddrMap;
       addrMap = m_cache->CpuAddrMap(addr);
       BusIfFIFO::BusReqMsg pendingTxMsg ;
       for (int i = 0; i < pendingQueueSize ;i++) {
         pendingTxMsg = m_busIfFIFO->m_txMsgFIFO.GetFrontElement();
         m_busIfFIFO->m_txMsgFIFO.PopElement();
         TxBufAddrMap = m_cache->CpuAddrMap(pendingTxMsg.addr);
         if (addrMap.idx_set == TxBufAddrMap.idx_set && addrMap.tag == TxBufAddrMap.tag && pendingTxMsg.cohrMsgId == SNOOPPrivCohTrans::GetMTrans) {
           pendingTxMsg.NoGetMResp = false;
           ModifyDone = true; 
         }
         m_busIfFIFO->m_txMsgFIFO.InsertElement(pendingTxMsg);
       }
       return ModifyDone;
     }

     // execute write back command
     bool PrivateCacheCtrl::DoWriteBack (uint64_t addr, uint16_t wbCoreId, uint64_t msgId, bool dualTrans=false) {
       if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {
         GenericCacheMapFrmt addrMap   = m_cache->CpuAddrMap (addr);
         GenericCacheFrmt wbLine = m_cache->ReadCacheLine (addrMap.idx_set);
         BusIfFIFO::BusRespMsg  wbMsg;
         wbMsg.reqCoreId    = wbCoreId;
         wbMsg.respCoreId   = m_coreId;
         wbMsg.addr         = addr;
         wbMsg.timestamp    = m_cacheCycle*m_dt;
         wbMsg.cycle        = m_cacheCycle;
         wbMsg.msgId        = msgId;
         wbMsg.dualTrans    = dualTrans;
         if (m_logFileGenEnable) {
           std::cout << "DoWriteBack:: coreId = " << m_coreId << " requested Core = " << wbCoreId <<"  dual: "<<dualTrans<< std::endl;
         }
         for (int i = 0; i < 8; i++)
           wbMsg.data[i] = wbLine.data[i];
         // push message into BusTxMsg FIFO
         m_busIfFIFO->m_txRespFIFO.InsertElement(wbMsg);
         if(m_reza_log_private) cout<<" TX response queue size of "<<m_coreId<<"  is "<<m_busIfFIFO->m_txRespFIFO.GetQueueSize()<<endl;
         if(m_reza_log_private) {
          cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
          cout<<" reqCoreID:  "<<wbMsg.reqCoreId<<"  respCoreID:  "<<wbMsg.respCoreId<<"  ID  "<<wbMsg.msgId<<endl;
          cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
          cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
          cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;                
         }
         return true;
       }
       else {
         if (m_logFileGenEnable) {
           std::cout << "Info: Cannot insert the Msg into BusTxResp FIFO, FIFO is Full" << std::endl;
           std::cout << "TxResp Buffer Size = "<< m_busIfFIFO->m_txRespFIFO.GetQueueSize() << std::endl;
         }
         return false;
       }
     }

     // process pending buffer
     bool PrivateCacheCtrl::SendPendingWB  (GenericCacheMapFrmt recvTrans, TransType type = MemOnly ) {
        bool ReqSentFlag = false;
        if(m_reza_log_private) cout<<"send pending WB ********************************************************"<<endl;
       // check if there is a pending write-back to this line in the pending buffer
       if (!m_PendingWbFIFO.IsEmpty()) {
          BusIfFIFO::BusReqMsg pendingWbMsg ;
          GenericCacheMapFrmt  pendingWbAddrMap;
          int pendingQueueSize = m_PendingWbFIFO.GetQueueSize();
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingWbMsg = m_PendingWbFIFO.GetFrontElement();
            pendingWbAddrMap = m_cache->CpuAddrMap (pendingWbMsg.addr);
            // Remove message from the busReq buffer
            m_PendingWbFIFO.PopElement();
            if (recvTrans.idx_set == pendingWbAddrMap.idx_set &&
                recvTrans.tag == pendingWbAddrMap.tag) {
              // b.3.2)send data to requestors
              //uint16_t reqCoreId = (type == CoreOnly || type == CorePlsMem) ? pendingWbMsg.reqCoreId : m_sharedMemId;
              uint16_t reqCoreId = (type == CoreOnly || type == CorePlsMem) ? pendingWbMsg.reqCoreId : pendingWbMsg.sharedCacheAgent;
              bool dualTrans = (type == CorePlsMem);
              if(m_reza_log_private) cout<<"111send pending WB ********************************************************"<<endl;
              if (!DoWriteBack (pendingWbMsg.addr, reqCoreId, pendingWbMsg.msgId, dualTrans)) {
                std::cout << "PrivCache " << m_coreId << " TxResp FIFO is Full!" << std::endl;
                exit(0);
              }
              ReqSentFlag = true;
            }
            else {
              // b.3.1) Dequeue the data again into pending buffer
              m_PendingWbFIFO.InsertElement(pendingWbMsg);
            }
          }
        } 
        return ReqSentFlag;
     }

     bool PrivateCacheCtrl::PendingCoreBufRemoveMsg  (uint64_t msgId, PendingMsg &removedMsg) {
       bool removalFlg = false;
       int pendingBufSize = m_cpuPendingFIFO->GetQueueSize();
       PendingMsg cpuPendingMsg;
       for (int i = 0; i < pendingBufSize ;i++) {
         cpuPendingMsg = m_cpuPendingFIFO->GetFrontElement();
         m_cpuPendingFIFO->PopElement();
         if (m_logFileGenEnable) {
           std::cout << "PendingCpuBuffer: Entery = " << i << ", core Id = " << m_coreId << " cpuMsg.msgId = " << cpuPendingMsg.cpuMsg.msgId << " request.msgId = " << msgId << ", addr = " << cpuPendingMsg.cpuMsg.addr << " IsProcessed = " << cpuPendingMsg.IsProcessed << std::endl;
         }
         // Remove message that has same Id from the pending buffer
         if (cpuPendingMsg.cpuMsg.msgId == msgId) {
           removalFlg = true;
           removedMsg = cpuPendingMsg;
           if (m_logFileGenEnable) {
             std::cout << "PendingCpuBuffer: Core Id = " << m_coreId << " New message get removed from the Pending Buffer, Pending Cnt " << m_pendingCpuReq << std::endl;
           }
         }
         else {
           m_cpuPendingFIFO->InsertElement(cpuPendingMsg); // dequeue
         }
       }
       return removalFlg;
     }

     // This function does most of the functionality.
     void PrivateCacheCtrl::CacheCtrlMain () {
      if(m_reza_log_private) cout<<" in CacheCtrlMain ---------------------------------- for  "<<m_coreId<<endl;
       PendingMsg cpuPendingMsg; // Message pushed into CPU's pending buffer

       // Check Coherence Protocol Events (i.e. Core, Request-Bus, and Response-Bus)
       m_cohProtocol->ChkCohEvents ();
       if(m_reza_log_private) cout<<" in CacheCtrlMain after  ChkCohEvents  m_pendingCpuReq "<<m_pendingCpuReq<<"  m_maxpending "<<m_maxPendingReq<<"  the core id is  "<<m_coreId<<endl;
       
       // Insert Core's requests into pending buffer       
       if (m_cohProtocol->GetCpuReqEvent() != SNOOPPrivCoreEvent::Null && m_pendingCpuReq < m_maxPendingReq) {
             if(m_reza_log_private) cout<<" in CacheCtrlMain 222"<<endl;
           if (!m_cpuPendingFIFO->IsFull()) {
           if(m_reza_log_private) cout<<" *******************IT SHOULD BE REMOVED FROM THE cpu fifo TX"<<endl;
           // remove request from CpuReq FIFO
           m_cpuFIFO->m_txFIFO.PopElement();
           // insert message into pending-queue
           cpuPendingMsg.IsProcessed = false;
           cpuPendingMsg.IsPending   = false;
           cpuPendingMsg.cpuMsg      = m_cohProtocol->GetCpuReqMsg ();
           cpuPendingMsg.cpuReqEvent = m_cohProtocol->GetCpuReqEvent();
           m_cpuPendingFIFO->InsertElement(cpuPendingMsg);
           m_pendingCpuReq++;
           if (m_logFileGenEnable) {
             std::cout << "Core Id = " << m_coreId << " New CPU Request get inserted into pending buffer, number of pending requests  = " << m_pendingCpuReq << std::endl;
           }
         }
       }
      //  cout<<" fetch a new request from pending buffer to process it"<<endl;
       // fetch a new request from pending buffer to process it
       m_cohProtocol->SetCpuReqEvent(SNOOPPrivCoreEvent::Null);
     //cout<<" FETCH 1"<<endl;
       int pendingBufSize = m_cpuPendingFIFO->GetQueueSize();
       for (int i = 0; i < pendingBufSize ;i++) {
        // cout<<" FETCH 2"<<endl;
         cpuPendingMsg = m_cpuPendingFIFO->GetFrontElement();

         if (cpuPendingMsg.IsProcessed == false) {
          // cout<<" FETCH 3"<<endl;
           m_cohProtocol->SetCpuReqMsg(cpuPendingMsg.cpuMsg);
           m_cohProtocol->SetCpuReqEvent(cpuPendingMsg.cpuReqEvent);
           break;
         }
         //cout<<" FETCH 4"<<endl;
         // dequeuing message
         m_cpuPendingFIFO->PopElement();
         m_cpuPendingFIFO->InsertElement(cpuPendingMsg);
       }

      //cout<<" CacheCtrlMain 3"<<endl;
       // Get Cache line information for all event
       m_cohProtocol->GetEventsCacheInfo ();


      // cout<<" CacheCtrlMain 4"<<endl;
       // Check Which Event to be executed first
       SNOOPPrivEventType currProcEvent;
       m_cohProtocol->CohEventsSerialize ();
       currProcEvent = m_cohProtocol->GetCurrProcEvent ();

       //cout<<" CacheCtrlMain 5"<<endl;
       // Run Coherence Protocol FSM for the current Event
       if (currProcEvent == SNOOPPrivEventType::Core) {
         //cout<<"Process core event"<<endl;
         m_cohProtocol->ProcessCoreEvent ();
       }
       else if (currProcEvent == SNOOPPrivEventType::ReqBus || 
                currProcEvent == SNOOPPrivEventType::RespBus) {
                  //cout<<"m_cohProtocol->ProcessBusEvents ();"<<endl;
         m_cohProtocol->ProcessBusEvents ();
       }



      // cout<<" CacheCtrlMain 6"<<endl;
       // Get current processed event action
       SNOOPPrivCtrlAction currEventCtrlAction;
       currEventCtrlAction = m_cohProtocol->GetCurrEventCtrlAction();
       //cout<<" CacheCtrlMain 7"<<endl;
       // Get current processed event transaction
       SNOOPPrivCohTrans currEventCohrTrans;
       currEventCohrTrans = m_cohProtocol->GetCurrEventCohrTrans();
       // Get next state of the current process event
       int currEventCacheNextState;
       currEventCacheNextState = m_cohProtocol->GetCurrEventCacheNextState();
       //cout<<" CacheCtrlMain 8"<<endl;
       // Get cache line information of the current process event
       GenericCache::CacheLineInfo currEventCacheLineInfo;
       currEventCacheLineInfo = m_cohProtocol->GetCurrEventCacheLineInfo();
       //cout<<" CacheCtrlMain 9"<<endl;
       // Get received message of the current process event
       CpuFIFO  ::ReqMsg      cpuReqMsg = {};
       BusIfFIFO::BusReqMsg   busReqMsg = {};
       BusIfFIFO::BusRespMsg  busRespMsg= {};


      // cout<<" CacheCtrlMain 10"<<endl;
       cpuReqMsg    = m_cohProtocol->GetCpuReqMsg();
       busReqMsg    = m_cohProtocol->GetBusReqMsg();
       busRespMsg   = m_cohProtocol->GetBusRespMsg();
       // cout<<" CacheCtrlMain 11"<<endl;
       // received cache line infomration
       GenericCacheFrmt cacheLineInfo;
       GenericCacheMapFrmt addrMap;

       // cpu RxResp buffer message
       CpuFIFO::RespMsg CpuResp;

       // cpuReq Event 
       SNOOPPrivCoreEvent cpuReqEvent;



       cpuReqEvent = m_cohProtocol->GetCpuReqEvent();
      // cout<<" CacheCtrlMain 12"<<endl;
       // Print Cache Line information
       if (m_logFileGenEnable) {
         if (!(currProcEvent == SNOOPPrivEventType::Core && currEventCtrlAction ==  SNOOPPrivCtrlAction::Stall)){
           if (currProcEvent != SNOOPPrivEventType::Null) 
             std::cout << "SNOOPPrivCohProtocol: CoreId = " << m_coreId << " Tick ================================================================== " << m_cacheCycle << std::endl;
           m_cohProtocol->PrintEventInfo ();
         }
       }
     // cout<<" CacheCtrlMain 22"<<endl;
       /************************
        * Process RespBus Event
        ***********************/
       if (currProcEvent == SNOOPPrivEventType::RespBus) {
         if(m_reza_log_private) cout<<"RespBus"<<endl;
         /* a) CopyThenHit ACK Processing
          */
         if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHit             ||
             currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitWB           ||
             currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly ||
             currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly  ||
             currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem ){
          // So far, we start by processing the RESP event and then here based on the coherency protocl the action is determined.
          // Since all of these are data hit meaning that data came from the interface to this core, first they need to be stored
          // in this private cache. 

           // 1) Remove message from the busResp buffer
           m_busIfFIFO->m_rxRespFIFO.PopElement();

           // 2) Update Private Cache Line
           addrMap = m_cache->CpuAddrMap(busRespMsg.addr);
           cacheLineInfo.state  = currEventCacheNextState;
           cacheLineInfo.valid  = m_cohProtocol->IsCacheBlkValid (currEventCacheNextState);
           cacheLineInfo.tag    = addrMap.tag;
           for (int i = 0; i < 8; i++)
             cacheLineInfo.data[i] = busRespMsg.data[i];

           m_cohProtocol->UpdateCacheLine(Line, 
                                          cacheLineInfo, 
                                          currEventCacheLineInfo.set_idx, 
                                          currEventCacheLineInfo.way_idx);

           // 3) Remove message from core's pending-buffer
           // Now, we need to check the pending core buffer for the request that asked for this data. This requet is finihsed
           // at this point so it should be removed from the buffer 
           if (PendingCoreBufRemoveMsg(busRespMsg.msgId, cpuPendingMsg) == true) {
             if(m_reza_log_private) cout<<"inja"<<endl;
             CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
             CpuResp.cycle    = m_cacheCycle;
             CpuResp.msgId    = cpuPendingMsg.cpuMsg.msgId;
             CpuResp.addr     = cpuPendingMsg.cpuMsg.addr;
             m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
             m_pendingCpuReq--;
             if(m_reza_log_private) cout<<"m_pendingCpuReq  "<<m_pendingCpuReq<<endl;
           }
           else {
             std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
             exit(0);
           }

           // Check if there is any assoicated writeback
           // need to be generated on the bus (i.e Pending write back)
           // Now, we should check if this data that is stored to the private cache, requires some other data (with the same tag, line) to be
           // written back to the memory (write back).  
           //cout<<"||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"<<endl;    
           if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitWB ||
               currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly ||
               currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly  ||
               currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem ) {
               // the type of the write back response is Data  therefore resp. CorePlusMem is for dual trans. The requCoreID of
               // all of these types is this core and the respCoreID could be shared memory or core depending on the type where 
               // the write back is going to (mem or core)    
               // CopyThenHitSendCoreOnly which means that send core only and the requested core: this only happens when you have
               // core2core capabilty. Notice the pendingWbMsg.reqCoreId could be "any" core that is waiting for this data
             TransType type = (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly) ? CoreOnly :
                              (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly ) ? MemOnly  :
                              (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem ) ? CorePlsMem : CorePlsMem ;

              if(currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreOnly ) cout<<"CORE ONLY"<<endl;
              else if(currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly) cout<<"Mem Only"<<endl;
              else if(currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendCoreMem) cout<<"Core Plus Mem"<<endl;
              else cout<<"nothing"<<endl;

      
             if (!SendPendingWB (addrMap,type)) {
               if (currEventCtrlAction == SNOOPPrivCtrlAction::CopyThenHitSendMemOnly) {
                 // Do write back to memory
                 //f (!DoWriteBack (busRespMsg.addr, m_sharedMemId, busRespMsg.msgId, false)) {
                 if (!DoWriteBack (busRespMsg.addr, busRespMsg.sharedCacheAgent, busRespMsg.msgId, false)) {
                   cout<<"11############################33  "<<busRespMsg.sharedCacheAgent<<"   previous  "<<m_sharedMemId<<endl;
                   //if (!DoWriteBack (busRespMsg.addr, busRespMsg.sharedCacheAgent, busRespMsg.msgId, false)) {
                   std::cout << "This is will cause stall in the PMSI state machine !!!!" << std::endl;
                   exit(0);
                 }
               }
               else {
                 std::cout << "PrivCache: No data send to core !!!!" << std::endl;
                 exit(0);
               }
             }
             
           }
         }
         /*
          * b) No Action Processing
          */
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck){
           // b.1) Remove "other" DataResp Msgs
           m_busIfFIFO->m_rxRespFIFO.PopElement();
         }
         /*
          * c) Fault Action Processing
          */
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault) {
           if (m_logFileGenEnable) {
             std::cout << "DataResp occur in illegal state!" << std::endl;
           }
           exit(0);
         }
         /*
          * c) Check For Uncovered scenario
          */
         else if (currEventCtrlAction != SNOOPPrivCtrlAction::NullAck){
           if (m_logFileGenEnable) {
             std::cout << "DataResp uncovered Events occured!" << std::endl;
           }
           exit(0);
         }
         // mark busResp Action as processed
         currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
      }
     // cout<<" CacheCtrlMain 13"<<endl;






       /*
        * Process CpuReq action
        */
       if (currProcEvent == SNOOPPrivEventType::Core) {
         if(m_reza_log_private) cout<<" Process CpuReq action  1"<<endl;
         
         if (currEventCtrlAction == SNOOPPrivCtrlAction::Hit) {
           // Remove message from core's pending-buffer
           // If it is a hit request to this private cache then it should be removed from the pending buffer of this core and then
           // send it to the core itself (core RX interface FIFO)
           if(m_reza_log_private) cout<<" Process CpuReq action  2"<<endl;
           if (PendingCoreBufRemoveMsg(cpuReqMsg.msgId, cpuPendingMsg) == true) {
            //cout<<" Process CpuReq action  3"<<endl;
             m_pendingCpuReq--;
             CpuResp.reqcycle = cpuReqMsg.cycle;
             CpuResp.addr     = cpuReqMsg.addr;
             CpuResp.cycle    = m_cacheCycle;
             CpuResp.msgId    = cpuReqMsg.msgId;
             // Push a new response msg into CpuRx FIFO 
             m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
           }
           else {
             std::cout << "Core Id = " << m_coreId << " Hit Response for CPU Request not found in the Pending Buffer" << std::endl;
             exit(0);
           }
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTrans) {
           if(m_reza_log_private) cout<<" Process CpuReq action  4"<<endl;
           // Cannot remove Cpu request unless hit or data response happen
           if (cpuReqEvent != SNOOPPrivCoreEvent::Replacement) {
             if(m_reza_log_private) cout<<" Process CpuReq action  5"<<endl;
             cpuPendingMsg.IsProcessed = true;
             m_cpuPendingFIFO->UpdateFrontElement(cpuPendingMsg);
           }
           else {
             if(m_reza_log_private) cout<<"It is replacement  "<<endl;
           }
          if(m_reza_log_private) cout<<" m_coreID "<<m_coreId<<"  m_sharedMemID  "<<m_sharedMemId<<endl;

          unsigned int mask = createMask(6,8);
          unsigned bank_num = mask & cpuReqMsg.addr;
          bank_num = bank_num >> 6;
          cpuReqMsg.sharedCacheAgent = retrieveCacheFIFOID(bank_num);

          // cpuReqMsg.sharedCacheAgent = 10;

           uint16_t wbCoreId  = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? m_coreId      : cpuReqMsg.sharedCacheAgent;
           uint16_t reqCoreId = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? cpuReqMsg.sharedCacheAgent : m_coreId;
           uint64_t msgId     = (cpuReqEvent == SNOOPPrivCoreEvent::Replacement) ? 0             : cpuReqMsg.msgId;
          if(m_reza_log_private) cout<<" Process CpuReq action  reqcoreID  "<<reqCoreId<<"  wbcoreID  "<<wbCoreId<<endl;
           bool NoGetMResp = false;
           if (m_pType == CohProtType::SNOOP_MOESI && currEventCohrTrans == SNOOPPrivCohTrans::GetMTrans) {
           //cout<<" Process CpuReq action  8"<<endl;
             NoGetMResp = (static_cast<SNOOP_MOESIPrivCacheState>(m_cohProtocol->GetCurrEventCacheCurrState()) == SNOOP_MOESIPrivCacheState::O) ;
           }
            if(m_reza_log_private) cout<<" Process CpuReq action  9"<<endl;
           
           if (!PushMsgInBusTxFIFO (msgId, reqCoreId, wbCoreId, currEventCohrTrans, cpuReqMsg.addr, cpuReqMsg.sharedCacheAgent, false, NoGetMResp)) {
             //cout<<" Process CpuReq action  10"<<endl;
             std::cout << "PrivCache " << m_coreId << " TxReq FIFO is Full!" << std::endl;
             exit(0);
           }
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::Stall) {
           /* No action is required          
            */ 
           //cout<<" Process CpuReq action  14"<<endl;
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck) {
          if(m_reza_log_private) cout<<" Process CpuReq action  15"<<endl;
           /* No action is required, just 
            * update state
            */ 
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault) {
           
           // illegal Event at this state
           std::cout << "PrivCache " << m_coreId << " CoreEvent occur in illegal state!" << std::endl;
           exit(0);
         }
         // Check For Uncovered actions
         else {
           std::cout << "PrivCache " << m_coreId << " uncovered Events occured!" << std::endl;
           exit(0);
         }
         if(m_reza_log_private) cout<<" Process CpuReq action  11"<<endl;


         // Update cache line state 
         if (currEventCtrlAction != SNOOPPrivCtrlAction::Stall) {
          if(m_reza_log_private) cout<<" Process CpuReq action  12"<<endl;
           // 2) Update Private Cache Line
           addrMap = m_cache->CpuAddrMap(cpuReqMsg.addr);
           cacheLineInfo.state  = currEventCacheNextState;
           cacheLineInfo.valid  = m_cohProtocol->IsCacheBlkValid (currEventCacheNextState);
           cacheLineInfo.tag    = addrMap.tag;
           for (int i = 0; i < 8; i++)
             cacheLineInfo.data[i] = cpuReqMsg.data[i];
          //cout<<" Process CpuReq action  13"<<endl;
           m_cohProtocol->UpdateCacheLine(Line, 
                                          cacheLineInfo, 
                                          currEventCacheLineInfo.set_idx, 
                                          currEventCacheLineInfo.way_idx);
         }
         // Mark Cpu action as processed
         currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
       }

       

















       
       //cout<<" CacheCtrlMain 14"<<endl;
       /*
        * Process BusReq action
        */
       if (currProcEvent == SNOOPPrivEventType::ReqBus) {
         if(m_reza_log_private) cout<<"Process ReqBus"<<endl;
         
         if (currEventCtrlAction == SNOOPPrivCtrlAction::Hit) { 
           // Remove message from the busReq buffer
           m_busIfFIFO->m_rxMsgFIFO.PopElement();

           // Remove message from core's pending-buffer
           if (PendingCoreBufRemoveMsg(busReqMsg.msgId, cpuPendingMsg) == true) {
             CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
             CpuResp.cycle    = m_cacheCycle;
             CpuResp.msgId    = cpuPendingMsg.cpuMsg.msgId;
             CpuResp.addr     = cpuPendingMsg.cpuMsg.addr;
             m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
             m_pendingCpuReq--;
           }
           else {
             std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
             exit(0);
           }
         }
         // This condition happens in write back states
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::WritBack     ||
                  currEventCtrlAction == SNOOPPrivCtrlAction::SendMemOnly  || 
                  currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreOnly ||
                  currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem  ||
                  currEventCtrlAction == SNOOPPrivCtrlAction::HitSendMemOnly) {


           if(currEventCtrlAction == SNOOPPrivCtrlAction::WritBack) cout<<"From "<<m_coreId<<" It is WriteBack"<<endl;

           if(currEventCtrlAction == SNOOPPrivCtrlAction::SendMemOnly) {
             if(m_reza_log_private) cout<<"From "<<m_coreId<<" It is SendMemOnly"<<endl;
             busReqMsg.orderDetermined = true;
             busReqMsg.orderofArbitration = 1;
             busReqMsg.currStage = "REQ";
             assignDeadlineAfterDetermination(busReqMsg);
             m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
           }
           else if(currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreOnly) {
             if(m_reza_log_private) cout<<"From "<<m_coreId<<" It is SendCoreOnly"<<endl;
             busReqMsg.orderDetermined = true;
             busReqMsg.orderofArbitration = 2;
             busReqMsg.currStage = "REQ";
             assignDeadlineAfterDetermination(busReqMsg);
             m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
           } 
           else if(currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem) {
             if(m_reza_log_private) cout<<"From "<<m_coreId<<" It is SendCoreMem"<<endl;
             busReqMsg.orderDetermined = true;
             busReqMsg.orderofArbitration = 1;
             busReqMsg.currStage = "REQ";
             assignDeadlineAfterDetermination(busReqMsg);
             m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);
           } 

                   
           if (currEventCtrlAction == SNOOPPrivCtrlAction::HitSendMemOnly) { // do hit
             // Remove message from core's pending-buffer
             if (PendingCoreBufRemoveMsg(busReqMsg.msgId, cpuPendingMsg) == true) {
               CpuResp.reqcycle = cpuPendingMsg.cpuMsg.cycle;
               CpuResp.cycle    = m_cacheCycle;
               CpuResp.msgId    = cpuPendingMsg.cpuMsg.msgId;
               CpuResp.addr     = cpuPendingMsg.cpuMsg.addr;
               m_cpuFIFO->m_rxFIFO.InsertElement(CpuResp);
               m_pendingCpuReq--;
             }
             else {
               std::cout << "Core Id = " << m_coreId << " Response for CPU Request not found in the Pending Buffer" << std::endl;
               exit(0);
             }
           }

           // Remove message from the busReq buffer
           m_busIfFIFO->m_rxMsgFIFO.PopElement();

           // Do write back  -- m_sharedMemId
           uint16_t reqCoreId = (currEventCtrlAction ==  SNOOPPrivCtrlAction::SendCoreOnly || 
                                 currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem  ) ? busReqMsg.reqCoreId : busReqMsg.sharedCacheAgent;
           bool dualTrans = (currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem);

          //  if(currEventCtrlAction == SNOOPPrivCtrlAction::SendCoreMem) {
          //    abort();
          //  }
           if (!DoWriteBack (busReqMsg.addr, reqCoreId, busReqMsg.msgId, dualTrans)) {
             std::cout << "This is will cause stall in the PMSI state machine !!!!" << std::endl;
             exit(0);
           }
           
           if (m_pType == CohProtType::SNOOP_MOESI && m_cohProtocol->GetBusReqEvent () == SNOOPPrivReqBusEvent::OtherGetM) {
             if (static_cast<SNOOP_MOESIPrivCacheState>(m_cohProtocol->GetCurrEventCacheCurrState()) == SNOOP_MOESIPrivCacheState::OM_a) {
               if (MOESI_Modify_NoGetMResp_TxFIFO(busReqMsg.addr) == false) {
                 std::cout << "PrivateCache: MOESI Fatal Error!!!!" << std::endl;
                 exit(0);
               }
             }
           }
           
         }
         // This condition occur in PutM()
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTrans || 
                  currEventCtrlAction == SNOOPPrivCtrlAction::issueTransSaveWbId ) {
           // Remove message from the busReq buffer
           m_busIfFIFO->m_rxMsgFIFO.PopElement();
           if (currEventCohrTrans != SNOOPPrivCohTrans::PutMTrans) {
             std::cout << "Error This should be PutM() transaction, coreId =  " << m_coreId << " transaction = " <<  currEventCohrTrans << std::endl;
             exit(0);
           }
           
           //uint16_t reqCoreId = (m_cohProtocol->GetBusReqEvent () == SNOOPPrivReqBusEvent::OwnInvTrans) ? m_sharedMemId : busReqMsg.reqCoreId;
           uint16_t reqCoreId = (m_cohProtocol->GetBusReqEvent () == SNOOPPrivReqBusEvent::OwnInvTrans) ? busReqMsg.sharedCacheAgent : busReqMsg.reqCoreId;
           if (!PushMsgInBusTxFIFO (busReqMsg.msgId, reqCoreId, m_coreId, currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, false)) {
             std::cout << "This is will cause stall in the PMSI state machine !!!!" << std::endl;
             exit(0);
           }
           if (currEventCtrlAction == SNOOPPrivCtrlAction::issueTransSaveWbId) {
             if (!PushMsgInBusTxFIFO (busReqMsg.msgId, busReqMsg.reqCoreId, m_coreId,currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, true)) {
               std::cout << "PrivCache: Pending Wb buffer is full !!!!" << std::endl;
               exit(0);
             }
           }
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::SaveWbCoreId) {
           m_busIfFIFO->m_rxMsgFIFO.PopElement();
           // save pending write back coreId & address
           
            if(m_reza_log_private) cout<<"From "<<m_coreId<<" save write back coreID msgID is "<<busReqMsg.msgId<<endl;   
            busReqMsg.orderDetermined = true;
            busReqMsg.orderofArbitration = 2;
            busReqMsg.currStage = "REQ";
            
            assignDeadlineAfterDetermination(busReqMsg);
            

            if(m_reza_log_private) cout<<"associate deadline is "<<busReqMsg.associateDeadline<<endl;
            m_GlobalQueue->m_MsgType.InsertElement(busReqMsg);            

           if(m_reza_log_private) cout<<"SAVEWBCOREID %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
           if (!PushMsgInBusTxFIFO (busReqMsg.msgId, busReqMsg.reqCoreId, m_coreId,currEventCohrTrans, busReqMsg.addr, busReqMsg.sharedCacheAgent, true)) {
                std::cout << "PrivCache: Pending Wb buffer is full !!!!" << std::endl;
                exit(0);
           }
         }
         else if (currEventCtrlAction == SNOOPPrivCtrlAction::NoAck) {
           if(!m_duetto && m_mode == "FCFS") {
             // remove no-action event
             m_busIfFIFO->m_rxMsgFIFO.PopElement();
           }
           else if((!m_duetto && m_mode == "RT") || m_duetto == true) {
             if(m_reza_log_private && m_duetto == true) cout<<"noACK and replacement no ack "<<endl;   
             // remove no-action event

             /*** if the req msg is replacement and there is no Ack, it means that there is no need
              * for replacement anymore, this, you should remove the request from the global oldest
              * and global queue ***/
             BusIfFIFO::BusReqMsg m_replacement_temp;
             m_replacement_temp = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();
             m_busIfFIFO->m_rxMsgFIFO.PopElement();
             if(m_replacement_temp.msgId == 0 && m_replacement_temp.wbCoreId == m_busIfFIFO->m_fifo_id){ /*** va own bashe na other**/

               // Remove from Oldest queue 
                bool success = false;
                BusIfFIFO::BusReqMsg tempOldestReqMsg;
                int queueSize = m_GlobalQueue->m_GlobalOldestQueue.GetQueueSize();
                for(int itr = 0; itr < queueSize && success == false; itr ++) {
                  tempOldestReqMsg = m_GlobalQueue->m_GlobalOldestQueue.GetFrontElement();
                  m_GlobalQueue->m_GlobalOldestQueue.PopElement();
                  //cout<<"in the global oldest queue address  "<<tempOldestReqMsg.addr<<" wb core id "<<tempOldestReqMsg.wbCoreId<<endl;
                  if(tempOldestReqMsg.msgId == 0 && tempOldestReqMsg.addr == m_replacement_temp.addr && tempOldestReqMsg.wbCoreId == m_replacement_temp.wbCoreId) {
                    success = true;
                    // remove from the RR order list //
                    /** make sure if you dont need to adjust the oldest queue **/
                    for(unsigned int h = 0; h < m_GlobalQueue->m_GlobalRRQueue.size() ; h++) {
                      if(m_GlobalQueue->m_GlobalRRQueue.at(h) == m_replacement_temp.wbCoreId) {
                        m_GlobalQueue->m_GlobalRRQueue.erase(m_GlobalQueue->m_GlobalRRQueue.begin() + h);
                      }
                    }
                  }            
                  if(success == false)
                    m_GlobalQueue->m_GlobalOldestQueue.InsertElement(tempOldestReqMsg);        
                  }

               // remove from non oldest global queue
                success = false;
                BusIfFIFO::BusReqMsg tempNonOldestReqMsg;
                queueSize = m_GlobalQueue->m_GlobalReqFIFO.GetQueueSize();
                for(int itr = 0; itr < queueSize && success == false; itr ++) {
                  tempNonOldestReqMsg = m_GlobalQueue->m_GlobalReqFIFO.GetFrontElement();
                  m_GlobalQueue->m_GlobalReqFIFO.PopElement();
                  if(tempNonOldestReqMsg.msgId == 0 && tempNonOldestReqMsg.addr == m_replacement_temp.addr && tempNonOldestReqMsg.wbCoreId == m_replacement_temp.wbCoreId) {
                    success = true;                                                         
                  }
                  if(success == false)
                    m_GlobalQueue->m_GlobalReqFIFO.InsertElement(tempNonOldestReqMsg);
                }
              }               
            }
          }
          else if (currEventCtrlAction == SNOOPPrivCtrlAction::Fault) {
           // genenerate error message
           std::cout << "BusReqEvent occur in illegal state!" << std::endl;
           exit(0);
         }
         // Check For Uncovered scenario
         else {
           std::cout << "busReqEvent " << m_coreId << " uncovered Events occured!" << std::endl;
           exit(0);
         }

         // Update Private Cache Line
         if (currEventCacheLineInfo.IsExist == true) {
           cacheLineInfo.state  = currEventCacheNextState;
           cacheLineInfo.valid  = m_cohProtocol->IsCacheBlkValid (currEventCacheNextState);
           m_cohProtocol->UpdateCacheLine(State, 
                                          cacheLineInfo, 
                                          currEventCacheLineInfo.set_idx, 
                                          currEventCacheLineInfo.way_idx);
         }
         // mark action as processed
         currEventCtrlAction = SNOOPPrivCtrlAction::ProcessedAck;
       }
       //cout<<" CacheCtrlMain 16"<<endl;
     } // PrivateCacheCtrl::CacheCtrlMain ()

    void PrivateCacheCtrl::CycleProcess() {
      // if(m_cacheCycle > 1 ){
      //    m_reza_log_private = true;
      //  }
     // cout<<"PrivateCacheCtrl  CycleProcess  "<<endl;
       // Call cache controller
      // cout<<"||||||||||||||||||||||||||||||||||||||||||||| CALLING MAIN |||||||||||||||||||||||||||"<<endl;
       CacheCtrlMain();
      // Schedule the next run
      Simulator::Schedule(NanoSeconds(m_dt), &PrivateCacheCtrl::Step, Ptr<PrivateCacheCtrl > (this));
      m_cacheCycle++;
    }

    // The init function starts the controller at the beginning 
    void PrivateCacheCtrl::init() {
       // cout<<"in Private Cache Controller Init"<<endl;
        // Initialized Cache Coherence Protocol
        m_cohProtocol->SetProtocolType    (m_pType           );
        m_cohProtocol->SetLogFileGenEnable(m_logFileGenEnable);
        m_cohProtocol->SetCoreId          (m_coreId          );
        m_cohProtocol->SetSharedMemId     (m_sharedMemId     );
        m_cohProtocol->SetPrivCachePtr    (m_cache           );
        m_cohProtocol->SetCpuFIFOPtr      (m_cpuFIFO         );
        m_cohProtocol->SetBusFIFOPtr      (m_busIfFIFO       );
        m_cohProtocol->SetCache2Cache     (m_cach2Cache      );
        m_cohProtocol->SetReqWbRatio      (m_reqWbRatio      );
        m_cohProtocol->InitializeCacheStates();
        Simulator::Schedule(NanoSeconds(m_clkSkew), &PrivateCacheCtrl::Step, Ptr<PrivateCacheCtrl > (this));
    }

    /**
     * Runs one mobility Step for the given vehicle generator.
     * This function is called each interval dt
     */
    void PrivateCacheCtrl::Step(Ptr<PrivateCacheCtrl> privateCacheCtrl) {
        // cout<<" in PrivateCacheCtrl::Step"<<endl;
        
        privateCacheCtrl->CycleProcess();
    }
}
