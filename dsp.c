//
//  dsp.c
//  deadbeef
//
//  Created by Oleksiy Yakovenko on 09/02/2017.
//  Copyright © 2017 Alexey Yakovenko. All rights reserved.
//

#include <string.h>
#include <limits.h>
#include <errno.h>
#include "deadbeef.h"
#include "dsp.h"
#include "streamer.h"
#include "messagepump.h"
#include "plugins.h"
#include "conf.h"
#include "premix.h"

static ddb_dsp_context_t *dsp_chain;
static DB_dsp_t *eqplug;
static ddb_dsp_context_t *eq;
static int dsp_on = 0;

static char *dsp_input_buffer;
static int dsp_input_buffer_size;

static char *dsp_temp_buffer;
static int dsp_temp_buffer_size;

// how much bigger should read-buffer be to allow upsampling.
// e.g. 8000Hz -> 192000Hz upsampling requires 24x buffer size,
// so if we originally request 4096 bytes blocks -
// that will require 24x buffer size, which is 98304 bytes buffer
#define MAX_DSP_RATIO 24

void
streamer_dsp_postinit (void);

static void
free_dsp_buffers (void);

void
dsp_free (void) {
    dsp_chain_free (dsp_chain);
    dsp_chain = NULL;

    free_dsp_buffers ();

    eqplug = NULL;
    eq = NULL;
}

void
dsp_reset (void) {
    // reset dsp
    ddb_dsp_context_t *dsp = dsp_chain;
    while (dsp) {
        if (dsp->plugin->reset) {
            dsp->plugin->reset (dsp);
        }
        dsp = dsp->next;
    }
}

static char *
ensure_dsp_input_buffer (int size) {
    if (!size) {
        if (dsp_input_buffer) {
            free (dsp_input_buffer);
            dsp_input_buffer = NULL;
        }
        return 0;
    }
    if (size != dsp_input_buffer_size) {
        dsp_input_buffer = realloc (dsp_input_buffer, size);
        dsp_input_buffer_size = size;
    }
    return dsp_input_buffer;
}

static char *
ensure_dsp_temp_buffer (int size) {
    if (!size) {
        if (dsp_temp_buffer) {
            free (dsp_temp_buffer);
            dsp_temp_buffer = NULL;
        }
        return NULL;
    }
    if (size != dsp_temp_buffer_size) {
        dsp_temp_buffer = realloc (dsp_temp_buffer, size);
        dsp_temp_buffer_size = size;
    }
    return dsp_temp_buffer;
}

static void
free_dsp_buffers (void) {
    ensure_dsp_input_buffer (0);
    ensure_dsp_temp_buffer (0);
}

ddb_dsp_context_t *
streamer_get_dsp_chain (void) {
    return dsp_chain;
}

ddb_dsp_context_t *
dsp_clone (ddb_dsp_context_t *from) {
    ddb_dsp_context_t *dsp = from->plugin->open ();
    char param[2000];
    if (from->plugin->num_params) {
        int n = from->plugin->num_params ();
        for (int i = 0; i < n; i++) {
            from->plugin->get_param (from, i, param, sizeof (param));
            dsp->plugin->set_param (dsp, i, param);
        }
    }
    dsp->enabled = from->enabled;
    return dsp;
}

void
streamer_set_dsp_chain_real (ddb_dsp_context_t *chain) {
    dsp_chain_free (dsp_chain);
    dsp_chain = chain;
    eq = NULL;
    streamer_dsp_postinit ();

    streamer_dsp_chain_save();
    streamer_reset (1);

    messagepump_push (DB_EV_DSPCHAINCHANGED, 0, 0, 0);
}

void
dsp_chain_free (ddb_dsp_context_t *dsp_chain) {
    while (dsp_chain) {
        ddb_dsp_context_t *next = dsp_chain->next;
        dsp_chain->plugin->close (dsp_chain);
        dsp_chain = next;
    }
}

ddb_dsp_context_t *
streamer_dsp_chain_load (const char *fname) {
    int err = 1;
    FILE *fp = fopen (fname, "rt");
    if (!fp) {
        return NULL;
    }

    char temp[100];
    ddb_dsp_context_t *chain = NULL;
    ddb_dsp_context_t *tail = NULL;
    for (;;) {
        // plugin enabled {
        int enabled = 0;
        int err = fscanf (fp, "%99s %d {\n", temp, &enabled);
        if (err == EOF) {
            break;
        }
        else if (2 != err) {
            fprintf (stderr, "error plugin name\n");
            goto error;
        }

        DB_dsp_t *plug = (DB_dsp_t *)plug_get_for_id (temp);
        if (!plug) {
            fprintf (stderr, "streamer_dsp_chain_load: plugin %s not found. preset will not be loaded\n", temp);
            goto error;
        }
        ddb_dsp_context_t *ctx = plug->open ();
        if (!ctx) {
            fprintf (stderr, "streamer_dsp_chain_load: failed to open ctxance of plugin %s\n", temp);
            goto error;
        }

        if (tail) {
            tail->next = ctx;
            tail = ctx;
        }
        else {
            tail = chain = ctx;
        }

        int n = 0;
        for (;;) {
            char value[1000];
            if (!fgets (temp, sizeof (temp), fp)) {
                fprintf (stderr, "streamer_dsp_chain_load: unexpected eof while reading plugin params\n");
                goto error;
            }
            if (!strcmp (temp, "}\n")) {
                break;
            }
            else if (1 != sscanf (temp, "\t%1000[^\n]\n", value)) {
                fprintf (stderr, "streamer_dsp_chain_load: error loading param %d\n", n);
                goto error;
            }
            if (plug->num_params) {
                plug->set_param (ctx, n, value);
            }
            n++;
        }
        ctx->enabled = enabled;
    }

    err = 0;
error:
    if (err) {
        fprintf (stderr, "streamer_dsp_chain_load: error loading %s\n", fname);
    }
    if (fp) {
        fclose (fp);
    }
    if (err && chain) {
        dsp_chain_free (chain);
        chain = NULL;
    }
    return chain;
}

int
streamer_dsp_chain_save_internal (const char *fname, ddb_dsp_context_t *chain) {
    char tempfile[PATH_MAX];
    snprintf (tempfile, sizeof (tempfile), "%s.tmp", fname);
    FILE *fp = fopen (tempfile, "w+t");
    if (!fp) {
        return -1;
    }

    ddb_dsp_context_t *ctx = chain;
    while (ctx) {
        if (fprintf (fp, "%s %d {\n", ctx->plugin->plugin.id, (int)ctx->enabled) < 0) {
            fprintf (stderr, "write to %s failed (%s)\n", tempfile, strerror (errno));
            goto error;
        }
        if (ctx->plugin->num_params) {
            int n = ctx->plugin->num_params ();
            int i;
            for (i = 0; i < n; i++) {
                char v[1000];
                ctx->plugin->get_param (ctx, i, v, sizeof (v));
                if (fprintf (fp, "\t%s\n", v) < 0) {
                    fprintf (stderr, "write to %s failed (%s)\n", tempfile, strerror (errno));
                    goto error;
                }
            }
        }
        if (fprintf (fp, "}\n") < 0) {
            fprintf (stderr, "write to %s failed (%s)\n", tempfile, strerror (errno));
            goto error;
        }
        ctx = ctx->next;
    }

    fclose (fp);
    if (rename (tempfile, fname) != 0) {
        fprintf (stderr, "dspconfig rename %s -> %s failed: %s\n", tempfile, fname, strerror (errno));
        return -1;
    }
    return 0;
error:
    fclose (fp);
    return -1;
}

int
streamer_dsp_chain_save (void) {
    char fname[PATH_MAX];
    snprintf (fname, sizeof (fname), "%s/dspconfig", plug_get_config_dir ());
    return streamer_dsp_chain_save_internal (fname, dsp_chain);
}

void
streamer_dsp_postinit (void) {
    // note about EQ hack:
    // we 1st check if there's an EQ in dsp chain, and just use it
    // if not -- we add our own

    // eq plug
    if (eqplug) {
        ddb_dsp_context_t *p;

        for (p = dsp_chain; p; p = p->next) {
            if (!strcmp (p->plugin->plugin.id, "supereq")) {
                break;
            }
        }
        if (p) {
            eq = p;
        }
        else {
            eq = eqplug->open ();
            eq->enabled = 0;
            eq->next = dsp_chain;
            dsp_chain = eq;
        }

    }
    ddb_dsp_context_t *ctx = dsp_chain;
    while (ctx) {
        if (ctx->enabled) {
            break;
        }
        ctx = ctx->next;
    }
    if (ctx) {
        dsp_on = 1;
    }
    else if (!ctx) {
        dsp_on = 0;
    }
}

void
streamer_dsp_init (void) {
    // load dsp chain from file
    char fname[PATH_MAX];
    snprintf (fname, sizeof (fname), "%s/dspconfig", plug_get_config_dir ());
    dsp_chain = streamer_dsp_chain_load (fname);
    if (!dsp_chain) {
        // first run, let's add resampler
        DB_dsp_t *src = (DB_dsp_t *)plug_get_for_id ("SRC");
        if (src) {
            ddb_dsp_context_t *inst = src->open ();
            inst->enabled = 1;
            src->set_param (inst, 0, "48000"); // samplerate
            src->set_param (inst, 1, "2"); // quality=SINC_FASTEST
            src->set_param (inst, 2, "1"); // auto
            inst->next = dsp_chain;
            dsp_chain = inst;
        }
    }

    eqplug = (DB_dsp_t *)plug_get_for_id ("supereq");
    streamer_dsp_postinit ();

    // load legacy eq settings from pre-0.5
    if (eq && eqplug && conf_find ("eq.", NULL)) {
        eq->enabled = deadbeef->conf_get_int ("eq.enable", 0);
        char s[50];

        // 0.4.4 was writing buggy settings, need to multiply by 2 to compensate
        conf_get_str ("eq.preamp", "0", s, sizeof (s));
        snprintf (s, sizeof (s), "%f", atof(s)*2);
        eqplug->set_param (eq, 0, s);
        for (int i = 0; i < 18; i++) {
            char key[100];
            snprintf (key, sizeof (key), "eq.band%d", i);
            conf_get_str (key, "0", s, sizeof (s));
            snprintf (s, sizeof (s), "%f", atof(s)*2);
            eqplug->set_param (eq, 1+i, s);
        }
        // delete obsolete settings
        conf_remove_items ("eq.");
    }
}


int
dsp_apply (ddb_waveformat_t *input_fmt, char *input, int inputsize,
           ddb_waveformat_t *out_fmt, char **out_bytes, int *out_numbytes, float *out_dsp_ratio) {

    *out_dsp_ratio = 1;

    ddb_waveformat_t dspfmt;
    memcpy (&dspfmt, input_fmt, sizeof (ddb_waveformat_t));
    dspfmt.bps = 32;
    dspfmt.is_float = 1;

    int can_bypass = 0;
    if (dsp_on) {
        // check if DSP can be passed through
        ddb_dsp_context_t *dsp = dsp_chain;
        while (dsp) {
            if (dsp->enabled) {
                if (dsp->plugin->plugin.api_vminor >= 1) {
                    if (dsp->plugin->can_bypass && !dsp->plugin->can_bypass (dsp, &dspfmt)) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
            dsp = dsp->next;
        }
        if (!dsp) {
            can_bypass = 1;
        }
    }

    if (!dsp_on || can_bypass) {
        return 0;
    }

    int inputsamplesize = input_fmt->channels * input_fmt->bps / 8;

    // convert to float, pass through streamer DSP chain
    int dspsamplesize = input_fmt->channels * sizeof (float);

    // make *MAX_DSP_RATIO sized buffer for float data
    int tempbuf_size = inputsize/inputsamplesize * dspsamplesize * MAX_DSP_RATIO;
    char *tempbuf = ensure_dsp_temp_buffer (tempbuf_size);

    // convert to float
    /*int tempsize = */pcm_convert (input_fmt, input, &dspfmt, tempbuf, inputsize);
    int nframes = inputsize / inputsamplesize;
    ddb_dsp_context_t *dsp = dsp_chain;
    float ratio = 1.f;
    int maxframes = tempbuf_size / dspsamplesize;
    while (dsp) {
        if (dsp->enabled) {
            float r = 1;
            nframes = dsp->plugin->process (dsp, (float *)tempbuf, nframes, maxframes, &dspfmt, &r);
            ratio *= r;
        }
        dsp = dsp->next;
    }

    *out_dsp_ratio = ratio;

    memcpy (out_fmt, &dspfmt, sizeof (ddb_waveformat_t));
    *out_bytes = tempbuf;
    *out_numbytes = nframes * dspfmt.channels * sizeof (float);

    return 1;

#warning blockbased FIXME: split post-dsp conversion and android resampler into separate functions
#if 0
    {
        int outputsamplesize = output->fmt.channels * output->fmt.bps / 8;
#ifdef ANDROID
        // if we not compensate here, the streamer loop will go crazy
        if (fileinfo->fmt.samplerate != output->fmt.samplerate) {
            if ((fileinfo->fmt.samplerate / output->fmt.samplerate) == 2 && (fileinfo->fmt.samplerate % output->fmt.samplerate) == 0) {
                size <<= 1;
            }
            else if ((fileinfo->fmt.samplerate / output->fmt.samplerate) == 4 && (fileinfo->fmt.samplerate % output->fmt.samplerate) == 0) {
                size <<= 2;
            }
        }
#endif

        // convert from input fmt to output fmt
        int inputsize = size/outputsamplesize*inputsamplesize;
        char input[inputsize];
        int nb = fileinfo->plugin->read (fileinfo, input, inputsize);
        if (nb != inputsize) {
            bytesread = nb;
            is_eof = 1;
        }
        inputsize = nb;
        //            trace ("convert %d|%d|%d|%d|%d|%d to %d|%d|%d|%d|%d|%d\n"
        //                , fileinfo->fmt.bps, fileinfo->fmt.channels, fileinfo->fmt.samplerate, fileinfo->fmt.channelmask, fileinfo->fmt.is_float, fileinfo->fmt.is_bigendian
        //                , output->fmt.bps, output->fmt.channels, output->fmt.samplerate, output->fmt.channelmask, output->fmt.is_float, output->fmt.is_bigendian);
        bytesread = pcm_convert (&fileinfo->fmt, input, &output->fmt, bytes, inputsize);

#ifdef ANDROID
        // downsample
        if (fileinfo->fmt.samplerate > output->fmt.samplerate) {
            if ((fileinfo->fmt.samplerate / output->fmt.samplerate) == 2 && (fileinfo->fmt.samplerate % output->fmt.samplerate) == 0) {
                // clip to multiple of 2 samples
                int outsamplesize = output->fmt.channels * (output->fmt.bps>>3) * 2;
                if ((bytesread % outsamplesize) != 0) {
                    bytesread -= (bytesread % outsamplesize);
                }

                // 2x downsample
                int nframes = bytesread / (output->fmt.bps >> 3) / output->fmt.channels;
                int16_t *in = (int16_t *)bytes;
                int16_t *out = in;
                for (int f = 0; f < nframes/2; f++) {
                    for (int c = 0; c < output->fmt.channels; c++) {
                        out[f*output->fmt.channels+c] = (in[f*2*output->fmt.channels+c] + in[(f*2+1)*output->fmt.channels+c]) >> 1;
                    }
                }
                bytesread >>= 1;
            }
            else if ((fileinfo->fmt.samplerate / output->fmt.samplerate) == 4 && (fileinfo->fmt.samplerate % output->fmt.samplerate) == 0) {
                // clip to multiple of 4 samples
                int outsamplesize = output->fmt.channels * (output->fmt.bps>>3) * 4;
                if ((bytesread % outsamplesize) != 0) {
                    bytesread -= (bytesread % outsamplesize);
                }

                // 4x downsample
                int nframes = bytesread / (output->fmt.bps >> 3) / output->fmt.channels;
                assert (bytesread % ((output->fmt.bps >> 3) * output->fmt.channels) == 0);
                int16_t *in = (int16_t *)bytes;
                for (int f = 0; f < nframes/4; f++) {
                    for (int c = 0; c < output->fmt.channels; c++) {
                        in[f*output->fmt.channels+c] = (in[f*4*output->fmt.channels+c]
                                                        + in[(f*4+1)*output->fmt.channels+c]
                                                        + in[(f*4+2)*output->fmt.channels+c]
                                                        + in[(f*4+3)*output->fmt.channels+c]) >> 2;
                    }
                }
                bytesread >>= 2;
            }
        }
        assert ((bytesread%2) == 0);
#endif

    }
#endif
}

void
dsp_get_output_format (ddb_waveformat_t *in_fmt, ddb_waveformat_t *out_fmt) {
    memcpy (out_fmt, in_fmt, sizeof (ddb_waveformat_t));
    if (dsp_on) {
        // check if DSP can be passed through
        ddb_dsp_context_t *dsp = dsp_chain;
        while (dsp) {
            if (dsp->enabled) {
                float ratio;
                if (dsp->plugin->plugin.api_vminor >= 1) {
                    if (dsp->plugin->can_bypass && !dsp->plugin->can_bypass (dsp, out_fmt)) {
                        dsp->plugin->process (dsp, NULL, 0, 0, out_fmt, &ratio);
                    }
                }
                else {
                    dsp->plugin->process (dsp, NULL, 0, 0, out_fmt, &ratio);
                }
            }
            dsp = dsp->next;
        }
    }

}
