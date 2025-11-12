import asyncio
import handlers
import logging
import sys
from config import Config
from db import DB


async def main_coro(config, db, logger):
    logger.info("hello from main")
    ws_handler = handlers.WSHandler(config, db)
    main_task = asyncio.ensure_future(ws_handler.run())

    try:
        await main_task
    except KeyboardInterrupt:
        main_task.cancel()
        await asyncio.sleep(0)


def main():
    config = Config("srv_conf.yaml")
    db = DB(config)
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        stream=sys.stdout
    )
    logger = logging.getLogger('my_app')
    asyncio.run(main_coro(config, db, logger))


if __name__ == "__main__":
    main()
