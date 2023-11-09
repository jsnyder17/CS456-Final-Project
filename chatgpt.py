from typing import TextIO
import openai
from openai import OpenAI


STR_MODEL: str = "gpt-3.5-turbo"
STR_HEADER_ROLE: str = "role"
STR_HEADER_CONTENT: str = "content"
STR_ROLE_SYSTEM: str = "system"
STR_ROLE_USER: str = "user"
STR_API_KEY_FILE: str = "./resources/api_key.txt"
STR_PROMPT_FORMAT: str = "{character_desc}. Provide the JSON to:{conversation}"


client = OpenAI(api_key=open(STR_API_KEY_FILE, "r").read())
messages: list[dict[str, str]] = [{STR_HEADER_ROLE: STR_ROLE_SYSTEM, STR_HEADER_CONTENT: "Please generate my scripts."}]


def call_api(character_desc: str, conversation: str) -> str:
    """
    Sends an API request to ChatGPT
    :param character_desc:
    :param conversation:
    :return JSON of the generated script:
    """
    prompt: str = STR_PROMPT_FORMAT.format(character_desc=character_desc, conversation=conversation)

    messages.append({STR_HEADER_ROLE: STR_ROLE_USER, STR_HEADER_CONTENT: prompt})
    chat = client.chat.completions.create(model=STR_MODEL, messages=messages)

    return chat.choices[0].message.content
