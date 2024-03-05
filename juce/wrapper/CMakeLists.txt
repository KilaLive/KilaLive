cmake_minimum_required(VERSION 2.8.6)
project(juce)

macro(source_group_by_dir source_files sgbd_cur_dir)
    if(MSVC)
        #set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            #message(STATUS "sgbd_fpath ${sgbd_fpath}")
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            #message(STATUS "sgbd_group_name ${sgbd_group_name}")
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            #message(STATUS "sgbd_group_name ${sgbd_group_name}")
            source_group(${sgbd_group_name} FILES ${sgbd_file})
            #break()
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)


set(JUCE_MODULES_USED 
    #juce_analytics
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    #juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    #juce_blocks_basics
    #juce_box2d
    juce_core
    juce_cryptography
    juce_data_structures
    #juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
    juce_opengl
    #juce_osc
    #juce_product_unlocking
    juce_video
    )

set(JUCE_SRCS "")    
set(JUCE_MODULES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../modules)

foreach(each_module ${JUCE_MODULES_USED})    
    file(GLOB_RECURSE HEADERS ${JUCE_MODULES_PATH}/${each_module}/*.h)
	
    file(GLOB_RECURSE CPPS   ${JUCE_MODULES_PATH}/${each_module}/*.cpp)
	set_source_files_properties(${CPPS} PROPERTIES HEADER_FILE_ONLY TRUE)
	
    set(SRCS ${HEADERS} ${CPPS})
    #message(STATUS "HEADERS ${HEADERS}")
    #message(STATUS "CPPS ${CPPS}")
    source_group_by_dir(SRCS ${JUCE_MODULES_PATH})
    list(APPEND JUCE_SRCS ${SRCS})
    #message(STATUS "JUCE_CPPS ${JUCE_CPPS}")
endforeach(each_module)


file(GLOB JUCE_WRAPPER  source/*.cpp source/*.h)
source_group("wrapper" FILES ${JUCE_WRAPPER})

include_directories(${JUCE_MODULES_PATH} 
                    source
                    ${JUCE_MODULES_PATH}/../3rdparty/ #asio
                    ${JUCE_MODULES_PATH}/../3rdparty/VST_SDK/VST3_SDK)
add_library(juce STATIC ${JUCE_WRAPPER} ${JUCE_SRCS})
#target_link_libraries()