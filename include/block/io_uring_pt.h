#ifndef QEMU_IO_URING_PT_H
#define QEMU_IO_URING_PT_H

#include <liburing.h>

#define IO_URING_MR_BASE        0xd0000000
#define IO_URING_MR_SIZE        (1<<20)
#define IO_URING_KICK_SIZE        1024

struct virtio_blk_iouring {
    uint64_t mr_base;
    uint64_t mr_size;
    uint64_t phy_offset;
    uint64_t sqcq_offset;
    uint64_t sqes_offset;
    uint64_t kick_offset;
    struct io_uring_params params;
};

struct BlockDriverIoUringPTParams {
    struct io_uring_params *params;
    void *sqes;
    size_t sqes_len;
    void *sqcq;
    size_t sqcq_len;
};

#endif /* QEMU_IO_URING_PT_H */
