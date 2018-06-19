#include "replacement_state.h"

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

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state
    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    // My algorithm: FIFO
    fifo_repl = new UINT32[numsets];
    for (UINT32 setIndex = 0; setIndex < numsets; setIndex ++) {
        fifo_repl[setIndex] = 0;
    }
    // My algorithm: SECRU
    
    // My algorithm: PSS
    pss_val = new double * [numsets];
    for (int si = 0; si < (int) numsets; si ++) {
        pss_val[si] = new double [assoc];
        for (int w = 0; w < (int) assoc; w ++) {
            pss_val[si][w] = 0;
        }
    }
    
    //My algorithm: FREQ64
    freq64 = new uint64 * [numsets];
    for (int si = 0; si < (int) numsets; si ++) {
        freq64[si] = new uint64 [assoc];
        for (int w = 0; w < (int) assoc; w ++) {
            freq64[si][w] = 0;
        }
    }
    freq64_bc = new int [1 << 16];
    freq64_bc[0] = 0;
    for (int i = 1; i < 1 << 16; i ++) {
        freq64_bc[i] = freq64_bc[i >> 1] + (i & 1);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the thread id, set index, pointers to ways in current set    //
// and the associativity.  We are also providing the PC, physical address,    //
// and accesstype should you wish to use them at victim selection time.       //
// The return value is the physical way index for the line being replaced.    //
// Return -1 if you wish to bypass LLC.                                       //
//                                                                            //
// vicSet is the current set. You can access the contents of the set by       //
// indexing using the wayID which ranges from 0 to assoc-1 e.g. vicSet[0]     //
// is the first way and vicSet[4] is the 4th physical way of the cache.       //
// Elements of LINE_STATE are defined in crc_cache_defs.h                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_FIFO )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
        return Get_FIFO_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_SECRU )
    {
        return Get_SECRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_PSS )
    {
        return Get_PSS_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_FREQ64 )
    {
        return Get_FREQ64_Victim( setIndex );
    }

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_FIFO )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
    }
    else if( replPolicy == CRC_REPL_SECRU )
    {
        UpdateSECRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_PSS )
    {
        UpdatePSS( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_FREQ64 )
    {
        UpdateFREQ64( setIndex, updateWayID );
    }
    
    
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here

    return out;
    
}

/*
 *  My algorithm: FIFO
 */
INT32 CACHE_REPLACEMENT_STATE::Get_FIFO_Victim( UINT32 setIndex ) {
    INT32 fifoWay = fifo_repl[setIndex];
    if (++ fifo_repl[setIndex] == assoc) {
        fifo_repl[setIndex] = 0;
    }
    return fifoWay;
}

/*
 *  My algorithm: SECRU
 */
INT32 CACHE_REPLACEMENT_STATE::Get_SECRU_Victim( UINT32 setIndex ) {
    LINE_REPLACEMENT_STATE * replSet = repl[setIndex];
    INT32 secruWay = 0;
    for (UINT32 way=0; way < assoc; way ++) {
        if (replSet[way].LRUstackposition == 1) { // pos = 0, 1, ... , assoc - 1; == 1 is the second one
            secruWay = way;
            break;
        }
    }
    return secruWay;
}
void CACHE_REPLACEMENT_STATE::UpdateSECRU( UINT32 setIndex, INT32 updateWayID ) {
    UINT32 curPos = repl[setIndex][updateWayID].LRUstackposition;
    for (UINT32 way = 0; way < assoc; way ++) {
        if (repl[setIndex][way].LRUstackposition < curPos) {
            repl[setIndex][way].LRUstackposition ++;
        }
    }
    repl[setIndex][updateWayID].LRUstackposition = 0;
}

/*
 *  My algorithm: PSS
 */
INT32 CACHE_REPLACEMENT_STATE::Get_PSS_Victim( UINT32 setIndex ) {
    double * curPss = pss_val[setIndex];
    int minWay = 0;
    for (int way = 0; way < (int) assoc; way ++) {
        if (curPss[minWay] > curPss[way]) {
            minWay = way;
        }
    }
    curPss[minWay] = 0;
    return minWay;
}
void CACHE_REPLACEMENT_STATE::UpdatePSS( UINT32 setIndex, INT32 updateWayID ) {
    double * curPss = pss_val[setIndex];
    for (int way = 0; way < (int) assoc; way ++) {
        curPss[way] *= PSS_CONSTANT;
    }
    curPss[updateWayID] += 1.0;
}

/*
 *  My algorithm: FREQ64
 */
int CACHE_REPLACEMENT_STATE::getBitCount(uint64 x) {
    return freq64_bc[x & 0xffff] + freq64_bc[x >> 16 & 0xffff] + freq64_bc[x >> 32 & 0xffff] + freq64_bc[x >> 48 & 0xffff];
}
INT32 CACHE_REPLACEMENT_STATE::Get_FREQ64_Victim( UINT32 setIndex ) {
    uint64 * curFreq = freq64[setIndex];
    int minWay = 0;
    for (int way = 0; way < (int) assoc; way ++) {
        if (getBitCount(curFreq[minWay]) > getBitCount(curFreq[way])) {
            minWay = way;
        }
    }
    curFreq[minWay] = 0;
    return minWay;
}
void CACHE_REPLACEMENT_STATE::UpdateFREQ64( UINT32 setIndex, INT32 updateWayID ) {
    uint64 * curFreq = freq64[setIndex];
    for (int way = 0; way < (int) assoc; way ++) {
        curFreq[way] <<= 1;
    }
    curFreq[updateWayID] |= 1;
}

