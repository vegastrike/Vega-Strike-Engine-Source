#include <string>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

#include "graphics_config.h"
#include "python_utils.h"



BOOST_PYTHON_MODULE(vegastrike_python) {
    boost::python::class_<Graphics2Config>("GraphicsConfig", boost::python::init<>())
    .def_readwrite("screen", &Graphics2Config::screen)
    .def_readwrite("resolution_x", &Graphics2Config::resolution_x)
    .def_readwrite("resolution_y", &Graphics2Config::resolution_y);
}



Graphics2Config GetGraphics2Config(
    const std::string build_path,
    const std::string path_string,
    const std::string file_name,
    const std::string function_name) {
    PyObject* object = GetClassFromPython(
        build_path, path_string, file_name, function_name);
                                           

    Graphics2Config cfg2 = boost::python::extract<Graphics2Config>(object);
    return cfg2;
}