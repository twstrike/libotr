#include <glib.h>
#include <string.h>

#include "../otrng.h"

void test_otrng_builds_query_message(otrng_fixture_t *otrng_fixture,
                                     gconstpointer data) {
  char *message = "And some random invitation text.";

  char *query_message = NULL;
  otrng_assert(otrng_build_query_message(&query_message, message,
                                         otrng_fixture->otr) == SUCCESS);

  char *expected_qm = "?OTRv4? And some random invitation text.";
  g_assert_cmpstr(query_message, ==, expected_qm);

  free(query_message);
}

void test_otrng_builds_query_message_v34(otrng_fixture_t *otrng_fixture,
                                         gconstpointer data) {
  char *message = "And some random invitation text.";

  char *query_message = NULL;
  otrng_assert(otrng_build_query_message(&query_message, message,
                                         otrng_fixture->v34) == SUCCESS);

  char *expected_qm = "?OTRv43? And some random invitation text.";
  g_assert_cmpstr(query_message, ==, expected_qm);

  free(query_message);
}

void test_otrng_builds_whitespace_tag(otrng_fixture_t *otrng_fixture,
                                      gconstpointer data) {
  char *expected_tag =
      " \t  \t\t\t\t \t \t \t    \t\t \t  And some random invitation text.";
  char *message = "And some random invitation text.";

  char *whitespace_tag = NULL;
  otrng_assert(otrng_build_whitespace_tag(&whitespace_tag, message,
                                          otrng_fixture->otr) == SUCCESS);
  g_assert_cmpstr(whitespace_tag, ==, expected_tag);
  free(whitespace_tag);
}

void test_otrng_builds_whitespace_tag_v34(otrng_fixture_t *otrng_fixture,
                                          gconstpointer data) {
  char *expected_tag = " \t  \t\t\t\t \t \t \t    \t\t \t    \t\t  \t\tAnd "
                       "some random invitation text";
  char *message = "And some random invitation text";

  char *whitespace_tag = NULL;
  otrng_assert(otrng_build_whitespace_tag(&whitespace_tag, message,
                                          otrng_fixture->v34) == SUCCESS);
  g_assert_cmpstr(whitespace_tag, ==, expected_tag);
  free(whitespace_tag);
}

void test_otrng_receives_plaintext_without_ws_tag_on_start(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  otrng_assert(otrng_receive_message(response, "Some random text.",
                                     otrng_fixture->otr) == SUCCESS);

  g_assert_cmpstr(response->to_display, ==, "Some random text.");

  otrng_response_free(response);
}

void test_otrng_receives_plaintext_without_ws_tag_not_on_start(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {
  otrng_fixture->otr->state = OTRNG_STATE_WAITING_AUTH_I;

  otrng_response_t *response = otrng_response_new();
  otrng_assert(otrng_receive_message(response, "Some random text.",
                                     otrng_fixture->otr) == SUCCESS);

  g_assert_cmpstr(response->to_display, ==, "Some random text.");
  g_assert_cmpint(response->warning, ==, OTRNG_WARN_RECEIVED_UNENCRYPTED);

  otrng_response_free(response);
}

void test_otrng_receives_plaintext_with_ws_tag(otrng_fixture_t *otrng_fixture,
                                               gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  string_t message =
      " \t  \t\t\t\t \t \t \t    \t\t \t  And some random invitation text.";

  otrng_assert(otrng_receive_message(response, message, otrng_fixture->otr) ==
               SUCCESS);
  g_assert_cmpstr(response->to_display, ==, "And some random invitation text.");
  otrng_assert(response->to_send);
  g_assert_cmpint(otrng_fixture->otr->state, ==, OTRNG_STATE_WAITING_AUTH_R);
  g_assert_cmpint(otrng_fixture->otr->running_version, ==, OTRNG_VERSION_4);

  otrng_response_free(response);
}

void test_otrng_receives_plaintext_with_ws_tag_after_text(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  string_t message =
      "Some random invitation text. \t  \t\t\t\t \t \t \t    \t\t \t  ";

  otrng_assert(otrng_receive_message(response, message, otrng_fixture->otr) ==
               SUCCESS);
  g_assert_cmpstr(response->to_display, ==, "Some random invitation text.");
  otrng_assert(response->to_send);
  g_assert_cmpint(otrng_fixture->otr->state, ==, OTRNG_STATE_WAITING_AUTH_R);
  g_assert_cmpint(otrng_fixture->otr->running_version, ==, OTRNG_VERSION_4);

  otrng_response_free(response);
}

void test_otrng_receives_plaintext_with_ws_tag_v3(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  string_t message =
      " \t  \t\t\t\t \t \t \t    \t\t  \t\tAnd some random invitation text.";
  otrng_assert(otrng_receive_message(response, message, otrng_fixture->v3) ==
               SUCCESS);

  // g_assert_cmpstr(response->to_display, ==, "And some random invitation
  // text.");
  // g_assert_cmpint(otrng_fixture->otr->state, ==,
  // OTRNG_STATE_AKE_IN_PROGRESS);
  g_assert_cmpint(otrng_fixture->v3->running_version, ==, OTRNG_VERSION_3);

  otrng_response_free(response);
}

void test_otrng_receives_query_message(otrng_fixture_t *otrng_fixture,
                                       gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  otrng_assert(otrng_receive_message(response,
                                     "?OTRv4? And some random invitation text.",
                                     otrng_fixture->otr) == SUCCESS);

  otrng_assert(response->to_send);
  g_assert_cmpint(otrng_fixture->otr->state, ==, OTRNG_STATE_WAITING_AUTH_R);
  g_assert_cmpint(otrng_fixture->otr->running_version, ==, OTRNG_VERSION_4);

  otrng_response_free(response);
}

void test_otrng_receives_query_message_v3(otrng_fixture_t *otrng_fixture,
                                          gconstpointer data) {
  otrng_response_t *response = otrng_response_new();
  otrng_assert(otrng_receive_message(response,
                                     "?OTRv3? And some random invitation text.",
                                     otrng_fixture->v3) == SUCCESS);

  g_assert_cmpint(otrng_fixture->v3->running_version, ==, OTRNG_VERSION_3);

  otrng_response_free(response);
}

void test_otrng_receives_identity_message_invalid_on_start(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {
  char *identity_message = "?OTR:";
  otrng_response_t *response = otrng_response_new();
  otrng_assert(otrng_receive_message(response, identity_message,
                                     otrng_fixture->otr) == SUCCESS);

  g_assert_cmpint(otrng_fixture->otr->state, ==, OTRNG_STATE_START);
  g_assert_cmpint(otrng_fixture->otr->running_version, ==, OTRNG_VERSION_4);
  otrng_assert(!response->to_display);
  otrng_assert(!response->to_send);

  otrng_response_free(response);
}

void test_otrng_receives_identity_message_validates_instance_tag(
    otrng_fixture_t *otrng_fixture, gconstpointer data) {

  char *message = "And some random invitation text.";

  // builds a query message
  char *query_message = NULL;
  otrng_build_query_message(&query_message, message, otrng_fixture->otr);

  // build an identity message
  otrng_response_t *id_msg = otrng_response_new();
  otrng_fixture->otr->their_instance_tag = 1;
  otrng_receive_message(id_msg, query_message, otrng_fixture->otr);
  free(query_message);

  // receive the identity message with non-zero their instance tag
  otrng_response_t *auth_msg = otrng_response_new();
  char *to_send = otrng_strdup(id_msg->to_send);
  otrng_receive_message(auth_msg, to_send, otrng_fixture->otr);
  otrng_assert(!auth_msg->to_send);

  free(to_send);
  otrng_response_free(id_msg);
  otrng_response_free(auth_msg);
}

void test_otrng_destroy() {
  otrng_client_state_t *state = otrng_client_state_new(NULL);

  otrng_policy_t policy = {.allows = OTRNG_ALLOW_V4};
  otrng_t *otr = otrng_new(state, policy);

  otr->profile = user_profile_new("4");
  otrng_destroy(otr);

  otrng_assert(otr->conversation == NULL);
  otrng_assert(otr->keys == NULL);
  otrng_assert(otr->profile == NULL);
  otrng_assert(otr->their_profile == NULL);
  otrng_assert(otr->v3_conn == NULL);

  free(otr);
  otrng_client_state_free(state);
}