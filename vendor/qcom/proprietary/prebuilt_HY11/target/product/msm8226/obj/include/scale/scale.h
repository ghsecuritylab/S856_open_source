/*============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

==============================================================================
*/
#ifndef Scale_H
#define Scale_H
#include <sys/types.h>
#include <linux/types.h>
#include <linux/msm_mdp.h>

#define PHASE_STEP_SHIFT    21
#define FIR_PIXEL_SHIFT     16
#define PHASE_RESIDUAL      (FIR_PIXEL_SHIFT - 1)
#define PCMN_PHASE_SHIFT    5
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

namespace scale {

struct scaleData {
    uint8_t chroma_sample_v[2];
    uint8_t chroma_sample_h[2];
};

class Scale {
public:
    Scale();
    virtual ~Scale();
    virtual int applyScale(struct mdp_overlay_list *list);

private:
    static struct scaleData scale;
    int scaleInit(struct mdp_overlay *left, struct mdp_overlay *right);
    bool isYuv(uint32_t format);
    int setCrop(struct mdp_overlay *left, struct mdp_overlay *right);
    int calcPhaseStep(uint32_t src, uint32_t dst, int *out_phase);
    void setChromaSample(uint32_t format, int index);
    int calcDualInitPhase(struct mdp_overlay *left, struct mdp_overlay *right);
    int calcPixelExtn(struct mdp_overlay *ov, int index);
    int calcPixelFetch(struct mdp_overlay *ov, int index);
    bool needScale(struct mdp_overlay *ov);
};

};

#endif

