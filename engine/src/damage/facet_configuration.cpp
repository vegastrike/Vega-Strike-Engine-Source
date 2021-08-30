
#include "facet_configuration.h"


int Find(FacetName facet_name, const FacetName facet_array[], int size) {
    for(int i=0;i<size;i++) {
        if(facet_name == facet_array[i]) {
            return i;
        }
    }

    return -1;
}



const FacetName* FacetConfigurationByName(FacetConfiguration configuration) {
    switch(configuration) {
    case FacetConfiguration::one: return one_configuration;
    case FacetConfiguration::two: return two_configuration;
    case FacetConfiguration::four: return four_configuration;
    case FacetConfiguration::six: return six_configuration;
    case FacetConfiguration::eight: return eight_configuration;
    }

    return one_configuration; // Literally cannot happen
}
