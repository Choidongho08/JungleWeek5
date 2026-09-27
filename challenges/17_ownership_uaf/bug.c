/*
 * Challenge 17 — 소유권 혼동 UAF (심화: 메시지 브로커)
 *
 * [시나리오]
 *   간단한 발행/구독 브로커. 발행된 메시지(Msg: 힙에 복사된 body)를 인박스 큐에 넣고, 
 *   deliver() 가 하나씩 꺼내 구독자 콜백에 넘긴다. 구독자는 메시지를 처리하고 나서
 *   "소비했으니" free 한다. 브로커는 감사(audit)를 위해 발행 시점에 같은 Msg 포인터를
 *   log[] 에도 담아둔다.
 *
 * [기대 동작]
 *   모든 메시지를 배달·소비하고, 브로커를 종료하며 누수 없이 정리한 뒤 정상 종료.
 *
 * [예시 상황]
 *   채팅 서버
 *   hello를 발행하면, 수신함(inbox)에 넣어서 상대 앱에 푸시하고, 동시에 대화 이력(log)에도 같은 메시지를 남깁니다.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int   id;
    char *body;      /* 힙 문자열 */
} Msg;

#define QCAP 16
typedef struct {
    Msg *inbox[QCAP];   int head, tail;      /* 원형 큐 */
    Msg *log[QCAP];     int log_n;            /* 감사용: 같은 Msg 포인터를 보관 */
} Broker;

typedef void (*Subscriber)(Msg *m);

static Msg *msg_new(int id, const char *body) {
    Msg *m = malloc(sizeof *m);
    if (!m) exit(1);
    m->id = id;
    m->body = malloc(strlen(body) + 1);
    if (!m->body) exit(1);
    strcpy(m->body, body);
    return m;
}

static void msg_free(Msg *m) {
    free(m->body);
    free(m);
}

static void publish(Broker *b, int id, const char *body) {
    Msg *m = msg_new(id, body);
    b->inbox[b->tail] = m;
    b->tail = (b->tail + 1) % QCAP;
    b->log[b->log_n++] = m;
}

static void deliver(Broker *b, Subscriber sub) {
    while (b->head != b->tail) {
        Msg *m = b->inbox[b->head];
        b->head = (b->head + 1) % QCAP;
        sub(m);
    }
}

static void on_message(Msg *m) {
    printf("recv #%d: %s\n", m->id, m->body);
    msg_free(m);                         
}

static void broker_shutdown(Broker *b) {
    // for (int i = 0; i < b->log_n; i++) {
    //     msg_free(b->log[i]);
    // }
    b->log_n = 0;
}

int main(void) {
    Broker b = { .head = 0, .tail = 0, .log_n = 0 };

    publish(&b, 1, "hello");
    publish(&b, 2, "world");
    publish(&b, 3, "broker");

    deliver(&b, on_message);             

    broker_shutdown(&b);                 
    printf("done\n");
    return 0;
}
