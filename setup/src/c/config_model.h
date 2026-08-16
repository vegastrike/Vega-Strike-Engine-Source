// config_model.h — minimal flat XML config model for the VS-05 settings app.
//
// One recursive value struct (no pointers, no parent indices) represents every
// config element (section/var/bind/axis/color/hatswitch) uniformly. Attributes
// are ordered name/value pairs. A small parser + generator round-trips the
// engine's vegastrike.config (flat <variables> sections, <bindings>, <colors>).
//
// This is standalone (std::string/vector only) so it can be tested without the
// SDL3/imgui UI.
#ifndef VS05_CONFIG_MODEL_H
#define VS05_CONFIG_MODEL_H

#include <string>
#include <vector>
#include <utility>

namespace vs05cfg {

struct Element {
    std::string name;                                   // "section"|"var"|"bind"|"axis"|"color"|"hatswitch"
    std::vector<std::pair<std::string,std::string>> attrs;  // ordered, preserves duplicates
    std::vector<Element> children;                      // sub-elements (hatswitch children of axis)
};

// The whole config: <vegaconfig> with <bindings>, <colors>, <variables>.
struct Model {
    Element root;                                       // the <vegaconfig> element
};

// Parse a config file into a Model. Returns false on unparseable input.
bool parse(const std::string &filename, Model &out);

// Parse a bare-root XML file (e.g. <bindings>...</bindings>) into an Element.
bool parse_file_root(const std::string &filename, Element &out);

// Serialize a Model to a file as engine-readable plain XML.
bool generate(const Model &m, const std::string &filename);

// Write vs-modern.config: a first-line mode-switch comment, then (if preset_header
// is non-empty) an app-owned #set header comment the engine ignores, then the
// model as engine-readable plain XML. mode is "modern" or "classic".
bool generate_modern_file(const Model &m, const std::string &filename,
                          const std::string &mode, const std::string &preset_header = "");

// Serialize a Model to a string (used by tests / preview).
std::string to_string(const Model &m);

// --- lookups / edits (by section-path and var name) ---

// Find a <section name=S> under <variables>, or null.
const Element *find_variables_section(const Model &m, const std::string &section);
Element       *find_variables_section(      Model &m, const std::string &section);

// Get/set a <var name=N> value inside a variables section. get returns "" if absent.
std::string get_var(const Model &m, const std::string &section, const std::string &name);
void set_var(Model &m, const std::string &section, const std::string &name, const std::string &value);

// Get an attribute from an element ("" if absent).
std::string attr(const Element &e, const std::string &name);
void set_attr(Element &e, const std::string &name, const std::string &value);
void remove_attr(Element &e, const std::string &name);

// All <bind> elements under <bindings> (for the Bindings dialog / tests).
const std::vector<Element> *bindings(const Model &m);

// Merge the <bind> elements from a bindings.xml-style file (a bare <bindings> root)
// into the model's <bindings> block, replacing the existing <bind> elements. Returns
// false if the file can't be read.
bool apply_bindings_file(Model &m, const std::string &filename);

// --- axis elements under <bindings> (joystick flight roles) ---

// Find an <axis name=N> under <bindings> (const + non-const).
const Element *find_axis(const Model &m, const std::string &name);
Element       *find_axis(      Model &m, const std::string &name);

// Set/remove an <axis name=N joystick=J axis=A inverse=B> element under <bindings>.
// If bound (axis>=0) it is created or updated; if unbound it is removed.
void set_axis(Model &m, const std::string &name, int joystick, int axis, bool inverse);
void remove_axis(Model &m, const std::string &name);

// ---------------------------------------------------------------------------
// Presets (setup/presets.xml): the modern UI's preset structure.
// ---------------------------------------------------------------------------

// One preset option and the vars it sets.
struct PresetOption {
    std::string name;
    std::string desc;
    std::vector<std::pair<std::string,std::string>> vars;  // (var-name, value)
    // Non-var elements the preset sets (e.g. <axis mouse=...> for mouse/joystick modes).
    std::vector<Element> elements;
};

// One group of preset options (e.g. Geometry -> GeomHigh...).
struct PresetGroup {
    std::string name;
    std::string current;   // active option name (from the #set line / selection)
    std::vector<PresetOption> options;
};

// Parse the shipped presets file (setup/presets.xml) into groups + options.
// Reads the #groups/#cat/#set/#desc header and the #option ... #end preset
// blocks. Returns false if the file can't be read.
bool parse_presets(const std::string &filename, std::vector<PresetGroup> &out);

// Apply one option's preset vars to a model (used on Save for each group's
// active selection). Each var's real section is located by name in the model.
void apply_preset(PresetGroup &g, const std::string &option, Model &m);

} // namespace vs05cfg

#endif
