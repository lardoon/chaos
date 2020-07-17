include(CheckCCompilerFlag)
set(WARN
    all extra init-self no-long-long
    everything
    abi
    address
    all
    bad-function-cast
    builtin-macro-redefined
    cast-qual
    char-subscripts
    clobbered
    comment
    comments
    deprecated
    div-by-zero
    double-promotion
    empty-body
    endif-labels
    enum-compare
    float-conversion
    float-equal
    format
    format-contains-nul
    format-extra-args
    format-nonliteral
    format-security
    format-y2k
    format-zero-length
    ignored-qualifiers
    implicit
    implicit-function-declaration
    implicit-int
    import
    init-self
    int-to-pointer-cast
    invalid-pch
    jump-misses-init
    logical-op
    main
    maybe-uninitialized
    missing-braces
    missing-declarations
    missing-field-initializers
    missing-format-attribute
    missing-include-dirs
    missing-parameter-type
    missing-prototypes
    multichar
    narrowing
    nested-externs
    nonnull
    old-style-declaration
    old-style-definition
    openmp-simd
    overlength-strings
    override-init
    packed-bitfield-compat
    parentheses
    pointer-arith
    pointer-sign
    pointer-to-int-cast
    pragmas
    psabi
    redundant-decls
    return-local-addr
    return-type
    sequence-point
    sign-compare
    sign-conversion

    sizeof-pointer-memaccess
    strict-prototypes
    switch
    switch-enum
    sync-nand
    trigraphs
    undef
    uninitialized
    unknown-pragmas
    unsuffixed-float-constants
    unused
    unused-local-typedefs
    unused-macros
    unused-result
    varargs
    variadic-macros
    vla
    volatile-register-var
    write-strings
    aggregate-return
    aggressive-loop-optimizations
    array-bounds
    attributes
    cast-align
    coverage-mismatch
    cpp
    deprecated-declarations
    disabled-optimization
    free-nonheap-object
    inline
    invalid-memory-model

    overflow
    packed
    shadow
    stack-protector
    strict-aliasing
    strict-overflow
    trampolines
    type-limits
    unsafe-loop-optimizations
    unused-but-set-parameter
    unused-but-set-variable
    unused-function
    unused-parameter
    unused-value
    unused-variable
    vector-operation-performance
    missing-noreturn
    unreachable-code
    )
set(NORMAL pipe)

foreach(flag ${WARN})
    check_c_compiler_flag(-W${flag} warn_${flag}_is_ok)
    if(warn_${flag}_is_ok)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W${flag}")
    endif()
endforeach()
foreach(flag ${NORMAL})
    check_c_compiler_flag(-${flag} ${flag}_is_ok)
    if(${flag}_is_ok)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -${flag}")
    endif()
endforeach()

check_c_compiler_flag(-pipe pipe_is_ok)
if(pipe_is_ok)
    set(CMAKE_EXE_LINKER_FLAGS "-pipe")
endif()

if("${CMAKE_C_FLAGS_RELEASE}" MATCHES "O[3-9]")
    message(STATUS "Reducing RELEASE optimization level to O2")
    string(REGEX REPLACE "O[3-9]" "O2" CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}"
        CACHE STRING "Flags used by the compiler during release builds" FORCE)
endif()
