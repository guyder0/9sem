import typer
import json
from pathlib import Path

def main(data: Path = Path("output/parsed.json"), antitop: int = 3):
    with open(data, "r") as f:
        data = json.load(f)

    all = len(data)
    data[0]["Полей определено"] = 0
    for key in data[0].keys():
        if key in ["title,", "Полей определено"]: continue
        count = 0
        for d in data:
            d["Полей определено"] = d.get("Полей определено", 0)
            if d[key] is not None:
                d["Полей определено"] += 1
                count += 1
        print(f"{key}: {count}/{all}")

    sorted(data, key=lambda x: x["Полей определено"])

    print(f"\nАнтитоп {antitop}:\n")
    print("\n\n".join([json.dumps(d, indent=4, ensure_ascii=False) for d in data[:antitop]]))

if __name__ == "__main__":
    typer.run(main)
