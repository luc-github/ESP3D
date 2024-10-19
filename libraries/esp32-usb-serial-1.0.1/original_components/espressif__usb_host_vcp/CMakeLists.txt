idf_component_register(SRCS "usb_host_vcp.cpp"
                    INCLUDE_DIRS "include")

set_target_properties(${COMPONENT_LIB} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fconcepts)
