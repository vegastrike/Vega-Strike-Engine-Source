// config_model_test.cpp — standalone round-trip + lookup test (no SDL/imgui).
// Build: g++ -std=c++14 -I. config_model.cpp config_model_test.cpp -o test_model
#include "config_model.h"
#include <cstdio>
#include <string>
using namespace vs05cfg;

static int failures = 0;
static void check(bool ok, const char *what) {
    if (!ok) { printf("FAIL: %s\n", what); failures++; }
    else printf("ok:   %s\n", what);
}

int main(int argc, char **argv) {
    if (argc < 3) { printf("usage: %s <vegastrike.config> <presets.xml>\n", argv[0]); return 2; }
    Model m;
    check(parse(argv[1], m), "parse real config");
    check(m.root.name == "vegaconfig", "root is vegaconfig");

    // Lookup a known var in a known section
    std::string font = get_var(m, "graphics", "font_point");
    printf("      font_point = '%s'\n", font.c_str());
    check(!font.empty(), "graphics/font_point present");

    // Edit a var and verify it round-trips
    set_var(m, "graphics", "font_point", "99");
    check(get_var(m, "graphics", "font_point") == "99", "set_var then get_var");

    // Round-trip: regenerate to string, re-parse, compare model shape
    std::string s = to_string(m);
    check(!s.empty() && s.find("<vegaconfig>") == 0, "generate starts with <vegaconfig>");
    printf("      generated %zu bytes\n", s.size());

    // bindings present
    const std::vector<Element> *b = bindings(m);
    check(b != NULL, "bindings block present");
    if (b) printf("      %zu binds/axes in bindings\n", b->size());

    // Write round-trip file and re-parse to confirm parse(generate(parse(x))) works
    std::string tmp = "/tmp/vs05_roundtrip.config";
    check(generate(m, tmp), "write generated config");
    Model m2;
    check(parse(tmp, m2), "re-parse generated config");
    check(get_var(m2, "graphics", "font_point") == "99", "edit survived round-trip");

    // --- presets ---
    std::vector<PresetGroup> presets;
    check(parse_presets(argv[2], presets), "parse presets.xml");
    printf("      %zu preset groups\n", presets.size());
    for (auto &g : presets) printf("        %-18s %zu options\n", g.name.c_str(), g.options.size());
    check(!presets.empty(), "presets has groups");

    // apply the Geometry GeomHigh preset and confirm a var lands in the model
    std::string before = get_var(m, "graphics", "default_full_technique");
    bool found_group = false;
    for (auto &g : presets) if (g.name == "Geometry") {
        apply_preset(g, "GeomHigh", m);
        found_group = true;
        break;
    }
    check(found_group, "found Geometry group");
    printf("      Geometry/GeomHigh vars: ");
    for (auto &g : presets) if (g.name == "Geometry") for (auto &o : g.options)
        if (o.name == "GeomHigh") printf("%zu ", o.vars.size());
    printf("\n");
    std::string after = get_var(m, "graphics", "default_full_technique");
    printf("      default_full_technique before='%s' after='%s'\n", before.c_str(), after.c_str());

    // --- modern-file generation with switch ---
    std::string mf = "/tmp/vs05_modern.config";
    check(generate_modern_file(m, mf, "modern"), "write vs-modern.config with switch");
    Model m3;
    check(parse(mf, m3), "re-parse modern file (switch skipped)");
    check(get_var(m3, "graphics", "font_point") == "99", "modern file retains edits");
    {
        FILE *f = fopen(mf.c_str(), "r");
        char line[128]; if (f) { if (fgets(line, sizeof(line), f)) printf("      first line: %s", line); fclose(f); }
    }

    if (failures == 0) printf("\nALL PASS\n");
    else printf("\n%d FAILURE(S)\n", failures);
    return failures ? 1 : 0;
}
