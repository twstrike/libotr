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

#include "test_fixtures.h"
#include "persistence.h"

int dh_mpi_cmp(const dh_mpi_t m1, const dh_mpi_t m2) {
  return gcry_mpi_cmp(m1, m2);
}

otrng_s *set_up(struct otrng_client_s *client, const char *account_name,
                int byte) {
  set_up_client(client, account_name, byte);
  otrng_policy_s policy = {.allows = OTRNG_ALLOW_V3 | OTRNG_ALLOW_V4};

  return otrng_new(client, policy);
}

otrng_client_id_s create_client_id(const char *protocol, const char *account) {
  const otrng_client_id_s cid = {
      .protocol = protocol,
      .account = account,
  };
  return cid;
}

otrng_shared_session_state_s get_shared_session_state_cb(const otrng_s *conv) {
  (void)conv;
  otrng_shared_session_state_s ret = {
      .identifier1 = otrng_xstrdup("alice"),
      .identifier2 = otrng_xstrdup("bob"),
      .password = NULL,
  };

  return ret;
}

static otrng_result
get_account_and_protocol_cb(char **account_name, char **protocol_name,
                            const otrng_client_id_s client_id) {
  const char *account =
      client_id.account; // tests use client_name as client_id.

  *account_name = otrng_xstrdup(account);
  *protocol_name = otrng_xstrdup("otr");
  return OTRNG_SUCCESS;
}

void create_client_profile_cb(struct otrng_client_s *client,
                              const otrng_client_id_s client_opdata) {
  const char *allowed_versions = "34";

  // TODO: The callback probably wants to invoke
  // otrng_client_state_create_client_profile(allowed_versions);
  // to create a profile with the current instance tag and long-term-key and
  // a reasonable expiration.
  uint32_t instance_tag = otrng_client_get_instance_tag(client);
  otrng_keypair_s *keypair = otrng_client_get_keypair_v4(client);

  otrng_client_profile_s *profile = otrng_client_profile_build(
      instance_tag, allowed_versions, keypair,
      *otrng_client_get_forging_key(client),
      otrng_client_get_client_profile_exp_time(client));

  (void)client_opdata;

  if (!instance_tag || !keypair || !profile) {
    return;
  }

  otrng_client_add_client_profile(client, profile);

  otrng_client_profile_free(profile);
}

void create_prekey_profile_cb(struct otrng_client_s *client,
                              const otrng_client_id_s client_opdata) {
  otrng_prekey_profile_s *profile =
      otrng_client_build_default_prekey_profile(client);

  (void)client_opdata;

  otrng_client_add_prekey_profile(client, profile);

  otrng_prekey_profile_free(profile);
}

otrng_client_callbacks_s test_callbacks[1] = {{
    .get_account_and_protocol = &get_account_and_protocol_cb,
    .create_client_profile = &create_client_profile_cb,
    .create_prekey_profile = &create_prekey_profile_cb,
    .get_shared_session_state = &get_shared_session_state_cb,
    .create_privkey_v3 = &create_privkey_v3_cb_empty,
    .create_privkey_v4 = &create_privkey_v4_cb_empty,
    .create_forging_key = &create_forging_key_cb_empty,
    .write_expired_client_profile = &write_expired_client_profile_cb_empty,
    .write_expired_prekey_profile = &write_expired_prekey_profile_cb_empty,
    .create_shared_prekey = &create_shared_prekey_cb_empty,
    .load_privkey_v4 = &load_privkey_v4_cb_empty,
    .load_client_profile = &load_client_profile_cb_empty,
    .load_prekey_profile = &load_prekey_profile_cb_empty,
}};

otrng_public_key *
create_forging_key_from(const uint8_t sym[ED448_PRIVATE_BYTES]) {
  otrng_keypair_s *kf = otrng_keypair_new();
  otrng_keypair_generate(kf, sym);
  otrng_public_key *res = otrng_xmalloc_z(sizeof(otrng_public_key));
  otrng_ec_point_copy(*res, kf->pub);
  otrng_keypair_free(kf);
  return res;
}

void otrng_fixture_set_up(otrng_fixture_s *otrng_fixture, gconstpointer data) {
  (void)data;
  otrng_fixture->gs = otrng_global_state_new(test_callbacks, otrng_false);
  otrng_fixture->client =
      otrng_client_new(create_client_id("proto-test", "account"));
  otrng_fixture->client->global_state = otrng_fixture->gs;

  uint8_t sym[ED448_PRIVATE_BYTES] = {1}; // non-random private key on purpose
  otrng_client_add_private_key_v4(otrng_fixture->client, sym);
  const uint8_t sym2[ED448_PRIVATE_BYTES] = {
      2}; // non-random forging key on purpose
  otrng_public_key *fk = create_forging_key_from(sym2);
  otrng_client_add_forging_key(otrng_fixture->client, *fk);
  free(fk);
  otrng_client_add_shared_prekey_v4(otrng_fixture->client, sym);

  otrng_policy_s policy = {.allows = OTRNG_ALLOW_V4};
  otrng_fixture->otr = otrng_new(otrng_fixture->client, policy);

  otrng_policy_s policyv3 = {.allows = OTRNG_ALLOW_V3};
  otrng_fixture->v3 = otrng_new(otrng_fixture->client, policyv3);
  otrng_fixture->v3->v3_conn =
      otrng_v3_conn_new(otrng_fixture->client, "they_are_bob");

  otrng_policy_s policyv34 = {.allows = OTRNG_ALLOW_V3 | OTRNG_ALLOW_V4};
  otrng_fixture->v34 = otrng_new(otrng_fixture->client, policyv34);
  otrng_fixture->v34->v3_conn =
      otrng_v3_conn_new(otrng_fixture->client, "they_are_alice");

  // TODO: @refactoring This should be done automatically
  FILE *tmpFILEp = tmpfile();

  otrng_assert_is_success(
      otrng_client_private_key_v3_write_to(otrng_fixture->client, tmpFILEp));
  fclose(tmpFILEp);

  // Generate instance tag
  otrng_client_add_instance_tag(otrng_fixture->client, 0x100 + 1);
  otrng_fixture->client->client_profile =
      otrng_client_build_default_client_profile(otrng_fixture->client);
}

void otrng_fixture_teardown(otrng_fixture_s *otrng_fixture,
                            gconstpointer data) {
  (void)data;
  otrng_global_state_free(otrng_fixture->client->global_state);
  otrng_fixture->client->global_state = NULL;

  otrng_client_free(otrng_fixture->client);
  otrng_fixture->client = NULL;

  otrng_free(otrng_fixture->otr);
  otrng_fixture->otr = NULL;

  otrng_free(otrng_fixture->v3);
  otrng_fixture->v3 = NULL;

  otrng_free(otrng_fixture->v34);
  otrng_fixture->v34 = NULL;
}

void dake_fixture_setup(dake_fixture_s *f, gconstpointer user_data) {
  (void)user_data;
  f->keypair = otrng_keypair_new();

  uint8_t sym[ED448_PRIVATE_BYTES] = {1}; // non-random private key on purpose
  otrng_keypair_generate(f->keypair, sym);
  otrng_assert(otrng_ec_point_valid(f->keypair->pub));

  f->profile = client_profile_new("4");

  f->shared_prekey = otrng_shared_prekey_pair_new();
  otrng_shared_prekey_pair_generate(f->shared_prekey, sym);
  otrng_assert(otrng_ec_point_valid(f->shared_prekey->pub));

  const uint8_t fsym[ED448_PRIVATE_BYTES] = {3};
  otrng_public_key *fk = create_forging_key_from(fsym);
  otrng_ec_point_copy(f->profile->forging_pub_key, *fk);
  free(fk);

  otrng_assert(f->profile != NULL);
  f->profile->expires = time(NULL) + 60 * 60;
  otrng_assert_is_success(client_profile_sign(f->profile, f->keypair));
}

void dake_fixture_teardown(dake_fixture_s *f, gconstpointer user_data) {
  (void)user_data;
  otrng_keypair_free(f->keypair);
  f->keypair = NULL;

  otrng_shared_prekey_pair_free(f->shared_prekey);
  f->shared_prekey = NULL;

  otrng_client_profile_free(f->profile);
  f->profile = NULL;
}

void do_dake_fixture(otrng_s *alice, otrng_s *bob) {
  otrng_response_s *response_to_bob = otrng_response_new();
  otrng_response_s *response_to_alice = otrng_response_new();
  string_p query_message = NULL;
  otrng_warning warn = OTRNG_WARN_NONE;

  otrng_assert(alice->state == OTRNG_STATE_START);
  otrng_assert(bob->state == OTRNG_STATE_START);

  // Alice sends a query message
  otrng_assert_is_success(otrng_build_query_message(&query_message, "", alice));
  otrng_assert(alice->state == OTRNG_STATE_START);
  otrng_assert_cmpmem("?OTRv4", query_message, 6);

  // Bob receives a query message
  otrng_assert_is_success(
      otrng_receive_message(response_to_alice, &warn, query_message, bob));
  free(query_message);

  // Bob replies with an identity message
  otrng_assert(bob->state == OTRNG_STATE_WAITING_AUTH_R);
  otrng_assert(response_to_alice->to_display == NULL);
  otrng_assert(response_to_alice->to_send);
  otrng_assert_cmpmem("?OTR:AAQ1", response_to_alice->to_send, 9);

  // Alice receives an identity message
  otrng_assert_is_success(otrng_receive_message(
      response_to_bob, &warn, response_to_alice->to_send, alice));
  free(response_to_alice->to_send);
  response_to_alice->to_send = NULL;

  // Alice has Bob's ephemeral keys
  otrng_assert_ec_public_key_eq(alice->keys->their_ecdh,
                                bob->keys->our_ecdh->pub);
  otrng_assert_dh_public_key_eq(alice->keys->their_dh, bob->keys->our_dh->pub);
  otrng_assert_not_zero(alice->keys->ssid, sizeof(alice->keys->ssid));
  otrng_assert_not_zero(alice->keys->shared_secret, sizeof(shared_secret_t));

  // Alice replies with an auth-r message
  otrng_assert(alice->state == OTRNG_STATE_WAITING_AUTH_I);
  otrng_assert(response_to_bob->to_display == NULL);
  otrng_assert(response_to_bob->to_send);
  otrng_assert_cmpmem("?OTR:AAQ2", response_to_bob->to_send, 9);

  // Bob receives an auth-r message
  otrng_assert_is_success(otrng_receive_message(response_to_alice, &warn,
                                                response_to_bob->to_send, bob));
  free(response_to_bob->to_send);
  response_to_bob->to_send = NULL;

  // Bob has Alice's ephemeral keys
  otrng_assert_ec_public_key_eq(bob->keys->their_ecdh,
                                alice->keys->our_ecdh->pub);
  otrng_assert_dh_public_key_eq(bob->keys->their_dh, alice->keys->our_dh->pub);
  otrng_assert_not_zero(bob->keys->ssid, sizeof(alice->keys->ssid));
  otrng_assert_zero(bob->keys->shared_secret, sizeof(shared_secret_t));
  otrng_assert_not_zero(bob->keys->current->root_key, sizeof(root_key_t));

  g_assert_cmpint(bob->keys->i, ==, 0);
  g_assert_cmpint(bob->keys->j, ==, 0);
  g_assert_cmpint(bob->keys->k, ==, 0);

  // Bob replies with an auth-i message
  otrng_assert(bob->state == OTRNG_STATE_WAITING_DAKE_DATA_MESSAGE);
  otrng_assert(response_to_alice->to_display == NULL);
  otrng_assert(response_to_alice->to_send);
  otrng_assert_cmpmem("?OTR:AAQ3", response_to_alice->to_send, 9);

  // The double ratchet is initialized
  otrng_assert(bob->keys->current);

  // Alice receives an auth-i message
  otrng_assert_is_success(otrng_receive_message(
      response_to_bob, &warn, response_to_alice->to_send, alice));
  free(response_to_alice->to_send);
  response_to_alice->to_send = NULL;

  // The double ratchet is initialized
  otrng_assert(alice->keys->current);

  // Both have the same shared secret
  otrng_assert_root_key_eq(alice->keys->shared_secret,
                           bob->keys->shared_secret);

  // Alice replies with initial data message "Data-0"
  otrng_assert(alice->state == OTRNG_STATE_ENCRYPTED_MESSAGES);
  otrng_assert_cmpmem("?OTR:AAQD", response_to_bob->to_send, 9);
  otrng_assert(response_to_bob->to_display == NULL);

  g_assert_cmpint(alice->keys->i, ==, 1);
  g_assert_cmpint(alice->keys->j, ==, 1);
  g_assert_cmpint(alice->keys->k, ==, 0);

  // Bob receives the initial data message
  otrng_assert_is_success(otrng_receive_message(response_to_alice, &warn,
                                                response_to_bob->to_send, bob));
  free(response_to_bob->to_send);
  response_to_bob->to_send = NULL;

  otrng_assert(bob->state == OTRNG_STATE_ENCRYPTED_MESSAGES);
  otrng_assert(response_to_alice->to_send == NULL);
  otrng_assert(response_to_alice->to_display == NULL);

  g_assert_cmpint(otrng_list_len(bob->keys->old_mac_keys), ==, 1);
  g_assert_cmpint(bob->keys->i, ==, 1);
  g_assert_cmpint(bob->keys->j, ==, 0);
  g_assert_cmpint(bob->keys->k, ==, 1);
  g_assert_cmpint(bob->keys->pn, ==, 0);

  otrng_response_free(response_to_alice);
  otrng_response_free(response_to_bob);
}

otrng_bool test_should_not_heartbeat(int last_sent) {
  (void)last_sent;
  return otrng_false;
}

void set_up_client(otrng_client_s *client, const char *account_name, int byte) {
  client->global_state = otrng_global_state_new(test_callbacks, otrng_false);

  // TODO: REMOVE after updating every otrng_client_state_new(NULL)
  client->client_id = create_client_id("proto-test", account_name);

  uint8_t long_term_priv[ED448_PRIVATE_BYTES] = {byte + 0xA};
  uint8_t forging_sym[ED448_PRIVATE_BYTES] = {byte + 0xD};
  uint8_t shared_prekey_priv[ED448_PRIVATE_BYTES] = {byte + 0xF};

  otrng_client_add_private_key_v4(client, long_term_priv);
  otrng_public_key *fk = create_forging_key_from(forging_sym);
  otrng_client_add_forging_key(client, *fk);
  free(fk);
  otrng_client_add_shared_prekey_v4(client, shared_prekey_priv);
  otrng_client_add_instance_tag(client, 0x100 + byte);

  client->client_profile = otrng_client_build_default_client_profile(client);

  client->should_heartbeat = test_should_not_heartbeat;
}

void free_message_and_response(otrng_response_s *response, string_p *message) {
  otrng_response_free(response);
  free(*message);
  *message = NULL;
}

otrng_result
get_account_and_protocol_cb_empty(char **account, char **protocol,
                                  const struct otrng_client_id_s client_id) {
  (void)account;
  (void)protocol;
  (void)client_id;
  return OTRNG_SUCCESS;
}

void create_privkey_v3_cb_empty(const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}

void create_privkey_v4_cb_empty(const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}

void create_forging_key_cb_empty(const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}

void create_client_profile_cb_empty(
    struct otrng_client_s *client,
    const struct otrng_client_id_s client_opdata) {
  (void)client;
  (void)client_opdata;
}

void write_expired_client_profile_cb_empty(
    struct otrng_client_s *client,
    const struct otrng_client_id_s client_opdata) {
  (void)client;
  (void)client_opdata;
}

void create_prekey_profile_cb_empty(
    struct otrng_client_s *client,
    const struct otrng_client_id_s client_opdata) {
  (void)client;
  (void)client_opdata;
}

void write_expired_prekey_profile_cb_empty(
    struct otrng_client_s *client,
    const struct otrng_client_id_s client_opdata) {
  (void)client;
  (void)client_opdata;
}

void create_shared_prekey_cb_empty(
    struct otrng_client_s *client,
    const struct otrng_client_id_s client_opdata) {
  (void)client;
  (void)client_opdata;
}

otrng_shared_session_state_s
get_shared_session_state_cb_empty(const struct otrng_s *conv) {
  otrng_shared_session_state_s result;
  result.identifier1 = otrng_xstrdup("one");
  result.identifier2 = otrng_xstrdup("two");
  result.password = otrng_xstrdup("three");
  (void)conv;
  return result;
}

void load_privkey_v4_cb_empty(const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}

void load_client_profile_cb_empty(
    const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}

void load_prekey_profile_cb_empty(
    const struct otrng_client_id_s client_opdata) {
  (void)client_opdata;
}
