import os
import requests
import re
import json
from enum import Enum
from urllib.parse import urljoin

"""
This script is used to pull game data from the website's github. 
This is an easy way to populate the mix data but also ensures that the tool and website agree with each other.
The final result is a header file with C-style arrays and structs.
The script is really hacky and very fragile. It will probably need tweaks every time the data changes.
"""

# for some reason rank names not in the GitHub repo
rank_tiers  = ["Street Rat", "Hoodlum", "Peddler", "Hustler", "Bagman", "Enforcer", "Shot Caller", "Block Boss", "Underlord", "Baron", "Kingpin"]
rank_suffix = ["I", "II", "III", "IV", "V"]

root_url    = "https://raw.githubusercontent.com/Schedule1-Tools/mixer/refs/heads/main/src/data/"
root_dir    = "./cache"

ind         = '    '

# not sure where to find the "display order" that effects appear in-game. hard coded until I bother reverse-engineering the game
effect_order = ["Calming", "Euphoric", "Focused", "Munchies", "Paranoia", "Refreshing", "Smelly", "Calorie-Dense", "Disorienting", "Energizing", "Gingeritis", "Sedating", "Sneaky", "Toxic", "Athletic", "Balding", "Foggy", "Laxative", "Seizure-Inducing", "Slippery", "Spicy", "Bright-Eyed", "Glowing", "Jennerising", "Schizophrenia", "Thought-Provoking", "Tropic Thunder", "Anti-Gravity", "Cyclopean", "Electrifying", "Explosive", "Long Faced", "Shrinking", "Zombifying"]

def get_cached_web_text(relative_path):
    file_url = urljoin(root_url, relative_path)
    local_path = os.path.join(root_dir, "web", relative_path.replace("/", os.sep))

    if os.path.exists(local_path):
        with open(local_path, 'r', encoding='utf-8') as fh:
            text = fh.read()
            return text

    os.makedirs(os.path.dirname(local_path), exist_ok=True)

    try:
        print(f"Downloading: {file_url}")
        response = requests.get(file_url, timeout=10)
        response.raise_for_status()

        with open(local_path, 'w', encoding='utf-8') as fh:
            fh.write(response.text)
        return response.text

    except requests.RequestException as e:
        print(f"Failed to download {file_url}: {e}")
        return None

def get_cached_json(name):
    file_name = f"json/{name}.json"
    local_path = os.path.join(root_dir, file_name.replace("/", os.sep))

    if os.path.exists(local_path):
        with open(local_path, 'r', encoding='utf-8') as fh:
            return json.load(fh)
    
    web_text = get_cached_web_text(name + ".ts")
    d = to_json(web_text)
    dump_data(local_path, d)

    return d

def to_json(js_text):
    # truly cursed regex parser to turn ts exports into json
    # breaks on - almost everything. only takes the first export
    match = re.search(r"export [^\{}]*= *({.*});", js_text, re.DOTALL)

    js_data      = match.group(1)
    fixed_keys   = re.subn(r"(\w+)(?=\s*:)", r'"\1"', js_data, 0)[0]
    fixed_quotes = re.subn(r"'(.*?)'", r'"\1"', fixed_keys, 0)[0]
    fixed_commas = re.subn(r",(\s*[}\]])", r'\1', fixed_quotes, 0, re.DOTALL)[0]

    return json.loads(fixed_commas)

def dump_data(json_path, data):    
    os.makedirs(os.path.dirname(json_path), exist_ok=True)
    with open(json_path, 'w', encoding='utf-8') as fh:
        json.dump(data, fh, indent=4)

def process_data(input_data):
    # effects
    effects = []
    eff_lookup = {}
    sorted_effects = input_data["effects"].items()
    # sorted_effects = sorted(sorted_effects, key=lambda it: it[1]['price'], reverse=True)
    order_fn = lambda name: effect_order.index(name) if name in effect_order else 100
    sorted_effects = sorted(sorted_effects, key=lambda it: order_fn(it[1]['name']), reverse=False)
    
    for i, (k, js_eff) in enumerate(sorted_effects):
        eff = {
            "name":         js_eff["name"],
            "abbreviation": k,
            "mult":         float(js_eff["price"]),
            "addiction":    float(js_eff["addiction"]),
            "color":        js_eff["color"],
        }
        effects.append(eff)
        eff_lookup[k] = i

    # rules
    flat_rules = []
    for k, rs in input_data["rules"].items():
        for r in rs:
            get_eff = lambda key: eff_lookup[r[key][0]]
            tx = {
                "sub":   k, 
                "from":  get_eff("ifPresent"),
                "to":    get_eff("ifNotPresent")
            }
            flat_rules.append(tx)
    # rule spans    
    rule_ranges = {}
    current = ""
    n = 0
    start = 0
    for i, r in enumerate(flat_rules):
        sub = r["sub"]
        if sub != current:
            rule_ranges[current] = (start, n)
            start = i
            n = 1
            current = sub
        else:
            n += 1
    rule_ranges[current] = (start, n)
    rule_ranges.pop("")
    
    assert(sum([t[1] for t in rule_ranges.values()]) == len(flat_rules))
    
    # technically the first 4 ingredients are available at Street Rat I, but the mixer isn't until Hoodlum I
    fix_rank = lambda r_str: int(r_str) + 4
    
    # substances/ingredients
    ingredients = []
    for i, (k, sub) in enumerate(input_data["substances"].items()):
        ing = {
            "name":         k, 
            "index":        i,
            "abbreviation": sub["abbreviation"],
            "price":        sub["price"],
            "effect":       eff_lookup[sub["effect"][0]],
            "rank":         fix_rank(sub["rank"]),
            "span":         rule_ranges[k],
        }

        ingredients.append(ing)

    # base products
    base_products = []
    for (k, product) in input_data["products"].items():
        bp = {
            "name": k,
            "abbreviation": product["abbreviation"],
            "price":        product["price"],
            "base_effect":  eff_lookup[product["effects"][0]] if product["effects"] else -1,
            "rank":         fix_rank(product["rank"]),
        }
        base_products.append(bp)

    output_data = {
        "effects":          effects,
        "ingredients":      ingredients,
        "rules":            flat_rules,
        "base_products":    base_products,
    }
    return output_data

# hideous formatting code begins here:
class CType(Enum):
    STRING  = "const char*"
    FLOAT   = "double"
    INT     = "int"

def get_type(val):
    """determines base type of val"""
    match val:
        case str():
            return CType.STRING
        case float():
            return CType.FLOAT
        case int():
            return CType.INT
        case [*vals]:
            return get_type(vals[0])
        case _:
            raise Exception("Oh no!")
        
def type_def_formatter(val, suffix = ""):
    match val:
        case [*vals]:
            return type_def_formatter(vals[0], f"[{len(vals)}]")
        case _:
            ctype = get_type(val)
            return lambda name: f"{ctype.value:<11} {name}{suffix}"

def literal(val):
    ctype = get_type(val)
    match val:
        case [*vals]:
            return f"{{{', '.join(literal(v) for v in vals)}}}"
        case _:
            match ctype:
                case CType.STRING:
                    return f'"{val}"'
                case CType.INT:
                    return f'{val:2d}'
                case CType.FLOAT:
                    return f'{val:0.2f}'

def dict_to_struct_def(struct_name, d):
    lines = [f"struct {struct_name} {{"]
    for name, val in d.items():
        lines.append(f"    {type_def_formatter(val)(name)};")
    lines.append("};")
    return "\n".join(lines) + "\n"

def dict_to_stuct_vals(d: dict):
    vals = []
    for i, v in enumerate(d.values()):
        lv = literal(v)
        if (i == 0 and get_type(v) == CType.STRING):
            lv = f"{lv:<20}" # improves struct alignment for readability
        vals.append(lv)
        
    return f"{{{', '.join(vals)}}}"

def indent_text(text, ind = "    "):
    m =  re.subn(r"^(?!\s*$)", ind, text, flags=re.MULTILINE)
    return m[0]

def generate_header(processed):
    s_names = ["Effect", "Ingredient", "Base_Product"]
    d_names = ["effects", "ingredients", "base_products"]
    
    # define the structs
    struct_defs = [ dict_to_struct_def(s_name, processed[d_name][0]) for s_name, d_name in zip(s_names, d_names) ]
    struct_defs = "\n".join(struct_defs)
    
    # populate the struct arrays
    struct_values = []
    for s_name, d_name in zip(s_names, d_names):
        ds = processed[d_name]
        lines =  [f"inline constexpr {s_name} {d_name}[] = {{"]
        lines += [",\n".join([ ind + dict_to_stuct_vals(d) for d in ds])]
        lines += ["};\n"]
        struct_values.append("\n".join(lines))
    struct_values = "\n".join(struct_values)
    
    # list the rules with a new line for each ingredient
    rules = [ literal([d["from"], d["to"]]) for d in processed["rules"] ]
    rule_lines = []
    for ing in processed["ingredients"]: # 
        start, length = ing["span"]
        rule_lines.append(ind + f"/* {ing['name']:<12} */ " + ", ".join(rules[start:start+length]))
    rules = ',\n'.join(rule_lines)
    rules = f"inline constexpr int rules[][2] = {{\n{rules}\n}};\n"

    # also need the rank names for GUI display
    ranks  = []
    for t in rank_tiers[:-1]:
        ranks.append([ f'{lt:<20}' for lt in [ f'"{t} {suff}", ' for suff in rank_suffix ] ])
    ranks.append([f'"{rank_tiers[-1]}"'])
    ranks = '\n'.join([ind + (''.join(ln)).rstrip() for ln in ranks ])
    ranks = f"inline constexpr const char* rank_names[] = {{\n{ranks}\n}};\n"

    # put together the header
    header = "\n".join([struct_defs, struct_values, rules, ranks])
    header = indent_text(header)
    header = "#pragma once\n\nnamespace raw {\n" + header + "}"
    
    return header

if __name__ == "__main__":
    names = [
        "effects",
        "products",
        "substances",
        "rules",
    ]

    raw_data = {}
    for name in names:
        d = get_cached_json(name)
        raw_data[name] = d

    processed = process_data(raw_data)
    dump_data('./processed.json', processed)
    header = generate_header(processed)

    with open("test.hpp", "w") as fh:
        fh.write(header)