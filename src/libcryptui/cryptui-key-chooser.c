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

#include "config.h"
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "cryptui-key-list.h"
#include "cryptui-key-combo.h"
#include "cryptui-key-chooser.h"
#include "cryptui-priv.h"

enum {
    PROP_0,
    PROP_KEYSET,
    PROP_MODE,
    PROP_ENFORCE_PREFS,
    PROP_SYMMETRIC
};

enum {
    CHANGED,
    LAST_SIGNAL
};


struct _CryptUIKeyChooserPriv {
    guint                   mode;
    gboolean                initialized : 1;
    gboolean                symmetric : 1;
    
    CryptUIKeyset           *ckset;
    CryptUIKeyStore         *ckstore;
    GtkTreeView             *keylist;
    GtkComboBox             *keycombo;
    GtkCheckButton          *signercheck;
    GSettings               *settings;

    GtkComboBox             *filtermode;
    GtkEntry                *filtertext;
};

G_DEFINE_TYPE (CryptUIKeyChooser, cryptui_key_chooser, GTK_TYPE_VBOX);
static guint signals[LAST_SIGNAL] = { 0 };

/* -----------------------------------------------------------------------------
 * INTERNAL
 */

static gboolean 
recipients_filter (CryptUIKeyset *ckset, const gchar *key, gpointer user_data)
{
    guint flags = cryptui_keyset_key_flags (ckset, key);
    return flags & CRYPTUI_FLAG_CAN_ENCRYPT;
}

static gboolean 
signer_filter (CryptUIKeyset *ckset, const gchar *key, gpointer user_data)
{
    guint flags = cryptui_keyset_key_flags (ckset, key);
    return flags & CRYPTUI_FLAG_CAN_SIGN;
}

static void
filtertext_changed (GtkWidget *widget, CryptUIKeyChooser *chooser)
{
    const gchar *text = gtk_entry_get_text (chooser->priv->filtertext);
    g_object_set (chooser->priv->ckstore, "search", text, NULL);
}

static void
filtertext_activate (GtkEntry *entry, CryptUIKeyChooser *chooser)
{
    gtk_widget_grab_focus (GTK_WIDGET (chooser->priv->keylist));
}

static void
filtermode_changed (GtkWidget *widget, CryptUIKeyChooser *chooser)
{
    gint active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    if (active >= 0)
        g_object_set (chooser->priv->ckstore, "mode", active, NULL);    
}

static void
recipients_changed (GtkWidget *widget, CryptUIKeyChooser *chooser)
{
    g_signal_emit (chooser, signals[CHANGED], 0);
}

static void
signer_changed (GtkWidget *widget, CryptUIKeyChooser *chooser)
{
	const gchar *signer;

	g_assert (chooser->priv->keycombo);

	if (chooser->priv->settings) {
		signer = cryptui_key_combo_get_key (chooser->priv->keycombo);
		g_settings_set_string (chooser->priv->settings, "last-signer",
		                       signer ? signer : "");
	}

	g_signal_emit (chooser, signals[CHANGED], 0);
}

static void
signer_toggled (GtkWidget *widget, CryptUIKeyChooser *chooser)
{
	const gchar *signer;

	g_assert (chooser->priv->signercheck);

	if (chooser->priv->settings) {
		signer = g_object_get_data ((GObject*) (chooser->priv->signercheck), "key");
		g_settings_set_string (chooser->priv->settings, "last-signer",
		                       signer ? signer : "");
	}

	g_signal_emit (chooser, signals[CHANGED], 0);
}

static void encryption_mode_changed (GtkToggleButton * button, CryptUIKeyChooser *chooser)
{
	gboolean use_public_key_encryption;

	use_public_key_encryption = gtk_toggle_button_get_active (button);
	chooser->priv->symmetric = !use_public_key_encryption;
	gtk_widget_set_sensitive (GTK_WIDGET (chooser->priv->filtermode), use_public_key_encryption);
	gtk_widget_set_sensitive (GTK_WIDGET (chooser->priv->filtertext), use_public_key_encryption);
	gtk_widget_set_sensitive (GTK_WIDGET (chooser->priv->keylist), use_public_key_encryption);

	g_signal_emit (chooser, signals[CHANGED], 0);
}

static void
construct_recipients (CryptUIKeyChooser *chooser, GtkBox *box)
{
    GtkTreeSelection *selection;
    GtkWidget *scroll;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *radio_public_key;
    GtkWidget *radio_symmetric;
    gboolean support_symmetric;
    gint indicator_size = 0;
    gint indicator_spacing = 0;
    gint focus_width = 0;
    gint focus_pad = 0;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

    /* Top filter box */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

    /* Filter Combo */
    chooser->priv->filtermode = GTK_COMBO_BOX (gtk_combo_box_text_new ());
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (chooser->priv->filtermode), _("All Keys"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (chooser->priv->filtermode), _("Selected Recipients"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (chooser->priv->filtermode), _("Search Results"));
    gtk_combo_box_set_active (chooser->priv->filtermode, 0);
    g_signal_connect (chooser->priv->filtermode, "changed", 
                        G_CALLBACK (filtermode_changed), chooser);
    gtk_widget_set_size_request (GTK_WIDGET (chooser->priv->filtermode), 140, -1);
    gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (chooser->priv->filtermode));
    gtk_box_set_child_packing (GTK_BOX (hbox), GTK_WIDGET (chooser->priv->filtermode), 
                                FALSE, TRUE, 0, GTK_PACK_START);
    
    /* Filter Label */
    label = gtk_label_new (_("Search _for:"));
    gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_container_add (GTK_CONTAINER (hbox), label);
    gtk_box_set_child_packing (GTK_BOX (hbox), label, 
                                TRUE, TRUE, 0, GTK_PACK_START);

    /* Filter Entry */
    chooser->priv->filtertext = GTK_ENTRY (gtk_entry_new ());
    gtk_entry_set_max_length (chooser->priv->filtertext, 256);
    gtk_widget_set_size_request (GTK_WIDGET (chooser->priv->filtertext), 140, -1);
    g_signal_connect (chooser->priv->filtertext, "changed", 
                        G_CALLBACK (filtertext_changed), chooser);
    g_signal_connect (chooser->priv->filtertext, "activate", 
                        G_CALLBACK (filtertext_activate), chooser);
    gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (chooser->priv->filtertext));
    gtk_box_set_child_packing (GTK_BOX (hbox), GTK_WIDGET (chooser->priv->filtertext), 
                                FALSE, TRUE, 0, GTK_PACK_START);

    /* Add Filter box */
    gtk_container_add (GTK_CONTAINER (vbox), hbox);
    gtk_box_set_child_packing (GTK_BOX (vbox), hbox,
                               FALSE, TRUE, 0, GTK_PACK_START);

    chooser->priv->ckstore = cryptui_key_store_new (chooser->priv->ckset, TRUE, NULL);
    cryptui_key_store_set_sortable (chooser->priv->ckstore, TRUE);
    cryptui_key_store_set_filter (chooser->priv->ckstore, recipients_filter, NULL);
    
    /* Main Key list */
    chooser->priv->keylist = cryptui_key_list_new (chooser->priv->ckstore, 
                                                    CRYPTUI_KEY_LIST_CHECKS);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (chooser->priv->keylist), FALSE);
    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scroll, 500, 300);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET (chooser->priv->keylist));
    gtk_container_add (GTK_CONTAINER (vbox), scroll);
    gtk_box_set_child_packing (GTK_BOX (vbox), scroll,
                               TRUE, TRUE, 0, GTK_PACK_START);

    support_symmetric = (chooser->priv->mode & CRYPTUI_KEY_CHOOSER_SUPPORT_SYMMETRIC) == CRYPTUI_KEY_CHOOSER_SUPPORT_SYMMETRIC;

    if (support_symmetric) {
        radio_symmetric = gtk_radio_button_new (NULL /* first of the group */);
        label = gtk_label_new (_("Use passphrase only"));
        gtk_container_add (GTK_CONTAINER (radio_symmetric), label);
        gtk_container_add (GTK_CONTAINER (box), radio_symmetric);
        gtk_box_set_child_packing (GTK_BOX (box), radio_symmetric,
                                   FALSE, TRUE, 0, GTK_PACK_START);

        radio_public_key = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON (radio_symmetric));
        g_signal_connect (radio_public_key, "toggled",
                          G_CALLBACK (encryption_mode_changed), chooser);
        label = gtk_label_new (_("Choose a set of recipients:"));
        gtk_container_add (GTK_CONTAINER (radio_public_key), label);
        gtk_container_add (GTK_CONTAINER (box), radio_public_key);
        gtk_box_set_child_packing (GTK_BOX (box), radio_public_key,
                                   FALSE, TRUE, 0, GTK_PACK_START);
        gtk_widget_style_get (GTK_WIDGET (radio_public_key),
                              "indicator-size", &indicator_size,
                              "indicator-spacing", &indicator_spacing,
                              NULL);
        gtk_widget_style_get (GTK_WIDGET (radio_public_key),
                              "focus-line-width", &focus_width,
                              "focus-padding", &focus_pad,
                              NULL);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radio_public_key),
                                     !cryptui_key_chooser_get_symmetric (chooser));

        gtk_widget_set_margin_left (vbox, indicator_size + 2 * indicator_spacing +
                                          focus_width + focus_pad);
    }
    gtk_container_add (GTK_CONTAINER (box), vbox);
    gtk_box_set_child_packing (GTK_BOX (box), vbox,
                               TRUE, TRUE, 0, GTK_PACK_START);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chooser->priv->keylist));
    g_signal_connect (selection, "changed", G_CALLBACK (recipients_changed), chooser);
}

static void
construct_signer (CryptUIKeyChooser *chooser, GtkBox *box)
{
    const gchar *none_option = NULL;
    CryptUIKeyStore *ckstore;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separator;
    guint count;
    GList *keys;
    gchar *keyname, *labelstr;
    gboolean support_symmetric;

    support_symmetric = (chooser->priv->mode & CRYPTUI_KEY_CHOOSER_SUPPORT_SYMMETRIC) == CRYPTUI_KEY_CHOOSER_SUPPORT_SYMMETRIC;
    
    /* TODO: HIG and beautification */
        
    if (!(chooser->priv->mode & CRYPTUI_KEY_CHOOSER_MUSTSIGN))
        none_option = _("None (Don't Sign)");

    /* The Sign combo */
    ckstore = cryptui_key_store_new (chooser->priv->ckset, TRUE, none_option);
    cryptui_key_store_set_filter (ckstore, signer_filter, NULL);

    count = cryptui_key_store_get_count (ckstore);
    
    if (count == 1) {
        keys = cryptui_key_store_get_all_keys (ckstore);
        
        keyname = cryptui_keyset_key_display_name (ckstore->ckset, (gchar*) keys->data);
        fprintf (stderr, "Display name is: %s\n", keyname);
        labelstr = g_strdup_printf (_("Sign this message as %s"), keyname);
        fprintf (stderr, "labelstr is: %s\nCreating check button", labelstr);
        
        chooser->priv->signercheck = (GtkCheckButton*) gtk_check_button_new_with_label (labelstr);
        g_object_set_data ((GObject*) (chooser->priv->signercheck), "ckset", ckstore->ckset);
        g_object_set_data ((GObject*) (chooser->priv->signercheck), "key", keys->data);
        
        g_signal_connect (chooser->priv->signercheck , "toggled", G_CALLBACK (signer_toggled), chooser);

        /* Add it in */
        if (support_symmetric) {
            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
            gtk_container_add (GTK_CONTAINER (box), separator);
            gtk_box_set_child_packing (box, separator, FALSE, TRUE, 0, GTK_PACK_START);
        }
        gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (chooser->priv->signercheck));
        gtk_box_set_child_packing (box, GTK_WIDGET (chooser->priv->signercheck), FALSE, TRUE, 0, GTK_PACK_START);

        
        g_free (labelstr);
        g_free (keyname);
        g_list_free (keys);
    } else if (count > 1) {
        /* Top filter box */
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

        /* Sign Label */
        label = gtk_label_new (_("_Sign message as:"));
        gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
        gtk_container_add (GTK_CONTAINER (hbox), label);
        gtk_box_set_child_packing (GTK_BOX (hbox), label, 
                                   FALSE, TRUE, 0, GTK_PACK_START);
        
        chooser->priv->keycombo = cryptui_key_combo_new (ckstore);
        g_signal_connect (chooser->priv->keycombo, "changed", G_CALLBACK (signer_changed), chooser);
        gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (chooser->priv->keycombo));
        gtk_box_set_child_packing (GTK_BOX (hbox), GTK_WIDGET (chooser->priv->keycombo), 
                                   TRUE, TRUE, 0, GTK_PACK_START);
                                                              
        /* Add it in */
        if (support_symmetric) {
            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
            gtk_container_add (GTK_CONTAINER (box), separator);
            gtk_box_set_child_packing (box, separator, FALSE, TRUE, 0, GTK_PACK_START);
        }
        gtk_container_add (GTK_CONTAINER (box), hbox);
        gtk_box_set_child_packing (box, hbox, FALSE, TRUE, 0, GTK_PACK_START);
    }
    
    g_object_unref (ckstore);
}

/* -----------------------------------------------------------------------------
 * OBJECT 
 */

static void
cryptui_key_chooser_init (CryptUIKeyChooser *chooser)
{
    /* init private vars */
    chooser->priv = g_new0 (CryptUIKeyChooserPriv, 1);
}

static void
cryptui_key_chooser_constructed (GObject *obj)
{
    CryptUIKeyChooser *chooser = CRYPTUI_KEY_CHOOSER (obj);
    gchar *value;

    G_OBJECT_CLASS (cryptui_key_chooser_parent_class)->constructed (obj);

    /* Set the spacing for this box */
    gtk_box_set_spacing (GTK_BOX (obj), 6);
    gtk_container_set_border_width (GTK_CONTAINER (obj), 6);
    
    /* Add the various objects now */
    if (chooser->priv->mode & CRYPTUI_KEY_CHOOSER_RECIPIENTS) {
        construct_recipients (chooser, GTK_BOX (obj));
    }
    
    /* The signing area */
    if (chooser->priv->mode & CRYPTUI_KEY_CHOOSER_SIGNER) {
        construct_signer (chooser, GTK_BOX (obj));
        
        if (chooser->priv->settings && chooser->priv->keycombo) {
            value = g_settings_get_string (chooser->priv->settings, "last-signer");
            cryptui_key_combo_set_key (chooser->priv->keycombo, value);
            g_free (value);
        }
    }

    /* Focus an appropriate widget */
    if (chooser->priv->filtertext)
        gtk_widget_grab_focus (GTK_WIDGET (chooser->priv->filtertext));
    else if (chooser->priv->keylist)
        gtk_widget_grab_focus (GTK_WIDGET (chooser->priv->keylist));
    else if (chooser->priv->keycombo)
        gtk_widget_grab_focus (GTK_WIDGET (chooser->priv->keycombo));
    else if (chooser->priv->signercheck)
        gtk_widget_grab_focus (GTK_WIDGET (chooser->priv->signercheck));
        
    chooser->priv->initialized = TRUE;
}

/* dispose of all our internal references */
static void
cryptui_key_chooser_dispose (GObject *gobject)
{
    CryptUIKeyChooser *chooser = CRYPTUI_KEY_CHOOSER (gobject);  

    if (chooser->priv->ckset)
        g_object_unref (chooser->priv->ckset);        
    chooser->priv->ckset = NULL;
    
    if (chooser->priv->ckstore)
        g_object_unref (chooser->priv->ckstore);
    chooser->priv->ckstore = NULL;
    
    G_OBJECT_CLASS (cryptui_key_chooser_parent_class)->dispose (gobject);
}

static void
cryptui_key_chooser_finalize (GObject *gobject)
{
    CryptUIKeyChooser *chooser = CRYPTUI_KEY_CHOOSER (gobject);
    
    g_assert (chooser->priv->ckset == NULL);
    g_clear_object (&chooser->priv->settings);
    g_free (chooser->priv);
    
    G_OBJECT_CLASS (cryptui_key_chooser_parent_class)->finalize (gobject);
}

static void
cryptui_key_chooser_set_property (GObject *gobject, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
    CryptUIKeyChooser *chooser = CRYPTUI_KEY_CHOOSER (gobject);
    
    switch (prop_id) {
    case PROP_KEYSET:
        g_assert (chooser->priv->ckset == NULL);
        chooser->priv->ckset = g_value_get_object (value);
        g_object_ref (chooser->priv->ckset);
        break;
    
    case PROP_MODE:
        chooser->priv->mode = g_value_get_uint (value);
        break;
    
    case PROP_ENFORCE_PREFS:
        if (g_value_get_boolean (value)) {
            if (!chooser->priv->settings)
                chooser->priv->settings = g_settings_new ("org.gnome.crypto.pgp");
        } else {
            g_clear_object (&chooser->priv->settings);
        }
        break;

    case PROP_SYMMETRIC:
        chooser->priv->symmetric = g_value_get_boolean (value);

    default:
        break;
    }
}

static void
cryptui_key_chooser_get_property (GObject *gobject, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
    CryptUIKeyChooser *chooser = CRYPTUI_KEY_CHOOSER (gobject);

    switch (prop_id) {
    case PROP_KEYSET:
        g_value_set_object (value, chooser->priv->ckset);
        break;
    
    case PROP_MODE:
        g_value_set_uint (value, chooser->priv->mode);
        break;
    
    case PROP_ENFORCE_PREFS:
        g_value_set_boolean (value, chooser->priv->settings != NULL);
        break;

    case PROP_SYMMETRIC:
        g_value_set_boolean (value, chooser->priv->symmetric);

    
    default:
        break;
    }
}

static void
cryptui_key_chooser_class_init (CryptUIKeyChooserClass *klass)
{
    GObjectClass *gclass;

    cryptui_key_chooser_parent_class = g_type_class_peek_parent (klass);
    gclass = G_OBJECT_CLASS (klass);

    gclass->constructed = cryptui_key_chooser_constructed;
    gclass->dispose = cryptui_key_chooser_dispose;
    gclass->finalize = cryptui_key_chooser_finalize;
    gclass->set_property = cryptui_key_chooser_set_property;
    gclass->get_property = cryptui_key_chooser_get_property;
    
    g_object_class_install_property (gclass, PROP_KEYSET,
        g_param_spec_object ("keyset", "CryptUI Keyset", "Current CryptUI Key Source to use",
                             CRYPTUI_TYPE_KEYSET, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    
    g_object_class_install_property (gclass, PROP_MODE,
        g_param_spec_uint ("mode", "Display Mode", "Display mode for chooser",
                           0, 0x0FFFFFFF, 0, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
    
    g_object_class_install_property (gclass, PROP_ENFORCE_PREFS,
        g_param_spec_boolean ("enforce-prefs", "Enforce User Preferences", "Enforce user preferences",
                              TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gclass, PROP_SYMMETRIC,
        g_param_spec_boolean ("symmetric", "Use symmetric encryption", "Use symmetric encryption",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    
    signals[CHANGED] = g_signal_new ("changed", CRYPTUI_TYPE_KEY_CHOOSER, 
                G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (CryptUIKeyChooserClass, changed),
                NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* -----------------------------------------------------------------------------
 * PUBLIC 
 */

/**
 * cryptui_key_chooser_new:
 * @ckset: key set to choose keys from
 * @mode: how to display the signer portion of the widget
 *
 * Creates a key chooser widget from a key set.
 *
 * Returns: a key shooser widget
 */
CryptUIKeyChooser*
cryptui_key_chooser_new (CryptUIKeyset *ckset, CryptUIKeyChooserMode mode)
{
    GObject *obj = g_object_new (CRYPTUI_TYPE_KEY_CHOOSER, "keyset", ckset, 
                                                           "mode", mode, NULL);
    return CRYPTUI_KEY_CHOOSER (obj);
}

/**
 * cryptui_key_chooser_get_enforce_prefs:
 * @chooser: the key chooser widget
 *
 * Gets whether or not the preference to encrypt to self will be enforced.  If
 * TRUE, the default signing key will be added to the recipient list.  If FALSE,
 * the selected recipients will be returned without adding the default signing
 * key.
 *
 * Returns: whether or not the default signing key will be included in the
 *          recipients even if not selected
 */
gboolean
cryptui_key_chooser_get_enforce_prefs (CryptUIKeyChooser *chooser)
{
    return chooser->priv->settings != NULL;
}

/**
 * cryptui_key_chooser_set_enforce_prefs:
 * @chooser: the chooser widget
 * @enforce_prefs: whether or not the default signing key will be included in the
 *                 recipients even if not selected
 *
 * Sets whether or not the preference to encrypt to self will be enforced.  If
 * TRUE, the default signing key will be added to the recipient list.  If FALSE,
 * the selected recipients will be returned without adding the default signing
 * key.
 */
void
cryptui_key_chooser_set_enforce_prefs (CryptUIKeyChooser *chooser,
                                       gboolean enforce_prefs)
{
	g_object_set (chooser, "enforce-prefs", enforce_prefs, NULL);
}

/**
 * cryptui_key_chooser_have_recipients:
 * @chooser: the chooser to check
 *
 * Determines if recipient keys have been selected.
 *
 * Returns: TRUE if recipients have been selected.
 */
gboolean
cryptui_key_chooser_have_recipients (CryptUIKeyChooser *chooser)
{
    g_return_val_if_fail (chooser->priv->keylist != NULL, FALSE);
    return cryptui_key_list_have_selected_keys (chooser->priv->keylist);
}

/**
 * cryptui_key_chooser_get_recipients:
 * @chooser: the chooser to get selected recipients from
 *
 * This function returns a list of recipients selected in the chooser widget.
 *
 * Returns: the list of recipients
 */
GList*
cryptui_key_chooser_get_recipients (CryptUIKeyChooser *chooser)
{
    CryptUIKeyset *keyset;
    GList *recipients;
    const gchar *key;
    gchar *value;

    g_return_val_if_fail (chooser->priv->keylist != NULL, NULL);
    recipients = cryptui_key_list_get_selected_keys (chooser->priv->keylist);

    if (!chooser->priv->settings)
        return recipients;

    if (!g_settings_get_boolean (chooser->priv->settings, "encrypt-to-self"))
        return recipients;

    /* If encrypt to self, then add that key */
    keyset = cryptui_key_list_get_keyset (chooser->priv->keylist);
    key = NULL;
    
    /* If we have a signer then use that */
    if (chooser->priv->keycombo)
        key = cryptui_key_combo_get_key (chooser->priv->keycombo);
    
    /* Lookup the default key */
    if (key == NULL) {
        value = g_settings_get_string (chooser->priv->settings, "default-key");
        if (value != NULL && value[0] != '\0')
            key = _cryptui_keyset_get_internal_keyid (keyset, value);
        g_free (value);
    }
    
    /* Use first secret key */
    if (key == NULL) {
        GList *l, *keys = cryptui_keyset_get_keys (keyset);
        for (l = keys; l; l = g_list_next (l)) {
            guint flags = cryptui_keyset_key_flags (keyset, (const gchar*)l->data);
            if (flags & CRYPTUI_FLAG_CAN_SIGN && flags & CRYPTUI_FLAG_CAN_ENCRYPT) {
                key = l->data;
                break;
            }
        }
        g_list_free (keys);
    }

    if (!key) {
        g_warning ("Encrypt to self is set, but no personal keys can be found");
        return recipients;
    }

    /* Only prepend the key if it's not already in the recipients. */
    if (!g_list_find (recipients, key))
        recipients = g_list_prepend (recipients, (gpointer)key);

    return recipients;
}

/**
 * cryptui_key_chooser_set_recipients:
 * @chooser: the chooser to set recipients on
 * @keys: the list of recipients to mark selected
 *
 * Marks the listed keys as selected in the chooser widget.
 */
void
cryptui_key_chooser_set_recipients (CryptUIKeyChooser *chooser, GList *keys)
{
    g_return_if_fail (chooser->priv->keylist != NULL);
    cryptui_key_list_set_selected_keys (chooser->priv->keylist, keys);
}

/**
 * cryptui_key_chooser_get_signer:
 * @chooser: the chooser widget to get the signer from
 *
 * Gets the key of the selected signer from the chooser widget.
 *
 * Returns: the selected signer's key
 */
const gchar*
cryptui_key_chooser_get_signer (CryptUIKeyChooser *chooser)
{
    if (chooser->priv->keycombo != NULL)
        return cryptui_key_combo_get_key (chooser->priv->keycombo);
    else if (chooser->priv->signercheck != NULL)
        return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chooser->priv->signercheck)) ? 
                (gchar*) g_object_get_data ((GObject*) (chooser->priv->signercheck), "key"):NULL;
    else
        return NULL;
}

/**
 * cryptui_key_chooser_set_signer:
 * @chooser: the chooser to set the signer on
 * @key: the signer key to set
 *
 * Sets the signer in the chooser to the provided key.
 */
void
cryptui_key_chooser_set_signer (CryptUIKeyChooser *chooser, const gchar *key)
{
    g_return_if_fail (chooser->priv->keycombo != NULL);
    cryptui_key_combo_set_key (chooser->priv->keycombo, key);
}

/**
 * cryptui_key_chooser_get_symmetric:
 * @chooser: the chooser widget to get symmetric setting from
 *
 * Gets if symmetric encryption was selected.
 *
 * Returns: TRUE if symmetric encrypted was selected, FALSE otherwise.
 */
gboolean
cryptui_key_chooser_get_symmetric (CryptUIKeyChooser *chooser)
{
	gboolean symmetric = FALSE;

	g_object_get (chooser, "symmetric", &symmetric, NULL);
	return symmetric;
}

/**
 * cryptui_key_chooser_set_symmetric:
 * @chooser: the chooser to set the signer on
 * @key: the signer key to set
 *
 * Sets the signer in the chooser to the provided key.
 */
void
cryptui_key_chooser_set_symmetric (CryptUIKeyChooser *chooser, gboolean symmetric)
{
	g_object_set (chooser, "symmetric", symmetric, NULL);
}


/**
 * cryptui_key_chooser_get_mode:
 * @chooser: the chooser to get the mode from
 *
 * Determines the mode of the given chooser widget.
 *
 * Returns: the chooser's mode
 */
CryptUIKeyChooserMode
cryptui_key_chooser_get_mode (CryptUIKeyChooser *chooser)
{
    return chooser->priv->mode;
}
