#include "ovunits.h"
#include "ovmem.h"
#include "ovutils.h"

static void ovnalu_free(OVNALUnit **nalu_p);

int
ov_nalu_init(OVNALUnit *nalu)
{
    nalu->rbsp_data = NULL;
    nalu->rbsp_size = 0;

    nalu->epb_pos = NULL;
    nalu->nb_epb = 0;
    nalu->release = ovnalu_free;

    atomic_init(&nalu->ref_count, 0);
    return 1;
}

int
ov_nalu_new_ref(OVNALUnit **nalu_p, OVNALUnit *nalu)
{
    if (!nalu) {
        return -1;
    }

    atomic_fetch_add_explicit(&nalu->ref_count, 1, memory_order_acq_rel);

    *nalu_p = nalu;

    return 0;
}

static void
ovnalu_free(OVNALUnit **nalu_p)
{
    OVNALUnit *nalu = *nalu_p;
    ov_freep(&nalu->rbsp_data);
    if (nalu->epb_pos) {
        ov_freep(&nalu->epb_pos);
    }
    ov_freep(nalu_p);
}

void
ov_nalu_unref(OVNALUnit **nalu_p)
{
    if (!nalu_p)
        return;

    OVNALUnit *nalu = *nalu_p;

    if (!nalu){
        ov_log(NULL, OVLOG_ERROR, "Trying to unref NULL nalu\n");
        return;
    }

    unsigned ref_count = atomic_fetch_add_explicit(&nalu->ref_count, -1, memory_order_acq_rel);

    if (!ref_count) {
        nalu->release(nalu_p);
    }

    *nalu_p = NULL;
}

int
ovnalu_init(OVNALUnit **nalu_p, uint8_t *rbsp_data, uint32_t *epb_offset, size_t rbsp_size,
            uint32_t nb_epb, uint8_t nalu_type, void (*release_callback)(struct OVNALUnit **))
{
    OVNALUnit *nalu = *nalu_p;
    nalu->rbsp_data = rbsp_data;
    nalu->rbsp_size = rbsp_size;

    nalu->epb_pos = epb_offset;
    nalu->nb_epb = 0;

    if (release_callback) {
        nalu->release = release_callback;
    } else {
        nalu->release = ovnalu_free;
    }

    atomic_init(&nalu->ref_count, 0);
    return 0;
}

/*FIXME Add and reference counting */
void
ov_free_pu(OVPictureUnit **ovpu_p)
{
    OVPictureUnit *ovpu = *ovpu_p;

    if (ovpu) {
        int i;
        for (i = 0; i < ovpu->nb_nalus; ++i) {
            OVNALUnit *nalu = ovpu->nalus[i];

            ov_nalu_unref(&nalu);
        }
        ov_free(ovpu->nalus);
    }

    ov_freep(ovpu_p);
}
