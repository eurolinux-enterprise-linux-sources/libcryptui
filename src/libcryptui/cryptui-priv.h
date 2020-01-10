/* 
 * Seahorse
 * 
 * Copyright (C) 2005 Stefan Walter
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *  
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.  
 */
 
#ifndef __CRYPTUI_PRIV_H__
#define __CRYPTUI_PRIV_H__

/* 
 * Used internally by libcryptui. 
 */

#ifndef LIBCRYPTUI_BUILD
#error "This header shouldn't be included outside of the libcryptui build."
#endif

#include "cryptui.h"
#include "cryptui-defines.h"

/* cryptui-keyset.c --------------------------------------------------------- */

const gchar*    _cryptui_keyset_get_internal_keyid  (CryptUIKeyset *ckset,
                                                     const gchar *keyid);

#endif /* __CRYPTUI_PRIV_H__ */
