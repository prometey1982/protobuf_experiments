import proto.interface_pb2 as pb
import ssl
import logging
from google.protobuf import text_format
from data_providers import DataProviderBase
from notification import NotificationCallbackBase

from websockets.asyncio.server import serve


class WSHandler:
    def __init__(self, config, data_provider: DataProviderBase, notification_callback: NotificationCallbackBase):
        self._config = config
        self._data_provider = data_provider
        self._notification_callback = notification_callback
        self._logger = logging.getLogger(__name__)
        self._websockets = []
        self._handlers = {
            pb.RequestType.REQ_AVAILABLE_PROJECTS: self._handle_available_projects,
            pb.RequestType.REQ_PROJECT: self._handle_project,
            pb.RequestType.REQ_LOGS_UPLOAD: self._handle_logs_upload,
            pb.RequestType.REQ_FLASH_UPLOAD: self._handle_flash_upload,
        }
        self.ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        self.ssl_context.check_hostname = False
        self.ssl_context.load_cert_chain(self._config.connection.certificate)

    async def handle(self, websocket):
        self._websockets.append(websocket)
        incoming_message = await websocket.recv()
        self._logger.debug(f"Incoming message {incoming_message}, type {type(incoming_message)}")
        if type(incoming_message) == str:
            array_msg = bytearray()
            array_msg.extend(map(ord, incoming_message))
            incoming_message = array_msg
        request = pb.Request()
        request.ParseFromString(incoming_message)
        response = pb.Response()
        await self._process_proto_request(request, response)
        response_data = response.SerializeToString()
        await websocket.send(response_data)

    async def _process_proto_request(self, request: pb.Request, response: pb.Response):
        self._logger.debug(text_format.MessageToString(request))
        response.header.version = 1
        try:
            handler = self._handlers.get(request.header.request_type)
            if handler:
                await handler(request, response)
            else:
                self._logger.error(f"Unknown request_type = {request.header.request_type}")
        except Exception as e:
            self._logger.error(f"Request process error = {e}")
            response.header.response_type = pb.ResponseType.RESP_ERROR

    async def _handle_available_projects(self, request: pb.Request, response: pb.Response):
        vin = request.header.vin
        await self._notification_callback.on_request_received(f"Запрос доступных проектов от {vin}")
        available_projects_response = pb.AvailableProjectsResponse()
        available_projects = []
        for project in await self._data_provider.get_available_projects(vin):
            available_project = pb.AvailableProject()
            available_project.name = project.name
            available_project.crc = project.crc
            available_projects.append(available_project)
        response.header.response_type = pb.ResponseType.RESP_AVAILABLE_PROJECTS
        response.header.vin = vin
        response.header.version = 1
        response.available_projects.available_projects.extend(available_projects)

    async def _handle_project(self, request: pb.Request, response: pb.Response):
        vin = request.header.vin
        self._notification_callback.on_request_received(f"Запрос проекта от {vin}")

    async def _handle_logs_upload(self, request: pb.Request, response: pb.Response):
        vin = request.header.vin
        self._notification_callback.on_request_received(f"Запрос на загрузку логов от {vin}")

    async def _handle_flash_upload(self, request: pb.Request, response: pb.Response):
        vin = request.header.vin
        self._notification_callback.on_request_received(f"Запрос на загрузку прошивки от {vin}")

    async def run(self):
        self._server = await serve(self.handle, self._config.connection.host, self._config.connection.port,
                         ssl=self.ssl_context)
        await self._server.serve_forever()

    async def stop(self):
        if self._server:
            self._server.close()
            await self._server.wait_closed()
