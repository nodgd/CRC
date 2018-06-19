#ifndef REPL_STATE_H
#define REPL_STATE_H

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"

// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_FIFO       = 2,
    CRC_REPL_SECRU      = 3,
    CRC_REPL_PSS        = 4,
    CRC_REPL_FREQ64     = 5,
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;

    // CONTESTANTS: Add extra state per cache line here

} LINE_REPLACEMENT_STATE;

const double PSS_CONSTANT = 0.8;

typedef unsigned long long uint64;

// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{

  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    
    LINE_REPLACEMENT_STATE   **repl;
    
    UINT32 * fifo_repl;
    
    double ** pss_val;
    
    uint64 ** freq64;
    int * freq64_bc;

    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here

  public:

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32  GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ostream&   PrintStats( ostream &out);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    
    INT32  Get_FIFO_Victim( UINT32 setIndex );
    
    INT32  Get_SECRU_Victim( UINT32 setIndex );
    void   UpdateSECRU( UINT32 setIndex, INT32 updateWayID );
    
    INT32  Get_PSS_Victim( UINT32 setIndex );
    void   UpdatePSS( UINT32 setIndex, INT32 updateWayID );
    
    int getBitCount(uint64 x);
    INT32  Get_FREQ64_Victim( UINT32 setIndex );
    void   UpdateFREQ64( UINT32 setIndex, INT32 updateWayID );
};


#endif

