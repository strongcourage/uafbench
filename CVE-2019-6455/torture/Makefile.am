# torture/ Makefile.am
# GNU rec library

# Copyright (C) 2009-2015 Jose E. Marchesi

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Note that we want to run the unit tests first.  They are stored in
# the rec-* subdirectories, but commanded by the Makefile in the
# current directory.  That explains the dot in the SUBDIRS variable
# definition.

SUBDIRS = . utils

if CHECK

TESTS = runtests
noinst_PROGRAMS = runtests

REC_MSET_TSUITE = rec-mset/elem-types.h \
                  rec-mset/rec-mset-new.c \
                  rec-mset/rec-mset-dup.c \
                  rec-mset/rec-mset-type-p.c \
                  rec-mset/rec-mset-register-type.c \
                  rec-mset/rec-mset-count.c \
                  rec-mset/rec-mset-get-at.c \
                  rec-mset/tsuite-rec-mset.c

REC_COMMENT_TSUITE = rec-comment/rec-comment-new.c \
                     rec-comment/rec-comment-destroy.c \
                     rec-comment/rec-comment-dup.c \
                     rec-comment/rec-comment-text.c \
                     rec-comment/rec-comment-set-text.c \
                     rec-comment/rec-comment-equal-p.c \
                     rec-comment/tsuite-rec-comment.c

REC_FIELD_NAME_TSUITE = rec-field-name/rec-field-name-equal-p.c \
                        rec-field-name/rec-field-name-p.c \
                        rec-field-name/rec-field-name-normalise.c \
                        rec-field-name/tsuite-rec-field-name.c

REC_TYPE_TSUITE = rec-type/rec-type-new.c \
                  rec-type/rec-type-destroy.c \
                  rec-type/rec-type-descr-p.c \
                  rec-type/rec-type-kind.c \
                  rec-type/rec-type-kind-str.c \
                  rec-type/rec-type-equal-p.c \
                  rec-type/rec-type-check.c \
                  rec-type/rec-type-name.c \
                  rec-type/rec-type-set-name.c \
                  rec-type/tsuite-rec-type.c

REC_TYPE_REG_TSUITE = rec-type-reg/rec-type-reg-new.c \
                      rec-type-reg/rec-type-reg-destroy.c \
                      rec-type-reg/tsuite-rec-type-reg.c \
                      rec-type-reg/rec-type-reg-get.c

REC_FEX_TSUITE = rec-fex/rec-fex-new.c \
                 rec-fex/rec-fex-destroy.c \
                 rec-fex/rec-fex-check.c \
                 rec-fex/rec-fex-sort.c \
                 rec-fex/rec-fex-size.c \
                 rec-fex/rec-fex-get.c \
                 rec-fex/rec-fex-elem-field-name.c \
                 rec-fex/rec-fex-elem-rewrite-to.c \
                 rec-fex/rec-fex-elem-min.c \
                 rec-fex/rec-fex-elem-max.c \
                 rec-fex/rec-fex-str.c \
                 rec-fex/tsuite-rec-fex.c

REC_FIELD_TSUITE = rec-field/rec-field-name.c \
                   rec-field/rec-field-set-name.c \
                   rec-field/rec-field-value.c \
                   rec-field/rec-field-set-value.c \
                   rec-field/rec-field-dup.c \
                   rec-field/rec-field-new.c \
                   rec-field/rec-field-destroy.c \
                   rec-field/rec-field-equal-p.c \
                   rec-field/rec-field-source.c \
                   rec-field/rec-field-set-source.c \
                   rec-field/rec-field-location.c \
                   rec-field/rec-field-location-str.c \
                   rec-field/rec-field-set-location.c \
                   rec-field/rec-field-char-location.c \
                   rec-field/rec-field-char-location-str.c \
                   rec-field/rec-field-set-char-location.c \
                   rec-field/rec-field-to-comment.c \
                   rec-field/tsuite-rec-field.c

REC_RECORD_TSUITE = rec-record/tsuite-rec-record.c

REC_PARSER_TSUITE = rec-parser/rec-parser-new.c \
                    rec-parser/rec-parser-new-str.c \
                    rec-parser/rec-parser-new-mem.c \
                    rec-parser/rec-parser-destroy.c \
                    rec-parser/rec-parse-field-name-str.c \
                    rec-parser/rec-parse-field-name.c \
                    rec-parser/rec-parse-field.c \
                    rec-parser/rec-parse-record.c \
                    rec-parser/rec-parse-record-str.c \
                    rec-parser/rec-parse-rset.c \
                    rec-parser/rec-parse-db.c \
                    rec-parser/rec-parser-eof.c \
                    rec-parser/rec-parser-error.c \
                    rec-parser/rec-parser-reset.c \
                    rec-parser/rec-parser-perror.c \
                    rec-parser/rec-parser-seek-mem.c \
                    rec-parser/tsuite-rec-parser.c

REC_WRITER_TSUITE= rec-writer/rec-write-comment.c \
                   rec-writer/rec-write-field.c \
                   rec-writer/rec-write-field-name.c \
                   rec-writer/rec-writer-destroy.c \
                   rec-writer/rec-write-record.c \
                   rec-writer/rec-writer-new.c \
                   rec-writer/rec-write-rset.c \
                   rec-writer/rec-write-db.c \
                   rec-writer/rec-write-field-name-str.c \
                   rec-writer/rec-write-field-str.c \
                   rec-writer/rec-write-comment-str.c \
                   rec-writer/tsuite-rec-writer.c

REC_SEX_TSUITE = rec-sex/rec-sex-new.c \
                 rec-sex/rec-sex-destroy.c \
                 rec-sex/rec-sex-compile.c \
                 rec-sex/rec-sex-eval.c \
                 rec-sex/tsuite-rec-sex.c

runtests_SOURCES = runtests.c \
                   $(REC_MSET_TSUITE) \
                   $(REC_COMMENT_TSUITE) \
                   $(REC_FIELD_NAME_TSUITE) \
                   $(REC_TYPE_TSUITE) \
                   $(REC_TYPE_REG_TSUITE) \
                   $(REC_FIELD_TSUITE) \
                   $(REC_RECORD_TSUITE) \
                   $(REC_FEX_TSUITE) \
                   $(REC_PARSER_TSUITE) \
                   $(REC_WRITER_TSUITE) \
                   $(REC_SEX_TSUITE)

AM_CPPFLAGS = -I$(top_srcdir)/src \
              -I$(top_srcdir)/torture
runtests_CFLAGS = $(CHECK_CFLAGS)
runtests_LDADD = $(CHECK_LIBS) ../src/librec.la

clean-local:
	-rm -f tests.log

endif # CHECK

# End of Makefile.am
