/* Copyright (C) 2014 GSS-NTLMSSP contributors, see COPYING for license */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#include "gss_ntlmssp.h"

#define UNKNOWN_ERROR "Unknown Error"

uint32_t gssntlm_display_status(uint32_t *minor_status,
                                uint32_t status_value,
                                int status_type,
                                gss_OID mech_type,
                                uint32_t *message_context,
                                gss_buffer_t status_string)
{
    /* if you can't say it in ~6 lines of text we don't bother */
    char buf[512];
    int err;

    if (!minor_status || !status_string) {
        *minor_status = EINVAL;
        return GSS_S_CALL_INACCESSIBLE_READ;
    }

    *minor_status = 0;
    status_string->length = 0;
    status_string->value = NULL;

    if (!status_value) {
        /* There must have been *some* error. No point saying 'Success' */
        goto done;
    }

    errno = 0;
    err = strerror_r(status_value, buf, 400);
    /* The XSI-compliant strerror_r() function returns 0 on success.
     * On error, a (positive) error number is returned (since glibc
     * 2.13), or -1 is returned and errno is set to indicate the
     * error (glibc versions before 2.13). */
    if (err == -1) err = errno;
    switch (err) {
    case ERANGE:
        /* Screw it, they can have a truncated version */
    case 0:
        status_string->value = strdup(buf);
        break;
    default:
        break;
    }

done:
    if (!status_string->value) {
        status_string->value = strdup(UNKNOWN_ERROR);
        if (!status_string->value) {
            *minor_status = ENOMEM;
            return GSS_S_FAILURE;
        }
    }
    status_string->length = strlen(status_string->value);
    return GSS_S_COMPLETE;
}
