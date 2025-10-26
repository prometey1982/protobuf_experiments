import asyncio
import yaml
import handlers
from config import Config
#from handlers import handler
#from handlers import WSHandler

def main():
    print("hello from main")
    config = Config("srv_conf.yaml")
    ws_handler = handlers.WSHandler(config)
    asyncio.run(ws_handler.run())

if __name__ == "__main__":
    main()
