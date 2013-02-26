//#define KCFG_ENABLE_TRACING
#define KCFG_ENABLE_MEMTRACING
//#define KCFG_LOG_FORCE_RENDER
#define KCFG_PAGE_SIZE 0x1000
#define KCFG_PML4_LOCATION 0x1000
#define KCFG_LOW_IDENTITY_PAGING_LENGTH 0x1000000
#define KCFG_HIGH_IDENTITY_PAGING_LENGTH 0x100000
#define KCFG_PAGING_POOL_START (KCFG_LOW_IDENTITY_PAGING_LENGTH + KCFG_HIGH_IDENTITY_PAGING_LENGTH)
#define KCFG_KERNEL_HEAP_START 0xfffffffff0000000
#define KCFG_KERNEL_HEAP_SIZE  0x0000000001000000