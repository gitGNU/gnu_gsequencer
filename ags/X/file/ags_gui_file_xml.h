/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_FILE_GUI_H__
#define __AGS_FILE_GUI_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <libxml/tree.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_lookup.h>

#include <ags/widget/ags_dial.h>
#include <ags/widget/ags_indicator.h>
#include <ags/widget/ags_vindicator.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_menu_bar.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>
#include <ags/X/ags_effect_pad.h>
#include <ags/X/ags_effect_line.h>
#include <ags/X/ags_editor.h>
#include <ags/X/ags_navigation.h>
#include <ags/X/ags_machine_editor.h>
#include <ags/X/ags_pad_editor.h>
#include <ags/X/ags_line_editor.h>
#include <ags/X/ags_link_editor.h>
#include <ags/X/ags_line_member_editor.h>
#include <ags/X/ags_link_collection_editor.h>
#include <ags/X/ags_resize_editor.h>

#include <ags/X/editor/ags_toolbar.h>
#include <ags/X/editor/ags_machine_selector.h>
#include <ags/X/editor/ags_machine_radio_button.h>
#include <ags/X/editor/ags_notebook.h>
#include <ags/X/editor/ags_note_edit.h>
#include <ags/X/editor/ags_pattern_edit.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>

/* GtkWidget */
void ags_file_read_widget(AgsFile *file, xmlNode *node, GtkWidget *widget);
xmlNode* ags_file_write_widget(AgsFile *file, xmlNode *parent, GtkWidget *widget);

/* AgsWindow */
void ags_file_read_window(AgsFile *file, xmlNode *node, AgsWindow **window);
xmlNode* ags_file_write_window(AgsFile *file, xmlNode *parent, AgsWindow *window);

/* AgsMenuBar */
void ags_file_read_menu_bar(AgsFile *file, xmlNode *node, AgsMenuBar **menu_bar);
xmlNode* ags_file_write_menu_bar(AgsFile *file, xmlNode *parent, AgsMenuBar *menu_bar);

/* AgsMachineCounter */
void ags_file_read_machine_counter(AgsFile *file, xmlNode *node, AgsMachineCounter **machine_counter);
xmlNode* ags_file_write_machine_counter(AgsFile *file, xmlNode *parent, AgsMachineCounter *machine_counter);

void ags_file_read_machine_counter_list(AgsFile *file, xmlNode *node, GList **machine_counter);
xmlNode* ags_file_write_machine_counter_list(AgsFile *file, xmlNode *parent, GList *machine_counter);

/* AgsMachine */
void ags_file_read_machine(AgsFile *file, xmlNode *node, AgsMachine **machine);
xmlNode* ags_file_write_machine(AgsFile *file, xmlNode *parent, AgsMachine *machine);

void ags_file_read_machine_resolve_audio(AgsFileLookup *file_lookup,
					 AgsMachine *machine);

void ags_file_read_machine_list(AgsFile *file, xmlNode *node, GList **machine);
xmlNode* ags_file_write_machine_list(AgsFile *file, xmlNode *parent, GList *machine);

/* AgsPad */
void ags_file_read_pad(AgsFile *file, xmlNode *node, AgsPad **pad);
xmlNode* ags_file_write_pad(AgsFile *file, xmlNode *parent, AgsPad *pad);

void ags_file_read_pad_list(AgsFile *file, xmlNode *node, GList **pad);
xmlNode* ags_file_write_pad_list(AgsFile *file, xmlNode *parent, GList *pad);

void ags_file_read_mixer_input_pad(AgsFile *file, xmlNode *node, AgsPad *mixer_input_pad);
xmlNode* ags_file_write_mixer_input_pad(AgsFile *file, xmlNode *parent, AgsPad *mixer_input_pad);

void ags_file_read_mixer_output_pad(AgsFile *file, xmlNode *node, AgsPad *mixer_output_pad);
xmlNode* ags_file_write_mixer_output_pad(AgsFile *file, xmlNode *parent, AgsPad *mixer_output_pad);

/* AgsLine */
void ags_file_read_line(AgsFile *file, xmlNode *node, AgsLine **line);
xmlNode* ags_file_write_line(AgsFile *file, xmlNode *parent, AgsLine *line);

void ags_file_read_line_list(AgsFile *file, xmlNode *node, GList **line);
xmlNode* ags_file_write_line_list(AgsFile *file, xmlNode *parent, GList *line);

/* AgsLineMember */
void ags_file_read_line_member(AgsFile *file, xmlNode *node, AgsLineMember **line_member);
xmlNode* ags_file_write_line_member(AgsFile *file, xmlNode *parent, AgsLineMember *line_member);

void ags_file_read_line_member_list(AgsFile *file, xmlNode *node, GList **line_member);
xmlNode* ags_file_write_line_member_list(AgsFile *file, xmlNode *parent, GList *line_member);

/* AgsEffectBridge */
void ags_file_read_effect_bridge(AgsFile *file, xmlNode *node, AgsEffectBridge **effect_bridge);
xmlNode* ags_file_write_effect_bridge(AgsFile *file, xmlNode *parent, AgsEffectBridge *effect_bridge);

void ags_file_read_effect_bridge_resolve_audio(AgsFileLookup *file_lookup,
					       AgsEffectBridge *effect_bridge);

void ags_file_read_effect_bridge_list(AgsFile *file, xmlNode *node, GList **effect_bridge);
xmlNode* ags_file_write_effect_bridge_list(AgsFile *file, xmlNode *parent, GList *effect_bridge);

/* AgsEffectBulk */
void ags_file_read_effect_bulk(AgsFile *file, xmlNode *node, AgsEffectBulk **effect_bulk);
xmlNode* ags_file_write_effect_bulk(AgsFile *file, xmlNode *parent, AgsEffectBulk *effect_bulk);

void ags_file_read_effect_bulk_list(AgsFile *file, xmlNode *node, GList **effect_bulk);
xmlNode* ags_file_write_effect_bulk_list(AgsFile *file, xmlNode *parent, GList *effect_bulk);

void ags_file_read_effect_bulk_resolve_audio(AgsFileLookup *file_lookup,
					     AgsEffectBulk *effect_bulk);

/* AgsBulkMember */
void ags_file_read_bulk_member(AgsFile *file, xmlNode *node, AgsBulkMember **bulk_member);
xmlNode* ags_file_write_bulk_member(AgsFile *file, xmlNode *parent, AgsBulkMember *bulk_member);

void ags_file_read_bulk_member_list(AgsFile *file, xmlNode *node, GList **bulk_member);
xmlNode* ags_file_write_bulk_member_list(AgsFile *file, xmlNode *parent, GList *bulk_member);

/* AgsEffectPad */
void ags_file_read_effect_pad(AgsFile *file, xmlNode *node, AgsEffectPad **effect_pad);
xmlNode* ags_file_write_effect_pad(AgsFile *file, xmlNode *parent, AgsEffectPad *effect_pad);

void ags_file_read_effect_pad_list(AgsFile *file, xmlNode *node, GList **effect_pad);
xmlNode* ags_file_write_effect_pad_list(AgsFile *file, xmlNode *parent, GList *effect_pad);

/* AgsEffectLine */
void ags_file_read_effect_line(AgsFile *file, xmlNode *node, AgsEffectLine **effect_line);
xmlNode* ags_file_write_effect_line(AgsFile *file, xmlNode *parent, AgsEffectLine *effect_line);

void ags_file_read_effect_line_list(AgsFile *file, xmlNode *node, GList **effect_line);
xmlNode* ags_file_write_effect_line_list(AgsFile *file, xmlNode *parent, GList *effect_line);

/* GtkDialog */
void ags_file_read_dialog(AgsFile *file, xmlNode *node, GtkDialog **dialog);
xmlNode* ags_file_write_dialog(AgsFile *file, xmlNode *parent, GtkDialog *dialog);

void ags_file_read_dialog_list(AgsFile *file, xmlNode *node, GList **dialog);
xmlNode* ags_file_write_dialog_list(AgsFile *file, xmlNode *parent, GList *dialog);

/* AgsMachineEditor */
void ags_file_read_machine_editor(AgsFile *file, xmlNode *node, AgsMachineEditor **machine_editor);
xmlNode* ags_file_write_machine_editor(AgsFile *file, xmlNode *parent, AgsMachineEditor *machine_editor);

void ags_file_read_machine_editor_list(AgsFile *file, xmlNode *node, GList **machine_editor);
xmlNode* ags_file_write_machine_editor_list(AgsFile *file, xmlNode *parent, GList *machine_editor);

void ags_file_read_pad_editor(AgsFile *file, xmlNode *node, AgsPadEditor **pad_editor);
xmlNode* ags_file_write_pad_editor(AgsFile *file, xmlNode *parent, AgsPadEditor *pad_editor);

void ags_file_read_line_editor(AgsFile *file, xmlNode *node, AgsLineEditor **line_editor);
xmlNode* ags_file_write_line_editor(AgsFile *file, xmlNode *parent, AgsLineEditor *line_editor);

void ags_file_read_link_editor(AgsFile *file, xmlNode *node, AgsLinkEditor **link_editor);
xmlNode* ags_file_write_link_editor(AgsFile *file, xmlNode *parent, AgsLinkEditor *link_editor);

void ags_file_read_line_member_editor(AgsFile *file, xmlNode *node, AgsLineMemberEditor **line_member_editor);
xmlNode* ags_file_write_line_member_editor(AgsFile *file, xmlNode *parent, AgsLineMemberEditor *line_member_editor);

void ags_file_read_link_collection_editor(AgsFile *file, xmlNode *node, AgsLinkCollectionEditor **link_collection_editor);
xmlNode* ags_file_write_link_collection_editor(AgsFile *file, xmlNode *parent, AgsLinkCollectionEditor *link_collection_editor);

void ags_file_read_resize_editor(AgsFile *file, xmlNode *node, AgsResizeEditor **resize_editor);
xmlNode* ags_file_write_resize_editor(AgsFile *file, xmlNode *parent, AgsResizeEditor *resize_editor);

/* AgsEditor */
void ags_file_read_editor(AgsFile *file, xmlNode *node, AgsEditor **editor);
xmlNode* ags_file_write_editor(AgsFile *file, xmlNode *parent, AgsEditor *editor);

/* AgsToolbar */
void ags_file_read_toolbar(AgsFile *file, xmlNode *node, AgsToolbar **toolbar);
xmlNode* ags_file_write_toolbar(AgsFile *file, xmlNode *parent, AgsToolbar *toolbar);

/* AgsMachineSelector */
void ags_file_read_machine_selector(AgsFile *file, xmlNode *parent, AgsMachineSelector **machine_selector);
xmlNode* ags_file_write_machine_selector(AgsFile *file, xmlNode *node, AgsMachineSelector *machine_selector);

/* AgsNotebook */
void ags_file_read_notebook(AgsFile *file, xmlNode *node, AgsNotebook **notebook);
xmlNode* ags_file_write_notebook(AgsFile *file, xmlNode *parent, AgsNotebook *notebook);

void ags_file_read_notebook_tab_list(AgsFile *file, xmlNode *node, GList **notebook_tab_list);
xmlNode* ags_file_write_notebook_tab_list(AgsFile *file, xmlNode *parent, GList *notebook_tab_list);

void ags_file_read_notebook_tab(AgsFile *file, xmlNode *node, AgsNotebookTab **notebook_tab);
xmlNode* ags_file_write_notebook_tab(AgsFile *file, xmlNode *parent, AgsNotebookTab *notebook_tab);

/* AgsNavigation */
void ags_file_read_navigation(AgsFile *file, xmlNode *node, AgsNavigation **navigation);
xmlNode* ags_file_write_navigation(AgsFile *file, xmlNode *parent, AgsNavigation *navigation);

/* AgsAutomationWindow */
void ags_file_read_automation_window(AgsFile *file, xmlNode *node, AgsAutomationWindow **automation_window);
xmlNode* ags_file_write_automation_window(AgsFile *file, xmlNode *parent, AgsAutomationWindow *automation_window);

void ags_file_read_automation_window_list(AgsFile *file, xmlNode *node, GList **automation_window_list);
xmlNode* ags_file_write_automation_window_list(AgsFile *file, xmlNode *parent, GList *automation_window_list);

/* AgsAutomationEditor */
void ags_file_read_automation_editor(AgsFile *file, xmlNode *node, AgsAutomationEditor **automation_editor);
xmlNode* ags_file_write_automation_editor(AgsFile *file, xmlNode *parent, AgsAutomationEditor *automation_editor);

void ags_file_read_automation_editor_list(AgsFile *file, xmlNode *node, GList **automation_editor_list);
xmlNode* ags_file_write_automation_editor_list(AgsFile *file, xmlNode *parent, GList *automation_editor_list);

/* AgsAutomationToolbar */
void ags_file_read_automation_toolbar(AgsFile *file, xmlNode *node, AgsAutomationToolbar **automation_toolbar);
xmlNode* ags_file_write_automation_toolbar(AgsFile *file, xmlNode *parent, AgsAutomationToolbar *automation_toolbar);

void ags_file_read_automation_toolbar_list(AgsFile *file, xmlNode *node, GList **automation_toolbar_list);
xmlNode* ags_file_write_automation_toolbar_list(AgsFile *file, xmlNode *parent, GList *automation_toolbar_list);

/* AgsAutomationEdit */
void ags_file_read_automation_edit(AgsFile *file, xmlNode *node, AgsAutomationEdit **automation_edit);
xmlNode* ags_file_write_automation_edit(AgsFile *file, xmlNode *parent, AgsAutomationEdit *automation_edit);

void ags_file_read_automation_edit_list(AgsFile *file, xmlNode *node, GList **automation_edit_list);
xmlNode* ags_file_write_automation_edit_list(AgsFile *file, xmlNode *parent, GList *automation_edit_list);

/* AgsAutomationArea */
void ags_file_read_automation_area(AgsFile *file, xmlNode *node, AgsAutomationArea **automation_area);
xmlNode* ags_file_write_automation_area(AgsFile *file, xmlNode *parent, AgsAutomationArea *automation_area);

void ags_file_read_automation_area_list(AgsFile *file, xmlNode *node, GList **automation_area_list);
xmlNode* ags_file_write_automation_area_list(AgsFile *file, xmlNode *parent, GList *automation_area_list);

#endif /*__AGS_FILE_GUI_H__*/
