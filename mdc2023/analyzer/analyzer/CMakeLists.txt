add_definitions (-DANADIR="${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable(mdcanalyzer analyzer.cpp
                HitVectorFlowEvent.cpp 
                AnaFillHits.cpp
                AnaPixelHistos.cpp)
target_compile_options(mdcanalyzer PUBLIC -DHAVE_ROOT -DHAVE_LIBZ -DHAVE_TMFE)
target_link_libraries(mdcanalyzer rt util dl pthread ${ROOT_LIBRARIES} manalyzer manalyzer_main midas pthread util)
install(TARGETS mdcanalyzer DESTINATION bin)
