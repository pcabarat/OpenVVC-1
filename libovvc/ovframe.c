#include "overror.h"
#include "ovutils.h"

#include "ovframe.h"
#include "ovframepool.h"

int
ovframe_new_ref(OVFrame **dst, OVFrame *src)
{
     if (!src) {
         return -1;
     }

    unsigned ref_count = atomic_fetch_add_explicit(&src->internal.ref_count, 1, memory_order_acq_rel);
    ov_log(NULL, OVLOG_DEBUG, "NewRef Frame %p ref_count: %d\n", src, ref_count);

    *dst = src;

    return 0;
}

void
ovframe_unref(OVFrame **frame_p)
{
    if (!frame_p)
        return;

    if (!*frame_p){
        ov_log(NULL, OVLOG_ERROR, "Trying to unref NULL frame\n");
        return;
    }

    unsigned ref_count = atomic_fetch_add_explicit(&(*frame_p)->internal.ref_count, -1, memory_order_acq_rel);
    ov_log(NULL, OVLOG_DEBUG, "Unref Frame %p ref_count: %d\n", *frame_p, ref_count);

    if (!ref_count) {
        OVFrame *frame = *frame_p;
        if (frame->pu) {
            ovpu_unref(&frame->pu);
        }
        ovframepool_release_frame(frame_p);
    }

    *frame_p = NULL;
}
