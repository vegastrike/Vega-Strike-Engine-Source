add_definitions(-DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_PYTHON_STATIC_LIB)
include_directories("${vsUTCS_SOURCE_DIR}/boost/1_35")
add_library(boost_python src/list.cpp src/module.cpp src/object_protocol.cpp src/dict.cpp src/numeric.cpp src/errors.cpp src/exec.cpp src/str.cpp src/import.cpp src/wrapper.cpp src/object_operators.cpp src/long.cpp src/object/pickle_support.cpp src/object/life_support.cpp src/object/inheritance.cpp src/object/function_doc_signature.cpp src/object/stl_iterator.cpp src/object/class.cpp src/object/enum.cpp src/object/function.cpp src/object/iterator.cpp src/converter/from_python.cpp src/converter/registry.cpp src/converter/arg_to_python_base.cpp src/converter/builtin_converters.cpp src/converter/type_id.cpp src/tuple.cpp)

