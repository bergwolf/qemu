/*
 * Virtio blk PCI Bindings
 *
 * Copyright IBM, Corp. 2007
 * Copyright (c) 2009 CodeSourcery
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *  Paul Brook        <paul@codesourcery.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Contributions after 2012-01-13 are licensed under the terms of the
 * GNU GPL, version 2 or (at your option) any later version.
 */

#include "qemu/osdep.h"

#include "hw/qdev-properties.h"
#include "hw/virtio/virtio-blk.h"
#include "virtio-pci.h"
#include "qapi/error.h"
#include "qemu/module.h"

typedef struct VirtIOBlkPCI VirtIOBlkPCI;

/*
 * virtio-blk-pci: This extends VirtioPCIProxy.
 */
#define TYPE_VIRTIO_BLK_PCI "virtio-blk-pci-base"
#define VIRTIO_BLK_PCI(obj) \
        OBJECT_CHECK(VirtIOBlkPCI, (obj), TYPE_VIRTIO_BLK_PCI)

struct VirtIOBlkPCI {
    VirtIOPCIProxy parent_obj;
    VirtIOBlock vdev;
    MemoryRegion membar;
};

#define VIRTIO_BLK_PCI_CACHE_BAR 2

#define VIRTIO_BLK_PCI_SHMCAP_ID_IOUPT 0

static Property virtio_blk_pci_properties[] = {
    DEFINE_PROP_UINT32("class", VirtIOPCIProxy, class_code, 0),
    DEFINE_PROP_BIT("ioeventfd", VirtIOPCIProxy, flags,
                    VIRTIO_PCI_FLAG_USE_IOEVENTFD_BIT, true),
    DEFINE_PROP_UINT32("vectors", VirtIOPCIProxy, nvectors,
                       DEV_NVECTORS_UNSPECIFIED),
    DEFINE_PROP_END_OF_LIST(),
};

static void virtio_blk_pci_realize(VirtIOPCIProxy *vpci_dev, Error **errp)
{
    VirtIOBlkPCI *dev = VIRTIO_BLK_PCI(vpci_dev);
    DeviceState *vdev = DEVICE(&dev->vdev);

    if (vpci_dev->nvectors == DEV_NVECTORS_UNSPECIFIED) {
        vpci_dev->nvectors = dev->vdev.conf.num_queues + 1;
    }

    qdev_set_parent_bus(vdev, BUS(&vpci_dev->bus));
    object_property_set_bool(OBJECT(vdev), true, "realized", errp);

    if (virtio_has_feature(dev->vdev.host_features, VIRTIO_BLK_F_IO_URING)) {
        uint64_t totalsize = pow2ceil(dev->vdev.io_uring->mr_size);

        printf("totalsize %lu\n", totalsize);

        memory_region_init(&dev->membar, OBJECT(vpci_dev),
                           "virtio-blk-pci-membar", totalsize);
        memory_region_add_subregion(&dev->membar, 0, &dev->vdev.iou_mr);
        virtio_pci_add_shm_cap(vpci_dev, VIRTIO_BLK_PCI_CACHE_BAR, 0, totalsize,
                               VIRTIO_BLK_PCI_SHMCAP_ID_IOUPT);

        /* After 'realized' so the memory region exists */
        pci_register_bar(&vpci_dev->pci_dev, VIRTIO_BLK_PCI_CACHE_BAR,
                         PCI_BASE_ADDRESS_SPACE_MEMORY |
                         PCI_BASE_ADDRESS_MEM_PREFETCH |
                         PCI_BASE_ADDRESS_MEM_TYPE_64,
                         &dev->membar);
    }
}

static void virtio_blk_pci_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioPCIClass *k = VIRTIO_PCI_CLASS(klass);
    PCIDeviceClass *pcidev_k = PCI_DEVICE_CLASS(klass);

    set_bit(DEVICE_CATEGORY_STORAGE, dc->categories);
    device_class_set_props(dc, virtio_blk_pci_properties);
    k->realize = virtio_blk_pci_realize;
    pcidev_k->vendor_id = PCI_VENDOR_ID_REDHAT_QUMRANET;
    pcidev_k->device_id = PCI_DEVICE_ID_VIRTIO_BLOCK;
    pcidev_k->revision = VIRTIO_PCI_ABI_VERSION;
    pcidev_k->class_id = PCI_CLASS_STORAGE_SCSI;
}

static void virtio_blk_pci_instance_init(Object *obj)
{
    VirtIOBlkPCI *dev = VIRTIO_BLK_PCI(obj);

    virtio_instance_init_common(obj, &dev->vdev, sizeof(dev->vdev),
                                TYPE_VIRTIO_BLK);
    object_property_add_alias(obj, "bootindex", OBJECT(&dev->vdev),
                              "bootindex", &error_abort);
}

static const VirtioPCIDeviceTypeInfo virtio_blk_pci_info = {
    .base_name              = TYPE_VIRTIO_BLK_PCI,
    .generic_name           = "virtio-blk-pci",
    .transitional_name      = "virtio-blk-pci-transitional",
    .non_transitional_name  = "virtio-blk-pci-non-transitional",
    .instance_size = sizeof(VirtIOBlkPCI),
    .instance_init = virtio_blk_pci_instance_init,
    .class_init    = virtio_blk_pci_class_init,
};

static void virtio_blk_pci_register(void)
{
    virtio_pci_types_register(&virtio_blk_pci_info);
}

type_init(virtio_blk_pci_register)
