import pygame
from pygame import Surface
import chatgpt


def main():
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


if __name__ == "__main__":
    main()
