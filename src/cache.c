//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <stdio.h>

//
// TODO:Student Information
//
const char *studentName = "Muhammad Zubair Khan";
const char *studentID   = "A15876135";
const char *email       = "mzkhan@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$
uint8_t icacheIdxBits;

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$
uint8_t dcacheIdxBits;

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint8_t l2cacheIdxBits;
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

// enum CacheType { I, D, L2 };

struct SetEntry
{
  uint8_t LRUbit;
  uint8_t valid;
  uint32_t tag;
};

struct SetEntry** icache;
struct SetEntry** dcache;
struct SetEntry** l2cache;

uint8_t boBits;
// uint8_t idxBits;
// uint8_t tagBits;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  uint32_t i, j;

  icache = (struct SetEntry**)malloc(icacheSets * sizeof(struct SetEntry*));
  for (i = 0; i < icacheSets; ++i) {
    icache[i] = (struct SetEntry*)malloc(icacheAssoc * sizeof(struct SetEntry));
    for (j = 0; j < icacheAssoc; ++j) {
      icache[i][j].valid = FALSE;
    }
  }
  
  dcache = (struct SetEntry**)malloc(dcacheSets * sizeof(struct SetEntry*));
  for (i = 0; i < dcacheSets; ++i) {
    dcache[i] = (struct SetEntry*)malloc(dcacheAssoc * sizeof(struct SetEntry));
    for (j = 0; j < dcacheAssoc; ++j) {
      dcache[i][j].valid = FALSE;
    }
  }

  l2cache = (struct SetEntry**)malloc(l2cacheSets * sizeof(struct SetEntry*));
  for (i = 0; i < l2cacheSets; ++i) {
    l2cache[i] = (struct SetEntry*)malloc(l2cacheAssoc * sizeof(struct SetEntry));
    for (j = 0; j < l2cacheAssoc; ++j) {
      l2cache[i][j].valid = FALSE;
    }
  }

  uint32_t temp;

  boBits = 0;
  if (blocksize) {
    temp = blocksize;
    while (temp >>= 1) {
      ++boBits;
    }
  }

  icacheIdxBits = 0;
  if (icacheSets) {
    temp = icacheSets;
    while (temp >>= 1) {
      ++icacheIdxBits;
    }
  }
  

  dcacheIdxBits = 0;
  if (dcacheSets) {
    temp = dcacheSets;
    while (temp >>= 1) {
      ++dcacheIdxBits;
    }
  }

  l2cacheIdxBits = 0;
  if (l2cacheSets) {
    temp = l2cacheSets;
    while (temp >>= 1) {
      ++l2cacheIdxBits;
    }
  }
}

void
evictIcache(uint32_t l2tag, uint32_t l2idx) {

  if (!icacheSets)
    return;

  uint32_t addr = (l2tag << l2cacheIdxBits) | l2idx;

  uint32_t idx = addr & ((1 << icacheIdxBits) - 1);
  uint32_t tag = addr >> icacheIdxBits;

  int j;
  for (j = 0; j < icacheAssoc; ++j) {
    if ((icache[idx][j].valid == TRUE) && (icache[idx][j].tag == tag)) {
      int k;
      for (k = 0; k < icacheAssoc; ++k) {
        if (k == j) 
          continue;
        if ((icache[idx][k].valid == TRUE) && (icache[idx][k].LRUbit < icache[idx][j].LRUbit)) {
          ++(icache[idx][k].LRUbit);
        }
      }
      icache[idx][j].valid = FALSE;
      return;
    }
  }
}

void
evictDcache(uint32_t l2tag, uint32_t l2idx) {

  if (!dcacheSets)
    return;

  uint32_t addr = (l2tag << l2cacheIdxBits) | l2idx;

  uint32_t idx = addr & ((1 << dcacheIdxBits) - 1);
  uint32_t tag = addr >> dcacheIdxBits;

  int j;
  for (j = 0; j < dcacheAssoc; ++j) {
    if ((dcache[idx][j].valid == TRUE) && (dcache[idx][j].tag == tag)) {
      int k;
      for (k = 0; k < dcacheAssoc; ++k) {
        if (k == j) 
          continue;
        if ((dcache[idx][k].valid == TRUE) && (dcache[idx][k].LRUbit < dcache[idx][j].LRUbit)) {
          ++(dcache[idx][k].LRUbit);
        }
      }
      dcache[idx][j].valid = FALSE;
      return;
    }
  }
}

void
updateLRUbits(enum CacheType cacheType, uint32_t setIdx, uint32_t accessIdx) {
  struct SetEntry** cache;
  uint32_t cacheAssoc; 


  switch(cacheType) {
    case I:
      cache = icache;
      cacheAssoc = icacheAssoc;
      break;
    case D:
      cache = dcache;
      cacheAssoc = dcacheAssoc;
      break;
    case L2:
      cache = l2cache;
      cacheAssoc = dcacheAssoc;
      break;
    default:
      printf("Warning: Undefined Cache Type!\n");
      return;
  }

  uint8_t accessLRUbit = cache[setIdx][accessIdx].LRUbit;
  cache[setIdx][accessIdx].LRUbit = cacheAssoc - 1; 

  int j;
  for (j = 0; j < cacheAssoc; ++j) {
    if (j == accessIdx) 
      continue;
    if ((cache[setIdx][j].valid == TRUE) && (cache[setIdx][j].LRUbit >= accessLRUbit)) {
      --(cache[setIdx][j].LRUbit);
    }
  }
}


// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //

  if (!icacheSets) {
    icachePenalties += l2cacheHitTime;
    return l2cache_access(addr, I);
  }

  ++icacheRefs;
 
  uint32_t idx = (addr >> boBits) & ((1 << icacheIdxBits) - 1);
  uint32_t tag = addr >> (boBits + icacheIdxBits);

  int j;
  for (j = 0; j < icacheAssoc; ++j) {
    if ((icache[idx][j].valid == TRUE) && (icache[idx][j].tag == tag)) {
      updateLRUbits(I, idx, j); 
      return icacheHitTime;
    }
  }

  for (j = 0; j < icacheAssoc; ++j) {
    if ((icache[idx][j].valid == FALSE) || (icache[idx][j].LRUbit == 0)) {
      icache[idx][j].valid = TRUE;
      icache[idx][j].tag = tag;
      icache[idx][j].LRUbit = 0;
      updateLRUbits(I, idx, j);
      ++icacheMisses;
      break;
    }
  }

  icachePenalties += l2cacheHitTime;
  return l2cache_access(addr, I);
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //

  if (!dcacheSets) {
    dcachePenalties += l2cacheHitTime;
    return l2cache_access(addr, D);
  }

  ++dcacheRefs;
  
  uint32_t idx = (addr >> boBits) & ((1 << dcacheIdxBits) - 1);
  uint32_t tag = addr >> (boBits + dcacheIdxBits);

  int j;
  for (j = 0; j < dcacheAssoc; ++j) {
    if ((dcache[idx][j].valid == TRUE) && (dcache[idx][j].tag == tag)) {
      updateLRUbits(D, idx, j);
      return dcacheHitTime;
    }
  }

  for (j = 0; j < dcacheAssoc; ++j) {
    if ((dcache[idx][j].valid == FALSE) || (dcache[idx][j].LRUbit == 0)) {
      dcache[idx][j].valid = TRUE;
      dcache[idx][j].tag = tag;
      dcache[idx][j].LRUbit = 0;
      updateLRUbits(D, idx, j);
      ++dcacheMisses;
      break;
    }
  }

  dcachePenalties += l2cacheHitTime;
  return l2cache_access(addr, D);
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr, enum CacheType l1cacheType)
{
  //
  //TODO: Implement L2$
  //

  if (!l2cacheSets) {
    if (l1cacheType == I) {
      icachePenalties += memspeed;
    }
    if (l1cacheType == D) {
      dcachePenalties += memspeed;
    }
    l2cachePenalties += memspeed;
    return memspeed;
  }
    

  ++l2cacheRefs;
  
  uint32_t idx = (addr >> boBits) & ((1 << l2cacheIdxBits) - 1);
  uint32_t tag = addr >> (boBits + l2cacheIdxBits);

  int j;
  for (j = 0; j < l2cacheAssoc; ++j) {
    if ((l2cache[idx][j].valid == TRUE) && (l2cache[idx][j].tag == tag)) {
      updateLRUbits(L2, idx, j);
      return l2cacheHitTime;
    }
  }

  for (j = 0; j < l2cacheAssoc; ++j) {
    if ((l2cache[idx][j].valid == FALSE) || (l2cache[idx][j].LRUbit == 0)) {
      // if (inclusive==TRUE && l2cache[idx][j].valid == TRUE) {
      //   if (l1cacheType == I)
      //     evictIcache(l2cache[idx][j].tag, idx);
      //   if (l1cacheType == D)
      //     evictDcache(l2cache[idx][j].tag, idx);
      // }
      l2cache[idx][j].valid = TRUE;
      l2cache[idx][j].tag = tag;
      l2cache[idx][j].LRUbit = 0;
      updateLRUbits(L2, idx, j);
      ++l2cacheMisses;
      break;
    }
  }

  if (l1cacheType == I) {
    icachePenalties += memspeed;
  }
  if (l1cacheType == D) {
    dcachePenalties += memspeed;
  }
  l2cachePenalties += memspeed;
  return memspeed;
}
