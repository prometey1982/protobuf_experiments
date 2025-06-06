import proto.interface_pb2
import asyncio
import pathlib
import ssl

from websockets.asyncio.server import serve

class WSHandler:
    def __init__(self):
        self.ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        self.ssl_context.options
        pem_path = pathlib.Path(__file__).parent.with_name("srv_cert.pem")
        self.ssl_context.load_cert_chain(pem_path)

    async def handle(self, websocket):
        incoming_message = await websocket.recv()
        request = proto.interface_pb2.Request()
        request.ParseFromString(incoming_message)


    async def run(self):
        async with serve(self.handle, "localhost", 8080, ssl=self.ssl_context) as server:
            await server.serve_forever()
