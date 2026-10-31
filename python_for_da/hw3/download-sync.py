from utils.download_logger import logger, filter

import requests
import typer
import re
import time

from bs4 import BeautifulSoup
from hashlib import md5
from pathlib import Path
from urllib.parse import urljoin, unquote

MAIN_PAGE = "https://wiki.atp-fivt.org/"
RE_FILTER = re.compile("2025|2026")

def download_page(url, dist, latency, timeout=3, save=True):
    last_download = getattr(download_page, "last_download", 0)

    uurl = unquote(url)
    url_hash = md5(url.encode()).hexdigest()
    dist = dist / f"{url_hash}.html"
    if dist.exists():
        return

    try:
        time.sleep(max(0, latency - time.perf_counter() + last_download))
        filter.started += 1
        download_page.last_download = time.perf_counter()
        response = requests.get(url, allow_redirects=True)
        content = response.content.decode()
        filter.loaded += 1
        if save:
            with open (dist, "w", encoding="utf-8") as f:
                f.write(content)
            logger.info(f"Сохранен на диск {uurl}")
        return content
    except requests.RequestException as e:
        filter.loaded += 1
        logger.error(f"Не удалось скачать {uurl}; {e}")
    except TimeoutError:
        filter.loaded += 1
        logger.error(f"Не удалось скачать {uurl}; таймаут")


def main(
    dist: Path = Path("data/"),
    latency: float = 0.05,
    timeout: int = 3,
):
    started = time.perf_counter()
    if not dist.exists():
        dist.mkdir()

    page = download_page(MAIN_PAGE, dist, latency, timeout=timeout, save=False)
    if page is None: return
    soup = BeautifulSoup(page, "html.parser")
    links = soup.find_all("a")
    urls = [
        urljoin(MAIN_PAGE, str(link['href']))
        for link in links
        if set(link.attrs.keys()) == { 'href', 'title' }
        and bool(RE_FILTER.search(str(link['title'])))
    ]

    for url in urls:
        download_page(url, dist, latency, timeout=timeout)

    logger.info(f"Затрачено времени: {time.perf_counter() - started:.4f} с.")


if __name__ == "__main__":
    typer.run(main)
