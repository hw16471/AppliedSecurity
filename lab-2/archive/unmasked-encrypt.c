/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "encrypt.h"
 
#define AES_ENC_MIX_STEP(a,b,c,d) {      \
  aes_gf28_t __a1 = s[ a ];                  \
  aes_gf28_t __b1 = s[ b ];                  \
  aes_gf28_t __c1 = s[ c ];                  \
  aes_gf28_t __d1 = s[ d ];                  \
                                             \
  aes_gf28_t __a2 = gf28_t_mulx( __a1 );     \
  aes_gf28_t __b2 = gf28_t_mulx( __b1 );     \
  aes_gf28_t __c2 = gf28_t_mulx( __c1 );     \
  aes_gf28_t __d2 = gf28_t_mulx( __d1 );     \
                                             \
  aes_gf28_t __a3 = __a1 ^ __a2;             \
  aes_gf28_t __b3 = __b1 ^ __b2;             \
  aes_gf28_t __c3 = __c1 ^ __c2;             \
  aes_gf28_t __d3 = __d1 ^ __d2;             \
                                             \
  s[a] = __a2 ^ __b3 ^ __c1 ^ __d1;          \
  s[b] = __a1 ^ __b2 ^ __c3 ^ __d1;          \
  s[c] = __a1 ^ __b1 ^ __c2 ^ __d3;          \
  s[d] = __a3 ^ __b1 ^ __c1 ^ __d2;          \
}

#define AES_ENC_SHIFT_STEP(a, b, c, d, e, f, g, h)  { \
  aes_gf28_t __a1 = s[ a ];                  \
  aes_gf28_t __b1 = s[ b ];                  \
  aes_gf28_t __c1 = s[ c ];                  \
  aes_gf28_t __d1 = s[ d ];                  \
                                             \
  s[ e ] = __a1;                             \
  s[ f ] = __b1;                             \
  s[ g ] = __c1;                             \
  s[ h ] = __d1;                             \
}

#define AES_KEY_ADD_STEP(a, b, c, d)  { \
  s[a] = s[a] ^ rk[a];                       \
  s[b] = s[b] ^ rk[b];                       \
  s[c] = s[c] ^ rk[c];                       \
  s[d] = s[d] ^ rk[d];                       \
}

#define AES_ENC_SUB_STEP(a, b, c, d)  { \
  s[a] = gf28_t_sbox(s[a]);                       \
  s[b] = gf28_t_sbox(s[b]);                       \
  s[c] = gf28_t_sbox(s[c]);                       \
  s[d] = gf28_t_sbox(s[d]);                       \
}

#define Nb 4

void aes_enc(uint8_t *c, uint8_t *m, uint8_t *k);
void aes_enc_mix_columns(aes_gf28_t *s);
void aes_enc_shift_rows(aes_gf28_t* s);
void aes_enc_sub_bytes(aes_gf28_t* s);
void aes_enc_key_add( aes_gf28_t* s, aes_gf28_t* rk );
void aes_enc_exp_step( aes_gf28_t* rk, uint8_t rc );
aes_gf28_t gf28_t_sbox( aes_gf28_t a );
aes_gf28_t gf28_t_inv( aes_gf28_t a );
aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b );
aes_gf28_t gf28_t_mulx( aes_gf28_t a );


void print_arr(aes_gf28_t *arr)  {
  for ( int i = 0; i < 16; i++)
    printf("%02X ", arr[i]);
  printf("\n\n");
}

int main( int argc, char* argv[] ) {
  // uint8_t k[ 16 ] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  //                     0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
  // uint8_t m[ 16 ] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
  //                     0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };

  // uint8_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
  //                     0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };
  uint8_t k0[ 16 ] = {128, 206, 252, 108, 120, 51, 218, 176, 138, 49, 165, 105, 4, 112, 119, 103};
  uint8_t m0[ 16 ] = {0xb7, 0x42, 0xc7, 0x58, 0x7f, 0x77, 0x3b, 0x1f, 0x34, 0x5d, 0xf7, 0xae, 0x6c, 0x1c, 0xd8, 0xb7};
  uint8_t c0[ 16 ] = {0x60, 0x55, 0xcf, 0xb, 0x37, 0x94, 0xf1, 0x1b, 0xb3, 0x17, 0xad, 0xb0, 0x90, 0x8e, 0x51, 0x3f};

  uint8_t t[ 16 ];


  for (int j = 0; j < 256; j++)  {
    for (int i = 0; i < 256; i++)  {
      k0[0]=(uint8_t)i;
      k0[13]=(uint8_t)j;
      // printf("%u\n", (uint8_t)i);
      aes_enc(t, m0, k0);
      // print_arr(t);
      if( !memcmp( t, c0, 16 * sizeof( uint8_t ) ) ) {
        printf( "AES.Enc( k, m ) == c\n" );
      }
      else {
        printf( "AES.Enc( k, m ) != c\n" );
      }
    }
  }




}



void transpose(uint8_t *arr1, uint8_t *arr2)  {
  arr1[0] = arr2[0];
  arr1[1] = arr2[4];
  arr1[2] = arr2[8];
  arr1[3] = arr2[12];

  arr1[4] = arr2[1];
  arr1[5] = arr2[5];
  arr1[6] = arr2[9];
  arr1[7] = arr2[13];

  arr1[8] = arr2[2];
  arr1[9] = arr2[6];
  arr1[10] = arr2[10];
  arr1[11] = arr2[14];

  arr1[12] = arr2[3];
  arr1[13] = arr2[7];
  arr1[14] = arr2[11];
  arr1[15] = arr2[15];

}

void aes_enc(uint8_t* c, uint8_t* m, uint8_t* k)  {
  aes_gf28_t AEC_RC[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
  aes_gf28_t rk[ 16 ], s[ 16 ];  // Declare current' round key and state matrices
  
  aes_gf28_t *rcp = AEC_RC;              // Declare pointer to the round constant
  aes_gf28_t* rkp = rk;                  // Declare pointer to rk current round key matrix


  memcpy(s, m, 16);
  memcpy(rkp, k, 16);


  aes_enc_key_add(s, rkp);


  for ( int r = 1; r <= 9; ++r )  {
    aes_enc_sub_bytes(s);
    aes_enc_shift_rows(s);
    aes_enc_mix_columns(s);
    aes_enc_exp_step(rkp, *(++rcp));
    
    aes_enc_key_add(s, rkp);


  }

  aes_enc_sub_bytes(s);
  aes_enc_shift_rows(s);
  aes_enc_exp_step(rkp, *(++rcp));

  aes_enc_key_add(s, rkp);
  memcpy(c, s, 16);
}



void aes_enc_mix_columns(aes_gf28_t *s)  {
  for ( int i = 0; i < 4; i++ , s+=4 ) 
    AES_ENC_MIX_STEP(0, 1, 2, 3);
}

void aes_enc_shift_rows(aes_gf28_t *s)  {
  AES_ENC_SHIFT_STEP(1,  5,  9, 13,
                     13,  1, 5,  9);
  AES_ENC_SHIFT_STEP(2,  6, 10, 14,
                     10, 14, 2,  6);
  AES_ENC_SHIFT_STEP(3, 7, 11, 15,
                     7, 11, 15, 3);
} 

//Performs sbox element-wise on state matrix 
void aes_enc_sub_bytes(aes_gf28_t* s)  {
  AES_ENC_SUB_STEP(0, 1, 2, 3);
  AES_ENC_SUB_STEP(4, 5, 6, 7);
  AES_ENC_SUB_STEP(8, 9, 10, 11);
  AES_ENC_SUB_STEP(12, 13, 14, 15);
  // for ( uint8_t i = 0; i < 16; ++i)
  //   s[i] = gf28_t_sbox(s[i]);
}

//Takes state matrix and a round key, and performs element-wise XOR
void aes_enc_key_add(aes_gf28_t* s, aes_gf28_t* rk )  {
  AES_KEY_ADD_STEP(0, 1, 2, 3);
  AES_KEY_ADD_STEP(4, 5, 6, 7);
  AES_KEY_ADD_STEP(8, 9, 10, 11);
  AES_KEY_ADD_STEP(12, 13, 14, 15);
  // for(uint8_t = 0; i < 16; ++i ) 
  //   s[i] ^= rk[i];
}

//Given ith round key and constant, it calculates the (i+1)th round key 
void aes_enc_exp_step(aes_gf28_t* rk, uint8_t rc)  {
  rk[0]  = rc ^ gf28_t_sbox(rk[13]) ^ rk[0];
  rk[1]  =      gf28_t_sbox(rk[14]) ^ rk[1];
  rk[2]  =      gf28_t_sbox(rk[15]) ^ rk[2];
  rk[3]  =      gf28_t_sbox(rk[12]) ^ rk[3];

  rk[4]  =      rk[0]               ^ rk[4];
  rk[5]  =      rk[1]               ^ rk[5];
  rk[6]  =      rk[2]               ^ rk[6];
  rk[7]  =      rk[3]               ^ rk[7];

  rk[8]  =      rk[4]               ^ rk[8];
  rk[9]  =      rk[5]               ^ rk[9];
  rk[10] =      rk[6]               ^ rk[10];
  rk[11] =      rk[7]               ^ rk[11];

  rk[12] =      rk[8]               ^ rk[12];
  rk[13] =      rk[9]               ^ rk[13];
  rk[14] =      rk[10]              ^ rk[14];
  rk[15] =      rk[11]              ^ rk[15];
}

//Calculates sbox of a, this is the inverse of a, followed by affine transformation, f
aes_gf28_t gf28_t_sbox( aes_gf28_t a ) {
  a = gf28_t_inv(a);

  a =   0x63     ^
      ( a      ) ^
      ( a << 1 ) ^
      ( a << 2 ) ^
      ( a << 3 ) ^
      ( a << 4 ) ^
      ( a >> 7 ) ^
      ( a >> 6 ) ^
      ( a >> 5 ) ^
      ( a >> 4 );
  return a;
}

//Calculates inverse of a
aes_gf28_t gf28_t_inv( aes_gf28_t a )  {

  //Aim is to find inverse of a, a^q = a, a^(q-1) = 1, a^(q-2) = a^(-1). q = 2^8(FF)
  aes_gf28_t t0 = gf28_t_mul(a, a); //t0 = a^2

  aes_gf28_t t1 = gf28_t_mul(t0, a); //t1 = a^3

  t0 = gf28_t_mul(t0, t0);           //t0 = a^4
  t1 = gf28_t_mul(t1, t0);           //t1 = a^7
  t0 = gf28_t_mul(t0, t0);           //t0 = a^8
  t0 = gf28_t_mul(t1, t0);           //t0 = a^15
  t0 = gf28_t_mul(t0, t0);           //t0 = a^30
  t0 = gf28_t_mul(t0, t0);           //t0 = a^60
  t1 = gf28_t_mul(t1, t0);           //t1 = a^67
  t0 = gf28_t_mul(t1, t0);           //t1 = a^127
  return gf28_t_mul(t0, t0);         //t1 = a^254 = a^(2^8)
} 

//Multiplies ff a and ff b
aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b)  {
  aes_gf28_t t = 0; // Initialise polynomial result, needs a max of 16 bits

  for ( uint8_t i = 7; i < 8; --i )  {
    t = gf28_t_mulx(t); //Multiply result by x

    if ( (b >> i) & 1 ) // Right shift polyn b by i, if there is a 1. XOR= result with a
      t ^= a;
  }
  return t;
}

//Multiplies ff a by x
aes_gf28_t gf28_t_mulx(aes_gf28_t a) {
  if( (a & 0x80) == 0x80) //If 8th bit is on, then it'll be shifted out of range, so reduce
    return (a << 1) ^ 0x1b; //0x1b is identity mod p(x), x^8 = x^4+x^3+x+1
  else 
    return (a << 1);
}


