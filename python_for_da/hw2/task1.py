import json
import typer
from io import TextIOWrapper

def parse_object(f: TextIOWrapper):
    constructed = {}
    while True:
        line = f.readline()
        if not line or "}" in line:
            break
        line = line.strip(" \t\n")
        if len(line) == 0: continue
        line = line if line[-1] != "," else line[:-1]

        kv = line.split(":")
        key, value = kv[0], "".join(kv[1:]).strip()
        if len(value) == 0: # обнаружен перенос value на след.строку
            value = f.readline().strip(" \t\n")
        try:
            constructed[key] = value_correctors[key](f, value)
        except Exception as e:
            # отладка поиска ошибок, тут я обнаружил "Region" как перенесенное на отдельную строку value
            print(f"line='{line}'")
            raise e

    return constructed

def parse_list(f: TextIOWrapper, value = None):
    constructed = []
    while True:
        line = f.readline()
        if not line or "]" in line:
            break
        line = line.strip(" \t\n")
        if len(line) == 0: continue
        line = line if line[-1] != "," else line[:-1]

        if line == "{":
            constructed.append(parse_object(f))
        else:
            # всё остальное в списках - строки (других списков глазами обнаружено не было)
            constructed.append(line)

    return constructed

def correct_float(f, value: str):
    # внесены все замеченные в процессе ошибок конвертации str->float ошибки
    value = value.replace(",", ".")
    value = value.replace("\t", "")
    return float(value)

"""
    корректоры - функции от f, x;
    f - файл, из которого читаем; требуется для корректной работы parse_list()
        другие функции не используют f
    x - само значение value для корректировки: True -> true, 26,4 -> 26.4 итд

    lambda f, x: x - identity преобразование; если преполагаем, что value=string
"""
value_correctors = {
    "type": lambda f, x: x,
    "id": lambda f, x: x,
    "objects": parse_list,
    "spec_version": lambda f, x: x,
    "created": lambda f, x: x,
    "modified": lambda f, x: x,
    "name": lambda f, x: x,
    "description": lambda f, x: x,
    "identity_class": lambda f, x: x,
    "revoked": lambda f, x: "true" in x.lower(),
    "confidence": lambda f, x: int(x),
    "object_marking_refs": parse_list,
    "x_opencti_organization_type": lambda f, x: x,
    "created_by_ref": lambda f, x: x,
    "latitude": correct_float,
    "longitude": correct_float,
    "region": lambda f, x: x,
    "x_opencti_location_type": lambda f, x: x,
    "relationship_type": lambda f, x: x,
    "source_ref": lambda f, x: x,
    "target_ref": lambda f, x: x,
    "country": lambda f, x: x,
    "x_opencti_aliases": parse_list,
}

app = typer.Typer(
    add_completion=False, # не добавлять флаги для автодополнения в bash/zsh
)

@app.command()
def main(src="data/geography.txt", target="data/parsed.txt"):
    with open(src, "r") as f:
        f.readline()
        parsed_json = parse_object(f)
    with open(target, "w") as f:
        json.dump(parsed_json, f, indent=4)

if __name__ == "__main__":
    app()
