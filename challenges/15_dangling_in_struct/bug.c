/*
 * Challenge 15 — 구조체에 저장된 Dangling Pointer (심화: 세션이 해제된 User 참조)
 *
 * [시나리오]
 *   로그인하면 User 객체를 힙에 만들고, Session 이 그 User 를 가리킨다. User 는 권한
 *   검사 콜백(permission)을 첫 멤버로 가진다. 요청을 처리할 때 세션의 user 를 통해
 *   권한 콜백을 호출한다.
 *
 * [기대 동작]
 *   로그인 → 요청 처리(권한 확인) → 로그아웃 순으로 정상 종료.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*PermFn)(const char *action);

typedef struct {
    PermFn permission;      
    int    uid;
    char   name[24];
} User;

typedef struct {
    User *user;             
    int   session_id;
} Session;

static int allow_all(const char *action) { (void)action; return 1; }

static User *login(int uid, const char *name) {
    User *u = malloc(sizeof *u);
    if (!u) { perror("malloc"); exit(1); }
    u->permission = allow_all;
    u->uid = uid;
    strncpy(u->name, name, sizeof(u->name) - 1);
    u->name[sizeof(u->name) - 1] = '\0';
    return u;
}

static void logout(Session *s) {
    free(s->user);      
    s->user = NULL;   
}

/* 감사 로그 항목. User 와 같은 크기라 해제된 청크를 재사용하기 쉽다. */
static char *audit_record(const char *event) {
    char *rec = malloc(sizeof(User));       
    if (!rec) exit(1);
    memset(rec, 0xAB, sizeof(User));        /* permission 자리를 0xAB.. 로 오염 */
    snprintf(rec, sizeof(User), "audit:%s", event);
    return rec;
}

static int handle_request(Session *s, const char *action) {

    return s->user->permission(action);    
}

int main(void) {
    Session s;
    s.session_id = 1;
    s.user = login(42, "alice");

    printf("first request allowed=%d\n", handle_request(&s, "read"));

    logout(&s);                      

    char *rec = audit_record("logout");      // 내부에서 s가 갖고있었던 user의 주소의 데이터에 값 넣기.
    printf("%s\n", rec);
    
    if(s.user)
        printf("second request allowed=%d\n", handle_request(&s, "write")); // 여기서 s의 user의 함수포인터 점근 시 오염된 데이터에 접근. 펑 터짐

    free(rec);
    return 0;
}
