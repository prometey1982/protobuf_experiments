import asyncio
import pathlib
import ssl
import proto.interface_pb2 as pb

from websockets.sync.client import connect

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.check_hostname = False
localhost_pem = pathlib.Path(__file__).with_name("srv_pub.pem")
ssl_context.load_verify_locations(localhost_pem, )

def hello():
    uri = "wss://localhost:8080"
    with connect(uri, ssl=ssl_context) as websocket:
        header = pb.RequestHeader()
        header.request_type = pb.REQ_AVAILABLE_PROJECTS
        header.version = 1
        header.vin = 'myvin'
        request = pb.Request()
        request.header.CopyFrom(header)
        data = request.SerializeToString()
        websocket.send(data)
        print(f">>> {data}")

        response_data = websocket.recv()
        response = pb.Response()
        response.ParseFromString(response_data)
        print(f"header: response_type = {response.header.response_type}, version = {response.header.version}")

if __name__ == "__main__":
    hello()
