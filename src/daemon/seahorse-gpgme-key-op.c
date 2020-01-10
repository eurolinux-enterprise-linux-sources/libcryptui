/*
 * Seahorse
 *
 * Copyright (C) 2003 Jacob Perkins
 * Copyright (C) 2004 Stefan Walter
 * Copyright (C) 2005 Adam Schreiber
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "seahorse-gpgme.h"
#include "seahorse-gpgme-data.h"
#include "seahorse-gpg-op.h"
#include "seahorse-gpgme-key-op.h"
#include "seahorse-gpgme-operation.h"
#include "seahorse-object-list.h"
#include "seahorse-libdialogs.h"
#include "seahorse-util.h"

#define PROMPT "keyedit.prompt"
#define QUIT "quit"
#define SAVE "keyedit.save.okay"
#define YES "Y"
#define NO "N"

#define PRINT(args)  if(!seahorse_util_print_fd args) return GPG_E (GPG_ERR_GENERAL)
#define PRINTF(args) if(!seahorse_util_printf_fd args) return GPG_E (GPG_ERR_GENERAL)

 
#ifndef DEBUG_OPERATION_ENABLE
#if _DEBUG
#define DEBUG_OPERATION_ENABLE 1
#else
#define DEBUG_OPERATION_ENABLE 0
#endif
#endif

#if DEBUG_OPERATION_ENABLE
#define DEBUG_OPERATION(x) g_printerr x
#else
#define DEBUG_OPERATION(x) 
#endif

#define GPG_UNKNOWN     1
#define GPG_NEVER       2
#define GPG_MARGINAL    3
#define GPG_FULL        4
#define GPG_ULTIMATE    5

/**
 * seahorse_gpgme_key_op_generate:
 * @sksrc: #SeahorseSource
 * @name: User ID name, must be at least 5 characters long
 * @email: Optional user ID email
 * @comment: Optional user ID comment
 * @passphrase: Passphrase for key
 * @type: Key type. Supported types are #DSA_ELGAMAL, #DSA, #RSA_SIGN, and #RSA_RSA
 * @length: Length of key, must be within the range of @type specified by #SeahorseKeyLength
 * @expires: Expiration date of key
 * @err: Catches errors in the params
 * 
 * Tries to generate a new key based on given parameters.
 *
 * Returns: SeahorseOperation*
 **/
SeahorseOperation*
seahorse_gpgme_key_op_generate (SeahorseGpgmeSource *psrc, const gchar *name,
                                const gchar *email, const gchar *comment,
                                const gchar *passphrase, const SeahorseKeyEncType type,
                                const guint length, const time_t expires, gpgme_error_t *err)
{
    SeahorseGpgmeOperation *pop = NULL;
    gchar *common, *key_type, *start, *expires_date;
    const gchar *parms;
    
    *err = GPG_OK;

    if (strlen (name) < 5)
        *err = GPG_E (GPG_ERR_INV_VALUE);

    /* Check lengths for each type */
    switch (type) {
    case DSA_ELGAMAL:
        if (length < ELGAMAL_MIN || length > LENGTH_MAX)
            *err = GPG_E (GPG_ERR_INV_VALUE);
        break;
    case DSA:
        if (length < DSA_MIN || length > DSA_MAX)
            *err = GPG_E (GPG_ERR_INV_VALUE);
        break;
    case RSA_RSA:
    case RSA_SIGN:
        if (length < RSA_MIN || length > LENGTH_MAX)
            *err = GPG_E (GPG_ERR_INV_VALUE);
        break;
    default:
        *err = GPG_E (GPG_ERR_INV_VALUE);
        break;
    }

    if (0 != expires)
        expires_date = seahorse_util_get_date_string (expires);
    else
        expires_date = g_strdup ("0");

    /* Common xml */
    common = g_strdup_printf ("Name-Real: %s\nExpire-Date: %s\nPassphrase: %s\n"
                              "</GnupgKeyParms>", name, expires_date, passphrase);
    if (email != NULL && strlen (email) > 0)
        common = g_strdup_printf ("Name-Email: %s\n%s", email, common);
    if (comment != NULL && strlen (comment) > 0)
        common = g_strdup_printf ("Name-Comment: %s\n%s", comment, common);

    if (type == DSA || type == DSA_ELGAMAL)
        key_type = "Key-Type: DSA\nKey-Usage: sign";
    else
        key_type = "Key-Type: RSA\nKey-Usage: sign";

    start = g_strdup_printf ("<GnupgKeyParms format=\"internal\">\n%s\nKey-Length: ", key_type);

    /* Subkey xml */
    if (type == DSA_ELGAMAL)
        parms = g_strdup_printf ("%s%d\nSubkey-Type: ELG-E\nSubkey-Length: %d\nSubkey-Usage: encrypt\n%s",
                                 start, (length < DSA_MAX) ? length : DSA_MAX, length, common);
    else if (type == RSA_RSA)
        parms = g_strdup_printf ("%s%d\nSubkey-Type: RSA\nSubkey-Length: %d\nSubkey-Usage: encrypt\n%s",
                                 start, length, length, common);
    else
        parms = g_strdup_printf ("%s%d\n%s", start, length, common);

    if (GPG_IS_OK (*err)) {
        pop = seahorse_gpgme_operation_new (NULL);
        *err = gpgme_op_genkey_start (pop->gctx, parms, NULL, NULL);
     }

    /* Free xmls */
    g_free (start);
    g_free (common);
    g_free (expires_date);

    return pop ? SEAHORSE_OPERATION (pop) : NULL;
}
