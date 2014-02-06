/*
 * Copyright (c) 2010 Mans Rullgard <mans@mansr.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "config.h"

#include "libavutil/arm/cpu.h"
#include "libavutil/attributes.h"
#include "libavcodec/dcadsp.h"

void ff_dca_lfe_fir_vfp(float *out, const float *in, const float *coefs,
                        int decifactor, float scale);
void ff_dca_qmf_32_subbands_vfp(float samples_in[32][8], int sb_act,
                                SynthFilterContext *synth, FFTContext *imdct,
                                float synth_buf_ptr[512],
                                int *synth_buf_offset, float synth_buf2[32],
                                const float window[512], float *samples_out,
                                float raXin[32], float scale);
void ff_dca_lfe_fir_neon(float *out, const float *in, const float *coefs,
                         int decifactor, float scale);

void ff_synth_filter_float_vfp(FFTContext *imdct,
                               float *synth_buf_ptr, int *synth_buf_offset,
                               float synth_buf2[32], const float window[512],
                               float out[32], const float in[32],
                               float scale);

void ff_synth_filter_float_neon(FFTContext *imdct,
                                float *synth_buf_ptr, int *synth_buf_offset,
                                float synth_buf2[32], const float window[512],
                                float out[32], const float in[32],
                                float scale);

static void lfe_fir0_vfp(float *out, const float *in, const float *coefs,
                         float scale)
{
    ff_dca_lfe_fir_vfp(out, in, coefs, 32, scale);
}

static void lfe_fir1_vfp(float *out, const float *in, const float *coefs,
                         float scale)
{
    ff_dca_lfe_fir_vfp(out, in, coefs, 64, scale);
}

static void lfe_fir0_neon(float *out, const float *in, const float *coefs,
                          float scale)
{
    ff_dca_lfe_fir_neon(out, in, coefs, 32, scale);
}

static void lfe_fir1_neon(float *out, const float *in, const float *coefs,
                          float scale)
{
    ff_dca_lfe_fir_neon(out, in, coefs, 64, scale);
}

av_cold void ff_dcadsp_init_arm(DCADSPContext *s)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_vfp(cpu_flags) && !have_vfpv3(cpu_flags)) {
        s->lfe_fir[0]      = lfe_fir0_vfp;
        s->lfe_fir[1]      = lfe_fir1_vfp;
        s->qmf_32_subbands = ff_dca_qmf_32_subbands_vfp;
    }
    if (have_neon(cpu_flags)) {
        s->lfe_fir[0] = lfe_fir0_neon;
        s->lfe_fir[1] = lfe_fir1_neon;
    }
}

av_cold void ff_synth_filter_init_arm(SynthFilterContext *s)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_vfp(cpu_flags) && !have_vfpv3(cpu_flags))
        s->synth_filter_float = ff_synth_filter_float_vfp;
    if (have_neon(cpu_flags))
        s->synth_filter_float = ff_synth_filter_float_neon;
}
