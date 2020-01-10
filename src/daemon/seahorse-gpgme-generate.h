/*
 * Seahorse
 *
 * Copyright (C) 2003 Jacob Perkins
 * Copyright (C) 2004-2005 Stefan Walter
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Various UI elements and dialogs used in pgp component.
 */
 
#ifndef __SEAHORSE_GPGME_GENERATE_H__
#define __SEAHORSE_GPGME_GENERATE_H__

#include <gtk/gtk.h>

#include "seahorse-gpgme-source.h"

void            seahorse_gpgme_generate_key         (SeahorseGpgmeSource *sksrc,
                                                     const gchar *name,
                                                     const gchar *email,
                                                     const gchar *comment,
                                                     guint type,
                                                     guint bits,
                                                     time_t expires);

#endif /* __SEAHORSE_GPGME_GENERATE_H__ */
