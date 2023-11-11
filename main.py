from enum import Enum
from typing import TextIO

import pygame
from pygame import Surface, Rect, time
from pygame.font import Font

import chatgpt
import json_handler
import logging


images_list: list[Surface] = []

x_dim: int = 800
y_dim: int = 600

white: tuple[int, int, int] = (255, 255, 255)
black: tuple[int, int, int] = (0, 0, 0)


class Images(Enum):
    BABCOCK = 0
    HAKE = 1
    MOSCOLA = 2
    ZELLER = 3


def load_images() -> None:
    images_list.append(pygame.image.load("./resources/babcock.png").convert())
    images_list.append(pygame.image.load("./resources/hake.jpeg").convert())
    images_list.append(pygame.image.load("./resources/moscola.jpeg").convert())
    images_list.append(pygame.image.load("./resources/zeller.png").convert())

    logging.log(logging.INFO, "Loaded images.")


def test_api_calls() -> None:
    character_desc: str = (
        "Dr. Moscola is a professor who hates the Windows operating system. Dr. Babcock is a professor"
        "who loves his Mac.")
    conversation: str = "A conversation between Dr. Moscola and Dr. Babcock about AI generating stories"

    result: str = chatgpt.call_api(character_desc, conversation)

    print(result)


def test_json_parsing() -> list[dict[str, str]]:
    file: TextIO = open('./resources/example_output.json', 'r')
    file_data: str = file.read()

    return json_handler.parse_json(file_data)


def pygame_stuff():
    pygame.init()

    screen: Surface = pygame.display.set_mode((x_dim, y_dim))
    pygame.display.set_caption('AI Sitcom')

    load_images()

    image: Surface

    script: list[dict[str, str]]

    font: Font = pygame.font.Font('freesansbold.ttf', 12)
    text: Surface
    text_rect: Rect

    status: bool = True
    while status:
        logging.log(logging.INFO, "Playing script.")
        script = test_json_parsing()

        for d in script:
            if d.get("speaker") == "Dr. Babcock":
                image = images_list[Images.BABCOCK.value]

            elif d.get("speaker") == "Prof. Hake":
                image = images_list[Images.HAKE.value]

            elif d.get("speaker") == "Dr. Moscola":
                image = images_list[Images.MOSCOLA.value]

            elif d.get("speaker") == "Dr. Zeller":
                image = images_list[Images.ZELLER.value]

            screen.blit(image, (x_dim / 2, 0))

            text = font.render(d.get("speech"), True, white, black)
            text_rect = text.get_rect()
            text_rect.center = (x_dim // 2, y_dim // 2)
            screen.blit(text, text_rect)

            pygame.display.flip()

            time.delay(4000)

        status = False

    pygame.quit()


def main():
    try:
        pygame_stuff()

    except Exception as e:
        logging.log(logging.ERROR, e)


if __name__ == "__main__":
    main()
