from multiprocessing import process

from utils.parser_logger import logger, filter

import typer
import re
import time
import json
import asyncio
import aiofiles

from bs4 import BeautifulSoup
from pathlib import Path


re_table = [
    ("Год", re.compile(r"[^\n]*(\d{4})[^\n]*")),
    ("Семестр", re.compile(r"[^\n]*(весна|осень|весенний|осенний)[^\n]*", re.IGNORECASE)),
    ("Руководил курсом", re.compile(r"[^\n]*руководитель()[^\n]*", re.IGNORECASE)),
    ("Кол-во дз", re.compile(r"[^\n]*(\d+) домашн[^\n]*", re.IGNORECASE)),
    ("Проект/курсовая", re.compile(r"[^\n]*\s(проект|курсовая)\s[^\n]*", re.IGNORECASE)),
    ("Зачет/экзамен", re.compile(r"[^\n]*(зачет|зачёт|экзамен)[^\n]*", re.IGNORECASE)),
    ("План курса", re.compile(r"[^\n]*(план курса)[^\n]*", re.IGNORECASE)),
]

parsed_list = []

def clear_page(page):
    soup = BeautifulSoup(page, "html.parser")
    title = soup.select_one("#firstHeading").get_text(strip=True)
    content = soup.select_one(".mw-parser-output")
    content = content.get_text(separator="\n", strip=True)
    return title, content

def parse_page(page):
    info = {}
    title, content = page
    def re_search(re):
        search = re.search(title)
        if search:
            return {
                "строка": search.group(0),
                "найдено": search.group(1),
            }
        else:
            search = re.search(content)
            return {
                "строка": search.group(0),
                "найдено": search.group(1),
            } if search else None

    info["Название предмета"] = title

    for key, re in re_table:
        info[key] = re_search(re)

    return info

async def process_page(path):
    filter.processed += 1
    async with aiofiles.open(path, "r") as f:
        logger.info(f"Чтение файла: {path}")
        page_text = await f.read()
        page_text = clear_page(page_text)
    page_info = parse_page(page_text)
    logger.info(f"Парсинг завершен: {path}")
    return page_info

async def async_main(data, output):
    if not output.parent.exists():
        output.parent.mkdir()

    pages = [page for page in data.iterdir() if page.glob(".html")]
    filter.all = len(pages)
    parsed_list = await asyncio.gather(*[process_page(page) for page in pages])
    with open(output, "w", encoding="utf-8") as f:
        json.dump(parsed_list, f, indent=4, ensure_ascii=False)

def main(data: Path = Path("data/"), output: Path = Path("output/parsed.json")):
    start_time = time.perf_counter()
    asyncio.run(async_main(data, output))
    logger.info(f"Затрачено времени: {time.perf_counter() - start_time:.4f} с.")

if __name__ == "__main__":
    typer.run(main)
