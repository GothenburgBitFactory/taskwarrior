////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_STRINGS
#define INCLUDED_STRINGS

#define STRING_COLUMN_LABEL_DATE     "Datum"
#define STRING_COLUMN_LABEL_DEFAULT  "Standard"
#define STRING_COLUMN_LABEL_DUE      "Fällig"
#define STRING_COLUMN_LABEL_END      "Ende"
#define STRING_COLUMN_LABEL_ENTERED  "Erfasst"
#define STRING_COLUMN_LABEL_LABEL    "Beschreibung"
#define STRING_COLUMN_LABEL_LAST     "Last instance"
#define STRING_COLUMN_LABEL_MOD      "Geändert"
#define STRING_COLUMN_LABEL_NAME     "Name"
#define STRING_COLUMN_LABEL_ORPHAN   "Verwaiste UDA"
#define STRING_COLUMN_LABEL_PROJECT  "Projekt"
#define STRING_COLUMN_LABEL_RECUR    "Wiederh."
#define STRING_COLUMN_LABEL_RECUR_L  "Wiederholung"
#define STRING_COLUMN_LABEL_RTYPE    "Recurrence type"
#define STRING_COLUMN_LABEL_SCHED    "Geplant"
#define STRING_COLUMN_LABEL_STAT     "St"
#define STRING_COLUMN_LABEL_STATUS   "Status"
#define STRING_COLUMN_LABEL_STAT_C   "E"
#define STRING_COLUMN_LABEL_STAT_CO  "Erledigt"
#define STRING_COLUMN_LABEL_STAT_D   "G"
#define STRING_COLUMN_LABEL_STAT_DE  "Gelöscht"
#define STRING_COLUMN_LABEL_STAT_P   "O"
#define STRING_COLUMN_LABEL_STAT_PE  "Offen"
#define STRING_COLUMN_LABEL_STAT_R   "W"
#define STRING_COLUMN_LABEL_STAT_RE  "Wiederholend"
#define STRING_COLUMN_LABEL_STAT_W   "A"
#define STRING_COLUMN_LABEL_STAT_WA  "Wartet"
#define STRING_COLUMN_LABEL_TAG      "Schlagwort"
#define STRING_COLUMN_LABEL_TAGS     "Schlagworte"
#define STRING_COLUMN_LABEL_UDA      "Name"
#define STRING_COLUMN_LABEL_UDACOUNT "Nutzungshäufigkeit"
#define STRING_COLUMN_LABEL_UNTIL    "Bis"
#define STRING_COLUMN_LABEL_URGENCY  "Dringlichkeit"
#define STRING_COLUMN_LABEL_UUID     "UUID"
#define STRING_COLUMN_LABEL_VALUE    "Wert"
#define STRING_COLUMN_LABEL_DATE     "Datum"
#define STRING_COLUMN_LABEL_COLUMN   "Spalten"
#define STRING_COLUMN_LABEL_STYLES   "Unterstützte Formate"
#define STRING_COLUMN_LABEL_EXAMPLES "Beispiele"
#define STRING_COLUMN_LABEL_SCHED    "Geplant"
#define STRING_COLUMN_LABEL_UDA      "Name"
#define STRING_COLUMN_LABEL_TYPE     "Typ"
#define STRING_COLUMN_LABEL_MODIFY   "Modifiable"
#define STRING_COLUMN_LABEL_NOMODIFY "Read Only"
#define STRING_COLUMN_LABEL_LABEL    "Beschreibung"
#define STRING_COLUMN_LABEL_DEFAULT  "Standard"
#define STRING_COLUMN_LABEL_VALUES   "Erlaubte Werte"
#define STRING_COLUMN_LABEL_WAIT     "Aufgeschoben"
#define STRING_COLUMN_LABEL_WAITING  "Aufgeschoben bis"
#define STRING_FEEDBACK_ANN_ADD      "Kommentar von '{1}' hinzugefügt."
#define STRING_FEEDBACK_ANN_DEL      "Kommentar '{1}' gelöscht."
#define STRING_FEEDBACK_ANN_WAS_MOD  "Kommentar zu '{1}' geändert."
#define STRING_FEEDBACK_ATT_DEL      "{1} wird gelöscht."
#define STRING_FEEDBACK_ATT_DEL_DUR  "{1} wird gelöscht (Dauer: {2})."
#define STRING_FEEDBACK_ATT_MOD      "{1} wird von '{2}' zu '{3}' geändert."
#define STRING_FEEDBACK_ATT_SET      "{1} wird auf '{2}' gesetzt."
#define STRING_FEEDBACK_ATT_WAS_MOD  "{1} wurde von '{2}' zu '{3}' geändert."
#define STRING_FEEDBACK_ATT_WAS_SET  "{1} wurde auf '{2}' gesetzt."
#define STRING_FEEDBACK_BACKLOG      "{1} Lokale Änderungen.  Datenabgleich erforderlich."
#define STRING_FEEDBACK_BACKLOG_N    "{1} Lokale Änderungen.  Datenabgleich erforderlich."
#define STRING_FEEDBACK_DELETED      "{1} wird gelöscht."
#define STRING_FEEDBACK_DEP_DEL      "Abhängigkeit '{1}' wird gelöscht."
#define STRING_FEEDBACK_DEP_MOD      "Abhängigkeit wird von '{1}' zu '{2}' geändert."
#define STRING_FEEDBACK_DEP_SET      "Abhängigkeit wird auf '{1}' gesetzt."
#define STRING_FEEDBACK_DEP_WAS_MOD  "Dependencies wurde von '{1}' zu '{2}' geändert."
#define STRING_FEEDBACK_DEP_WAS_SET  "Abhängigkeit wurde auf '{1}' gesetzt."
#define STRING_FEEDBACK_EXPIRED      "Aufgabe {1} '{2}' ist abgelaufen und wurde gelöscht."
#define STRING_FEEDBACK_NOP          "Keine Änderungen werden durchgeführt."
#define STRING_FEEDBACK_NO_MATCH     "Keine treffer."
#define STRING_FEEDBACK_NO_TASKS     "Keine Aufgaben."
#define STRING_FEEDBACK_TAG_NEXT     "Das besondere Schlagwort 'next' erhöht die Dringlichkeit dieser Aufgabe, sodass sie im 'next'-Report erscheint."
#define STRING_FEEDBACK_TAG_NOCAL    "Das besondere Schlagwort 'nocal' verhindert, dass diese Aufgabe im 'calendar'-Report erscheint."
#define STRING_FEEDBACK_TAG_NOCOLOR  "Das besondere Schlagwort 'nocolor' deaktiviert Farb-Regeln für diese Aufgabe."
#define STRING_FEEDBACK_TAG_NONAG    "Das besondere Schlagwort 'nonag' verhindert Nachfragen, wenn diese Aufgabe geändert wird."
#define STRING_FEEDBACK_TAG_VIRTUAL  "Virtual tags (including '{1}') are reserved and may not be added or removed."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} Aufgaben)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 Aufgabe)"
#define STRING_FEEDBACK_UNBLOCKED    "Aufgabe {1} '{2}' entsperrt."
#define STRING_FEEDBACK_WAS_NOP      "Keine Änderungen wurden durchgeführt."
#define STRING_TASK_DEPEND_CIRCULAR  "Verbotene zyklische Abhängigkeit erkannt."
#define STRING_TASK_DEPEND_DUP       "Aufgabe {1} hängt bereits von Aufgabe {2} ab."
#define STRING_TASK_DEPEND_ITSELF    "Eine Aufgabe kann nicht von sich selbst abhängen."
#define STRING_TASK_DEPEND_MISS_CREA "Konnte keine Abhängigkeit von Aufgabe {1} erstellen - Aufgabe nicht gefunden."
#define STRING_TASK_DEPEND_MISS_DEL  "Konnte keine Abhängigkeit zu Aufgabe {1} löschen - nicht gefunden."
#define STRING_TASK_INVALID_COL_TYPE "Nicht erkannter Typ '{1}' für Spalte '{2}'"
#define STRING_TASK_NO_DESC          "Kommentar fehlt Beschreibung: {1}"
#define STRING_TASK_NO_ENTRY         "Kommentar fehlt Erfassungsdatum: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior unterstützt Datei-Format 1 nicht mehr, welches ursprünglich zwischen dem 27. November 2006 und 31. Dezember 2007 eingesetzt wurde."
#define STRING_TASK_NO_FF2           "Taskwarrior unterstützt Datei-Format 2 nicht mehr, welches ursprünglich zwischen dem 1. Januar 2008 und 12. April 2009 eingesetzt wurde."
#define STRING_TASK_NO_FF3           "Taskwarrior unterstützt Datei-Format 3 nicht mehr, welches ursprünglich zwischen dem 23. März 2009 und 16. Mai 2009 eingesetzt wurde."
#define STRING_TASK_PARSE_UNREC_FF   "Nicht erkanntes taskwarrior-Dateiformat."
#define STRING_TASK_SAFETY_ALLOW     "Kein Filter angegeben, und durch den konfigurierten 'allow.empty.filter'-Wert wurde keine Aktion durchgeführt."
#define STRING_TASK_SAFETY_FAIL      "Befehl an Ausführung gehindert."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Warnung: Das '{1}'-Datum ist nach dem '{2}'-Datum."
#define STRING_TASK_VALID_BLANK      "Leere Aufgaben können nicht angelegt werden."
#define STRING_TASK_VALID_REC_DUE    "Wiederholende Aufgaben müssen eine Fälligkeit besitzen."
tdefine STRING_COLUMN_LABEL_START    "Beginn"

#endif
