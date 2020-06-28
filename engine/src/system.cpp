#include "system.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace pt = boost::property_tree;

using std::string;
using std::map;
using std::vector;
using std::cout;
using std::endl;



System::System(string const &system_file)
{
    root = Object();
    root.type = ("root");

    pt::ptree tree;
    pt::read_xml(system_file, tree);
    recursiveProcess(tree, root);
}

void System::recursiveProcess(pt::ptree tree, Object object)
{
    for (const auto& iterator : tree)
    {
        Object inner_object = Object();
        inner_object.type = iterator.first;
        //cout << iterator.first << endl;

        // Process attributes
        if(inner_object.type == "<xmlattr>")
        {
            for (const auto& attributes_iterator : iterator.second)
            {
                string key = attributes_iterator.first.data();
                string value = attributes_iterator.second.data();
                //cout << "Adding to " << object.type << " attribute " <<  key << "=" << value << endl;
                object.attributes[key] = value;
            }

            // We've already processed the children
            continue;
        }

        // Process children
        object.objects.push_back(inner_object);
        if(iterator.second.empty()) continue;
        recursiveProcess(iterator.second, inner_object);
    }
}
