/*
 * Challenge 18 — 에러 처리(goto cleanup) 경로의 Double Free (심화: 다자원 래더)
 *
 * [시나리오]
 *   연결(Conn)을 열며 여러 자원을 순서대로 확보한다: 수신 버퍼(rx) → 송신 버퍼(tx)
 *   → 세션 상태(state). 확보 도중 실패하면 goto 라벨 사다리로 "역순 정리"한다.
 *   마지막에 핸드셰이크 검증을 수행하고, 실패하면 역시 정리 경로로 빠진다.
 *
 * [예시 상황]
 *  TCP 소켓 + TLS (HTTPS)
 *  소켓을 열고, 읽기/쓰기 버퍼를 잡고, SSL 세션을 만든 뒤 SSL_do_handshake()로 인증서를 확인. 
 *  핸드셰이크가 실패하면 소켓·SSL 객체·버퍼를 역순으로 닫음. handshake_ok가 바로 이 단계.
 *
 * [기대 동작]
 *   각 자원을 확보한 만큼만, 정확히 한 번씩 해제하고 실패 코드를 반환.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *rx;
    char *tx;
    int  *state;
} Conn;

static int handshake_ok(const Conn *c) {
    (void)c;
    return 0;                     /* 실패 */
}

static int conn_open(Conn *c, size_t bufsz) {
    c->rx = c->tx = NULL;
    c->state = NULL;

    c->rx = malloc(bufsz);
    if (!c->rx) goto fail_rx;

    c->tx = malloc(bufsz);
    if (!c->tx) goto fail_tx;

    c->state = malloc(sizeof(int) * 4);
    if (!c->state) goto fail_state;

    strcpy(c->rx, "rx-ready");
    strcpy(c->tx, "tx-ready");
    for (int i = 0; i < 4; i++) c->state[i] = i;

    if (!handshake_ok(c)) {

        // free(c->tx);
        // c->tx = NULL;
        goto fail_tx;             
    }

    return 0;                     

fail_state:
fail_tx:
fail_rx:
    free(c->state);
    c->state = NULL;
    free(c->tx);
    c->tx = NULL;             
    free(c->rx);
    c->rx = NULL;
    return -1;
}

int main(void) {
    Conn c;
    int rc = conn_open(&c, 32);   
    printf("conn_open rc=%d\n", rc);
    return 0;
}
