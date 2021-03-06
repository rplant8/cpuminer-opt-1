#include "skein-gate.h"
#include <string.h>
#include <stdint.h>
#include "skein-hash-4way.h"

// 8 way is faster than SHA on Icelake
// SHA is faster than 4 way on Ryzen
//
#if defined(__SHA__)
  #include <openssl/sha.h>
#endif
#include "algo/sha/sha-hash-4way.h"

#if defined (SKEIN_8WAY)

void skeinhash_8way( void *state, const void *input )
{
     uint64_t vhash64[8*8] __attribute__ ((aligned (128)));
     skein512_8way_context ctx_skein;

//#if defined(__SHA__)
//     uint32_t hash0[16] __attribute__ ((aligned (64)));
//     uint32_t hash1[16] __attribute__ ((aligned (64)));
//     uint32_t hash2[16] __attribute__ ((aligned (64)));
//     uint32_t hash3[16] __attribute__ ((aligned (64)));
//     uint32_t hash4[16] __attribute__ ((aligned (64)));
//     uint32_t hash5[16] __attribute__ ((aligned (64)));
//     uint32_t hash6[16] __attribute__ ((aligned (64)));
//     uint32_t hash7[16] __attribute__ ((aligned (64)));
//     SHA256_CTX           ctx_sha256;
//#else
     uint32_t vhash32[16*8] __attribute__ ((aligned (128)));
     sha256_8way_context ctx_sha256;
//#endif

     skein512_8way_init( &ctx_skein );
     skein512_8way_update( &ctx_skein, input, 80 );
     skein512_8way_close( &ctx_skein, vhash64 );
/*
#if defined(__SHA__)      
     dintrlv_8x64( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                   vhash64, 512 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash0, 64 );
     SHA256_Final( (unsigned char*)hash0, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash1, 64 );
     SHA256_Final( (unsigned char*)hash1, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash2, 64 );
     SHA256_Final( (unsigned char*)hash2, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash3, 64 );
     SHA256_Final( (unsigned char*)hash3, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash4, 64 );
     SHA256_Final( (unsigned char*)hash4, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash5, 64 );
     SHA256_Final( (unsigned char*)hash5, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash6, 64 );
     SHA256_Final( (unsigned char*)hash6, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash7, 64 );
     SHA256_Final( (unsigned char*)hash7, &ctx_sha256 );
     
     intrlv_8x32( state, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
                  hash7, 256 );
#else
*/

     rintrlv_8x64_8x32( vhash32, vhash64, 512 );
//     dintrlv_8x64( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
//                   vhash64, 512 );
//     intrlv_8x32( vhash32, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
//                   hash7, 512 );

     sha256_8way_init( &ctx_sha256 );
     sha256_8way( &ctx_sha256, vhash32, 64 );
     sha256_8way_close( &ctx_sha256, state );
//#endif
}

int scanhash_skein_8way( struct work *work, uint32_t max_nonce,
                         uint64_t *hashes_done, struct thr_info *mythr )
{
    uint32_t vdata[20*8] __attribute__ ((aligned (128)));
    uint32_t hash[16*8] __attribute__ ((aligned (64)));
    uint32_t lane_hash[8] __attribute__ ((aligned (64)));
    uint32_t *hash7 = &(hash[7<<3]);
    uint32_t *pdata = work->data;
    uint32_t *ptarget = work->target;
    const uint32_t Htarg = ptarget[7];
    const uint32_t first_nonce = pdata[19];
    uint32_t n = first_nonce;
    __m512i  *noncev = (__m512i*)vdata + 9;   // aligned
    int thr_id = mythr->id; 

   mm512_bswap32_intrlv80_8x64( vdata, pdata );
   do
   {
       *noncev = mm512_intrlv_blend_32( mm512_bswap_32(
                _mm512_set_epi32( n+7, 0, n+6, 0, n+5, 0, n+4, 0,
                                  n+3, 0, n+2, 0, n+1, 0, n  , 0 ) ), *noncev );

       skeinhash_8way( hash, vdata );

       for ( int lane = 0; lane < 8; lane++ )
       if (  hash7[ lane ] <= Htarg )
       {
          extr_lane_8x32( lane_hash, hash, lane, 256 );
          if ( fulltest( lane_hash, ptarget ) )
          {
             pdata[19] = n + lane;
             submit_lane_solution( work, lane_hash, mythr, lane );
          }
       }
       n += 8;
    } while ( (n < max_nonce-8) && !work_restart[thr_id].restart );

    *hashes_done = n - first_nonce;
    return 0;
}

#elif defined (SKEIN_4WAY)

void skeinhash_4way( void *state, const void *input )
{
     uint64_t vhash64[8*4] __attribute__ ((aligned (128)));
     skein512_4way_context ctx_skein;
#if defined(__SHA__)
     uint32_t hash0[16] __attribute__ ((aligned (64)));
     uint32_t hash1[16] __attribute__ ((aligned (64)));
     uint32_t hash2[16] __attribute__ ((aligned (64)));
     uint32_t hash3[16] __attribute__ ((aligned (64)));
     SHA256_CTX           ctx_sha256;
#else
     uint32_t vhash32[16*4] __attribute__ ((aligned (64)));
     sha256_4way_context ctx_sha256;
#endif

     skein512_4way_init( &ctx_skein );
     skein512_4way_update( &ctx_skein, input, 80 );
     skein512_4way_close( &ctx_skein, vhash64 );

#if defined(__SHA__)      
     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash64, 512 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash0, 64 );
     SHA256_Final( (unsigned char*)hash0, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash1, 64 );
     SHA256_Final( (unsigned char*)hash1, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash2, 64 );
     SHA256_Final( (unsigned char*)hash2, &ctx_sha256 );

     SHA256_Init( &ctx_sha256 );
     SHA256_Update( &ctx_sha256, (unsigned char*)hash3, 64 );
     SHA256_Final( (unsigned char*)hash3, &ctx_sha256 );

     intrlv_4x32( state, hash0, hash1, hash2, hash3, 256 );
#else
     rintrlv_4x64_4x32( vhash32, vhash64, 512 );

     sha256_4way_init( &ctx_sha256 );
     sha256_4way( &ctx_sha256, vhash32, 64 );
     sha256_4way_close( &ctx_sha256, state );
#endif
}

int scanhash_skein_4way( struct work *work, uint32_t max_nonce,
                         uint64_t *hashes_done, struct thr_info *mythr )
{
    uint32_t vdata[20*4] __attribute__ ((aligned (64)));
    uint32_t hash[16*4] __attribute__ ((aligned (64)));
    uint32_t lane_hash[8] __attribute__ ((aligned (32)));
    uint32_t *hash7 = &(hash[7<<2]);
    uint32_t *pdata = work->data;
    uint32_t *ptarget = work->target;
    const uint32_t Htarg = ptarget[7];
    const uint32_t first_nonce = pdata[19];
    uint32_t n = first_nonce;
    __m256i  *noncev = (__m256i*)vdata + 9;   // aligned
    int thr_id = mythr->id; 

   mm256_bswap32_intrlv80_4x64( vdata, pdata );
   do
   {
       *noncev = mm256_intrlv_blend_32( mm256_bswap_32(
                _mm256_set_epi32( n+3, 0, n+2, 0, n+1, 0, n, 0 ) ), *noncev );

       skeinhash_4way( hash, vdata );

       for ( int lane = 0; lane < 4; lane++ )
       if (  hash7[ lane ] <= Htarg )
       {
          extr_lane_4x32( lane_hash, hash, lane, 256 );
          if ( fulltest( lane_hash, ptarget ) )
          {
             pdata[19] = n + lane;
             submit_lane_solution( work, lane_hash, mythr, lane );
          }
       }
       n += 4;
    } while ( (n < max_nonce-4) && !work_restart[thr_id].restart );

    *hashes_done = n - first_nonce;
    return 0;
}

#endif
