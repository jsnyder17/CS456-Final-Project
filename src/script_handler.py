import logging
import textwrap
from logging import Logger

import pygame
from pygame import Surface, time, Rect
from pygame.font import Font
from pygame.threads import Thread

from json_handler import JSONHandler
from chatgpt import ChatGPTHandler
from images import Images


class ScriptHandler:
    logger: Logger
    images_list: list[Surface] = []

    chatgpt_handler: ChatGPTHandler
    json_handler: JSONHandler

    white: tuple[int, int, int] = (255, 255, 255)
    black: tuple[int, int, int] = (0, 0, 0)

    character_desc: str
    prompt: str

    x_dim: int
    y_dim: int

    screen: Surface = None

    curr_script: list[dict[str, str]] = []

    curr_image: Surface = None

    font: Font = None

    text: Surface = None
    text_rect: Rect = None

    text_rects: list[tuple[Surface, Rect, int]] = []

    timer: int

    def __init__(self, prompt: str, x_dim: int, y_dim: int, api_key_file_path: str = "./resources/api_key.txt"):
        self.logger = logging.getLogger("ScriptHandler")
        self.prompt = prompt
        self.x_dim = x_dim
        self.y_dim = y_dim
        self.timer = 0

        self.rect = Rect(150, 500, 500, 500)

        self.chatgpt_handler = ChatGPTHandler(api_key_file_path)
        self.json_handler = JSONHandler()

    def initialize_script_loop(self) -> None:
        self.logger.info("Initializing script loop.")
        print("Initializing script loop.")

        pygame.init()

        self.screen = pygame.display.set_mode((self.x_dim, self.y_dim))
        pygame.display.set_caption('AI Sitcom')

        self.load_character_descs()
        self.generate_script()
        self.load_images()

        self.font = Font('freesansbold.ttf', 18)

    def load_images(self) -> None:
        self.logger.info("Loading images.")
        print("Loading images.")

        self.images_list.append(pygame.image.load("./resources/babcock.png").convert())
        self.images_list.append(pygame.image.load("./resources/hake.jpeg").convert())
        self.images_list.append(pygame.image.load("./resources/moscola.jpeg").convert())
        self.images_list.append(pygame.image.load("./resources/zeller.png").convert())

        logging.log(logging.INFO, "Loaded images.")

    def start_script_loop(self) -> None:
        self.logger.info("Starting script loop.")
        print("Starting script loop.")

        logging.log(logging.INFO, "Playing script.")

        script_thread: Thread = Thread(target=self.run_script, name="script thread")
        script_thread.start()

        running: bool = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

        pygame.quit()

    def run_script(self) -> None:
        print("Playing script.")
        print(self.prompt)
        for d in self.curr_script:
            if d.get("speaker") == "Dr. Babcock":
                self.curr_image = self.images_list[Images.BABCOCK.value]

            elif d.get("speaker") == "Prof. Hake":
                self.curr_image = self.images_list[Images.HAKE.value]

            elif d.get("speaker") == "Dr. Moscola":
                self.curr_image = self.images_list[Images.MOSCOLA.value]

            elif d.get("speaker") == "Prof. Zeller":
                self.curr_image = self.images_list[Images.ZELLER.value]

            # Display image of speaker
            self.screen.blit(self.curr_image, (325, 300))

            # Display speaker's line
            self.draw_text(d.get("speech"), self.white, self.black, True)

            pygame.display.flip()

            # Transition through each line every 4 seconds
            start_ticks: int = time.get_ticks()
            while self.timer < 5000:
                self.timer = time.get_ticks() - start_ticks

            # Prepare for the next speaker
            self.clear_text_area()
            self.timer = 0

        pygame.event.post(pygame.event.Event(pygame.QUIT))

    def load_character_descs(self) -> None:
        print("Loading character descriptions.")

        desc_str: str = ""

        file_data: str = open("./resources/character_desc.json", "r").read()
        json_data: list[dict[str, str]] = self.json_handler.parse_json(file_data, "characters")

        for desc in json_data:
            desc_str += "{name} is {desc}".format(name=desc.get("name"), desc=desc.get("description"))

        self.character_desc = desc_str

    def generate_script(self) -> None:
        result: str = self.chatgpt_handler.call_api(self.character_desc, self.prompt)
        print(result)

        self.curr_script = self.json_handler.parse_json(result, "conversation")

        # file_data: str = open("./resources/example_output.json").read()
        # self.curr_script = self.json_handler.parse_json(file_data, "conversation")

    def clear_text_area(self) -> None:
        for t in self.text_rects:
            t[0].fill(self.black, t[0].get_rect())
            self.screen.blit(t[0], (t[1].left, t[2]))

        self.text_rects.clear()

    def draw_text(self, text: str, color: tuple[int, int, int], bkg: tuple[int, int, int], aa = False) \
            -> Surface:
        rect: Rect = Rect(self.rect)
        y: int = rect.top
        line_spacing: int = -2

        # Get the height of the font
        font_height: int = self.font.size("Tg")[1]

        while text:
            i: int = 1

            # Determine if the row of text will be outside our area
            if y + font_height > rect.bottom:
                break

            # Determine maximum width of line
            while self.font.size(text[:i])[0] < rect.width and i < len(text):
                i += 1

            # If we've wrapped the text, then adjust the wrap to the last word
            if i < len(text):
                i = text.rfind(" ", 0, i) + 1

            # Render the line and blit it to the surface
            text_render: Surface = self.font.render(text[:i], aa, color, bkg)
            self.screen.blit(text_render, (rect.left, y))
            self.text_rects.append((text_render, rect, y))

            y += font_height + line_spacing

            # Remove the text we just blitted
            text = text[i:]

        return text
