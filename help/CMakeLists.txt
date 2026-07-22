set (LANGS "RU" "DE" "ES" "FR" "ZH" "SK")
# pwsafePL -- nothing there :-(

macro(set_lang_help_files FILES_VAR DIR)
  file(GLOB_RECURSE ${FILES_VAR}
    LIST_DIRECTORIES FALSE
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    ${DIR}/html/*
    ${DIR}/pwsafe*.hh?
  )
endmacro(set_lang_help_files)

if (WIN32 AND NOT WX_WINDOWS)

  find_program(HHC_EXECUTABLE hhc.exe DOC "Microsoft's help compiler (hhc.exe)")
  if (NOT HHC_EXECUTABLE)
    message(WARNING "The Microsoft help compiler was not found.")
    return()
  endif()

  function(make_help OUTPUT_FILE_VAR DIR LANG)
    set_lang_help_files(INPUT_FILES "${DIR}")

    set(OUTPUT_FILE_NAME "pwsafe${LANG}.chm")
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${DIR}/${OUTPUT_FILE_NAME}")

    add_custom_command(OUTPUT "${OUTPUT_FILE}"
                       DEPENDS ${INPUT_FILES} cmake/html_help_hhc.cmake
                       COMMAND "${CMAKE_COMMAND}"
                            -D "HHC_EXECUTABLE=${HHC_EXECUTABLE}"
                            -D "INPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${DIR}"
                            -D "OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/${DIR}"
                            -D "OUTPUT_FILE=${OUTPUT_FILE}"
                            -D "OUTPUT_FILE_NAME=${OUTPUT_FILE_NAME}"
                            -P cmake/html_help_hhc.cmake
                       COMMENT "Compiling pwsafe ${LANG} help"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                       VERBATIM
                       )

    set(${OUTPUT_FILE_VAR} "${OUTPUT_FILE}" PARENT_SCOPE)
  endfunction(make_help)

else (WIN32 AND NOT WX_WINDOWS)
  function(make_help OUTPUT_FILE_VAR DIR LANG)
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/help${LANG}.zip")

    set(INPUT_DIR "${DIR}")
    # If a helpfile directory exists for wxWidgets (ending with _wx), use it for
    # generating the zip file, otherwise use the same one that's used for Windows.
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}_wx")
      set(INPUT_DIR "${DIR}_wx")
    endif()

    set_lang_help_files(INPUT_FILES "${INPUT_DIR}")
    add_custom_command(OUTPUT "${OUTPUT_FILE}"
      DEPENDS ${INPUT_FILES}
      COMMAND "${CMAKE_COMMAND}" -E tar cf "${OUTPUT_FILE}" . --format=zip
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_DIR}"
      VERBATIM
      )

    set(${OUTPUT_FILE_VAR} "${OUTPUT_FILE}" PARENT_SCOPE)
  endfunction(make_help)
endif (WIN32 AND NOT WX_WINDOWS)

unset(OUTPUT_FILES)

# English help is a special case: default -> pwsafe.chm

if (WIN32 AND NOT WX_WINDOWS)
  make_help(DEFAULT_OUTPUT_FILE default "")
else()
  make_help(DEFAULT_OUTPUT_FILE default_wx EN)
endif()

set(OUTPUT_FILES "${DEFAULT_OUTPUT_FILE}")

foreach (LANG ${LANGS})
  make_help(OUTPUT_FILE "pwsafe${LANG}" ${LANG})
  list(APPEND OUTPUT_FILES "${OUTPUT_FILE}")
endforeach(LANG)

if (WIN32 AND NOT WX_WINDOW)
option(PWS_BUILD_HELP "Build the help files by default." OFF)
else()
option(PWS_BUILD_HELP "Build the help files by default." ON)
endif()

if (PWS_BUILD_HELP)
  add_custom_target(helpfiles ALL DEPENDS ${OUTPUT_FILES})
else()
  add_custom_target(helpfiles DEPENDS ${OUTPUT_FILES})
endif ()
