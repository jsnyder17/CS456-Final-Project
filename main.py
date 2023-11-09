import pygame
from pygame import Surface
import chatgpt


def test_api_calls():
    character_desc: str = ("Dr. Moscola is a professor who hates the Windows operating system. Dr. Babcock is a professor"
                           "who loves his Mac.")
    conversation: str = "A conversation between Dr. Moscola and Dr. Babcock about AI generating stories"

    result: str = chatgpt.call_api(character_desc, conversation)

    print(result)


def pygame_stuff():
    pygame.init()

    x_dim: int = 600
    y_dim: int = 600
    screen: Surface = pygame.display.set_mode((x_dim, y_dim))
    pygame.display.set_caption('image')

    image: Surface = pygame.image.load("./resources/moscola.jpeg").convert()
    screen.blit(image, (0, 0))

    pygame.display.flip()

    status: bool = True
    while status:
        for i in pygame.event.get():
            if i.type == pygame.QUIT:
                status = False

    pygame.quit()


def main():
    test_api_calls()


if __name__ == "__main__":
    main()
