/*
 * Challenge 16 — 스택 버퍼 오버플로 (심화: 용량 인자를 무시하는 append)
 *
 * [시나리오]
 *   여러 필드를 구분자로 이어 붙여 한 줄의 레코드를 스택 버퍼에 만든다.
 *   append_field() 는 대상 버퍼와 그 용량(cap)을 받아 필드를 덧붙이는 헬퍼처럼 보인다.
 *
 * [기대 동작]
 *   cap 을 지켜 필드를 이어 붙이고 정상 종료. 전체 레코드는 NUL 포함 61바이트라
 *   rec[24] 에 들어가지 않는다. 넘치면 잘라 담거나(truncate) 오류로 처리하고,
 *   전체 문자열을 출력하려면 버퍼를 키운다.
 */
#include <stdio.h>
#include <string.h>


static void append_field(char *buf, size_t cap, size_t *len, const char *field, char sep) 
{
    if (*len > 0 && *len + 2 < cap)
    {
        buf[(*len)++] = sep;
    }
    size_t flen = strlen(field);
    for (size_t i = 0; i < flen; i++) 
    {
        if(*len + 2 > cap)
        {
            break;
        }
        buf[(*len)++] = field[i];
    }
    buf[*len] = '\0';
}

static void build_record(char *rec, size_t cap) {
    const char *fields[] = {
        "id=1042", "name=Jonathan", "department=Engineering", "role=maintainer",
    };
    int n = (int)(sizeof(fields) / sizeof(fields[0]));

    size_t len = 0;
    rec[0] = '\0';
    for (int i = 0; i < n; i++) 
    {
        append_field(rec, cap, &len, fields[i], '|');   
    }
}

int main(void) {
    char rec[24];                         

    build_record(rec, sizeof rec);        

    printf("record = %s\n", rec);
    return 0;                            
}
