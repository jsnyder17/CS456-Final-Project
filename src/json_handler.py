import json
import logging
from logging import Logger


class JSONHandler:
    logger: Logger

    def __init__(self):
        self.logger = logging.getLogger(self.__class__.__name__)

    def parse_json(self, raw_data: str, cls: str) -> list[dict[str, str]]:
        self.logger.info("Parsing data.")
        print("Parsing data for {cls}.".format(cls=cls))
        return json.loads(raw_data)[cls]