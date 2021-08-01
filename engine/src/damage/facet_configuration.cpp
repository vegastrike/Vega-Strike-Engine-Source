
#include "facet_configuration.h"


int Find(FacetName facet_name, const FacetName facet_array[], int size) {
    for(int i=0;i<size;i++) {
        if(facet_name == facet_array[i]) {
            return i;
        }
    }

    return -1;
}

int FacetConfigurationSize(FacetConfiguration configuration) {
    if(configuration ==  FacetConfiguration::one) { return 1; }
    if(configuration ==  FacetConfiguration::two) { return 2; }
    if(configuration ==  FacetConfiguration::four) { return 4; }
    if(configuration ==  FacetConfiguration::six) { return 6; }
    if(configuration ==  FacetConfiguration::eight) { return 8; }
}

const FacetName* FacetConfigurationByName(FacetConfiguration configuration) {
    if(configuration ==  FacetConfiguration::one) { return one_configuration; }
    if(configuration ==  FacetConfiguration::two) { return two_configuration; }
    if(configuration ==  FacetConfiguration::four) { return four_configuration; }
    if(configuration ==  FacetConfiguration::six) { return six_configuration; }
    if(configuration ==  FacetConfiguration::eight) { return eight_configuration; }
}
