## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-narrator.
#
# Kolibre-narrator is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-narrator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-narrator. If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign

SUBDIRS = src tests $(subdirs)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkolibre-narrator.pc

include doxygen.am

AM_DISTCHECK_CONFIGURE_FLAGS = "PKG_CONFIG_PATH=${PKG_CONFIG_PATH}"

promptdir = $(datadir)/libkolibre/narrator

prompt_DATA = prompts/narrator.csv \
		prompts/types.csv \
		prompts/en_translations.csv \
		prompts/sv_translations.csv \
		prompts/fi_translations.csv \
		prompts/ru_translations.csv \
		prompts/sq_translations.csv \
		prompts/ar_translations.csv \
		prompts/hy_translations.csv \
		prompts/af_translations.csv \
		prompts/bs_translations.csv \
		prompts/cy_translations.csv \
		prompts/hu_translations.csv \
		prompts/vi_translations.csv \
		prompts/el_translations.csv \
		prompts/da_translations.csv \
		prompts/id_translations.csv \
		prompts/is_translations.csv \
		prompts/es_translations.csv \
		prompts/it_translations.csv \
		prompts/ca_translations.csv \
		prompts/zh_translations.csv \
		prompts/ko_translations.csv \
		prompts/ht_translations.csv \
		prompts/la_translations.csv \
		prompts/lv_translations.csv \
		prompts/mk_translations.csv \
		prompts/de_translations.csv \
		prompts/nl_translations.csv \
		prompts/no_translations.csv \
		prompts/pl_translations.csv \
		prompts/pt_translations.csv \
		prompts/ro_translations.csv \
		prompts/sr_translations.csv \
		prompts/sk_translations.csv \
		prompts/sw_translations.csv \
		prompts/th_translations.csv \
		prompts/ta_translations.csv \
		prompts/tr_translations.csv \
		prompts/fr_translations.csv \
		prompts/hi_translations.csv \
		prompts/hr_translations.csv \
		prompts/cs_translations.csv \
		prompts/eo_translations.csv \
		prompts/ja_translations.csv

EXTRA_DIST = $(prompt_DATA)
