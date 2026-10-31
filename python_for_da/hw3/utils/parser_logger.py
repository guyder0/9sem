import logging
import sys
from pathlib import Path

if not Path("log/").exists():
    Path("log/").mkdir()

class LoggerFilter(logging.Filter):
    processed = 0
    all = 0

    def filter(self, record):
        record.processed = self.processed
        record.all = self.all
        return True

filter = LoggerFilter()

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

formatter = logging.Formatter('[%(asctime)s] [%(processed)d/%(all)d] [%(levelname)s] - %(message)s')

file_handler = logging.FileHandler('log/parser.log', encoding='utf-8')
file_handler.setFormatter(formatter)
file_handler.addFilter(filter)

stream_handler = logging.StreamHandler(sys.stdout)
stream_handler.setFormatter(formatter)
stream_handler.addFilter(filter)

logger.addHandler(file_handler)
logger.addHandler(stream_handler)
