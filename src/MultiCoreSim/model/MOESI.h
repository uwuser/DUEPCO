/*
 * File  :      MOESI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 5, 2020
 */

#ifndef _MOESI_H
#define _MOESI_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "SNOOPProtocolCommon.h"
#include "IFCohProtocol.h"

namespace ns3 { 

  // MOESI Protocal states enconding (Private Cache Side)
  enum class SNOOP_MOESIPrivCacheState {
    I = CohProtType::SNOOP_MOESI,   
    IS_ad,
    IS_d,
    IS_a,
    IE_a,
    IE_d,
    IE_d_O,
    IE_d_I,
    IS_d_I,
    IM_ad,
    IM_d,
    IM_a,
    IM_d_I,
    IM_d_O,
    S,
    SM_ad,
    SM_d,
    SM_a,
    SM_d_I,
    SM_d_O,
    E,
    O,
    OM_a,
    M,
    MI_a,
    OI_a,
    EI_a,
    II_a,
    EorMorO_a_I,
    I_EorM_d_I,
    O_d_I,
    OM_a_I
  };

  // Protocal states enconding  (Memory Controller Side)
  enum class SNOOP_MOESISharedCacheState {
    I = CohProtType::SNOOP_MOESI, 
    S,
    EorM,
    O,	    
    O_d_S,
    EorM_d_I,
    EorM_a,
    O_a,
    DRAM_d
  };

  class MOESI : public ns3::Object, public ns3::IFCohProtocol {
  public:

      MOESI ();

     ~MOESI ();

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
     void SNOOPSharedEventProcessing (SNOOPSharedEventType         eventType, 
                                      bool                         Cache2Cache,
                                      SNOOPSharedEventList         eventList, 
                                      int                          &cacheState, 
                                      SNOOPSharedOwnerState        &ownerState,
                                      SNOOPSharedCtrlAction        &ctrlAction);
    
     std::string SharedStateName (int s);
  }; // class MOESI


}

#endif /* _MOESI_H */
