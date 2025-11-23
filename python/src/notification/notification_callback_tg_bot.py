import asyncio
import logging
from .notification_callback_base import NotificationCallbackBase
from config import Config
from telegram import Update
from telegram.ext import Application, MessageHandler, CommandHandler, filters


class NotificationCallbackTgBot(NotificationCallbackBase):
    def __init__(self, config: Config):
        self._config = config
        self._logger = logging.getLogger(__name__)
        self._application = Application.builder().token(self._config.bot.token).build()
        self.setup_handlers()

    def setup_handlers(self):
        self._application.add_handler(CommandHandler("id", self.get_chat_id))

    async def run(self):
        self._logger.info("Запуск Telegram бота...")
        await self._application.initialize()
        await self._application.start()
        await self._application.updater.start_polling()

    async def stop(self):
        await self._application.updater.stop()
        await self._application.stop()

    async def get_chat_id(self, update, context):
        self._logger.debug("received get_chat_id command")
        """Команда для получения chat_id"""
        chat_id = update.effective_chat.id
        user = update.effective_user

        await update.message.reply_text(
            f"📋 Ваш chat_id: `{chat_id}`\n\n"
            f"Имя: {user.first_name}\n"
            f"Фамилия: {user.last_name or 'Не указана'}\n"
            f"Username: @{user.username or 'Не указан'}"
        )

    async def on_request_received(self, *args):
        await self.send_message_to_users(args)

    async def send_message_to_users(self, *args):
        self._logger.debug(args)
        for user_id in self._config.bot.users_to_notify:
            await self.send_message_to_user(user_id, str(args))

    async def send_message_to_user(self, chat_id: int, text: str):
        try:
            await self._application.bot.send_message(chat_id=chat_id, text=text)
            return True
        except Exception as e:
            self._logger.error(f"Ошибка отправки сообщения пользователю {chat_id}: {e}")
            return False
