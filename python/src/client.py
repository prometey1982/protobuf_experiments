import pathlib
import ssl
from collections import namedtuple

import proto.interface_pb2 as pb

from websockets.sync.client import connect


def get_ssl_context():
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_NONE  # Consider security implications
    localhost_pem = pathlib.Path(__file__).parent / "srv_pub.pem"

    if localhost_pem.exists():
        ssl_context.load_verify_locations(localhost_pem)

    return ssl_context


def hello():
    ssl_context = get_ssl_context()
    uri = "wss://localhost:8080"

    try:
        with connect(uri, ssl=ssl_context) as websocket:
            header = pb.RequestHeader()
            header.request_type = pb.REQ_AVAILABLE_PROJECTS
            header.version = 1
            header.vin = 'myvin'
            request = pb.Request()
            request.header.CopyFrom(header)
            data = request.SerializeToString()
            websocket.send(data)
            print(f">>> Sent request of {len(data)} bytes")
            response_data = websocket.recv()
            response = pb.Response()
            response.ParseFromString(response_data)
            print(f"header: response_type = {response.header.response_type}, version = {response.header.version}")

            if response.available_projects:
                Project = namedtuple('Project', ['name', 'crc'])
                projects_to_request = {Project(x.name, x.crc) for x in response.available_projects.available_projects}
                for available_project in projects_to_request:
                    print(f"name = {available_project.name}, crc = {available_project.crc}")
                    header.request_type = pb.REQ_PROJECT
                    request = pb.Request()
                    request.header.CopyFrom(header)
                    project_request = pb.ProjectRequest()
                    project_request.name = available_project.name
                    request.project.CopyFrom(project_request)
                    data = request.SerializeToString()
                    websocket.send(data)
                    response_data = websocket.recv()
                    response = pb.Response()
                    response.ParseFromString(response_data)
                    print(f"header: response_type = {response.header.response_type}, version = {response.header.version}")


            else:
                print("No available projects received")

    except Exception as e:
        print(f"Connection error: {e}")


if __name__ == "__main__":
    hello()
