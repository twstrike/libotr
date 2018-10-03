/*
 *  This file is part of the Off-the-Record Next Generation Messaging
 *  library (libotr-ng).
 *
 *  Copyright (C) 2016-2018, the libotr-ng contributors.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include "test_helpers.h"

#include "test_fixtures.h"

#include "dh.h"

static void test_dh_api() {
  dh_keypair_s alice, bob;
  otrng_dh_keypair_generate(&alice);
  otrng_dh_keypair_generate(&bob);

  dh_shared_secret_t shared1, shared2;
  size_t len1 = 0, len2 = 0;

  otrng_assert_is_success(
      otrng_dh_shared_secret(shared1, &len1, alice.priv, bob.pub));
  otrng_assert_is_success(
      otrng_dh_shared_secret(shared2, &len2, bob.priv, alice.pub));

  otrng_assert(len1 == len2);
  otrng_assert_cmpmem(shared1, shared2, len1);

  otrng_dh_keypair_destroy(&alice);
  otrng_dh_keypair_destroy(&bob);
}

static void test_dh_shared_secret() {
  uint8_t expected_secret[DH3072_MOD_LEN_BYTES - 1] = {
      0x1e, 0xe9, 0xef, 0x72, 0xaf, 0x08, 0x96, 0xd9, 0x2d, 0x1c, 0x9e, 0x7d,
      0x4b, 0x7d, 0xdf, 0x01, 0x4a, 0x8e, 0x45, 0x7f, 0x64, 0x4c, 0xa2, 0xd1,
      0x16, 0x84, 0xe2, 0xc7, 0x6e, 0x58, 0x16, 0x0d, 0xcc, 0xc3, 0x40, 0x04,
      0x76, 0x20, 0x7b, 0xeb, 0x39, 0xbe, 0x61, 0x10, 0xfe, 0xc4, 0x73, 0x8a,
      0x41, 0x00, 0x8b, 0xee, 0xc7, 0x99, 0x79, 0xa1, 0x18, 0x35, 0x40, 0x2e,
      0x98, 0x72, 0xef, 0xff, 0x10, 0x1d, 0x8a, 0x6d, 0x63, 0x69, 0x90, 0x7f,
      0x66, 0xce, 0x61, 0x7c, 0x39, 0x25, 0xef, 0xd6, 0x1f, 0x55, 0x31, 0xe3,
      0x0b, 0xb4, 0x65, 0x55, 0x86, 0xd1, 0xcb, 0x87, 0x3c, 0x0e, 0xb2, 0x6b,
      0x20, 0x40, 0x9c, 0xd9, 0xa7, 0xb0, 0xeb, 0x6c, 0x41, 0xb6, 0x61, 0x2a,
      0xa2, 0x95, 0x27, 0x8a, 0x0c, 0xd4, 0xae, 0xc1, 0xa5, 0x23, 0xcd, 0x71,
      0x17, 0xeb, 0x94, 0xeb, 0x64, 0x9f, 0x94, 0x2e, 0x59, 0x83, 0xd4, 0xd0,
      0x41, 0xb5, 0x93, 0x0b, 0x75, 0x9c, 0x67, 0x1b, 0x18, 0x91, 0xc7, 0x35,
      0xa7, 0xaf, 0x60, 0xaf, 0xc5, 0xae, 0x85, 0xa1, 0xcb, 0xe1, 0xef, 0x8e,
      0xf6, 0x43, 0x00, 0xc2, 0xb8, 0xdc, 0xd2, 0x83, 0xb3, 0x5e, 0xc4, 0xf2,
      0x75, 0x59, 0x26, 0xd6, 0x3f, 0x04, 0xd5, 0x56, 0xbf, 0x92, 0xa8, 0x89,
      0x31, 0x36, 0xb2, 0xdc, 0x6a, 0xa0, 0xc3, 0xbb, 0xb0, 0x98, 0x92, 0x4f,
      0xb9, 0x55, 0x82, 0xb1, 0xd7, 0x8f, 0x04, 0x4c, 0x88, 0xed, 0xd9, 0x9c,
      0xdb, 0x65, 0xbb, 0x46, 0x95, 0x7c, 0x9a, 0xaf, 0xac, 0xc5, 0x66, 0x3d,
      0xca, 0x66, 0xd9, 0xaa, 0x4f, 0xb5, 0x93, 0xdb, 0x4e, 0xd7, 0x1d, 0xbc,
      0x71, 0x08, 0x30, 0x3b, 0x54, 0x71, 0x22, 0x0f, 0xd3, 0x26, 0xdf, 0xf0,
      0xc1, 0xf1, 0x7b, 0xb6, 0x44, 0x40, 0x92, 0x41, 0xb8, 0x77, 0x1b, 0xa3,
      0xdd, 0x76, 0x7f, 0x9c, 0x92, 0xf0, 0x12, 0xaa, 0xe4, 0x5f, 0x80, 0x4f,
      0xc5, 0x5f, 0x03, 0xc7, 0xdd, 0xf6, 0x6a, 0xf3, 0x5c, 0x6d, 0x11, 0x2f,
      0x3f, 0x7c, 0x57, 0x90, 0x33, 0xcb, 0x1a, 0x46, 0x1c, 0xfa, 0xc4, 0x74,
      0x8b, 0x7d, 0xc3, 0x59, 0xba, 0x25, 0x90, 0x55, 0xf5, 0x7a, 0xf8, 0x73,
      0x5f, 0xe2, 0x40, 0x4d, 0x51, 0xf8, 0x00, 0xd3, 0x8c, 0x8c, 0x52, 0x75,
      0x65, 0x32, 0x62, 0xc3, 0x37, 0x48, 0x1f, 0x3a, 0xb9, 0x1e, 0xfb, 0xc2,
      0x7e, 0x32, 0x77, 0x52, 0x69, 0xa1, 0xad, 0x0f, 0x85, 0xff, 0x20, 0xda,
      0xa8, 0x59, 0x9f, 0x6d, 0x23, 0x5d, 0x74, 0xb0, 0x49, 0x87, 0x5b, 0xa2,
      0x29, 0xd1, 0x9b, 0xdc, 0xbc, 0xb2, 0x9d, 0x97, 0x68, 0xb0, 0x93, 0x5b,
      0x25, 0xe0, 0x5c, 0x0d, 0x45, 0xa6, 0xc7, 0x9c, 0x9a, 0xd8, 0x8b, 0xdf,
      0xdc, 0xca, 0x4b, 0x9b, 0x01, 0x1b, 0x09, 0xad, 0x43, 0x3d, 0x1d,
  };

  uint8_t priv[DH_KEY_SIZE] = {
      0x37, 0x17, 0x34, 0x73, 0x9b, 0xb1, 0x19, 0x47, 0xcf, 0x7a, 0x07, 0xbc,
      0x13, 0x0b, 0xbf, 0x00, 0x00, 0xb9, 0x67, 0x80, 0xc6, 0x2a, 0x5e, 0x4b,
      0x2c, 0x4a, 0x9b, 0x9c, 0x00, 0xe7, 0x97, 0x56, 0x11, 0xe0, 0x92, 0xe0,
      0xe1, 0x8c, 0x88, 0xec, 0xc5, 0xed, 0x24, 0xf4, 0x85, 0xa8, 0x17, 0x53,
      0x8c, 0x3b, 0x6b, 0x21, 0x23, 0x70, 0x30, 0x95, 0x94, 0xe2, 0xab, 0xaf,
      0x1b, 0xd0, 0xd5, 0xfd, 0x7b, 0x75, 0x6b, 0x76, 0x80, 0xdf, 0x39, 0xb0,
      0x2b, 0x41, 0x9f, 0xf6, 0xe6, 0x7f, 0x50, 0x54,
  };

  uint8_t pub[DH3072_MOD_LEN_BYTES] = {
      0xca, 0x14, 0x64, 0xe3, 0x6a, 0x5f, 0xcb, 0xe7, 0x60, 0xe6, 0x4d, 0x6a,
      0xcd, 0x20, 0x46, 0xc6, 0x85, 0x2a, 0xa3, 0x98, 0xc8, 0x4d, 0xac, 0x7a,
      0x82, 0x5a, 0x8d, 0xd8, 0x34, 0x7c, 0xc7, 0x7a, 0x46, 0x1c, 0x3f, 0xe1,
      0xa5, 0x7e, 0x21, 0x0c, 0xf3, 0xd5, 0xf3, 0x0a, 0x58, 0x5f, 0x57, 0xbc,
      0x6d, 0xe9, 0xf2, 0x4b, 0xb3, 0x93, 0xa8, 0x51, 0x90, 0x3e, 0xc9, 0x15,
      0xe4, 0xc6, 0x43, 0x76, 0xcf, 0xef, 0x6a, 0x33, 0xae, 0xf3, 0x73, 0x1d,
      0xd2, 0x05, 0x2b, 0x36, 0xe9, 0xf0, 0x1b, 0xc8, 0x0b, 0x41, 0xaa, 0xcb,
      0x21, 0xe6, 0xd1, 0xf4, 0xda, 0x34, 0xf8, 0xb4, 0x42, 0xb7, 0x9d, 0x5f,
      0x6e, 0x55, 0x52, 0x8b, 0x76, 0xd1, 0x35, 0x48, 0x78, 0x53, 0xf9, 0xf7,
      0x70, 0xff, 0x75, 0xc6, 0x73, 0xba, 0x58, 0x1a, 0xdf, 0x51, 0x69, 0xf4,
      0x5c, 0xfd, 0x04, 0xca, 0x73, 0x61, 0xaf, 0x69, 0x1c, 0x5b, 0xcf, 0xb9,
      0x87, 0x3b, 0x85, 0xa7, 0x76, 0xd5, 0x77, 0xad, 0x99, 0x43, 0x81, 0x07,
      0xe3, 0x14, 0xec, 0x86, 0xe6, 0xcf, 0xbd, 0xa2, 0x23, 0xe6, 0x7f, 0x1e,
      0x87, 0xdd, 0x6c, 0x06, 0x4b, 0x49, 0xd8, 0x3d, 0x38, 0x6e, 0xc7, 0x63,
      0x40, 0x68, 0x72, 0xa6, 0x6b, 0x64, 0xbb, 0xb1, 0x37, 0x85, 0x01, 0xcb,
      0xbf, 0x77, 0x46, 0x14, 0xdf, 0x74, 0x55, 0x52, 0x64, 0x01, 0xc4, 0xdb,
      0xd3, 0xdb, 0x8a, 0x90, 0xc8, 0xe9, 0xa2, 0xa2, 0x51, 0x3c, 0x97, 0xea,
      0x48, 0x38, 0x76, 0x0f, 0x75, 0x61, 0x30, 0x7c, 0xe6, 0x9b, 0x0a, 0x2e,
      0xe2, 0x94, 0x82, 0xe5, 0x6d, 0xe0, 0x2d, 0xa9, 0x8d, 0x40, 0x22, 0xcf,
      0xd2, 0x1a, 0x3b, 0xea, 0xa8, 0xaa, 0xbd, 0x6b, 0x8a, 0x7c, 0xd6, 0xe1,
      0xe8, 0x35, 0x66, 0x1a, 0xb7, 0xa5, 0x32, 0x0b, 0x06, 0xeb, 0xa9, 0xc1,
      0x8d, 0x7b, 0x9a, 0x5b, 0xa5, 0x9a, 0x62, 0x4e, 0x2e, 0x2f, 0xe8, 0xec,
      0xbc, 0xc1, 0xcb, 0x87, 0x79, 0x3e, 0x32, 0xae, 0x66, 0xaa, 0x52, 0xfd,
      0xa2, 0x97, 0xbd, 0x95, 0xff, 0x82, 0xfc, 0x38, 0x05, 0x45, 0x7d, 0x71,
      0x20, 0x83, 0x37, 0x3f, 0xd8, 0x71, 0xda, 0x21, 0xd7, 0x12, 0xa2, 0x3d,
      0x9a, 0x64, 0xb8, 0x6e, 0xa1, 0x0b, 0x2b, 0xb3, 0xef, 0xee, 0x37, 0xfc,
      0xb2, 0x95, 0xdc, 0x89, 0x4d, 0x0c, 0x1c, 0x99, 0x6f, 0x4a, 0xc6, 0x74,
      0x64, 0xd5, 0x13, 0x4c, 0x59, 0xf5, 0xcf, 0x6a, 0x72, 0xfe, 0xd5, 0xbe,
      0x08, 0x93, 0x74, 0x69, 0x35, 0xea, 0x82, 0x62, 0x4e, 0x0b, 0xdb, 0xd0,
      0xec, 0x1c, 0xe6, 0x1d, 0x02, 0xb4, 0xae, 0x2b, 0x16, 0xce, 0x21, 0xa6,
      0x05, 0xf5, 0x73, 0x54, 0x52, 0x7f, 0x7d, 0x3e, 0xbe, 0xf0, 0x30, 0x54,
      0xf6, 0x6f, 0xf6, 0x4e, 0x15, 0x6b, 0xc3, 0x01, 0x58, 0x7d, 0xbf, 0x9d,
  };

  dh_mpi_t priv_dh = NULL, pub_dh = NULL;
  otrng_dh_mpi_deserialize(&priv_dh, priv, DH_KEY_SIZE, NULL);
  otrng_dh_mpi_deserialize(&pub_dh, pub, DH3072_MOD_LEN_BYTES, NULL);

  dh_shared_secret_t secret;
  size_t len = 0;

  otrng_assert_is_success(
      otrng_dh_shared_secret(secret, &len, priv_dh, pub_dh));
  otrng_assert(len == 383);
  otrng_assert_cmpmem(expected_secret, secret, len);

  otrng_dh_mpi_release(priv_dh);
  otrng_dh_mpi_release(pub_dh);
}

static void test_dh_serialize() {
  uint8_t buf[DH3072_MOD_LEN_BYTES] = {0};
  dh_mpi_t mpi = gcry_mpi_new(DH3072_MOD_LEN_BITS);

  size_t mpi_len = 0;
  otrng_assert_is_success(
      otrng_dh_mpi_serialize(buf, DH3072_MOD_LEN_BYTES, &mpi_len, mpi));
  g_assert_cmpint(mpi_len, ==, 0);

  gcry_mpi_set_ui(mpi, 1);
  otrng_assert_is_success(
      otrng_dh_mpi_serialize(buf, DH3072_MOD_LEN_BYTES, &mpi_len, mpi));
  g_assert_cmpint(mpi_len, ==, 1);

  gcry_mpi_set_ui(mpi, 0xffffffff);
  otrng_assert_is_success(
      otrng_dh_mpi_serialize(buf, DH3072_MOD_LEN_BYTES, &mpi_len, mpi));
  g_assert_cmpint(mpi_len, ==, 4);

  gcry_mpi_release(mpi);

  otrng_assert_is_success(
      otrng_dh_mpi_serialize(buf, DH3072_MOD_LEN_BYTES, &mpi_len, NULL));
  g_assert_cmpint(mpi_len, ==, 0);
}

static void test_dh_keypair_destroy() {
  dh_keypair_s alice;

  otrng_dh_keypair_generate(&alice);

  otrng_assert(alice.priv);
  otrng_assert(alice.pub);

  otrng_dh_keypair_destroy(&alice);

  otrng_assert(!alice.priv);
  otrng_assert(!alice.pub);
}

void units_dh_add_tests(void) {
  g_test_add_func("/dh/api", test_dh_api);
  g_test_add_func("/dh/serialize", test_dh_serialize);
  g_test_add_func("/dh/shared-secret", test_dh_shared_secret);
  g_test_add_func("/dh/destroy", test_dh_keypair_destroy);
}
