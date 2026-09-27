/*
 * Challenge 20 — 동적 배열 성장 후 stale 원소 포인터 (심화: 히스토그램 hot 포인터)
 *
 * [시나리오]
 *   키별 빈도를 세는 히스토그램. 버킷들을 동적 배열(Histogram.data)에 담는다.
 *   자주 갱신되는 버킷 하나의 "주소"를 hot 포인터로 캐시해 두고 빠르게 증가시킨다
 *   (인덱스 재조회 없이 hot->count 만 바로 갱신하는 흔한 최적화).
 *
 * [기대 동작]
 *   스트림의 키들을 모두 반영하고, hot 버킷을 갱신한 값과 전체 합을 출력.
 *
 * [gdb 로 잡기]
 *   make gdb NAME=20_vector_stale_pointer
 *   (gdb) run                        → 크래시(SIGSEGV)
 *   (gdb) bt                         → hot->count += ... 지점
 *   (gdb) print hot                  → 캐시해 둔 옛 주소
 *   (gdb) print h.data               → 성장 후 새 기준 주소(hot 과 범위가 다름)
 *   (gdb) print (char*)hot - (char*)h.data   → hot 이 현재 버퍼 범위 밖임을 확인
 *
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int  key;
    long count;
} Bucket;

typedef struct {
    Bucket *data;
    size_t  len, cap;
} Histogram;

static void hist_grow(Histogram *h) 
{
    h->cap = h->cap ? h->cap * 2 : 16; // cap이 0이면 16 아니면 2배
    Bucket *p = realloc(h->data, h->cap * sizeof(Bucket));   /* 큰 배열은 이동(mmap 재배치) */
    if (!p) { perror("realloc"); free(h->data); exit(1); }
    h->data = p;
}

/* 키를 추가하고, 그 버킷의 주소를 돌려준다(성장이 일어날 수 있음). */
static Bucket *hist_add(Histogram *h, int key) {
    if (h->len == h->cap) 
        hist_grow(h);
    Bucket *b = &h->data[h->len++];
    b->key = key;
    b->count = 0;
    return b;
}

static long hist_total(const Histogram *h) {
    long t = 0;
    for (size_t i = 0; i < h->len; i++) t += h->data[i].count;
    return t;
}

int main(void) {
    Histogram h = { .data = NULL, .len = 0, .cap = 0 };

    for (int k = 0; k < 200000; k++) 
        hist_add(&h, k);

    int hot = 100000;
    h.data[hot].count = 1;

    for (int k = 200000; k < 600000; k++) 
        hist_add(&h, k);

    h.data[hot].count += 1000;

    printf("hot=%ld total=%ld len=%zu\n", h.data[hot].count, hist_total(&h), h.len);
    free(h.data);
    return 0;
}
