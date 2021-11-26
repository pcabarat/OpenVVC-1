#ifndef RCN_MC_H
#define RCN_MC_H

struct RCNFunctions;

void rcn_init_mc_functions(struct RCNFunctions *const rcn_funcs);

void put_weighted_gpm_bi_pixels(uint16_t* _dst, int _dststride, const int16_t* _src0,
                  int srcstride0, const int16_t* _src1, int srcstride1, 
                  int height, int width, int step_x, int step_y, int16_t* weight);

#endif // RCN_MC_H
