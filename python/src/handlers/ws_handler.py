import proto.interface_pb2 as pb
import asyncio
import pathlib
import ssl

from websockets.asyncio.server import serve

class WSHandler:
    def __init__(self):
        self.ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        self.ssl_context.check_hostname = False
        pem_path = pathlib.Path(__file__).parent.with_name("srv_cert.pem")
        self.ssl_context.load_cert_chain(pem_path)

    async def handle(self, websocket):
        incoming_message = await websocket.recv()
        request = pb.Request()
        request.ParseFromString(incoming_message)
        print(f"header: request_type = {request.header.request_type}, version = {request.header.version}, vin = {request.header.vin}")
        response = pb.Response()
        header = pb.ResponseHeader()
        header.version = 1
        header.response_type = pb.ResponseType.RESP_AVAILABLE_PROJECTS
        response.header.CopyFrom(header)
        response_data = response.SerializeToString()
        await websocket.send(response_data)

    async def run(self):
        async with serve(self.handle, "localhost", 8080, ssl=self.ssl_context) as server:
            await server.serve_forever()
