/*
 * File  :      MCoreSimProject.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 15, 2020
 */

#ifndef _MCoreSimProject_H
#define _MCoreSimProject_H

#include "ns3/core-module.h"
#include "ns3/ptr.h"
#include "MCoreSimProjectXml.h"
#include "CpuCoreGenerator.h"
#include "PrivateCacheCtrl.h"
#include "SharedCacheCtrl.h"
#include "BusArbiter.h"
#include "DRAMCtrl.h"
#include "LatencyLogger.h"
#include <string>

using namespace std;
using namespace ns3;

class MCoreSimProject {
private:
    // Shared Bus Max Clk Frequency 
    int m_busClkMHz;

    // The Simulation Time Step
    double m_dt;

    // Simulation Time in seconds
    double m_totalTimeInSeconds;

    // Run 2 End Flag
    bool m_runTillSimEnd;

    // Enable Log File dump
    bool m_logFileGenEnable;

    // bus clk count
    uint64_t m_busCycle;
    
    // coherence protocol type
    CohProtType m_cohrProt;
    
    int m_maxPendReq;
    
    // A list of Cpu Core generators
    std::list<ns3::Ptr<ns3::CpuCoreGenerator> > m_cpuCoreGens;
    
    // A list of Cpu buffers
    std::list<ns3::Ptr<ns3::CpuFIFO> > m_cpuFIFO;

    // A list of Cache Ctrl engines
    std::list<ns3::Ptr<ns3::PrivateCacheCtrl> > m_cpuCacheCtrl;

    // A list of Cache Ctrl Bus interface buffers
    std::list<ns3::Ptr<ns3::BusIfFIFO> > m_busIfFIFO;

    // A pointer to shared cache controller engine
    std::list<Ptr<ns3::SharedCacheCtrl> > m_SharedCacheCtrl;                // Modified to Accomodate Multi Shared Cache

    // A pointer to shared cache Bus IF buffers
    //ns3::Ptr<ns3::BusIfFIFO> m_sharedCacheBusIfFIFO;
    std::list<ns3::Ptr<ns3::BusIfFIFO> > m_sharedCacheBusIfFIFO;

    // A pointer to shared cache Bus IF buffers                         // Modified to Accomodate Multi Shared Cache
    //std::list<ns3::Ptr<ns3::DRAMIfFIFO> > m_sharedCacheDRAMBusIfFIFO;     // Modified to Accomodate Multi Shared Cache
    ns3::Ptr<ns3::DRAMIfFIFO> m_sharedCacheDRAMBusIfFIFO;                   // Modified to Accomodate Multi Shared Cache
    
    // A pointer to DRAM Controller engine
    ns3::Ptr<ns3::DRAMCtrl> m_dramCtrl;

    // Interconnect FIFOs:
    ns3::Ptr<ns3::InterConnectFIFO> m_interConnectFIFO;

    // global queue FIFO                                                    // Modified to Accomodate Multi Shared Cache
    ns3::Ptr<ns3::GlobalQueue> m_GlobalQueue;                               // Modified to Accomodate Multi Shared Cache

    // A pointer to Bus Arbiter                                                 
    ns3::Ptr<ns3::BusArbiter> m_busArbiter;

    // A pointer to Latency Logger component
    std::list<ns3::Ptr<ns3::LatencyLogger> > m_latencyLogger;

    // The Xml documents for simulation configuration
    MCoreSimProjectXml m_projectXmlCfg;
    
    void GetCohrProtocolType ();
    
    ReplcPolicy ReplcPolicyDecode (std::string replcPolicy);

     // cycle process 
     void CycleProcess  ();
     
     // enable debug flag 
     void EnableDebugFlag(bool Enable);
public:
    // Constructor
    MCoreSimProject(MCoreSimProjectXml projectXmlCfg);
    uint32_t nsetshared;    
    // Starts MCore simulation
    void Start ();

    // Calls the next step on the simulation
    static void Step (MCoreSimProject* project);

};

#endif /* _MCoreSimProject_H */
