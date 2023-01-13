cmake_minimum_required (VERSION 3.21)

message(STATUS "Building help file ${OUTPUT_FILE_NAME}")

if (NOT EXISTS "${HHC_EXECUTABLE}")
  find_program(HHC_EXECUTABLE hhc.exe DOC "Microsoft's help compiler (hhc.exe)")
  if (NOT HHC_EXECUTABLE)
    message(FATAL_ERROR "The Microsoft help compiler (hhc.exe) was not found.")
  endif ()
endif ()

file(COPY "${INPUT_DIR}/html/" DESTINATION "${OUTPUT_DIR}/html/")

file(COPY "${INPUT_DIR}/"
  DESTINATION "${OUTPUT_DIR}/"
  FILES_MATCHING PATTERN */pwsafe*.hh?)

# Help detect failed builds and the case where a .chm
# already exists in the source folder.
file(REMOVE "${OUTPUT_FILE}")

file(GLOB LOG_FILES
  LIST_DIRECTORIES FALSE
  "${OUTPUT_DIR}/*.log"
)
if (LOG_FILES)
  file(REMOVE ${LOG_FILES})
endif ()

execute_process(
  COMMAND "${HHC_EXECUTABLE}" pwsafe.hhp
  WORKING_DIRECTORY "${OUTPUT_DIR}"
  RESULT_VARIABLE HHC_RESULT
  OUTPUT_QUIET
)

if (HHC_RESULT EQUAL "1" AND EXISTS "${OUTPUT_FILE}")
  file(SIZE "${OUTPUT_FILE}" OUTPUT_SIZE)

  if ("${OUTPUT_SIZE}" GREATER 10000)
    return()
  endif ()
endif ()

file(REMOVE "${OUTPUT_FILE}")

file(GLOB LOG_FILES
  LIST_DIRECTORIES FALSE
  "${OUTPUT_DIR}/*.log"
)

unset(HHC_ERROR_MESSAGE)

foreach (FILE ${LOG_FILES})
  file(STRINGS "${FILE}" LOG_STRINGS)

  string(APPEND HHC_ERROR_MESSAGE " <<<<<${FILE}\n")
  foreach (LINE ${LOG_STRINGS})
    string(APPEND HHC_ERROR_MESSAGE " ${LINE}\n")
  endforeach ()
  string(APPEND HHC_ERROR_MESSAGE " ${FILE}>>>>>\n")
endforeach ()

string(APPEND HHC_ERROR_MESSAGE " ${HHC_EXECUTABLE}: failed for ${OUTPUT_FILE}.")
message(FATAL_ERROR "${HHC_ERROR_MESSAGE}")
