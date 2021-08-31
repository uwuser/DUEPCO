/*
 * File  :      MSI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On May 28, 2020
 */

#ifndef _MSI_H
#define _MSI_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "SNOOPProtocolCommon.h"
#include "IFCohProtocol.h"

namespace ns3 { 

  // MSI Protocal states enconding (Private Cache Side)
  enum class SNOOP_MSIPrivCacheState {
    I = CohProtType::SNOOP_MSI,   
    IS_ad,
    IS_d,
    IS_a,
    IS_d_I,
    IM_ad,
    IM_d,
    IM_a,
    IM_d_I,
    IM_d_S,
    IM_d_SI,
    S,
    SM_ad,
    SM_d,
    SM_a,
    SM_d_I,
    SM_d_S,
    SM_d_SI,
    M,
    MI_a,
    II_a,
    I_M_d_I,
    III_a
  };

  // Protocal states enconding  (Memory Controller Side)
  enum class SNOOP_MSISharedCacheState {
    IorS = CohProtType::SNOOP_MSI, 
    M,	    
    M_d_M,	    
    M_d_IorS,
    IorSorM_a,
    DRAM_d
  };

  class MSI : public ns3::Object, public ns3::IFCohProtocol {
  public:

      MSI ();

     ~MSI ();

     // Override TypeId.
     static TypeId GetTypeId(void);

     // Protocol FSM at PrivCache Control Side
     void SNOOPPrivEventProcessing (SNOOPPrivEventType         eventType, 
                                    SNOOPPrivEventList         eventList,
                                    int                        &cacheState,     
                                    SNOOPPrivCohTrans          &trans2Issue,
                                    SNOOPPrivCtrlAction        &ctrlAction,
                                    bool                       cache2Cache);

     SNOOPPrivEventPriority PrivCacheEventPriority (SNOOPPrivEventList       eventList,
                                                    SNOOPPrivEventsCacheInfo eventCacheInfoList );



     bool IsValidBlk (int s); 

     std::string PrivStateName (int s);

     // Protocol FSM at SharedCache Control Side
     void SNOOPSharedEventProcessing (SNOOPSharedEventType      eventType, 
                                      bool                      Cache2Cache,
                                      SNOOPSharedEventList      eventList, 
                                      int                       &cacheState, 
                                      SNOOPSharedOwnerState     &ownerState,
                                      SNOOPSharedCtrlAction     &ctrlAction);


     std::string SharedStateName (int s);

  }; // class MSI

}

#endif /* _MSI_H */
