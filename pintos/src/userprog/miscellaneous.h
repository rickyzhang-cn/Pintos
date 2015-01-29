#ifndef USER_MEMORY_H
#define USER_MEMORY_H

int get_user(const uint8_t*);
bool put_user(uint8_t*, uint8_t);
void sys_exit(int);
bool valid (char* );

#endif
