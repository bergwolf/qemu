#ifndef QEMU_IO_URING_PT_H
#define QEMU_IO_URING_PT_H

#include <liburing.h>

struct BlockDriverIoUringParams {
    struct io_uring_params params;
    uint64_t sq_ptr;
    uint64_t sq_size;
    uint64_t sqes_ptr;
    uint64_t sqes_size;
    uint64_t cq_ptr;
    uint64_t cq_size;
};

#endif /* QEMU_IO_URING_PT_H */
