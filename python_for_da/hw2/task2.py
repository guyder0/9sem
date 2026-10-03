from io import TextIOWrapper
import typer
import json
import re
import warnings

LEVELS = r"DEBUG|INFO|WARN|WARNING|ERROR|CRITICAL"

TIMESTAMP = (
    # поглощают timestamp 0000/00/00 0000-00-00 00/00/0000 00-00-0000 + 00:00:00
    # плюс разделителем между ними T либо пробел
    r"\d{4}[-/]\d{2}[-/]\d{2}(?:[T ]\d{2}:\d{2}:\d{2}(?:Z|[+-]\d{2}:?\d{2})?)?"
    r"|\d{2}-\d{2}-\d{4} \d{2}:\d{2}:\d{2}"
    # YYYYMMDDTHHMMSSZ
    r"|\d{8}T\d{6}Z"
)

KV_PAIR = (
    r'[\w\.\-]+='
    r'(?:'
        r'"(?:[^"\\]|\\.)*"|' # " Двойные кавычки
        r'`(?:[^`\\]|\\.)*`|' # ` Одинарные кавычки, тогда их надо экранировать
        r"'(?:[^'\\]|\\.)*'|" # ' Другие одинарные кавычки, аналогично
        r'[^\s"`]+'               # Без кавычек
    r')'
)

RE_KV = re.compile(rf"^(?P<timestamp>{TIMESTAMP})\s+(?P<level>{LEVELS})\s+(?P<body>(?:{KV_PAIR}\s*)+)$")

RE_KV_PATTERN = re.compile(
    r'(?P<key>[\w\.\-]+)='
    r'(?:'
        r'"(?P<val_vq>(?:[^"\\]|\\.)*)"|' # Значение в двойных кавычках
        r'(?P<val_wq>[^\s"]+)' # Значение без кавычек
    r')'
)

# Думал еще добавить другие логи, но в предложенных тестах они какие-то мусорные
#RE_DELIMITER_PIPE = re.compile(rf"^[^|\r\n]+\|({LEVELS})\|")
#RE_DELIMITER_COMMA = re.compile(rf"^[^,\r\n]+,({LEVELS}),")
#RE_DELIMITER_SEMICOLON = re.compile(rf"^[^;\r\n]+;({LEVELS});")

#RE_BRACKETED = re.compile(rf"^\[(?P<timestamp>[^\]]+)\]\s+\[{LEVELS}\]\s+(?P<body>.*)$")
#RE_ANGLE_BRACKETED = re.compile(rf"^<(?P<timestamp>[^>]+)>\s+<{LEVELS}>\s+<(?P<service>[^>]+)>\s*(?P<body>.*)$")

def parse(fs: TextIOWrapper, fe: TextIOWrapper):
    constructed = []
    parsers = [parse_kv]
    while True:
        log = fs.readline()
        if not log:
            break
        if len(log.strip()) == 0:
            continue

        parsed_log = None
        for parser in parsers:
            parsed_log = parser(log)
            if parsed_log:
                break # пока не получится распарсить

        if parsed_log: # удалость распарсить
            constructed.append(parsed_log)
        else: # не удалось распарсить
            fe.write(log + "\n")
    return constructed

def parse_kv(log):
    match = RE_KV.search(log)
    if not match:
        return
    constructed = {}
    match = match.groupdict()
    constructed["__timestamp__"] = match["timestamp"]
    constructed["__log_level__"] = match["level"]
    body = match["body"]
    for match in RE_KV_PATTERN.finditer(body):
        kv = match.groupdict()
        if kv["val_vq"] is not None:
            try:
                with warnings.catch_warnings():
                    warnings.filterwarnings("error", category=DeprecationWarning)
                    constructed[kv["key"]] = kv["val_vq"].encode('utf-8').decode('unicode_escape')
            except DeprecationWarning:
                return None # если проблемы с деэкранированием
        else:
            constructed[kv["key"]] = kv["val_wq"]
    return constructed

# См. комментарий на 38 строке
def parse_delimeter(log):
    match = None
    delimiter = ""
    _match = RE_DELIMITER_PIPE.search(log)
    if _match:
        delimiter = "|"
        match = _match
    _match = RE_DELIMITER_COMMA.search(log)
    if _match:
        delimiter = ","
        match = _match
    _match = RE_DELIMITER_SEMICOLON.search(log)
    if _match:
        delimiter = ";"
        match = _match
    if not match:
        return
    constructed = {}
    return None

app = typer.Typer(
    add_completion=False, # не добавлять флаги для автодополнения в bash/zsh
)

@app.command()
def main(src="data/log_example.txt", target="data/parsed.json", err="data/invalid.txt"):
    with open(src, "r") as fs, open(err, "w") as fe:
        parsed_logs = parse(fs, fe)
    with open(target, "w") as f:
        json.dump(parsed_logs, f, indent=4)

if __name__ == "__main__":
    app()
