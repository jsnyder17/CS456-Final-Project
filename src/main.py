import logging

from script_handler import ScriptHandler


def input_prompt() -> str:
    return input("Enter the prompt for your script\n'exit' to exit\n-> ")


def input_api_key_path() -> str:
    return input("Enter the path to your api key file 'api_key.txt'\nDefault is './resources/api_key.txt' -> ")


def main() -> None:
    try:
        prompt: str = input_prompt()
        if prompt == "exit":
            raise Exception("User exit")

        api_key_path: str = input_api_key_path()
        if api_key_path == "exit":
            raise Exception("User exit")

        script_handler: ScriptHandler = ScriptHandler(prompt, api_key_path, 800, 800)
        script_handler.initialize_script_loop()
        script_handler.start_script_loop()

    except Exception as e:
        logging.log(logging.ERROR, e)


if __name__ == "__main__":
    main()
