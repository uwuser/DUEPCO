
#ifdef NS3_MODULE_COMPILATION
# error "Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts."
#endif

#ifndef NS3_MODULE_MULTICORESIM
    

// Module headers:
#include "BusArbiter.h"
#include "CacheXml.h"
#include "CohProtocolCommon.h"
#include "CpuCoreGenerator.h"
#include "DRAMCtrl.h"
#include "GenericCache.h"
#include "IFCohProtocol.h"
#include "IdGenerator.h"
#include "L1BusCnfgXml.h"
#include "LatencyLogger.h"
#include "MCoreSimProject.h"
#include "MCoreSimProjectXml.h"
#include "MESI.h"
#include "MOESI.h"
#include "MSI.h"
#include "MemTemplate.h"
#include "PMSI.h"
#include "PrivateCacheCtrl.h"
#include "SNOOPPrivCohProtocol.h"
#include "SNOOPProtocolCommon.h"
#include "SharedCacheCtrl.h"
#include "tinystr.h"
#include "tinyxml.h"
#endif
