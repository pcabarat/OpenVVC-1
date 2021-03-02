#include <stdlib.h>

#include "vcl.h"
#include "cabac_internal.h"
#include "ctudec.h"
#include "nvcl_structures.h"

//TODO: change name and location of definition
// #define VVC_SAO_LUMA_SLICE_FLAG     (1 << 0)
// #define VVC_SAO_CHROMA_SLICE_FLAG   (1 << 1)
// #define VVC_ALF_LUMA_SLICE_FLAG     (1 << 2)

void
ovcabac_read_ae_sao_ctu( OVCTUDec *const ctudec, const OVPS *const prms)
{   
    uint8_t sao_luma_flag   =  prms->sh->sh_sao_luma_used_flag;
    uint8_t sao_chroma_flag =  prms->sh->sh_sao_chroma_used_flag;
    if(sao_luma_flag || sao_chroma_flag)
        // loop_filter_state_flags & (VVC_SAO_LUMA_SLICE_FLAG|VVC_SAO_CHROMA_SLICE_FLAG))
    {
        //TODO: creer le tableau de SAOParams qui va etre utilise
        SAOParams *sao_ctu = malloc(sizeof(SAOParams));
        // SAOParams* sao_ctu = &vvc_ctx->sao[ctb_rs];
        OVCABACCtx *const cabac_ctx = ctudec->cabac_ctx;
        uint64_t *const cabac_state = cabac_ctx->ctx_table;
        const uint8_t ctu_neighbour_flags = ctudec->ctu_ngh_flags;
        uint8_t val = ovcabac_read_ae_sao_merge_type(cabac_ctx, cabac_state, ctu_neighbour_flags);
        // val = vvc_decode_sao_merge_type(cabac_ctx, cabac_state, ctu_neighbour_flags);

        if (!val) {
            //TODO: calculer les vraies valeurs dans sps
            uint8_t num_bits_sao   = 31;
            uint8_t num_bits_sao_c = 31;
            // uint8_t sao_trunc_bitdepth   = vvc_ctx->sao_truncated_bitdepth;
            // uint8_t sao_trunc_bitdepth_c = vvc_ctx->sao_truncated_bitdepth_chroma;
            // ff_vvc_decode_sao_type_idx(cabac_ctx,cabac_state,
            //                                  loop_filter_state_flags,
            //                                  sao_trunc_bitdepth,
            //                                  sao_trunc_bitdepth_c,sao_ctu);
            ovcabac_read_ae_sao_type_idx(cabac_ctx, cabac_state, sao_ctu, sao_luma_flag, sao_chroma_flag, num_bits_sao, num_bits_sao_c);
        }
        else{
            //TODO: gerer cas MERGE
            // if (val == SAO_MERGE_LEFT)
            // {
            //     int ctb_left = ctb_rs-1; 
            //     *sao_ctu = vvc_ctx->sao[ctb_left];
            // }
            // if (val == SAO_MERGE_ABOVE)
            // {
            //     int ctb_above = ctb_rs-vvc_ctx->nb_ctu_w; 
            //     *sao_ctu = vvc_ctx->sao[ctb_above];
            // }
            // sao_ctu->type_idx[0]=sao_ctu->old_type_idx[0];
            // sao_ctu->type_idx[1]=sao_ctu->old_type_idx[1];
            // sao_ctu->type_idx[2]=sao_ctu->old_type_idx[2];
        }
    }
}



uint8_t
ovcabac_read_ae_sao_merge_type(OVCABACCtx *const cabac_ctx, uint64_t *const cabac_state,
                          uint8_t neighbour_flags)
{
    uint8_t sao_merge_type = 0;

    if(neighbour_flags & CTU_LFT_FLG){
        sao_merge_type = ovcabac_ae_read(cabac_ctx,&cabac_state[SAO_MERGE_FLAG_CTX_OFFSET]);
    }

    if(!sao_merge_type && neighbour_flags & CTU_UP_FLG ){
        sao_merge_type = ovcabac_ae_read(cabac_ctx,&cabac_state[SAO_MERGE_FLAG_CTX_OFFSET]);
        sao_merge_type = sao_merge_type << 1;
    }

    return sao_merge_type;
}

void
ovcabac_read_ae_sao_type_idx(OVCABACCtx *const cabac_ctx, uint64_t *const cabac_state, SAOParams *sao_ctu,
                        uint8_t sao_luma_flag, uint8_t sao_chroma_flag, uint8_t num_bits_sao, uint8_t num_bits_sao_c)
{
    //TODO: creer le tableau de SAOParams qui va etre utilise
    SAOParams *sao = malloc(sizeof(SAOParams));
    int k;
    int i;

    // if(sao_flags & VVC_SAO_LUMA_SLICE_FLAG){
    if(sao_luma_flag){
        if(ovcabac_ae_read(cabac_ctx,&cabac_state[SAO_TYPE_IDX_CTX_OFFSET])){
            sao->type_idx[0] = ovcabac_bypass_read(cabac_ctx) ? SAO_EDGE : SAO_BAND;
            sao->old_type_idx[0] = sao->type_idx[0];
            //offsets
            for (i = 0; i < 4; i++){
                 //not 5
                for(  k = 0; k < num_bits_sao; k++ ) {
                    if( !ovcabac_bypass_read(cabac_ctx) ){
                        break;
                    }
                }
                sao->offset_abs[0][i] = k;
            }

            if(sao->type_idx[0] & SAO_BAND) {
                for( k = 0; k < 4; k++ ) {
                  if(sao->offset_abs[0][k] && ovcabac_bypass_read(cabac_ctx)){
                      sao->offset_sign[0][k]    = 1;
                      sao->offset_val[0][k]     = -sao->offset_abs[0][k];
                  } else {
                      sao->offset_sign[0][k]    = 0;
                      sao->offset_val[0][k]     = sao->offset_abs[0][k];
                  }

                } // band position
                sao->band_position[0] = 0;
                for(i=1; i < 6; i++)
                    sao->band_position[0] |= ovcabac_bypass_read(cabac_ctx)<<(5-i);
            } else {    //edge class 0; 1, 2, 3
                sao->eo_class[0] = ovcabac_bypass_read(cabac_ctx)<<1;
                sao->eo_class[0] |= ovcabac_bypass_read(cabac_ctx);

                sao->offset_val[0][ 0 ] =  sao->offset_abs[0][0];
                sao->offset_val[0][ 1 ] =  sao->offset_abs[0][1];
                sao->offset_val[0][ 2 ] =  0;
                sao->offset_val[0][ 3 ] = -sao->offset_abs[0][2];
                sao->offset_val[0][ 4 ] = -sao->offset_abs[0][3];
            }
        }
    }

    // if(sao_flags & VVC_SAO_CHROMA_SLICE_FLAG){
    if(sao_chroma_flag){
        if(ovcabac_ae_read(cabac_ctx,&cabac_state[SAO_TYPE_IDX_CTX_OFFSET])){
            sao->type_idx[2] = sao->type_idx[1] = ovcabac_bypass_read(cabac_ctx) ? SAO_EDGE : SAO_BAND;
            sao->old_type_idx[1] = sao->type_idx[1];
            sao->old_type_idx[2] = sao->type_idx[2];
            //offsets
            for (i = 0; i < 4; i++){
                //not 5
                for(  k = 0; k < num_bits_sao_c; k++ ) {
                    if( !ovcabac_bypass_read(cabac_ctx) ){
                        break;
                    }
                }
                sao->offset_abs[1][i] = k;
            }//cb
            if(sao->type_idx[1] & SAO_BAND) {
                for( k = 0; k < 4; k++ ) {
                    if(sao->offset_abs[1][k] && ovcabac_bypass_read(cabac_ctx)){
                        sao->offset_sign[1][k]    = 1;
                        sao->offset_val[1][k]     = -sao->offset_abs[1][k];
                    } else {
                        sao->offset_sign[1][k]    = 0;
                        sao->offset_val[1][k]     = sao->offset_abs[1][k];
                    }
                }//band position
                sao->band_position[1] = 0;
                for(i=1; i < 6; i++)
                    sao->band_position[1] |= ovcabac_bypass_read(cabac_ctx)<<(5-i);
            } else {//edge
                sao->eo_class[1] = ovcabac_bypass_read(cabac_ctx)<<1;
                sao->eo_class[1] |= ovcabac_bypass_read(cabac_ctx);
                sao->offset_val[1][ 0 ] =  sao->offset_abs[1][0];
                sao->offset_val[1][ 1 ] =  sao->offset_abs[1][1];
                sao->offset_val[1][ 2 ] =  0;
                sao->offset_val[1][ 3 ] = -sao->offset_abs[1][2];
                sao->offset_val[1][ 4 ] = -sao->offset_abs[1][3];
            }

            //cr
            for (i = 0; i < 4; i++){
                //not 5
                for(  k = 0; k < num_bits_sao_c; k++ ) {
                    if( !ovcabac_bypass_read(cabac_ctx) ){
                        break;
                    }
                }
                sao->offset_abs[2][i] = k;
            }
            if(sao->type_idx[2] & SAO_BAND){
                for( k = 0; k < 4; k++ ) {
                  if(sao->offset_abs[2][k] && ovcabac_bypass_read(cabac_ctx)){
                      sao->offset_sign[2][k]    = 1;
                      sao->offset_val[2][k]     = -sao->offset_abs[2][k];
                  } else {
                      sao->offset_sign[2][k]    = 0;
                      sao->offset_val[2][k]     = sao->offset_abs[2][k];
                  }
                }   //band position
                sao->band_position[2] = 0;
                for(i=1; i < 6; i++)
                    sao->band_position[2] |= ovcabac_bypass_read(cabac_ctx)<<(5-i);
            }
            else{
                sao->eo_class[2] = sao->eo_class[1];

                sao->offset_val[2][ 0 ] =  sao->offset_abs[2][0];
                sao->offset_val[2][ 1 ] =  sao->offset_abs[2][1];
                sao->offset_val[2][ 2 ] =  0;
                sao->offset_val[2][ 3 ] = -sao->offset_abs[2][2];
                sao->offset_val[2][ 4 ] = -sao->offset_abs[2][3];
            }
        }
    }
}
