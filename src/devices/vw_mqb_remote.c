/* VW group MQB-platform remote contols. 
 * Tested with:
 *  FCC ID: 5E0 959 752 D
 *
 * Copyright (C) 2019 Ivan Vasilev 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "decoder.h"

#define MSG_LENGTH_MIN      628     // in bits
#define MSG_LENGTH_MAX      629     // in bits
#define SYNC_LENGTH         53      // number of sync words before actual data
#define SYNC_VALUE          0xaa
#define ID_LENGTH           4
#define BUTTON_LENGTH       2

static int vw_mqb_remote_callback(r_device *decoder, bitbuffer_t *bitbuffer)
{
    // first +1 is to allow space for the extra bits that are not part of the last byte
    // the second +1 is for the zero terminator.
    char printbuffer[(((MSG_LENGTH_MAX / 8) + 1) * 2) + 1];
    char id_str[ID_LENGTH * 2 + 1];
    char button_str[BUTTON_LENGTH * 2 + 1];
    data_t *data;

    //fprintf(stderr, "Number of rows: %d\r\n", bitbuffer->num_rows);

    //for (uint16_t i = 0; i<bitbuffer->num_rows; i++)
    //{
    //    fprintf(stderr, "Row %d, bits: %d\r\n", i, bitbuffer->bits_per_row[i]);
    //}

    if (bitbuffer->num_rows != 1)
    {
        return DECODE_ABORT_LENGTH;    
    }

    if ((bitbuffer->bits_per_row[0] < MSG_LENGTH_MIN) || (bitbuffer->bits_per_row[0] > MSG_LENGTH_MAX))
    {
        return DECODE_ABORT_LENGTH;
    }

    for (uint16_t i = 0; i < SYNC_LENGTH; i++) 
    {
        if (bitbuffer->bb[0][i] != SYNC_VALUE)
        {
            return DECODE_FAIL_MIC;
        }
    }


    memset(printbuffer, 0, sizeof(printbuffer));
    // Message seems valid, try to process it
    for (int col = 0; col < (bitbuffer->bits_per_row[0] + 7) / 8; ++col) 
    {
        sprintf(&printbuffer[2 * col], "%02x", bitbuffer->bb[0][col]);
    }

    memcpy(id_str, &printbuffer[SYNC_LENGTH * 2], ID_LENGTH * 2);
    id_str[ID_LENGTH * 2] = 0;

    memcpy(button_str, &printbuffer[(SYNC_LENGTH + ID_LENGTH) * 2], BUTTON_LENGTH * 2);
    button_str[BUTTON_LENGTH * 2] = 0;

    data = data_make(
            "model",        "",       DATA_STRING, _X("VW MQB Remote Control","MQB Remote"),
            "Remote ID",    "",       DATA_STRING, id_str,
            "Button ID",    "",       DATA_STRING, button_str,
            "Buffer",       "Data",   DATA_STRING, &printbuffer[(SYNC_LENGTH + ID_LENGTH + BUTTON_LENGTH) * 2],
            NULL);

    decoder_output_data(decoder, data);

    return 1;
}

static char *output_fields[] = {
    "data",
    NULL
};

r_device vw_mqb_remote = {
    .name           = "VW MQB Remote Keyfob",
    .modulation     = OOK_PULSE_PCM_RZ ,
    .short_width    = 300,
    .long_width     = 300,
    .reset_limit    = 1800,
    .sync_width     = 0,    // No sync bit used
    .tolerance      = 10, // us
    .decode_fn      = &vw_mqb_remote_callback,
    .disabled       = 0,
    .fields         = output_fields,
};
