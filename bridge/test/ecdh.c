

#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"


int main( int argc, char *argv[] )
{
  int ret = 1;
  int exit_code = MBEDTLS_EXIT_FAILURE;
  mbedtls_ecdh_context ctx_cli, ctx_srv;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  unsigned char cli_to_srv[32], srv_to_cli[32];
  const char pers[] = "ecdh";
  ((void) argc);
  ((void) argv);

  mbedtls_ecdh_init( &ctx_cli );
  mbedtls_ecdh_init( &ctx_srv );
  mbedtls_ctr_drbg_init( &ctr_drbg );

    /*
     * Initialize random number generation
     */

  mbedtls_printf( "  . Seeding the random number generator...\n" );
  fflush( stdout );
  mbedtls_entropy_init( &entropy );


  // https://tls.mbed.org/api/ctr__drbg_8h.html#ad93d675f998550b4478c1fe6f4f34ebc
  if( 
    ( ret = mbedtls_ctr_drbg_seed( 
      &ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const unsigned char *) pers,
                              sizeof pers ) ) != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
      goto exit;
  }

  mbedtls_printf( " ok\n" );

    /*
     * Client: inialize context and generate keypair
     */
  mbedtls_printf( "  . Setting up client context... \n" );
  fflush( stdout );

  ret = mbedtls_ecp_group_load( &ctx_cli.grp, MBEDTLS_ECP_DP_SECP256R1 );
  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_ecp_group_load returned %d\n", ret );
      goto exit;
  }

  ret = mbedtls_ecdh_gen_public( &ctx_cli.grp, &ctx_cli.d, &ctx_cli.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_ecdh_gen_public returned %d\n", ret );
      goto exit;
  }

  ret = mbedtls_mpi_write_binary( &ctx_cli.Q.X, cli_to_srv, 32 );
  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_mpi_write_binary returned %d\n", ret );
      goto exit;
  }

  mbedtls_printf( " ok\n" );

    /*
     * Server: initialize context and generate keypair
     */

  mbedtls_printf( "  . Setting up server context... \n" );
  fflush( stdout );
  ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_SECP256R1 );

  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_ecp_group_load returned %d\n", ret );
      goto exit;
  }

  ret = mbedtls_ecdh_gen_public( &ctx_srv.grp, &ctx_srv.d, &ctx_srv.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_ecdh_gen_public returned %d\n", ret );
      goto exit;
  }

  ret = mbedtls_mpi_write_binary( &ctx_srv.Q.X, srv_to_cli, 32 );
  if( ret != 0 )
  {
      mbedtls_printf( " failed\n  ! mbedtls_mpi_write_binary returned %d\n", ret );
      goto exit;
  }

  mbedtls_printf( " ok\n" );
  
exit:
  mbedtls_ecdh_free( &ctx_srv );
  mbedtls_ecdh_free( &ctx_cli );
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );
  mbedtls_exit( exit_code );
}