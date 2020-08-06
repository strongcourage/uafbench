;;; rec-mode.el --- Major mode for viewing/editing rec files

;; Copyright (C) 2009-2019 Jose E. Marchesi

;; Maintainer: Jose E. Marchesi

;; This file is NOT part of GNU Emacs.

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;;; Commentary:

;; A major mode for editing rec files.
;;
;; To see the structure of this file activate the outline minor mode
;; and execute M-xhide-body

;;; Code:

(require 'compile)
(eval-when-compile (require 'cl))
(require 'calendar)
(require 'hl-line)
(require 'tabulated-list)

;;;; Customization

(defgroup rec-mode nil
  "rec-mode subsystem"
  :group 'applications
  :link '(url-link "http://www.gnu.org/software/recutils"))

(defcustom rec-open-mode 'navigation
  "Default mode to use when switching a buffer to rec-mode.
Valid values are `edit' and `navigation'.  The default is `navigation'"
  :type 'symbol
  :group 'rec-mode)

(defcustom rec-popup-calendar t
  "Whether to use a popup calendar to select dates when editing field
values.  The default is `t'."
  :type 'boolean
  :group 'rec-mode)

(defcustom rec-mode-hook nil
  "Hook run when entering rec mode."
  :type 'hook
  :group 'rec-mode)

(defvar rec-max-lines-in-fields 15
  "Values of fields having more than the specified lines will be
hidden by default in navigation mode.")

(defvar rec-recsel "recsel"
  "Name of the 'recsel' utility from the GNU recutils.")

(defvar rec-recinf "recinf"
  "Name of the 'recinf' utility from the GNU recutils.")

(defvar rec-recfix "recfix"
  "Name of the 'recfix' utility from the GNU recutils.")

(defface rec-field-name-face '((t :weight bold)) "" :group 'rec-mode)
(defface rec-keyword-face '((t :weight bold)) "" :group 'rec-mode)
(defface rec-continuation-line-face '((t :weight bold)) "" :group 'rec-mode)

;;;; Variables and constants that the user does not want to touch (really!)

(defconst rec-mode-version "1.5"
  "Version of rec-mode.el")

(defconst rec-keyword-prefix "%"
  "Prefix used to distinguish special fields.")

(defconst rec-keyword-rec (concat rec-keyword-prefix "rec")
  ;; Remember to update `rec-font-lock-keywords' if you change this
  ;; value!!
  "Rec keyword.")

(defconst rec-keyword-key (concat rec-keyword-prefix "key")
  "Key keyword.")

(defconst rec-keyword-mandatory (concat rec-keyword-prefix "mandatory")
  "Mandatory keyword.")

(defconst rec-keyword-summary (concat rec-keyword-prefix "summary")
  "Summary keyword.")

(defconst rec-time-stamp-format "%Y-%m-%d %a %H:%M"
  "Format for `format-time-string' which is used for time stamps.")

(defvar rec-comment-re "^#.*"
  "regexp denoting a comment line")

(defvar rec-comment-field-re "^\\(#.*\n\\)*\\([a-zA-Z0-1_%-]+:\\)+"
  "regexp denoting the beginning of a record")

(defvar rec-field-name-re
  "^[a-zA-Z%][a-zA-Z0-9_]*:"
  "Regexp matching a field name")

(defvar rec-field-value-re
  (let ((ret-re "\n\\+ ?")
        (esc-ret-re "\\\\\n"))
    (concat
     "\\("
     "\\(" ret-re "\\)*"
     "\\(" esc-ret-re "\\)*"
     "\\(" "\\\\[^\n]" "\\)*"
     "[^\\\n]*"
     "\\)*"))
  "Regexp matching a field value")

(defvar rec-field-re
  (concat rec-field-name-re
          rec-field-value-re
          "\n")
  "Regexp matching a field")

(defvar rec-record-re
  (concat rec-field-re "\\(" rec-field-re "\\|" rec-comment-re "\\)*")
  "Regexp matching a record")

(defvar rec-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?# "<" st)   ; Comment start
    (modify-syntax-entry ?\n ">" st)  ; Comment end
    (modify-syntax-entry ?\" "w" st)
    (modify-syntax-entry ?\' "w" st)
    st)
  "Syntax table used in rec-mode")

(defvar rec-font-lock-keywords
`((,(concat "^" rec-keyword-prefix "[a-zA-Z0-9_]+:") . rec-field-name-face)
  (,rec-field-name-re . rec-keyword-face)
  ("^\\+" . rec-continuation-line-face))
"Font lock keywords used in rec-mode")

(defvar rec-mode-edit-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-cn" 'rec-cmd-goto-next-rec)
    (define-key map "\C-cp" 'rec-cmd-goto-previous-rec)
    (define-key map "\C-ce" 'rec-cmd-edit-field)
    (define-key map "\C-cd" 'rec-cmd-show-descriptor)
    (define-key map "\C-ct" 'rec-cmd-show-type)
    (define-key map "\C-c#" 'rec-cmd-count)
    (define-key map "\C-cm" 'rec-cmd-trim-field-value)
    (define-key map "\C-cc" 'rec-cmd-compile)
    (define-key map "\C-csq" 'rec-cmd-select-fast)
    (define-key map "\C-css" 'rec-cmd-select-sex)
    (define-key map "\C-ch" 'rec-cmd-show-summary)
    (define-key map "\C-cI" 'rec-cmd-show-info)
    (define-key map "\C-cf\C-w" 'rec-cmd-kill-field)
    (define-key map "\C-cf\M-w" 'rec-cmd-copy-field)
    (define-key map "\C-cr\C-w" 'rec-cmd-kill-record)
    (define-key map "\C-cr\M-w" 'rec-cmd-copy-record)
    (define-key map (kbd "TAB") 'rec-cmd-goto-next-field)
    (define-key map "\C-cb" 'rec-cmd-jump-back)
    (define-key map "\C-c\C-c" 'rec-finish-editing)
    map)
  "Keymap for rec-mode")

(defvar rec-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "n" 'rec-cmd-goto-next-rec)
    (define-key map "p" 'rec-cmd-goto-previous-rec)
    (define-key map "e" 'rec-cmd-edit-field)
    (define-key map "R" 'rec-edit-record)
    (define-key map "T" 'rec-edit-type)
    (define-key map "B" 'rec-edit-buffer)
    (define-key map "A" 'rec-cmd-append-field)
    (define-key map "I" 'rec-cmd-show-info)
    (define-key map "d" 'rec-cmd-show-descriptor)
    (define-key map "t" 'rec-cmd-show-type)
    (define-key map "m" 'rec-cmd-trim-field-value)
    (define-key map "c" 'rec-cmd-compile)
    (define-key map "f\C-w" 'rec-cmd-kill-field)
    (define-key map "f\M-w" 'rec-cmd-copy-field)
    (define-key map "r\C-w" 'rec-cmd-kill-record)
    (define-key map "r\M-w" 'rec-cmd-copy-record)
    (define-key map "f\C-w" 'rec-cmd-kill-field)
    (define-key map "f\M-w" 'rec-cmd-copy-field)
    (define-key map "sq" 'rec-cmd-select-fast)
    (define-key map "ss" 'rec-cmd-select-sex)
    (define-key map "h" 'rec-cmd-show-summary)
    (define-key map "\C-ct" 'rec-find-type)
;;    (define-key map [remap move-beginning-of-line] 'rec-cmd-beginning-of-line)
    (define-key map [remap undo] 'rec-cmd-undo)
    (define-key map "#" 'rec-cmd-count)
    (define-key map "%" 'rec-cmd-statistic)
    (define-key map (kbd "RET") 'rec-cmd-jump)
    (define-key map (kbd "TAB") 'rec-cmd-goto-next-field)
    (define-key map (kbd "SPC") 'rec-cmd-toggle-field-visibility)
    (define-key map "b" 'rec-cmd-jump-back)
    map)
  "Keymap for rec-mode")

;;;; Parsing functions (rec-parse-*)
;;
;; Those functions read the contents of the buffer (starting at the
;; current position of the pointer) and try to parse field, comment
;; and records structures.

(defun rec-parse-comment ()
  "Parse and return a comment starting at point.

Return a list whose first element is the symbol 'comment and the
second element is the string with the contents of the comment,
including the leading #:

   (comment POSITION \"# foo\")

If the point is not at the beginning of a comment then return nil"
  (let ((there (point))
        comment)
    (when (and (equal (current-column) 0)
               (looking-at rec-comment-re))
      (setq comment (list 'comment
                          there
                          (buffer-substring-no-properties (match-beginning 0)
                                                          (match-end 0))))
      (goto-char (match-end 0))
      ;; Skip a newline if needed
      (when (looking-at "\n") (goto-char (match-end 0))))
    comment))

(defun rec-parse-field-name ()
  "Parse and return a field name starting at point.
If the point is not at the beginning of a field name return nil"
  (when (and (equal (current-column) 0)
             (looking-at rec-field-name-re))
    (goto-char (match-end 0))
    (buffer-substring-no-properties (match-beginning 0)
                                    (- (match-end 0) 1))))

(defun rec-parse-field-value ()
  "Return the field value under the pointer.

Return a string containing the value of the field.

If the pointer is not at the beginning of a field value, return
nil"
  (when (looking-at rec-field-value-re)
    (goto-char (match-end 0))
    (let ((val (buffer-substring-no-properties (match-beginning 0)
                                               (match-end 0))))
      ;; Replace escaped newlines
      (setq val (replace-regexp-in-string "\\\\\n" "" val))
      ;; Replace continuation lines
      (setq val (replace-regexp-in-string "\n\\+ ?" "\n" val))
      ;; Remove the initial blank
      (with-temp-buffer
        (insert val)
        (goto-char (point-min))
        (if (equal (char-after (point)) ? )
            (progn
              (delete-char 1)))
        (setq val (buffer-substring-no-properties (point-min)
                                                  (point-max))))
      val)))

(defun rec-parse-field ()
  "Return a structure describing the field starting from the
pointer.

The returned structure is a list whose first element is the
symbol 'field', the second element is the name of the field and
the second element is the value of the field:

   (field POSITION FIELD-NAME FIELD-VALUE)

If the pointer is not at the beginning of a field
descriptor then return nil"
  (let ((there (point))
        field-name field-value)
    (and (setq field-name (rec-parse-field-name))
         (setq field-value (rec-parse-field-value)))
    (when (and field-name field-value)
        ;; Skip a newline if needed
        (when (looking-at "\n") (goto-char (match-end 0)))
        (list 'field there field-name field-value))))

(defun rec-parse-record ()
  "Return a structure describing the record starting from the pointer.

The returned structure is a list of fields preceded by the symbol
'record':

   (record POSITION (FIELD-1 FIELD-2 ... FIELD-N))

If the pointer is not at the beginning of a record, then return
nil"
  (let ((there (point))
        record field-or-comment)
    (while (setq field-or-comment (or (rec-parse-field)
                                      (rec-parse-comment)))
      (setq record (cons field-or-comment record)))
    (setq record (list 'record there (reverse record)))))

;;;; Writer functions (rec-insert-*)
;;
;; Those functions dump the written representation of the parser
;; structures (field, comment, record, etc) into the current buffer
;; starting at the current position.

(defun rec-insert-comment (comment)
  "Insert the written form of COMMENT in the current buffer"
  (when (rec-comment-p comment)
    (insert (rec-comment-string comment))))

(defun rec-insert-field-name (field-name)
  "Insert the written form of FIELD-NAME in the current buffer"
  (when (stringp field-name)
    (insert (concat field-name ":"))
    t))

(defun rec-insert-field-value (field-value)
  "Insert the written form of FIELD-VALUE in the current buffer"
  (when (stringp field-value)
    (let ((val field-value))
      ;; FIXME: Maximum line size
      (insert (replace-regexp-in-string "\n" "\n+ " val)))
    (insert "\n")))

(defun rec-insert-field (field)
  "Insert the written form of FIELD in the current buffer"
  (when (rec-field-p field)
    (when (rec-insert-field-name (rec-field-name field))
      (insert " ")
      (rec-insert-field-value (rec-field-value field)))))

(defun rec-insert-record (record)
  "Insert the written form of RECORD in the current buffer."
  (when (rec-record-p record)
    (mapcar (lambda (elem)
              (cond
               ((rec-comment-p elem) (rec-insert-comment elem))
               ((rec-field-p elem) (rec-insert-field elem))))
            (rec-record-elems record))))

;;;; Operations on record structures
;;
;; Those functions retrieve or set properties of field structures.

(defun rec-record-p (record)
  "Determine if the provided structure is a record."
  (and (listp record)
       (= (length record) 3)
       (equal (car record) 'record)))

(defun rec-record-position (record)
  "Return the start position of the given record."
  (when (rec-record-p record)
    (nth 1 record)))

(defun rec-record-elems (record)
  "Return a list with the elements of the given record."
  (when (rec-record-p record)
    (nth 2 record)))

(defun rec-record-descriptor-p (record)
  "Determine if the given record is a descriptor."
  (not (null (rec-record-assoc rec-keyword-rec record))))

(defun rec-record-assoc (name record)
  "Get a list with the values of the fields in RECORD named NAME.

NAME shall be a field name.
If no such field exists in RECORD then nil is returned."
  (when (rec-record-p record)
    (let (result)
      (mapc (lambda (field)
              (when (and (rec-field-p field)
                         (equal name (rec-field-name field)))
                (setq result (cons (rec-field-value field) result))))
            (rec-record-elems record))
      (reverse result))))

(defun rec-record-names (record)
  "Get a list of the field names in the record"
  (when (rec-record-p record)
    (let (result)
      (mapc (lambda (field)
              (when (rec-field-p field)
                (setq result (cons (rec-field-name field) result))))
            (rec-record-elems record))
      (reverse result))))

(defun rec-record-values (record fields)
  "Given a list of field names, return a list of the values."
  (when fields
    (append (rec-record-assoc (car fields) record)
            (rec-record-values record (cdr fields)))))

;;;; Operations on comment structures
;;
;; Those functions retrieve or set properties of comment structures.

(defun rec-comment-p (comment)
  "Determine if the provided structure is a comment"
  (and (listp comment)
       (= (length comment) 3)
       (equal (car comment) 'comment)))

(defun rec-comment-position (comment)
  "Return the start position of the given comment."
  (when (rec-comment-p comment)
    (nth 1 comment)))

(defun rec-comment-string (comment)
  "Return the string composig the comment, including the initial '#' character."
  (when (rec-comment-p comment)
    (nth 2 comment)))

;;;; Operations on field structures
;;
;; Those functions retrieve or set properties of field structures.

(defun rec-field-p (field)
  "Determine if the provided structure is a field"
  (and (listp field)
       (= (length field) 4)
       (equal (car field) 'field)))

(defun rec-field-position (field)
  "Return the start position of the given field."
  (when (rec-field-p field)
    (nth 1 field)))

(defun rec-field-name (field)
  "Return the name of the provided field"
  (when (rec-field-p field)
    (nth 2 field)))

(defun rec-field-value (field)
  "Return the value of the provided field"
  (when (rec-field-p field)
    (nth 3 field)))

(defun rec-field-set-value (field value)
  "Return FIELD with its value replaced by VALUE."
  (list 'field
        (rec-field-position field)
        (rec-field-name field)
        value))

(defun rec-field-trim-value (field)
  "Return FIELD with its value trimmed."
  (when (rec-field-p field)
    (let ((value (rec-field-value field))
          c)
      (with-temp-buffer
        (insert value)
        (goto-char (point-min))
        (when (looking-at "[ \t\n][ \t\n]+")
          (delete-region (match-beginning 0)
                         (match-end 0)))
        (goto-char (point-max))
        (setq c (char-before))
        (while (and c
                    (or (equal c ?\n)
                        (equal c ?\t)
                        (equal c ? )))
          (backward-char)
          (setq c (char-before)))
        (delete-region (point) (point-max))
        (setq value (buffer-substring-no-properties (point-min)
                                                    (point-max))))
      (rec-field-set-value field value))))

;;;; Get entities under pointer
;;
;; Those functions retrieve structures of the entities under pointer
;; like comments, fields and records.  If the especified entity is not
;; under the pointer then nil is returned.

(defun rec-beginning-of-field-pos ()
  "Return the position of the beginning of the current field, or
nil if the pointer is not on a field."
  (save-excursion
    (beginning-of-line)
    (let (res exit)
      (while (not exit)
        (cond
         ((and (not (= (line-beginning-position) 1))
               (or (looking-at "+")
                   (looking-back "\\\\\n" 2)))
          (forward-line -1))
         ((looking-at rec-field-name-re)
          (setq res (point))
          (setq exit t))
         (t
          (setq exit t))))
      res)))

(defun rec-end-of-field-pos ()
  "Return the position of the end of the current field, or nil if
the pointer is not on a field."
  (let ((begin-pos (rec-beginning-of-field-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        ;; The following hack is due to the fact that
        ;; the regular expressions search engine is
        ;; hanging on rec-field-re
        (when (looking-at rec-field-name-re)
          (goto-char (match-end 0)))
        (when (looking-at rec-field-value-re)
          (match-end 0))))))

(defun rec-beginning-of-comment-pos ()
  "Return the position of the beginning of the current comment,
or nil if the pointer is not on a comment."
  (save-excursion
    (beginning-of-line)
    (when (looking-at rec-comment-re)
      (point))))

(defun rec-end-of-comment-pos ()
  "Return the position of the end of the current comment,
or nil if the pointer is not on a comment."
  (let ((begin-pos (rec-beginning-of-comment-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        (when (looking-at rec-comment-re)
          (match-end 0))))))

(defun rec-beginning-of-record-pos ()
  "Return the position of the beginning of the current record, or nil if
the pointer is not on a record."
  (save-excursion
    (let (field-pos prev-pos)
      (setq prev-pos (point))
      (while (and (not (equal (point) (point-min)))
                  (or (setq field-pos (rec-beginning-of-field-pos))
                      (setq field-pos (rec-beginning-of-comment-pos))))
        (goto-char field-pos)
        (if (not (equal (point) (point-min)))
            (backward-char)))
      (unless (or (eobp)
                  (looking-at rec-comment-field-re))
          (forward-char))
      (when (looking-at rec-comment-field-re)
        (point)))))

(defun rec-end-of-record-pos ()
  "Return the position of the end of the current record,
or nil if the pointer is not on a record."
  (let ((begin-pos (rec-beginning-of-record-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        (while (or (looking-at rec-field-name-re)
                   (looking-at rec-comment-re))
          (goto-char (match-end 0))
          (when (or (looking-at rec-field-value-re)
                    (looking-at rec-comment-re))
            (goto-char (+ (match-end 0) 1))))
        (point)))))

(defun rec-current-field ()
  "Return a structure with the contents of the current field.
The current field is the field where the pointer is."
  (save-excursion
    (let ((begin-pos (rec-beginning-of-field-pos)))
      (when begin-pos
        (goto-char begin-pos)
        (rec-parse-field)))))

(defun rec-current-field-type ()
  "Return the field type of the field under point, if any."
  (let ((current-field (rec-current-field)))
    (when current-field
      (rec-field-type (rec-field-name current-field)))))

(defun rec-current-record ()
  "Return a structure with the contents of the current record.
The current record is the record where the pointer is"
  (save-excursion
    (let ((begin-pos (rec-beginning-of-record-pos)))
      (when begin-pos
        (goto-char begin-pos)
        (rec-parse-record)))))

;;;; Visibility
;;
;; These functions manage the visibility in the rec buffer.

(defun rec-narrow-to-record ()
  "Narrow to the current record, if any"
  (let ((begin-pos (rec-beginning-of-record-pos))
        (end-pos (rec-end-of-record-pos)))
    (if (and begin-pos end-pos)
        (narrow-to-region begin-pos end-pos))))

(defun rec-narrow-to-type (type)
  "Narrow to the specified type, if any"
  (let ((begin-pos (or (rec-type-pos type) (point-min)))
        (end-pos (or (rec-type-pos (rec-type-next type)) (point-max))))
    (narrow-to-region begin-pos end-pos)))

;;;; Record collection management
;;
;; These functions perform the management of the collection of records
;; in the buffer.

(defun rec-buffer-valid-p ()
  "Determine whether the current buffer contains valid rec data."
  (equal (call-process-region (point-min) (point-max)
                              rec-recinf
                              nil ; delete
                              nil ; discard output
                              nil ; display
                              ) 0))

(defun rec-update-buffer-descriptors-and-check (&optional dont-go-fundamental)
  "Update buffer descriptors and switch to fundamental mode if
there is a parse error."
  (if (rec-buffer-valid-p)
      (progn
        (rec-update-buffer-descriptors)
        t)
    (unless dont-go-fundamental
      (fundamental-mode))
    (let* ((cur-buf (current-buffer))
           (errmsg (with-temp-buffer
                     (let ((err-buf (current-buffer)))
                       (with-current-buffer cur-buf
                         (call-process-region (point-min) (point-max)
                                              rec-recfix
                                              nil ; delete
                                              (list err-buf t)
                                              nil ; display
                                              "--check")))
                     (goto-char (point-min))
                     (when (looking-at "stdin: ")
                       (delete-region (point-min) (match-end 0)))
                     (goto-char (point-max))
                     (when (equal (char-after) ?\n)
                       (delete-char 1))
                     (buffer-substring (point-min) (- (point-max) 1)))))
      (message (concat (buffer-name) ": " errmsg))
      nil)))

(defun rec-update-buffer-descriptors ()
  "Get a list of the record descriptors in the current buffer.

If the contents of the current buffer are not valid rec data then
this function returns `nil'."
  (setq rec-buffer-descriptors
	(let ((buffer (generate-new-buffer "Rec Inf "))
	      descriptors records status)
          ;; Call 'recinf' to get the list of record descriptors in
          ;; sexp format.
          (setq status (call-process-region (point-min) (point-max)
                                            rec-recinf
                                            nil ; delete
                                            buffer
                                            nil ; display
                                            "-S" "-d"))
          (if (equal status 0)
              (progn (with-current-buffer buffer
                       (goto-char (point-min))
                       (insert "(")
                       (goto-char (point-max))
                       (insert ")")
                       (unwind-protect
                           (setq descriptors (read (point-min-marker)))
                         (kill-buffer buffer)))
                     (when descriptors
                       (mapc (lambda (descriptor)
                               (let ((marker (make-marker)))
                                 (set-marker marker (rec-record-position descriptor))
                                 (setq records (cons (list 'descriptor descriptor marker)
                                                     records))))
                             descriptors)
                       (reverse records)))
            (kill-buffer buffer)
            nil))))

(defun rec-buffer-types ()
  "Return a list with the names of the record types in the
existing buffer."
  ;; If a descriptor has more than a %rec field, then the first one is
  ;; used.  The rest are ignored.
  (mapcar
   (lambda (elem)
     (car (rec-record-assoc rec-keyword-rec (cadr elem))))
   rec-buffer-descriptors))

(defun rec-type-p (type)
  "Determine if there are records of type TYPE in the current
file."
  (member type (rec-buffer-types)))

(defun rec-goto-type (type)
  "Goto the beginning of the descriptor with type TYPE.

If there are records of type TYPE in the record set then goto the
first record.  Otherwise goto to the record descriptor.

If the type do not exist in the current buffer then
this function returns nil."
  (if (or (not type)
          (equal type ""))
      ;; If there is a regular record in the
      ;; beginning of the file, go there.
      (if (save-excursion
            (goto-char (point-min))
            (unless (looking-at rec-comment-field-re)
              (rec-goto-next-rec))
            (rec-regular-p))
          (progn
            (goto-char (point-min))
            (unless (looking-at rec-comment-field-re)
              (rec-goto-next-rec))
            t)
        nil)
    (let (found
          (descriptors rec-buffer-descriptors))
      (mapc
       (lambda (elem)
         (when (equal (car (rec-record-assoc rec-keyword-rec
                                             (cadr elem)))
                      type)
           (setq found t)
           (goto-char (nth 2 elem))))
       descriptors)
      found)))

(defun rec-type-pos (type)
  "Return the position where the records of type TYPE start in
the current file.  If no records of type TYPE are defined in the
current file then return nil."
  (when (rec-type-p type)
    (save-excursion
      (rec-goto-type type)
      (point))))

(defun rec-type-next (type)
  "Return the name of the type following TYPE in the file, if
any.  If the specified type is the last appearing in the file,
or the specified type does not exist, then return nil."
  (let ((types (member type (rec-buffer-types))))
    (nth 1 types)))

(defun rec-type-previous (type)
  "Return the name of the type preceding TYPE in the file, if
any.  If the specified type is the first appearing in the file,
or the specified type does not exist, then return nil."
  (let ((types (member type (reverse (rec-buffer-types)))))
    (nth 1 types)))

(defun rec-goto-next-field ()
  "Move the pointer to the beginning of the next field in the
file."
  (let ((pos (save-excursion
               (rec-end-of-field)
               (when (re-search-forward rec-field-name-re nil t)
                 (match-beginning 0)))))
    (when pos
      (goto-char pos)
      t)))

(defun rec-goto-next-rec ()
  "Move the pointer to the beginning of the next record in the
file."
  (let ((pos (save-excursion
               (rec-end-of-record)
               (when (re-search-forward rec-comment-field-re nil t)
                 (match-beginning 0)))))
    (when pos
        (goto-char pos)
        t)))

(defun rec-goto-previous-rec ()
  "Move the pointer to the end of the previous record in the
file."
    (let ((pos (save-excursion
                 (rec-beginning-of-record)
                 (if (not (= (point) (point-min)))
                     (backward-char))
                 (when (and (re-search-backward rec-record-re nil t)
                            (rec-beginning-of-record))
                   (point)))))
      (when pos
        (goto-char pos)
        t)))

(defun rec-type-first-rec-pos (type)
  "Return the position of the first record of the specified TYPE.

If TYPE is nil then return the position of the first regular record in the file.
If there are no regular records in the file, return nil."
  (save-excursion
    (when (or (not type) (rec-type-p type))
      (if type
          (rec-goto-type type)
        (goto-char (point-min)))
      ;; Find the next regular record
      (when (and (rec-goto-next-rec)
                 (rec-regular-p))
        (point)))))

(defun rec-goto-type-first-rec (type)
  "Goto to the first record of type TYPE present in the file.
If TYPE is nil then goto to the first Unknown record on the file.

If the record is found, return its position.
If no such record exist then don't move and return nil."
  (let ((pos (rec-type-first-rec-pos type)))
    (when pos
      (goto-char pos))))

(defun rec-count (&optional type sex)
  "If TYPE is a string, return the number of records of the
specified type in the current file."
  (let (num
        (rec-file-name (if buffer-file-name
                           buffer-file-name
                         "")))
    (with-temp-buffer
      (if (stringp type)
          (if (stringp sex)
            (call-process rec-recsel
                          nil ; infile
                          t   ; output to current buffer.
                          nil ; display
                          "-t" type "-e" sex "-c" rec-file-name)
            (call-process rec-recsel
                          nil ; infile
                          t   ; output to current buffer.
                          nil ; display
                          "-t" type "-c" rec-file-name))
        (if (stringp sex)
          (call-process rec-recsel
                        nil ; infile
                        t   ; output to current buffer.
                        nil ; display
                        "-e" sex "-c" rec-file-name)
          (call-process rec-recsel
                        nil ; infile
                        t   ; output to current buffer.
                        nil ; display
                        "-c" rec-file-name)))
      (setq num (buffer-substring-no-properties (point-min) (point-max))))
    (string-to-number num)))

(defun rec-regular-p ()
  "Return t if the record under point is a regular record.
Return nil otherwise."
  (let ((rec (rec-current-record)))
    (when rec
      (= (length (rec-record-assoc rec-keyword-rec rec))
         0))))

(defun rec-record-type ()
  "Return the type of the record under point.

If the record is of no known type, return nil."
  (let ((descriptor (rec-record-descriptor)))
    (cond
     ((listp descriptor)
      (car (rec-record-assoc rec-keyword-rec
                             (cadr descriptor))))
     ((equal descriptor "")
      "")
     (t
      nil))))

(defun rec-record-descriptor ()
  "Return the record descriptor of the record under point.

Return \"\" if no proper record descriptor is found in the file.
Return nil if the point is not on a record."
  (when (rec-current-record)
    (let ((descriptors rec-buffer-descriptors)
          descriptor type position found
          (i 0))
      (while (and (not found)
                  (< i (length descriptors)))
        (setq descriptor (nth i rec-buffer-descriptors))
        (setq position (marker-position (nth 2 descriptor)))
        (if (and (>= (point) position)
                 (or (= i (- (length rec-buffer-descriptors) 1))
                     (< (point) (marker-position (nth 2 (nth (+ i 1) rec-buffer-descriptors))))))
            (setq found t)
          (setq i (+ i 1))))
      (if found
          descriptor
        nil))))

(defun rec-summary-fields ()
  "Return a list with the names of the summary fields in the
current record set."
  (let ((descriptor (cadr (rec-record-descriptor))))
    (when descriptor
      (let ((fields-str (rec-record-assoc rec-keyword-summary descriptor)))
        (when fields-str
          (split-string (car fields-str)))))))

(defun rec-mandatory-fields ()
  "Return a list with the names of the mandatory fields in the
current record set."
  (let ((descriptor (cadr (rec-record-descriptor))))
    (when descriptor
      (let ((fields-str (rec-record-assoc rec-keyword-mandatory descriptor)))
        (when fields-str
          (split-string (car fields-str)))))))

(defun rec-key ()
  "Return the name of the field declared as the key of the
current record set, if any.  Otherwise return `nil'."
  (let ((descriptor (cadr (rec-record-descriptor))))
    (when descriptor
      (let ((key (rec-record-assoc rec-keyword-key descriptor)))
        (when key
          (car key))))))

;;;; Navigation

(defun rec-show-type (type &optional show-descriptor)
  "Show the records of the given type.  If TYPE is nil then the records
of the default type are shown."
  (widen)
  (unless (rec-goto-type type)
    (message "No records of the requested type were found."))
  ;; Show the first data record of this type, if it exists.
  (if (and (not show-descriptor)
           (save-excursion
             (let ((record-type (rec-record-type)))
               (and (rec-goto-next-rec)
                    (equal (rec-record-type) record-type)))))
      (rec-goto-next-rec))
  (rec-show-record))

(defun rec-show-record ()
  "Show the record under the point"
  (setq buffer-read-only t)
  (rec-narrow-to-record)
  (use-local-map rec-mode-map)
  (rec-set-head-line nil)
  (rec-set-mode-line (rec-record-type))
  ;; Hide the contents of big fields.
  (rec-hide-record-fields)
  ;; Change the appearance of continuation line markers to look like
  ;; indentation.
  (rec-hide-continuation-line-markers))

(defvar rec-continuation-line-markers-width 4
  "Width (in number of characters) used to represent continuation
  line markers in navigation mode.")

(defvar rec-continuation-line-markers-overlays nil
  "Continuation line markers overlays.")

(defun rec-hide-continuation-line-markers ()
  "Change the appearance of continuation line markers in the
current buffer to look like indentation."
  (let ((record (rec-current-record)))
    (when (rec-record-p record)
      (mapcar
       (lambda (field)
         (when (rec-field-p field)
           (let* ((pos (rec-field-position field))
                  (value-begin (+ pos (length (rec-field-name field)) 1))
                  (value-end (+ value-begin
                                (length (with-temp-buffer
                                          (rec-insert-field-value (rec-field-value field))
                                          (buffer-substring (point-min) (point-max)))))))
             (save-excursion
               (goto-char value-begin)
               (while (re-search-forward "^\\+ ?" (+ value-end 1) t)
                 (let ((ov (make-overlay (match-beginning 0) (match-end 0))))
                   (overlay-put ov 'display '(space . (:width rec-continuation-line-markers-width)))
                   (push ov rec-continuation-line-markers-overlays)))))))
       (rec-record-elems record)))))

(defun rec-remove-continuation-line-marker-overlays ()
  "Delete all the continuation line markers overlays."
  (mapc 'delete-overlay rec-continuation-line-markers-overlays)
  (setq rec-continuation-line-markers-overlays nil))

;;;; Field folding

(defvar rec-hide-field-overlays nil
  "Overlays hiding fields.")

(defun rec-hide-record-fields ()
  "Hide the contents of fields whose value exceeds
`rec-max-lines-in-fields' lines.  TAB can then be used to toggle
the visibility."
  (let ((record (rec-current-record)))
    (when (rec-record-p record)
      (mapcar
       (lambda (field)
         (when (rec-field-p field)
           (let ((lines-in-value (with-temp-buffer
                                   (insert (rec-field-value field))
                                   (count-lines (point-min) (point-max))))
                 ov)
             (when (> lines-in-value rec-max-lines-in-fields)
               (save-excursion
                 (goto-char (rec-field-position field))
                 (rec-fold-field))
               t))))
       (rec-record-elems record)))))

(defun rec-field-folded-p ()
  "Return whether the current field is folded."
  (let ((field (rec-current-field)))
    (when (rec-field-p field)
      (let ((value-start (+ (point) (length (rec-field-name field)) 1)))
        (memq t (mapcar (lambda (overlay)
                          (eq (overlay-get overlay 'invisible)
                              'rec-hide-field))
                        (overlays-at value-start)))))))

(defun rec-fold-field ()
  "Fold the current field."
  (let ((field (rec-current-field)))
    (when (rec-field-p field)
      (save-excursion
        (goto-char (rec-field-position field))
        (when (looking-at rec-field-re)
          (let ((value-start (+ (point) (length (rec-field-name field)) 1))
                (value-end (- (match-end 0) 1))
                ov)
            (setq ov (make-overlay value-start value-end))
            (overlay-put ov 'invisible 'rec-hide-field)
            (push ov rec-hide-field-overlays)))))))

(defun rec-unfold-field ()
  "Unfold the current field."
  (let ((field (rec-current-field)))
    (when (rec-field-p field)
      (let ((value-start (+ (point) (length (rec-field-name field)) 1)))
        (mapcar (lambda (overlay)
                  (when (eq (overlay-get overlay 'invisible) 'rec-hide-field)
                    (delete-overlay overlay)))
                (overlays-at value-start))))))

(defun rec-unfold-all-fields ()
  "Unfold all folded fields in the buffer."
  (mapc 'delete-overlay rec-hide-field-overlays)
  (setq rec-hide-field-overlays nil))

(defun rec-unfold-record-fields ()
  "Unfold any folded field in the current record."
  (let ((record (rec-current-record)))
    (when (rec-record-p record)
      (mapcar
       (lambda (field)
         (when (rec-field-p field)
           (save-excursion
             (goto-char (rec-field-position field))
             (rec-unfold-field))))
       (rec-record-elems record)))))

(defun rec-toggle-field-visibility ()
  "Toggle the visibility of the current field."
  (let ((field (rec-current-field)))
    (when (rec-field-p field)
      (save-excursion
        (goto-char (rec-field-position field))
        (when (looking-at rec-field-re)
          (let* ((value-start (+ (point) (length (rec-field-name field)) 1))
                 (value-end (- (match-end 0) 1))
                 ov)
            (if (memq t (mapcar (lambda (overlay)
                                  (eq (overlay-get overlay 'invisible)
                                      'rec-hide-field))
                                (overlays-at value-start)))
                (mapcar
                 (lambda (overlay)
                   (delete-overlay overlay))
                 (overlays-at value-start))
              (setq ov (make-overlay value-start value-end))
              (overlay-put ov 'invisible 'rec-hide-field))))))))

;;;; Field types
;;
;; This section contains functions and variable implementing field
;; types as described in the recutils manual.
;;
;; Each type is stored in a structure like:
;;
;;    (type KIND EXPR DATA)
;;
;; Where EXPR is the type descriptor used to create the type, and NAME
;; is the name of the type.
;;
;; KIND is the class of the type, and is one of:
;;
;;    int, bool, range, real, size, line, regexp, date,
;;    enum, field, email, uuid, rec
;;
;; DESCR is the data describing the type, and its value depends on the
;; kind:
;;
;;    - For sized strings, it is the maximum size of the string.
;;
;;    - For ranges, it is a list (MIN MAX) defining the range
;;      [MIN,MAX].  Open ranges can be specified by using nil.  For
;;      example: (0,nil).
;;
;;    - For regexps, it is a string containing the regexp.
;;
;;    - For record types, it is a string containing the type of
;;      the referred records.
;;
;;    - For any other type, it is nil.

(defvar rec-types
  '("int" "bool" "range" "real" "size" "line" "regexp" "date" "enum" "field" "email" "uuid" "rec")
  "Kind of supported types")

(defun rec-type-kind-p (kind)
  "Determine whether the given symbol or string is a type kind."
  (let (kind-symbol)
    (cond
     ((symbolp kind)
      (member (symbol-name kind) rec-types))
     ((stringp kind)
      (member kind rec-types))
     (t
      nil))))

(defun rec-parse-type (str)
  "Parse STR into a new type structure and return it.

STR must contain a type description as defined in the recutils
manual."
  (let (type)
    (with-temp-buffer
      (insert str)
      (goto-char (point-min))
      (when (looking-at "[a-z]+")
        (let ((kind (match-string 0)))
          (goto-char (match-end 0))
          (when (rec-type-kind-p kind)
            (cond
             ((member kind '("int" "bool" "real" "line" "date" "field" "email" "uuid"))
              (when (looking-at "[ \n\t]*$")
                (list 'type (intern kind) str nil)))
             ((equal kind "size")
              (when (looking-at "[ \n\t]*\\([0-9]+\\)[ \n\t]*$")
                (list (intern kind) str (string-to-number (match-string 1)))))
             ((equal kind "range")
              (when (or
                     (looking-at "[ \n\t]*\\(-?[0-9]+\\)[ \n\t]*$")
                     (looking-at "[ \n\t]*\\(-?[0-9]+\\)[ \n\t]+\\([0-9]+\\)[ \n\t]*$"))
                (let ((min (string-to-number (match-string 1)))
                      (max (when (stringp (match-string 2))
                             (string-to-number (match-string 2)))))
                (list 'type (intern kind) str (list min max)))))
             ((equal kind "enum")
              (let ((str-copy str)
                    (here (point)))
                ;; Remove comments from the enum description.
                (delete-region (point-min) (point-max))
                (insert (replace-regexp-in-string "([^)]*)" "" str-copy))
                (goto-char here)
                (when (looking-at "\\([ \n\t]*[a-zA-Z0-9][a-zA-Z0-9_-]*\\)+[ \n\t]*$")
                  (let (names)
                    ;; Scan the enum literals.
                    (while (looking-at "[ \n\t]+\\([a-zA-Z0-9][a-zA-Z0-9_-]*\\)")
                      (setq names (cons (match-string 1) names))
                      (goto-char (match-end 0)))
                    (list 'type (intern kind) str-copy (reverse names))))))
             ((equal kind "rec")
              (when (looking-at "[ \n\t]*\\([a-zA-Z%][a-zA-Z0-9_]*\\)[ \n\t]*$") ; Field name without a colon.
                (let ((referred-record (match-string 1)))
                  (list 'type (intern kind) str referred-record))))
             ((equal kind "regexp")
              (when (looking-at "[ \n\t]*\\(.*?\\)[ \n\t]*$")
                (let ((expr (match-string 1)))
                  (when (and (>= (length expr) 2)
                             (equal (elt expr 0) (elt expr (- (length expr) 1))))
                    (list 'type (intern kind) str (substring expr 1 -1))))))
             (t
              nil))))))))

(defun rec-type-kind (type)
  "Return the kind of a given type."
  (cadr type))

(defun rec-type-data (type)
  "Return the data associated with a given type."
  (cadddr type))

(defun rec-type-text (type)
  "Return the textual description of a given type."
  (caddr type))

(defun rec-check-type (type str)
  "Check whether STR contains a value conforming to TYPE, which
is a field type structure."
  (let* ((kind (cadr type))
         (expr (caddr type))
         (data (cadddr type))
         (value (if (equal kind 'line)
                    str
                  str)))
    (cond
     ((equal kind 'int)
      (string-match-p "^-?[0-9]+$" value))
     ((equal kind 'bool)
      (string-match-p "^\\(yes\\|no\\|0\\|1\\|true\\|false\\)$" value))
     ((equal kind 'range)
      (let ((min (car data))
            (max (cadr data)))
        (when (looking-at "-?[0-9]+$")
          (let ((number (string-to-number (match-string 0))))
          (and (>= number min) (<= number max))))))
     ((equal kind 'real)
      (string-match-p "^-?\\([0-9]*\\.\\)?[0-9]+$" value))
     ((equal kind 'size)
      (<= (length str) data))
     ((equal kind 'line)
      (string-match-p "^[^\n]*$" value))
     ((equal kind 'regexp)
      (string-match-p data value))
     ((equal kind 'date)
      ;; TODO.
      t)
     ((equal kind 'enum)
      (member value data))
     ((equal kind 'field)
      (string-match-p "^[a-zA-Z%][a-zA-Z0-9_]*$" value))
     ((equal kind 'email)
      (string-match-p "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]+$" value))
     ((equal kind 'uuid)
      ;; TODO.
      t)
     ((equal kind 'rec)
      ;; TODO.
      t)
     (t
      nil))))

(defun rec-field-type (field-name)
  "Return the type of the given field, if any, as determined in
the current record set.  If the field has no type, i.e. it is an
unrestricted field which can contain any text, then `nil' is
returned."
  (let* ((descriptor (rec-record-descriptor))
         (types (rec-record-assoc "%type" (cadr descriptor)))
         res-type)
    ;; Note that invalid %type entries are simply ignored.
    (mapc
     (lambda (type-descr)
       (with-temp-buffer
         (insert type-descr)
         (goto-char (point-min))
         (when (looking-at "[ \n\t]*\\([a-zA-Z%][a-zA-Z0-9_-]*\\(,[a-zA-Z%][a-zA-Z0-9_-]*\\)?\\)[ \n\t]*")
           (let ((names (match-string 1))
                 (begin-description (match-end 0))
                 name)
             (goto-char (match-beginning 1))
             (while (looking-at "\\([a-zA-Z%][a-zA-Z0-9_]*\\),?")
               (if (equal (match-string 1) field-name)
                   (progn
                     (goto-char begin-description)
                     (setq res-type (rec-parse-type (buffer-substring (point) (point-max)))))
                 (goto-char (match-end 0))))))))
     types)
    res-type))

;;;; Mode line and Head line

(defun rec-set-mode-line (str)
  "Set the modeline in rec buffers."
  (when str
    (setq mode-line-buffer-identification
          (list 20
                "%b " str))))

(defun rec-set-head-line (str)
  "Set the headline in rec buffers."
  (setq header-line-format str))

;;;; Fast selection

(defun rec-fast-selection (names prompt)
  "Fast group tag selection with single keys.

NAMES is an association list of the form:

    ((\"NAME1\" char1) ...)

Each character should identify only one name."
  ;; Adapted from `org-fast-tag-selection' in org.el by Carsten Dominic
  ;; Thanks Carsten! :D
  (let* ((maxlen (apply 'max (mapcar (lambda (name)
                                       (string-width (car name))) names)))
         (buf (current-buffer))
         (fwidth (+ maxlen 3 1 3))
         (ncol (/ (- (window-width) 4) fwidth))
         name count result char i key-list)
    (save-window-excursion
      (set-buffer (get-buffer-create " *Rec Fast Selection*"))
      (delete-other-windows)
;;      (split-window-vertically)
      (switch-to-buffer-other-window (get-buffer-create " *Rec Fast Selection*"))
      (erase-buffer)
      (insert prompt ":")
      (insert "\n\n")
      (setq count 0)
      (while (setq name (pop names))
        (setq key-list (cons (cadr name) key-list))
        (insert "[" (cadr name) "] "
                (car name)
                (make-string (- fwidth 4 (length (car name))) ?\ ))
        (when (= (setq count (+ count 1)) ncol)
          (insert "\n")
          (setq count 0)))
      (goto-char (point-min))
      (if (fboundp 'fit-window-to-buffer)
          (fit-window-to-buffer))
      (catch 'exit
        (while t
          (message "[a-z0-9...]: Select entry   [RET]: Exit")
          (setq char (let ((inhibit-quit t)) (read-char-exclusive)))
          (cond
           ((= char ?\r)
            (setq result nil)
            (throw 'exit t))
           ((member char key-list)
            (setq result char)
            (throw 'exit t)))))
      result)))

;;;; Summary mode

(defvar rec-summary-mode-map
  (let ((map (make-sparse-keymap)))
    (set-keymap-parent map tabulated-list-mode-map)
    (define-key map "\C-m" 'rec-summary-cmd-jump-to-record)
    map)
  "Local keymap for `rec-summary-mode' buffers.")

(defvar rec-summary-rec-buffer nil
  "rec-mode buffer paired with this summary buffer.")
(make-variable-buffer-local 'rec-summary-rec-buffer)

(define-derived-mode rec-summary-mode tabulated-list-mode "Rec Summary"
  "Major mode for summarizing the contents of a recfile.
\\<rec-summary-mode-map>
\\{rec-summary-mode-map}"
  (setq tabulated-list-format nil)
  (setq tabulated-list-padding 2)
  (setq tabulated-list-sort-key nil)
  (tabulated-list-init-header))

(defun rec-summary-populate (headers entries)
  "Populate a rec-mode summary buffer with the data in ENTRIES.

The data has the same structure than `tabulated-list-entries'."
  (setq tabulated-list-format headers)
  (setq tabulated-list-padding 2)
;;  (setq tabulated-list-sort-key (cons (car (elt headers 0)) nil))
  (tabulated-list-init-header)
  (setq tabulated-list-entries entries)
  (tabulated-list-print nil))

(defun rec-summary-cmd-jump-to-record ()
  "Jump to the selected record in the rec-mode buffer."
  (interactive)
;;  (if (buffer-live-p rec-summary-rec-buffer)
  (save-excursion
    (let ((rec-marker (tabulated-list-get-id)))
      (set-buffer (marker-buffer rec-marker))
      (widen)
      (goto-char (marker-position rec-marker))
      (rec-show-record))))
;;    (message "The rec buffer paired with this summary is not alive.")))

;;;; Database access functions
;;
;; The following functions map the high-level API rec_db_* provided by
;; librec, using the command line utilities instead.  Note that we are
;; using the support for keyword arguments provided by the cl package
;; in order to ease the usage of the functions.
;;
;; Note that the functions are not checking for the integrity of the
;; arguments: it is the invoked recutil which is in charge of that.

(defun* rec-query (&rest args
                   &key (type nil) (join nil) (index nil) (sex nil)
                        (fast-string nil) (random nil) (fex nil) (password nil)
                        (group-by nil) (sort-by nil) (icase nil) (uniq nil))
  "Perform a query in the current buffer using recsel.

ARGS contains the arguments to pass to the program."
  (let ((buffer (generate-new-buffer "Rec Sel "))
        args records status)
    (save-restriction
      (widen)
      (unwind-protect
          (progn
            ;; Prepare the arguments to recsel based on the arguments
            ;; passed to this function.

            (when (stringp type)
              (setq args (cons "-t" (cons type args))))
            (when (stringp join)
              (setq args (cons "-j" (cons join args))))
            (when (integerp index)
              (setq args (cons "-n" (cons (number-to-string index) args))))
            (when (stringp sex)
              (setq args (cons "-e" (cons sex args))))
            (when (stringp fast-string)
              (setq args (cons "-q" (cons fast-string args))))
            (when (integerp random)
              (setq args (cons "-m" (cons (number-to-string random) args))))
            (when (stringp fex)
              (setq args (cons "-p" (cons fex args))))
            (when (stringp password)
              (setq args (cons "-s" (cons password args))))
            (when (stringp group-by)
              (setq args (cons "-G" (cons group-by args))))
            (when (stringp sort-by)
              (setq args (cons "-S" (cons sort-by args))))
            (when icase
              (setq args (cons "-i" args)))
            (when uniq
              (setq args (cons "-U" args)))
            (when (and (not group-by) (not sort-by))
              (setq args (cons "--print-sexps" args)))
            ;; Call 'recsel' to perform the query.
            (setq status (apply #'call-process-region
                                (point-min) (point-max)
                                rec-recsel
                                nil ; delete
                                buffer
                                nil ; display
                                args))
            (if (/= status 0)
                (error "recsel returned error: %d" status))
            (with-current-buffer buffer
              (goto-char (point-min))
              (insert "(")
              (goto-char (point-max))
              (insert ")")
              (setq records (read (point-min-marker)))))
        (kill-buffer buffer)))
    records))

;;;; Selection of records
;;
;; The following functions implement selection of records, which
;; maintains a subset of the records in the current buffer.

(defvar rec-current-selection nil
  "List of records corresponding to the last executed selection,
or `nil' if no selection is active.")
(make-variable-buffer-local 'rec-current-selection)

(defun rec-navigate-selection ()
  "Goto the first record of the current selection, if any."
  (if (not rec-current-selection)
      (message "No current selection")
    (widen)
    (let* ((first-record (car rec-current-selection))
           (pos (rec-record-position first-record)))
      (goto-char pos)
      (rec-show-record))))

(defun rec-cmd-select ()
  "Perform a selection on the current buffer using some criteria.

The result of the selection is stored in `rec-current-selection'."
  (interactive)
  (setq rec-current-selection (rec-query)))

(defun rec-cmd-select-fast (prefix str)
  "Perform a selection on the current record set using a fast string search.

A prefix argument means to use a case-insensitive search."
  (interactive "P\nsFast string query: ")
  (when (not (equal str ""))
    (setq rec-current-selection (rec-query :fast-string str
                                           :icase prefix
                                           :type (rec-record-type)))
    (rec-navigate-selection)))

(defun rec-cmd-select-sex (prefix sex)
  "Perform a selection on the current record set using a selection expression.

A prefix argument means to use a case-insensitive search."
  (interactive "P\nsSelection expression: ")
  (when (not (equal sex ""))
    (setq rec-current-selection (rec-query :sex sex
                                           :icase prefix
                                           :type (rec-record-type)))
    (rec-navigate-selection)))

;;;; Commands
;;
;; The following functions implement interactive commands available in
;; the several modes defined in this file.

(defvar rec-field-name)
(make-variable-buffer-local 'rec-field-name)
(defvar rec-marker)
(make-variable-buffer-local 'rec-marker)
(defvar rec-buffer)
(make-variable-buffer-local 'rec-buffer)

(defun rec-cmd-edit-field (n)
  "Edit the contents of the field under point in a separate
buffer.

The input method used for getting the field value depends on its
type, unless a prefix argument is used.  Then the more general
method, i.e. asking for the new value in an unrestricted buffer,
will be used for fields of any type."
  (interactive "P")
  (let* (edit-buf
         (field (rec-current-field))
         (field-value (rec-field-value field))
         (field-name (rec-field-name field))
         (field-type (rec-field-type field-name))
         (field-type-kind (when field-type (rec-type-kind field-type)))
         (pointer (rec-beginning-of-field-pos))
         (prev-buffer (current-buffer)))
    (if field-value
        (cond
         ((and (or (equal field-type-kind 'enum)
                   (equal field-type-kind 'bool))
               (null n))
          (let* ((data (rec-type-data field-type))
                 (fast-selection-data
                  (cond
                   ((equal field-type-kind 'enum)
                    (let (used-letters)
                      (mapcar
                       (lambda (elem)
                         ;; Assign a letter to this enumerated entry.
                         ;; The letters are chosen using the following
                         ;; criteria: if the first letter of the entry
                         ;; is free then use it.  Otherwise, if the
                         ;; second letter of the entry is free then
                         ;; use it.  Otherwise use the first available
                         ;; symbol in the alphabet. Note that ELEM
                         ;; cannot be the empty string.
                         (let ((letter (if (not (member (elt elem 0) used-letters))
                                           (elt elem 0)
                                         (if (and (> (length elem) 1)
                                                  (not (member (elt elem 1) used-letters)))
                                             (elt elem 1)
                                           (let* ((c ?a) (i 0))
                                             (while (member c used-letters)
                                               (setq c (+ ?a i))
                                               (setq i (+ i 1)))
                                             c)))))
                           (setq used-letters (cons letter used-letters))
                           (list elem letter)))
                       data)))
                   ((equal field-type-kind 'bool)
                    '(("yes" ?y) ("no" ?n) ("1" ?o) ("0" ?z) ("true" ?t) ("false" ?f)))
                   (t
                    (error "Invalid kind of type"))))
                 (letter (rec-fast-selection fast-selection-data "New value")))
            (when letter
              (let ((buffer-read-only nil)
                    new-value)
                (mapc
                 (lambda (elem)
                   (when (equal letter (cadr elem))
                     (setq new-value (car elem))))
                 fast-selection-data)
                (rec-delete-field)
                (save-excursion
                  (rec-insert-field (list 'field
                                          0
                                          field-name
                                          new-value)))))))
         ((and (equal field-type-kind 'date) rec-popup-calendar
               (null n))
          (setq rec-field-name field-name)
          (setq rec-prev-buffer prev-buffer)
          (setq rec-pointer pointer)
          (calendar)
          (let ((old-map (current-local-map))
                (map (copy-keymap calendar-mode-map)))
            (define-key map "q"
              `(lambda () (interactive)
                 (use-local-map (quote ,old-map))
                 (calendar-exit)))
            (define-key map "t"
              `(lambda () (interactive)
                 (use-local-map (quote ,old-map))
                 (calendar-exit)
                 (set-buffer rec-prev-buffer)
                 (let ((buffer-read-only nil))
                   (rec-delete-field)
                   (save-excursion
                     (rec-insert-field (list 'field
                                             0
                                             rec-field-name
                                             (format-time-string rec-time-stamp-format)))))))
            (define-key map (kbd "RET")
              `(lambda () (interactive)
                 (let* ((date (calendar-cursor-to-date))
                        (time (encode-time 0 0 0 (nth 1 date) (nth 0 date) (nth 2 date))))
                   (use-local-map (quote ,old-map))
                   (calendar-exit)
                   (set-buffer rec-prev-buffer)
                   (let ((buffer-read-only nil))
                     (rec-delete-field)
                     (save-excursion
                       (rec-insert-field (list 'field
                                               0
                                               rec-field-name
                                               (format-time-string "%Y-%m-%d" time))))))))
            (use-local-map map)
            (message "[RET]: Select date [t]: Time-stamp     [q]: Exit")))
         (t
          (setq edit-buf (get-buffer-create "Rec Edit"))
          (set-buffer edit-buf)
          (delete-region (point-min) (point-max))
          (rec-edit-field-mode)
          (setq rec-field-name field-name)
          (setq rec-marker (make-marker))
          (set-marker rec-marker pointer prev-buffer)
          (setq rec-prev-buffer prev-buffer)
          (setq rec-pointer pointer)
          (insert field-value)
          (switch-to-buffer-other-window edit-buf)
          (goto-char (point-min))
          (message "Edit the value of the field and use C-c C-c to exit")))
      (message "Not in a field"))))

(defun rec-finish-editing-field ()
  "Stop editing the value of a field."
  (interactive)
  (let ((marker rec-marker)
        (prev-pointer rec-pointer)
        (edit-buffer (current-buffer))
        (name rec-field-name)
        (value (buffer-substring-no-properties (point-min) (point-max))))
    (if (equal (length (window-list)) 1)
        (set-window-buffer (selected-window) rec-prev-buffer)
      (delete-window))
    (switch-to-buffer rec-prev-buffer)
    (let ((buffer-read-only nil))
      (kill-buffer edit-buffer)
      (goto-char marker)
      (rec-delete-field)
      (rec-insert-field (list 'field
                              0
                              name
                              value))
      (goto-char prev-pointer)
      (unless rec-editing
        (rec-hide-continuation-line-markers)))))

(defun rec-beginning-of-field ()
  "Goto to the beginning of the current field"
  (interactive)
  (let ((pos (rec-beginning-of-field-pos)))
    (when pos
      (goto-char pos))))

(defun rec-end-of-field ()
  "Goto to the end of the current field"
  (interactive)
  (let ((pos (rec-end-of-field-pos)))
    (when pos
      (goto-char pos))))

(defun rec-beginning-of-record ()
  "Goto to the beginning of the current record"
  (interactive)
  (let ((pos (rec-beginning-of-record-pos)))
    (when pos
      (goto-char pos))))

(defun rec-end-of-record ()
  "Goto to the end of the current record"
  (interactive)
  (let ((pos (rec-end-of-record-pos)))
    (when pos
      (goto-char pos))))

(defun rec-delete-field ()
  "Delete the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (when (and begin-pos end-pos)
      (delete-region begin-pos end-pos)
      (when (equal (char-after) ?\n)
        (delete-char 1)))))

(defun rec-copy-record ()
  "Copy the current record"
  (interactive))

(defun rec-find-type ()
  "Goto the beginning of the descriptor with a given type."
  (interactive)
  (let ((type (completing-read "Record type: "
                               (save-restriction
                                 (widen)
                                 (rec-buffer-types)))))
    (if (equal type "") (setq type nil))
    (rec-show-type type)))

(defun rec-cmd-goto-next-field ()
  "Move the pointer to the beginning of the next field in the
record.  Interactive version."
  (interactive)
  (if (save-excursion
        (not (rec-goto-next-field)))
      (if rec-editing
          (progn
            (goto-char (point-min))
            (unless (looking-at rec-field-name-re)
              (rec-goto-next-field)))
      (rec-beginning-of-record))
    (rec-goto-next-field)))

(defun rec-cmd-goto-next-rec (&optional n)
  "Move the pointer to the beginning of the next record in the
file.  Interactive version."
  (interactive "P")
  (when (null n) (setq n 1))
  (widen)
  (let ((record-type (rec-record-type)))
    (dotimes (i n)
      (if (save-excursion
	    (and (rec-goto-next-rec)
		 (equal (rec-record-type) record-type)
		 (not (rec-record-descriptor-p (rec-current-record)))))
	  (progn
	    (rec-unfold-all-fields)
	    (rec-remove-continuation-line-marker-overlays)
	    (rec-goto-next-rec))
	(if (not (rec-record-type))
	    (message "No more records")
	  (message "%s" (concat "No more records of type "
				(rec-record-type)))))))
  (unless rec-editing
    (rec-show-record)))

(defun rec-cmd-goto-previous-rec (&optional n)
  "Move the pointer to the beginning of the previous record in
the file.  Interactive version."
  (interactive "P")
  (when (null n) (setq n 1))
  (widen)
  (let ((record-type (rec-record-type)))
    (dotimes (i n)
      (if (save-excursion
	    (and (rec-goto-previous-rec)
		 (equal (rec-record-type) record-type)
		 (not (rec-record-descriptor-p (rec-current-record)))))
	  (progn
	    (rec-unfold-all-fields)
	    (rec-remove-continuation-line-marker-overlays)
	    (rec-goto-previous-rec))
	(if (not (rec-record-type))
	    (message "No more records")
	  (message "%s" (concat "No more records of type "
				(rec-record-type)))))))
  (unless rec-editing
    (rec-show-record)))

(defun rec-cmd-undo ()
  "Undo a change in the buffer when in navigation mode."
  (interactive)
  (let ((buffer-read-only nil))
    (undo)))

(defun rec-cmd-jump-back ()
  "Undo the previous jump"
  (interactive)
  (if rec-jump-back
      (progn
        (widen)
        (goto-char (marker-position rec-jump-back))
        (unless rec-editing
          (rec-show-record))
        (setq rec-jump-back nil))
    (message "No previous position to jump")))

(defun rec-edit-record ()
  "Go to the record edition mode"
  (interactive)
  (setq rec-editing t)
  (rec-unfold-all-fields)
  (rec-remove-continuation-line-marker-overlays)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (rec-set-head-line "Editing record - use C-cC-c to return to navigation mode")
  (rec-set-mode-line "Edit record")
  (setq rec-update-p nil)
  (setq rec-preserve-last-newline t))

(defun rec-edit-type ()
  "Go to the type edition mode"
  (interactive)
  (setq rec-editing t)
  (rec-unfold-all-fields)
  (rec-remove-continuation-line-marker-overlays)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (widen)
  (rec-narrow-to-type (rec-record-type))
  (setq rec-update-p t)
  (rec-set-head-line (concat "Editing type "
                             "'" (rec-record-type) "'"
                             " - use C-cC-c to return to navigation mode"))
  (rec-set-mode-line "Edit type"))

(defun rec-edit-buffer ()
  "Go to the buffer edition mode"
  (interactive)
  (setq rec-editing t)
  (rec-unfold-all-fields)
  (rec-remove-continuation-line-marker-overlays)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (widen)
  (setq rec-update-p t)
  (rec-set-head-line "Editing buffer - use C-cC-c to return to navigation mode")
  (rec-set-mode-line "Edit buffer"))

(defun rec-finish-editing ()
  "Go back from the record edition mode"
  (interactive)
  (when (or (not rec-update-p)
            (and rec-update-p
                 (save-restriction (widen) (rec-update-buffer-descriptors-and-check t))))
    (or (rec-current-record)
        (rec-goto-next-rec)
        (rec-goto-previous-rec))
    (when rec-preserve-last-newline
      (save-excursion
        (goto-char (point-max))
        (unless (equal (char-before) ?\n)
          (insert ?\n))))
    (setq rec-update-p nil)
    (rec-show-record)
    (rec-set-head-line nil)
    (rec-set-mode-line (rec-record-type))
    (setq rec-editing nil)
    (message "End of edition")))

(defun rec-cmd-show-descriptor ()
  "Show the descriptor record of the current record.

This jump sets jump-back."
  (interactive)
  (let ((type (rec-record-type)))
    (when type
      (setq rec-jump-back (point-marker))
      (if rec-editing
          (rec-goto-type type)
        (rec-show-type type t)))))

(defun rec-cmd-show-type ()
  "Show the descriptor corresponding to the field under point, in
the modeline."
  (interactive)
  (let ((type (rec-current-field-type)))
    (if type
        (display-message-or-buffer (rec-type-text type))
      (message "Unrestricted text"))))

(defun rec-cmd-count (n)
  "Display a message in the minibuffer showing the number of
records of the current type.

If a numeric argument is used then prompt for a selection
expression."
  (interactive "P")
  (let* ((default-sex (let ((current-field (rec-current-field)))
                        (when (and current-field
                                   (not (string-match "\n" (rec-field-value current-field)))
                                   (< (length (rec-field-value current-field)) 20))
                          (concat (rec-field-name current-field) " = '" (rec-field-value current-field) "'"))))
         (sex (and (not (null n)) (read-from-minibuffer (concat "Selection expression"
                                                                (if default-sex
                                                                    (concat " (default " default-sex ")")
                                                                  "")
                                                                ": ")))))
    (when (equal sex "")
      (setq sex default-sex))
    (message "Counting records...")
    (let ((type (rec-record-type)))
      (message "%s" (concat (number-to-string (rec-count type sex))
                            (if (or (not type)
                                    (equal type ""))
                                " records"
                              (concat " records of type " type))
                            (when (and sex (not (equal sex "")))
                              (concat " with sex " sex)))))))

(defun rec-cmd-statistic ()
  "Display a statistic on the occurrence of the value contained
  in the field under point in the minibuffer, if any.

This command is especially useful with enumerated types."
  (interactive)
  (let* ((field (rec-current-field))
         (field-name (rec-field-name field))
         (type (rec-field-type field-name))
         (type-kind (when type (rec-type-kind type))))
    (cond ((equal type-kind 'enum)
           (let* ((keys (rec-type-data type))
                  (total (rec-count (rec-record-type)))
                  (percentages (mapcar (lambda (key)
                                         (let ((key-count (rec-count (rec-record-type)
                                                                     (concat field-name " = '" key "'"))))
                                           (list key key-count (/ (* key-count 100) total))))
                                       keys))
                  str)
             (mapc (lambda (occurrence)
                     (setq str (concat str
                                       (number-to-string (nth 1 occurrence))
                                       " "
                                       (nth 0 occurrence)
                                       " ("
                                       (number-to-string (nth 2 occurrence))
                                       "%) ")))
                   percentages)
             (message "%s" str))))))

(defun rec-cmd-append-field ()
  "Goto the end of the record and switch to edit record mode."
  (interactive)
  (unless rec-editing
    (rec-edit-record)
    (goto-char (point-max))
    (insert "\n")
    (backward-char)))

(defun rec-cmd-trim-field-value ()
  "Trim the value of the field under point, if any."
  (interactive)
  (save-excursion
    (let ((buffer-read-only nil)
          (field (rec-current-field)))
      (setq field (rec-field-trim-value field))
      (rec-delete-field)
      (rec-insert-field field))))

(defun rec-cmd-compile ()
  "Compile the current file with recfix."
  (interactive)
  (let ((cur-buf (current-buffer))
        (cmd (concat rec-recfix " "))
        (tmpfile (make-temp-file "rec-mode-")))
    (if buffer-file-name
        (setq cmd (concat cmd (shell-quote-argument buffer-file-name)))
      (with-temp-file tmpfile
        (insert-buffer-substring cur-buf))
      (setq cmd (concat cmd (shell-quote-argument tmpfile))))
    (compilation-start cmd)))

(defun rec-cmd-show-info ()
  "Show information about the recfile in the modeline."
  (interactive)
  (let ((cur-buf (current-buffer))
        (filename (if buffer-file-name
                      buffer-file-name
                    (make-temp-file "rec-mode-")))
        (msg ""))
    (if (not buffer-file-name)
        (with-temp-file filename
          (insert-buffer-substring cur-buf)))
    (with-temp-buffer
      (call-process rec-recinf
                    nil ; infile
                    t   ; output to current buffer
                    nil ; display
                    filename)
      (setq msg (buffer-substring-no-properties (point-min)
                                                (point-max))))
    ;; Delete temporary file.
    (if (not buffer-file-name)
        (delete-file filename))
    ;; Show the message.
    (setq msg (replace-regexp-in-string "\n$" "" msg))
    (setq msg (replace-regexp-in-string "\n" ", " msg))
    (message "%s" msg)))

(defun rec-cmd-toggle-field-visibility ()
  "Toggle the visibility of the field under point."
  (interactive)
  (when (rec-field-p (rec-current-field))
    (if (rec-field-folded-p)
        (rec-unfold-field)
      (rec-fold-field))))

(defun rec-cmd-kill-field ()
  "Kill the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (if (and begin-pos end-pos)
        (kill-region begin-pos end-pos)
      (message "Not in a field"))))

(defun rec-cmd-copy-field ()
  "Copy the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (if (and begin-pos end-pos)
        (progn
          (copy-region-as-kill begin-pos end-pos)
          (message "Field copied to kill ring"))
      (message "Not in a field"))))

(defun rec-cmd-kill-record ()
  "Kill the current record"
  (interactive)
  (let ((begin-pos (rec-beginning-of-record-pos))
        (end-pos (rec-end-of-record-pos)))
    (if (and begin-pos end-pos)
        (progn
          (when (looking-back "^[ \t]*")
            ;; Delete the newline before the record as well, but do
            ;; not include it in the kill ring.
            (delete-region (match-beginning 0) (+ (match-end 0) 1)))
          (kill-region begin-pos end-pos))
      (message "Not in a record"))))

(defun rec-cmd-copy-record ()
  "Copy the current record"
  (interactive)
  (let ((begin-pos (rec-beginning-of-record-pos))
        (end-pos (rec-end-of-record-pos)))
    (if (and begin-pos end-pos)
        (progn
          (copy-region-as-kill begin-pos end-pos)
          (message "record copied to kill ring"))
      (message "Not in a record"))))

(defun rec-cmd-show-summary ()
  "Show a window with a summary of the contents of the current
record set.

The fields used to build the summary are determined in the
following way: if there is a %summary field in the record
descriptor of the current record set then it must contain a comma
separated list of fields.  Otherwise the %key is used.  Otherwise
the user is prompted."
  (interactive)
  (let ((summary-buffer-name (concat (buffer-name (current-buffer)) " Summary")))
    (if (buffer-live-p (get-buffer summary-buffer-name))
        (progn
          (delete-other-windows)
          (split-window-vertically 10)
          (switch-to-buffer summary-buffer-name))
      (let ((summary-fields (rec-summary-fields)))
        (unless summary-fields
          (setq summary-fields (list (rec-key)))
          (unless (car summary-fields)
            (setq summary-fields (list (read-from-minibuffer "Fields to use in the summary: ")))))
        (if (car summary-fields)
            (let* ((query (rec-query :fex (rec-join-string summary-fields ",")))
                   (summary-list (mapcar (lambda (rec)
                                           (let ((entry-marker (make-marker)))
                                             (set-marker entry-marker (rec-record-position rec))
                                             (list entry-marker (vconcat (rec-record-values rec summary-fields)))))
                                         query)))
              ;; Create the summary window if it does not exist and populate
              ;; it.
              (let ((rec-buf (current-buffer))
                    (buf (get-buffer-create (concat (buffer-name (current-buffer)) " Summary"))))
                (delete-other-windows)
                (split-window-vertically 10)
                (switch-to-buffer buf)
                (let ((buffer-read-only nil))
                  (delete-region (point-min) (point-max))
                  (setq rec-summary-rec-buffer rec-buf)
                  (rec-summary-mode)
                  (rec-summary-populate (vconcat (mapcar (lambda (field) (list field 15 nil)) summary-fields)) summary-list)
                  (hl-line-mode 1))))
          (message "No fields to build the summary."))))))

;;;; Interacting with other modes

(defun rec-log-current-defun ()
  "Return the value of the key in the current record, if any.  If
no key is defined then return the value of the first field in the
record.  In case the pointer is not in a record then this
function returns `nil'."
  (let ((record (rec-current-record))
        (key (rec-key)))
    (when record
      (if key
          (let ((values (rec-record-assoc key record)))
            (if values
                (car values)
              (rec-field-value (car (rec-record-elems record)))))
        (rec-field-value (car (rec-record-elems record)))))))

;;;; Definition of modes

(defvar font-lock-defaults)
(make-variable-buffer-local 'font-lock-defaults)
(defvar rec-type)
(make-variable-buffer-local 'rec-type)
(defvar rec-buffer-descriptors)
(make-variable-buffer-local 'rec-buffer-descriptors)
(defvar rec-jump-back)
(make-variable-buffer-local 'rec-jump-back)
(defvar rec-update-p)
(make-variable-buffer-local 'rec-update-p)
(defvar rec-preserve-last-newline)
(make-variable-buffer-local 'rec-preserve-last-newline)
(defvar rec-editing)
(make-variable-buffer-local 'rec-editing)
(defvar add-log-current-defun-section)
(make-variable-buffer-local 'add-log-current-defun-section)

(defun rec-mode ()
  "A major mode for editing rec files.

Commands:
\\{rec-mode-map}

Turning on rec-mode calls the members of the variable
`rec-mode-hook' with no args, if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (widen)
  ;; Local variables
  (setq add-log-current-defun-section #'rec-log-current-defun)
  (setq rec-editing nil)
  (setq rec-jump-back nil)
  (setq rec-update-p nil)
  (setq rec-preserve-last-newline nil)
  (setq font-lock-defaults '(rec-font-lock-keywords))
  (add-to-invisibility-spec '(rec-hide-field . "..."))
  (use-local-map rec-mode-map)
  (set-syntax-table rec-mode-syntax-table)
  (setq mode-name "Rec")
  (setq major-mode 'rec-mode)
  (run-hooks 'rec-mode-hook)
  ;; Goto the first record of the first type (including the Unknown).
  ;; If there is a problem (i.e.  syntax error) then go to fundamental
  ;; mode and show the output of recfix in a separated buffer.
  (when (rec-update-buffer-descriptors-and-check)
    ;; If the configured open-mode is navigation, set up the buffer
    ;; accordingly.  But don't go into navigation mode if the file is
    ;; empty.
    (if (and (equal rec-open-mode 'navigation)
             (> (buffer-size (current-buffer)) 0))
        (progn
          (setq buffer-read-only t)
          (setq rec-type (car (rec-buffer-types)))
          (rec-show-type rec-type))
      ;; Edit mode
      (use-local-map rec-mode-edit-map)
      (setq rec-editing t)
      (rec-set-mode-line "Edit buffer"))))

(defvar rec-edit-field-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-c\C-c" 'rec-finish-editing-field)
    map)
  "Keymap for rec-edit-field-mode")

(defun rec-edit-field-mode ()
  "A major mode for editing rec field values.

Commands:
\\{rec-edit-field-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (use-local-map rec-edit-field-mode-map)
  (setq mode-name "Rec Edit")
  (setq major-mode 'rec-edit-field-mode))

;;;; Miscellaneous utilities

(defun rec-join-string (l c)
  (if (> (length l) 1)
    (concat (car l) c (rec-join-string (cdr l) c))
    (car l)))

(provide 'rec-mode)

;; Local variables:
;; outline-regexp: ";;;;"
;; End:

;;; rec-mode.el ends here
