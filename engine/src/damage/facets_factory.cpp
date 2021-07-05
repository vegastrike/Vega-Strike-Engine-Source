#include "facets_factory.h"

#include "gfx/vec.h"

DamageableFacet CreateFacetFromTemplate(const DamageableFacet& template_facet,
                                        const Vector& min_v,
                                        const Vector& max_v) {
    return DamageableFacet(min_v, max_v, template_facet);
}

const std::vector<DamageableFacet> FacetsCreator(const FacetConfiguration& configuration,
                                                 const DamageableFacet& template_facet) {
    if(configuration == FacetConfiguration::one) {
        const std::vector<DamageableFacet> facets = { template_facet };
        return facets;
    }
}
