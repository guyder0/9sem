from utils.download_logger import logger, filter

import aiohttp
import typer
import re
import asyncio
import time

from bs4 import BeautifulSoup
from hashlib import md5
from pathlib import Path
from urllib.parse import urljoin, unquote

MAIN_PAGE = "https://wiki.atp-fivt.org/"
RE_FILTER = re.compile("2025|2026")

async def download_page(session, url, dist, sem, timeout=3, save=True):
    uurl = unquote(url)
    url_hash = md5(url.encode()).hexdigest()
    dist = dist / f"{url_hash}.html"
    if dist.exists():
        return

    try:
        filter.started += 1
        logger.info(f"Начался запрос к {uurl}")
        async with sem, session.get(url) as response:
            response.raise_for_status()
            page = await response.text()
            filter.loaded += 1
            logger.info(f"Успешно загрузился {uurl}")
            if save:
                await asyncio.to_thread(dist.write_text, page, encoding='utf-8')
                logger.info(f"Сохранен на диск {uurl}")
            return page
    except aiohttp.ClientError as e:
        filter.loaded += 1
        logger.error(f"Не удалось скачать {uurl}; {e}")
    except TimeoutError:
        filter.loaded += 1
        logger.error(f"Не удалось скачать {uurl}; таймаут")


async def async_main(dist, latency, max_requests, timeout):
    if not dist.exists():
        dist.mkdir()

    sem = asyncio.Semaphore(max_requests)

    async with aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=timeout)) as session:
        page = await asyncio.create_task(download_page(session, MAIN_PAGE, dist, sem, save=False))
        if page is None: return # если почему-то главный сайт не скачался (но у меня вообще ide просто дальше жалуется)
        soup = BeautifulSoup(page, "html.parser")
        links = soup.find_all("a")
        urls = [
            urljoin(MAIN_PAGE, str(link['href']))
            for link in links
            if set(link.attrs.keys()) == { 'href', 'title' }
            and bool(RE_FILTER.search(str(link['title'])))
        ]

        tasks = []
        for url in urls:
            task = asyncio.create_task(download_page(session, url, dist, sem))
            tasks.append(task)
            await asyncio.sleep(latency)

        await asyncio.gather(*tasks)


def main(
    dist: Path = Path("data/"),
    latency: float = 0.05,
    max_requests: int = 10,
    timeout: int = 3,
):
    started = time.perf_counter()
    asyncio.run(async_main(dist, latency, max_requests, timeout))
    logger.info(f"Затрачено времени: {time.perf_counter() - started:.4f} с.")


if __name__ == "__main__":
    typer.run(main)
