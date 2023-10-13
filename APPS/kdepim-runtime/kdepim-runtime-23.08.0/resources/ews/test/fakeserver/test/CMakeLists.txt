#
#  SPDX-FileCopyrightText: 2016 Krzysztof Nowicki <krissn@op.pl>
#
#  SPDX-License-Identifier: LGPL-2.0-or-later

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

macro(akonadi_fakeserver_add_test tname)
  add_executable(${tname} ${tname}.cpp)
  target_link_libraries(${tname} Qt::Test fakeewsserver)
  add_test(NAME ${tname} COMMAND ${tname})
endmacro(akonadi_fakeserver_add_test tname)

akonadi_fakeserver_add_test(ewsfakesrv_test)
