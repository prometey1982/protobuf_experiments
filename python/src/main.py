import asyncio
from handlers.handlers import handler
from handlers.ws_handler import WSHandler

def main():
    print("hello from main")
    handler()
    ws_handler = WSHandler()
    asyncio.run(ws_handler.run())

if __name__ == "__main__":
    main()
