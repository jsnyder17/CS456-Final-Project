import json


def parse_json(raw_data: str) -> list[dict[str, str]]:
    return json.loads(raw_data)["conversation"]