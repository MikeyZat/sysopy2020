#ifndef UTIL_H
#define UTIL_H

#define S_SPACE "/space"
#define S_CREATED "/created"
#define S_PACKED "/packed"
#define S_CAN_MODIFY "/can_modify"
#define S_MEMORY "/memory"

#define LOOP 1

#define CREATORS_NUM 9
#define PACKERS_NUM 6
#define RESHIPPERS_NUM 4

#define PACKAGES_NUM 15
#define MAX_CREATED_NUM 15

typedef enum { CREATED, PACKED, SENT } package_status;

typedef struct {
    package_status status;
    int value;
} package_type;

typedef struct {
    int idx;
    int size;
    package_type packages[PACKAGES_NUM];
} memory_type;

#endif

