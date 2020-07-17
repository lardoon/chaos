set(nl "
")
file(WRITE ${OUT} "/* AUTO GENERATED */${nl}")
get_filename_component(FN ${IN} NAME_WE)

file(APPEND ${OUT} "#ifndef ${FN}_h_seen${nl}")
file(APPEND ${OUT} "#define ${FN}_h_seen${nl}")
file(APPEND ${OUT} "extern const unsigned char ${FN}_end[];${nl}")
file(APPEND ${OUT} "extern const unsigned char ${FN}[];${nl}")
file(APPEND ${OUT} "extern const unsigned int ${FN}_size;${nl}")
file(APPEND ${OUT} "#endif${nl}")
