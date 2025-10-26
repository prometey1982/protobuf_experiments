import proto.interface_pb2 as pb
import asyncio
import pathlib
import ssl

from websockets.asyncio.server import serve

class WSHandler:
    def __init__(self, config):
        self._config = config
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
        incoming_message = await websocket.recv()
        request = pb.Request()
        request.ParseFromString(incoming_message)
        response = pb.Response()
        await self._process_proto_request(request, response)
        response_data = response.SerializeToString()
        await websocket.send(response_data)

    async def _process_proto_request(self, request: pb.Request, response: pb.Response):
        print(f"header: request_type = {request.header.request_type}, version = {request.header.version}, vin = {request.header.vin}")
        response.header.version = 1
        try:
            handler = self._handlers.get(request.header.request_type)
            if handler:
                await handler(request, response)
            else:
                print(f"Unknown request_type = {request.header.request_type}")
        except Exception as e:
            print(f"Request process error = {e}")
            response.header.response_type = pb.ResponseType.RESP_ERROR

    async def _handle_available_projects(self, request: pb.Request, response: pb.Response):
        vin = request.header.vin
        available_projects_response = pb.AvailableProjectsResponse()
        available_projects = []
        available_project = pb.AvailableProject()
        available_project.name = "test_project"
        available_project.crc = 0x1234
        available_projects.append(available_project)
        response.header.response_type = pb.ResponseType.RESP_AVAILABLE_PROJECTS
        response.available_projects.available_projects.extend(available_projects)

    async def _handle_project(self, request: pb.Request, response: pb.Response):
        pass

    async def _handle_logs_upload(self, request: pb.Request, response: pb.Response):
        pass

    async def _handle_flash_upload(self, request: pb.Request, response: pb.Response):
        pass

    async def run(self):
        async with serve(self.handle, self._config.connection.host, self._config.connection.port, ssl=self.ssl_context) as server:
            await server.serve_forever()
