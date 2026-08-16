// config_model.cpp — implementation of the minimal XML config model.
#include "config_model.h"
#include <cstdio>
#include <cstring>
#include <cctype>

namespace vs05cfg {

// ---------------------------------------------------------------------------
// XML-lite tokenizer over the whole file text.
// ---------------------------------------------------------------------------

static std::string read_all(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return "";
    std::string s;
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    fclose(f);
    return s;
}

// Skip whitespace and <!-- comments -->, advancing i. Leaves i at '<' or end.
static void skip_ws_and_comments(const std::string &s, size_t &i) {
    for (;;) {
        while (i < s.size() && isspace((unsigned char)s[i])) i++;
        if (i + 3 < s.size() && s[i] == '<' && s[i+1] == '!' && s[i+2] == '-' && s[i+3] == '-') {
            size_t end = s.find("-->", i + 4);
            if (end == std::string::npos) { i = s.size(); return; }
            i = end + 3;
            continue;
        }
        return;
    }
}

// Parse one attribute (name + optional ="value" or ='value').
static bool parse_attr(const std::string &s, size_t &i,
                       std::pair<std::string,std::string> &out) {
    size_t start = i;
    while (i < s.size() && !isspace((unsigned char)s[i]) && s[i] != '=' && s[i] != '>' && s[i] != '/')
        i++;
    out.first = s.substr(start, i - start);
    // skip whitespace before '=' (e.g. <var value ="x"/>)
    while (i < s.size() && isspace((unsigned char)s[i])) i++;
    if (i < s.size() && s[i] == '=') {
        i++; // skip '='
        while (i < s.size() && isspace((unsigned char)s[i])) i++;
        if (i >= s.size()) return false;
        char q = s[i];
        if (q == '"' || q == '\'') {
            i++;
            size_t end = s.find(q, i);
            if (end == std::string::npos) return false;
            out.second = s.substr(i, end - i);
            i = end + 1;
        }
    }
    return true;
}

// Parse one element (recursive). Expects s[i] == '<'.
static bool parse_element(const std::string &s, size_t &i, Element &out) {
    if (i >= s.size() || s[i] != '<') return false;
    i++; // '<'
    size_t start = i;
    while (i < s.size() && !isspace((unsigned char)s[i]) && s[i] != '>' && s[i] != '/')
        i++;
    out.name = s.substr(start, i - start);

    // attributes
    for (;;) {
        while (i < s.size() && isspace((unsigned char)s[i])) i++;
        if (i >= s.size()) return false;
        if (s[i] == '>') { i++; break; }                       // open tag, has children
        if (s[i] == '/' && i + 1 < s.size() && s[i+1] == '>') { i += 2; return true; } // self-closing
        std::pair<std::string,std::string> a;
        if (!parse_attr(s, i, a)) return false;
        out.attrs.push_back(a);
    }

    // children / text
    for (;;) {
        skip_ws_and_comments(s, i);
        if (i >= s.size()) return false;
        if (s[i] == '<' && i + 1 < s.size() && s[i+1] == '/') {  // closing tag
            i += 2;
            while (i < s.size() && s[i] != '>') i++;
            if (i < s.size()) i++;
            return true;
        }
        if (s[i] == '<') {
            Element child;
            if (!parse_element(s, i, child)) return false;
            out.children.push_back(child);
        } else {
            // bare text: skip to next '<'
            while (i < s.size() && s[i] != '<') i++;
        }
    }
}

bool parse(const std::string &filename, Model &out) {
    std::string s = read_all(filename.c_str());
    if (s.empty()) return false;
    size_t i = 0;
    skip_ws_and_comments(s, i);
    // tolerate an optional <?xml ...?> declaration
    if (s.compare(i, 5, "<?xml") == 0) {
        size_t end = s.find("?>", i);
        if (end == std::string::npos) return false;
        i = end + 2;
        skip_ws_and_comments(s, i);
    }
    Element root;
    if (!parse_element(s, i, root)) return false;
    out.root = root;
    return true;
}

// Parse a bare-root XML file (e.g. <bindings>...</bindings>) into an Element.
bool parse_file_root(const std::string &filename, Element &out) {
    std::string s = read_all(filename.c_str());
    if (s.empty()) return false;
    size_t i = 0;
    skip_ws_and_comments(s, i);
    if (s.compare(i, 5, "<?xml") == 0) {
        size_t end = s.find("?>", i);
        if (end == std::string::npos) return false;
        i = end + 2;
        skip_ws_and_comments(s, i);
    }
    return parse_element(s, i, out);
}

// ---------------------------------------------------------------------------
// Generator
// ---------------------------------------------------------------------------

static void write_attrs(const Element &e, std::string &o) {
    for (auto &a : e.attrs) {
        o += ' ';
        o += a.first;
        o += "=\"";
        o += a.second;
        o += '"';
    }
}

static void write_element(const Element &e, std::string &o, int depth) {
    o.append((size_t)depth, '\t');
    o += '<';
    o += e.name;
    write_attrs(e, o);
    // Only leaf elements (var/bind/axis/hatswitch) self-close when empty. Sections
    // (and any other container) always use open/close - the engine parser rejects
    // self-closing <section/>, which silently breaks config loading (the
    // graphics/general + graphics/mesh empty sections caused a launch crash).
    bool leaf = (e.name == "var" || e.name == "bind" || e.name == "axis" || e.name == "hatswitch");
    if (e.children.empty() && leaf) {
        o += "/>\n";
        return;
    }
    o += ">\n";
    for (auto &c : e.children)
        write_element(c, o, depth + 1);
    o.append((size_t)depth, '\t');
    o += "</";
    o += e.name;
    o += ">\n";
}

std::string to_string(const Model &m) {
    std::string o;
    write_element(m.root, o, 0);
    return o;
}

static bool write_all(const char *path, const std::string &s) {
    FILE *f = fopen(path, "w");
    if (!f) return false;
    size_t n = fwrite(s.c_str(), 1, s.size(), f);
    fclose(f);
    return n == s.size();
}

bool generate(const Model &m, const std::string &filename) {
    return write_all(filename.c_str(), to_string(m));
}

bool generate_modern_file(const Model &m, const std::string &filename, const std::string &mode, const std::string &preset_header) {
    std::string s = "<!-- vssetup:mode=" + mode + " -->\n";
    if (!preset_header.empty()) {
        s += "<!-- vssetup presets\n";
        s += preset_header;
        s += "-->\n";
    }
    s += to_string(m);
    return write_all(filename.c_str(), s);
}

// ---------------------------------------------------------------------------
// Lookups / edits
// ---------------------------------------------------------------------------

// <variables> element under root, or null.
static const Element *variables_elem(const Model &m) {
    for (auto &c : m.root.children)
        if (c.name == "variables") return &c;
    return NULL;
}

static Element *variables_elem(Model &m) {
    for (auto &c : m.root.children)
        if (c.name == "variables") return &c;
    return NULL;
}

const Element *find_variables_section(const Model &m, const std::string &section) {
    const Element *v = variables_elem(m);
    if (!v) return NULL;
    for (auto &c : v->children)
        if (c.name == "section" && attr(c, "name") == section) return &c;
    return NULL;
}

Element *find_variables_section(Model &m, const std::string &section) {
    Element *v = variables_elem(m);
    if (!v) return NULL;
    for (auto &c : v->children)
        if (c.name == "section" && attr(c, "name") == section) return &c;
    return NULL;
}

std::string attr(const Element &e, const std::string &name) {
    for (auto &a : e.attrs)
        if (a.first == name) return a.second;
    return "";
}

void set_attr(Element &e, const std::string &name, const std::string &value) {
    for (auto &a : e.attrs) {
        if (a.first == name) { a.second = value; return; }
    }
    e.attrs.push_back(std::make_pair(name, value));
}

void remove_attr(Element &e, const std::string &name) {
    for (size_t i = 0; i < e.attrs.size(); i++)
        if (e.attrs[i].first == name) { e.attrs.erase(e.attrs.begin() + i); return; }
}

bool has_attr(const Element &e, const std::string &name) {
    for (auto &a : e.attrs)
        if (a.first == name) return true;
    return false;
}

std::string get_var(const Model &m, const std::string &section, const std::string &name) {
    const Element *s = find_variables_section(m, section);
    if (!s) return "";
    for (auto &c : s->children)
        if (c.name == "var" && attr(c, "name") == name) return attr(c, "value");
    return "";
}

void set_var(Model &m, const std::string &section, const std::string &name, const std::string &value) {
    Element *s = find_variables_section(m, section);
    if (!s) return;  // section not found -> cannot set (caller ensures it exists)
    for (auto &c : s->children) {
        if (c.name == "var" && attr(c, "name") == name) {
            set_attr(c, "value", value);
            return;
        }
    }
    // var not present: add it
    Element v; v.name = "var";
    v.attrs.push_back(std::make_pair("name", name));
    v.attrs.push_back(std::make_pair("value", value));
    s->children.push_back(v);
}

const std::vector<Element> *bindings(const Model &m) {
    for (auto &c : m.root.children)
        if (c.name == "bindings") return &c.children;
    return NULL;
}

Element *bindings_elem(Model &m);   // defined below

bool apply_bindings_file(Model &m, const std::string &filename) {
    Element file;
    if (!parse_file_root(filename, file)) return false;   // <bindings> root
    Element *b = bindings_elem(m);
    if (!b) return false;
    // Replace the existing <bind> elements with those from the file.
    std::vector<Element> new_children;
    for (auto &c : file.children)
        if (c.name == "bind") new_children.push_back(c);
    // keep the non-bind children (e.g. existing <axis>), drop old binds
    for (auto &c : b->children)
        if (c.name != "bind") new_children.push_back(c);
    b->children = new_children;
    return true;
}

Element &add_bind(Model &m, const std::vector<std::pair<std::string,std::string>> &attrs) {
    Element *b = bindings_elem(m);
    Element e; e.name = "bind"; e.attrs = attrs;
    b->children.push_back(e);
    return b->children.back();
}

void remove_bind(Model &m, size_t index) {
    Element *b = bindings_elem(m);
    if (!b) return;
    size_t n = 0;
    for (size_t i = 0; i < b->children.size(); i++) {
        if (b->children[i].name != "bind") continue;
        if (n == index) { b->children.erase(b->children.begin() + i); return; }
        n++;
    }
}

// ---------------------------------------------------------------------------
// Axis elements under <bindings> (joystick flight roles)
// ---------------------------------------------------------------------------

Element *bindings_elem(Model &m) {
    for (auto &c : m.root.children)
        if (c.name == "bindings") return &c;
    Element b; b.name = "bindings";
    m.root.children.push_back(b);
    return &m.root.children.back();
}

const Element *find_axis(const Model &m, const std::string &name) {
    const std::vector<Element> *b = bindings(m);
    if (!b) return NULL;
    for (auto &c : *b)
        if (c.name == "axis" && attr(c, "name") == name) return &c;
    return NULL;
}

Element *find_axis(Model &m, const std::string &name) {
    Element *b = bindings_elem(m);
    if (!b) return NULL;
    for (auto &c : b->children)
        if (c.name == "axis" && attr(c, "name") == name) return &c;
    return NULL;
}

void set_axis(Model &m, const std::string &name, int joystick, int axis, bool inverse) {
    Element *b = bindings_elem(m);
    if (!b) return;
    Element *ax = find_axis(m, name);
    if (!ax) {
        Element e; e.name = "axis";
        e.attrs.push_back(std::make_pair("name", name));
        b->children.push_back(e);
        ax = &b->children.back();
    }
    set_attr(*ax, "joystick", std::to_string(joystick));
    set_attr(*ax, "axis", std::to_string(axis));
    set_attr(*ax, "inverse", inverse ? "true" : "false");
    remove_attr(*ax, "mouse");   // this is a joystick axis; drop any stale mouse attr
}

void remove_axis(Model &m, const std::string &name) {
    Element *b = bindings_elem(m);
    if (!b) return;
    for (size_t i = 0; i < b->children.size(); i++)
        if (b->children[i].name == "axis" && attr(b->children[i], "name") == name) {
            b->children.erase(b->children.begin() + i);
            return;
        }
}

// ---------------------------------------------------------------------------
// Presets (setup/presets.xml) — the modern UI's preset structure.
// ---------------------------------------------------------------------------

// Split on whitespace into tokens (like strtok but on std::string).
static void tokenize(const std::string &s, std::vector<std::string> &out) {
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && isspace((unsigned char)s[i])) i++;
        size_t start = i;
        while (i < s.size() && !isspace((unsigned char)s[i])) i++;
        if (i > start) out.push_back(s.substr(start, i - start));
    }
}

// Parse one var setting out of a line like:
//   <var name="X" value="Y"/>
static bool parse_var_line(const std::string &line, std::string &name, std::string &value) {
    size_t p = line.find("name=\"");
    size_t q = line.find("value=\"");
    if (p == std::string::npos || q == std::string::npos) return false;
    size_t p2 = line.find('\"', p + 6);
    size_t q2 = line.find('\"', q + 7);
    if (p2 == std::string::npos || q2 == std::string::npos) return false;
    name = line.substr(p + 6, p2 - (p + 6));
    value = line.substr(q + 7, q2 - (q + 7));
    return true;
}

// Parse a non-var element line like <axis name="x" mouse="0" axis="0" inverse="false"/>
// into an Element (name + ordered attrs). Returns false if it isn't one.
static bool parse_element_line(const std::string &line, Element &out) {
    size_t lt = line.find('<');
    size_t gt = line.find('>');
    if (lt == std::string::npos || gt == std::string::npos || gt <= lt + 1) return false;
    std::string tag = line.substr(lt + 1, gt - lt - 1);
    size_t sp = tag.find_first_of(" \t/");
    out.name = (sp == std::string::npos) ? tag : tag.substr(0, sp);
    // only axis/bind/hatswitch are meaningful non-var elements in presets
    if (out.name != "axis" && out.name != "bind" && out.name != "hatswitch") return false;
    std::string rest = (sp == std::string::npos) ? "" : tag.substr(sp);
    size_t i = 0;
    while (i < rest.size()) {
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t' || rest[i] == '/')) i++;
        if (i >= rest.size()) break;
        size_t an = i;
        while (i < rest.size() && rest[i] != '=' && rest[i] != ' ' && rest[i] != '\t') i++;
        std::string aname = rest.substr(an, i - an);
        if (i < rest.size() && rest[i] == '=') {
            i++;
            while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) i++;
            if (i < rest.size() && rest[i] == '\"') {
                i++;
                size_t vs = i;
                while (i < rest.size() && rest[i] != '\"') i++;
                out.attrs.push_back(std::make_pair(aname, rest.substr(vs, i - vs)));
                if (i < rest.size()) i++;
            }
        }
    }
    return true;
}


bool parse_presets(const std::string &filename, std::vector<PresetGroup> &out) {
    out.clear();
    std::string s = read_all(filename.c_str());
    if (s.empty()) return false;

    std::vector<std::string> lines;
    {
        std::string cur;
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '\n') { lines.push_back(cur); cur.clear(); }
            else cur += s[i];
        }
        if (!cur.empty()) lines.push_back(cur);
    }

    // strip surrounding whitespace and <!-- --> comment markers from a line
    auto strip = [](std::string t) -> std::string {
        size_t a = t.find_first_not_of(" \t\r");
        if (a == std::string::npos) return "";
        t = t.substr(a);
        size_t b = t.find("<!--");
        if (b != std::string::npos) t.erase(b, 4);
        b = t.find("-->");
        if (b != std::string::npos) t.erase(b);
        a = t.find_first_not_of(" \t\r");
        if (a == std::string::npos) return "";
        t = t.substr(a);
        size_t e = t.find_last_not_of(" \t\r");
        return e == std::string::npos ? "" : t.substr(0, e + 1);
    };

    // options listed in the current preset block header (may be several)
    std::vector<PresetOption*> active;
    for (auto &raw : lines) {
        std::string t = strip(raw);
        if (t.empty()) continue;

        if (t == "</vegaconfig>") { active.clear(); continue; }   // skip base config
        if (t.compare(0, 7, "#groups") == 0) continue;
        if (t.compare(0, 5, "#set ") == 0) {
            // record the default/active option for the group
            std::vector<std::string> toks; tokenize(t.substr(5), toks);
            if (toks.size() >= 2)
                for (auto &g : out)
                    if (g.name == toks[0]) { g.current = toks[1]; break; }
            continue;
        }
        if (t.compare(0, 6, "#endhe") == 0) { active.clear(); continue; }
        if (t.compare(0, 5, "#end") == 0) { active.clear(); continue; }

        if (t.compare(0, 5, "#cat ") == 0) {
            std::vector<std::string> toks; tokenize(t.substr(5), toks);
            if (toks.empty()) continue;
            PresetGroup g; g.name = toks[0];
            for (size_t i = 1; i < toks.size(); i++) { PresetOption o; o.name = toks[i]; g.options.push_back(o); }
            out.push_back(g);
        }
        else if (t.compare(0, 5, "#desc") == 0) {
            std::vector<std::string> toks; tokenize(t.substr(5), toks);
            if (toks.empty()) continue;
            // description = the option name + the remaining words joined
            std::string desc;
            for (size_t i = 1; i < toks.size(); i++) { if (i > 1) desc += " "; desc += toks[i]; }
            for (auto &g : out)
                for (auto &o : g.options)
                    if (o.name == toks[0]) { if (!desc.empty()) o.desc = desc; break; }
        }
        else if (t[0] == '#' ) {
            // preset block header: #opt1 opt2 ...  (multiple options share the block's vars)
            std::vector<std::string> toks; tokenize(t.substr(1), toks);
            active.clear();
            for (auto &name : toks) {
                for (auto &g : out)
                    for (auto &o : g.options)
                        if (o.name == name) { active.push_back(&o); break; }
            }
        }
        else if (!active.empty()) {
            // a var or element setting inside a preset block
            std::string name, value;
            if (parse_var_line(t, name, value)) {
                for (auto *o : active) o->vars.push_back(std::make_pair(name, value));
            } else {
                Element el;
                if (parse_element_line(t, el))
                    for (auto *o : active) o->elements.push_back(el);
            }
        }
    }
    return true;
}

// True if a preset var belongs in the joystick section (the mouse/joystick vars).
static bool is_joystick_var(const std::string &name) {
    static const char *joy_vars[] = {
        "mouse_cursor", "mouse_cursor_pancam", "mouse_cursor_pantgt", "mouse_cursor_chasecam",
        "warp_mouse", "warp_mouse_zone", "mouse_sensitivity", "mouse_exponent",
        "reverse_mouse_spr", "deadband", "mouse_deadband", "force_use_of_joystick",
        "force_feedback", "ff_device", "joystick_exponent", "polling_rate",
        "debug_digital_hatswitch", "mouse_blur", "clamp_axes", NULL
    };
    for (int i = 0; joy_vars[i]; i++) if (name == joy_vars[i]) return true;
    return false;
}

void apply_preset(PresetGroup &g, const std::string &option, Model &m) {
    for (auto &o : g.options) {
        if (o.name != option) continue;
        // vars: put each var into its real section. Prefer the section in the model that
        // already holds an active var with that name; else joystick for mouse/joystick vars,
        // else graphics (the default home for most preset vars like per_pixel_lighting).
        for (auto &kv : o.vars) {
            const Element *v = variables_elem(m);
            std::string section;
            if (v) {
                for (auto &sec : v->children) {
                    if (sec.name != "section") continue;
                    for (auto &vv : sec.children)
                        if (vv.name == "var" && attr(vv, "name") == kv.first) { section = attr(sec, "name"); break; }
                    if (!section.empty()) break;
                }
            }
            if (section.empty())
                section = is_joystick_var(kv.first) ? "joystick" : "graphics";
            set_var(m, section, kv.first, kv.second);
        }
        // non-var elements (axis/bind): apply to the <bindings> block
        Element *b = NULL;
        for (auto &c : m.root.children)
            if (c.name == "bindings") { b = &c; break; }
        if (b) {
            for (auto &el : o.elements) {
                Element *found = NULL;
                for (auto &c : b->children)
                    if (c.name == el.name && attr(c, "name") == attr(el, "name")) { found = &c; break; }
                if (found) {
                    for (auto &a : el.attrs) set_attr(*found, a.first, a.second);
                } else {
                    b->children.push_back(el);
                    found = &b->children.back();
                    for (auto &a : el.attrs) set_attr(*found, a.first, a.second);
                }
                // A preset axis declares one device; drop the other so they don't
                // conflict on x/y (e.g. a mouse-mode axis must not keep a joystick attr).
                if (attr(el, "mouse") != "") { remove_attr(*found, "joystick"); }
                else if (attr(el, "joystick") != "") { remove_attr(*found, "mouse"); }
            }
        }
        break;
    }
}

} // namespace vs05cfg
