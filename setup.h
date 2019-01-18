typedef void (*setupcall_t)(void);
extern setupcall_t __setupcall_start, __setupcall_end;

#define __setup_call __attribute__                  \
    ((unused,__section__ (".setupcall.init"), aligned(1)))
#define __setupcall(fn)								\
	setupcall_t __setupcall_##fn __setup_call = fn
