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

#include "prekey_profile.h"

#include <string.h>
#include <time.h>

#include "alloc.h"
#include "debug.h"
#include "deserialize.h"
#include "instance_tag.h"
#include "serialize.h"

INTERNAL void
otrng_prekey_profile_destroy(otrng_prekey_profile_s *destination) {
  otrng_shared_prekey_pair_free(destination->keys);
  otrng_ec_point_destroy(destination->shared_prekey);
  memset(destination->signature, 0, ED448_SIGNATURE_BYTES);
}

INTERNAL void otrng_prekey_profile_free(otrng_prekey_profile_s *destination) {
  if (!destination) {
    return;
  }

  otrng_prekey_profile_destroy(destination);
  free(destination);
}

INTERNAL void otrng_prekey_profile_copy(otrng_prekey_profile_s *destination,
                                        const otrng_prekey_profile_s *source) {
  memset(destination, 0, sizeof(otrng_prekey_profile_s));

  if (!source) {
    return;
  }

  destination->instance_tag = source->instance_tag;
  destination->expires = source->expires;

  otrng_ec_point_copy(destination->shared_prekey, source->shared_prekey);
  if (destination->keys != NULL) {
    otrng_shared_prekey_pair_free(destination->keys);
  }
  destination->keys = otrng_secure_alloc(sizeof(otrng_shared_prekey_pair_s));
  memcpy(destination->keys, source->keys, sizeof(otrng_shared_prekey_pair_s));
  memcpy(destination->signature, source->signature, ED448_SIGNATURE_BYTES);

  destination->should_publish = source->should_publish;
  destination->is_publishing = source->is_publishing;
}

tstatic size_t otrng_prekey_profile_body_serialize(
    uint8_t *destination, size_t destination_len,
    const otrng_prekey_profile_s *profile) {
  size_t w = 0;

  if (4 > destination_len - w) {
    return 0;
  }

  w += otrng_serialize_uint32(destination + w, profile->instance_tag);

  if (8 > destination_len - w) {
    return 0;
  }

  w += otrng_serialize_uint64(destination + w, profile->expires);

  if (ED448_SHARED_PREKEY_BYTES > destination_len - w) {
    return 0;
  }

  w += otrng_serialize_shared_prekey(destination + w, profile->shared_prekey);

  return w;
}

INTERNAL otrng_result otrng_prekey_profile_deserialize(
    otrng_prekey_profile_s *target, const uint8_t *buffer, size_t buflen,
    size_t *nread) {
  size_t read = 0;
  size_t w = 0;

  if (!target) {
    return OTRNG_ERROR;
  }

  if (!otrng_deserialize_uint32(&target->instance_tag, buffer + w, buflen - w,
                                &read)) {
    return OTRNG_ERROR;
  }

  w += read;

  if (!otrng_deserialize_uint64(&target->expires, buffer + w, buflen - w,
                                &read)) {
    return OTRNG_ERROR;
  }

  w += read;

  if (!otrng_deserialize_shared_prekey(target->shared_prekey, buffer + w,
                                       buflen - w, &read)) {
    return OTRNG_ERROR;
  }

  w += read;

  if (!otrng_deserialize_bytes_array(target->signature, ED448_SIGNATURE_BYTES,
                                     buffer + w, buflen - w)) {
    return OTRNG_ERROR;
  }

  w += ED448_SIGNATURE_BYTES;

  if (nread) {
    *nread = w;
  }

  return OTRNG_SUCCESS;
}

INTERNAL otrng_result otrng_prekey_profile_deserialize_with_metadata(
    otrng_prekey_profile_s *target, const uint8_t *buffer, size_t buflen,
    size_t *nread) {
  size_t read = 0;
  size_t w = 0;
  otrng_result result =
      otrng_prekey_profile_deserialize(target, buffer, buflen, &read);
  if (otrng_failed(result)) {
    return result;
  }

  w += read;

  result = otrng_deserialize_uint8(&target->should_publish, buffer + w,
                                   buflen - w, &read);

  if (otrng_failed(result)) {
    return result;
  }

  w += read;

  target->keys = otrng_secure_alloc(sizeof(otrng_shared_prekey_pair_s));
  result = otrng_deserialize_bytes_array(target->keys->sym, ED448_PRIVATE_BYTES,
                                         buffer + w, buflen - w);
  if (otrng_failed(result)) {
    return OTRNG_ERROR;
  }

  otrng_shared_prekey_pair_generate(target->keys, target->keys->sym);
  otrng_ec_point_copy(target->shared_prekey, target->keys->pub);

  w += ED448_PRIVATE_BYTES;

  if (otrng_failed(result)) {
    return result;
  }

  if (nread) {
    *nread = w;
  }

  return OTRNG_SUCCESS;
}

#define PREKEY_PROFILE_BODY_BYTES 4 + 8 + ED448_PUBKEY_BYTES

tstatic otrng_result otrng_prekey_profile_body_serialize_into(
    uint8_t **destination, size_t *nbytes,
    const otrng_prekey_profile_s *profile) {
  size_t size = PREKEY_PROFILE_BODY_BYTES;
  uint8_t *buff;
  size_t written;

  if (!destination) {
    return OTRNG_ERROR;
  }

  buff = otrng_xmalloc_z(size);

  written = otrng_prekey_profile_body_serialize(buff, size, profile);
  if (written == 0) {
    free(buff);
    return OTRNG_ERROR;
  }

  *destination = buff;

  if (nbytes) {
    *nbytes = written;
  }

  return OTRNG_SUCCESS;
}

INTERNAL otrng_result otrng_prekey_profile_serialize(
    uint8_t **destination, size_t *nbytes, otrng_prekey_profile_s *profile) {
  size_t size = PREKEY_PROFILE_BODY_BYTES + ED448_SIGNATURE_BYTES;
  uint8_t *buff = otrng_xmalloc_z(size);
  size_t written;

  written = otrng_prekey_profile_body_serialize(buff, PREKEY_PROFILE_BODY_BYTES,
                                                profile);
  if (written == 0) {
    free(buff);
    return OTRNG_ERROR;
  }

  if (size - written < ED448_SIGNATURE_BYTES) {
    free(buff);
    return OTRNG_ERROR;
  }

  written += otrng_serialize_bytes_array(buff + written, profile->signature,
                                         ED448_SIGNATURE_BYTES);

  *destination = buff;
  if (nbytes) {
    *nbytes = written;
  } else {
    return OTRNG_ERROR;
  }

  return OTRNG_SUCCESS;
}

INTERNAL otrng_result otrng_prekey_profile_serialize_with_metadata(
    uint8_t **destination, size_t *nbytes, otrng_prekey_profile_s *profile) {
  size_t size = PREKEY_PROFILE_BODY_BYTES + ED448_SIGNATURE_BYTES + 1 +
                ED448_PRIVATE_BYTES;
  uint8_t *buff = otrng_xmalloc_z(size);
  size_t written;

  written = otrng_prekey_profile_body_serialize(buff, PREKEY_PROFILE_BODY_BYTES,
                                                profile);
  if (written == 0) {
    free(buff);
    return OTRNG_ERROR;
  }

  if (size - written < ED448_SIGNATURE_BYTES) {
    free(buff);
    return OTRNG_ERROR;
  }

  written += otrng_serialize_bytes_array(buff + written, profile->signature,
                                         ED448_SIGNATURE_BYTES);

  written += otrng_serialize_uint8(buff + written, profile->should_publish);

  written += otrng_serialize_bytes_array(buff + written, profile->keys->sym,
                                         ED448_PRIVATE_BYTES);

  *destination = buff;
  if (nbytes) {
    *nbytes = written;
  } else {
    return OTRNG_ERROR;
  }

  return OTRNG_SUCCESS;
}

INTERNAL otrng_result prekey_profile_sign(
    otrng_prekey_profile_s *profile, const otrng_keypair_s *longterm_pair) {
  uint8_t *body = NULL;
  size_t bodylen = 0;
  if (!otrng_prekey_profile_body_serialize_into(&body, &bodylen, profile)) {
    return OTRNG_ERROR;
  }

  otrng_ec_sign_simple(profile->signature, longterm_pair->sym, body, bodylen);
  free(body);

  return OTRNG_SUCCESS;
}

INTERNAL otrng_prekey_profile_s *
otrng_prekey_profile_build(uint32_t instance_tag,
                           const otrng_keypair_s *longterm_pair) {
  otrng_prekey_profile_s *prekey_profile;
  time_t expires = time(NULL);

  if (!longterm_pair || !otrng_instance_tag_valid(instance_tag)) {
    return NULL;
  }

  prekey_profile = otrng_xmalloc_z(sizeof(otrng_prekey_profile_s));

  prekey_profile->keys = otrng_shared_prekey_pair_new();
  gcry_randomize(prekey_profile->keys->sym, ED448_PRIVATE_BYTES,
                 GCRY_VERY_STRONG_RANDOM);
  otrng_shared_prekey_pair_generate(prekey_profile->keys,
                                    prekey_profile->keys->sym);
  otrng_ec_point_copy(prekey_profile->shared_prekey, prekey_profile->keys->pub);

  prekey_profile->instance_tag = instance_tag;

#define PREKEY_PROFILE_EXPIRATION_SECONDS 1 * 30 * 24 * 60 * 60; /* 1 month */
  prekey_profile->expires = expires + PREKEY_PROFILE_EXPIRATION_SECONDS;

  if (!prekey_profile_sign(prekey_profile, longterm_pair)) {
    otrng_prekey_profile_free(prekey_profile);
    return NULL;
  }

  return prekey_profile;
}

static otrng_bool
otrng_prekey_profile_verify_signature(const otrng_prekey_profile_s *profile,
                                      const otrng_public_key_t pub) {
  uint8_t *body = NULL;
  size_t bodylen = 0;
  uint8_t zero_buff[ED448_SIGNATURE_BYTES];
  uint8_t pubkey[ED448_POINT_BYTES];
  otrng_bool valid;

  memset(zero_buff, 0, ED448_SIGNATURE_BYTES);

  if (memcmp(profile->signature, zero_buff, ED448_SIGNATURE_BYTES) == 0) {
    return otrng_false;
  }

  if (!otrng_prekey_profile_body_serialize_into(&body, &bodylen, profile)) {
    return otrng_false;
  }

  if (otrng_serialize_ec_point(pubkey, pub) != ED448_POINT_BYTES) {
    return otrng_false;
  }

  valid = otrng_ec_verify(profile->signature, pubkey, body, bodylen);

  free(body);
  return valid;
}

INTERNAL otrng_bool otrng_prekey_profile_expired(time_t expires) {
  return (difftime(expires, time(NULL)) <= 0);
}

INTERNAL otrng_bool otrng_prekey_profile_invalid(time_t expires,
                                                 uint64_t extra_valid_time) {
  return difftime(expires + extra_valid_time, time(NULL)) <= 0;
}

INTERNAL otrng_bool otrng_prekey_profile_valid(
    const otrng_prekey_profile_s *profile, const uint32_t sender_instance_tag,
    const otrng_public_key_t pub) {
  /* 1. Verify that the Prekey Profile signature is valid. */
  if (!otrng_prekey_profile_verify_signature(profile, pub)) {
    return otrng_false;
  }

  /* 2. Verify that the Prekey Profile owner's instance tag is equal to the
   * Sender Instance tag of the person that sent the DAKE message in which the
   * Prekey Profile is received. */
  if (sender_instance_tag != profile->instance_tag) {
    return otrng_false;
  }

  /* 3. Verify that the Prekey Profile has not expired. */
  if (otrng_prekey_profile_expired(profile->expires)) {
    return otrng_false;
  }

  /* 4. Validate that the Public Shared Prekey is on the curve Ed448-Goldilocks.
   */
  if (!otrng_ec_point_valid(profile->shared_prekey)) {
    return otrng_false;
  }

  return otrng_true;
}

API void
otrng_prekey_profile_start_publishing(otrng_prekey_profile_s *profile) {
  profile->is_publishing = otrng_true;
}

API otrng_bool
otrng_prekey_profile_should_publish(const otrng_prekey_profile_s *profile) {
  return profile->should_publish && !profile->is_publishing;
}

#ifdef DEBUG_API

#include "debug.h"

API void otrng_prekey_profile_debug_print(FILE *f, int indent,
                                          otrng_prekey_profile_s *pp) {
  if (otrng_debug_print_should_ignore("prekey_profile")) {
    return;
  }

  otrng_print_indent(f, indent);
  debug_api_print(f, "prekey_profile(");
  otrng_debug_print_pointer(f, pp);
  debug_api_print(f, ") {\n");

  otrng_print_indent(f, indent + 2);
  if (otrng_debug_print_should_ignore("prekey_profile->instance_tag")) {
    debug_api_print(f, "instance_tag = IGNORED\n");
  } else {
    debug_api_print(f, "instance_tag = %x\n", pp->instance_tag);
  }

  otrng_print_indent(f, indent + 2);
  if (otrng_debug_print_should_ignore("prekey_profile->expires")) {
    debug_api_print(f, "expires = IGNORED\n");
  } else {
    debug_api_print(f, "expires = ");
    otrng_debug_print_data(f, (uint8_t *)&(pp->expires), 8);
    debug_api_print(f, "\n");
  }

  otrng_print_indent(f, indent + 2);
  if (otrng_debug_print_should_ignore("prekey_profile->shared_prekey")) {
    debug_api_print(f, "shared_prekey = IGNORED\n");
  } else {
    debug_api_print(f, "shared_prekey = ");
    otrng_public_key_debug_print(f, pp->shared_prekey);
    debug_api_print(f, "\n");
  }

  otrng_print_indent(f, indent + 2);
  if (otrng_debug_print_should_ignore("prekey_profile->signature")) {
    debug_api_print(f, "signature = IGNORED\n");
  } else {
    debug_api_print(f, "signature = ");
    otrng_debug_print_data(f, pp->signature, ED448_SIGNATURE_BYTES);
    debug_api_print(f, "\n");
  }

  otrng_print_indent(f, indent);
  debug_api_print(f, "} // client_profile\n");
}

#endif
