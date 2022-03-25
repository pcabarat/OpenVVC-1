#ifndef OV_fRAME_H
#define OV_fRAME_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include "ovdefs.h"

enum ChromaFmt
{
    OV_YUV_420_P8  = 0,
    OV_YUV_420_P10 = 1,
};

struct ColorDescription
{
    uint8_t colour_primaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coeffs;
    uint8_t full_range;
};

/* Miscelaneous information on Picture */
struct FrameInfo
{
    enum ChromaFmt chroma_format;
    struct ColorDescription color_desc;
};

struct FramePool;

/* OVFrame private data */
struct FrameInternal
{
    /* reference counter */
    atomic_uint ref_count;

    struct FramePool *frame_pool;
    void *felem;
    void *pool_elem[4];
};

struct Window
{
    uint16_t offset_lft;
    uint16_t offset_rgt;
    uint16_t offset_abv;
    uint16_t offset_blw;
};

struct Frame
{
    /* Pointer to Picture data planes per component
     */
    void *data[3];

    /* Per component line size in bytes
     */
    size_t linesize[3];

    /* Per component size in bytes of allocated plane data */
    size_t size[3];

    /* Picture width and height in samples
     */
    size_t width;
    size_t height;

    /* Picture conformance window information
     */
    struct Window output_window;

    /* Picture Order Count
     */
    uint32_t poc;

    /* Presentation Time Stamp
     * (Experimental)
     */
    uint64_t pts;

    struct FrameInfo frame_info;

    /* OVFrame Private data
     * Do not modify.
     */
    struct FrameInternal internal;

    /* Opaque Data */
    void *opaque;

    /* Reference to associated Picture Unit */
    struct OVPictureUnit *pu;
};

/* Reference an OVFrame pointer
 *
 * Add a new reference to an OVFrame pointed by src and increase its
 * internal reference counter.
 *
 * Note:
 *     - Do not call if the value pointed by dst_p already
 *     references an OVFrame
 *     - src must be a valid OVFrame (i.e. from the decoder output)
 */
int ovframe_new_ref(OVFrame **dst_p, OVFrame *src);

/* Dereference an OVFrame pointer
 *
 * Decrements reference counter of the OVFrame pointed by ovframe_p
 * and set ovframe_p to NULL. If the reference counter drops down to zero
 * the OVFrame pointed by ovframe_p will be freed.
 */
void ovframe_unref(OVFrame **frame_p);

#endif
