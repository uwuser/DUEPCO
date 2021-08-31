/*
 * File  :      MESI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 5, 2020
 */

#ifndef _MESI_H
#define _MESI_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "SNOOPProtocolCommon.h"
#include "IFCohProtocol.h"

namespace ns3 { 

  // MESI Protocal states enconding (Private Cache Side)
  enum class SNOOP_MESIPrivCacheState {
    I = CohProtType::SNOOP_MESI,   
    IS_ad,
    IS_d,
    IS_a,
    IE_a,
    IE_d,
    IE_d_S,
    IE_d_SI,
    IE_d_I,
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
    E,
    M,
    MI_a,
    EI_a,
    II_a,
    EorM_a_I,
    I_EorM_d_I
  };
         
  // Protocal states enconding  (Memory Controller Side)
  enum class SNOOP_MESISharedCacheState {
    I = CohProtType::SNOOP_MESI, 
    S,
    EorM,	    
    EorM_d_S,	    
    EorM_d_EorM,
    EorM_d_I,
    EorM_a,
    DRAM_d
  };

  class MESI : public ns3::Object, public ns3::IFCohProtocol {
  public:

      MESI ();

     ~MESI ();

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
     
     int SNOOPSharedCacheWaitDRAMState ();

     // Protocol FSM at SharedCache Control Side
     void SNOOPSharedEventProcessing (SNOOPSharedEventType      eventType, 
                                      bool                         Cache2Cache,
                                      SNOOPSharedEventList         eventList, 
                                      int                          &cacheState, 
                                      SNOOPSharedOwnerState        &ownerState,
                                      SNOOPSharedCtrlAction        &ctrlAction);
    
     std::string SharedStateName (int s);

     bool ByPassMESI (int s);
  }; // class MESI


} 

#endif /* _MESI_H */
