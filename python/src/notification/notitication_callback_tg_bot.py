import telegram

from .notitication_callback_base import NotificationCallbackBase
import python-telegram-bot

class NotificationCallbackTgBot(NotificationCallbackBase):
    def __init__(self, config):
        self._config = config

    async def on_request_received(self, **kwargs):
        pass
