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

#ifndef OTRNG_AUTH_H
#define OTRNG_AUTH_H

#include <stddef.h>

#include "ed448.h"
#include "keys.h"
#include "shared.h"

#define OTRNG_PROTOCOL_USAGE_AUTH 0x1C
#define OTRNG_PROTOCOL_DOMAIN_SEPARATION "OTRv4"

/* The size of the ring signature. */
#define RING_SIG_BYTES 6 * ED448_SCALAR_BYTES

/**
 * @brief The ring_sig_s structure represents the ring signature.
 *
 *  [c1..r3] the scalars for the signature
 */
typedef struct ring_sig_s {
  ec_scalar_p c1;
  ec_scalar_p r1;
  ec_scalar_p c2;
  ec_scalar_p r2;
  ec_scalar_p c3;
  ec_scalar_p r3;
} ring_sig_s, ring_sig_p[1];

/**
 * @brief The Authentication function of the Ring Sig.
 *
 * It produces a signature of knowledge, named sigma, bound to the
 * message msg, that demonstrates knowledge of a private key
 * corresponding to one of three public keys.
 *
 * @param [dst] The signature of knowledge
 * @param [priv] The known private key.
 * @param [pub] The public counterpart of priv.
 * @param [A1] The first public key.
 * @param [A2] The second public key.
 * @param [A3] The thrid public key.
 * @param [msg] The message to "sign".
 * @param [msg_len] The length of the message.
 *
 * @return OTRNG_SUCCESS if pub is one of (A1, A2, A3) and a signature of
 * knowledge could be created. Returns OTRNG_ERROR otherwise.
 */

INTERNAL otrng_result otrng_rsig_authenticate(
    ring_sig_p dst, const otrng_private_key_p priv,
    const otrng_public_key_p pub, const otrng_public_key_p A1,
    const otrng_public_key_p A2, const otrng_public_key_p A3,
    const uint8_t *msg, size_t msglen);

/**
 * @brief The Verification function of the Ring Sig.
 *
 * The verification function for the SoK sigma, created by rsig_authenticate.
 *
 * @param [src] The signature of knowledge
 * @param [A1] The first public key.
 * @param [A2] The second public key.
 * @param [A3] The third public key.
 * @param [msg] The message to "verify".
 * @param [msg_len] The length of the message.
 */
INTERNAL otrng_bool otrng_rsig_verify(const ring_sig_p src,
                                      const otrng_public_key_p A1,
                                      const otrng_public_key_p A2,
                                      const otrng_public_key_p A3,
                                      const uint8_t *message, size_t msglen);

/**
 * @brief Zero the values of the Ring Sig.
 *
 *
 * @param [src] The signature of knowledge
 */
INTERNAL void otrng_ring_sig_destroy(ring_sig_p src);

/**
 * @brief Calculates the 'c' parameter used in the Ring Signature.
 *
 * @param [dst] The 'c' value to be calculated.
 * @param [A1] The first public key.
 * @param [A2] The second public key.
 * @param [A3] The third public key.
 * @param [T1] The first T value.
 * @param [T2] The second T value.
 * @param [T3] The third T value.
 * @param [msg] The message to "verify".
 * @param [msg_len] The length of the message.
 */
INTERNAL void otrng_rsig_calculate_c_with_usage_and_domain(
    uint8_t usage_auth, const char *domain_sep, goldilocks_448_scalar_p dst,
    const goldilocks_448_point_p A1, const goldilocks_448_point_p A2,
    const goldilocks_448_point_p A3, const goldilocks_448_point_p T1,
    const goldilocks_448_point_p T2, const goldilocks_448_point_p T3,
    const uint8_t *message, size_t message_len);

INTERNAL void otrng_rsig_calculate_c_from_sigma_with_usage_and_domain(
    uint8_t usage, const char *domain_sep, goldilocks_448_scalar_p c,
    const ring_sig_p src, const otrng_public_key_p A1,
    const otrng_public_key_p A2, const otrng_public_key_p A3,
    const uint8_t *message, size_t message_len);

INTERNAL otrng_result otrng_rsig_authenticate_with_usage_and_domain(
    uint8_t usage, const char *domain_sep, ring_sig_p dst,
    const otrng_private_key_p secret, const otrng_public_key_p pub,
    const otrng_public_key_p A1, const otrng_public_key_p A2,
    const otrng_public_key_p A3, const uint8_t *message, size_t message_len);

INTERNAL otrng_bool otrng_rsig_verify_with_usage_and_domain(
    uint8_t usage, const char *domain_sep, const ring_sig_p src,
    const otrng_public_key_p A1, const otrng_public_key_p A2,
    const otrng_public_key_p A3, const uint8_t *message, size_t message_len);

#ifdef OTRNG_AUTH_PRIVATE
#endif

#endif
