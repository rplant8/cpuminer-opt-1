#include "x15-gate.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "algo/blake/blake-hash-4way.h"
#include "algo/bmw/bmw-hash-4way.h"
#include "algo/groestl/aes_ni/hash-groestl.h"
#include "algo/skein/skein-hash-4way.h"
#include "algo/jh/jh-hash-4way.h"
#include "algo/keccak/keccak-hash-4way.h"
#include "algo/luffa/luffa-hash-2way.h"
#include "algo/cubehash/cubehash_sse2.h"
#include "algo/cubehash/cube-hash-2way.h"
#include "algo/shavite/sph_shavite.h"
#include "algo/simd/simd-hash-2way.h"
#include "algo/echo/aes_ni/hash_api.h"
#include "algo/echo/sph_echo.h"
#include "algo/hamsi/hamsi-hash-4way.h"
#include "algo/fugue/sph_fugue.h"
#include "algo/shabal/shabal-hash-4way.h"
#include "algo/whirlpool/sph_whirlpool.h"

#if defined(X15_8WAY)


typedef struct {
    blake512_8way_context   blake;
    bmw512_8way_context     bmw;
    hashState_groestl       groestl;
    skein512_8way_context   skein;
    jh512_8way_context      jh;
    keccak512_8way_context  keccak;
    luffa_4way_context      luffa;
    cube_4way_context       cube;
    sph_shavite512_context  shavite;
    simd_4way_context       simd;
    hashState_echo          echo;
    hamsi512_8way_context   hamsi;
    sph_fugue512_context    fugue;
    shabal512_8way_context  shabal;
    sph_whirlpool_context   whirlpool;
} x15_8way_ctx_holder;

x15_8way_ctx_holder x15_8way_ctx __attribute__ ((aligned (64)));

void init_x15_8way_ctx()
{
     blake512_8way_init( &x15_8way_ctx.blake );
     bmw512_8way_init( &x15_8way_ctx.bmw );
     init_groestl( &x15_8way_ctx.groestl, 64 );
     skein512_8way_init( &x15_8way_ctx.skein );
     jh512_8way_init( &x15_8way_ctx.jh );
     keccak512_8way_init( &x15_8way_ctx.keccak );
     luffa_4way_init( &x15_8way_ctx.luffa, 512 );
     cube_4way_init( &x15_8way_ctx.cube, 512, 16, 32 );
     sph_shavite512_init( &x15_8way_ctx.shavite );
     simd_4way_init( &x15_8way_ctx.simd, 512 );
     init_echo( &x15_8way_ctx.echo, 512 );
     hamsi512_8way_init( &x15_8way_ctx.hamsi );
     sph_fugue512_init( &x15_8way_ctx.fugue );
     shabal512_8way_init( &x15_8way_ctx.shabal );
     sph_whirlpool_init( &x15_8way_ctx.whirlpool );
};

void x15_8way_hash( void *state, const void *input )
{
     uint64_t vhash[8*8] __attribute__ ((aligned (128)));
     uint64_t vhash0[4*8] __attribute__ ((aligned (64)));
     uint64_t vhash1[4*8] __attribute__ ((aligned (64)));
     uint64_t hash0[8] __attribute__ ((aligned (64)));
     uint64_t hash1[8] __attribute__ ((aligned (64)));
     uint64_t hash2[8] __attribute__ ((aligned (64)));
     uint64_t hash3[8] __attribute__ ((aligned (64)));
     uint64_t hash4[8] __attribute__ ((aligned (64)));
     uint64_t hash5[8] __attribute__ ((aligned (64)));
     uint64_t hash6[8] __attribute__ ((aligned (64)));
     uint64_t hash7[8] __attribute__ ((aligned (64)));
     x15_8way_ctx_holder ctx;
     memcpy( &ctx, &x15_8way_ctx, sizeof(x15_8way_ctx) );

     // 1 Blake
     blake512_8way_update( &ctx.blake, input, 80 );
     blake512_8way_close( &ctx.blake, vhash );

     // 2 Bmw
     bmw512_8way_update( &ctx.bmw, vhash, 64 );
     bmw512_8way_close( &ctx.bmw, vhash );
     dintrlv_8x64_512( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                       vhash );

     // 3 Groestl
     update_and_final_groestl( &ctx.groestl, (char*)hash0, (char*)hash0, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash1, (char*)hash1, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash2, (char*)hash2, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash3, (char*)hash3, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash4, (char*)hash4, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash5, (char*)hash5, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash6, (char*)hash6, 512 );
     memcpy( &ctx.groestl, &x15_8way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash7, (char*)hash7, 512 );

     intrlv_8x64_512( vhash, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
                      hash7 );
     skein512_8way_update( &ctx.skein, vhash, 64 );
     skein512_8way_close( &ctx.skein, vhash );

     // 5 JH
     jh512_8way_update( &ctx.jh, vhash, 64 );
     jh512_8way_close( &ctx.jh, vhash );

     // 6 Keccak
     keccak512_8way_update( &ctx.keccak, vhash, 64 );
     keccak512_8way_close( &ctx.keccak, vhash );
     dintrlv_8x64_512( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                       vhash );

     rintrlv_8x64_4x128( vhash0, vhash1, vhash, 512 );

     luffa_4way_update_close( &ctx.luffa, vhash0, vhash0, 64 );
     luffa_4way_init( &ctx.luffa, 512 );
     luffa_4way_update_close( &ctx.luffa, vhash1, vhash1, 64 );

     cube_4way_update_close( &ctx.cube, vhash0, vhash0, 64 );
     cube_4way_init( &ctx.cube, 512, 16, 32 );
     cube_4way_update_close( &ctx.cube, vhash1, vhash1, 64 );

     dintrlv_4x128_512( hash0, hash1, hash2, hash3, vhash0 );
     dintrlv_4x128_512( hash4, hash5, hash6, hash7, vhash1 );

     // 9 Shavite
     sph_shavite512( &ctx.shavite, hash0, 64 );
     sph_shavite512_close( &ctx.shavite, hash0 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash1, 64 );
     sph_shavite512_close( &ctx.shavite, hash1 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash2, 64 );
     sph_shavite512_close( &ctx.shavite, hash2 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash3, 64 );
     sph_shavite512_close( &ctx.shavite, hash3 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash4, 64 );
     sph_shavite512_close( &ctx.shavite, hash4 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash5, 64 );
     sph_shavite512_close( &ctx.shavite, hash5 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash6, 64 );
     sph_shavite512_close( &ctx.shavite, hash6 );
     memcpy( &ctx.shavite, &x15_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash7, 64 );
     sph_shavite512_close( &ctx.shavite, hash7 );

     // 10 Simd
     intrlv_4x128_512( vhash, hash0, hash1, hash2, hash3 );
     simd_4way_update_close( &ctx.simd, vhash, vhash, 512 );
     dintrlv_4x128_512( hash0, hash1, hash2, hash3, vhash );
     intrlv_4x128_512( vhash, hash4, hash5, hash6, hash7 );
     simd_4way_init( &ctx.simd, 512 );
     simd_4way_update_close( &ctx.simd, vhash, vhash, 512 );
     dintrlv_4x128_512( hash4, hash5, hash6, hash7, vhash );

     // 11 Echo
     update_final_echo( &ctx.echo, (BitSequence *)hash0,
                       (const BitSequence *) hash0, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash1,
                       (const BitSequence *) hash1, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash2,
                       (const BitSequence *) hash2, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash3,
                       (const BitSequence *) hash3, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash4,
                       (const BitSequence *) hash4, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash5,
                       (const BitSequence *) hash5, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash6,
                       (const BitSequence *) hash6, 512 );
     memcpy( &ctx.echo, &x15_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash7,
                       (const BitSequence *) hash7, 512 );


     // 12 Hamsi parallel 4way 64 bit
     intrlv_8x64_512( vhash, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
                      hash7 );
     hamsi512_8way_update( &ctx.hamsi, vhash, 64 );
     hamsi512_8way_close( &ctx.hamsi, vhash );
     dintrlv_8x64_512( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                       vhash );

     // 13 Fugue
     sph_fugue512( &ctx.fugue, hash0, 64 );
     sph_fugue512_close( &ctx.fugue, hash0 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash1, 64 );
     sph_fugue512_close( &ctx.fugue, hash1 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash2, 64 );
     sph_fugue512_close( &ctx.fugue, hash2 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash3, 64 );
     sph_fugue512_close( &ctx.fugue, hash3 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash4, 64 );
     sph_fugue512_close( &ctx.fugue, hash4 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash5, 64 );
     sph_fugue512_close( &ctx.fugue, hash5 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash6, 64 );
     sph_fugue512_close( &ctx.fugue, hash6 );
     memcpy( &ctx.fugue, &x15_8way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash7, 64 );
     sph_fugue512_close( &ctx.fugue, hash7 );


     // 14 Shabal, parallel 32 bit
     intrlv_8x32_512( vhash, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
                      hash7 );
     shabal512_8way_update( &ctx.shabal, vhash, 64 );
     shabal512_8way_close( &ctx.shabal, vhash );
     dintrlv_8x32_512( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                       vhash );

     // 15 Whirlpool
     sph_whirlpool( &ctx.whirlpool, hash0, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash0 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash1, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash1 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash2, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash2 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash3, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash3 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash4, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash4 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash5, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash5 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash6, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash6 );
     memcpy( &ctx.whirlpool, &x15_8way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash7, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash7 );

     memcpy( state,    hash0, 32 );
     memcpy( state+ 32, hash1, 32 );
     memcpy( state+ 64, hash2, 32 );
     memcpy( state+ 96, hash3, 32 );
     memcpy( state+128, hash4, 32 );
     memcpy( state+160, hash5, 32 );
     memcpy( state+192, hash6, 32 );
     memcpy( state+224, hash7, 32 );
}

int scanhash_x15_8way( struct work *work, uint32_t max_nonce,
                       uint64_t *hashes_done, struct thr_info *mythr )
{
     uint32_t hash[8*8] __attribute__ ((aligned (128)));
     uint32_t vdata[24*8] __attribute__ ((aligned (64)));
     uint32_t *pdata = work->data;
     uint32_t *ptarget = work->target;
     const uint32_t first_nonce = pdata[19];
     uint32_t n = first_nonce;
     const uint32_t last_nonce = max_nonce - 8;
     __m512i  *noncev = (__m512i*)vdata + 9;  
     const uint32_t Htarg = ptarget[7];
     int thr_id = mythr->id;  
     mm512_bswap32_intrlv80_8x64( vdata, pdata );

     do
     {
        *noncev = mm512_intrlv_blend_32( mm512_bswap_32(
           _mm512_set_epi32( n+7, 0, n+6, 0, n+5, 0, n+4, 0,
                             n+3, 0, n+2, 0, n+1, 0, n,   0 ) ), *noncev );

        x15_8way_hash( hash, vdata );
        pdata[19] = n;

        for ( int i = 0; i < 8; i++ )
        if ( ( hash+(i<<3) )[7] < Htarg )
        if ( fulltest( hash+(i<<3), ptarget ) && !opt_benchmark )
        {
           pdata[19] = n+i;
           submit_lane_solution( work, hash, mythr, i );
        }
        n += 8;
     } while ( ( n < last_nonce ) && !work_restart[thr_id].restart );
     *hashes_done = n - first_nonce;
     return 0;
}

#elif defined(X15_4WAY)

typedef struct {
    blake512_4way_context   blake;
    bmw512_4way_context     bmw;
    hashState_groestl       groestl;
    skein512_4way_context   skein;
    jh512_4way_context      jh;
    keccak512_4way_context  keccak;
    luffa_2way_context      luffa;
    cubehashParam           cube;
    sph_shavite512_context  shavite;
    simd_2way_context       simd;
    hashState_echo          echo;
    hamsi512_4way_context   hamsi;
    sph_fugue512_context    fugue;
    shabal512_4way_context  shabal;
    sph_whirlpool_context   whirlpool;
} x15_4way_ctx_holder;

x15_4way_ctx_holder x15_4way_ctx __attribute__ ((aligned (64)));

void init_x15_4way_ctx()
{
     blake512_4way_init( &x15_4way_ctx.blake );
     bmw512_4way_init( &x15_4way_ctx.bmw );
     init_groestl( &x15_4way_ctx.groestl, 64 );
     skein512_4way_init( &x15_4way_ctx.skein );
     jh512_4way_init( &x15_4way_ctx.jh );
     keccak512_4way_init( &x15_4way_ctx.keccak );
     luffa_2way_init( &x15_4way_ctx.luffa, 512 );
     cubehashInit( &x15_4way_ctx.cube, 512, 16, 32 );
     sph_shavite512_init( &x15_4way_ctx.shavite );
     simd_2way_init( &x15_4way_ctx.simd, 512 );
     init_echo( &x15_4way_ctx.echo, 512 );
     hamsi512_4way_init( &x15_4way_ctx.hamsi );
     sph_fugue512_init( &x15_4way_ctx.fugue );
     shabal512_4way_init( &x15_4way_ctx.shabal );
     sph_whirlpool_init( &x15_4way_ctx.whirlpool );
};

void x15_4way_hash( void *state, const void *input )
{
     uint64_t vhash[8*4] __attribute__ ((aligned (128)));
     uint64_t hash0[8] __attribute__ ((aligned (64)));
     uint64_t hash1[8] __attribute__ ((aligned (64)));
     uint64_t hash2[8] __attribute__ ((aligned (64)));
     uint64_t hash3[8] __attribute__ ((aligned (64)));
     x15_4way_ctx_holder ctx;
     memcpy( &ctx, &x15_4way_ctx, sizeof(x15_4way_ctx) );

     // 1 Blake
     blake512_4way( &ctx.blake, input, 80 );
     blake512_4way_close( &ctx.blake, vhash );

     // 2 Bmw
     bmw512_4way( &ctx.bmw, vhash, 64 );
     bmw512_4way_close( &ctx.bmw, vhash );

     // Serial
     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );

     // 3 Groestl
     update_and_final_groestl( &ctx.groestl, (char*)hash0, (char*)hash0, 512 );
     memcpy( &ctx.groestl, &x15_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash1, (char*)hash1, 512 );
     memcpy( &ctx.groestl, &x15_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash2, (char*)hash2, 512 );
     memcpy( &ctx.groestl, &x15_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash3, (char*)hash3, 512 );

     // Parallel 4way
     intrlv_4x64( vhash, hash0, hash1, hash2, hash3, 512 );

     // 4 Skein
     skein512_4way( &ctx.skein, vhash, 64 );
     skein512_4way_close( &ctx.skein, vhash );

     // 5 JH
     jh512_4way( &ctx.jh, vhash, 64 );
     jh512_4way_close( &ctx.jh, vhash );

     // 6 Keccak
     keccak512_4way( &ctx.keccak, vhash, 64 );
     keccak512_4way_close( &ctx.keccak, vhash );

     // Serial to the end
     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );

     // 7 Luffa
     intrlv_2x128( vhash, hash0, hash1, 512 );
     luffa_2way_update_close( &ctx.luffa, vhash, vhash, 64 );
     dintrlv_2x128( hash0, hash1, vhash, 512 );
     intrlv_2x128( vhash, hash2, hash3, 512 );
     luffa_2way_init( &ctx.luffa, 512 );
     luffa_2way_update_close( &ctx.luffa, vhash, vhash, 64 );
     dintrlv_2x128( hash2, hash3, vhash, 512 );

     // 8 Cubehash
     cubehashUpdateDigest( &ctx.cube, (byte*)hash0, (const byte*) hash0, 64 );
     memcpy( &ctx.cube, &x15_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash1, (const byte*) hash1, 64 );
     memcpy( &ctx.cube, &x15_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash2, (const byte*) hash2, 64 );
     memcpy( &ctx.cube, &x15_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash3, (const byte*) hash3, 64 );

     // 9 Shavite
     sph_shavite512( &ctx.shavite, hash0, 64 );
     sph_shavite512_close( &ctx.shavite, hash0 );
     memcpy( &ctx.shavite, &x15_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash1, 64 );
     sph_shavite512_close( &ctx.shavite, hash1 );
     memcpy( &ctx.shavite, &x15_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash2, 64 );
     sph_shavite512_close( &ctx.shavite, hash2 );
     memcpy( &ctx.shavite, &x15_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash3, 64 );
     sph_shavite512_close( &ctx.shavite, hash3 );

     // 10 Simd
     intrlv_2x128( vhash, hash0, hash1, 512 );
     simd_2way_update_close( &ctx.simd, vhash, vhash, 512 );
     dintrlv_2x128( hash0, hash1, vhash, 512 );
     intrlv_2x128( vhash, hash2, hash3, 512 );
     simd_2way_init( &ctx.simd, 512 );
     simd_2way_update_close( &ctx.simd, vhash, vhash, 512 );
     dintrlv_2x128( hash2, hash3, vhash, 512 );

     // 11 Echo
     update_final_echo( &ctx.echo, (BitSequence *)hash0,
                       (const BitSequence *) hash0, 512 );
     memcpy( &ctx.echo, &x15_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash1,
                       (const BitSequence *) hash1, 512 );
     memcpy( &ctx.echo, &x15_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash2,
                       (const BitSequence *) hash2, 512 );
     memcpy( &ctx.echo, &x15_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash3,
                       (const BitSequence *) hash3, 512 );

     // 12 Hamsi parallel 4way 32 bit
     intrlv_4x64( vhash, hash0, hash1, hash2, hash3, 512 );
     hamsi512_4way( &ctx.hamsi, vhash, 64 );
     hamsi512_4way_close( &ctx.hamsi, vhash );
     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );

     // 13 Fugue
     sph_fugue512( &ctx.fugue, hash0, 64 );
     sph_fugue512_close( &ctx.fugue, hash0 );
     memcpy( &ctx.fugue, &x15_4way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash1, 64 );
     sph_fugue512_close( &ctx.fugue, hash1 );
     memcpy( &ctx.fugue, &x15_4way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash2, 64 );
     sph_fugue512_close( &ctx.fugue, hash2 );
     memcpy( &ctx.fugue, &x15_4way_ctx.fugue, sizeof(sph_fugue512_context) );
     sph_fugue512( &ctx.fugue, hash3, 64 );
     sph_fugue512_close( &ctx.fugue, hash3 );

     // 14 Shabal, parallel 32 bit
     intrlv_4x32( vhash, hash0, hash1, hash2, hash3, 512 );
     shabal512_4way_update( &ctx.shabal, vhash, 64 );
     shabal512_4way_close( &ctx.shabal, vhash );
     dintrlv_4x32( hash0, hash1, hash2, hash3, vhash, 512 );
       
     // 15 Whirlpool
     sph_whirlpool( &ctx.whirlpool, hash0, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash0 );
     memcpy( &ctx.whirlpool, &x15_4way_ctx.whirlpool,
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash1, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash1 );
     memcpy( &ctx.whirlpool, &x15_4way_ctx.whirlpool, 
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash2, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash2 );
     memcpy( &ctx.whirlpool, &x15_4way_ctx.whirlpool, 
             sizeof(sph_whirlpool_context) );
     sph_whirlpool( &ctx.whirlpool, hash3, 64 );
     sph_whirlpool_close( &ctx.whirlpool, hash3 );

     memcpy( state,    hash0, 32 );
     memcpy( state+32, hash1, 32 );
     memcpy( state+64, hash2, 32 );
     memcpy( state+96, hash3, 32 );
}

int scanhash_x15_4way( struct work *work, uint32_t max_nonce,
                       uint64_t *hashes_done, struct thr_info *mythr )
{
     uint32_t hash[4*8] __attribute__ ((aligned (128)));
     uint32_t vdata[24*4] __attribute__ ((aligned (64)));
     uint32_t *pdata = work->data;
     uint32_t *ptarget = work->target;
     const uint32_t first_nonce = pdata[19];
     uint32_t n = first_nonce;
     const uint32_t last_nonce = max_nonce - 4;
     __m256i  *noncev = (__m256i*)vdata + 9;
     const uint32_t Htarg = ptarget[7];
     int thr_id = mythr->id;  
     mm256_bswap32_intrlv80_4x64( vdata, pdata );

     do
     {
        *noncev = mm256_intrlv_blend_32( mm256_bswap_32(
              _mm256_set_epi32( n+3, 0, n+2, 0, n+1, 0, n, 0 ) ), *noncev );

         x15_4way_hash( hash, vdata );
         pdata[19] = n;

         for ( int i = 0; i < 4; i++ )
         if ( ( hash+(i<<3) )[7] < Htarg )
         if ( fulltest( hash+(i<<3), ptarget ) && !opt_benchmark )
         {
            pdata[19] = n+i;
            submit_lane_solution( work, hash, mythr, i );
         }
         n += 4;
     } while ( ( n < last_nonce ) && !work_restart[thr_id].restart );

     *hashes_done = n - first_nonce;
     return 0;
}

#endif
