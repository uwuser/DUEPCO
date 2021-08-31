/*
 * File  :      PMSI.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 18, 2020
 */

#ifndef _PMSI_H
#define _PMSI_H


#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"

#include "SNOOPProtocolCommon.h"
#include "IFCohProtocol.h"

namespace ns3 { 


  // Protocal states enconding
  enum class SNOOP_PMSIPrivCacheState {
   I = CohProtType::SNOOP_PMSI,   
   S,       
   M,       
   IS_a_d,  
   IM_a_d,  
   IS_a,    
   IM_a,    
   IS_d,    
   IM_d,   
   SM_w,   
   MI_wb,   
   MS_wb,   
   IM_d_I,  
   IS_d_I,  
   IM_d_S   
  };

  // Protocal states enconding  (Memory Controller Side)
  enum class SNOOP_PMSISharedCacheState {
    IorS = CohProtType::SNOOP_PMSI, 
    M,	    
    M_d_M,	    
    M_d_IorS,
    IorSorM_a
  };

  class PMSI : public ns3::Object, public ns3::IFCohProtocol {
  public:

      PMSI ();

     ~PMSI ();

      // Override TypeId.
      static TypeId GetTypeId(void);


     // Process Received Events, update Cache state if needed,
     // and issue needed transaction if any.
     void SNOOPPrivEventProcessing 
          (SNOOPPrivEventType  eventType, 
           SNOOPPrivEventList  eventList,
           int                 &cacheState,       
           SNOOPPrivCohTrans   &trans2Issue,
           SNOOPPrivCtrlAction &ctrlAction,
           bool                cache2Cache
           );
                                            
     void SNOOPSharedEventProcessing (SNOOPSharedEventType         eventType, 
                                      bool                         Cache2Cache,
                                      SNOOPSharedEventList         eventList, 
                                      int                          &cacheState, 
                                      SNOOPSharedOwnerState        &ownerState,
                                      SNOOPSharedCtrlAction        &ctrlAction);
                                          

    bool IsValidBlk (int s);

    SNOOPPrivEventPriority PrivCacheEventPriority (SNOOPPrivEventList eventList,SNOOPPrivEventsCacheInfo eventCacheInfoList );


     std::string PrivStateName (int s);


     std::string SharedStateName (int s);


  };

}

#endif /* _PMSI_H */


