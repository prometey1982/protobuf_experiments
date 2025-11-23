import asyncio
import handlers
import logging
import logging.config
import sys
from config import Config
from data_providers import DBProvider
from db import DB
from pathlib import Path
from notification import NotificationCallbackTgBot


def setup_logging(config):
    log_dir = Path(config["log_dir"])
    log_dir.mkdir(exist_ok=True)

    logging.config.dictConfig(config)

async def background_task(logger):
    while True:
        logger.info("Background task")
        await asyncio.sleep(5)


async def main_coro(config, ws_handler, notification_callback, logger):
    logger.info("hello from main")

    try:
        await asyncio.gather(notification_callback.run(), ws_handler.run(), return_exceptions=True)
    except asyncio.CancelledError:
        logger.info("Tasks cancelled")
    except Exception as e:
        logger.error(f"Error in main_coro: {e}")
    finally:
        await ws_handler.stop()
        await notification_callback.stop()
        await asyncio.sleep(0)


def main():
    config = Config("srv_conf.yaml")
    db = DBProvider(DB(config))
    setup_logging(config.to_dict("logging"))
    logger = logging.getLogger('my_app')
    notification_callback = NotificationCallbackTgBot(config)
    ws_handler = handlers.WSHandler(config, db, notification_callback)
    asyncio.run(main_coro(config, ws_handler, notification_callback, logger))


if __name__ == "__main__":
    main()
