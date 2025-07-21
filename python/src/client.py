import pathlib
import ssl
import proto.interface_pb2

from websockets.sync.client import connect

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
localhost_pem = pathlib.Path(__file__).with_name("srv_pub.pem")
ssl_context.load_verify_locations(localhost_pem)

def hello():
    uri = "wss://localhost:8080"
    with connect(uri, ssl=ssl_context) as websocket:
        request = proto.interface_pb2.Request()
        request.
        websocket.send(name)
        print(f">>> {name}")

        greeting = websocket.recv()
        print(f"<<< {greeting}")

if __name__ == "__main__":
    hello()
