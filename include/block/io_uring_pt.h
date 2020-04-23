#ifndef QEMU_IO_URING_PT_H
#define QEMU_IO_URING_PT_H

#include <liburing.h>

#define VIRTIO_BLK_F_IO_URING	15
#define VIRTIO_BLK_F_KTHREAD    16

#define IO_URING_MR_BASE        0x100000000
#define IO_URING_MR_SIZE        (1<<20)
#define IO_URING_KICK_SIZE        1024

#ifndef VIRTIO_BLK_ID_BYTES
#define VIRTIO_BLK_ID_BYTES	20	/* ID string length */
#endif

struct virtio_blk_iouring_enter {
    unsigned int to_submit;
    unsigned int min_complete;
    unsigned int flags;
};

#define VBI_F_BLKDEV            0x0001llu
#define VBI_F_USERSPACE         0x0002llu

struct virtio_blk_iouring {
    uint64_t flags;
    uint64_t mr_base;
    uint64_t mr_size;
    uint64_t phy_offset;
    uint64_t sqcq_offset;
    uint64_t sqes_offset;
    uint64_t kick_offset;
    struct io_uring_params params;
    struct virtio_blk_iouring_enter enter;
    char serial_id[VIRTIO_BLK_ID_BYTES];
};

struct BlockDriverIoUringPTParams {
    struct io_uring_params *params;
    void *sqes;
    size_t sqes_len;
    void *sqcq;
    size_t sqcq_len;
    bool blkdev;
};

struct BlockDriverIoUringPTKick {
    struct virtio_blk_iouring_enter enter;
};
#endif /* QEMU_IO_URING_PT_H */
