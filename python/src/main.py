import asyncio
import handlers
import logging
import logging.config
import sys
from config import Config
from db import DB
from pathlib import Path


def setup_logging(config):
    log_dir = Path(config["log_dir"])
    log_dir.mkdir(exist_ok=True)

    logging.config.dictConfig(config)


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
    setup_logging(config.to_dict("logging"))
    logger = logging.getLogger('my_app')
    asyncio.run(main_coro(config, db, logger))


if __name__ == "__main__":
    main()
