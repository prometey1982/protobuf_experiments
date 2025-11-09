import asyncio
import yaml
import handlers
import functools
from config import Config
from db import DB
import signal

def ask_exit(signame, loop):
    print("got signal %s: exit" % signame)
    loop.stop()

async def main_coro(config, db):
    print("hello from main")
    loop = asyncio.get_running_loop()
    ws_handler = handlers.WSHandler(config, db)
    main_task = asyncio.ensure_future(ws_handler.run())

    try:
        await main_task
    except KeyboardInterrupt:
        main_task.cancel()

        await asyncio.sleep(0)

        try:
            await task
        except CancelledError:
            pass

def main():
    config = Config("srv_conf.yaml")
    db = DB(config)
    asyncio.run(main_coro(config, db))

if __name__ == "__main__":
    main()
