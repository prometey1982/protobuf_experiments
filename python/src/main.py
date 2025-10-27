import asyncio
import yaml
import handlers
from config import Config
from db import DB

def main():
    print("hello from main")
    config = Config("srv_conf.yaml")
    db = DB(config)
    ws_handler = handlers.WSHandler(config, db)
    asyncio.run(ws_handler.run())


if __name__ == "__main__":
    main()
