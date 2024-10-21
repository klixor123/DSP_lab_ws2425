#
# helper functions
#

include_guard(GLOBAL)

#
# debug helpers:
#
#   cmake_print_properties(<TARGETS       [<target1> ...] |
#                           SOURCES       [<source1> ...] |
#                           DIRECTORIES   [<dir1> ...]    |
#                           TESTS         [<test1> ...]   |
#                           CACHE_ENTRIES [<entry1> ...]  >
#                           PROPERTIES [<prop1> ...]
#                           )
#
#   cmake_print_variables(var1 var2 ..  varN)
#
include(CMakePrintHelpers)


#
# Add a linker script to the given target.
# The script will be passed through the preprocessor using current include paths
# and definitions to produce the effective script passed to the linker.
#
function(add_linker_script tgt ldscript)

    # dir where target will be placed
    get_target_property(bin_dir ${tgt} BINARY_DIR)

    # target name without ext
    cmake_path(REMOVE_EXTENSION tgt OUTPUT_VARIABLE tgt_basename)

    # generated script file path
    set(generated_linker_script ${bin_dir}/${tgt_basename}_GeneratedLinkerScript.ld)

    # pass it to the linker
    target_link_options(${tgt} PRIVATE "-T${generated_linker_script}")

    # add to target sources so that compilation gets triggerd
    target_sources(${tgt} PRIVATE ${generated_linker_script})

    # add a custom rule which preprocesses the script
    add_custom_command(
        OUTPUT "${generated_linker_script}"
        DEPENDS "${ldscript}"
        COMMENT "Generating linker script ${generated_linker_script}"
        COMMAND ${CMAKE_C_COMPILER}
            -o ${generated_linker_script}
            -E ${ldscript}
            -P
            "-I$<JOIN:$<TARGET_PROPERTY:${tgt},INCLUDE_DIRECTORIES>,;-I>"
            "-D$<JOIN:$<TARGET_PROPERTY:${tgt},COMPILE_DEFINITIONS>,;-D>"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    # remember file names in custom target properties
    set_target_properties(${tgt} PROPERTIES
        SourceLinkerScript "${ldscript}"
        EffectiveLinkerScript "${generated_linker_script}"
    )

endfunction()


#
# When the linker runs to produce the given target, automatically generate
# an object file that contains a string constant as follows:
#
#   const char strname[] = __DATE__ " " __TIME__;
#
# In order to use this string constant in your code, declare it as follows:
#
#   extern const char strname[];
#
function(create_link_timestamp_string tgt strname)

    # dir where target will be placed
    get_target_property(bin_dir ${tgt} BINARY_DIR)

    # target name without ext
    cmake_path(REMOVE_EXTENSION tgt OUTPUT_VARIABLE tgt_basename)

    # generated files
    set(c_file ${bin_dir}/${tgt_basename}_GeneratedLinkTimestamp.c)
    set(o_file ${bin_dir}/${tgt_basename}_GeneratedLinkTimestamp.o)

    # convert white space separated CMAKE_C_FLAGS to list
    set(c_flags_as_list ${CMAKE_C_FLAGS})
    separate_arguments(c_flags_as_list)

    # pre-link commands that generate the source and object files
    add_custom_command(TARGET ${tgt}
        PRE_LINK
        COMMENT "Generate link timestamp"
        COMMAND ${CMAKE_COMMAND} -E echo "/* auto-generated code */ const char ${strname}[] = __DATE__ \" \" __TIME__\;" > ${c_file}
        COMMAND ${CMAKE_C_COMPILER} ${c_flags_as_list} -o ${o_file} -c ${c_file}
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    # link the given target against the generated object file
    target_link_options(${tgt} PRIVATE ${o_file})

endfunction()


#
# Add linker options to generate a map file.
#
# create_linker_map_file(
#     <target>
#     [ANNOTATE]
#   )
#
#   ANNOTATE
#       Create an annotated copy of the .map file to improve readability.
#       The variable TLIMGTOOL must point to tlimgtool.exe.
#
function(create_linker_map_file tgt)

    set(options ANNOTATE)

    cmake_parse_arguments(
        arg             # <prefix>
        "${options}"    # <options>
        ""              # <one_value_keywords>
        ""              # <multi_value_keywords>
        ${ARGN}         # <args>...
    )

    # full path to tgt file
    set(target_file "$<TARGET_FILE:${tgt}>")
    # full path but extension removed
    set(target_file_base "$<PATH:REMOVE_EXTENSION,${target_file}>")
    # directory only
    set(target_dir "$<PATH:GET_PARENT_PATH,${target_file}>")
    # filename without extension
    set(target_file_stem "$<PATH:GET_STEM,${target_file}>")


    target_link_options(${tgt} PRIVATE "-Wl,-Map=${target_file_base}.map")

    # remember file names in custom target properties
    set_target_properties(${tgt} PROPERTIES
        LinkerMapFile "${target_file_base}.map"
    )

    if (arg_ANNOTATE)
        set(out_file "${target_file_stem}.map-annotated")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${TLIMGTOOL} annotatemap --force "${target_file_stem}.map" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # To ensure no outdated files remain in case the link step fails,
        # we delete any existing output files before we run the linker.
        add_custom_command(TARGET ${tgt}
            PRE_LINK
            COMMENT "Delete old ${out_file}"
            COMMAND ${CMAKE_COMMAND} -E rm -f ${out_file}
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
    endif()

endfunction()


#
# Add post-build commands to the given target.
#
# add_post_build_steps(
#     <target>
#     [SYMBOLS]
#     [SECTIONS]
#     [DISASM]
#     [BIN]
#     [HEX]
#     [TLIMG]
#     [TLIMG_HEX]
#     [TLIMG_NAME <name>]
#   )
#
#   SYMBOLS
#       Create symbols listing (.symbols).
#
#   SECTIONS
#       Create sections listing (.sections).
#
#   DISASM
#       Create disassembler listing (.disasm).
#
#   BIN
#       Create flat binary image (.bin).
#
#   HEX
#       Create Intel HEX file (.hex).
#
#   RAW_BIN
#       Create flat binary image (.bin) with .boot_header section removed.
#
#   RAW_HEX
#       Create Intel HEX file (.hex) with .boot_header section removed.
#
#   C_INC
#       Create C-style inc file from binary image. Implies BIN.
#       The variable TLIMGTOOL must point to tlimgtool.exe.
#
#   TLIMG
#       Create flat binary image with metadata and checksum (.tlimg). Implies BIN.
#       The variable TLIMGTOOL must point to tlimgtool.exe.
#
#   TLIMG_HEX
#       Create Intel HEX file from binary image with metadata and checksum (.tlimg.hex). Implies TLIMG.
#       The variable TLIMGTOOL must point to tlimgtool.exe.
#
#   TLIMG_NAME <name>
#       Specifies the basename (stem) of the resulting .tlimg and .tlimg.hex files.
#       If not given, defaults to target basename.
#
function(add_post_build_steps tgt)

    set(options SYMBOLS SECTIONS DISASM BIN HEX RAW_BIN RAW_HEX C_INC TLIMG TLIMG_HEX)
    set(one_value_keywords TLIMG_NAME)

    cmake_parse_arguments(
        arg                     # <prefix>
        "${options}"            # <options>
        "${one_value_keywords}" # <one_value_keywords>
        ""                      # <multi_value_keywords>
        ${ARGN}                 # <args>...
    )

    # .hex implies .tlimg
    if (arg_TLIMG_HEX)
        set(arg_TLIMG "TRUE")
    endif()

    # .tlimg implies .bin
    if (arg_TLIMG)
        set(arg_BIN "TRUE")
    endif()

    # .bin is required for .inc
    if (arg_C_INC)
        set(arg_BIN "TRUE")
    endif()

    # full path to tgt file
    set(target_file "$<TARGET_FILE:${tgt}>")
    # full path but extension removed
    set(target_file_base "$<PATH:REMOVE_EXTENSION,${target_file}>")
    # filename without extension
    set(target_file_stem "$<PATH:GET_STEM,${target_file}>")
    # directory only
    set(target_dir "$<PATH:GET_PARENT_PATH,${target_file}>")

    # For further reference, attach path as custom target property.
    set_target_properties(${tgt} PROPERTIES TargetImageDir "${target_dir}")

    # .tlimg filename without extension
    if (DEFINED arg_TLIMG_NAME)
        set(tlimg_file_stem "${arg_TLIMG_NAME}")
    else()
        set(tlimg_file_stem "${target_file_stem}")
    endif()


    if (arg_SYMBOLS)
        set(out_file "${target_file_base}.symbols")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_NM} --numeric-sort --line-numbers --demangle --print-size "${target_file}" ">" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
    endif()

    if (arg_SECTIONS)
        set(out_file "${target_file_base}.sections")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJDUMP} -h -w "${target_file}" ">" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
    endif()

    if (arg_DISASM)
        set(out_file "${target_file_base}.disasm")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJDUMP} --disassemble --demangle "${target_file}" ">" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
    endif()

    if (arg_BIN)
        set(out_file "${target_file_stem}.bin")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJCOPY} --output-target binary "${target_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetBinFile "${out_file}")
    endif()

    if (arg_HEX)
        set(out_file "${target_file_stem}.hex")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJCOPY} --output-target ihex "${target_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetHexFile "${out_file}")
    endif()

    if (arg_RAW_BIN)
        set(out_file "${target_file_stem}_raw.bin")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJCOPY} --remove-section .boot_header --output-target binary "${target_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetRawBinFile "${out_file}")
    endif()

    if (arg_RAW_HEX)
        set(out_file "${target_file_stem}_raw.hex")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${CMAKE_OBJCOPY} --remove-section .boot_header --output-target ihex "${target_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetRawHexFile "${out_file}")
    endif()

    if (arg_C_INC)
        # Retrieve property set above.
        get_target_property(target_bin_file ${tgt} TargetBinFile)
        set(out_file "${target_file_stem}.inc")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${TLIMGTOOL} bin2c --force "${target_bin_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetCIncFile "${out_file}")
    endif()

    if (arg_TLIMG)
        # Retrieve property set above.
        get_target_property(target_bin_file ${tgt} TargetBinFile)
        set(out_file "${tlimg_file_stem}.tlimg")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${TLIMGTOOL} addcrc --force "${target_bin_file}" "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetTLImgFile "${out_file}")
    endif()

    if (arg_TLIMG_HEX)
        # Retrieve property set above.
        get_target_property(target_tlimg_file ${tgt} TargetTLImgFile)
        set(out_file "${tlimg_file_stem}.tlimg.hex")
        list(APPEND post_build_output "${out_file}")
        add_custom_command(TARGET ${tgt}
            POST_BUILD
            COMMENT "Creating ${out_file}"
            COMMAND ${TLIMGTOOL} convert --force "${target_tlimg_file}" -ohex "${out_file}"
            WORKING_DIRECTORY "${target_dir}"
            VERBATIM
        )
        # For further reference, attach output filename as custom target property.
        set_target_properties(${tgt} PROPERTIES TargetTLImgHexFile "${out_file}")
    endif()

    # To ensure no outdated files remain in case the link step fails,
    # we delete any existing output files before we run the linker.
    if (post_build_output)
        add_custom_command(TARGET ${tgt}
            PRE_LINK
            COMMENT "Delete old post-build products"
            COMMAND ${CMAKE_COMMAND} -E rm -f ${post_build_output}
            WORKING_DIRECTORY "${target_dir}"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()

endfunction()


#
# Combine the .tlimg files produces by two targets (bl and app)
# into one flat binary image.
# This produces two files: result.bin and result.hex
#
function(create_combined_image tgt1 tgt2 result)

    # Retrieve properties set in add_post_build_steps
    get_target_property(tgt1_tlimg_file ${tgt1} TargetTLImgFile)
    get_target_property(tgt2_tlimg_file ${tgt2} TargetTLImgFile)

    get_target_property(target_dir ${tgt1} TargetImageDir)

    add_custom_command(
        OUTPUT "${result}.bin" "${result}.hex"
        COMMENT "Creating ${result}"
        COMMAND ${TLIMGTOOL} combine --force "${tgt1_tlimg_file}" "${tgt2_tlimg_file}" -obin "${result}.bin" -ohex "${result}.hex"
        WORKING_DIRECTORY "${target_dir}"
        VERBATIM
        DEPENDS ${tgt1} ${tgt2}
    )

    add_custom_target(combined_image ALL
        DEPENDS "${result}.bin" "${result}.hex"
    )

endfunction()




#
# debugging helpers
#


# Sets the CMAKE_PROPERTY_LIST and CMAKE_WHITELISTED_PROPERTY_LIST variables to the list of properties.
# Taken from https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake
function(get_cmake_property_list)
    # See https://stackoverflow.com/a/44477728/240845
    set(LANGS C CXX ASM)
    set(AVAILABLE_CONFIGURATION_TYPES "Debug,Release")

    # Get all propreties that cmake supports
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

    # Populate "<CONFIG>" with AVAILBLE_CONFIG_TYPES
    set(CONFIG_LINES ${CMAKE_PROPERTY_LIST})
    list(FILTER CONFIG_LINES INCLUDE REGEX "<CONFIG>")
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "<CONFIG>")
    foreach(CONFIG_LINE IN LISTS CONFIG_LINES)
        foreach(CONFIG_VALUE IN LISTS AVAILABLE_CONFIGURATION_TYPES)
            string(REPLACE "<CONFIG>" "${CONFIG_VALUE}" FIXED "${CONFIG_LINE}")
            list(APPEND CMAKE_PROPERTY_LIST ${FIXED})
        endforeach()
    endforeach()

    # Populate "<LANG>" with LANGS
    set(LANG_LINES ${CMAKE_PROPERTY_LIST})
    list(FILTER LANG_LINES INCLUDE REGEX "<LANG>")
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "<LANG>")
    foreach(LANG_LINE IN LISTS LANG_LINES)
        foreach(LANG IN LISTS LANGS)
            string(REPLACE "<LANG>" "${LANG}" FIXED "${LANG_LINE}")
            list(APPEND CMAKE_PROPERTY_LIST ${FIXED})
        endforeach()
    endforeach()

    # no repeats
    list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)

    # Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "^LOCATION$|^LOCATION_|_LOCATION$")

    list(SORT CMAKE_PROPERTY_LIST)

    # Whitelisted property list for use with interface libraries to reduce warnings
    set(CMAKE_WHITELISTED_PROPERTY_LIST ${CMAKE_PROPERTY_LIST})

    # regex from https://stackoverflow.com/a/51987470/240845
    list(FILTER CMAKE_WHITELISTED_PROPERTY_LIST INCLUDE REGEX "^(INTERFACE|[_a-z]|IMPORTED_LIBNAME_|MAP_IMPORTED_CONFIG_)|^(COMPATIBLE_INTERFACE_(BOOL|NUMBER_MAX|NUMBER_MIN|STRING)|EXPORT_NAME|IMPORTED(_GLOBAL|_CONFIGURATIONS|_LIBNAME)?|NAME|TYPE|NO_SYSTEM_FROM_IMPORTED)$")

    # make the lists available
    set(CMAKE_PROPERTY_LIST ${CMAKE_PROPERTY_LIST} PARENT_SCOPE)
    set(CMAKE_WHITELISTED_PROPERTY_LIST ${CMAKE_WHITELISTED_PROPERTY_LIST} PARENT_SCOPE)
endfunction()

# Taken from https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake
function(print_target_properties tgt)
    if(NOT TARGET ${tgt})
      message("There is no target named '${tgt}'")
      return()
    endif()

    get_cmake_property_list()

    get_target_property(target_type ${tgt} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(PROPERTIES ${CMAKE_WHITELISTED_PROPERTY_LIST})
    else()
        set(PROPERTIES ${CMAKE_PROPERTY_LIST})
    endif()

    foreach (prop ${PROPERTIES})
        #message ("Checking ${prop}")
        get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
        if (propval)
            get_target_property(propval ${tgt} ${prop})
            message("${tgt} : ${prop} = ${propval}")
        else()
            #message("${tgt} : ${prop} = [empty]")
        endif()
    endforeach(prop)
endfunction()
