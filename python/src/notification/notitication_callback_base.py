from abc import ABC, abstractmethod

class NotificationCallbackBase(ABC):
    @abstractmethod
    async def on_request_received(self, **kwargs):
        pass
